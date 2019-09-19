
#ifndef jsonrpc_h
#define jsonrpc_h

#include <stdlib.h>
#include <stdio.h>
#include "httploader.h"
#include "cJSON.h"
#include "files.h"

typedef struct Para{
	char * name;
	int required;
	char* type;
	void* value;
	struct Para *next;
}Para;

typedef struct Method{
	char * name;
	char * description;
	Para * params;
	int num_params;
	struct Method*next;
}Method;

void Method_clear(Method* list);
void Jsonrpc_print_value(cJSON*pRoot);
Method* Jsonrpc_print_node(Method*methodlist,cJSON *pRoot);
#endif
