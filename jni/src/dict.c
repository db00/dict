/**
 *
 gcc -g -I"../SDL2/include/" files.c array.c mystring.c dict.c myregex.c -lm -D debug_dict  -lSDL2 && ./a.out close
 gcc -g -I"../SDL2/include/" files.c array.c mystring.c dict.c myregex.c -lm -D debug_dict  -lmingw32 -lSDL2main -lSDL2 && a help
 gcc  -Wall files.c array.c mystring.c dict.c myregex.c -lm -Ddebug  && ./a.out help
 gcc  -Wall files.c mystring.c dict.c myregex.c regex.c -lm -Ddebug -DSTDC_HEADERS && a nude
 gcc -Wall mystrings.c dict.c myregex.c regex.c -lm -Ddebug -DSTDC_HEADERS -o ~/a && ~/a
 gcc -Wall dict.c myregex.c -Ddebug && ./a.out
 gcc -Wall dict.c -Ddebug && a fun
 > dict.txt
 * @file readall.c
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2013-11-13
 */
#include "dict.h"

Dict * Dict_new()
{
	Dict * dict= (Dict*)malloc(sizeof(Dict));
	memset(dict,0,sizeof(*dict));
	return dict;
}


void Dict_freeWords(Dict*dict)
{
	if(dict->words)
	{
		int i= 0;
		Word *curWord = dict->words;
		while(curWord && i < dict->numIndexDealed)
		{
			if(curWord->word)
				free(curWord->word);
			++i;
			curWord ++;
		}
		free(dict->words);
		dict->words = NULL;
	}
}

void Dict_free(Dict*dict)
{
	if(dict){
		Dict_freeWords(dict);
		if(dict->index_file)
		{
			fclose(dict->index_file);
			dict->index_file= NULL;
		}
		if(dict->file)
		{
			fclose(dict->file);
			dict->file = NULL;
		}
		if(dict->name)
		{
			//free(dict->name);
			dict->name= NULL;
		}
		free(dict);
	}
}

inline static int to_int(unsigned char *from_int) {
	/*
	   int i=0;
	 *((char*)(&i)+0) = *(from_int+3);
	 *((char*)(&i)+1) = *(from_int+2);
	 *((char*)(&i)+2) = *(from_int+1);
	 *((char*)(&i)+3) = *(from_int+0);
	 return i;
	 */
	return *(from_int+3)+(*(from_int+2)<<8)+(*(from_int+1)<<16)+(*from_int<<24);
}


int Dict_readIndexFile(Dict *dict)
{
	if(dict == NULL) 
		return -1;
	if(dict->file == NULL){
		return -2;
		dict->file = Dict_open(dict);
	}

	if(dict->words == NULL)
	{
		dict->words = (Word*)malloc(sizeof(Word)*dict->wordcount);
		memset(dict->words,0,sizeof(Word)*dict->wordcount);
	}

	unsigned char* buffer = (unsigned char*)malloc(dict->idxfilesize +1);//read the index file
	memset(buffer,0,dict->idxfilesize+1);

	rewind(dict->index_file);
	fread(buffer,dict->idxfilesize,1,dict->index_file);

	unsigned char *head,*tail;
	tail = buffer;
	head = buffer;
	int it=0;//cur pos;
	int numWords = dict->numIndexDealed;//the len of word arr;
	for(; it < dict->idxfilesize; it++)
	{
		Word*curWord = NULL;
		if(*head == '\0')//reach the end of the word
		{
			curWord = dict->words + numWords;

			size_t wordLength = head-tail+1;
			if(curWord->word){
				free(curWord->word);
			}
			curWord->word = malloc(wordLength+1);
			memset(curWord->word,0,wordLength+1);

			strncpy(curWord->word,(char*)tail,wordLength);//save the word to arr;
			curWord->offset = to_int(head+1);//the offset of the word mean; 
			curWord->length = to_int(head+5);//the length of the word mean;
			curWord->index = numWords;
			//if(900<numWords && numWords < 910) printf("%s:%d,%d\n",curWord->word,curWord->offset,curWord->length);
			head += 9;
			tail = head;
			numWords ++;

			if(numWords==dict->wordcount){
				//printf("file end\r\n");
				break;//reach the end of file 
			}
		}else{
			head++;
		}
	}
	fclose(dict->index_file);
	dict->index_file= NULL;
	if(buffer)free(buffer);
	buffer = NULL;
	return 0;
}

