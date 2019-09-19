/**
 * @file filter.c
 gcc -g -Wall -I"../SDL2_image/" -I"../SDL2/include/" filter.c array.c sprite.c matrix.c -lSDL2 -lSDL2_image -lm -D debug_filter && ./a.out
 gcc -g -Wall -I"../SDL2/include/" filter.c array.c sprite.c matrix.c -lSDL2 -lm -D debug_filter && ./a.out
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2016-09-23
 */
#include "sprite.h"
typedef struct Filter
{
	int matrixX;//矩阵的 x 维度（矩阵中列的数目）。默认值为 0。 
	int matrixY;//矩阵的 y 维度（矩阵中行的数目）。默认值为 0。 
	int * matrix;/*用于矩阵转换的值的数组。数组中的项数必须等于 matrixX * matrixY。

				   矩阵盘绕基于一个 n x m 矩阵，该矩阵说明输入图像中的给定像素值如何与其相邻的像素值合并以生成最终的像素值。每个结果像素通过将矩阵应用到相应的源像素及其相邻像素来确定。

				   对于 3 x 3 矩阵卷积，将以下公式用于每个独立的颜色通道：


				   dst (x, y) = ((src (x-1, y-1) * a0 + src(x, y-1) * a1....
				   src(x, y+1) * a7 + src (x+1,y+1) * a8) / divisor) + bias


				   某些规格的滤镜在由提供 SSE（SIMD 流扩展）的处理器运行时执行速度更快。以下是更快的卷积操作的条件：

				   滤镜必须是 3x3 滤镜。
				   所有滤镜项必须是介于 -127 和 +127 之间的整数。
				   所有滤镜项的总和不能包含大于 127 的绝对值。
				   如果任何滤镜项为负，则除数必须介于 2.00001 和 256 之间。
				   如果所有滤镜项都为正，则除数必须介于 1.1 和 256 之间。
				   偏差必须是整数。*/
	int divisor;//矩阵转换中使用的除数。默认值为 1。如果除数是所有矩阵值的总和，则可调平结果的总体色彩强度。忽略 0 值，此时使用默认值。 
	int bias;//要添加到矩阵转换结果中的偏差量。偏差可增加每个通道的颜色值，以便暗色变得较明亮。默认值为 0。
	int preserveAlpha;//表示是否已保留 Alpha 通道并且不使用滤镜效果，或是否对 Alpha 通道以及颜色通道应用卷积滤镜。值为 false 表示卷积应用于所有通道，包括 Alpha 通道。值为 true 表示只对颜色通道应用卷积。默认值为 true。 
	int clamp;//表示是否应锁定图像。对于源图像之外的像素，如果值为 true，则表明通过复制输入图像每个相应的边缘处的颜色值，沿着输入图像的每个边框按需要扩展输入图像。如果值为 false，则表明应按照 color 和 alpha 属性中的指定使用其他颜色。默认值为 true。 
	int color;//要替换源图像之外的像素的十六进制颜色。它是一个没有 Alpha 成分的 RGB 值。默认值为 0。 
	float alpha;//替换颜色的 Alpha 透明度值。有效值为 0 到 1.0。默认值为 0。例如，0.25 设置透明度值为 25%。


	char * operation;//— 下列比较运算符之一（作为字符串传递）:“<”、“<=”、“>”、“>=”、“==”“!=”
	unsigned int threshold;//:uint — 测试每个像素时要比较的值，以查看该值是达到还是超过阈值。
	unsigned int passcolor;//:uint (default = 0) — 阈值测试成功时对像素设置的颜色值。默认值为 0x00000000。
	unsigned int mask;//:uint (default = 0xFFFFFFFF) — 用于隔离颜色成分的遮罩。
	int copySource;//:Boolean (default = false) — 如果该值为 true，则源图像中的像素值将在阈值测试失败时复制到目标图像。如果为 false，则在阈值测试失败时不会复制源图像。 


	/**
	 *
	 由 20 个项目组成的数组，适用于 4 x 5 颜色转换。matrix 属性不能通过直接修改它的值来更改（例如 myFilter.matrix[2] = 1;）。相反，必须先获取对数组的引用，对引用进行更改，然后重置该值。

	 颜色矩阵滤镜将每个源像素分离成它的红色、绿色、蓝色和 Alpha 成分，分别以 srcR、srcG、srcB 和 srcA 表示。要计算四个通道中每个通道的结果，可将图像中每个像素的值乘以转换矩阵中的值。（可选）可以将偏移量（介于 -255 至 255 之间）添加到每个结果（矩阵的每行中的第五项）中。滤镜将各颜色成分重新组合为单一像素，并写出结果。在下列公式中，a[0] 到 a[19] 对应于由 20 个项目组成的数组中的条目 0 至 19，该数组已传递到 matrix 属性：

	 redResult   = (a[0]  * srcR) + (a[1]  * srcG) + (a[2]  * srcB) + (a[3]  * srcA) + a[4]
	 greenResult = (a[5]  * srcR) + (a[6]  * srcG) + (a[7]  * srcB) + (a[8]  * srcA) + a[9]
	 blueResult  = (a[10] * srcR) + (a[11] * srcG) + (a[12] * srcB) + (a[13] * srcA) + a[14]
	 alphaResult = (a[15] * srcR) + (a[16] * srcG) + (a[17] * srcB) + (a[18] * srcA) + a[19]


	 对于数组中的每个颜色值，值 1 等于正发送到输出的通道的 100%，同时保留颜色通道的值。

	 计算是对非相乘的颜色值执行的。如果输入图形由预先相乘的颜色值组成，这些值会自动转换为非相乘的颜色值以执行此操作。

*/
	float * colorMatrix;
}Filter;

