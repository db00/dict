/**
 * @file ipstring.h
 gcc -g -Wall -I"." ipstring.c array.c myregex.c regex.c filetypes.c dict.c files.c mystring.c urlcode.c base64.c -lpthread -lws2_32 -lm -D STDC_HEADERS -D DEBUG -D testip && a
 gcc -Wall -g -D testip array.c mystring.c ipstring.c && ./a.out
 gcc -Wall -I"../SDL2_image/" -I"../SDL2_ttf" -I"include" -L"lib" mysurface.c textfield.c files.c array.c matrix.c tween.c ease.c base64.c ipstring.c sprite.c httploader.c mystring.c lib/libeay32.dll.a lib/libssl32.dll.a -lgdi32 -lws2_32 -lwsock32 -lssl -lssl32 -lcrypto -lSDL2_image -lSDL2_ttf -lmingw32 -lSDL2main -lSDL2 -I"../SDL2/include/" -lm -D debug_httploader && a
 */
#ifndef __ipstring_h
#define __ipstring_h

#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif
//#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef WIN32
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include "mystring.h"
int isIpString(char*host);
char * domain2ipString(char * hostname);
#endif
