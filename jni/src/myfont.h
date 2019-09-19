#ifndef myfont__h
#define myfont__h

#include "SDL_platform.h"
#include "SDL_ttf.h"
#include "files.h"
#include "mystring.h"
#include "sprite.h"
#include "utf8.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>



TTF_Font * getFontByName(const char * fontName,int fontSize);
TTF_Font * getFontByPath(char * path,int fontSize);
TTF_Font * getDefaultFont(int fontSize);
TTF_Font * getFontByContainString(char * s,int fontSize);//返回能正确渲染文字的字体


#endif
