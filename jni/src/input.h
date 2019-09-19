#ifndef input__h
#define input__h

#include "textfield.h"

typedef struct Input
{
    TextField * textfield;
    Sprite *sprite;
	SDL_Rect *rect;
    char * value;
	void (*textChangFunc)(struct Input*);
}Input;
Input * Input_new(int w,int h);
void Input_clear(Input * input);
char * Input_setText(Input * input,char * s);

#endif