FILE * Dict_open(Dict *dict)
{
	char * rootpath = decodePath("~/sound");
	char *_file = dict->name;

	if(dict->files==NULL){
		char dir[1024];
		memset(dir,0,1024);
		sprintf(dir,"%s/%s",rootpath,_file);
		dict->files = listDir(dir);
		if(dict->files==NULL)
			return NULL;
		int i=0;
		while(i<dict->files->length)
		{
			char * filename = Array_getByIndex(dict->files,i);
			int len = strlen(filename);
			if(strcmp(filename+len-5,".dict")==0){
				dict->dict_path = filename;
			}else if(strcmp(filename+len-4,".ifo")==0){
				dict->ifo_path = filename;
			}else if(strcmp(filename+len-4,".idx")==0){
				dict->idx_path = filename;
			}
			++i;
		}
	}
	dict->index_file = fopen(dict->idx_path,"rb");
	if(dict->index_file==NULL)
		return NULL;


	char* file_content = NULL;
	if(dict->ifo_path){
		file_content = readfile(dict->ifo_path,NULL);
	}

	if(file_content){
		char *curpara = strtok(file_content,"\r\n");
		while(curpara){
			char *wordcount = "wordcount=";
			char *idxfilesize = "idxfilesize=";
			if(strncasecmp(curpara,wordcount,strlen(wordcount))==0) {
				dict->wordcount=atoi(curpara+strlen(wordcount));
				//printf("dict->wordcount:%d,",dict->wordcount);
			}else if(strncasecmp(curpara,idxfilesize,strlen(idxfilesize))==0) {
				dict->idxfilesize=atoi(curpara+strlen(idxfilesize));
				//printf("dict->idxfilesize:%d,",dict->idxfilesize);
			}
			curpara = strtok(NULL,"\r\n");
		}
		free(file_content);
		file_content = NULL;
	}

	dict->file = fopen(dict->dict_path,"rb");
	if(Dict_readIndexFile(dict))
	{
		printf("open dict index_file error!\n");
	}
	free(rootpath);
	return dict->file;
}

int Dict_getWordIndex(Dict *dict,char * target_word)
{
	if(target_word== NULL || dict == NULL)
		return -1;

	if(dict->file== NULL)
		dict->file = Dict_open(dict);

	if(dict->words==NULL)
		return -1;

	//二分法查找
	int head=0;
	int tail=dict->wordcount;
	int cur=tail/2;
	while(1)
	{
		if(cur<0)
			return 0;
		if(cur>=dict->wordcount)
			return dict->wordcount -1;
		int cmp = strcasecmp(target_word,dict->words[cur].word);
		//int cmp = 
		if(0 == cmp)
		{
			if(strcmp(target_word,dict->words[cur].word)==0)
				return cur;
			int first = cur;
			int _i=-1;
			while(cur+_i>=0 && strcasecmp(target_word,dict->words[cur+_i].word)==0)
			{
				first = cur + _i;
				if(strcmp(target_word,dict->words[cur+_i].word)==0)
				{
					cur = cur+_i;
					return cur;
				}
				--_i;
			}
			_i = 1;
			while(cur+_i<dict->wordcount && strcasecmp(target_word,dict->words[cur+_i].word)==0)
			{
				if(strcmp(target_word,dict->words[cur+_i].word)==0)
				{
					cur = cur+_i;
					return cur;
				}
				++_i;
			}
			return first;
		}else if(0 > cmp){
			tail = cur-1;
		}else{
			head = cur+1;
		}
		cur=(tail+head)/2;

		if(tail<head){
			cur++;
			if(abs(cur)>=dict->wordcount)
				cur = dict->wordcount-1;
			return -(cur);
		}
	}
	return -1;
}

