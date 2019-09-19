/**
 * @file jsonrpc.c
 gcc -I"../SDL2/include/" -D test_jsonrpc jsonrpc.c files.c httploader.c ipstring.c mystring.c cJSON.c base64.c myregex.c array.c urlcode.c -lSDL2 -lpthread -lssl -lcrypto -lm && ./a.out  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-09-09
 */
#include "jsonrpc.h"

void Para_clear(Para * para)
{
	Para *cur = para;
	while(cur)
	{
		Para * tmp = cur->next;
		free(cur);
		cur = tmp;
	}
}
void Method_clear(Method* list)
{
	Method *cur = list;
	while(cur)
	{
		Method * tmp = cur->next;
		if(cur->params)
			Para_clear(cur->params);
		free(cur);
		cur = tmp;
	}
}

Para *Method_getParaByIndex(Method*method,int i)
{
	Para * cur = method->params;
	int index = 0;
	while(cur)
	{
		if(index == i)
			return cur;
		++index;
		cur = cur->next;
	}
	return NULL;
}

Para *Method_getParaByName(Method *method,char *name)
{
	Para * cur = method->params;
	while(cur)
	{
		if(!strcmp(cur->name,name))
			return cur;
		cur = cur->next;
	}
	return NULL;
}

Method *Method_getMethodByName(Method * methodlist,char *name)
{
	Method * cur = methodlist;
	while(cur)
	{
		if(!strcmp(cur->name,name))
			return cur;
		cur = cur->next;
	}
	return NULL;
}

static int requestId=0;
char * getRequestStr(Method *method)
{
	//{\"method\":\"Addons.ExecuteAddon\",\"params\":{\"addonid\":\"metadata.common.fanart.tv\"},\"id\":1,\"jsonrpc\":\"2.0\"}";
	char *params = malloc(1);
	*params = '\0';
	if(method->params)
	{
		Para * param = method->params;
		while(param)
		{
			if(strlen(params)){
				params = append_str(params,",");
			}
			if(!strcmp(param->type,"string")) {
				params = append_str(params,"\"%s\":\"%s\"",param->name,(char*)param->value);
			}else if(!strcmp(param->type,"boolean")) {
				params = append_str(params,"\"%s\":%s",param->name,(char*)param->value);
			}else if(!strcmp(param->type,"integer")) {
				params = append_str(params,"\"%s\":%d",param->name,(int)*((int*)param->value));
			}else if(!strcmp(param->type,"double")) {
				params = append_str(params,"\"%s\":%f",param->name,(double)*((double*)(param->value)));
			}else if(!strcmp(param->type,"array")) {
				char * value = cJSON_Print((cJSON*)param->value);
				params = append_str(params,"\"%s\":%s",param->name,value);
				free(value);
			}else if(!strcmp(param->type,"object")) {
				char * value = cJSON_Print((cJSON*)param->value);
				params = append_str(params,"\"%s\":%s",param->name,value);
				free(value);
			}
			param = param->next;
		}
	}
	char *ret = append_str("{\"method\":\"","%s\",\"params\":{%s},\"id\":%d,\"jsonrpc\":\"2.0\"}"
			,method->name
			,params
			,++requestId
			);
	return ret;
}

