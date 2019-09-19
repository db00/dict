/**
 *
 gcc screenshot.c
 adb push screenshot.c /sdcard/screenshot.c && adb shell "cd  /data/data/com.n0n3m4.droidc/files/gcc/bin/ && /data/data/com.n0n3m4.droidc/files/gcc/bin/arm-linux-androideabi-gcc /sdcard/screenshot.c -lEGL -lGLESv1_CM -lGLESv2 -landroid -lvorbisfile -lvorbis -lm -ldl -llog -lz -I/storage/sdcard0/Android/data/com.n0n3m4.droidc/files/gcc/arm-linux-androideabi/include/SDL2  -Wl,--no-undefined -o /data/data/com.n0n3m4.droidc/files/gcc/bin/a.out && /data/data/com.n0n3m4.droidc/files/busybox sh -c /data/data/com.n0n3m4.droidc/files/gcc/bin/a.out" && adb pull /sdcard/s.bmp && s.bmp
 gcc screenshot.c && ./a.out

 adb shell "cd  /data/data/com.n0n3m4.droidc/files/gcc/bin/ && /data/data/com.n0n3m4.droidc/files/gcc/bin/arm-linux-androideabi-gcc /sdcard/screenshot.c -lSDL2_test -lSDL2 -lEGL -lGLESv1_CM -lGLESv2 -landroid -lsmpeg2 -lvorbisfile -lvorbis -lm -ldl -llog -lz -I/storage/sdcard0/Android/data/com.n0n3m4.droidc/files/gcc/arm-linux-androideabi/include/SDL2  -Wl,--no-undefined -o /data/data/com.n0n3m4.droidc/files/gcc/bin/a.out && /data/data/com.n0n3m4.droidc/files/busybox sh -c /data/data/com.n0n3m4.droidc/files/gcc/bin/a.out"
 adb shell /data/data/com.n0n3m4.droidc/files/gcc/bin/a.out 
 adb pull /sdcard/s.bmp && s.bmp

 adb shell screencap  /sdcard/s.png && adb pull /sdcard/s.png && s.png
 adb shell "cd sdcard && /sdcard/android/data/com.n0n3m4.droidc/files/gcc/bin/arm-linux-androideabi-gcc /sdcard/c/testver.c  -lSDL2_test -lSDL2 -lEGL -lGLESv1_CM -lGLESv2 -landroid -lsmpeg2 -lvorbisfile -lvorbis -lm -ldl -llog -lz -I/storage/sdcard0/Android/data/com.n0n3m4.droidc/files/gcc/arm-linux-androideabi/include/SDL2  -Wl,--no-undefined  -o /sdcard/a && /sdcard/a"
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/ioctl.h>

struct FB {
	unsigned short *bits;
	unsigned size;
	int fd;
	struct fb_fix_screeninfo fi;
	struct fb_var_screeninfo vi;
};
static struct FB g_fb;
/*
   　　位图文件的组成
   　　结构名称 符 号
   　　位图文件头 (bitmap-file header) BITMAPFILEHEADER bmfh
   　　位图信息头 (bitmap-information header) BITMAPINFOHEADER bmih
   　　彩色表　(color table) RGBQUAD aColors[]
   　　图象数据阵列字节 BYTE aBitmapBits[]
   */
typedef struct bmp_header 
{
	short twobyte			;//两个字节，用来保证下面成员紧凑排列，这两个字符不能写到文件中
	//14B
	char bfType[2]			;//!文件的类型,该值必需是0x4D42，也就是字符'BM'
	unsigned int bfSize		;//!说明文件的大小，用字节为单位
	unsigned int bfReserved1;//保留，必须设置为0
	unsigned int bfOffBits	;//!说明从文件头开始到实际的图象数据之间的字节的偏移量，这里为14B+sizeof()
}BMPHEADER;

