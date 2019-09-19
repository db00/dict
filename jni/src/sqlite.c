/***
 *
 gcc -g sqlite.c array.c -lsqlite3 -D debug_sqlite && ./a.out
 */
#include "sqlite.h"

void DataBase_clear(DataBase*database)
{
	if(database){
		DataBase_free_result(database);
		sqlite3_close(database->db);
		if(database->name)
			free(database->name);
		free(database);
		database=NULL;
	}
}

DataBase * DataBase_new(char*name)
{
	DataBase *database = (DataBase*)malloc(sizeof(*database));
	memset(database,0,sizeof(*database));
	database->name = (char*)malloc(strlen(name)+1);
	memset(database->name,0,strlen(name)+1);
	sprintf(database->name,"%s",name);
	int rc = sqlite3_open(database->name, &(database->db));
	if( rc ){
		const char *errmsg = sqlite3_errmsg(database->db);
		fprintf(stderr, "Can't open database: %s\n", errmsg);
		DataBase_clear(database);
		return NULL;
	}
	return database;
}

static char * string_append(char * old,char * s)
{
	int len = 1;
	if(old != NULL){
		len += strlen(old);
	}
	if(s != NULL){
		len += strlen(s);
	}
	char * ret = malloc(len);
	memset(ret,0,len);

	if(old){
		strcat(ret,old);
		free(old);
	}
	if(s){
		strcat(ret,s);
	}
	return ret;
}
static int callback2(void *database, int argc, char **argv, char **azColName){
	DataBase*db = (DataBase*)database;
	int i;
	Array * nameArr = NULL;
	if(db->result_arr==NULL)
	{
		db->result_arr = Array_new();
		if(db->result_arr==NULL)
			return 1;
		nameArr = Array_new();
		Array_push(db->result_arr,nameArr);
	}else{
		nameArr = Array_getByIndex(db->result_arr,0);
	}
	for(i=0; i<argc; i++){
		if(nameArr->length<argc)
		{
			char * names = string_append(NULL,azColName[i]);
			Array_push(nameArr,names);
			Array_setByIndex(db->result_arr,i+1,Array_new());
		}
		Array * arr = Array_getByIndex(db->result_arr,i+1);
		char * values = string_append(NULL,argv[i]);
		Array_push(arr,values);
	}
	return 0;
}

static int callback(void *database, int argc, char **argv, char **azColName){
	DataBase*db = (DataBase*)database;
	int i;
	if(db->result_str && strlen(db->result_str)>1){
		db->result_str[strlen(db->result_str)-1]=',';
	}else{
		db->result_str = string_append(db->result_str,"[");
	}
	db->result_str = string_append(db->result_str,"{");
	for(i=0; i<argc; i++){
		db->result_str = string_append(db->result_str,"\"");
		db->result_str = string_append(db->result_str,azColName[i]);
		db->result_str = string_append(db->result_str,"\":\"");
		if(argv[i]) db->result_str = string_append(db->result_str,argv[i]);
		db->result_str = string_append(db->result_str,"\"");
		db->result_str = string_append(db->result_str,",");
	}
	db->result_str[strlen(db->result_str)-1]='\0';
	db->result_str = string_append(db->result_str,"}]");
	return 0;
}

void DataBase_free_result(DataBase * db)
{
	if(db->result_str){
		free(db->result_str);
		db->result_str = NULL;
	}
	if(db->result_arr){
		int i = 0;
		while(i<db->result_arr->length)
		{
			Array * arr = Array_getByIndex(db->result_arr,i);
			if(arr) Array_freeEach(arr);
			++i;
		}
		Array_clear(db->result_arr);
		db->result_arr= NULL;
	}
}

int DataBase_exec(DataBase*db,const char * sql)
{
	if(db == NULL || sql == NULL)
	{
		return -1;
	}

	DataBase_free_result(db);

	char *zErrMsg = 0;
	int rc = sqlite3_exec(db->db, sql ,callback, db, &zErrMsg);
	if(rc != SQLITE_OK){
		db->result_str = string_append(db->result_str,zErrMsg);
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -2;
	}
	if(db->result_str==NULL)
	{
		db->result_str = string_append(db->result_str,"(ok)\n");
		db->result_str = string_append(db->result_str,(char*)sql);
	}
	return 0;
}
int DataBase_exec2array(DataBase*db,const char * sql)
{
	if(db == NULL || sql == NULL)
	{
		return -1;
	}

	DataBase_free_result(db);


	char *zErrMsg = 0;
	int rc = sqlite3_exec(db->db, sql ,callback2, db, &zErrMsg);
	if(rc != SQLITE_OK){
		db->result_str = string_append(db->result_str,zErrMsg);
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -2;
	}
	if(db->result_str==NULL)
	{
		db->result_str = string_append(db->result_str,"(ok)\n");
		db->result_str = string_append(db->result_str,(char*)sql);
	}
	return 0;
}

