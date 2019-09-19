/***
 *
 gcc -D debug_udpserver udpserver.c -o ~/a && ~/a 
 */

#include "udpserver.h"

void UdpSerever_close(UdpSerever*udpServer)
{
	if(udpServer)
	{
		if(udpServer->bound)
		{
			close(udpServer->sockfd); /* close listenfd */
			udpServer->bound = 0;
		}
		free(udpServer);
	}
}
void UdpClient_close(UdpClient*udpClient)
{
	if(udpClient){
		if(udpClient->sendbuf)
		{
			free(udpClient->sendbuf);
		}
		udpClient->sendbuflen = 0;
		if(udpClient->recvbuf)
		{
			free(udpClient->recvbuf);
		}
		udpClient->recvbuflen = 0;
		free(udpClient);
	}
}

int UdpClient_send(UdpClient*udpClient,char*message,int message_len)
{
	socklen_t sin_size=sizeof(struct sockaddr_in);

	UdpSerever * udpServer = udpClient->server;

	udpClient->sendbuflen = 1024;
	udpClient->sendbuf = malloc(udpClient->sendbuflen);
	memset(udpClient->sendbuf,0,udpClient->sendbuflen);
	memcpy(udpClient->sendbuf,message,message_len);
	return sendto(udpServer->sockfd,udpClient->sendbuf,message_len,0,(struct sockaddr *)&(udpClient->client),sin_size);
}

UdpClient * UdpClient_new(UdpSerever*udpServer)
{
	UdpClient * udpClient= (UdpClient*)malloc(sizeof(UdpClient));
	memset(udpClient,0,sizeof(UdpClient));
	socklen_t sin_size=sizeof(struct sockaddr_in);
	udpClient->recvbuflen = 1024;
	udpClient->recvbuf = malloc(udpClient->recvbuflen);
	int num = recvfrom(udpServer->sockfd,udpClient->recvbuf,udpClient->recvbuflen,0,(struct sockaddr *)&(udpClient->client),&sin_size);
	printf("You got a message (%s) from %s\n",udpClient->recvbuf,inet_ntoa(udpClient->client.sin_addr) ); /* prints client's IP */
	if (num < 0){
		perror("recvfrom error\n");
		UdpClient_close(udpClient);
		return NULL;
	}
	udpClient->recvbuf[num] = '\0';
	udpClient->server = udpServer;
	return udpClient;
}

UdpSerever *UdpSerever_new(unsigned int port)
{
	UdpSerever * udpServer = (UdpSerever*)malloc(sizeof(UdpSerever));
	memset(udpServer,0,sizeof(UdpSerever));


	if ((udpServer->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		/* handle exception */
		perror("Creating socket failed.");
		UdpSerever_close(udpServer);
		return NULL;
	}

	struct sockaddr_in server; /* server's address information */
	bzero(&server,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(port);
	server.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind(udpServer->sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
		/* handle exception */
		perror("Bind error.");
		UdpSerever_close(udpServer);
		return NULL;
	}
	udpServer->bound = 1;//has already bound;
	return udpServer;
}

#ifdef debug_udpserver

int main()
{
	UdpSerever * udpServer = UdpSerever_new(1234);
	if(udpServer==NULL)
		return 0;


	while (1) {
		UdpClient * udpClient = UdpClient_new(udpServer);
		if(udpClient){
			UdpClient_send(udpClient,"message",strlen("message"));
			UdpClient_close(udpClient);
		}
	}

	UdpSerever_close(udpServer);
	return 0;
}

#endif