void Jsonrpc_print_value(cJSON*pRoot)
{
	if(pRoot==NULL)
	{
		printf("cJSON_Parse Error!\n");
		return ;
	}
	if(pRoot->string)
		printf(" %s = ",pRoot->string);
	else
		printf(" %s = ","");

	switch(pRoot->type)
	{
		case cJSON_False:
			printf("false\n");
			break;
		case cJSON_True:
			printf("true\n");
			break;
		case cJSON_NULL:
			printf("NULL\n");
			break;
		case cJSON_Number:
			printf("i %d\n",pRoot->valueint);
			printf("f %lf\n",pRoot->valuedouble);
			break;
		case cJSON_String:
			printf("s: %s\n",pRoot->valuestring);
			break;
		case cJSON_Array:
			printf("array\n");
			break;
		case cJSON_Object:
			printf("object:\n");
			break;
	}
}
typedef struct RpcType{
	char *id;
	char *extends;
	char *ref;
	char *vdefault;
	char * enums;
	char * type;
	char * properties;


}RpcType;
int numMethod = 0;
Method * Jsonrpc_print_node(Method*methodlist,cJSON *pRoot)
{
	if(pRoot==NULL)
	{
		return methodlist;
	}
	if(!strcmp(pRoot->string,"types")){
		cJSON * child = pRoot->child;
		while(child){
			child = child->next;
		}
	}else if(!strcmp(pRoot->string,"methods") || (!strcmp(pRoot->string,"notifications") && 0)){
		cJSON * child = pRoot->child;
		while(child){
			Method* method = (Method*)malloc(sizeof(Method));
			memset(method,0,sizeof(Method));
			if(methodlist) {
				Method *_tmp = methodlist;
				while(_tmp->next)
					_tmp = _tmp->next;
				_tmp->next = method;
				numMethod++;
			}else{
				methodlist = method;
			}



			method->name = child->string;
			//printf("%s(",method->name);
			cJSON *params = cJSON_GetObjectItem(child,"params");
			method->num_params = cJSON_GetArraySize(params);
			int i = 0;
			while(i<method->num_params)
			{
				//if(i>0) printf(",");
				Para * para = malloc(sizeof(Para));
				memset(para,0,sizeof(Para));
				if(method->params==NULL){
					method->params = para;
				}else{
					Para *tmp = method->params;
					while(tmp->next)
						tmp = tmp->next;
					tmp->next = para;
				}

				cJSON *param = cJSON_GetArrayItem(params,i);
				cJSON * type = cJSON_GetObjectItem(param,"type");
				if(type){
					switch(type->type)
					{
						case cJSON_String:
							para->type = type->valuestring;
							break;
						case cJSON_Object:
							para->type = "Object";
							break;
						case cJSON_Array:
							para->type = "Array";
							break;
						case cJSON_Number:
							para->type = "Number";
							break;
						case cJSON_NULL:
							para->type = "NULL";
							break;
					}
				}else{
					type = cJSON_GetObjectItem(param,"$ref");
					if(type==NULL)
						type = cJSON_GetObjectItem(param,"extends");
					para->type = type->valuestring;
				}
				//printf("%s ",para->type);

				cJSON * name = cJSON_GetObjectItem(param,"name");
				para->name = name->valuestring;
				//printf("%s",para->name);

				cJSON * required= cJSON_GetObjectItem(param,"required");
				if(required){
					para->required = required->type;
					//if(para->required) printf("*");
				}

				++i;
			}


			cJSON * description = cJSON_GetObjectItem(child,"description");
			if(description){
				method->description = description->valuestring;
				//if(method->description) printf("%s\n",method->description);
			}



			//printf(")\n");
			child = child->next;
		}
	}else{
		Jsonrpc_print_value(pRoot);
	}
	return methodlist;
}

#ifdef test_jsonrpc
static Method * method_list = NULL;

int main()
{
	URLRequest *urlrequest = URLRequest_new("http://127.0.0.1:8080/jsonrpc");
	//URLRequest *urlrequest = URLRequest_new("http://127.0.0.1:8080/jsonrpc?request={\"method\":\"Addons.ExecuteAddon\",\"params\":{\"addonid\":\"metadata.common.fanart.tv\"},\"id\":1,\"jsonrpc\":\"2.0\"}");
	//URLRequest *urlrequest = URLRequest_new("http://127.0.0.1:8080/jsonrpc?request={\"method\":\"Input.SendText\",\"params\":{\"text\":\"Kodi\",\"done\":true},\"id\":1,\"jsonrpc\":\"2.0\"}");
	//URLRequest *urlrequest = URLRequest_new("http://127.0.0.1:8080/jsonrpc?request={\"method\":\"Input.ExecuteAction\",\"params\":{\"action\":\"back\"},\"id\":1,\"jsonrpc\":\"2.0\"}");
	//URLRequest *urlrequest = URLRequest_new("http://127.0.0.1:8080/jsonrpc?request={\"method\":\"Playlist.GetPlaylists\",\"params\":{},\"id\":1,\"jsonrpc\":\"2.0\"}");
	URLRequest_setAuthorization(urlrequest,"kodi","sbhame");
	urlrequest = Httploader_request(urlrequest);
	if(urlrequest->statusCode == 200){
		if(urlrequest->respond->contentLength == strlen(urlrequest->data))
		{
			//printf("repond data:\n%s\n",urlrequest->data);
			//char *s = readfile("c.json",NULL);	
			//printf("%s",s);
			cJSON* pRoot = cJSON_Parse(urlrequest->data);
			if(pRoot){
				cJSON *child = pRoot->child;
				while(child){
					method_list = Jsonrpc_print_node(method_list,child);
					child = child->next;
				}
				cJSON_Delete(pRoot);
				pRoot = NULL;
			}
			//printf("%s",cJSON_Print(pRoot));
			//printf("repond data:\n%d\n",urlrequest->respond->contentLength);
			fflush(stdout);
			Method_clear(method_list);
			method_list = NULL;
		}
	}
	URLRequest_clear(urlrequest);
	urlrequest = NULL;
	int i=0;
	//while(i<0xffffffff) ++i;
	return 0;
}
#endif
