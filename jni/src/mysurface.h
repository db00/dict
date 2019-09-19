#ifndef mysurface_h
#define mysurface_h

#include "SDL.h"
#include "SDL_image.h"
#include "httploader.h"
#include "sprite.h"
#include "textfield.h"
//#include "pinyin.h"
#include "pthread.h"
//#include "files.h"
#ifdef __ANDROID__
#include <jni.h>
#endif
//SDL_Surface * render2surface(SDL_Renderer*renderer);

Sprite * Sprite_newImg(char *url);
Sprite * Sprite_newText(char *s,int fontSize,Uint32 fontColor,Uint32 bgColor);
SDL_Surface * Httploader_loadimg(char * url);
void Sprite_alertText(char * s);//显示弹窗
void vibrate();//
#endif
