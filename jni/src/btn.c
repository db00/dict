/**
 * @file sprite.c
 gcc -Wall -I"../SDL2/include/" -I"../SDL2_ttf/" -DSTDC_HEADERS -I"../SDL2_image/" array.c tween.c ease.c utf8.c btn.c myregex.c dict.c sprite.c mystring.c textfield.c files.c matrix.c -lSDL2_image -lSDL2_ttf -LGLESv2 -lm -lSDL2 && ./a.out
 gcc -Wall -I"../SDL2/include/" -I"../SDL2_ttf/" -DSTDC_HEADERS -I"../SDL2_image/" btn.c myregex.c regex.c dict.c sprite.c mystring.c textfield.c files.c matrix.c -lSDL2_image -lSDL2_ttf -lmingw32 -lSDL2main -lSDL2 && a
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-07-21
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "sprite.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "matrix.h"
#include "textfield.h"
#include "dict.h"
#include "myregex.h"

typedef struct Button{
	void (*clicked)(struct Button*);
	Sprite * sprite;
	Sprite * bg;
	TextField * textfield;
}Button;

void mouseDown(SpriteEvent*e)
{
	Sprite * sprite1 = Sprite_getChildByName(stage->sprite,"sprite1");
	Point3d *p = Sprite_localToGlobal(sprite1,NULL);
	//Sprite * sprite2 = Sprite_getChildByName(stage->sprite,"sprite2");
	//Point3d *p = Sprite_localToGlobal(Sprite_getChildByName(sprite2,"sprite3"),NULL);
	//Point3d *p = Sprite_localToGlobal(sprite2,NULL);
	printf("mouseDown:-----------------------------%s,%d,%d,,,%d,%d\n"
			,e->target->name
			,stage->mouse->x
			,stage->mouse->y
			,p->x
			,p->y
		  );
	if(e->target->parent)
		Sprite_addChild(e->target->parent,e->target);
	if(e->target!= stage->sprite)e->target->y++;
}

int main(int argc, char *argv[])
{
	Stage_init();
	Sprite_addEventListener(stage->sprite,SDL_MOUSEBUTTONDOWN,mouseDown);
	if(stage->GLEScontext == NULL){
		//return 0;
	}
	//else
	{
		Sprite*sprite = Sprite_new();
#ifdef __ANDROID__
		sprite->surface = IMG_Load("ic_launcher.png");
#else
		sprite->surface = IMG_Load("../../res/drawable-hdpi/ic_launcher.png");
#endif
		//sprite->surface = IMG_Load("1.bmp");
		//sprite->texture = SDL_CreateTextureFromSurface(stage->renderer,sprite->surface);
		sprite->x = 30;
		sprite->y = 30;
		sprite->w = 100.0;
		sprite->h = 120.0;
		//sprite->rotation =120;
		//sprite->alpha =0.4;
		sprite->canDrag = SDL_TRUE;
		Sprite_addChild(stage->sprite,sprite);
		Sprite_addEventListener(sprite,SDL_MOUSEBUTTONDOWN,mouseDown);
		//Point3d *gp = Sprite_localToGlobal(sprite,NULL);
		//printf("local:%d,%d---------global:%f,%f\n",sprite->x,sprite->y,gp->x,gp->y);

	}
	{	
		TextField* txt2 = TextField_new();
		txt2->w = stage->stage_w;
		txt2->h = stage->stage_h;
		txt2->sprite->canDrag = 1;
		txt2 = TextField_appendText(txt2,"");

		char * explain = NULL;
		Dict * dict = NULL;
		dict = Dict_new(); dict->name = "oxford-gb"; explain = Dict_explain(dict,"help"); if(dict)Dict_free(dict); dict = NULL;

		if(explain){
			char *tmp = regex_replace_all(explain,"([^a-zA-Z])( [\\*0-9]+ )","$1\r\n$2");
			free(explain);
			explain = tmp;
			//printf("%s",explain);
			txt2 = TextField_appendText(txt2,explain);
		}
		Sprite_addChild(stage->sprite,txt2->sprite);
	}

	printf("stage ----------- size:%dx%d\n",stage->stage_w,stage->stage_h);
	Stage_loopEvents();
	//SDL_Delay(20);
	return 0;
}
