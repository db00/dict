
#ifndef _httpserver_h__
#define _httpserver_h__

#ifndef linux
#ifndef WIN32
#define linux
#endif
#endif
#ifdef WIN32
#include <ws2tcpip.h>
//#include <winsock2.h>
#include <windows.h>
//extern unsigned int getpid();
extern size_t getpagesize(void);
# define MSG_WAITALL (1 << 3)
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>
#endif

#include <pthread.h>  
#include <math.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include "array.h"
#include "myregex.h"
#include "mystring.h"
#include "base64.h"
#include "urlcode.h"
#include "filetypes.h"


typedef struct Parameter
{
	char *name;
	char *value;
	struct Parameter*next;
}Parameter;


typedef struct Server{
	int sock_fd;
	unsigned int port;
	char *root;//根目录
	char * defaulPage;// 如： /index.html
	int numClient;
}Server;

typedef struct Client{
	Server * server;
	int fd;//sock fd
	int type;//0:GET 1:POST

	int recved_len;//recieved data Length

	char *auth_usr;
	char *auth_pswd;

	char *head;//head area point
	char *data;//data area point

	char * get_param_str;

	char *host;//host area point
	char *path;//path area point
	char *real_path;//real abs path on server

	char *boundary_str;

	socklen_t addrlen;
	struct sockaddr_in* addr;

	unsigned int content_length;//data area Length
	Parameter *parameters;//parameters
	Parameter *lastParameter;//parameters
	int num_paras;//number of parameters
}Client;

Server * Server_new(char * root,int port);
int Server_recv(Server*server);
void Server_clear(Server*server);

#endif
