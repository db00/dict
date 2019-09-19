/**
 *
 gcc -lpthread udpclient.c ipstring.c -D debug_udpclient && ./a.out
 *
 */


#include "udpclient.h"

typedef struct UdpSender{
	int fd;
	char * sendmsg;
	int sendmsg_len;//space size
	char * respond;
	int respond_len; //server respond number of bytes
	int respond_size;//space size
	struct sockaddr_in* server; /* server's address information */
	struct sockaddr_in* client; /* server's address information */
}UdpSender;


UdpSender * UdpSender_new()
{
	UdpSender * udpsender= (UdpSender*)malloc(sizeof(UdpSender));
	memset(udpsender,0,sizeof(UdpSender));
	return udpsender;
}

void UdpSender_clear(UdpSender*udpsender)
{
	if(udpsender)
	{
		if(udpsender->sendmsg)
			free(udpsender->sendmsg);
		if(udpsender->respond)
			free(udpsender->respond);
		if(udpsender->server)
			free(udpsender->server);
		if(udpsender->client)
			free(udpsender->client);
		if(udpsender->fd)
			close(udpsender->fd); /* close fd */
		free(udpsender);
	}
}

UdpSender * UdpSender_init(UdpSender*udpsender,char*host,unsigned int port)
{
	if(udpsender==NULL){
		udpsender = UdpSender_new();
	}

	struct hostent *he=NULL; /* structure that will get information about remote host */
	char * ip = NULL;

	if(isIpString(host)){
		ip = host;
	}else{
		if ((he=gethostbyname(host))==NULL){ /* calls gethostbyname() */
			printf("gethostbyname() error\n");
			return 0;
		}
	}
	if ((udpsender->fd=socket(AF_INET, SOCK_DGRAM, 0))==-1){ /* calls socket() */
		printf("socket() error\n");
		return 0;
	}

	if(udpsender->server == NULL)
		udpsender->server = malloc(sizeof(struct sockaddr_in));
	bzero(udpsender->server,sizeof(struct sockaddr_in));
	udpsender->server->sin_family = AF_INET;
	udpsender->server->sin_port = htons(port); /* htons() is needed again */
	if(ip){
		udpsender->server->sin_addr.s_addr = inet_addr(ip);
	}else if(he){
		udpsender->server->sin_addr = *((struct in_addr *)he->h_addr); /*he->h_addr passes "*he"'s info to "h_addr" */
	}
	return udpsender;
}

void* UdpSender_recv(void*_udpsender)
{
	UdpSender*udpsender = (UdpSender*)_udpsender;
	socklen_t len = sizeof(struct sockaddr_in);
	//接到信息前阻塞
	if ((udpsender->respond_len = recvfrom(udpsender->fd,udpsender->respond,udpsender->respond_size-1,0,(struct sockaddr *)udpsender->server,&len)) == -1){ /* calls recvfrom() */
		printf("recvfrom() error\n");
		UdpSender_clear(udpsender);
		return NULL;
	}

	udpsender->respond[udpsender->respond_len]='\0';
	printf("Server Message: %s\n",udpsender->respond); /* it prints server's welcome message */

	close(udpsender->fd); /* close fd */
	udpsender->fd = 0;
	//UdpSender_clear(udpsender);
	pthread_exit(NULL);
}

UdpSender *UdpSender_send(UdpSender*udpsender,char*message,int message_len)
{
	if(udpsender==NULL)
		return NULL;

	if(udpsender->sendmsg)
	{
		free(udpsender->sendmsg);
	}
	udpsender->sendmsg_len = message_len;
	udpsender->sendmsg = malloc(message_len+1);
	memset(udpsender->sendmsg,0,message_len+1);
	memcpy(udpsender->sendmsg,message,message_len);


	socklen_t len = sizeof(struct sockaddr_in);
	sendto(udpsender->fd,udpsender->sendmsg,udpsender->sendmsg_len,0,(struct sockaddr *)udpsender->server,len);

	if(udpsender->respond==NULL)
	{
		udpsender->respond_size = 2048 *2;
		udpsender->respond = malloc(udpsender->respond_size);
	}
	memset(udpsender->respond,0,udpsender->respond_size);


	pthread_t thread;//创建不同的子线程以区别不同的客户端  
	if(pthread_create(&thread, NULL, UdpSender_recv, udpsender)!=0)//创建子线程  
	{  
		perror("pthread_create");  
		UdpSender_clear(udpsender);
		return NULL;
	}
	//pthread_detach(thread);
	int wait = 1;//1 秒超时
	while(wait>0)
	{
		sleep(1);
		wait--;
	}
	pthread_cancel(thread);
	pthread_join(thread,NULL);
	return udpsender;
}


#ifdef debug_udpclient
int main(int argc, char *argv[])
{
	unsigned int port = 1234;
	char * message = "test2";
	int message_len = strlen(message);
	char respond[2048];
	int respond_len;
	UdpSender_clear(UdpSender_send(UdpSender_init(UdpSender_new(),"localhost",port),message,message_len));
	return 0;
}
#endif
