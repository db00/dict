/**
 * @file array.c
 gcc -Wall array.c -D test_array && ./a.out 0
 gcc -Wall array.c -D test_array  && a 0
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2016-01-12
 */
#include "array.h"

Array * Array_new()
{
	Array * array = malloc(sizeof(Array));
	memset(array,0,sizeof(Array));
	return array;
}
void Array_clear(Array * array)
{
	if(array)
	{
		if(array->length>0)
		{
			free(array->data);
		}
		free(array);
	}
}
void Array_freeEach(Array * array)
{
	if(array)
	{
		if(array->length>0)
		{
			int i = 0;
			while(i<array->length)
			{
				char * s = Array_getByIndex(array,i);
				if(s)
				{
					free(s);
				}
				++i;
			}
			free(array->data);
		}
		free(array);
	}
}
Array * Array_resize(Array* array,int i)
{
	if(array==NULL)
		array = Array_new();
	if(i==array->length)
		return array;
	if(i>0)
	{
		if(array->length==0)
		{
			array->data = malloc(i*sizeof(void*));
			memset(array->data,0,i*sizeof(void*));
		}else if(i>array->length){
			array->data = realloc(array->data,i*sizeof(void*));
			if(i>array->length)
				memset(&(array->data[array->length]),0,(i-array->length)*sizeof(void*));
		}else{//i<array->length
			memset(&(array->data[i]),0,(array->length-i)*sizeof(void*));
			array->data = realloc(array->data,i*sizeof(void*));
		}
		array->length = i;
		return array;
	}
	//i<=0
	if(array->length>0)
		free(array->data);
	array->length = 0;
	array->data = NULL;
	return array;
}
Array* Array_push(Array * array,void * d)
{
	if(array==NULL)
		array = Array_new();
	array = Array_resize(array,array->length+1);
	array->data[array->length-1] = d;
	return array;
}
void * Array_getByIndex(Array * array,int i)
{
	if(array==NULL)
		return NULL;
	void * data = NULL;
	if(0<=i && i<array->length)
	{
		data = array->data[i];
	}
	return data;
}
Array * Array_setByIndex(Array * array,int i,void * data)
{
	if(array==NULL)
		array = Array_new();
	if(i>=0)
	{
		if(i>=array->length)
		{
			array = Array_resize(array,i+1);
		}
		array->data[i]=data;
	}
	return array;
}
Array * Array_insert(Array * array,int i,void * data)
{
	if(array==NULL)
		array = Array_new();
	if(i>=0)
	{
		if(i>=array->length)
		{
			array = Array_resize(array,i+1);
		}else{
			array = Array_resize(array,array->length+1);//length + 1
			int movelen = (array->length-1-i)*sizeof(void*);//from i
			char * tmp = malloc(movelen);
			memcpy(tmp,&(array->data[i]),movelen);//copy start from i
			memcpy(&(array->data[i+1]),tmp,movelen);//copy to i+1
			free(tmp);
			//memmove(&(array->data[i+1]),&(array->data[i]),(array->length-i-1)*sizeof(void*));
		}
		array->data[i]=data;
	}
	return array;
}
Array * Array_removeByIndexs(Array*array,int start,int end)
{
	if(array==NULL)
		return NULL;
	if(start<0)
		start = 0;
	if(end>=array->length)
		end = array->length-1;
	int num = end-start+1;
	if(num>=0 && num<=array->length)
	{
		int movelen = (array->length-(start+num))*sizeof(void*);//from start+num
		char * tmp = malloc(movelen);
		memcpy(tmp,&(array->data[start+num]),movelen);//copy from start+num
		memcpy(&(array->data[start]),tmp,movelen);//copy to start
		free(tmp);
		//memmove(&(array->data[start]),&(array->data[start+num]),(array->length-start-num)*sizeof(void*));
		array = Array_resize(array,array->length-num);
	}
	return array;
}
Array * Array_removeByIndex(Array*array,int i)
{
	if(array==NULL)
		return NULL;
	if(i>=0 && i<array->length)
	{
		int movelen = (array->length-(i+1))*sizeof(void*);//copy from i+1
		if(movelen>0){
			char * tmp = malloc(movelen);
			memcpy(tmp,&(array->data[i+1]),movelen);//copy from i+1
			memcpy(&(array->data[i]),tmp,movelen);//copy to i
			free(tmp);
		}
		//memmove(&(array->data[i]),&(array->data[i+1]),(array->length-i-1)*sizeof(void*));
		array = Array_resize(array,array->length-1);
	}
	return array;
}
int Array_getIndexByValue(Array *array,void * value)
{
	if(array && value){
		int i = 0;
		while(i<array->length)
		{
			if(Array_getByIndex(array,i)==value)
				return i;
			++i;
		}
	}
	return -1;
}
int Array_getIndexByStringValue(Array *array,void * value)
{
	if(array && value){
		int i = 0;
		while(i<array->length)
		{
			if(strcmp(Array_getByIndex(array,i),value)==0)
				return i;
			++i;
		}
	}
	return -1;
}
Array * Array_removeByValue(Array *array,void * value)
{
	int i = Array_getIndexByValue(array,value);
	if(i>=0){
		return Array_removeByIndex(array,i);
	}
	return array;
}
Array * Array_getByIndexs(Array * array,int start,int end)
{
	if(start>=array->length)
		return NULL;
	if(end<0)
		return NULL;
	if(array==NULL)
		array = Array_new();
	Array * ret=NULL;
	end = array->length-1;
	if(start<0)
		start=0;
	if(end>=array->length)
		end = array->length-1;
	if(end>=start)
	{
		int i = start;
		while(i<=end)
		{
			char * data = array->data[i];
			Array_push(ret,data);
			++i;
		}
	}
	return ret;
}

