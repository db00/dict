#ifndef pinyin_h
#define pinyin_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "utf8.h"
#include "files.h"
#include "myregex.h"
#include "httploader.h"
#include "readbaidu.h"
#include "music.h"
#include "sprite.h"

int playHzsPinyin(char * s);
int playEasyPinyin(char*pinyin);
char * num2hzs(int num);
Array * hzs2pinyin(char*s);
//int playYinbiao(char*pinyin);

#endif
