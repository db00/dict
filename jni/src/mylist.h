#ifndef mylist_h
#define mylist_h
#include "sprite.h"
#include "tween.h"
Sprite * List_AddItem(Sprite * curlistSprite,Sprite * sprite,int end);
Sprite * List_removeItem(Sprite * curlistSprite,Sprite * sprite);
void List_showOptical(Sprite * curlistSprite);
//void List_roll(Sprite * curlistSprite);
void List_roll(Sprite * curlistSprite,void (*onEachMove)(void*) );
#endif
