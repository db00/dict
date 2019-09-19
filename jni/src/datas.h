#ifndef datas_h
#define datas_h

#include "sqlite.h"
#include "files.h"
#include "cJSON.h"


/**
 * oxford word max length: 21
 * langdao word max length: 48 35
 *
 * word max explain length go:23445
 *

 CREATE TABLE list(
 wordid INTEGER primary key asc,
 word varchar(50),
 date real,//加入时间
 remembered char,//0:new word,1:remembered
 numAccess INTEGER,//访问次数
 numTest INTEGER;//测试次数
 );

 CREATE TABLE history(
 id INTEGER primary key asc,
 wordid INTEGER;
 status varchar(10),//-:no data,0:test fail,1:test pass,
 date real,//访问时间
 );
 *
 */
#include <time.h>
DataBase *history_db;
int init_db();
int get_word_id(char * word);
int add_to_history(int wordid);
char * get_remembered_history(int remembered);
int add_new_word(char * word,time_t t);
int del_word(char * word);
char * datas_query(char * sql);
void add_remembered_word(char * word,int remembered);
Array * get_test_list(int startIndex,int numWords);
Array * get_review_list(int startIndex,int numWords);

Array * get_history_list(int numWords,char * word,char * compare);
Array * get_remembered_list(int remembered,int numWords,char * word,char * compare);
void change_word_rights(char * word,int num);//change word right number of test
#endif
