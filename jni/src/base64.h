/*
 *
 gcc -Wall h.c base64.c -lm -o ~/a && ~/a
 *
 */
#ifndef __base64__h
#define __base64__h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char * base64_encode(const char* data, int data_len); //需要free
char * base64_decode(const char* data, int data_len,int *ret_len); //需要free

#endif