void DataBase_result_print(DataBase * db)
{
	if(db==NULL)
		return;
	if(db->result_arr==NULL)
		return;
	if(db->result_arr->length<=0)
		return;
	Array * nameArr = Array_getByIndex(db->result_arr,0);
	int i=0;
	if(nameArr==NULL)
		return;
	printf("\n");
	printf("|");
	while(i<nameArr->length)
	{
		printf("%s|",(char*)Array_getByIndex(nameArr,i));
		++i;
	}
	printf("\n");

	i = 0;
	Array * arr = Array_getByIndex(db->result_arr,1);
	while(i<arr->length)
	{
		int j=0;
		printf("|");
		while(j<nameArr->length)
		{
			Array * varr = Array_getByIndex(db->result_arr,j+1);
			printf("%s|",(char*)Array_getByIndex(varr,i));
			++j;
		}
		printf("\n");
		++i;
	}
	fflush(stdout);
}

DataBase *exec_sql(DataBase*db,char*Path)
{
	char *sql=strstr(Path,":");
	char *sqlite=Path;
	if(sql!=NULL){
		sql=sql+1;
		if(strlen(sql)>0){
			if(strncmp("/sql:",sqlite,4)==0){//访问数据库
				if(db==NULL)
					db = DataBase_new("test.db");
				if(db){
					DataBase_exec(db,sql);
				}else{
					//DataBase_clear(db);
					//db = NULL;
				}
				return db;
			}
		}
	}
	return NULL;
}

#ifdef debug_sqlite
int main()
{
	DataBase *db = DataBase_new("test.db");
	if(db){
		int rc=0;
		rc = DataBase_exec(db,"create table if not exists member(id INTEGER primary key asc,time INTEGER,name,fromschool,age,sex INTEGER ,idcard,policies,familyaddress,parents_tel,tel,workplace,studyfee,workfee,classname,classno,photo);");
		if(!rc)printf("\nsql_result_str:%s",db->result_str);
		//rc = DataBase_exec(db,"insert into member(name,sex,fromschool,age,idcard,policies,familyaddress,parents_tel,tel,workplace,studyfee,workfee,classname,classno,photo) values (\"name\",1,\"fromschool\",\"age\",\"430422000000000000\",\"qunzhong\",\"familyaddress\",\"parents_tel\",\"tel\",\"workplace\",\"studyfee\",\"workfee\",\"classname\",\"classno\",\"photo\");");
		//if(!rc)printf("\nsql_result_str:%s",db->result_str);
		//rc = DataBase_exec(db,"select * from member;");
		//if(!rc)printf("\nsql_result_str:%s",db->result_str);
		//rc = DataBase_exec(db,"select * from sqlite_master;");
		//if(!rc)printf("\nsql_result_str:%s",db->result_str);
		rc = DataBase_exec2array(db,"select * from member;");
		DataBase_result_print(db);
		DataBase_clear(db);
		db = NULL;
	}
	return 0;
}
#endif

//create table if not exists member(id INTEGER primary key asc,time INTEGER,name,fromschool,age,sex INTEGER,idcard,policies,familyaddress,parents_tel,tel,workplace,studyfee,workfee,classname,classno,photo);
//insert into member(name,fromschool,age,sex,idcard,policies,familyaddress,parents_tel,tel,workplace,studyfee,workfee,classname,classno,photo) values ("name","fromschool","age",1,"430422000000000000","qunzhong","familyaddress","parents_tel","tel","workplace","studyfee","workfee","classname","classno","photo");
//select * from sqlite_master
//"update 'table_name' 'column_name' = 'value'"
//"DELETE FROM FAVORITE_TABLE WHERE EntryIndex = %d"
//"delete from 'table_name'"
//"drop table 'table_name'"

//adb push gcc /mnt/sdcard/Android/data/com.n0n3m4.droidc/files/gcc/
/*ln -s /mnt/sdcard/Android/data/com.n0n3m4.droidc/files/gcc/bin/arm-linux-androideabi-gcc /system/bin/gcc*/
/*ln -s /mnt/sdcard/Android/data/com.n0n3m4.droidc/files/gcc/bin/arm-linux-androideabi-g++ /system/bin/g++*/
/*ln -s /mnt/sdcard/Android/data/com.n0n3m4.droidc/files/gcc/bin/make /system/bin/make*/
/**/
/*lib = /mnt/sdcard/Android/data/com.n0n3m4.droidc/files/gcc/arm-linux-androideabi/lib/ */
/*include = /mnt/sdcard/Android/data/com.n0n3m4.droidc/files/gcc/arm-linux-androideabi/include/ */
