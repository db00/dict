
#ifndef _urlcode__h
#define _urlcode__h
#include <ctype.h>
#include <stdlib.h>
int url_decode(char *str, int len);//不需free
char *url_encode(char const *s, int len, int *new_length,int not_encode_url_split);//需要free
#endif
