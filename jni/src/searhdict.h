#ifndef searhdict_h
#define searhdict_h

#include "dict.h"
#include "sprite.h"

enum DICT_STATS {
	DICT,
	HISTORY,
	NEW,
	REMEMBERED,
	END
} STATS;

Sprite * dictContainer;

void showSearchDict(int b);

#endif


