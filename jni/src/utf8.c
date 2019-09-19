/**
 * @file utf8.c
 gcc -D debug_utf8 mystring.c utf8.c array.c && ./a.out 
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2016-09-07
 */
#include "utf8.h"

// one charactor size
int UTF8_numByte(char * utf8)
{
	unsigned char c = *utf8;
	if(c==0)
		return 0;
	if(c<0x80)
		return 1;
	if(c<0xe0)
		return 2;
	if(c<0xf0)
		return 3;
	if(c<0xf8)
		return 4;
	if(c<0xfe)
		return 5;
	if(c<0xff)
		return 6;
	return 0;
}

unsigned short int utf8_to_u16(char * s)//utf8 -> Unicode
{
	if(s==NULL)
		return 0;
	unsigned int w = 0;
	int len = UTF8_numByte(s);
	switch(len)
	{
		case 1:
			w += s[0];
			break;
		case 2:
			//110X XXXX
			w += (s[0] & 0x1f)<<6;
			//10XX XXXX
			w += (s[1] & 0x3f);
			break;
		case 3:
			//1110 XXXX
			w += (s[0] & 0x0f)<<12;
			//10XX XXXX
			w += (s[1] & 0x3f)<<6;
			//10XX XXXX
			w += (s[2] & 0x3f);
			break;
		case 4:
			//1111 0XXX
			w += (s[0] & 0x07)<<18;
			//10XX XXXX
			w += (s[1] & 0x3f)<<12;
			//10XX XXXX
			w += (s[2] & 0x3f)<<6;
			//10XX XXXX
			w += (s[3] & 0x3f);
			break;
		case 5:
			//1111 10XX
			w += (s[0] & 0x03)<<24;
			//10XX XXXX
			w += (s[1] & 0x3f)<<18;
			//10XX XXXX
			w += (s[2] & 0x3f)<<12;
			//10XX XXXX
			w += (s[3] & 0x3f)<<6;
			//10XX XXXX
			w += (s[4] & 0x3f);
			break;
		case 6:
			//1111 110X
			w += (s[0] & 0x01)<<30;
			//10XX XXXX
			w += (s[1] & 0x3f)<<24;
			//10XX XXXX
			w += (s[2] & 0x3f)<<18;
			//10XX XXXX
			w += (s[3] & 0x3f)<<12;
			//10XX XXXX
			w += (s[4] & 0x3f)<<6;
			//10XX XXXX
			w += (s[5] & 0x3f);
			break;
	}
	//printf("from utf8 to Unicode : %s->%d\r\n",s,w);
	return (unsigned short)w;
}


//num charactor of a utf8 string 
size_t UTF8_length(char * utf8)
{
	size_t len = 0;
	char * p = (char*)utf8;
	if(utf8)
	{
		int l = UTF8_numByte(p);
		while(l>0)
		{
			len++;
			p += l;
			l = UTF8_numByte(p);
		}

	}
	return len;
}

//make a Array contains each charactor of the utf8 string
Array * UTF8_each(const char * utf8)
{
	Array * array = NULL;
	char * p = (char*)utf8;
	if(utf8)
	{
		int l = UTF8_numByte(p);
		while(l>0)
		{
			char * s = getSubStr(p,0,l);
			array = Array_push(array,s);

			p += l;
			l = UTF8_numByte(p);
		}

	}
	return array;
}

#ifndef __ANDROID__
char * UTF8_encodeTo(char* inbuf,size_t * outlen,char * coding){
	size_t inlen =strlen(inbuf);
	iconv_t cd=iconv_open(coding,"UTF-8");
	char*outbuf=(char*)malloc(inlen*4);
	memset(outbuf,0,inlen*4);
	char*in=inbuf;
	char*out=outbuf;
	*outlen=inlen*4;
	iconv(cd,&in,(size_t*)&inlen,&out,outlen);
	*outlen=strlen(outbuf);
	//printf("%s\n",outbuf);
	//free(outbuf);
	iconv_close(cd);
	return outbuf;
}

char * UTF8_readFrom(char* inbuf,size_t * outlen,char * coding){
	size_t inlen =strlen(inbuf);
	iconv_t cd=iconv_open("UTF-8",coding);
	char*outbuf=(char*)malloc(inlen*4);
	memset(outbuf,0,inlen*4);
	char*in=inbuf;
	char*out=outbuf;
	*outlen=inlen*4;
	iconv(cd,&in,(size_t*)&inlen,&out,outlen);
	*outlen=strlen(outbuf);
	//printf("%s\n",outbuf);
	//free(outbuf);
	iconv_close(cd);
	return outbuf;
}
#endif

#ifdef debug_utf8
int main()
{
	printf("%d\n",UTF8_length("help年可!\n"));
	Array * array = UTF8_each("和可了ijs！");
	if(array)
	{
		printf("array->length:%d\n",array->length);
		int i = 0;
		while(i<array->length)
		{
			char * s = Array_getByIndex(array,i);
			if(s)
				printf("%d:%s\n",i,s);
			++i;
		}
		Array_freeEach(array);
		array=NULL;
	}
	return 0;
}
#endif
