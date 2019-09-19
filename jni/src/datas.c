/***
 *
 gcc cJSON.c -I"../SDL2/include/" -lSDL2 sqlite.c datas.c files.c array.c mystring.c myregex.c -lm -lsqlite3 -D debug_datas -I"../SDL2/include/" && ./a.out
 */
#include "datas.h"

int add_new_word(char * word,time_t t)
{
	int id = get_word_id(word);
	if(id>0)
		return id;
	char * s ="insert or ignore into list(word,date) values (\"%s\",%d);";
	char sql[256];
	memset(sql,0,256);
	//t = time(NULL);
	sprintf(sql,s,word,t);
	int rc = DataBase_exec(history_db,sql);
	//if(!rc)printf("\n insert sql_result_str:%s",history_db->result_str);
	printf("\r\n%d",rc);
	return 0;
}

int del_word(char * word)
{
	int id = get_word_id(word);
	if(id>0)
	{
		char * s ="delete from list where wordid=%d;";
		char sql[256];
		memset(sql,0,256);
		sprintf(sql,s,id);
		int rc = DataBase_exec(history_db,sql);
		if(!rc)printf("\n update sql_result_str:%s",history_db->result_str);
	}
	return 0;
}

/**
 *
 * 把word加入熟词（1）/生词（0）
 *
 */
void add_remembered_word(char * word,int remembered)
{
	int id = get_word_id(word);
	int len = strlen(word)+50;
	char sql[len];
	memset(sql,0,len);
	if(id>0)
	{
		char * s ="update list set remembered=%d,date=%d where word=\"%s\";";
		sprintf(sql,s,remembered,time(NULL),word);
	}else{
		char * s ="insert or ignore into list(remembered,date,word) values (%d,%d,\"%s\");";
		//char * s ="update list set remembered=%d,date=%d where word=\"%s\";";
		sprintf(sql,s,remembered,time(NULL),word);
	}
	int rc = DataBase_exec(history_db,sql);
	if(!rc)printf("\n update sql_result_str:%s",history_db->result_str);
}

void change_word_rights(char * word,int num)
{
	char * s;
	if(num==0)
		s ="update list set numTest=%d,date=%d,numAccess=max(numAccess,numTest,numError)+1,numError=numError+1 where word='%s';";
	else
		s ="update list set numTest=%d,date=%d,numAccess=max(numAccess,numTest,numError)+1 where word='%s';";
	char sql[200];
	memset(sql,0,200);
	sprintf(sql,s,num,time(NULL),word);
	int rc = DataBase_exec(history_db,sql);
	if(!rc)printf("\n update sql_result_str:%s",history_db->result_str);
}

int get_word_id(char * word)
{
	char * s = "select wordid from list where word='%s';";
	char sql[200];
	memset(sql,0,200);
	sprintf(sql,s,word);
	int rc = DataBase_exec(history_db,sql);
	printf("\r\n%s,%d,%s",sql,rc,history_db->result_str);
	if(regex_match(history_db->result_str,"/:\"[0-9]+\"/"))
	{
		//if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
		int len=0;
		char * r =regex_replace(history_db->result_str,"/^.*:\"([0-9]+)\"\\}]$/i","$1",&len);
		printf("\r\n-------------------%s id: %s\r\n",word,r);
		return atoi(r);
	}
	return 0;
}

char * datas_query(char * sql)
{
	int rc;
	rc = DataBase_exec(history_db,sql);
	if(!rc){
		//printf("\n history :\n%s",history_db->result_str);
		return history_db->result_str;
	}
	return NULL;
}

Array * datas_query2(char * sql)
{
	int rc = DataBase_exec2array(history_db,sql);
	if(!rc){
		DataBase_result_print(history_db);
		return history_db->result_arr;
	}
	return NULL;
}



/*
   char * get_history()
   {
   return datas_query("select * from list group by wordid ORDER BY date desc;");
   }
   */
