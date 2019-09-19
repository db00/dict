#include "urlcode.h"
static const unsigned char hexchars[] = "0123456789ABCDEF";

static int htoi(char *s)
{/*{{{*/
	int value;
	int c;

	c = ((unsigned char *)s)[0];
	if (isupper(c))
		c = tolower(c);
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;
	c = ((unsigned char *)s)[1];
	if (isupper(c))
		c = tolower(c);
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;
	return (value);
}/*}}}*/
int url_decode(char *str, int len)
{/*{{{*/
	if(str!=NULL && len>0){}else{return 0;}
	char *dest = str;
	char *data = str;

	if(len>0)
		while (len--)
		{
			if (*data == '+')
			{
				*dest = ' ';
			}
			else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2)))
			{
				*dest = (char) htoi(data + 1);
				data += 2;
				len -= 2;
			}
			else
			{
				*dest = *data;
			}
			data++;
			dest++;
		}
	if(dest && data)*dest = '\0';
	return dest - str;
}/*}}}*/


char *url_encode(const char *s, int len, int *new_length,int skip_more)
{/*{{{*/
	//skip
	//0-9a-zA-Z-_.!~*'()
	register unsigned char c;
	unsigned char *to, *start;
	unsigned char const *from, *end;
	from = (unsigned char *)s;
	end  = (unsigned char *)s + len;
	start = to = (unsigned char *) calloc(1, 3*len+1);
	if(start == NULL)return NULL;
	while (from < end)
	{
		c = *from++;
		if (c == ' ') {
			*to++ = '+';
			continue;
		}else if(skip_more && ( // ;/?:@&=+$,# 
					c == '#' ||//35
					c == '$' ||//36
					c == '&' ||//38
					c == '+' ||//43
					c == ',' ||//44
					c == '/' ||//47
					c == ':' ||//58
					c == ';' ||//59
					c == '=' ||//61
					c == '?' ||//63
					c == '@' //64
					))
		{
			*to++ = c;
			continue;
		}
		if(
				('0'<= c && c<= '9') ||//48-57
				('a'<= c && c<= 'z') ||//97-122
				('A'<= c && c<= 'Z') ||//65-90
				c == '!' ||//33
				c == '\'' ||//39
				c == '(' ||//40
				c == ')' ||//41
				c == '*' ||//42
				c == '-' ||//45
				c == '.' ||//46
				c == '_' ||//95
				c == '~' //126
		  )
		{
			*to++ = c;
			continue;
		}else{
			to[0] = '%';
			to[1] = hexchars[c >> 4];
			to[2] = hexchars[c & 15];
			to += 3;
			continue;
		}
	}
	*to = 0;
	if (new_length)
	{
		*new_length = to - start;
	}
	return (char *) start;
}/*}}}*/
