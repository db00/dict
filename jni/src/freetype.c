/**
 *
  gcc -g -D debug_freetyp -Wall -I"../SDL2/include/" -I"/usr/include/freetype2/" -lfreetype -I"../SDL2_image" -I"../SDL2_ttf/" -lSDL2_ttf -lSDL2_image -lSDL2 myfont.c bytearray.c loading.c update.c zip.c textfield.c httpserver.c array.c filetypes.c urlcode.c utf8.c dict.c sqlite.c tween.c ease.c sprite.c matrix.c myregex.c freetype.c files.c httploader.c ipstring.c mystring.c base64.c -lssl -lsqlite3 -lpthread -ldl -lz -lcrypto -lm && ./a.out
  gcc freetype.c -lfreetype -I"/usr/include/freetype2/" -lm && ./a.out DroidSansFallback.ttf 天
https://www.freetype.org/freetype2/docs/tutorial/step1.html#section-4
*/
/*                                                                 */
/* This small program shows how to print a rotated string with the */
/* FreeType 2 library.                                             */


#include <stdio.h>
#include <string.h>
#include <math.h>

#include "sprite.h"
#include "files.h"
#include "SDL_image.h"

#include <ft2build.h>
#include FT_FREETYPE_H


#define WIDTH   160
#define HEIGHT 60


/* origin is the upper left corner */
unsigned char image[HEIGHT][WIDTH];


/* Replace this function with something useful. */

	void
draw_bitmap( FT_Bitmap*  bitmap,
		FT_Int      x,
		FT_Int      y)
{
	FT_Int  i, j, p, q;
	FT_Int  x_max = x + bitmap->width;
	FT_Int  y_max = y + bitmap->rows;


	for ( i = x, p = 0; i < x_max; i++, p++ )
	{
		for ( j = y, q = 0; j < y_max; j++, q++ )
		{
			if ( i < 0      || j < 0       ||
					i >= WIDTH || j >= HEIGHT )
				continue;

			image[j][i] |= bitmap->buffer[q * bitmap->width + p];
		}
	}
}

Sprite * container;
static void showBox(int x,int y,int w)
{
	Sprite*sprite = Sprite_new();
	char sname[] = "earth";
	sprite->name = malloc(sizeof(sname)+1);
	memset(sprite->name,0,sizeof(sname)+1);
	strcpy(sprite->name,sname);
	char * filepath = decodePath("~/sound/1.bmp");
	sprite->surface = IMG_Load(filepath);

	Data3d*_data3D = sprite->data3d;
	if(_data3D==NULL){
		_data3D = (Data3d*)malloc(sizeof(Data3d));
		memset(_data3D,0,sizeof(Data3d));

		if(_data3D->programObject==0){
			Data3d *data2D = Data3D_init();
			Data3d_set(_data3D,data2D);
		}
		sprite->data3d = _data3D;
		//_data3D->numIndices = esGenSphere ( 20, 0.5f, &_data3D->vertices, &_data3D->normals, &_data3D->texCoords, &_data3D->indices);
		//_data3D->numIndices = esGenSphere ( 20, 0.5f, &_data3D->vertices, NULL, &_data3D->texCoords, &_data3D->indices);
		_data3D->numIndices = esGenCube( w*1.0/stage->stage_w, &_data3D->vertices, &_data3D->normals, &_data3D->texCoords, &_data3D->indices);
	}
	//sprite->filter = 1;
	sprite->x = w/2 - stage->stage_w/2;
	sprite->y = w/2 - stage->stage_h/2;
	sprite->scaleZ = 5.0;
	//sprite->alpha = 0.9;
	//sprite->z = -100;
	//sprite->w = stage->stage_w - sprite->x*2;
	//sprite->h = stage->stage_h - sprite->y*2;
	Sprite*sprite2 = Sprite_new();
	sprite2->x = x;
	sprite2->y = y;
	Sprite_addChildAt(container,sprite2,0);
	Sprite_addChild(sprite2,sprite);
}


void show_image( void )
{
	int  i, j;

	//showBox(0,0,10);
	//showBox(0,10,10);

	for ( i = 0; i < HEIGHT; i++ )
	{
		for ( j = 0; j < WIDTH; j++ )
		{
			putchar( image[i][j] == 0 ? ' '
					: image[i][j] < 128 ? '+'
					: '*' );
			//if(image[i][j]<128 && image[i][j]>0)
			if(image[i][j]>128)
			{
				showBox(j*10*stage->stage_w/240.0,i*10*stage->stage_h/320.0,14*stage->stage_w/240.0);
			}
		}
		putchar( '\n' );
	}
}