char * get_remembered_history(int remembered)
{
	char * s = ("select * from list where remembered=%d group by wordid ORDER BY date desc;");
	int len = strlen(s)+5;
	char sql[len];
	memset(sql,0,len);
	sprintf(sql,s,remembered);
	//printf("\n%s\n",sql);fflush(stdout);
	return datas_query(sql);
}

Array * get_history_list(int numWords,char * word,char * compare)
{
	//if(word==NULL||strlen(word)==0) word="";
	char *s;
	if(word && compare)
	{
		//s = ("select * from list where group by wordid ORDER BY date desc;");
		if(compare[0]=='<')
			s = "select *  from list where wordid %s (select wordid from list where word=='%s') order by wordid desc limit 0,%d;";
		else
			s = "select *  from list where wordid %s (select wordid from list where word=='%s') order by wordid limit 0,%d;";
		int len = strlen(s)+strlen(word)+strlen(compare)+10;
		char sql[len];
		memset(sql,0,len);
		sprintf(sql,s,compare,word,numWords);
		printf("\n%s\n",sql);fflush(stdout);
		return datas_query2(sql);
	}else{
		s = "select *  from list order by wordid desc limit 0,%d;";
		int len = strlen(s)+10;
		char sql[len];
		memset(sql,0,len);
		sprintf(sql,s,numWords);
		printf("\n%s\n",sql);fflush(stdout);
		return datas_query2(sql);
	}
}
Array * get_test_list(int startIndex,int numWords)
{
	init_db();
	char *s=NULL;
	s = "select * from list where remembered==0 order by wordid desc limit %d,%d;";
	int len = strlen(s)+20;
	char sql[len];
	memset(sql,0,len);
	sprintf(sql,s,startIndex,numWords);

	printf("\n%s\n",sql);fflush(stdout);
	return datas_query2(sql);
}
Array * get_review_list(int lastdate,int numWords)
{
	init_db();
	char *s=NULL;
	s = "select * from list where date<%d and remembered==1 order by date limit 0,%d;";
	int len = strlen(s)+20;
	char sql[len];
	memset(sql,0,len);

	//time_t t = time(NULL)-24*3600;
	//printf("%s",ctime(&t));
	sprintf(sql,s,lastdate,numWords);

	printf("\n%s\n",sql);fflush(stdout);
	return datas_query2(sql);
}

Array * get_remembered_list(int remembered,int numWords,char * word,char * compare)
{
	char *s=NULL;
	if(word && compare)
	{
		//s = ("select * from list where remembered=%d group by wordid ORDER BY date desc;");
		if(remembered){
			if(compare[0]=='<')
				s = "select * from list where date %s (select date from list where word=='%s') and remembered==%d order by date desc limit 0,%d;";
			else
				s = "select * from list where date %s (select date from list where word=='%s') and remembered==%d order by date limit 0,%d;";
		}else{
			if(compare[0]=='<')
				s = "select * from list where wordid %s (select wordid from list where word=='%s') and remembered==%d order by wordid desc limit 0,%d;";
			else
				s = "select * from list where wordid %s (select wordid from list where word=='%s') and remembered==%d order by wordid limit 0,%d;";
		}
		int len = strlen(s)+10;
		len += strlen(word);
		char sql[len];
		memset(sql,0,len);
		sprintf(sql,s,compare,word,remembered,numWords);
		return datas_query2(sql);
	}else{
		if(remembered){
			s = "select *  from list where remembered==%d order by date desc limit 0,%d;";
		}else{
			s = "select *  from list where remembered==%d order by wordid desc limit 0,%d;";
		}
		int len = strlen(s)+10;
		char sql[len];
		memset(sql,0,len);
		sprintf(sql,s,remembered,numWords);
		return datas_query2(sql);
	}

	//select *  from list where wordid <= (select wordid from list where word=="good") order by wordid desc limit 0,20;
	//char * s = ("select * from list where remembered=%d group by wordid ORDER BY date desc;");
	//sprintf(sql,s,remembered);
	//printf("\n%s\n",sql);fflush(stdout);
}


