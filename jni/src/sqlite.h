#ifndef sqlite_h
#define sqlite_h
#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"

typedef struct DataBase
{
	sqlite3 *db;
	char * name;
	char * result_str;//json格式数据 或者错误信息
	Array * result_arr;//result_arr[0]:字段名，result_arr[1]:第一列数据数组,result_arr[2]:第二列数据数组,....
}DataBase;

DataBase *exec_sql(DataBase*db,char*Path);

DataBase * DataBase_new(char*name);//新建DataBase
void DataBase_clear(DataBase*database);//释放DataBase数据

int DataBase_exec(DataBase*db,const char * sql);//执行sql语句，把结果和错误信息保存至result_str
int DataBase_exec2array(DataBase*db,const char * sql);//执行sql语句，把结果保存至result_arr，错误信息保存至result_str

void DataBase_result_print(DataBase * db);//打印结果
void DataBase_free_result(DataBase * db);//释放结果


#endif