int getUtf8size(char * utf8)
{
	unsigned char c = *utf8;
	if(c<0x80)
		return 1;
	if(c<0xe0)
		return 2;
	if(c<0xf0)
		return 3;
	if(c<0xf8)
		return 4;
	if(c<0xfe)
		return 5;
	if(c<0xff)
		return 6;
	return 0;
}
unsigned short utf2u(char * utf,size_t * outlen)
{
	int len = getUtf8size(utf);
	char * out = (char*)malloc(len+1);
	printf("len:%d\n",len);
	memset(out,0,len+1);

	unsigned short int c = 0;
	int i = 0;
	switch(len)
	{
		case 1:
			c = *utf;
			out[i++]=c;
			break;
		case 2:
			c += ((utf[0] & 0x1f)<<6);
			c += (utf[1] & 0x3f);
			out[i++]= *((char*)&c);
			out[i++]=c & 0xff;
			break;
		case 3:
			c += ((utf[0]<<12));
			c += ((utf[1] & 0x3f)<<6);
			c += ((utf[2] & 0x3f));
			out[i++]=(char)(c>>8) & 0xff;
			out[i++]=(char)c & 0xff;
			break;
		default:
			free(out);
			return 0;
	}
	if(outlen)
		*outlen = i;
	printf("0x5929=0x%x,%s\n",c,utf);
	return c;
}

static void mousehandl(SpriteEvent*e)
{
	Sprite * sprite = (Sprite *)e->target;
	SDL_Event * event = (SDL_Event*)e->e;
	switch(e->type){
		case SDL_MOUSEMOTION:
			//if(e->target->parent) Sprite_addChildAt(e->target->parent,e->target,0);
			if(event->motion.state){
				sprite->rotationX += event->motion.yrel;
				sprite->rotationY += event->motion.xrel;
				Stage_redraw();
			}
			break;
	}
}

#ifdef debug_freetyp
int main( int argc, char**  argv )
{
	FT_Library    library;
	FT_Face       face;

	FT_GlyphSlot  slot;
	FT_Matrix     matrix;                 /* transformation matrix */
	FT_Vector     pen;                    /* untransformed origin  */
	FT_Error      error;

	char*         filename;
	//char*         text;
	unsigned short text;

	double        angle;
	int           target_height;
	int           n, num_chars;


	//return 0;

	Stage_init();
	container = Sprite_new();
	Sprite_addChild(stage->sprite,container);
	container->mouseChildren = 0;
	container->x = stage->stage_w/2;
	container->y = -stage->stage_h/2;
	Sprite_addEventListener(container,SDL_MOUSEMOTION,mousehandl);
	Sprite_addEventListener(container,SDL_MOUSEBUTTONDOWN,mousehandl);
	Sprite_addEventListener(container,SDL_MOUSEBUTTONUP,mousehandl);
	GL_CHECK(gles2.glEnable(GL_DEPTH_TEST));





	size_t outlen;

	if(argc>1)
		filename      = argv[1];                           /* first argument     */
	else{
		filename = decodePath("~/sound/DroidSansFallback.ttf");
	}
	if(argc>2)
		text          = utf2u(argv[2],&outlen);                           /* second argument    */
	else
		text          = utf2u("中",&outlen);                           /* second argument    */
	num_chars     = 1;//strlen( text );
	angle         = ( 25.0 / 180 ) * 3.14159 * 0;      /* use 25 degrees     */
	target_height = HEIGHT;

	error = FT_Init_FreeType( &library );              /* initialize library */
	/* error handling omitted */

	error = FT_New_Face( library, filename, 0, &face );/* create face object */
	/* error handling omitted */

	/* use 50pt at 100dpi */
	error = FT_Set_Char_Size( face, 16 * 64, 0,
			100, 0 );                /* set character size 16 */
	/* error handling omitted */
	if(error){
	}

	slot = face->glyph;

	/* set up matrix */
	matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
	matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
	matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
	matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,55) relative to the upper left corner  */
	pen.x = 300 * 64 * 0;
	pen.y = ( target_height - 55 ) * 64 ;

	for ( n = 0; n < num_chars; n++ )
	{
		/* set transformation */
		FT_Set_Transform( face, &matrix, &pen );

		/* load glyph image into the slot (erase previous one) */
		//error = FT_Load_Char(face, text[n], FT_LOAD_RENDER );
		error = FT_Load_Char(face,text, FT_LOAD_RENDER );
		//if ( error ) continue;                 /* ignore errors */

		/* now, draw to our target surface (convert position) */
		draw_bitmap( &slot->bitmap,
				slot->bitmap_left,
				target_height - slot->bitmap_top 
				);

		/* increment pen position */
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}

	show_image();

	FT_Done_Face    ( face );
	FT_Done_FreeType( library );

	Stage_loopEvents();
	return 0;
}
#endif

/* EOF */