int init_db()
{
	if(history_db)
		return 0;

	history_db = DataBase_new(decodePath("~/sound/test.db"));
	int rc=0;
	//date 最近测试时间
	//numTest 连续正确次数
	//numError 测试错误次数
	//numAccess 测试总次数
	rc = DataBase_exec(history_db,"create table if not exists list(wordid INTEGER primary key asc,word text UNIQUE, date INTEGER, remembered char(1) default(0), numAccess INTEGER default(0),numTest INTEGER default(0),numError INTEGER default(0));");
	//rc = DataBase_exec(history_db,"create table if not exists list(wordid INTEGER primary key asc,word varchar(50), date real, remembered char(1), numAccess INTEGER, numTest INTEGER);");
	//if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
	//if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
	rc = DataBase_exec(history_db,"delete from list where word like \"%(%\" or \"%)%\";");
	rc = DataBase_exec(history_db,"delete from list where word like \"_\";");
	rc = DataBase_exec(history_db,"delete from list where word like \"__\";");
	rc = DataBase_exec(history_db,"delete from list where word like \"% %\";");
	rc = DataBase_exec(history_db,"delete from list where word like \"-%%\";");

	//添加列
	Array * result_arr =  datas_query2("select * from sqlite_master where name='list' and sql like '%%numError%%';");
	if(result_arr==NULL)
		rc = DataBase_exec(history_db,"alter table list add column numError INTEGER default(0);");
	/*
	 *
	 * alter table list add column numError INTEGER default(0);
	 *	select * from sqlite_master;
	 *	drop table if exists student;
	 alter table teacher rename to student;

	 select * from sqlite_master where name='list' and sql like '%numError%';

	 *
	 rc = DataBase_exec(history_db,"DROP TABLE IF EXISTS history");
	 rc = DataBase_exec(history_db,"delete from list where remembered=0 and word like \"%×%\" or \"%√%\";");
	 rc = DataBase_exec(history_db,"create table if not exists history(id INTEGER primary key asc, wordid INTEGER, status varchar(1), date real);");
	 rc = DataBase_exec(history_db,"delete from list where remembered=0 and word not like \"___%\";");
	 rc = DataBase_exec(history_db,"delete from list where remembered=0 and word=\"drafman\";");
	 rc = DataBase_exec(history_db,"delete from list where remembered=0 and word=\"rountine\";");
	 */
	printf("\r\n%d",rc);
	return 0;
}




#ifdef debug_datas

static int inserts(sqlite3 * conn,sqlite3_stmt * stmt3,char * word,int date,char * remembered,int numAccess,int numTest,int numError)
{
	/*
	   word text,
	   date INTEGER,
	   remembered char(1)
	   numAccess INTEGER
	   numTest INTEGER
	   */
	//在绑定时，最左面的变量索引值是1。
	//sqlite3_bind_int(stmt3,1,i);
	//sqlite3_bind_double(stmt3,2,i * 1.0);
	sqlite3_bind_text(stmt3,1,word,strlen(word),SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt3,2,date);
	sqlite3_bind_text(stmt3,3,remembered,strlen(remembered),SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt3,4,numAccess);
	sqlite3_bind_int(stmt3,5,numTest);
	sqlite3_bind_int(stmt3,5,numError);
	if (sqlite3_step(stmt3) != SQLITE_DONE) {
		sqlite3_finalize(stmt3);
		sqlite3_close(conn);
		return 1;
	}
	//重新初始化该sqlite3_stmt对象绑定的变量。
	sqlite3_reset(stmt3);
	//printf("Insert Succeed.\n");
	return 0;
}

