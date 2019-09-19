/**
 * @file tween.h
 gcc -Wall -I"../SDL2/include/" array.c ease.c mystring.c tween.c matrix.c sprite.c -lSDL2 -Ddebug_tween -lm && ./a.out
 gcc -Wall -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/"  tween.c sprite.c -lSDL2_image -lSDL2_ttf -lmingw32 -lSDL2_test -lSDL2main -lSDL2 -Ddebug_tween && a
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-07-21
 */
#ifndef tween_h
#define tween_h

#include "sprite.h"
#include "ease.h"
typedef struct SpriteStatus{
	int x;
	int y;
	int z;

	int w;
	int h;

	float alpha;

	float scaleX;
	float scaleY;
	float scaleZ;

	int rotationX;
	int rotationY;
	int rotationZ;
}SpriteStatus;

typedef struct TweenObj {
	SpriteStatus*start;
	SpriteStatus*end;
	SpriteStatus*cur;
}TweenObj;

typedef struct Tween{
	Sprite * sprite;
	TweenObj * obj;
	Uint32 time;//总时间
	Uint32 passedTime;//从开始播放过的时间 SDL_GetTicks();
	int loop;//循环播放
	SDL_TimerID timer;
	Uint32 interval;
	SDL_TimerCallback callback;
	void * param;
	float (*ease)(float,float,float,float);

	Array * surfaces;//序列帧画面数组

	void (*onEachMove)(void *);
	void * onEachMoveParas;

	void (*onComplete)(void *);
	void * onCompleteParas;
}Tween;

TweenObj * TweenObj_new(Sprite*sprite);
Tween * tween_to(Sprite*sprite,int time,TweenObj*obj);
Tween * Tween_new(Sprite*sprite,int time,TweenObj*obj);
void Tween_kill(void*tween,int toEnd);
SpriteStatus*storeStatus(Sprite*sprite,SpriteStatus*status);
int setSpriteStatus(Sprite*sprite,SpriteStatus*status);

#endif
