#ifndef _sdlfiles_h
#define _sdlfiles_h
#include "SDL.h"
#include "files.h"

long sdlfileSize(FILE*stream);
char* sdlreadfile(char * path,size_t * fileSize);
int sdlwritefile(char * path,char *data,size_t data_length);
char *sdlbasepath();
char *sdlprefpath();
#endif
