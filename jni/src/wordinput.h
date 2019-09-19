#ifndef wordinput_h
#define wordinput_h

#include "input.h"
#include "datas.h"
#include "dict.h"
#include "searhdict.h"
#include "explain.h"
#include "update.h"
#include "sidebtns.h"
#include "btnlist.h"
#include "mylist.h"

Dict * ec_dict;
Dict * ce_dict;

int isCE(char * value);

Input * input;

int Wordinput_getHeight();
void Wordinput_show();
void Wordinput_changeWordList();

void Wordinput_hideList();

Sprite * makeWordBtn(Word * word,void (*selectedEvent)(SpriteEvent *));
void List_removeOuts(Sprite * curlistSprite);
void Wordinput_setText(char * s);
int Wordinput_searchWord(char * word);
#endif
