#ifndef array_h
#define array_h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Array{
	int length;
	void **data;
}Array;
Array * Array_new();
void Array_clear(Array*array);
void Array_freeEach(Array * array);
Array * Array_resize(Array* array,int i);
Array * Array_push(Array*array,void*data);
void * Array_getByIndex(Array*array,int i);
Array * Array_getByIndexs(Array*array,int start,int end);
Array * Array_concat(Array*array,Array * array2);
Array * Array_setByIndex(Array * array,int i,void * data);
Array * Array_insert(Array * array,int i,void * data);
Array * Array_removeByIndex(Array*array,int i);
int Array_getIndexByValue(Array *array,void * value);
int Array_getIndexByStringValue(Array *array,void * value);
Array * Array_removeByValue(Array *array,void * value);
void Array_prints(Array * array);
void Array_print(Array * array);
char * Array_joins(Array *array,char * split);
#endif
