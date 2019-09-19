#ifndef filetypes_h
#define filetypes_h
#include <stdio.h>
#include <string.h>

int data2file(const char *filename,const char *data,unsigned int *len);
const char*type2mine(char*type);
char*type_by_head(const char*buffer);
#endif
