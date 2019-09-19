#ifndef readbaidu_h__
#define readbaidu_h__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef unix
#include <unistd.h>
#endif

#include "SDL.h"
#include "SDL_mixer.h"
#include "string.h"
#include "urlcode.h"
#include "httploader.h"
#include "files.h"
#include "music.h"
#include "sprite.h"

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
void READ_loadSound(char *word,int type);
void Sound_playEng(char * s,int type);
#endif
