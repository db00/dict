/**
 * @file searhdict.c
 gcc -D test_searchdict searhdict.c readbaidu.c cJSON.c datas.c sqlite.c btnlist.c wordinput.c alert.c explain.c sidebtns.c myfont.c update.c mylist.c pictures.c bytearray.c zip.c loading.c besier.c testwords.c sdlstring.c sqlite3.c input.c textfield.c dict.c music.c urlcode.c base64.c myregex.c files.c httploader.c ease.c tween.c utf8.c ipstring.c sprite.c array.c mystring.c matrix.c mysurface.c -lssl -lcrypto -lm -ldl -lpthread -lSDL2 -lz -lSDL2_ttf -lSDL2_mixer -lSDL2_image -I"../SDL2_mixer/"  -I"../SDL2_ttf/" -I"../SDL2_image/" -I"../SDL2/include/" && ./a.out 
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2016-09-22
 *
 *
 * 单词纠错
 * 单词导入
 * 中文read
 * 上一词，下一词
 *
 */

#include "searhdict.h"
#include "update.h"
#include "sidebtns.h"
#include "wordinput.h"
#include "btnlist.h"


static void * uiThread(void *ptr){
	if(dictContainer==NULL){
		dictContainer = Sprite_new();
		dictContainer->surface = Surface_new(1,1);
		char pixels[4] ={'\0','\0','\0','\xff'};
		memcpy(dictContainer->surface->pixels,(char*)pixels,sizeof(pixels));
		Sprite_addChild(stage->sprite,dictContainer);
		dictContainer->w = stage->stage_w;
		dictContainer->h = stage->stage_h;

		Btnlist_show();
	}

	Wordinput_show();
	Sidebtns_show();


	char * clipboardtext = getClipboardText(0);
	if(clipboardtext && strlen(clipboardtext)>0){//显示剪切版单词
		if(regex_match(clipboardtext,"/^[\x01-\x7f]*$/"))
			Input_setText(input,clipboardtext);
	}

	SDL_StartTextInput();
	//pthread_exit(NULL);  
	return NULL;
}

static int startx;
static int starty;
static void stageMouseEvent(SpriteEvent* e)
{
	if(Sprite_getVisible(dictContainer)==0)
		return;
	//Sprite*target = e->target;
	SDL_Event* event = e->e;
	int w = Sidebtns_getWidth();
	if(w==0)
		w = 90;
	switch(event->type)
	{
		case SDL_MOUSEBUTTONDOWN:
			startx = event->button.x;
			starty = event->button.y;
			Sprite_addEventListener(stage->sprite,SDL_MOUSEMOTION,stageMouseEvent); 
			break;
		case SDL_MOUSEBUTTONUP:
			Sprite_removeEventListener(stage->sprite,SDL_MOUSEMOTION,stageMouseEvent); 
			break;
		case SDL_MOUSEMOTION:
			//printf("\r\n%d",w); fflush(stdout);
			if(startx<stage->stage_w/10){
				if(event->button.x>stage->stage_w*2/10)
				{
					Sidebtns_hide();
					//printf("\r\nfrom left"); fflush(stdout);
					Stage_redraw();
					Sprite_removeEventListener(stage->sprite,SDL_MOUSEMOTION,stageMouseEvent); 
				}
			}else if(startx>stage->stage_w-w/2){//show 
				if(event->button.x<stage->stage_w-w)
				{
					Sidebtns_show();
					//printf("\r\nfrom right"); fflush(stdout);
					Stage_redraw();
					Sprite_removeEventListener(stage->sprite,SDL_MOUSEMOTION,stageMouseEvent); 
				}
				/*
				   }else if(starty<stage->stage_h/10){
				   if(event->button.y>stage->stage_h*2/10)
				   {
				//printf("\r\nfrom top\r\n"); fflush(stdout);
				//Sidebtns_hide();
				//Stage_redraw();
				Sprite_removeEventListener(stage->sprite,SDL_MOUSEMOTION,stageMouseEvent); 
				}
				}else if(starty>stage->stage_h*9/10){
				if(event->button.y<stage->stage_h*8/10)
				{
				//Sidebtns_show();
				//Stage_redraw();
				//printf("\r\nfrom bottom\r\n"); fflush(stdout);
				Sprite_removeEventListener(stage->sprite,SDL_MOUSEMOTION,stageMouseEvent); 
				}
				*/
	}else if(stage->stage_w*5/10 < startx && startx<stage->stage_w-w){
		if(event->button.x>stage->stage_w-w/2)
		{
			Sidebtns_hide();
			//printf("\r\n to right"); fflush(stdout);
			Stage_redraw();
			Sprite_removeEventListener(stage->sprite,SDL_MOUSEMOTION,stageMouseEvent); 
		}
	}
	break;
	}

}


void showSearchDict(int b)
{
	if(history_db==NULL)
		init_db();

	if(b){
		STATS = DICT;
		Test_end();
		Sprite_addEventListener(stage->sprite,SDL_MOUSEBUTTONDOWN,stageMouseEvent); 
		SDL_SetWindowTitle(stage->window, "词典");
		uiThread(NULL);
	}
	if(dictContainer)
		dictContainer->visible = b;

	//Sprite_roundRect2D(0,0,100,100,30,30);
}


#ifdef test_searchdict
int main()
{
	Stage_init();
	if(stage==NULL)return 0;
	showSearchDict(1);


	Stage_loopEvents();
	return 0;
}
#endif
