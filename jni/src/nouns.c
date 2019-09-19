/**
 *
 gcc nouns.c  array.c  base64.c ipstring.c mystring.c urlcode.c myregex.c files.c httploader.c dict.c datas.c sqlite.c -lm -I"../SDL2/include/" -I"../SDL2_image/" -I"../SDL2_ttf/" -lSDL2_ttf -lSDL2_image -lSDL2 -lssl -lsqlite3 -lcrypto -lpthread -D debug_none && ./a.out
 */
#include "bytearray.h"
#include "dict.h"
#include "datas.h"
#include "files.h"
#include "httploader.h"
#include "mystring.h"
#include "sdlfiles.h"

Array * nounsArr = NULL;
void loadsub(char * _url)
{
	char * url = contact_str("http://www.24en.com",_url);
	URLRequest * urlrequest = Httploader_load(url);
	if(urlrequest->statusCode == 200){
		char filename[128];
		memset(filename,0,128);
		//SDL_Log("%s",urlrequest->data);

		Array * matched_arr = Array_new();
		//<a href="http://www.24en.com/subject/words/art/classical.html" class="cblue">文学</a> 

		//int n = regex_search_all(urlrequest->data,"/<a href=\"([^\"]{5,})\" class=\"[^\"]{3,}\">[^<]{1,}</a>/", matched_arr);
		int n = regex_search_all(urlrequest->data,"/[ \t\r\n]{1,}href=\"([^\"]{5,100})\"/i", matched_arr);
		printf("ok:%d\r\n",n);
		int i = 0;
		while(i<n)
		{
			if(nounsArr==NULL)
			{
				nounsArr = Array_new();
			}
			char * thumbURL = Array_getByIndex(matched_arr,i);
			char * _url = getStrBtw(thumbURL,"=\"","\"",0);
			free(thumbURL);
			//if(regex_match(_url,"//[-a-z ]{2,}$/i"))
			if(regex_match(_url,"//[-a-z() ]{2,}$/i"))
			{
				char * word = regex_replace_all(_url,"/^.*/([^/]{2,})$/i","$1");
				printf("%d:%s\r\n",i,word);
				Array_push(nounsArr,word);
				//loadsub(_url);
				//return;
			}


			++i;
		}
		Array_clear(matched_arr);
	}
	URLRequest_clear(urlrequest);
	fflush(stdout);
}
void None_loadApk()
{
	char *url = "http://www.24en.com/subject/words/";
	URLRequest * urlrequest = Httploader_load(url);
	if(urlrequest->statusCode == 200){
		char filename[128];
		memset(filename,0,128);
		//SDL_Log("%s",urlrequest->data);

		Array * matched_arr = Array_new();
		//<a href="http://www.24en.com/subject/words/art/classical.html" class="cblue">文学</a> 

		Array * urlArr = NULL;
		//int n = regex_search_all(urlrequest->data,"/<a href=\"([^\"]{5,})\" class=\"[^\"]{3,}\">[^<]{1,}</a>/", matched_arr);
		int n = regex_search_all(urlrequest->data,"/A[ \t\r\n]{1,}href=\"([^\"]{5,100})\"/", matched_arr);
		printf("ok:%d\r\n",n);
		int i = 0;
		while(i<n)
		{
			if(urlArr==NULL)
			{
				urlArr = Array_new();
			}
			char * thumbURL = Array_getByIndex(matched_arr,i);
			char * _url = getStrBtw(thumbURL,"=\"","\"",0);
			free(thumbURL);
			printf("%d:%s\r\n",i,_url);
			if(regex_match(_url,"^/"))
			{
				Array_push(urlArr,_url);
				loadsub(_url);
			}


			++i;
		}
		Array_clear(matched_arr);
	}
	URLRequest_clear(urlrequest);
	fflush(stdout);
	if(nounsArr)
	{
		int i = 0;
		char * s = "";
		while(i<nounsArr->length)
		{
			char * _word = (char*)Array_getByIndex(nounsArr,i);
			s = contact_str(s,_word);
			s = contact_str(s,"\n");
			//int _index = Dict_getWordIndex(dict,_word);
			++i;
		}
		writefile("~/sound/nouns.txt",s,strlen(s));
	}
}


#ifdef debug_none
int main()
{
	init_db();
	Dict * dict = Dict_new();
	dict->name = "oxford-gb";
	if(!fileExists("~/sound/nouns.txt"))
		None_loadApk();

	char * data = readfile("~/sound/nouns.txt",NULL);
	Array * array = string_split(data,"\n");
	if(array)
	{
		int i=0;
		int j=0;
		while(i<array->length)
		{
			char * _word = (char*)Array_getByIndex(array,i);
			int _index = Dict_getWordIndex(dict,_word);
			Word * word = Dict_getWordByIndex(dict,_index);
			//if(_index>0 && strcmp(_word,word->word))
			if(_index>0)
			{
				//del_word(_word);
				if(regex_match(_word,"/^[a-z-]{3,}$/i"))
				{
					printf("%d:%d:%s\r\n",j,i,_word);
					add_new_word(word->word,time(NULL));
					++j;
				}
			}
			++i;
		}
	}

	//Update_init();
	return 0;
}
#endif


