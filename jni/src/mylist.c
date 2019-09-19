#include "mylist.h"

static Tween * tween;

Sprite * List_AddItem(Sprite * curlistSprite,Sprite * sprite,int end)
{
	if(curlistSprite->children && curlistSprite->children->length>0)
	{
		Sprite * lastSprite = NULL;
		if(end){
			lastSprite = Sprite_getChildByIndex(curlistSprite,curlistSprite->children->length-1);
		}else{
			lastSprite = Sprite_getChildByIndex(curlistSprite,0);
		}
		if(lastSprite)
		{
			if(end){
				sprite->y = lastSprite->y + lastSprite->h + 1;
			}else{
				sprite->y = lastSprite->y - sprite->h - 1;
			}
		}
	}
	if(end){
		Sprite_addChild(curlistSprite,sprite);
	}else{
		Sprite_addChildAt(curlistSprite,sprite,0);
	}
	return sprite;
}


Sprite * List_removeItemByIndex(Sprite * curlistSprite,int i)
{
	int j = curlistSprite->children->length-1;
	if(i<0 || i>j)
		return NULL;
	Sprite * cur = NULL;

	while(j>i)
	{
		if(cur==NULL)
			cur = Sprite_getChildByIndex(curlistSprite,j);
		Sprite * pre = Sprite_getChildByIndex(curlistSprite,j-1);
		//SDL_Log("(%d,%d)(%d,%d)\r\n",cur->x,cur->y,pre->x,pre->y);
		cur->y = pre->y;
		cur = pre;
		--j;
	}

	Sprite * sprite = Sprite_removeChildAt(curlistSprite,i);
	Stage_redraw();
	return sprite;
}

Sprite * List_removeItem(Sprite * curlistSprite,Sprite * sprite)
{
	int i = Sprite_getChildIndex(curlistSprite,sprite);
	return List_removeItemByIndex(curlistSprite,i);
}

void List_showOptical(Sprite * curlistSprite)
{
	if(curlistSprite->children && curlistSprite->children->length>0)
	{
		int i = 0;
		while(i<curlistSprite->children->length)
		{
			Sprite * lastSprite = Sprite_getChildByIndex(curlistSprite,i);
			if(lastSprite->y + lastSprite->h + curlistSprite->y<0)
			{
				lastSprite->visible = SDL_FALSE;
			}else if(lastSprite->y + curlistSprite->y > stage->stage_h){
				lastSprite->visible = SDL_FALSE;
			}else{
				lastSprite->visible = SDL_TRUE;
			}
			++i;
		}
	}
	Stage_redraw();
}


void List_rollback(void * list)
{
	tween = NULL;
	Sprite * curlistSprite = list;
	//Tween * tween = NULL;
	TweenObj * tweenObj = NULL;
	int h = curlistSprite->h;
	if(h<=0)
	{
		if(curlistSprite->children)
		{
			int i = curlistSprite->children->length;
			Sprite * lastSprite = Sprite_getChildByIndex(curlistSprite,i-1);
			if(lastSprite)
				h = lastSprite->y + lastSprite->h;
		}
	}
	if(h<=0)
		return;


	tween = NULL;
	//Point3d * p = Sprite_localToGlobal(curlistSprite,NULL);
	if(curlistSprite->y > 0*stage->stage_h){
		tweenObj = (TweenObj*)TweenObj_new(curlistSprite);
		tweenObj->end->y = 0;//stage->stage_h/3;// (container->y + container->h);
	}else if( curlistSprite->y + h < stage->stage_h && h>stage->stage_h){
		tweenObj = (TweenObj*)TweenObj_new(curlistSprite);
		int y = 0;
		Sprite * parent = curlistSprite->parent;
		while(parent && parent!=stage->sprite)
		{
			y += parent->y;
			parent = parent->parent;
		}
		tweenObj->end->y = stage->stage_h - h - y;
	}
	if(tweenObj)
	{
		tween = tween_to(curlistSprite,abs(tweenObj->end->y-tweenObj->start->y)*5,tweenObj);
		tween->ease = easeOut_bounce;
	}
}

