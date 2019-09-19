/**
 * @file dict.h
 gcc -Wall dict.c files.c myregex.c array.c mystring.c -lSDL2 -lm -D debug_dict  && ./a.out nude
 gcc -Wall dict.c files.c myregex.c regex.c ic.c -lm -Ddebug -DSTDC_HEADERS -liconv && a nude
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-08-03
 */
#ifndef dict_h
#define dict_h
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "mystring.h"
#include "myregex.h"
#include "files.h"
#include "array.h"

typedef struct
{
	int index;
	char *word;//单词
	int offset;//
	int length;//
} Word;

typedef struct
{
	char* version;            //版本
	int wordcount;            //单词数量
	int idxfilesize;          //索引文件大小
	char* sametypesequence;
	Array * files;//文件列表
	char * ifo_path;
	char * idx_path;
	char * dict_path;

	int numIndexDealed;//处理的index_file 单词个数
	char* name;           //词典名称
	FILE* index_file;//字典索引文件
	FILE* file;//字典文件
	Word* words;//单词索引
} Dict;

int Dict_getNumWords(Dict * dict);
Dict * Dict_new();
void Dict_freeWords(Dict*dict);
void Dict_free(Dict*dict);
//int Dict_readIndexFile(Dict *dict);
FILE * Dict_open(Dict *dict);
Word*Dict_contains(Dict *dict,char *word);
int Dict_getWordIndex(Dict *dict,char * target_word);
Word*Dict_getWord(Dict *dict,char * target_word);
char *Dict_getByIndex(Dict * dict,int id);
Word *Dict_getWordByIndex(Dict * dict,int id);
char *Dict_explain(Dict* dict,char* target_word);
char *Dict_getMean(Dict* dict,Word* word);
Array *Dict_getWordList(Dict*dict,char*s,int* numWords);
Word *Dict_getWordByRegIndex(Dict * dict,char * _regex,int id);//search the first match after index(id)
Word *Dict_getWordByRegWordNext(Dict * dict,char * _regex,Word*_word);//search the next match after _word
Word *Dict_getWordByRegWordPrev(Dict * dict,char * _regex,Word*_word);//search the previous match before _word
char * get_history();

#endif// dbs_h_libiao
