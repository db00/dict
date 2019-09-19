/**
 * @file tween.c
 gcc -Wall -I"../SDL2/include/" array.c files.c myregex.c ease.c mystring.c tween.c matrix.c sprite.c -lSDL2 -Ddebug_tween -lm && ./a.out
 gcc -Wall -I"../SDL2/include/" array.c ease.c mystring.c tween.c matrix.c sprite.c -lmingw32 -lSDL2main -lSDL2 -Ddebug_tween -lm && a
 gcc -Wall -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/"  tween.c matrix.c sprite.c -lSDL2_image -lSDL2_ttf -lmingw32 -lSDL2_test -lSDL2main -lSDL2 -Ddebug_tween && a
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-07-21
 */
#include "tween.h"

TweenObj * TweenObj_new(Sprite*sprite)
{
	if(sprite == NULL)
		return NULL;
	Stage_redraw();
	int len = sizeof(TweenObj);
	TweenObj * tweenObj = (TweenObj*)malloc(len);
	memset(tweenObj,0,len);

	tweenObj->start = (SpriteStatus*)storeStatus(sprite,NULL);
	tweenObj->cur = (SpriteStatus*)malloc(sizeof(SpriteStatus));
	tweenObj->end = (SpriteStatus*)malloc(sizeof(SpriteStatus));
	memcpy(tweenObj->cur,tweenObj->start,sizeof(SpriteStatus));
	memcpy(tweenObj->end,tweenObj->start,sizeof(SpriteStatus));
	return tweenObj;
}

void TweenObj_clear(TweenObj * tweenObj)
{
	if(tweenObj){
		if(tweenObj->start){
			free(tweenObj->start);
			tweenObj->start = NULL;
		}
		if(tweenObj->end){
			free(tweenObj->end);
			tweenObj->end=NULL;
		}
		if(tweenObj->cur){
			free(tweenObj->cur);
			tweenObj->cur= NULL;
		}
		free(tweenObj);
	}
}

void Tween_clear(Tween *tween)
{
	if(tween)
	{
		if(tween->timer){
			SDL_bool b = SDL_RemoveTimer(tween->timer);
			if(b){
				SDL_Log("SDL_RemoveTimer success!");fflush(stdout);
			}else{
				return;
			}
			tween->timer = 0;
		}
		if(tween->sprite){
			tween->sprite->tween = NULL;
			tween->sprite->Tween_kill = NULL;
			tween->sprite = NULL;
		}
		if(tween->obj){
			//SDL_Log("TweenObj_clear start!");fflush(stdout);
			TweenObj_clear(tween->obj);
			//SDL_Log("TweenObj_clear success!");fflush(stdout);
			tween->obj = NULL;
		}
		free(tween);
	}
}

SpriteStatus*storeStatus(Sprite*sprite,SpriteStatus*status)
{
	if(status==NULL) {
		status = (SpriteStatus*)malloc(sizeof(SpriteStatus));
	}
	status->x = sprite->x;
	status->y = sprite->y;
	status->z = sprite->z;

	status->w = sprite->w;
	status->h = sprite->h;

	status->alpha = sprite->alpha;

	status->scaleX = sprite->scaleX;
	status->scaleY = sprite->scaleY;
	status->scaleZ = sprite->scaleZ;

	status->rotationX = sprite->rotationX;
	status->rotationY = sprite->rotationY;
	status->rotationZ = sprite->rotationZ;
	return status;
}

int setSpriteStatus(Sprite*sprite,SpriteStatus*status)
{
	if(status==NULL) {
		return -1;
	}
	sprite->x = status->x;
	sprite->y = status->y;
	sprite->z = status->z;

	sprite->w = status->w;
	sprite->h = status->h;

	sprite->alpha = status->alpha;

	sprite->scaleX = status->scaleX;
	sprite->scaleY = status->scaleY;
	sprite->scaleZ = status->scaleZ;

	sprite->rotationX = status->rotationX;
	sprite->rotationY = status->rotationY;
	sprite->rotationZ = status->rotationZ;
	return 0;
}


