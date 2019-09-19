#ifndef _mime_h_
#define _mime_h_

#include "myregex.h"
#include "base64.h"
#include "mystring.h"
#include "files.h"


typedef struct Mime{
	char * data;
	int data_len;

	char*head;
	int head_len;

	char*body;
	int body_len;

	char * name;
	char * contentType;
	char * charset;
	char * Encoding;

	char * boundary;

	struct Mime * children;
	struct Mime * next;
	struct Mime * parent;
}Mime;

Mime* Mime_new(char*data,int data_len);
void Mime_clear(Mime*mime);
void Mime_parse(Mime*mime);

#endif