typedef struct bmp_info
{
	//40B
	unsigned int biSize			;//!BMPINFO结构所需要的字数
	int biWidth					;//!图象的宽度，以象素为单位
	int biHeight				;//!图象的宽度，以象素为单位,如果该值是正数，说明图像是倒向的，如果该值是负数，则是正向的
	unsigned short biPlanes		;//!目标设备说明位面数，其值将总是被设为1
	unsigned short biBitCount	;//!比特数/象素，其值为1、4、8、16、24、或32
	unsigned int biCompression	;//说明图象数据压缩的类型
#define BI_RGB        0L	//没有压缩
#define BI_RLE8       1L	//每个象素8比特的RLE压缩编码，压缩格式由2字节组成（重复象素计数和颜色索引）；
#define BI_RLE4       2L	//每个象素4比特的RLE压缩编码，压缩格式由2字节组成
#define BI_BITFIELDS  3L	//每个象素的比特由指定的掩码决定。
	unsigned int biSizeImage	;//图象的大小，以字节为单位。当用BI_RGB格式时，可设置为0
	int biXPelsPerMeter			;//水平分辨率，用象素/米表示
	int biYPelsPerMeter			;//垂直分辨率，用象素/米表示
	unsigned int biClrUsed		;//位图实际使用的彩色表中的颜色索引数（设为0的话，则说明使用所有调色板项）
	unsigned int biClrImportant	;//对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要。
}BMPINFO;

typedef struct tagRGBQUAD {
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
	BMPINFO    bmiHeader;
	//RGBQUAD    bmiColors[1];
	unsigned int rgb[3];
} BITMAPINFO;

static int get_rgb888_header(int w, int h, BMPHEADER * head, BMPINFO * info)
{
	int size = 0;
	if (head && info) {
		size = w * h * 3;
		memset(head, 0, sizeof(* head));
		memset(info, 0, sizeof(* info));
		head->bfType[0] = 'B';
		head->bfType[1] = 'M';
		head->bfOffBits = 14 + sizeof(* info);
		head->bfSize = head->bfOffBits + size;
		head->bfSize = (head->bfSize + 3) & ~3;
		size = head->bfSize - head->bfOffBits;

		info->biSize = sizeof(BMPINFO);
		info->biWidth = w;
		info->biHeight = -h;
		info->biPlanes = 1;
		info->biBitCount = 24;
		info->biCompression = BI_RGB;
		info->biSizeImage = size;

		printf("rgb888:%dbit,%d*%d,%d\n", info->biBitCount, w, h, head->bfSize);
	}
	return size;
}

static int get_rgb565_header(int w, int h, BMPHEADER * head, BITMAPINFO * info)
{
	int size = 0;
	if (head && info) {
		size = w * h * 2;
		memset(head, 0, sizeof(* head));
		memset(info, 0, sizeof(* info));
		head->bfType[0] = 'B';
		head->bfType[1] = 'M';
		head->bfOffBits = 14 + sizeof(* info);
		head->bfSize = head->bfOffBits + size;
		head->bfSize = (head->bfSize + 3) & ~3;
		size = head->bfSize - head->bfOffBits;

		info->bmiHeader.biSize = sizeof(info->bmiHeader);
		info->bmiHeader.biWidth = w;
		info->bmiHeader.biHeight = -h;
		info->bmiHeader.biPlanes = 1;
		info->bmiHeader.biBitCount = 16;
		info->bmiHeader.biCompression = BI_BITFIELDS;
		info->bmiHeader.biSizeImage = size;

		info->rgb[0] = 0xF800;
		info->rgb[1] = 0x07E0;
		info->rgb[2] = 0x001F;

		printf("rgb565:%dbit,%d*%d,%d\n", info->bmiHeader.biBitCount, w, h, head->bfSize);
	}
	return size;
}

static int save_bmp_rgb565(FILE * hfile, int w, int h, void * pdata)
{
	int success = 0;
	int size = 0;
	BMPHEADER head;
	BITMAPINFO info;

	size = get_rgb565_header(w, h, &head, &info);
	if (size > 0) {
		fwrite(head.bfType, 1, 14, hfile);
		fwrite(&info, 1, sizeof(info), hfile);
		fwrite(pdata, 1, size, hfile);
		success = 1;
	}

	return success;
}