void Filter_free(Filter * filter)
{
	if(filter)
	{
		if(filter->matrix)
			free(filter->matrix);
		free(filter);
	}
}

Filter * Filter_new()
{
	Filter * filter = NULL;
	filter = malloc(sizeof(*filter));
	memset(filter,0,sizeof(*filter));
	filter->clamp = 1;
	filter->preserveAlpha = 1;
	filter->divisor = 1;
	filter->mask = 0xFFFFFFFF;
	return filter;
}

unsigned char * getPixel(int x, int y,unsigned char * pixels,int w,int h)
{
	if(x<w && y<h && x>=0 && y>=0)
		return pixels + (x + y*w)*4;
	return NULL;
}

/* -----------------------------------------------*/
/**
 *  
 *
 * @param filter
 * @param rgba：0：b,1:g,2:r,3:a
 * @param x
 * @param y
 * @param pixels
 * @param w
 * @param h
 *
 * @return  
 */
unsigned char ConvolutionFilter(Filter * filter,int rgba,int x,int y,unsigned char * pixels,int w,int h)
{
	int pixel = 0;
	int _x = 0;
	while(_x<filter->matrixX)
	{
		int _y = 0;
		while(_y<filter->matrixY)
		{
			int k = filter->matrix[_x+_y*filter->matrixX];
			if(k){
				unsigned char * p = getPixel(x+_x,y+_y,pixels,w,h)+rgba;
				//#if SDL_BYTEORDER == SDL_LIL_ENDIAN
				if(p==NULL)
					p = (unsigned char*)(&(filter->color))+rgba;
				//#else
				//if(p==NULL)
				//p = (unsigned char*)(&(filter->color)) + (3-rgba);
				//#endif
				//# if __WORDSIZE == 64
				//#else
				//#endif
				pixel += (*p) * k;
			}
			++_y;
		}
		++_x;
	}
	int r = pixel*1.0/filter->divisor;
	//printf("pixel:%x,%x\n",pixel,r); exit(0);
	if(r>0xff) r=0xff;
	return (unsigned char)r;

}

/**
 *
 * src_bmp: source bitmap
 * w: source bitmap width
 * h: source bitmap height
 * 
 * */
