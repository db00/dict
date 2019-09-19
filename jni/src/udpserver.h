#ifndef udpserver__h_
#define udpserver__h_

#include <stdio.h> /* These are the usual header files */
#include <string.h>
#include <unistd.h> /* for close() */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct UdpSerever{
	int sockfd;
	int bound;//
}UdpSerever;

typedef struct UdpClient{
	char * recvbuf;
	int recvbuflen;
	char * sendbuf;
	int sendbuflen;
	UdpSerever *server;
	struct sockaddr_in client; /* client's address information */
}UdpClient;

#endif
