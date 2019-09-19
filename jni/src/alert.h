#ifndef alert_h
#define alert_h

#include "sprite.h"
#include "textfield.h"
#include "mysurface.h"
#include "besier.h"

typedef struct AlertItem{
	char * str;
	void * (*func)(void *);
}AlertItem;

AlertItem * AlertItem_new(char * s, void * (*func)(void *));
Array * AlertItem_push(Array * arr,AlertItem * item);
void Alert_show(char * _title,Array * _middleBtns,Array * _bottomBtns,int time);
void Alert_hide();
#endif
