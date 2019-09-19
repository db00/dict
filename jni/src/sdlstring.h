#ifndef __sdlstring_h__
#define __sdlstring_h__

#include "mystring.h"
#include <SDL.h>
#include <SDL_revision.h>

char*getCompiledVersionString();
char*getLinkedVersionString();
const char*getPlatformString();//Windows, Mac OS X, Linux, iOS, Android 
void printfStatus();
int setClipboardText(char *s);
char * getClipboardText(int showAlert);

#endif
