#ifndef music_h
#define music_h
#include "SDL.h"
#include "SDL_platform.h"
#include "SDL_mixer.h"
#include "httploader.h"
#include "files.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
typedef struct Sound
{
	unsigned short audio_format;
	int initted;
	int volume;
	int audio_rate;
	int audio_channels;
	Mix_Music * music;
	int position;
}Sound;
Sound * Sound_new(int audio_rate);
void Sound_free(Sound *sound);
int Sound_playFile(Sound*sound,char * file);
int Sound_playUrl(Sound*sound,char * url);
int Sound_playData(Sound*sound,char * data,size_t data_length);
#endif

