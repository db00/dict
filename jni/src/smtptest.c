/**
 *
 gcc -D debug_email email.c base64.c ipstring.c && ./a.out
 */
#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <resolv.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
#include "ipstring.h"
#include "base64.h"

// 打开TCP Socket连接
int open_socket(struct sockaddr *addr)
{
	int sockfd = 0;
	sockfd=socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		fprintf(stderr, "Open sockfd(TCP) error!\n");
		exit(-1);
	}
	if(connect(sockfd, addr, sizeof(struct sockaddr)) < 0)
	{
		fprintf(stderr, "Connect sockfd(TCP) error!\n");
		exit(-1);
	}
	return sockfd;
} 

// 发送邮件
void sendemail(char *user,char *pswd,char *email, char *body)
{
	//printf("%s,%s\n",email,body);
	int sockfd = 0;
	struct sockaddr_in their_addr = {0};
	char buf[1500] = {0};
	char rbuf[1500] = {0};
	char *login = NULL;
	char *pass = NULL;
#ifdef WIN32
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
#endif
	memset(&their_addr, 0, sizeof(their_addr));
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(25);
	their_addr.sin_addr.s_addr = inet_addr(domain2ipString("smtp.qq.com"));//qq smtp 服务器
	// 连接邮件服务器，如果连接后没有响应，则2 秒后重新连接
	sockfd = open_socket((struct sockaddr *)&their_addr);
	memset(rbuf,0,1500);
	while(recv(sockfd, rbuf, 1500, 0) == 0)
	{
		printf("reconnect...\n");fflush(stdout);
		sleep(2);
		//close(sockfd);
		sockfd = open_socket((struct sockaddr *)&their_addr);

		memset(rbuf,0,1500);
	}

	printf("%s\n", rbuf);

	// EHLO
	memset(buf, 0, 1500);
	sprintf(buf, "EHLO abcdefg-PC\r\n");
	send(sockfd, buf, strlen(buf), 0);
	memset(rbuf, 0, 1500);
	recv(sockfd, rbuf, 1500, 0);
	printf("%s\n", rbuf);

	// AUTH LOGIN
	memset(buf, 0, 1500);
	sprintf(buf, "AUTH LOGIN\r\n");
	send(sockfd, buf, strlen(buf), 0);
	printf("%s\n", buf);
	memset(rbuf, 0, 1500);
	recv(sockfd, rbuf, 1500, 0);
	printf("%s\n", rbuf);

	// USER
	memset(buf, 0, 1500);
	sprintf(buf,user);//你的qq号
	login = base64_encode(buf, strlen(buf));
	sprintf(buf, "%s\r\n", login);
	free(login);
	send(sockfd, buf, strlen(buf), 0);
	printf("%s\n", buf);
	memset(rbuf, 0, 1500);
	recv(sockfd, rbuf, 1500, 0);
	printf("%s\n", rbuf);

	// PASSWORD
	sprintf(buf, pswd);//你的qq密码
	pass = base64_encode(buf, strlen(buf));
	sprintf(buf, "%s\r\n", pass);
	free(pass);
	send(sockfd, buf, strlen(buf), 0);
	printf("%s\n", buf);

	memset(rbuf, 0, 1500);
	recv(sockfd, rbuf, 1500, 0);
	printf("%s\n", rbuf);

	// MAIL FROM
	memset(buf, 0, 1500);
	sprintf(buf, "MAIL FROM: <db0@qq.com>\r\n");
	send(sockfd, buf, strlen(buf), 0);
	memset(rbuf, 0, 1500);
	recv(sockfd, rbuf, 1500, 0);
	printf("%s\n", rbuf);

	// RCPT TO 第一个收件人
	sprintf(buf, "RCPT TO:<%s>\r\n", email);
	send(sockfd, buf, strlen(buf), 0);
	memset(rbuf, 0, 1500);
	recv(sockfd, rbuf, 1500, 0);
	printf("%s\n", rbuf);

	// DATA 准备开始发送邮件内容
	sprintf(buf, "DATA\r\n");
	send(sockfd, buf, strlen(buf), 0);
	memset(rbuf, 0, 1500);
	recv(sockfd, rbuf, 1500, 0);
	printf("%s\n", rbuf);

	// 发送邮件内容，\r\n.\r\n内容结束标记
	sprintf(buf, "%s\r\n.\r\n", body);
	send(sockfd, buf, strlen(buf), 0);
	memset(rbuf, 0, 1500);
	recv(sockfd, rbuf, 1500, 0);
	printf("%s\n", rbuf);

	// QUIT
	sprintf(buf, "QUIT\r\n");
	send(sockfd, buf, strlen(buf), 0);
	memset(rbuf, 0, 1500);
	recv(sockfd, rbuf, 1500, 0);
	printf("%s\n", rbuf);

	// VC2005 需要使用
#ifdef WIN32
	closesocket(sockfd);
#else
	close(sockfd);
#endif
#ifdef WIN32
	WSACleanup();
#endif
	return;

}

#ifdef debug_email
int main()
{
	char email[] = "db0@qq.com";
	char body[] = "From: \"lucy\"<db0@qq.com>\r\n"
		"To: \"dasiy\"<158386185@qq.com>\r\n"
		"Subject: Hello\r\n\r\n"
		"Hello World, Hello Email!";
	sendemail("158386185","**********",email, body);
	return 0;
}
#endif