void Tween_kill(void * tweenobj,int toEnd)
{
	Tween * tween = tweenobj;
	if(tween==NULL)return;
	TweenObj * obj = tween->obj;
	if(toEnd){
		memcpy(obj->cur,obj->end,sizeof(SpriteStatus));//结束值
		setSpriteStatus(tween->sprite,obj->cur);
	}
	Stage_redraw();
	void (*onComplete)(void*) = tween->onComplete;//void (*onComplete)(void *);
	void * onCompleteParas = tween->onCompleteParas;

	Tween_clear(tween);

	if(onComplete){
		onComplete(onCompleteParas);
	}
}


static Uint32 my_callbackfunc(Uint32 interval, void *param)
{
	Uint32 then = SDL_GetTicks();//当前时间;
	Tween * tween = (Tween*)param;
	Sprite * sprite = tween->sprite;

	tween->passedTime += tween->interval;
	int timeLeft = tween->time - tween->passedTime;
	if(timeLeft <=0 && tween->loop) {
		tween->passedTime = 0;
		timeLeft = tween->time - tween->passedTime;
	}
	if(timeLeft <=0) {
		if(tween->surfaces && tween->surfaces->length>0)
			Sprite_setSurface(sprite,Array_getByIndex(tween->surfaces,tween->surfaces->length-1));
		if(sprite->Tween_kill==NULL)
		{
			sprite->Tween_kill = Tween_kill;
		}
		Tween_kill(tween,1);
	}else{
		SpriteStatus*cur = tween->obj->cur;
		SpriteStatus*start= tween->obj->start;
		SpriteStatus*end= tween->obj->end;

		float t = tween->passedTime;
		float b = 0.0;
		float c = 1.0;
		float d = tween->time;

		float completeRate = tween->ease(t,b,c,d);

		if(tween->surfaces && tween->surfaces->length>0){
			int i = (int)((float)(tween->surfaces->length)*completeRate);
			if(i<0)
				i=0;
			if(i>=tween->surfaces->length)
				i = tween->surfaces->length-1;
			Sprite_setSurface(sprite,Array_getByIndex(tween->surfaces,i));
		}

		cur->x = start->x + (end->x - start->x)*completeRate;
		cur->y = start->y + (end->y - start->y)*completeRate;
		cur->z = start->z + (end->z - start->z)*completeRate;

		cur->w = start->w + (end->w - start->w)*completeRate;
		cur->h = start->h + (end->h - start->h)*completeRate;

		cur->alpha = start->alpha + (end->alpha - start->alpha)*completeRate;

		cur->scaleX = start->scaleX + (end->scaleX - start->scaleX)*completeRate;
		cur->scaleY = start->scaleY + (end->scaleY - start->scaleY)*completeRate;
		cur->scaleZ = start->scaleZ + (end->scaleZ - start->scaleZ)*completeRate;

		cur->rotationX = start->rotationX + (end->rotationX - start->rotationX)*completeRate;
		cur->rotationY = start->rotationY + (end->rotationY - start->rotationY)*completeRate;
		cur->rotationZ = start->rotationZ + (end->rotationZ - start->rotationZ)*completeRate;

		setSpriteStatus(sprite,cur);
	}

	Stage_redraw();
	//UserEvent_new(SDL_USEREVENT,0,Stage_redraw,NULL);//this line is equal to the following code block.
	/*
	   SDL_Event event;
	   SDL_UserEvent userevent;
	   userevent.type = SDL_USEREVENT;
	   userevent.code = 0;
	   userevent.data1 = Stage_redraw;
	   userevent.data2 = param;
	   event.type = SDL_USEREVENT;
	   event.user = userevent;
	   SDL_PushEvent(&event);//
	   */
	if(tween->onEachMove)
	{
		tween->onEachMove(tween->onCompleteParas);
	}

	int timeFromThen = (SDL_GetTicks()-then);
	if(timeFromThen>0)
		return(interval-timeFromThen);
	return interval;
}

