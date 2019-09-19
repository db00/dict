/**
 * @file loading.c
 gcc -g -Wall -I"../SDL2/include/"  files.c mystring.c myregex.c update.c httploader.c bytearray.c loading.c tween.c ease.c sprite.c matrix.c array.c zip.c ipstring.c urlcode.c base64.c -lz -lcrypto -lpthread -lssl -lSDL2 -lm -D debug_loading && ./a.out
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2017-05-12
 */
#include "loading.h"

//static Data3d * data2D = NULL;
static Tween * tween = NULL;
static Sprite * earth_contener=NULL;
static Sprite * earth_sprite=NULL;

void Loading_show(int boolean,char * s)
{
	if(s){
		printf("%s\r\n",s);
	}
	if(earth_sprite==NULL)
	{
		char * path = decodePath("~/sound/1.bmp");
		if(!fileExists(path))
		{
			loadAndunzip("https://git.oschina.net/db0/kodi/raw/master/earth.zip","~/sound/");
		}
		if(!fileExists(path))
		{
			free(path);
			return;
		}
		earth_sprite = Sprite_new();
		earth_sprite->is3D = 1;
		earth_sprite->surface = (SDL_LoadBMP(path));
		SDL_Log("surface size:%dx%d",earth_sprite->surface->w,earth_sprite->surface->h);
		Data3d*_data3D = earth_sprite->data3d;
		if(_data3D==NULL){
			_data3D = Data3D_init();
			/*
			if(_data3D->programObject==0)
			{
				data2D = Data3D_init();
				Data3d_set(_data3D,data2D);
			}
			*/
			earth_sprite->data3d = _data3D;
			_data3D->numIndices = esGenSphere ( 20, .5f, &_data3D->vertices, &_data3D->normals, &_data3D->texCoords, &_data3D->indices );
		}
		//earth_sprite->alpha = 0.5;
	}
	if(earth_contener == NULL)
	{
		earth_contener = Sprite_new();
		Sprite_addChild(earth_contener,earth_sprite);
		earth_contener->x = stage->stage_w/2;
		earth_contener->y = stage->stage_h/2;
		earth_contener->w = stage->stage_w;
		earth_contener->h = stage->stage_h;
	}

	if(boolean)
	{
		//char * path = decodePath("~/sound/1.bmp");
		//earth_sprite->surface = (SDL_LoadBMP(path));
		Sprite_addChild(stage->sprite,earth_contener);
		earth_contener->visible = SDL_TRUE;
		if(tween == NULL)
		{
			earth_sprite->rotationY = 0;
			TweenObj * tweenObj = (TweenObj*)TweenObj_new(earth_sprite);
			//tweenObj->end->scaleX = 8.0;
			//tweenObj->end->scaleY = 4.0;
			//tweenObj->end->x=240-earth_sprite->w;
			//tweenObj->end->y=320-earth_sprite->h;
			//tweenObj->end->alpha=1.0;
			//tweenObj->end->rotationX=(double)360.0*4;//度数
			//tweenObj->end->rotationY=(double)360.0*4;//度数
			tweenObj->end->rotationY=(double)360.0;//度数
			tween = tween_to(earth_sprite,3000 ,tweenObj);
			//tween->ease = easeInOut_bounce;
			tween->loop = 1;
		}
	}else{
		if(earth_contener)
			earth_contener->visible = SDL_FALSE;
		//Stage_redraw();
		//if(Sprite_contains(stage->sprite,earth_contener)) Sprite_removeChild(stage->sprite,earth_contener);
		if(tween)
		{
			//Tween_kill(tween,1); tween = NULL;
		}
	}
}


#ifdef debug_loading




int main(int argc, char *argv[])
{
	Stage_init();


	Loading_show(1,"show");
	Loading_show(0,"hide");
	Loading_show(1,"show");
	//Loading_show(0,"hide");



	Stage_loopEvents();
	return 0;
}
#endif //debug_loading
