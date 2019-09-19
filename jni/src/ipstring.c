/**
 * @file ipstring.c
 gcc -g -Wall -I"." ipstring.c array.c myregex.c regex.c filetypes.c dict.c files.c mystring.c urlcode.c base64.c -lpthread -lws2_32 -lm -D STDC_HEADERS -D DEBUG -D testip && a
 gcc -Wall -g -D testip array.c mystring.c ipstring.c && ./a.out
 gcc -Wall -I"../SDL2_image/" -I"../SDL2_ttf" -I"include" -L"lib" mysurface.c textfield.c files.c array.c matrix.c tween.c ease.c base64.c ipstring.c sprite.c httploader.c mystring.c lib/libeay32.dll.a lib/libssl32.dll.a -lgdi32 -lwsock32 -lssl -lssl32 -lcrypto -lSDL2_image -lSDL2_ttf -lmingw32 -lSDL2main -lSDL2 -I"../SDL2/include/" -lm -D debug_httploader && a
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2016-03-18
 */
#include "ipstring.h"

int isIpString(char*host)
{
	char *p= host;
	if(host && strlen(host)>3){
		while((*p != '\0') && (*p != ' '))
		{
			//printf("%c",*p);
			if(*p!='.' && isxdigit(*p)==0){
				return 0;
			}
			p++;
		}
		return 1;
	}
	return 0;
}

#ifdef __WIN32__
static const char * inet_ntop_v6 (const u_char *src, char *dst, size_t size);
static const char * inet_ntop_v4 (const void *src, char *dst, size_t size);
const char * inet_ntop(int af, const void *src, char *dst, size_t size)
{
    switch (af) {
    case AF_INET :
    return inet_ntop_v4 (src, dst, size);
#ifdef INET6
    case AF_INET6:
         return inet_ntop_v6 ((const u_char*)src, dst, size);
#endif
    default :
    //errno = EAFNOSUPPORT;
    return NULL;
    }
}
 
 
static const char * inet_ntop_v4 (const void *src, char *dst, size_t size)
{
    const char digits[] = "0123456789";
    int i;
    struct in_addr *addr = (struct in_addr *)src;
    u_long a = ntohl(addr->s_addr);
    const char *orig_dst = dst;
 
    if (size < INET_ADDRSTRLEN) {
    //errno = ENOSPC;
    return NULL;
    }
    for (i = 0; i < 4; ++i) {
    int n = (a >> (24 - i * 8)) & 0xFF;
    int non_zerop = 0;
 
    if (non_zerop || n / 100 > 0) {
        *dst++ = digits[n / 100];
        n %= 100;
        non_zerop = 1;
    }
    if (non_zerop || n / 10 > 0) {
        *dst++ = digits[n / 10];
        n %= 10;
        non_zerop = 1;
    }
    *dst++ = digits[n];
    if (i != 3)
        *dst++ = '.';
    }
    *dst++ = '\0';
    return orig_dst;
}
 
 
 
/*
 * Convert IPv6 binary address into presentation (printable) format.
 */
static const char * inet_ntop_v6 (const u_char *src, char *dst, size_t size)
{
  /*
   * Note that int32_t and int16_t need only be "at least" large enough
   * to contain a value of the specified size.  On some systems, like
   * Crays, there is no such thing as an integer variable with 16 bits.
   * Keep this in mind if you think this function should have been coded
   * to use pointer overlays.  All the world's not a VAX.
   */
#define IN6ADDRSZ 64
#define INT16SZ 4
  char  tmp [64+1];
  char *tp;
  struct {
    long base;
    long len;
  } best, cur;
  u_long words [IN6ADDRSZ / INT16SZ];
  int    i;
 
  /* Preprocess:
   *  Copy the input (bytewise) array into a wordwise array.
   *  Find the longest run of 0x00's in src[] for :: shorthanding.
   */
  memset (words, 0, sizeof(words));
  for (i = 0; i < IN6ADDRSZ; i++)
      words[i/2] |= (src[i] << ((1 - (i % 2)) << 3));
 
  best.base = -1;
  cur.base  = -1;
  for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
  {
    if (words[i] == 0)
    {
      if (cur.base == -1)
           cur.base = i, cur.len = 1;
      else cur.len++;
    }
    else if (cur.base != -1)
    {
      if (best.base == -1 || cur.len > best.len)
         best = cur;
      cur.base = -1;
    }
  }
  if ((cur.base != -1) && (best.base == -1 || cur.len > best.len))
     best = cur;
  if (best.base != -1 && best.len < 2)
     best.base = -1;
 
  /* Format the result.
   */
  tp = tmp;
  for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
  {
    /* Are we inside the best run of 0x00's?
     */
    if (best.base != -1 && i >= best.base && i < (best.base + best.len))
    {
      if (i == best.base)
         *tp++ = ':';
      continue;
    }
 
    /* Are we following an initial run of 0x00s or any real hex?
     */
    if (i != 0)
       *tp++ = ':';
 
    /* Is this address an encapsulated IPv4?
     */
    if (i == 6 && best.base == 0 &&
        (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
    {
      if (!inet_ntop_v4(src+12, tp, sizeof(tmp) - (tp - tmp)))
      {
        //errno = ENOSPC;
        return (NULL);
      }
      tp += strlen(tp);
      break;
    }
    tp += sprintf (tp, "%lX", words[i]);
  }
 
  /* Was it a trailing run of 0x00's?
   */
  if (best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ))
     *tp++ = ':';
  *tp++ = '\0';
 
  /* Check for overflow, copy, and we're done.
   */
  if ((size_t)(tp - tmp) > size)
  {
    //errno = ENOSPC;
    return (NULL);
  }
  return strcpy (dst, tmp);
  return (NULL);
}
#endif
char * domain2ipString(char * hostname)
{
	if(hostname && isIpString(hostname))
		return hostname;
	//return NULL;

#ifdef __WIN32__
	WORD wVersion;
	WSADATA WSAData;
	wVersion=MAKEWORD(2,2);
	WSAStartup(wVersion,&WSAData);
#endif
	struct addrinfo hints;
	struct addrinfo *res, *cur;
	int ret;
	struct sockaddr_in *addr;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family   = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags    = AI_PASSIVE;

	ret = getaddrinfo(hostname, NULL,&hints,&res);
	if (ret){
		fprintf(stderr,"getaddrinfo:%s --> %s\n", hostname,gai_strerror(ret));
		return NULL;
	}
	for (cur = res; cur != NULL; cur = cur->ai_next) {
		char ipbuf[32];
		memset(ipbuf,0,sizeof(ipbuf));
		addr = (struct sockaddr_in *)cur->ai_addr;
		if(res->ai_family == AF_INET){//IPv4
			inet_ntop(AF_INET, &addr->sin_addr, ipbuf, 16);
			if(isIpString(ipbuf))
			{
				printf("SDL get %s ip %s\n",hostname ,ipbuf);
				freeaddrinfo(res);
				return contact_str(ipbuf,"");
			}
		}if(res->ai_family == AF_INET6){
			// Found IPv6 address
			inet_ntop(AF_INET6,
					&(((struct sockaddr_in *)addr)->sin_addr),
					ipbuf, 32);
			printf("IPv6: %s\n", ipbuf);
			return contact_str(ipbuf,"");
		}
	}
	freeaddrinfo(res);
	return NULL;
}

#ifdef testip
int  main(int argc, char **argv)
{
	domain2ipString("baidu.com");
	return 0;
}
#endif