static int stageX;
static int stageY;
static int timestamp;
static void mouseMoves(SpriteEvent*e)
{
	Sprite * target = e->target;
	if(target==NULL)
		return;
	if(Sprite_getVisible(target)==0)
		return;

	int curY;
	int deltaY;
	float deltaTime;
	float time;
	float speed;
	SDL_Event* event = e->e;
	switch(event->type)
	{
		case SDL_MOUSEMOTION:
			if(event->motion.state){
				//if(abs(event->motion.xrel)<20 && abs(event->motion.yrel)<20)
				target->x += event->motion.xrel;
				target->y += event->motion.yrel;
				Sprite_limitPosion(target,target->dragRect);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			curY = event->button.y;
			deltaY = curY - stageY;
			deltaTime = event->button.timestamp - timestamp;
			speed = deltaY*1000.0/deltaTime;
			time = abs(speed * 1.0);
			//SDL_Log("up : (%d X %f) , %f\r\n",deltaY,deltaTime,speed);
			if(abs(deltaY)>5)
			{
				TweenObj * tweenObj;
				tweenObj = (TweenObj*)TweenObj_new(target);
				tweenObj->end->y = target->y + time*speed/1000;// (container->y + container->h);
				//SDL_Log("target->h = %d\r\n",target->Bounds->h);
				if(tweenObj->end->y>0 || target->Bounds->h<stage->stage_h)
				{
					if(tweenObj->end->y > tweenObj->start->y)//down
						tweenObj->end->y = stage->stage_h/2;
					else
						tweenObj->end->y = -stage->stage_h/2;
				}else if(tweenObj->end->y + target->Bounds->h < stage->stage_h){
					if(tweenObj->end->y > tweenObj->start->y)//down
						tweenObj->end->y = tweenObj->start->y;
					else
						tweenObj->end->y = stage->stage_h*.5 - target->Bounds->h;
				}
				SDL_Log("speed:%f",speed);
				float maxspeed = 3000.0;
				float minispeed = 50.0;
				if(abs(speed)>maxspeed)
					speed = maxspeed;
				else if(abs(speed)<minispeed)
					speed = minispeed;
				time = max(abs((tweenObj->end->y-tweenObj->start->y)/speed*1000),100);
				tween = tween_to(target,(int)time,tweenObj);
				tween->ease = easeOut_strong;
				/*
				tween->onComplete = List_rollback;
				tween->onCompleteParas = target;
				*/
				tween->onEachMove = target->other;
				tween->onEachMoveParas = NULL;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			stageX = event->button.x;
			stageY = event->button.y;
			timestamp = event->button.timestamp;
			if(tween && tween->sprite){
				TweenObj * tweenObj;
				tweenObj = (TweenObj*)TweenObj_new(target);
				tween = tween_to(target,10,tweenObj);
				//Tween_kill(tween->sprite,0);
			}
			//SDL_Log("down : (%d,%d),%d\r\n",stageX,stageY,timestamp);
			break;
		default:
			return;
	}
	Stage_redraw();

}
void List_roll(Sprite * curlistSprite,void (*onEachMove)(void*) )
{
	if(curlistSprite->dragRect==NULL)
	{
		SDL_Rect * rect = malloc(sizeof(*rect));
		rect->x = curlistSprite->x;
		rect->w = 0;
		rect->y = -(((unsigned int)-1)/4);
		rect->h = (((unsigned int)-1)/2);
		curlistSprite->dragRect = rect;
	}
	curlistSprite->other = onEachMove;
	Sprite_addEventListener(curlistSprite,SDL_MOUSEMOTION,mouseMoves);
	Sprite_addEventListener(curlistSprite,SDL_MOUSEBUTTONUP,mouseMoves);
	Sprite_addEventListener(curlistSprite,SDL_MOUSEBUTTONDOWN,mouseMoves);
}