Word * Dict_contains(Dict *dict,char *word)
{
	int id = Dict_getWordIndex(dict,word);
	if(id>=0){
		Word* retword = (Word*)(dict->words+id);
		return retword;
	}
	return NULL;
}


Word * Dict_getWord(Dict *dict,char * target_word)
{
	//+s,+es,y->ies
	//f->ves,fe->ves,o->os,o->oes
	//+ing,e->ing,[rptl...]->{2}ing,c->cking
	//+ed,e->+ed,y->ied,[rptl..]->{2}ed,c->cked,
	//ox->oxen,child->children,man->men,ouse->ice
	int index = Dict_getWordIndex(dict,target_word);
	if(index>0 && index<dict->wordcount){
		return (Word*)(dict->words+index);
	}else if(index<0){
		int len = strlen(target_word);
		if(len<1)
			return (Word*)(dict->words+0);
		if(len<3)
			return (Word*)(dict->words-index);
		char word2[len];
		memset(word2,0,len);
		int _index;
		if(target_word[len-1]=='s' || target_word[len-1]=='d'){// +s +d
			snprintf(word2,len,"%s",target_word);
			printf("end with \"s\" or \"d\": %s",word2);
			_index = Dict_getWordIndex(dict,word2);
			if(_index>0)
				return (Word*)(dict->words+_index);
			if(word2[len-2]=='e'){// +es +ed
				snprintf(word2,len-1,"%s",target_word);
				_index = Dict_getWordIndex(dict,word2);
				if(_index>0)
					return (Word*)(dict->words+_index);
				if(word2[len-3]=='i'){// +ies +ied
					word2[len-3]='y';
					_index = Dict_getWordIndex(dict,word2);
					printf("end with \"s\" or \"d\": %s",word2);
					if(_index>0)
						return (Word*)(dict->words+_index);
				}else if(word2[len-3]=='v'){// +ves +ved
					word2[len-3]='f';
					_index = Dict_getWordIndex(dict,word2);
					printf("end with \"s\" or \"d\": %s",word2);
					if(_index>0)
						return (Word*)(dict->words+_index);
				}else// if(word2[len-3]=='k')
				{//c-> ckes cked
					word2[len-3]='\0';
					_index = Dict_getWordIndex(dict,word2);
					printf("end with \"s\" or \"d\": %s",word2);
					if(_index>0)
						return (Word*)(dict->words+_index);
				}
			}
		}else if(strcmp(target_word+len-3,"ing")==0){//+ing
			snprintf(word2,len-2,"%s",target_word);
			printf("end with \"ing\": %s",word2);
			_index = Dict_getWordIndex(dict,word2);
			if(_index>0)
				return (Word*)(dict->words+_index);
			word2[strlen(word2)]='e';//e -> ing
			printf("end with \"ing\": %s",word2);
			_index = Dict_getWordIndex(dict,word2);
			if(_index>0)
				return (Word*)(dict->words+_index);
			word2[strlen(word2)-2]='\0';//[rnbp]{2} + ing
			printf("end with \"ing\": %s",word2);
			_index = Dict_getWordIndex(dict,word2);
			if(_index>0)
				return (Word*)(dict->words+_index);

		}

	}

	int id = abs(index);
	if(id>=dict->wordcount)
		id=dict->wordcount-1;
	Word* retword = (Word*)(dict->words+id);
	return retword;
}


int matchs(char * word,char *s)
{
	int i= 0;
	char * s2 = malloc(strlen(s)+1);
	memcpy(s2,s,strlen(s));
	s2[strlen(s)]='\0';
	for(i=0;i<strlen(word);i++)
	{
		int j;
		int has=0;
		for(j=0;j<strlen(s2);j++)
		{
			if(word[i]==s2[j])
			{
				//int l = strlen(s2)-j-1;
				char a[strlen(s2)+1];
				memset(a,0,strlen(s2)+1);

				memcpy(a,s2,j);
				sprintf(a+j,"%s",s2+j+1);

				sprintf(s2,"%s",a);
				s2[strlen(a)]='\0';
				has = 1;
				break;
			}
		}
		if(!has)
		{
			free(s2);
			return 0;
		}
	}
	free(s2);
	return 1;
}