Array * Array_concat(Array * array,Array * array2)
{
	if(array ==NULL)
	{
		array = Array_new();
	}
	if(array2==NULL || array2->length==0)
		return array;
	int len = array->length;
	array = Array_resize(array,array->length+array2->length);
	memcpy((void*)&(array->data[len]),array2->data,array2->length*sizeof(void*));
	return array;
}

char * Array_joins(Array *array,char * split)
{
	if(array && array->length)
	{
		int i = 0;
		int len = 0;
		int splitLen = 0;
		if(split)
			splitLen = strlen(split);
		while(i<array->length)
		{
			char * s = Array_getByIndex(array,i);
			if(s)
				len += strlen(s)+splitLen;
			else
				len += 6+splitLen;
			++i;
		}
		char * ret = malloc(len+1);
		memset(ret,0,len+1);
		i = 0;
		while(i<array->length)
		{
			char * s = Array_getByIndex(array,i);
			if(split && strlen(split) && strlen(ret))
				sprintf(ret+strlen(ret),"%s",split);
			if(s)
				sprintf(ret+strlen(ret),"%s",s);
			else
				sprintf(ret+strlen(ret),"(NULL)");
			++i;
		}
		return ret;
	}
	return NULL;
}

Array * Array_sort(Array * array,int (*sort)(const void *,const void *))
{
	qsort((void *)array->data,array->length,sizeof(void*),(int (*)(const void *,const void *))sort);
	return array;
}

void* Array_search(Array * array,void * key,int (*sort)(const void *,const void *))//必须先排序
{
	//qsort((void *)array->data,array->length,sizeof(void*),(int (*)(const void *,const void *))sort);//
	return (void *)bsearch(key,array->data,array->length,sizeof(void*),(int (*)(const void *,const void *))sort);
}


void Array_prints(Array * array)
{
	if(array==NULL)
	{
		printf("NULL\n");
		return;
	}
	int i=0;
	printf("[");
	while(i<array->length)
	{
		printf("%d:%s,",i,(char*)(array->data[i]));
		++i;
	}
	printf("]\n");
}

void Array_print(Array * array)
{
	if(array==NULL)
	{
		printf("NULL\n");
		return;
	}
	int i=0;
	printf("[");
	while(i<array->length)
	{
#ifdef __x86_64__
		printf("%d:%llx,",i,(unsigned long long )(array->data[i]));
#elif __i386__
		printf("%d:%x,",i,(unsigned)(array->data[i]));
#endif
		++i;
	}
	printf("]\n");
}

#ifdef test_array
int main(int argc,char**argv)
{

	//for(int i=0;i<1;i++)printf("hello\n");
	int i=0;
	while(i<argc)
	{
		printf("%d:%s\n",i,argv[i]);
		++i;
	}
	printf("\n");

	Array * array =Array_new();
	Array_prints(array);
	char * a="1";
	Array_push(array,a);
	Array_prints(array);
	a = "2";
	Array_push(array,a);
	a = "3";
	Array_push(array,a);
	a = "4";
	Array_push(array,a);
	a = "5";
	Array_push(array,a);
	a = "6";
	Array_push(array,a);
	Array_prints(array);
	Array_removeByIndex(array,0);
	Array_prints(array);
	Array_removeByIndexs(array,0,1);
	Array_prints(array);
	a = "iiiiiiiiii";
	Array_insert(array,9,a);
	Array_prints(array);
	Array_removeByIndexs(array,2,8);
	Array_prints(array);
	Array_removeByIndex(array,0);
	Array_prints(array);
	Array_removeByIndex(array,0);
	Array_prints(array);
	Array_removeByIndex(array,0);
	Array_prints(array);
	printf("%d",array->length);
	return 0;
}
#endif
