/**
 *
 gcc -lssl -lcrypto pop3test.c && ./a.out
 *
 */
#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

int main()
{
	/* Initializing OpenSSL */
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_all_algorithms();
	SSL_library_init();
	int len;
	BIO *bio;
	char *buf = "user db0\n";
	char *buf1 = "pass biaoli512\n";
	char *buf3 ="retr 1\n";
	//char *buf3 ="list\n";
	//char *buf4 ="uidl 1\n";
	len = strlen(buf);
	char buf2[1024];
	char buf11[3*1024];
	memset(buf11,0,sizeof(buf11));
	SSL_CTX  *contex=SSL_CTX_new(TLSv1_client_method());    //新建ssl认证方法
	SSL *ssl;
	if(!SSL_CTX_load_verify_locations(contex,"/etc/pki/tls/certs/ca-bundle.crt",NULL))  //加载所有证书进行验证
	{
		/* Handle failed load here */
		printf("Error: %s\n",ERR_reason_error_string(ERR_get_error()));   //加载所有证书是否正常
	}

	bio = BIO_new_ssl_connect(contex);     //设置BIO 对像
	BIO_get_ssl(bio, &ssl);
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
	/* Attempt to connect */
	//设置连接
	BIO_set_conn_hostname(bio,"183.60.15.163:995");

	/* Verify the connection opened and perform the handshake */
	//验证连接是否打开，而且执行握手情况
	if(BIO_do_connect(bio) <= 0)   //连接失败
	{
		/* Handle failed connection */
		printf("Error: %s\n",ERR_reason_error_string(ERR_get_error()));   //加载所有证书是否正常
		return 0;
	}
	//检查证书是否有效
	if(SSL_get_verify_result(ssl) != X509_V_OK)
	{
		/* Handle the failed verification */
		printf("Error: %s\n",ERR_reason_error_string(ERR_get_error()));   //加载所有证书是否正常
		return 0;
	}
	//写入到连接
	if(BIO_write(bio, buf, len) <= 0)
	{
		if(! BIO_should_retry(bio))
		{
			/* Handle failed write here */
			printf("Handle failed write here");
		}

		/* Do something to handle the retry */
	}
	//从连接读取
	int x = BIO_read(bio, buf2, 1024);
	if(x == 0)
	{
		/* Handle closed connection */
		return 0;
	}
	else if(x < 0)
	{
		if(! BIO_should_retry(bio))
		{
			/* Handle failed read here */
			printf("Handle failed read here ");
		}
		/* Do something to handle the retry */
	}else
	{
		printf("user result:%s\n",buf2);

	}
	//写入到连接
	if(BIO_write(bio, buf1, strlen(buf1)) <= 0)
	{
		if(! BIO_should_retry(bio))
		{
			/* Handle failed write here */
			printf(" Handle failed write here");
		}

		/* Do something to handle the retry */
	}
	//从连接读取
	x = BIO_read(bio, buf2, 1024);
	if(x == 0)
	{
		/* Handle closed connection */
		return 0;
	}
	else if(x < 0)
	{
		if(! BIO_should_retry(bio))
		{
			/* Handle failed read here */
			printf("Handle failed read  here ");
		}
		/* Do something to handle the retry */
		return 0;
	}
	else
	{
		printf("pass result:%s\n",buf2);
	}

	//写入到连接
	if(BIO_write(bio,buf3, strlen(buf3)) <= 0)
	{
		if(! BIO_should_retry(bio))
		{
			/* Handle failed write here */
			printf(" Handle failed write here");
		}

		/* Do something to handle the retry */
	}
	//从连接读取
	x = BIO_read(bio, buf11,sizeof(buf11)-1);
	if(x == 0)
	{
		printf("no data");
		/* Handle closed connection */
		return 0;
	}
	else if(x < 0)
	{
		if(! BIO_should_retry(bio))
		{
			printf("retry!\n"); fflush(stdout);
			/* Handle failed read here */
		}
		/* Do something to handle the retry */
		return 0;
	}
	else
	{
		int recvd = 0;
		while(x>0){
			recvd += x;
			x = BIO_read(bio, buf11+recvd,sizeof(buf11)-recvd-1);
		}
		printf("retr 1 result:%s\n",buf11);
		fflush(stdout);
	}

	/*
	//写入到连接
	if(BIO_write(bio,buf4, strlen(buf4)) <= 0)
	{
	if(! BIO_should_retry(bio))
	{
	//Handle failed write here 
	printf(" Handle failed write here");
	}

	//Do something to handle the retry 
	}
	//从连接读取
	x = BIO_read(bio, buf11,1024);
	if(x == 0)
	{
	//Handle closed connection 
	return 0;
	}
	else if(x < 0)
	{
	if(! BIO_should_retry(bio))
	{
	//Handle failed read here 
	}
	//Do something to handle the retry 
	return 0;
	}
	else
	{
	printf("result:%s\n",buf11);
	}
	printf("end:%s\n",buf11);
	*/
	fflush(stdout);

	SSL_CTX_free(contex);
	BIO_free_all(bio);
	return 0;
}

/**
 *
 * To:db0
From: =?gb2312?b?UVHTys/kudzA7dSx?=<10000@qq.com>
Subject: =?gb2312?b?wKy7+NPKvP649MDrzOHQ0Q==?=
X-QQ-STYLE: 1
Mime-Version: 1.0
Date:Thu, 27 Aug 2015 0:12:25 +0800
Content-Type: text/html;
charset="gb2312"
Content-Transfer-Encoding: base64

CQk8ZGl2IHN0eWxlPSJiYWNrZ3JvdW5kOiNlY2VjZWM7cGFkZGluZzozNXB4O3RleHQtYWxpZ246
Y2VudGVyOyI+CgkJCTxkaXYgY2xhc3M9InFtU3lzUGFnZSIgc3R5bGU9ImJhY2tncm91bmQ6I2Zm
ZiB1cmwoaHR0cDovL3Jlc2Nkbi5xcW1haWwuY29tL3poX0NOL2h0bWxlZGl0aW9uL2ltYWdlcy9u
ZXdpY29uL3N5c21haWwvc3lzX3NoYWRvd19iZy5naWYpIHJlcGVhdC15O3dpZHRoOjYxMnB4O3Rl
eHQtYWxpZ246bGVmdDtwb3NpdGlvbjpyZWxhdGl2ZTttYXJnaW46MCBhdXRvO2ZvbnQtc2l6ZTox
NHB4OyBmb250LWZhbWlseTonbHVjaWRhIEdyYW5kZScsVmVyZGFuYTtsaW5lLWhlaWdodDoxLjU7
Ij4KCQkJCTxkaXYgY2xhc3M9InFtU3lzSGVhZGVyIiBzdHlsZT0iYmFja2dyb3VuZDp1cmwoaHR0
cDovL3Jlc2Nkbi5xcW1haWwuY29tL3poX0NOL2h0bWxlZGl0aW9uL2ltYWdlcy9uZXdpY29uL3N5
c21haWwvc3lzX3RvcC5naWYpIG5vLXJlcGVhdCB0b3AgIGxlZnQ7cGFkZGluZzo0cHggNnB4IDA7
Ij4KCQkJCQk8ZGl2IGNsYXNzPSJxbVN5c0hlYWRlckNvbnRhaW5lciIgc+OK
QMail POP3 Server v1.0 Service Ready(QQMail v2.0)
°   ö   ·

 *
 */