static int save_bmp_bgra8888(FILE * hfile, int w, int h, void * pdata)
{
	int success = 0;
	int size = 0;
	BMPHEADER head;
	BMPINFO info;
	size = get_rgb888_header(w, h, &head, &info);
//	printf("size:%d\n",size);
	if (size > 0 && size >= w*h*3) {
		if(hfile==NULL)return 0;
		fwrite(head.bfType, 1, 14, hfile);
		fwrite(&info, 1, sizeof(info), hfile);

		char *pixels = (char*)malloc(size);
		if(pixels==NULL)return 0;
		memset(pixels,0,size);
		int i=0;
		while(i<w*h){
			int y = i/g_fb.vi.xres;
			int x = i%g_fb.vi.xres;
			//printf("(%d,%d)",x,y);
			/*long int location = (x+g_fb->vi.xoffset) * (g_fb->vi.bits_per_pixel/8) +(y+g_fb->vi.yoffset) * g_fb->fi.line_length;*/
			long int location = x * g_fb.vi.bits_per_pixel/8 + y * g_fb.fi.line_length;
			*(pixels+i*3)=*((char*)(pdata+location)+2);//b
			*(pixels+i*3+1)=*((char*)(pdata+location)+1);//g
			*(pixels+i*3+2)=*((char*)(pdata+location));//r
			++i;
		}
		printf("size:%d\n",size);
		fwrite(pixels, 1, size, hfile);
		success = 1;
	}

	return success;
}

static int save_bmp_rgb888(FILE * hfile, int w, int h, void * pdata)
{
	int success = 0;
	int size = 0;
	BMPHEADER head;
	BMPINFO info;

	size = get_rgb888_header(w, h, &head, &info);
	if (size > 0) {
		fwrite(head.bfType, 1, 14, hfile);
		fwrite(&info, 1, sizeof(info), hfile);
		fwrite(pdata, 1, size, hfile);
		success = 1;
	}

	return success;
}

int save_bmp(const char * path, int w, int h, void * pdata, int bpp)
{
	int success = 0;
	FILE * hfile = NULL;

	do 
	{
		if (path == NULL || w <= 0 || h <= 0 || pdata == NULL) {
			printf("if (path == NULL || w <= 0 || h <= 0 || pdata == NULL)\n");
			break;
		}

		remove(path);
		hfile = fopen(path, "wb");
		if (hfile == NULL) {
			printf("open(%s) failed!\n", path);
			break;
		}

		printf("bpp:%d\n",bpp);
		switch (bpp)
		{
			case 16:
				success = save_bmp_rgb565(hfile, w, h, pdata);
				break;
			case 24:
				success = save_bmp_rgb888(hfile, w, h, pdata);
				break;
			case 32:
				success = save_bmp_bgra8888(hfile, w, h, pdata);
				break;
			default:
				printf("error: not support format!\n");
				success = 0;
				break;
		}
	} while (0);

	if (hfile != NULL)
		fclose(hfile);

	return success;
}

int fb_bpp(struct FB *fb)
{
	if (fb) {
		return fb->vi.bits_per_pixel;
	}
	return 0;
}

int fb_width(struct FB *fb)
{
	if (fb) {
		return fb->vi.xres;
	}
	return 0;
}

int fb_height(struct FB *fb)
{
	if (fb) {
		return fb->vi.yres;
	}
	return 0;
}

int fb_size(struct FB *fb)
{
	if (fb) {
		unsigned bytespp = fb->vi.bits_per_pixel / 8;
		return (fb->vi.xres * fb->vi.yres * bytespp);
	}
	return 0;
}

int fb_virtual_size(struct FB *fb)
{
	if (fb) {
		unsigned bytespp = fb->vi.bits_per_pixel / 8;
		return (fb->vi.xres_virtual * fb->vi.yres_virtual * bytespp);
	}
	return 0;
}