int Dict_getNumWords(Dict * dict)
{
	if(dict==NULL || dict->name==NULL)
		return 0;
	if(dict->file==0){
		dict->file = Dict_open(dict);
	}
	return dict->wordcount;
}

Word * Dict_getWordByIndex(Dict * dict,int id)
{
	if(dict==NULL)
		return NULL;
	if(dict->file==0){
		dict->file = Dict_open(dict);
	}
	Word * word = dict->words + id;
	return word;
}

Word *Dict_getWordByRegIndex(Dict * dict,char * _regex,int id)
{
	id++;
	while(id<dict->wordcount)
	{
		Word * word = Dict_getWordByIndex(dict,id);
		if(regex_match(word->word,_regex))
		{
			return word;
		}
		++id;
	}
	return NULL;
}
Word *Dict_getWordByRegWordNext(Dict * dict,char * _regex,Word*_word)
{
	int id = 0;
	if(_word)id=_word->index;
	while(id<dict->wordcount)
	{
		Word * word = Dict_getWordByIndex(dict,id);
		if(regex_match(word->word,_regex))
		{
			printf("found %s:%s\n",_regex,word->word);
			return word;
		}
		++id;
	}
	return NULL;
}
Word *Dict_getWordByRegWordPrev(Dict * dict,char * _regex,Word*_word)
{
	int id = dict->wordcount-1;
	if(_word)id=_word->index;
	while(id>0)
	{
		Word * word = Dict_getWordByIndex(dict,id);
		if(regex_match(word->word,_regex))
		{
			printf("found %s:%s\n",_regex,word->word);
			return word;
		}
		--id;
	}
	return NULL;
}


int maxlen = 10;
char * Dict_getByIndex(Dict * dict,int id){
	if(dict==NULL)
		return NULL;

	if(dict->file==0){
		dict->file = Dict_open(dict);
	}

	Word*word = dict->words + id;
	if(word == NULL)
		return NULL;

	/*printf("word:%s,offset:%d,wordLength:%d\n",word->word,word->offset,word->length);*/
	/*
	   if(
	   regex_match(word->word,"/^[AAIUEODRM]{7,}$/i")
	   && matchs(word->word,"aaiueodrm")
	   )
	   printf("\r\n%s",word->word);
	   fflush(stdout);
	   */
	/*
	   if( !regex_match(word->word,"/[\\// ,()]/i")
	   && word->length >maxlen)
	   {
	   maxlen = word->length;
	   printf("\r\n%d==%d,%s:%d\r\n",id,word->index,word->word,maxlen);
	   }
	   */
	//else if (strlen(word->word)>50) printf("\r\n%s:%d\r\n",word->word,maxlen);

	rewind(dict->file);

	if(0 != fseek(dict->file,word->offset,SEEK_SET)){
		printf("seek word error\n");
		return NULL;
	}
	int len = word->length+1;
	char * explain = (char*)malloc(len);
	memset(explain,0,len);
	fread(explain,word->length,1,dict->file);
	return explain;
}

char * Dict_getMean(Dict* dict,Word* word)
{
	if(dict->file==NULL){
		dict->file = Dict_open(dict);
	}
	if(dict->file ==NULL)
		return "can not Dict_open";

	if(word == NULL){
		printf("#ERROR:not such word\n");
		return NULL;
	}

	//printf("word:%s,offset:%d,wordLength:%d\n",word->word,word->offset,word->length);
	//return NULL;
	if(0 != fseek(dict->file,word->offset,SEEK_SET)){
		printf("seek word error\n");
		return NULL;
	}

	char *explain = NULL;
	if(word && word->length && word->word){
		int len = word->length+2 + strlen(word->word);
		explain = (char*)malloc(len);
		memset(explain,0,len);
		//sprintf(explain,"%s",word->word);
		//sprintf(explain+strlen(explain),"\n");
		len = fread(explain+strlen(explain),1,word->length,dict->file);
		if(len != word->length)
			return NULL;
	}
	return explain;
}

