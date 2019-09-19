#ifndef update_h__
#define update_h__

#include "loading.h"
#include "bytearray.h"
#include "files.h"
#include "httploader.h"
#include "mystring.h"
#include "sdlfiles.h"
#include "sprite.h"
#include "zip.h"

typedef struct UPDATE
{
	char *data;
	char *version;
	char *path;
	char *modify;
}UPDATE;

UPDATE cur;
UPDATE net;

void Update_clear(UPDATE*update);
UPDATE * Update_decode(UPDATE*update,char *data);
void Update_init();
void * update(void *ptr);

int loadAndunzip(char * url,char * toDir);
#endif
