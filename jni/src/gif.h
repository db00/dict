#ifndef gif_h
#define gif_h

#include "tween.h"


#ifdef _WIN32
#include <io.h>
#endif /* _WIN32 */

#include "gif_lib.h"

Array * Surface_gif(char *data, int * delay);
#endif