char *Dict_explain(Dict* dict,char* target_word)
{
	if(dict->file==NULL){
		dict->file = Dict_open(dict);
	}
	if(dict->file ==NULL)
		return "can not Dict_open";

	Word*word = Dict_getWord(dict,target_word);

	return Dict_getMean(dict,word);
}

Array *Dict_getWordList(Dict*dict,char*s,int * numWords)
{
	int target_index = abs(Dict_getWordIndex(dict,s));
	printf("%d,",target_index);
	fflush(stdout);
	if(target_index >= dict->wordcount)
		target_index = dict->wordcount-1;
	int i = 0;
	Array *ret = NULL;
	while(i<*numWords){
		if(i+target_index >= dict->wordcount)
			break;
		Word * word = dict->words + target_index + i;

		if(word==NULL || word->word==NULL || strlen(word->word)==0)
			break;

		ret = Array_setByIndex(ret,i,dict->words + target_index + i);
		++i;
	}
	*numWords = i;
	return ret;
}

#ifdef debug_dict
#ifdef WIN32
#include "SDL.h"
#endif
int main(int argc,char**argv)
{

	Dict * dict = Dict_new();
	dict->name = "oxford-gb";
	printf("numWords:%d\r\n",Dict_getNumWords(dict));
	int _index = Dict_getWordIndex(dict,"hello");
	printf("%d",_index);
	return 0;
	int numWords = 10;
	int _i=0;
	Array *wordlist = Dict_getWordList(dict,"zz",&numWords);
	printf("\n %d\n",numWords);
	while(_i<numWords)
	{
		Word*word = Array_getByIndex(wordlist,_i);
		if(word) printf("%s\n",word->word);
		++_i;
	}
	Array_clear(wordlist);


	char* explain;


	if(argc>1){
		explain = Dict_explain(dict,argv[1]);
		if(explain==NULL)return 0;
		/*
		   char *tmp = regex_replace_all(explain,"([^a-zA-Z,;])( [\\*0-9]+ )","$1\n$2");
		   free(explain);
		   explain = regex_replace_all(tmp,"([:?!\\.]) ","$1\n");
		   free(tmp);
		   */
		char *tmp = regex_replace_all(explain,"([^a-zA-Z,;'> ]|[:?!\\.])([ \\*0-9]{,2} )","$1\n$2");
		free(explain);
		explain = tmp;
		printf("explaination:%s\n",explain);
		fflush(stdout);
	}

	Dict_free(dict);
	//return 0;
#ifdef WIN32
	//SDL_setenv("SDL_VIDEODRIVER","dummy",1);
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0) {
	}
	char*explain2 = SDL_iconv_string("gb2312", "UTF-8", explain, SDL_strlen(explain) + 1);
	//char*explain2 = SDL_iconv_string("UTF-8","gb2312", explain, SDL_strlen(explain) + 1);
	free(explain);
	explain = explain2;
#endif
	//printf("explaination:%s\n",explain);
	//return 0;
	fflush(stdout);
	free(explain);
	dict = Dict_new();
	//dict->name = "langdao";
	dict->name = "oxford-gb";
	dict->name = "ce";
	int i =1;
	//while(i < dict->wordcount)
	while(i < 39429)
		//while(i < 435468)
	{
		explain = regex_replace_all(Dict_getByIndex(dict,i)," \\* ","\r\n");
		printf("%s\r\n",explain);

		fflush(stdout);
		free(explain);
		break;
		++i;
	}
	Word * w = Dict_getWordByRegWordNext(dict,"/^s.*b.*d/i",NULL);
	/*getchar();*/
	return 0;
}
#endif
