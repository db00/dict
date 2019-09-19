/**
 *
 gcc -Wall -g urlcode.c array.c base64.c ipstring.c httploader.c mystring.c -lpthread -lssl -lcrypto -lm -D debug_httploader &&./a.out
 */
#ifndef _httploader_h
#define _httploader_h

/* Include normal system headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>

#ifndef _WIN32_WCE
#include <errno.h>
#endif

/* Include system network headers */
#if defined(__WIN32__) || defined(WIN32)
#include <ws2tcpip.h>
#define __USE_W32_SOCKETS
#ifdef _WIN64
#include <winsock2.h>
//#include <ws2tcpip.h>
#else
#include <winsock.h>
/* NOTE: windows socklen_t is signed
 * and is defined only for winsock2. */
typedef int socklen_t;
#endif /* W64 */
#include <iphlpapi.h>
#else /* UNIX */
#include <sys/types.h>
#ifdef __FreeBSD__
#include <sys/socket.h>
#endif
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#ifndef __BEOS__
#include <arpa/inet.h>
#endif
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#endif /* WIN32 */

#ifndef __USE_W32_SOCKETS
#ifdef __OS2__
#define closesocket soclose
#else  /* !__OS2__ */
#define closesocket close
#endif /* __OS2__ */
#define SOCKET  int
#define INVALID_SOCKET  -1
#define SOCKET_ERROR    -1
#endif /* __USE_W32_SOCKETS */


#include "base64.h"
#include "urlcode.h"
#include "ipstring.h"
#include "mystring.h"

//#include "SDL_platform.h"
//https://github.com/guardianproject/openssl-android.git
#if !defined(__IPHONEOS__) && !defined(__MACOS__)
//https://github.com/x2on/OpenSSL-for-iPhone
//openssl is not supported by apple from mac os 10.7
// or Use  -Wno-error=deprecated-declarations while compile
#include "openssl/crypto.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/rand.h"
#else
#include "openssl/crypto.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/rand.h"
#endif

#ifndef INADDR_ANY
#define INADDR_ANY      0x00000000
#endif
#ifndef INADDR_NONE
#define INADDR_NONE     0xFFFFFFFF
#endif
#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK     0x7f000001
#endif
#ifndef INADDR_BROADCAST
#define INADDR_BROADCAST    0xFFFFFFFF
#endif


typedef struct RequestData
{
	unsigned int statusCode;
	char * contentType;
	char * server;
	char * date;
	char * Authorization;

	unsigned int contentLength;
	char * data;
}RequestData;

typedef struct URLRequest
{
	char * url;
	char * ip;
	char * host;
	unsigned int statusCode;
	unsigned int port;
	int isHttps;
	char * path;

	void(* onComplete)(struct URLRequest *);// async load complete function
	char * data;//downloaded data
	int bytesOfData;//downloaded data Length;
	RequestData * request;
	RequestData * respond;
}URLRequest;

int isIpString(char*host);
char * domain2ipString(char * hostname);
RequestData *RequestData_new(char* _s);
void RequestData_parseLine(RequestData*header,char * line);
void RequestData_clear(RequestData *header);
void URLRequest_clear(URLRequest* urlrequest);

URLRequest * URLRequest_new(char *_url);
URLRequest * Httploader_request(URLRequest *urlrequest);
URLRequest * Httploader_load(char *url);
URLRequest * Httploader_asyncload(char * url,void (*load_func)(URLRequest *));
URLRequest * URLRequest_setAuthorization(URLRequest*urlrequest,char*userName,char*pswd);
char * loadUrl(char * url,size_t* len);

#endif