Tween * tween_to(Sprite * sprite,int time,TweenObj*obj)
{
	if(sprite == NULL || obj == NULL || time<=0)
		return NULL;
	if(sprite->Tween_kill && sprite->tween){
		sprite->Tween_kill(sprite->tween,0);
	}

	Tween * tween = (Tween*)malloc(sizeof(Tween));
	memset(tween,0,sizeof(Tween));
	tween->time = time;

	sprite->Tween_kill = Tween_kill;
	sprite->tween = tween;

	if(obj->start==NULL){
		obj->start = (SpriteStatus*)storeStatus(sprite,obj->start);
	}

	if(obj->cur == NULL){
		obj->cur= (SpriteStatus*)malloc(sizeof(SpriteStatus));
		memcpy(obj->cur,obj->start,sizeof(SpriteStatus));//当前值
	}

	if(obj->end == NULL){
		obj->end= (SpriteStatus*)malloc(sizeof(SpriteStatus));
		memcpy(obj->end,obj->start,sizeof(SpriteStatus));
	}
	tween->obj = obj;

	if(tween->ease==NULL){
		/**
		  easeNone_linear
		  easeInOut_back
		  easeInOut_bounce
		  easeInOut_circ
		  easeInOut_cubic
		  easeInOut_elastic
		  easeInOut_expo
		  easeInOut_linear
		  easeInOut_quad
		  easeInOut_quart
		  easeInOut_quint
		  easeInOut_sine
		  easeInOut_strong
		  */
		tween->ease = easeNone_linear;
	}
	tween->sprite = sprite;

	//tween->StartTime = SDL_GetTicks();
	tween->callback = my_callbackfunc;
	if(tween->interval<=0)tween->interval = 20;
	tween->param = tween;
	tween->timer= SDL_AddTimer(tween->interval, tween->callback, tween->param);
	return tween;
}
Tween * Tween_new(Sprite*sprite,int time,TweenObj*obj)
{
	return tween_to(sprite,time,obj);
}

#ifdef debug_tween

static Tween * tween;
static void start_move(Sprite * sprite,int b)
{
	if(b){
		TweenObj * tweenObj = (TweenObj*)TweenObj_new(sprite);
		//tweenObj->end->scaleX = 8.0;
		//tweenObj->end->scaleY = 4.0;
		//tweenObj->end->x=240-sprite->w;
		//tweenObj->end->y=320-sprite->h;
		tweenObj->start->alpha=.3;
		tweenObj->end->alpha=1.0;
		//tweenObj->end->rotationX=(double)360.0*4;//度数
		//tweenObj->end->rotationY=(double)360.0*4;//度数
		//tweenObj->start->rotationZ=.0;
		tweenObj->end->rotationZ=tweenObj->start->rotationZ+(double)360.0;//度数
		tween = tween_to(sprite,1000 ,tweenObj);
		tween->ease = easeInOut_bounce;
		tween->loop = 1;
	}else{
		if(tween){
			Tween_kill(tween,0);
			tween = NULL;
		}
	}
}

static void events(SpriteEvent * e)
{
	if(e==NULL)
		return;
	Sprite * sprite = e->target;
	SDL_Event* event = e->e;
	switch(event->type)
	{
		case SDL_MOUSEBUTTONDOWN:
			SDL_Log("down");
			start_move(sprite,0);
			break;
		case SDL_MOUSEBUTTONUP:
			SDL_Log("up");
			start_move(sprite,1);
			break;
	}
}



//#include "files.h"
int main(int argc, char *argv[])
{
	Stage_init();

	Sprite*m1 = Sprite_new(NULL);
	m1->x = stage->stage_w/2;
	m1->y = stage->stage_h/2;

	Sprite*sprite = Sprite_new(NULL);
	sprite->surface = SDL_LoadBMP("1.bmp");
	//sprite->surface = SDL_LoadBMP(decodePath("~/sound/1.bmp"));
	//sprite->x =30;
	//sprite->y =100;
	sprite->h=50.0;
	sprite->w=100.0;
	//sprite->rotationZ =120;
	sprite->alpha =0.4;
	Sprite_addChild(m1,sprite);

	Sprite_addChild(stage->sprite,m1);
	Sprite_addEventListener(sprite,SDL_MOUSEBUTTONDOWN,events);
	Sprite_addEventListener(sprite,SDL_MOUSEBUTTONUP,events);

	start_move(sprite,1);

	Stage_loopEvents();
	return 0;
}
#endif