static void _backup()
{
	system("rm ~/sound/test.db");
	system("adb pull /sdcard/sound/test.db ~/");
	DataBase * db = DataBase_new(decodePath("~/test.db"));
	char * sql = "select * from list;";
	DataBase_exec2array(db,sql);
	Array * data = db->result_arr;
	if(data)
	{
		Array * names = Array_getByIndex(data,0);
		if(names==NULL){
			printf("no names Array");
			return ;
		}
		int nCount = names->length;
		int i = 0;
		Array * words = NULL;
		Array * dates = NULL;
		Array * remembereds = NULL;
		Array * numAccesses = NULL;
		Array * numTests = NULL;
		Array * numErrors = NULL;
		while(i<nCount)
		{
			char * curName =Array_getByIndex(names,i);
			if(strcmp(curName,"word")==0){
				words = Array_getByIndex(data,i+1);
			}else if(strcmp(curName,"date")==0){
				dates = Array_getByIndex(data,i+1);
			}else if(strcmp(curName,"remembered")==0){
				remembereds = Array_getByIndex(data,i+1);
			}else if(strcmp(curName,"numAccess")==0){
				numAccesses = Array_getByIndex(data,i+1);
			}else if(strcmp(curName,"numTest")==0){
				numTests = Array_getByIndex(data,i+1);
			}else if(strcmp(curName,"numError")==0){
				numErrors = Array_getByIndex(data,i+1);
			}
			++i;
		}
		if(history_db==NULL){
			init_db();
		}

		const char* insertSQL = "replace into list(word,date,remembered,numAccess,numTest,numError) values(?,?,?,?,?,?);";
		sqlite3_stmt* stmt3 = NULL;
		sqlite3 * conn = history_db->db;
		if (sqlite3_prepare_v2(conn,insertSQL,strlen(insertSQL),&stmt3,NULL) != SQLITE_OK) {
			if (stmt3)
				sqlite3_finalize(stmt3);
			sqlite3_close(conn);
			printf("stmt3 Error\r\n");
			return;
		}
		i = 0;
		while(i<words->length)
		{
			int rc = inserts(conn,stmt3,
					Array_getByIndex(words,i),
					atoi(Array_getByIndex(dates,i)),
					Array_getByIndex(remembereds,i),
					atoi(Array_getByIndex(numAccesses,i)),
					atoi(Array_getByIndex(numTests,i)),
					atoi(Array_getByIndex(numErrors,i))
					);
			if(!rc){
				printf("%d: %s\r\n",i,Array_getByIndex(words,i));fflush(stdout);
				//printf("\nsql_result_str:%s",db->result_str);
			}else{
				printf("%d: %s\r\n",i,Array_getByIndex(words,i));fflush(stdout);
				printf("insert Error\r\n");
				return ;
			}
			++i;
		}
		sqlite3_finalize(stmt3);
	}
}

int main()
{
	init_db();
	Array * result_arr =  datas_query2("select * from sqlite_master where name='list' and sql like '%%numError%%';");
	if(result_arr)
	{
		printf("result_arr not NULL");
	}else{
		printf("result_arr is NULL");
	}
	return 0;
	time_t t = time(NULL)-24*3600;
	printf("%s,%d",ctime(&t));
	return 0;
	_backup();
	return 0;

	unsigned int i = -1;
	unsigned int j = -1;
	//printf("%d\r\n",-(((unsigned int)-1)/4));
	printf("%d\r\n",((unsigned int)-1)/4);
	if(history_db){
		int rc=0;
		/*
		   add_new_word("test",time(NULL));
		   printf("\r\n-------------------id:%d\r\n",rc);
		   if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
		//printf("\r\n-------------------id:%d\r\n",rc);
		rc = DataBase_exec(history_db,"select * from sqlite_master;");
		if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
		free(history_db->result_str);
		history_db->result_str=NULL;
		rc = DataBase_exec2array(history_db,"select * from list;");
		if(!rc)DataBase_result_print(history_db);
		*/
		//printf("%s",get_history());
		//add_remembered_word("test",1);
		//printf("%s",get_remembered_history(1));
		Array * arr = get_remembered_list(0,20,"good",">");
		DataBase_clear(history_db);
		history_db = NULL;
	}
	return 0;
}
#endif
