/**
 * @file mime.c
 gcc -Wall base64.c mime.c mystring.c regex.c myregex.c files.c  -lm -D debug_mime && ./a.out
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-07-22
 */

#include "mime.h"

void Mime_parse(Mime*mime)
{
	if(mime->head)
		return;
	printf("=======================\n");
	char *head_end = strstr(mime->data,"\r\n\r\n");
	if(head_end)
	{
		mime->head_len = head_end - mime->data;
		mime->head = malloc(mime->head_len+1);
		memset(mime->head,0,mime->head_len+1);
		strncat(mime->head,mime->data,mime->head_len);

		mime->contentType = regex_replace_all(mime->head,".*Content-Type:[\t ]*([\\/a-z\\-]+);.*","$1");
		printf("contentType ==> %s\n",mime->contentType);

		if(regex_match(mime->head,"name="))
		{
			mime->name = regex_replace_all(mime->head,".*name=\"([^\"]+)\".*","$1");
			printf("name ==> %s\n",mime->name);
		}
		if(regex_match(mime->head,"Content-Transfer-Encoding: "))
		{
			mime->Encoding= regex_replace_all(mime->head,".*Content-Transfer-Encoding:[\t ]*([\\/-a-zA-Z0-9]+).*","$1");
			printf("Encoding ==> %s\n",mime->Encoding);
		}
		if(regex_match(mime->head,"charset=\""))
		{
			mime->charset = regex_replace_all(mime->head,".*charset=\"([^\"]+)\".*","$1");
			printf("charset ==> %s\n",mime->charset);
		}

		mime->body_len = mime->data_len - (head_end-mime->data)-2;
		mime->body = malloc(mime->body_len+1);
		memset(mime->body,0,mime->body_len+ 1);
		strncat(mime->body,head_end,mime->body_len);
		printf("body_len:%d\n",strlen(mime->body));
	}

	printf("---------------------------\n");

	mime->boundary = strstr(mime->data,"boundary=\"");

	if(mime->boundary){
		char *boundary = mime->boundary + strlen("boundary=\"");
		char *boundaryEnd = strstr(boundary+1,"\"");
		int boundarylen = boundaryEnd - boundary;
		mime->boundary = malloc(boundarylen + 1);
		memset(mime->boundary,0,boundarylen+1);
		strncat(mime->boundary,boundary,boundarylen);
		//printf("%s\n",mime->boundary);

		int remainLen = mime->data_len - (boundaryEnd-mime->data);
		char *child_data_start = memstr(boundaryEnd,remainLen,mime->boundary);

		while(child_data_start)
		{
			child_data_start = child_data_start + strlen(mime->boundary) + 2;
			//printf("%d\n",strlen(child_data_start));
			remainLen = mime->data_len - (child_data_start - mime->data);
			char *child_data_end = memstr(child_data_start,remainLen,mime->boundary);
			int child_data_len = child_data_end - child_data_start;
			//printf("len:%d\n",child_data_len);
			Mime * child = Mime_new(child_data_start,child_data_len);
			child->parent = mime;
			if(child->name && regex_match(child->contentType,"application\\/octet-stream"))
			{
				printf("binary: %s\n",child->name);
				//printf("binaryhead:%s\n",child->head);

				if(regex_match(child->Encoding,"/base64/i"))
				{
					if(regex_match(child->name,"/(\\.png|\\.jpg|\\.jpeg|\\.gif)$/i"))
					{
						char *databytes = regex_replace_all(child->body,"[\r\n]+","");
						printf("len: %d\n",strlen(databytes));
						//printf("databytes:%s\n",databytes);
						int numbytes;
						char *bytes = base64_decode(databytes,strlen(databytes),&numbytes);
						writefile(child->name,bytes,numbytes);
						free(databytes);
						free(bytes);
					}
				}

			}else if(child->contentType && regex_match(child->contentType,"text/")) {
				printf("child->contentType:%s\n",child->contentType);
				if(regex_match(child->Encoding,"/base64/i"))
				{
					char *databytes = regex_replace_all(child->body,"[\r\n]+","");
					printf("len: %d\n",strlen(databytes));
					//printf("databytes:%s\n",databytes);
					int numbytes;
					char *bytes = base64_decode(databytes,strlen(databytes),&numbytes);
					printf("child->body:%s\n",bytes);
					free(databytes);
					free(bytes);
				}
			}
			if(mime->children==NULL){
				mime->children = child;
			}else{
				Mime *prev = mime->children;
				while(prev)
				{
					if(prev->next){
						prev = prev->next;
					}else{
						break;
					}
				}
				prev->next = child;
			}


			child_data_start = child_data_end;
			if(*(child_data_end+strlen(mime->boundary))=='-' && *(child_data_end+strlen(mime->boundary)+1)=='-')
			{
				//printf("%s end\n",mime->boundary);
				break;
			}
		}
	}
}

Mime* Mime_new(char*data,int data_len)
{
	Mime *mime = (Mime*)malloc(sizeof(*mime));
	memset(mime,0,sizeof(*mime));
	mime->data = data;
	mime->data_len = data_len;
	if(data && data_len>0){
		Mime_parse(mime);
	}
	return mime;
}

void Mime_clear(Mime*mime)
{
	if(mime)
	{
		if(mime->head)
			free(mime->head);
		if(mime->body)
			free(mime->body);

		if(mime->boundary)
			free(mime->boundary);

		if(mime->name)
			free(mime->name);
		if(mime->contentType)
			free(mime->contentType);
		if(mime->charset)
			free(mime->charset);
		if(mime->Encoding)
			free(mime->Encoding);

		Mime*child = mime->children;
		while(child)
		{
			Mime *next = child->next;
			Mime_clear(child);
			child = next;
		}

		free(mime);
	}
}



#ifdef debug_mime

int main(int argc, char *argv[])
{
	size_t data_len;
	char *data = readfile("a.data",&data_len);
	Mime *mime = Mime_new(data,data_len);
	Mime_clear(mime);
	free(data);

	return 0;
}
#endif