unsigned char * Filter_apply(Filter * filter,unsigned char * src_bmp,int w,int h,SDL_Rect * clip_rect)
{
	if(src_bmp==NULL)
		return NULL;
	int startx = 0;
	int starty = 0;
	//int endx = w;
	int endy = h;
	int targetw = w;
	int targeth = h;
	if(clip_rect)
	{
		startx = clip_rect->x;
		starty = clip_rect->y;
		targetw = clip_rect->w;
		targeth = clip_rect->h;
		if(startx+targetw>w) targetw = w-startx;
		if(starty+targeth>h) targeth = h-starty;
		//endx = startx+targetw;
		endy = starty+targeth;
	}
	if(targetw<=0 || targeth<=0)
		return NULL;
	unsigned char * bitmap = malloc(targetw*targeth*4);

	memset(bitmap,0,targetw*targeth*4);
	if(filter == NULL || filter->matrix==NULL || filter->matrixX*filter->matrixY == 0)
	{
		int _y = starty;
		int cury = 0;
		while(_y<endy)
		{
#if 1
			memcpy(bitmap+(cury*w)*4,src_bmp+(startx+_y*w)*4,targetw*4);//copy a line
#else
			int _x = 0;
			while(_x<targetw)
			{
				unsigned char r=0x0;
				unsigned char g=0x0;
				unsigned char b=0x0;
				unsigned char a=0xff;

				unsigned char * p = getPixel(_x,_y,src_bmp,w,h);
				r = *(p+0);
				g = *(p+1);
				b = *(p+2);
				a = *(p+3);

				bitmap[(_x+_y*targetw)*4+0]=r;
				bitmap[(_x+_y*targetw)*4+1]=g;
				bitmap[(_x+_y*targetw)*4+2]=b;
				bitmap[(_x+_y*targetw)*4+3]=a;
				++_x;
			}

#endif
			++_y;
			++cury;
		}
		return bitmap;
	}else{
		int _x = 0;
		while(_x<targetw-filter->matrixX)
		{
			int _y = 0;
			while(_y<targeth-filter->matrixY)
			{
				unsigned char r=0x0;
				unsigned char g=0x0;
				unsigned char b=0x0;
				unsigned char a=0xff;
				//a = 0;

				if(filter->mask)
				{
					if((filter->mask & 0xff000000) == 0xff000000)
						r = ConvolutionFilter(filter,0,startx+_x,starty+_y,src_bmp,w,h)+filter->bias;
					if((filter->mask & 0x00ff0000) == 0x00ff0000)
						g = ConvolutionFilter(filter,1,startx+_x,starty+_y,src_bmp,w,h)+filter->bias;
					if((filter->mask & 0x0000ff00) == 0x0000ff00)
						b = ConvolutionFilter(filter,2,startx+_x,starty+_y,src_bmp,w,h)+filter->bias;
					//if(filter->mask & 0x000000ff) a = ConvolutionFilter(filter,3,startx+_x,starty+_y,src_bmp,w,h)+filter->bias;
				}
				if(filter->operation)
				{
					unsigned int color = (r<<24)+(g<<16)+(b<<8)+a;
					int pass = 0;
					if(strcmp(filter->operation,"<")==0) {
						if((filter->mask & color) < filter->threshold)
							pass = 1;
					}else if(strcmp(filter->operation,"<=")==0){
						if((filter->mask & color) <= filter->threshold)
							pass = 1;
					}else if(strcmp(filter->operation,">")==0){
						if((filter->mask & color) > filter->threshold)
							pass = 1;
					}else if(strcmp(filter->operation,">=")==0){
						if((filter->mask & color) >= filter->threshold)
							pass = 1;
					}else if(strcmp(filter->operation,"!=")==0){
						if((filter->mask & color) != filter->threshold)
							pass = 1;
					}else if(strcmp(filter->operation,"==")==0){
						if((filter->mask & color) == filter->threshold)
							pass = 1;
					}
					if(pass){
						color = filter->passcolor;
					}else if(!filter->copySource){
						color = 0;
					}
					r = *((char*)&color+3);
					g = *((char*)&color+2);
					b = *((char*)&color+1);
					a = *((char*)&color+0);
				}
				if(filter->colorMatrix)
				{
					unsigned char srcR = r;
					unsigned char srcG = g;
					unsigned char srcB = b;
					unsigned char srcA = a;
					float *_arr = filter->colorMatrix;
					r = (_arr[0]  * srcR) + (_arr[1]  * srcG) + (_arr[2]  * srcB) + (_arr[3]  * srcA) + _arr[4];
					g = (_arr[5]  * srcR) + (_arr[6]  * srcG) + (_arr[7]  * srcB) + (_arr[8]  * srcA) + _arr[9];
					b= (_arr[10] * srcR) + (_arr[11] * srcG) + (_arr[12] * srcB) + (_arr[13] * srcA) + _arr[14];
					a= (_arr[15] * srcR) + (_arr[16] * srcG) + (_arr[17] * srcB) + (_arr[18] * srcA) + _arr[19];
				}
				/*
				*/

				//if(!filter->preserveAlpha) a = filter->alpha;
				//r=0xff;
				//a=0xff;

				bitmap[(_x+_y*targetw)*4+0]=r;
				bitmap[(_x+_y*targetw)*4+1]=g;
				bitmap[(_x+_y*targetw)*4+2]=b;
				bitmap[(_x+_y*targetw)*4+3]=a;
				++_y;
			}
			++_x;
		}
	}
	return bitmap;
}