void * fb_bits(struct FB *fb)
{
	unsigned short * bits = NULL;
	if (fb) {
		int offset, bytespp;
		bytespp = fb->vi.bits_per_pixel / 8;

		/* HACK: for several of our 3d cores a specific alignment
		 * is required so the start of the fb may not be an integer number of lines
		 * from the base.  As a result we are storing the additional offset in
		 * xoffset. This is not the correct usage for xoffset, it should be added
		 * to each line, not just once at the beginning */
		//(x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +(y+vinfo.yoffset) * finfo.line_length;
		/*offset = (fb->vi.xoffset + fb->vi.res * fb->vi.yoffset) * bytespp;*/
		offset = (fb->vi.xoffset* bytespp + fb->fi.line_length* fb->vi.yoffset);
		bits = fb->bits + offset / sizeof(*fb->bits);
	}
	return bits;
}

void fb_update(struct FB *fb)
{
	if (fb) {
		fb->vi.yoffset = 1;
		ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi);
		fb->vi.yoffset = 0;
		ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi);
	}
}

static int fb_open(struct FB *fb)
{
	if (NULL == fb) {
		return -1;
	}

	fb->fd = open("/dev/graphics/fb0", O_RDONLY);
	if (fb->fd < 0) {
		printf("open(\"/dev/graphics/fb0\") failed!\n");
		return -1;
	}

	if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->fi) < 0) {
		printf("FBIOGET_FSCREENINFO failed!\n");
		goto fail;
	}
	if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0) {
		printf("FBIOGET_VSCREENINFO failed!\n");
		goto fail;
	}

	fb->bits = mmap(0, fb_virtual_size(fb), PROT_READ, MAP_SHARED, fb->fd, 0);
	if (fb->bits == MAP_FAILED) {
		printf("mmap() failed!\n");
		goto fail;
	}

	return 0;

fail:
	close(fb->fd);
	return -1;
}

static void fb_close(struct FB *fb)
{
	if (fb) {
		munmap(fb->bits, fb_virtual_size(fb));
		close(fb->fd);
		fb = 0;
	}
}

struct FB * fb_create(void)
{
	memset(&g_fb, 0, sizeof(struct FB));
	if (fb_open(&g_fb)) {
		return NULL;
	}
	return &g_fb;
}

void fb_destory(struct FB *fb)
{
	fb_close(fb);
}

int screen_shot(const char * path)
{
	struct FB * fb = NULL;
	fb = fb_create();
	if (fb) {
		save_bmp(path, fb_width(fb), fb_height(fb), fb_bits(fb), fb_bpp(fb));
		fb_destory(fb);
	}
	return 0;
}

int getpixl(int _x,int _y,int* r,int* g,int* b)
{

	struct FB * fb = &g_fb;
	if(fb == NULL) fb = fb_create();
	char* bits = fb_bits(fb);
	//	int diff= (_x+_y*fb->vi.xres) * (fb->vi.bits_per_pixel/8);
	long int diff= _x * fb->vi.bits_per_pixel/8 +_y * fb->fi.line_length;
	unsigned short color = (unsigned short)*(bits+diff);
	switch(fb_bpp(fb)){
		case 16:
			*r =(color >>11) & 0x001F;
			*g =(color >>5) & 0x003F;
			*b =(color)&0x001F;
			break;
		case 24:
			*r = *(bits+diff);
			*g = *(bits+diff+1);
			*b = *(bits+diff+2);
			break;
		case 32:
			*r = *(bits+diff);
			*g = *(bits+diff+1);
			*b = *(bits+diff+2);
			break;
	}
	/*if(fb) fb_destory(fb);*/
	return 0;
}

int main()
{
#ifdef __ANDROID__
	char path[256] = "/mnt/sdcard/s.bmp";
#else
	char path[256] = "s.bmp";
#endif
	printf("main enter!\n");
	screen_shot(path);
	printf("main exit!\n");
	//	int r,g,b;
	//	getpixl(200,25,&r,&g,&b);
	//	printf("color:%2x%2x%2x\n",r,g,b);
	//	if(&g_fb)fb_destory(&g_fb);
	return 0;
}

