/**
 * @file alert.c
 gcc -g -Wall -I"../SDL2/include/" -I"../SDL2_image/" -I"../SDL2_ttf/" alert.c myfont.c mystring.c besier.c mysurface.c files.c ipstring.c base64.c urlcode.c bytearray.c zip.c utf8.c myregex.c update.c httploader.c textfield.c sprite.c matrix.c array.c -lSDL2 -lSDL2_ttf -lSDL2_image -lpthread -lssl -lcrypto -lz -ldl -lm -D debug_alert && ./a.out
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2017-06-09
 */

#include "alert.h"

static Sprite * container;
static SDL_TimerID timer;
static Array * middleBtns;
static Array * bottomBtns;
static Sprite * title;
static Sprite * bg;
static Sprite * middleList;
static Sprite * bottomList;


void Alert_hide()
{
	if(container){
		container->visible = SDL_FALSE;
		Stage_redraw();
	}
}

AlertItem * AlertItem_new(char * s, void * (*func)(void *))
{
	AlertItem * item = malloc(sizeof(AlertItem));
	memset(item,0,sizeof(AlertItem));
	item->str = s;
	item->func = func;
	return item;
}

Array * AlertItem_push(Array * arr,AlertItem * item)
{
	if(arr==NULL)
		arr = Array_new();
	Array_push(arr,item);
	return arr;
}



static int stageMouseY;
static int stageMouseX;
static void selectedEvents(SpriteEvent * e)
{
	SDL_Log("----------->selectedEvents\r\n");
	fflush(stdout);
	if(e==NULL)
		return;
	Sprite*sprite = e->target;
	SDL_Event* event = e->e;
	AlertItem * obj = sprite->obj;
	if(obj ==NULL)
		return;
	switch(e->type)
	{
		case SDL_MOUSEBUTTONDOWN:
			stageMouseY = event->button.y;
			stageMouseX = event->button.x;
			SDL_Log("down:(%d,%d)\r\n",stageMouseX,stageMouseY);
			break;
		case SDL_MOUSEBUTTONUP:
			if(abs(stageMouseY - event->button.y)>20)
				return;
			if(abs(stageMouseX - event->button.x)>20)
				return;
			obj->func(obj);
			Alert_hide();
			break;
	}
}

static int middleList_H=0;
Sprite * AlertItem_newMiddlItem(AlertItem * item)
{
	Sprite * sprite = Sprite_new();
	if(middleList==NULL)
		middleList = Sprite_new();
	Sprite * txt = Sprite_newText(item->str,36*stage->stage_h/800,0x0,0x00ffffff);

	sprite->y = middleList_H;
	Sprite_addChild(sprite,txt);

	//middleList_H += sprite->h+1;
	middleList_H += txt->h+1;
	Sprite_addChild(middleList,sprite);
	sprite->mouseChildren = SDL_FALSE;

	if(item->func){
		//sprite->w = txt->w;
		//sprite->h = txt->h;
		SDL_Log("has callbackFunc\r\n");
		Sprite_addEventListener(sprite,SDL_MOUSEBUTTONDOWN,selectedEvents);
		Sprite_addEventListener(sprite,SDL_MOUSEBUTTONUP,selectedEvents);
		sprite->obj = item;
	}
	return sprite;
}
static int BottomList_W=0;
Sprite * AlertItem_newBottomItem(AlertItem * item)
{
	//Sprite * sprite = Sprite_new();
	if(bottomList==NULL)
		bottomList = Sprite_new();
	Sprite * sprite = Sprite_newText(item->str,32*stage->stage_h/800,0xff000000,0xffffff33);
	//Sprite_addChild(sprite,txt);
	sprite->x = BottomList_W;
	//BottomList_W += txt->w + 1;
	BottomList_W += sprite->w + 1;
	Sprite_addChild(bottomList,sprite);
	sprite->mouseChildren = SDL_FALSE;
	if(item->func){
		sprite->obj = item;
		Sprite_addEventListener(sprite,SDL_MOUSEBUTTONDOWN,selectedEvents);
		Sprite_addEventListener(sprite,SDL_MOUSEBUTTONUP,selectedEvents);
	}
	return sprite;
}

static Uint32 timercallback(Uint32 interval, void *param)
{
	Alert_hide();
	if(timer){
		SDL_RemoveTimer(timer);
		timer = 0;
	}
	return 0;
}


void Alert_show(char * _title,Array * _middleBtns,Array * _bottomBtns,int time)
{
	if(container==NULL)
	{
		container = Sprite_new();
	}
	container->visible = SDL_TRUE;
	middleBtns = _middleBtns;
	bottomBtns = _bottomBtns;
	if(title){
		Sprite_destroy(title);
		title = NULL;
	}

	title = Sprite_newText(_title,28*stage->stage_h/800,0xff000000,0xffffff33);
	Sprite_addChild(container,title);

	if(middleList ==NULL)
	{
		middleList = Sprite_new();
	}
	Sprite_removeChildren(middleList);
	middleList_H = 0;
	if(_middleBtns && _middleBtns->length>0)
	{
		int i = 0;
		while(i<_middleBtns->length)
		{
			AlertItem_newMiddlItem(Array_getByIndex(_middleBtns,i));
			++i;
		}
	}
	middleList->y = title->y + title->h;
	Sprite_addChild(container,middleList);
	if(bottomList==NULL)
	{
		bottomList = Sprite_new();
	}
	Sprite_removeChildren(bottomList);
	BottomList_W = 0;
	if(_bottomBtns && _bottomBtns->length>0)
	{
		int i = 0;
		while(i<_bottomBtns->length)
		{
			AlertItem_newBottomItem(Array_getByIndex(_bottomBtns,i));
			++i;
		}
	}
	bottomList->y = middleList->y + middleList_H;
	Sprite_addChild(container,bottomList);

	Sprite_addChild(stage->sprite,container);
	int w = BottomList_W;
	int h = middleList_H + 80;
	int gap = 10;
	if(bg==NULL)
	{
		bg = Sprite_roundRect2D(0,0,w*2+gap*2,h+gap*2,30,30);
		Sprite_addChildAt(container,bg,0);
	}

	container->x = (stage->stage_w - (w*2+gap*2))/2;
	container->y = (stage->stage_h - (h+gap*2))/2;

	if(time){
		timer= SDL_AddTimer(time, timercallback, NULL);
	}
	Stage_redraw();
}

#ifdef debug_alert
void * callbackFunc(void * v)
{
	AlertItem * item = v;
	SDL_Log("%s",item->str);
	return NULL;
}
int main()
{
	Stage_init();
	Array * middleBtns = AlertItem_push(NULL,AlertItem_new("hello1",callbackFunc));
	AlertItem_push(middleBtns,AlertItem_new("hello2",callbackFunc));
	AlertItem_push(middleBtns,AlertItem_new("hello3",NULL));
	Array * bottomBtns = NULL;
	bottomBtns = AlertItem_push(NULL,AlertItem_new("hello11",callbackFunc));
	//Alert_show("hello",middleBtns,bottomBtns,5000);
	Alert_show("hello",middleBtns,bottomBtns,0);
	Stage_loopEvents();
	return 0;
}
#endif
