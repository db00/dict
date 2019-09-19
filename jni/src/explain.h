#ifndef explain__h
#define explain__h

#include "pictures.h"
#include "dict.h"
#include "searhdict.h"
#include "sdlstring.h"
#include "wordinput.h"
#include "myttf.h"

void Explain_hide();
Sprite * Explain_show(Dict * dict,Word * word);

#endif