Filter * Filter_setMatrix(Filter * filter,int * arr,int matrixX,int matrixY)
{
	int len = matrixX * matrixY;
	//printf("%d:%d,%d,",len,matrixX,matrixY);

	if(len>0){
		filter->matrixX = matrixX;
		filter->matrixY = matrixY;
		filter->matrix = malloc(len*sizeof(int));
		memset(filter->matrix,0,len*sizeof(int));
		printf("[");
		int i = 0;
		while(i<len)
		{
			//printf("%d:%d,",i,arr[i]);
			filter->matrix[i] = arr[i];
			printf("%d:%d,",i,filter->matrix[i]);
			++i;
		}
		printf("]\n");
	}else{
		return filter;
	}
	return filter;
}

#ifdef debug_filter
#include "SDL_image.h"
//#define debug 1
int main()
{
	int x = 0x12345678;
	char * p = (char*)&x;
	printf("%x\n",*p);
	printf("%x\n",*(p+3));

	Stage_init();

	//SDL_SetWindowOpacity(stage->window,.5);
	//SDL_SetWindowBrightness(stage->window,1.0);
	Sprite*sprite = Sprite_new();
	//sprite->surface = RGBA_surface(SDL_LoadBMP("/home/db0/sound/1.bmp"));
	sprite->surface = RGBA_surface(IMG_Load("/home/db0/sound/img/9.jpg"));
	Filter * filter = Filter_new();
#if debug==1
	//锐化
	int arr[]={
		0,-1,0,
		-1,5,-1,
		0,-1,0
	};
#elif debug==2
	//模糊
	int arr[]={
		1,1,1,
		1,1,1,
		1,1,1
	};
	filter->divisor = 9;
#elif debug==3
	//浮雕
	int arr[]={
		2,0,0,
		0,-1,0,
		0,0,-1
	};
#else
	//边缘
	int arr[]={
		0,-1,0,
		-1,4,-1,
		0,-1,0
	};
	float colorMatrix[] = {
		/*
		//replace red blue
		0.0,0.0,1.0,0.0,0.0,    
		0.0,1.0,0.0,0.0,0.0,    
		1.0,0.0,0.0,0.0,0.0,    
		0.0,0.0,0.0,1.0,0.0
		*/

		//black white
		1.0/3,1.0/3,1.0/3,0.0,0.0,    
		1.0/3,1.0/3,1.0/3,0.0,0.0,    
		1.0/3,1.0/3,1.0/3,0.0,0.0,    
		0.0,  0.0,  0.0,1.0,0.0
			/*
			 * //old photo
			 1.0/2,1.0/2,1.0/2,0.0,0.0,    
			 1.0/3,1.0/3,1.0/3,0.0,0.0,    
			 1.0/4,1.0/4,1.0/4,0.0,0.0,    
			 0.0,0.0,0.0,1.0,0.0
			 */
	};
	filter->colorMatrix = colorMatrix;
#endif
	filter = Filter_setMatrix(filter,arr,3,3);
	fflush(stdout);
	//return 0;
	//filter = NULL;
	sprite->surface->pixels = Filter_apply(filter,sprite->surface->pixels,sprite->surface->w,sprite->surface->h,NULL);
	Sprite_addChild(stage->sprite,sprite);
	//sprite->w = stage->stage_w;
	//sprite->h = stage->stage_h;
	Filter_free(filter);
	Stage_loopEvents();
	return 0;
}
#endif
