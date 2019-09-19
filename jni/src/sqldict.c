/**
 * @file sqldict.c
 gcc -ldl -lpthread -lsqlite3 -lSDL2 date.c array.c myregex.c mystring.c files.c -I"../SDL2/include/" sqldict.c sqlite.c dict.c  && ./a.out
 gcc regex.c array.c sqlite3.c myregex.c mystring.c files.c -I"../SDL2/include/" sqldict.c sqlite.c dict.c  && a
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2017-05-03
 */

#include "sqlite.h"
#include "date.h"
#include "dict.h"

int inserts(sqlite3*  conn,sqlite3_stmt * stmt3,char * word,char * explain)
{
	//在绑定时，最左面的变量索引值是1。
	//sqlite3_bind_int(stmt3,1,i);
	//sqlite3_bind_double(stmt3,2,i * 1.0);
	sqlite3_bind_text(stmt3,1,explain,strlen(explain),SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt3,2,word,strlen(word),SQLITE_TRANSIENT);
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


char * ifo(char * name,char * version,int wordcount,int idxfilesize,char * bookname)
{
	int len = 1024;
	char * str = malloc(len);
	Date * date = Date_new(NULL);
	char s[] = "%s\r\nversion=%s\r\nwordcount=%d\r\nidxfilesize=%d\r\nbookname=%s\r\nauthor=xx\r\ndescription=db0@qq.com modified\r\ndate=%d.%d.%d\r\nsametypesequence=m";
	memset(str,0,len);
	sprintf(str,s,name,version,wordcount,idxfilesize,bookname,date->tm_year+1900,date->tm_mon,date->tm_mday);
	return str;
}
char * idx_item(char * p,char * word,int offset,int len)
{
	int l = strlen(word);
	sprintf(p,"%s",word);
	*(p+l)='\0';
	*(p+l+1)=*((char*)&offset+3);
	*(p+l+2)=*((char*)&offset+2);
	*(p+l+3)=*((char*)&offset+1);
	*(p+l+4)=*((char*)&offset+0);
	*(p+l+5)=*((char*)&len+3);
	*(p+l+6)=*((char*)&len+2);
	*(p+l+7)=*((char*)&len+1);
	*(p+l+8)=*((char*)&len+0);
	return p+l+9;
}
char * word_item(char*idx_p,char * dict_p,int * dictlen,char * word,char * explain)
{
	int len = strlen(explain);
	idx_p = idx_item(idx_p,word,strlen(dict_p),strlen(explain));
	sprintf(dict_p+(*dictlen),"%s",explain);
	*dictlen += len;
	return idx_p;
}

int make_dict(char * db_path,char * dict_name)
{
	DataBase *db = DataBase_new(db_path);
	if(db==NULL){
		printf("can not open DataBase %s ! \r\n",db_path);
		return 5;
	}

	int i = 0;
	char * dict = malloc(0x8000000);
	memset(dict,0,0x8000000);//2^27=64M
	char * idx = malloc(0x4000000);
	memset(idx,0,0x2000000);//2^25=16M
	char * idx_p = idx;
	int dictlen=0;



	Array * wordsArr = NULL;
	Array * explainsArr = NULL;
	char sql[128];
	memset(sql,0,sizeof(sql));
	sprintf(sql,"select * from dict ORDER BY word COLLATE NOCASE limit 0,-1;");
	int rc = DataBase_exec2array(db,sql);
	if(rc){
		//printf("\nsql_result_str:%s\r\n",db->result_str);
		return 6;
	}else if(db->result_arr){
		Array * data = db->result_arr;
		Array * names = Array_getByIndex(data,0);
		if(names==NULL){
			printf("no names Array");
			return 4;
		}
		int nCount = names->length;
		int i = 0;
		while(i<nCount)
		{
			char * curName =Array_getByIndex(names,i);
			if(strcmp(curName,"word")==0)
			{
				wordsArr = Array_getByIndex(data,i+1);
				if(wordsArr == NULL || wordsArr->length==0){
					printf("%d:no word\r\n",i);
					return 1;
					break;
				}
			}else if(strcmp(curName,"explain")==0){
				explainsArr = Array_getByIndex(data,i+1);
				if(explainsArr == NULL || explainsArr->length==0){
					printf("%d:no explain\r\n",i);
					return 2;
					break;
				}
			}
			++i;
		}
	}

	if(wordsArr && explainsArr && wordsArr->length>0 && wordsArr->length == explainsArr->length)
	{
		int i = 0;
		while(i<wordsArr->length){
			char * word = NULL;
			char * explain = NULL;
			word = Array_getByIndex(wordsArr,i);
			explain = Array_getByIndex(explainsArr,i);
			if(word == NULL || explain == NULL)
				break;
			if(i%1000==0)
			{
				printf("\r\n %d,%s,",i,word);
			}
			idx_p = word_item(idx_p,dict,&dictlen,word,explain);
			++i;
		}
	}else{
		return 7;
	}

	printf("\r\n total word: %d,",wordsArr->length);

	int idxfilesize = idx_p - idx;

	char * ifo_s = ifo(dict_name,"2.4.5",wordsArr->length,idxfilesize,dict_name);
	char filename[128];
	memset(filename,0,sizeof(filename));

	char * dict_dir = decodePath(contact_str("~/sound/",dict_name));
	sprintf(filename,"%s/%s.ifo",dict_dir,dict_name);
	writefile(filename,ifo_s,strlen(ifo_s));

	memset(filename,0,sizeof(filename));
	sprintf(filename,"%s/%s.idx",dict_dir,dict_name);
	writefile(filename,idx,idxfilesize);

	memset(filename,0,sizeof(filename));
	sprintf(filename,"%s/%s.dict",dict_dir,dict_name);
	writefile(filename,dict,dictlen);

	//DataBase_result_print(db);
	DataBase_clear(db);
	db = NULL;

	free(ifo_s);
	free(idx);
	free(dict);

	//int writefile(char * path,char *data,size_t data_length)

	return 0;
}

int update_one_word(DataBase * db,char * word,char * explain)
{
	if(word==NULL || explain==NULL || strlen(word)==0 || strlen(explain)==0)
		return 1;
	int rc=0;
	if(db==NULL){
		rc = DataBase_exec(db,"create table if not exists dict(id INTEGER primary key asc,word text UNIQUE,explain text);");
		if(!rc){
			//printf("\nsql_result_str:%s",db->result_str);
		}else{
			return 1;
		}
	}

	//const char* insertSQL = "replace into dict(explain,word) values(?,?);";
	//const char* insertSQL = "insert into dict(explain,word) values(?,?);";
	const char* insertSQL = "replace into dict(explain,word) values(?,?);";
	//const char* insertSQL = "update dict set explain=? where word=?;";
	sqlite3_stmt* stmt3 = NULL;
	sqlite3 * conn = db->db;
	if (sqlite3_prepare_v2(conn,insertSQL,strlen(insertSQL),&stmt3,NULL) != SQLITE_OK) {
		if (stmt3)
			sqlite3_finalize(stmt3);
		sqlite3_close(conn);
		printf("\n ERROR 0:%s,%s\r\n",explain,word);
		return 1;
	}
	rc = inserts(db->db,stmt3,word,explain);
	if(!rc){
		//printf("\r\n %s",word);fflush(stdout);
		//printf("\nsql_result_str:%s",db->result_str);
	}else{
		printf("\n ERROR:%s,%s\r\n",explain,word);
		return 1;
	}
	sqlite3_finalize(stmt3);
	//DataBase_clear(db); db = NULL;
	return 0;
}
int update_word(char * db_path,char * word,char * explain)
{
	if(word==NULL || explain==NULL || strlen(word)==0 || strlen(explain)==0)
		return 1;
	DataBase *db = DataBase_new(db_path);
	if(db){
		return update_one_word(db,word,explain);
	}
	return -1;
}


int make_db(char * dict_name,char * db_path)
{
	DataBase *db = DataBase_new(db_path);
	if(db){
		int rc=0;
		//rc = DataBase_exec(db,"create table if not exists dict(id INTEGER primary key asc,word varchar(200) UNIQUE,explain text);");
		rc = DataBase_exec(db,"create table if not exists dict(id INTEGER primary key asc,word text UNIQUE,explain text);");
		if(!rc)printf("\nsql_result_str:%s",db->result_str);
		else return 1;
		//rc = DataBase_exec(db,"CREATE UNIQUE INDEX unique_index_word ON dict(word);");
		//if(!rc)printf("\nsql_result_str:%s",db->result_str);
		//else return 2;

		//char sql[30000];
		//memset(sql,0,sizeof(sql));


		//const char* insertSQL = "insert into dict(word,explain) values(?,?);";
		const char* insertSQL = "replace into dict(explain,word) values(?,?);";
		sqlite3_stmt* stmt3 = NULL;
		sqlite3 * conn = db->db;
		if (sqlite3_prepare_v2(conn,insertSQL,strlen(insertSQL),&stmt3,NULL) != SQLITE_OK) {
			if (stmt3)
				sqlite3_finalize(stmt3);
			sqlite3_close(conn);
			return 1;
		}


		Dict * dict = Dict_new();
		dict->name = dict_name;
		if(dict->file==0){
			dict->file = Dict_open(dict);
		}
		int i = 0;
		/*while(i < dict->wordcount)*/
		while(i < dict->wordcount)
		{
			Word*word = Dict_getWordByIndex(dict,i);
			//if(word && regex_match(word->word,"\\(Brit\\)"))
			if(word)
			{
				//printf("%s\r\n",word->word);
				char * explain = Dict_getMean(dict,word);
				if(explain==NULL)
				{
					printf("\n ERROR:%s,%d,%s\r\n",explain,i,word->word);
					return 2;
				}
				int rc = inserts(db->db,stmt3,word->word,explain);
				if(!rc){
					if(i%100==0)
					{
						printf("\r\n %d,%s",i,word->word);fflush(stdout);
					}
					//printf("\nsql_result_str:%s",db->result_str);
				}else{
					printf("\n ERROR:%s,%d,%s\r\n",explain,i,word->word);
					return 1;
				}
				free(explain);
			}
			++i;
		}
		sqlite3_finalize(stmt3);



		//rc = DataBase_exec(db,"select * from dict;");
		//if(!rc)printf("\nsql_result_str:%s",db->result_str);
		DataBase_clear(db);
		db = NULL;
	}
	return 0;
}
//REPLACE INTO dict(word, explain) VALUES (?, ?);

int addirregularverb()
{
	char *s = readfile("irregularVerbs.txt",NULL);
	Array * arr = string_split(s,"\n");
	int i = 0;
	Dict * dict = Dict_new();
	dict->name = "oxford-gb";
	while(i<arr->length)
	{
		char * s2 = (char*)Array_getByIndex(arr,i);
		//printf("%d:%s\r\n",i,s2);
		Array * arr2 = string_split(s2," ");
		//if(arr2) printf("%d:%s\r\n",arr2->length,s2);
		//pt pp
		if(arr2 && arr2->length==3)
		{
			//printf("%d:%s\r\n",i,s2);
			char * verb = (char*)Array_getByIndex(arr2,0);
			char * pt = (char*)Array_getByIndex(arr2,1);
			char * pp = (char*)Array_getByIndex(arr2,2);
			if(strcmp(pp,"-")==0){
				printf("%d:%s\r\n",i,s2);
			}else{

				Array * pts = string_split(pt,",");
				Array * pps = string_split(pp,",");

				if(pts && pts->length>0)
				{
					int j=0;
					while(j<pts->length)
					{
						char * _pt = (char*)Array_getByIndex(pts,j);
						int _index = Dict_getWordIndex(dict,_pt);
						if(_index<0)
						{
							int len = strlen(_pt)+16;
							char explain[len];
							memset(explain,0,len);
							sprintf(explain,"pt of %s",verb);
							update_word("/home/libiao/dict.db",_pt,explain);
							printf("%s:%s\r\n",_pt,explain);
						}
						++j;
					}
				}

				if(pps && pps->length>0)
				{
					int j=0;
					while(j<pps->length)
					{
						char * _pp = (char*)Array_getByIndex(pps,j);
						int _index = Dict_getWordIndex(dict,_pp);
						if(_index<0)
						{
							int len = strlen(_pp)+16;
							char explain[len];
							memset(explain,0,len);
							if(Array_getIndexByStringValue(pts,_pp)>=0){
								sprintf(explain,"pt,pp of %s",verb);
							}else{
								sprintf(explain,"pp of %s",verb);
							}
							update_word("/home/libiao/dict.db",_pp,explain);
							printf("%s:%s\r\n",_pp,explain);
						}
						++j;
					}
				}
			}
			//Array * arr2 = string_split(s2," ");

		}
		++i;
	}
	return 0;
}
int startWith(Dict * dict,char * w)
{
	char * words[]={
		"re",
		"un",
		"super",
		"mis",
		"dis",
		"micro",
		NULL
	};
	int i=0;
	int _index;
	while(1)
	{
		char * word = words[i++];
		if(word==NULL)
			break;

		int k=strlen(word);
		if(k>0) 
		{
			if(strncmp(word,w,k)==0){
				_index = Dict_getWordIndex(dict,w+k);
				if(_index>0)
					return 1;
			}
		}
	}
	return 0;
}
int isOtherFormIn(Dict * dict,char * w)
{
	int len = strlen(w);
	char _w[len];
	int _index;
	memset(_w,0,len);
	if(regex_match(w,"/ing$/"))
	{
		snprintf(_w,len-2,"%s",w);
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
		//printf("%s\t\t%s ing\n",w,_w);
		if(_w[strlen(_w)-1]==(_w[strlen(_w)-2]))
		{
			_w[strlen(_w)-1]='\0';// del e
			_index = Dict_getWordIndex(dict,_w);
			if(_index>0)
				return 1;
		}
		_w[strlen(_w)]='e';//+ e
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;

		printf("%s,%s ing\n",w,_w);
	}else if(regex_match(w,"/est$/")){
		snprintf(_w,len-2,"%s",w);
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
		if(_w[strlen(_w)-1]==(_w[strlen(_w)-2]))
		{
			_w[strlen(_w)-1]='\0';// del e
			_index = Dict_getWordIndex(dict,_w);
			if(_index>0)
				return 1;
		}
		if(_w[strlen(_w)-1]=='i')
		{
			_w[strlen(_w)-1]='y';// i->y
			_index = Dict_getWordIndex(dict,_w);
			if(_index>0)
				return 1;
		}
		_w[strlen(_w)]='e';//+ e
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
		printf("%s,%s est\n",w,_w);
	}else if(regex_match(w,"/al$/") || regex_match(w,"/ly$/") || regex_match(w,"/ic$/") ){
		memset(_w,0,len);
		snprintf(_w,len,"%s",w);
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
		_w[len-1]='e';//+ e
		_w[len]='\0';//+ e
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
		//printf("%s,%s ly\n",w,_w);
		memset(_w,0,len);
		snprintf(_w,len-1,"%s",w);
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
		_w[strlen(_w)]='e';//+ e
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
		printf("%s,%s ly\n",w,_w);
	}else if(regex_match(w,"/er$/") || regex_match(w,"/or$/") || regex_match(w,"/ed$/")){
		snprintf(_w,len,"%s",w);
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
		snprintf(_w,len-1,"%s",w);
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
		if(_w[strlen(_w)-1]==(_w[strlen(_w)-2]))
		{
			_w[strlen(_w)-1]='\0';// del e
			_index = Dict_getWordIndex(dict,_w);
			if(_index>0)
				return 1;
		}
		if(_w[strlen(_w)-1]=='i')
		{
			_w[strlen(_w)-1]='y';// i->y
			_index = Dict_getWordIndex(dict,_w);
			if(_index>0)
				return 1;
		}
		_w[strlen(_w)]='e';//+ e
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
		printf("%s,%s er\n",w,_w);
	}else if(regex_match(w,"/s$/")){//s结尾
		memset(_w,0,len);
		snprintf(_w,len,"%s",w);
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
		if(regex_match(w,"/ves$/")){//ves结尾
			memset(_w,0,len);
			snprintf(_w,len-1,"%s",w);
			printf("%s,%s s\n",w,_w);
			_w[strlen(_w)-1]='f';
			_index = Dict_getWordIndex(dict,_w);
			if(_index>0)
				return 1;
			_w[strlen(_w)]='e';
			_index = Dict_getWordIndex(dict,_w);
			if(_index>0)
				return 1;
		}
		if(regex_match(w,"/ers$/")){//ves结尾
			memset(_w,0,len);
			snprintf(_w,len-1,"%s",w);
			printf("%s,%s s\n",w,_w);
			_index = Dict_getWordIndex(dict,_w);
			if(_index>0)
				return 1;
		}
		if(_w[strlen(_w)-1]=='e')
		{
			_w[strlen(_w)-1]='\0';// del e
			_index = Dict_getWordIndex(dict,_w);
			if(_index>0)
				return 1;
		}
		if(_w[strlen(_w)-1]=='i')
		{
			_w[strlen(_w)-1]='y';// i->y
			_index = Dict_getWordIndex(dict,_w);
			if(_index>0)
				return 1;
		}
		printf("%s,%s s\n",w,_w);
	}else if(regex_match(w,"/y$/")){//y结尾
		memset(_w,0,len);
		snprintf(_w,len,"%s",w);
		_index = Dict_getWordIndex(dict,_w);
		if(_index>0)
			return 1;
	}
	return 0;
}

int endwith(char * w)
{
	char * words[]={
		"speak",
		"wide",
		"side",
		"wise",
		"like",
		"ship",
		"less",
		"ress",
		"ness",
		"tion",
		"ism",
		"men",
		"tal",
		"able",
		"ian",
		"ed",
		"man",
		"ingly",
		"ments",
		"ment",
		"ful",
		NULL
	};
	int i=0;
	while(1)
	{
		char * word = words[i++];
		if(word==NULL)
			break;

		int k=strlen(w)-strlen(word);

		if(k>0) 
		{
			//printf("%s end with %s %d\n",w,w+k-1,k);
			if(strcmp(word,w+k)==0){
				//printf("%s end with %s %d\n",word,w+k,k);
				return 1;
			}
		}
	}
	return 0;
}

char * replace_word(char * w)
{
	char * words[][2]={
		/*
		   {"formingns","forming ns"},
		   {"uncountablens","uncountable ns"},
		   {"widespread","wide spread"},
		   {"achievementemphasize","achievement emphasize"},
		   {"orcould","or could"},
		   {"complainswhen","complains when"},
		   {"seenthis","seen this"},
		   {"atnight","at night"},
		   {"describeadequately","describe adequately"},
		   {"ringingchurch","ringing church"},
		   {"abellicose","a bellicose"},
		   {"mountainpeak","mountain peak"},
		   {"womanbenefactor","woman benefactor"},
		   {"instrumentwith","instrument with"},
		   {"investigatingthe","investigating the"},
		   {"hydrogenbomb","hydrogen bomb"},
		   {"newpapers","newspapers"},
		   {"compoundof","compound of"},
		   {"approvedschool","approved school"},
		   {"shoelaces","shoe laces"},
		   {"crowdedwith","crowded with"},
		   {"aggressivelymasculine","aggressively masculine"},
		   {"becomea","become a"},
		   {"permanentgroup","permanent group"},
		   {"faraway","far away"},
		   {"economicsystem","economic system"},
		   {"northbound","north bound"},
		   {"transformationalgrammar","transformational grammar"},
		   {"toexamine","to examine"},
		   {"aggressiveand","aggressive and"},
		   {"characteristicof","characteristic of"},
		   {"supposedpower","supposed power"},
		   {"inflammationof","inflammation of"},
		   {"grouprather","group rather"},
		   {"mewith","me with"},
		   {"entertainerwho","entertainer who"},
		   {"commitmentto","commitment to"},
		   {"aneighbourhood","a neighbourhood"},
		   {"andkeep","and keep"},
		   {"satisfactionwith","satisfaction with"},
		   {"comprehensiveschool","comprehensive school"},
		   {"alphabeticalindex","alphabetical index"},
		   {"conductingof","conducting of"},
		   {"isconfident","is confident"},
		   {"philosopherand","philosopher and"},
		   {"spreadingof","spreading of"},
		   {"urbanarea","urban area"},
		   {"litttle","little"},
		   {"opposeher","oppose her"},
		   {"pvovide","provide"},
		   {"upperpart","upper part"},
		   {"damnadv","damn adv"},
		   {"oldand","old and"},
		   {"makean","make an"},
		   {"adjthat","adj that"},
		   {"dismemberedbody","dismembered body"},
		   {"havebeen","have been"},
		   {"disproportionateamount","disproportionate amount"},
		   {"iswrong","is wrong"},
		   {"allegationshave","allegations have"},
		   {"manufacturersof","manufacturers of"},
		   {"drippingfrom","dripping from"},
		   {"regulardrops","regular drops"},
		   {"soundtrack","sound track"},
		   {"deadkings","dead kings"},
		   {"lowestrank","lowest rank"},
		   {"meatessences","meat essences"},
		   {"prosecutionthe","prosecution the"},
		   {"oralquestions","oral questions"},
		   {"forexcavating","for excavating"},
		{"goodthan","good than"},
		{"filmhas","film has"},
		{"typeface","type face"},
		{"causedby","caused by"},
		{"disguisedbottom","disguised bottom"},
		{"businesscompany","business company"},
		{"fitthe","fit the"},
		{"forthe","for the"},
		{"unemploymentis","unemployment is"},
		{"flagmake","flag make"},
		{"legalcareer","legal career"},
		{"programminglanguage","programming language"},
		{"ofkilling","of killing"},
		{"saltwater","salt water"},
		{"andcheese","and cheese"},
		{"intothe","into the"},
		{"onself","oneself"},
		{"fundamentaldifferences","fundamental differences"},
		{"varioustypes","various types"},
		{"theircharacter","their character"},
		{"gamblein","gamble in"},
		{"agenealogical","a genealogical"},
		{"generallowering","general lowering"},
		{"thoughtoo","though too"},
		{"thestage","the stage"},
		{"withoutflame","without flame"},
		{"greatgood","great good"},
		{"hishandsome","his handsome"},
		{"environmentand","environment and"},
		{"cyclerace","cycle race"},
		{"thinon","thin on"},
		{"localresidents","local residents"},
		{"inhabitingor","inhabiting or"},
		{"substanceto","substance to"},
		{"universitystudents","university students"},
		{"ofexcellence","of excellence"},
		{"somemoney","some money"},
		{"demonstratorhad","demonstrator had"},
		{"bestanding","be standing"},
		{"rootof","root of"},
		{"canalso","can also"},
		{"cocaineare","cocaine are"},
		{"causingharm","causing harm"},
		{"ofbuildings","of buildings"},
		{"combinationof","combination of"},
		{"neverthought","never thought"},
		{"orhabitual","or habitual"},
		{"withsharp","with sharp"},
		{"twovehicles","two vehicles"},
		{"mostimportant","most important"},
		{"egwhen","eg when"},
		{"becomehigher","become higher"},
		{"heighteningtension","heightening tension"},
		{"schooldays","school days"},
		{"arefighting","are fighting"},
		{"henceforward","hence forward"},
		{"inspeaking","in speaking"},
		*/
		{"justabove","just above"},
		{"forpeople","for people"},
		{"everyonein","everyone in"},
		{"sbmake","sb make"},
		{"sentimentaland","sentimental and"},
		{"eachother","each other"},
		{"soundproof","sound proof"},
		{"hopefullyadv","hopefully adv"},
		{"personwho","person who"},
		{"veryclosely","very closely"},
		{"mindwaiting","mind waiting"},
		{"ofgoods","of goods"},
		{"mountaineersfor","mountaineers for"},
		{"idealweather","ideal weather"},
		{"whatis","what is"},
		{"andtherefore","and therefore"},
		{"temporaryshelter","temporary shelter"},
		{"housebuilt","house built"},
		{"ignorantbehaviour","ignorant behaviour"},
		{"illiberalhelpings","illiberal helpings"},
		{"illiberalattitudes","illiberal attitudes"},
		{"ofsociety","of society"},
		{"servingas","serving as"},
		{"animmaculate","an immaculate"},
		{"abroken","a broken"},
		{"immoralbehaviour","immoral behaviour"},
		{"impactedwisdom","impacted wisdom"},
		{"showingstrong","showing strong"},
		{"impassionedplea","impassioned plea"},
		{"extendinga","extending a"},
		{"thingnot","thing not"},
		{"doingpractical","doing practical"},
		{"theline","the line"},
		{"improvementin","improvement in"},
		{"substancepresent","substance present"},
		{"orquality","or quality"},
		{"attributesth","attribute sth"},
		{"extremis","extrem is"},
		{"carriedout","carried out"},
		{"cannotbe","cannot be"},
		{"havingsigns","having signs"},
		{"betweenpeople","between people"},
		{"nothaving","not having"},
		{"areincomprehensible","are incomprehensible"},
		{"shirttails","shirt tails"},
		{"adjnot","adj not"},
		{"withannual","with annual"},
		{"indemnitiesfrom","indemnities from"},
		{"compensationfor","compensation for"},
		{"womanof","woman of"},
		{"anyoneelse","anyone else"},
		{"indigenousto","indigenous to"},
		{"productionof","production of"},
		{"industrialdiamonds","industrial diamonds"},
		{"undesirableideas","undesirable ideas"},
		{"crimeof","crime of"},
		{"sbfeel","sb feel"},
		{"aninfernal","an infernal"},
		{"extremelysmall","extremely small"},
		{"belowthe","below the"},
		{"betweencountries","between countries"},
		{"injudiciousremarks","injudicious remarks"},
		{"insatiablefor","insatiable for"},
		{"tombstonewith","tombstone with"},
		{"advertisementput","advertisement put"},
		{"explanationof","explanation of"},
		{"becauseof","because of"},
		{"installationof","installation of"},
		{"wholenumber","whole number"},
		{"mattersof","matters of"},
		{"interactionbetween","interaction between"},
		{"synonymsare","synonyms are"},
		{"communicationby","communication by"},
		{"interimduring","interim during"},
		{"existingor","existing or"},
		{"inquestions","in questions"},
		{"intolerantof","intolerant of"},
		{"freefrom","free from"},
		{"contractingand","contracting and"},
		{"leavesth","leave sth"},
		{"mischievouschild","mischievous child"},
		{"iecover","ie cover"},
		{"partof","part of"},
		{"religiousorder","religious order"},
		{"adiamond","a diamond"},
		{"jokerin","joker in"},
		{"judgementwas","judgement was"},
		{"havinggood","having good"},
		{"startermotor","starter motor"},
		{"ofugly","of ugly"},
		{"theirteachers","their teachers"},
		{"huntersbrought","hunters brought"},
		{"enjoymentof","enjoyment of"},
		{"sbsome","sb some"},
		{"bookshelves","book shelves"},
		{"twistedpiece","twisted piece"},
		{"isvery","is very"},
		{"fromthe","from the"},
		{"ahouse","a house"},
		{"percolatethrough","percolate through"},
		{"beenleft","been left"},
		{"sandhills","sand hills"},
		{"legsoff","legs off"},
		{"newborn","new born"},
		{"touristsduring","tourists during"},
		{"communicationand","communication and"},
		{"beingtolerant","being tolerant"},
		{"lifestyle","life style"},
		{"disregardingthe","disregarding the"},
		{"preparationfor","preparation for"},
		{"whitesubstance","white substance"},
		{"thesoldiers","the soldiers"},
		{"touchline","touch line"},
		{"stillbirth","still birth"},
		{"pricesare","prices are"},
		{"correspondingshort","corresponding short"},
		{"fondof","fond of"},
		{"substancethat","substance that"},
		{"ludicrousidea","ludicrous idea"},
		{"ofvitality","of vitality"},
		{"youngstersat","youngsters at"},
		{"songfor","song for"},
		{"apparentlyusing","apparently using"},
		{"magnifyingobjects","magnifying objects"},
		{"mainlinedheroin","mainlined heroin"},
		{"principalsail","principal sail"},
		{"foremast","fore mast"},
		{"sweetcorn","sweet corn"},
		{"precedingpossess","preceding possess"},
		{"spokesmanby","spokesman by"},
		{"compensatesb","compensate sb"},
		{"mineralthat","mineral that"},
		{"nursemaid","nurse maid"},
		{"womanwho","woman who"},
		{"conversationround","conversation round"},
		{"manureinto","manure into"},
		{"aboriginalrace","aboriginal race"},
		{"marginallybigger","marginally bigger"},
		{"withleisure","with leisure"},
		{"ahusband","a husband"},
		{"distinguishingfeature","distinguishing feature"},
		{"overshoot","over shoot"},
		{"regularlyheld","regularly held"},
		{"newproduct","new product"},
		{"commercialselling","commercial selling"},
		{"piecesof","pieces of"},
		{"swallowfamily","swallow family"},
		{"showedcomplete","showed complete"},
		{"demonstratea","demonstrate a"},
		{"amaterialistic","a materialistic"},
		{"materiallydifferent","materially different"},
		{"formsthe","forms the"},
		{"arrangementof","arrangement of"},
		{"halfway","half way"},
		{"birthplaceof","birthplace of"},
		{"governmentscheme","government scheme"},
		{"compromisewith","compromise with"},
		{"producingpleasant","producing pleasant"},
		{"remembercorrectly","remember correctly"},
		{"photographicfilm","photographic film"},
		{"coalmines","coal mines"},
		{"instrumentwith","instrument with"},
		{"eardrum","ear drum"},
		{"finalbuyer","final buyer"},
		{"chargeof","charge of"},
		{"youwere","you were"},
		{"amongpeople","among people"},
		{"makefun","make fun"},
		{"chemicalcompound","chemical compound"},
		{"lastingvalue","lasting value"},
		{"liketo","like to"},
		{"introducesth","introduce sth"},
		{"publiclibrary","public library"},
		{"whohas","who has"},
		{"whichobjects","which objects"},
		{"givethe","give the"},
		{"importantaspect","important aspect"},
		{"thename","the name"},
		{"throughthe","through the"},
		{"readyto","ready to"},
		{"themiddle","the middle"},
		{"attachedat","attached at"},
		{"nearhas","near has"},
		{"ofwriting","of writing"},
		{"crossingeach","crossing each"},
		{"newsreader","news reader"},
		{"detailedthought","detailed thought"},
		{"referenceto","reference to"},
		{"latenights","late nights"},
		{"duringthe","during the"},
		{"unpleasantexperience","unpleasant experience"},
		{"nperson","n person"},
		{"northbound","north bound"},
		{"formthe","form the"},
		{"figurerepresenting","figure representing"},
		{"obeisanceto","obeisance to"},
		{"occurrenceand","occurrence and"},
		{"widespread","wide spread"},
		{"swimsuit","swim suit"},
		{"tomany","to many"},
		{"operaseason","opera season"},
		{"anopera","an opera"},
		{"appearingopposite","appearing opposite"},
		{"conditionof","condition of"},
		{"outclassedfrom","outclassed from"},
		{"manufactureof","manufacture of"},
		{"heatproof","heat proof"},
		{"amountof","amount of"},
		{"themajority","the majority"},
		{"liveson","lives on"},
		{"compoundof","compound of"},
		{"varioustypes","various types"},
		{"completelyfill","completely fill"},
		{"ofancient","of ancient"},
		{"withoutremoving","without removing"},
		{"palpitationsif","palpitations if"},
		{"paralysisof","paralysis of"},
		{"currencieshave","currencies have"},
		{"aparsimonious","a parsimonious"},
		{"brandyafter","brandy after"},
		{"consistingof","consisting of"},
		{"knowabout","know about"},
		{"happenedin","happened in"},
		{"relatingto","relating to"},
		{"beinga","being a"},
		{"deniedpaternity","denied paternity"},
		{"agriculturalworker","agricultural worker"},
		{"apedagogically","a pedagogically"},
		{"practisespederasty","practises pederasty"},
		{"drawnwith","drawn with"},
		{"understandquickly","understand quickly"},
		{"federalor","federal or"},
		{"likepepper","like pepper"},
		{"walkaround","walk around"},
		{"perambulateafter","perambulate after"},
		{"aperennial","a perennial"},
		{"freedomof","freedom of"},
		{"arefining","a refining"},
		{"specializesin","specializes in"},
		{"philistinismof","philistinism of"},
		{"studyof","study of"},
		{"philosophizingabout","philosophizing about"},
		{"commentsfrom","comments from"},
		{"televisionprogramme","television programme"},
		{"phonologyof","phonology of"},
		{"heavyweightboxing","heavyweight boxing"},
		{"bitteryellow","bitter yellow"},
		{"backgroundof","background of"},
		{"messagesor","messages or"},
		{"artificialhormone","artificial hormone"},
		{"whichthese","which these"},
		{"whichthis","which this"},
		{"deservingpity","deserving pity"},
		{"dependon","depend on"},
		{"plannern","planner n"},
		{"atraffic","a traffic"},
		{"surfacesuch","surface such"},
		{"representingthe","representing the"},
		{"misleadingevidence","misleading evidence"},
		{"manufacturingprocess","manufacturing process"},
		{"bloodtransfusions","blood transfusions"},
		{"achievementsjust","achievements just"},
		{"modernizationof","modernization of"},
		{"politicalprisoners","political prisoners"},
		{"amabbr","am abbr"},
		{"twofigures","two figures"},
		{"speechfor","speech for"},
		{"deliberatelyto","deliberately to"},
		{"racialsuperiority","racial superiority"},
		{"fellowwho","fellow who"},
		{"chiefparty","chief party"},
		{"inlarge","in large"},
		{"orcontempt","or contempt"},
		{"poppeddown","popped down"},
		{"populationresulting","population resulting"},
		{"permanentlyin","permanently in"},
		{"poseuse","pose use"},
		{"possessiveparents","possessive parents"},
		{"etcmust","etc must"},
		{"adecision","a decision"},
		{"expressingviews","expressing views"},
		{"havinggreat","having great"},
		{"earthenwarepots","earthenware pots"},
		{"aleisurely","a leisurely"},
		{"prisonerof","prisoner of"},
		{"holdinggunpowder","holding gunpowder"},
		{"potentiallydangerous","potentially dangerous"},
		{"thanideas","than ideas"},
		{"inscribedwith","inscribed with"},
		{"positionswhen","positions when"},
		{"practisewhat","practise what"},
		{"precipitatedas","precipitated as"},
		{"thatfalls","that falls"},
		{"andaccurately","and accurately"},
		{"smallgroup","small group"},
		{"wanteda","wanted a"},
		{"theprejudice","the prejudice"},
		{"developmentsprejudicial","developments prejudicial"},
		{"physiologicalupset","physiological upset"},
		{"makinga","making a"},
		{"speculationin","speculation in"},
		{"todinner","to dinner"},
		{"gramophonerecord","gramophone record"},
		{"orwidespread","or widespread"},
		{"intendedto","intended to"},
		{"independencewas","independence was"},
		{"repeatedlyreduce","repeatedly reduce"},
		{"governmentcomplacency","government complacency"},
		{"disapprovesof","disapproves of"},
		{"manufacturingsth","manufacturing sth"},
		{"gunpowderin","gunpowder in"},
		{"smallcontainer","small container"},
		{"themselveson","themselves on"},
		{"annoyinglyprecise","annoyingly precise"},
		{"opposedto","opposed to"},
		{"enjoymentand","enjoyment and"},
		{"whichcannot","which cannot"},
		{"suggestedsolutions","suggested solutions"},
		{"iewith","ie with"},
		{"proceedingalong","proceeding along"},
		{"rustproofing","rust proofing"},
		{"productivehour","productive hour"},
		{"lawyerby","lawyer by"},
		{"specifiedannoying","specified annoying"},
		{"madeprofessor","made professor"},
		{"proficientat","proficient at"},
		{"theuse","the use"},
		{"biographyof","biography of"},
		{"toweragainst","tower against"},
		{"accountshowing","account showing"},
		{"profitableafternoon","profitable afternoon"},
		{"maketoo","make too"},
		{"verygreat","very great"},
		{"profoundthinker","profound thinker"},
		{"progenitorof","progenitor of"},
		{"instructionsto","instructions to"},
		{"spreadof","spread of"},
		{"organizationworks","organization works"},
		{"enthusiasticpromoter","enthusiastic promoter"},
		{"sthsupporter","sth supporter"},
		{"particularproduct","particular product"},
		{"promotionaltour","promotional tour"},
		{"childis","child is"},
		{"consideredopinion","considered opinion"},
		{"bypropaganda","by propaganda"},
		{"continuedin","continued in"},
		{"acriminal","a criminal"},
		{"alanguage","a language"},
		{"decisionswhich","decisions which"},
		{"provisionmerchant","provision merchant"},
		{"psychedelicmusic","psychedelic music"},
		{"antisocialway","antisocial way"},
		{"psychosomaticmedicine","psychosomatic medicine"},
		{"ofpresenting","of presenting"},
		{"professionalboxing","professional boxing"},
		{"professionalboxer","professional boxer"},
		{"tomove","to move"},
		{"countingthe","counting the"},
		{"informationis","information is"},
		{"andimitates","and imitates"},
		{"puritanicalzeal","puritanical zeal"},
		{"awardedto","awarded to"},
		{"repeatedand","repeated and"},
		{"containingtwo","containing two"},
		{"beginningto","beginning to"},
		{"putrescentcorpse","putrescent corpse"},
		{"governmentby","government by"},
		{"miniaturegolf","miniature golf"},
		{"grassfor","grass for"},
		{"questionthat","question that"},
		{"characterof","character of"},
		{"figurewith","figure with"},
		{"fourthyear","fourth year"},
		{"timesas","times as"},
		{"ortone","or tone"},
		{"mainpoint","main point"},
		{"generallyindicate","generally indicate"},
		{"telephonefor","telephone for"},
		{"ofbawdy","of bawdy"},
		{"radiofor","radio for"},
		{"harmfulelectrical","harmful electrical"},
		{"canpenetrate","can penetrate"},
		{"orsideways","or sideways"},
		{"recoveryof","recovery of"},
		{"rampiles","ram piles"},
		{"esppassive","esp passive"},
		{"storedpreviously","stored previously"},
		{"practiceof","practice of"},
		{"theexpression","the expression"},
		{"producedby","produced by"},
		{"interestingto","interesting to"},
		{"readand","read and"},
		{"navalofficer","naval officer"},
		{"economicactivity","economic activity"},
		{"commitscrimes","commits crimes"},
		{"usedin","used in"},
		{"geometricfigure","geometric figure"},
		{"positionsor","positions or"},
		{"bywire","by wire"},
		{"viewfinder","view finder"},
		{"armedresistance","armed resistance"},
		{"resistanceto","resistance to"},
		{"absorptionof","absorption of"},
		{"apparatusfor","apparatus for"},
		{"internationalcricket","international cricket"},
		{"egteachers","eg teachers"},
		{"developmentsin","developments in"},
		{"characteristicfeatures","characteristic features"},
		{"withdrawalfrom","withdrawal from"},
		{"revolutionof","revolution of"},
		{"deliberatelyinflicting","deliberately inflicting"},
		{"revolutionaryconsequences","revolutionary consequences"},
		{"lampposts","lamp posts"},
		{"toldin","told in"},
		{"rhythmictread","rhythmic tread"},
		{"ofmeat","of meat"},
		{"embarrassmentof","embarrassment of"},
		{"ormore","or more"},
		{"throughwoods","through woods"},
		{"ridewith","ride with"},
		{"inderogatory","in derogatory"},
		{"stocktaking","stock taking"},
		{"avenueis","avenue is"},
		{"actionsof","actions of"},
		{"decorationin","decoration in"},
		{"merrymaking","merry making"},
		{"relationshipcan","relationship can"},
		{"relationsand","relations and"},
		{"newsflash","news flash"},
		{"inquality","in quality"},
		{"containerfor","container for"},
		{"detailedreports","detailed reports"},
		{"crowdon","crowd on"},
		{"representationin","representation in"},
		{"stoppingfor","stopping for"},
		{"resurfacingwork","resurfacing work"},
		{"newsurface","new surface"},
		{"responsibilityfor","responsibility for"},
		{"pointdo","point do"},
		{"ablackboard","a blackboard"},
		{"otherauthority","other authority"},
		{"schoolfriend","school friend"},
		{"numberand","number and"},
		{"separatelyfor","separately for"},
		{"ofwater","of water"},
		{"wantto","want to"},
		{"headquartersof","headquarters of"},
		{"andwith","and with"},
		{"schoolwork","school work"},
		{"piercingsound","piercing sound"},
		{"paperdisc","paper disc"},
		{"clothingmaterial","clothing material"},
		{"thefirst","the first"},
		{"handicappedbe","handicapped be"},
		{"needto","need to"},
		{"franknessand","frankness and"},
		{"etcseem","etc seem"},
		{"cosabbr","cos abbr"},
		{"worseningdiplomatic","worsening diplomatic"},
		{"adifficult","a difficult"},
		{"sixfoldincrease","sixfold increase"},
		{"supportingstructure","supporting structure"},
		{"contemptibleperson","contemptible person"},
		{"slovenlyperson","slovenly person"},
		{"pieceof","piece of"},
		{"overhand","over hand"},
		{"accusationof","accusation of"},
		{"accusationthat","accusation that"},
		{"smokescreen","smoke screen"},
		{"cigarettesmouldering","cigarettes mouldering"},
		{"smugglersin","smugglers in"},
		{"materialthat","material that"},
		{"candlelight","candle light"},
		{"skyward","sky ward"},
		{"forfood","for food"},
		{"slaveringover","slavering over"},
		{"seemingnervous","seeming nervous"},
		{"workingindependently","working independently"},
		{"containinga","containing a"},
		{"advertisementread","advertisement read"},
		{"nopassive","no passive"},
		{"excitementhas","excitement has"},
		{"relationswith","relations with"},
		{"lunchtime","lunch time"},
		{"wearingsome","wearing some"},
		{"nationalemblem","national emblem"},
		{"chainsaw","chain saw"},
		//{"brassware","brassware"},
		{"reproductionin","reproduction in"},
		{"representativen","representative n"},
		{"patchworkquilt","patchwork quilt"},
		{"alarger","a larger"},
		{"ierising","ie rising"},
		{"concerningrank","concerning rank"},
		{"solublecompound","soluble compound"},
		{"asolemn","a solemn"},
		{"othersingular","other singular"},
		{"thinkof","think of"},
		{"birdsong","bird song"},
		{"soporificspeech","soporific speech"},
		{"southerlybreezes","southerly breezes"},
		{"knowledgespans","knowledge spans"},
		{"stretchacross","stretch across"},
		{"praisinghim","praising him"},
		{"producesparks","produces parks"},
		{"coveredwith","covered with"},
		{"correctlyfrom","correctly from"},
		{"mushroomsand","mushrooms and"},
		{"spendours","spend ours"},
		{"didnot","did not"},
		{"havingthe","having the"},
		{"othersto","others to"},
		{"tomind","to mind"},
		{"employedby","employed by"},
		{"specialservice","special service"},
		{"closeto","close to"},
		{"containingminerals","containing minerals"},
		{"thathas","that has"},
		{"rainwater","rain water"},
		{"etcwhere","etc where"},
		{"mechanismfor","mechanism for"},
		{"standardizedin","standardized in"},
		{"constructedor","constructed or"},
		{"stockand","stock and"},
		{"apparentlycalm","apparently calm"},
		{"announcementof","announcement of"},
		{"blackand","black and"},
		{"fairin","fair in"},
		{"unpleasanttension","unpleasant tension"},
		{"stridesettle","stride settle"},
		{"onsmoking","on smoking"},
		{"instrumentthat","instrument that"},
		{"stumbledacross","stumbled across"},
		{"andcorrect","and correct"},
		{"thingthat","thing that"},
		{"subjunctivemood","subjunctive mood"},
		{"perceivedor","perceived or"},
		{"calleris","caller is"},
		{"besold","be sold"},
		{"concerningthe","concerning the"},
		{"whichrelays","which relays"},
		{"underthe","under the"},
		{"managingpart","managing part"},
		{"appropriatefor","appropriate for"},
		{"acceptinga","accepting a"},
		{"summaryof","summary of"},
		{"themain","the main"},
		{"summerwe","summer we"},
		{"summeryday","summery day"},
		{"sendinga","sending a"},
		{"friendlyway","friendly way"},
		{"manwith","man with"},
		{"asuppurating","a suppurating"},
		{"unstoppablyby","unstoppably by"},
		{"becomelarger","become larger"},
		{"pleasantlyand","pleasantly and"},
		{"switchboardoperators","switchboard operators"},
		{"ofwritten","of written"},
		{"withpages","with pages"},
		{"displayedin","displayed in"},
		{"understoodwithout","understood without"},
		{"tailtake","tail take"},
		{"tookhim","took him"},
		{"reporterstook","reporters took"},
		{"talltame","tall tame"},
		{"uncountablenoun","uncountable noun"},
		{"circularframe","circular frame"},
		{"fightwith","fight with"},
		{"orother","or other"},
		{"stripof","strip of"},
		{"publicpurposes","public purposes"},
		{"knowledgeand","knowledge and"},
		{"theoverall","the overall"},
		{"thegood","the good"},
		{"projectwas","project was"},
		{"combinedeffort","combined effort"},
		{"tearingdown","tearing down"},
		{"pullingsharply","pulling sharply"},
		{"impetuousand","impetuous and"},
		{"questionsor","questions or"},
		{"thetechnical","the technical"},
		{"itstechniques","its techniques"},
		{"thetechnique","the technique"},
		{"theworst","the worst"},
		{"withmusic","with music"},
		{"smallprojections","small projections"},
		{"technologyof","technology of"},
		{"scientificstudy","scientific study"},
		{"teeteredon","teetered on"},
		{"shortenedstyle","shortened style"},
		{"receivean","receive an"},
		{"deviceby","device by"},
		{"becomeshorter","become shorter"},
		{"unacceptableand","unacceptable and"},
		{"noticeableeffect","noticeable effect"},
		{"paysout","pays out"},
		{"accordingto","according to"},
		{"notpermanent","not permanent"},
		{"certainway","certain way"},
		{"climbingplant","climbing plant"},
		{"appropriateand","appropriate and"},
		{"orbark","or bark"},
		{"computingsystem","computing system"},
		{"forcommunicating","for communicating"},
		{"terminalcancer","terminal cancer"},
		{"asnames","as names"},
		{"terminaln","terminal n"},
		{"expressedas","expressed as"},
		{"territorialclaims","territorial claims"},
		{"ofviolence","of violence"},
		{"participatesin","participates in"},
		{"weaponsunder","weapons under"},
		{"outcomeof","outcome of"},
		{"sedativedrug","sedative drug"},
		{"whiskyin","whisky in"},
		{"leadingthe","leading the"},
		{"tellingthe","telling the"},
		{"findthe","find the"},
		{"etcthat","etc that"},
		{"ordinarypeople","ordinary people"},
		{"discussionwas","discussion was"},
		{"aparticular","a particular"},
		{"speechtherapist","speech therapist"},
		{"thereabout","there about"},
		{"inthat","in that"},
		{"possessionstherein","possessions therein"},
		{"ofheat","of heat"},
		{"branchof","branch of"},
		{"differentmetals","different metals"},
		{"thermoelectricvoltage","thermoelectric voltage"},
		{"sentenceswhen","sentences when"},
		{"supposedbut","supposed but"},
		{"instrumentfor","instrument for"},
		{"orrestoring","or restoring"},
		{"subjectof","subject of"},
		{"rapidlyand","rapidly and"},
		{"thingabout","thing about"},
		{"sthconsider","sth consider"},
		{"havethought","have thought"},
		{"expertsproviding","experts providing"},
		{"spentseveral","spent several"},
		{"absorbedin","absorbed in"},
		{"examplesat","examples at"},
		{"threada","thread a"},
		{"etcthrough","etc through"},
		{"nervoustremor","nervous tremor"},
		{"telephonecall","telephone call"},
		{"youbut","you but"},
		{"similardisease","similar disease"},
		{"inchildren","in children"},
		{"tryingto","trying to"},
		{"varioussections","various sections"},
		{"theedge","the edge"},
		{"announcinghis","announcing his"},
		{"stockmarket","stock market"},
		{"youborrow","you borrow"},
		{"driftwood","drift wood"},
		{"unwashedparts","unwashed parts"},
		{"atheatre","a theatre"},
		{"beforea","before a"},
		{"allowslittle","allows little"},
		{"characteristicquality","characteristic quality"},
		{"certainlytook","certainly took"},
		{"beunreasonably","be unreasonably"},
		{"suitablefor","suitable for"},
		{"performersand","performers and"},
		{"wholeplace","whole place"},
		{"hischeek","his cheek"},
		{"anadvance","an advance"},
		{"memberof","member of"},
		{"completelyat","completely at"},
		{"shopkeeperwho","shopkeeper who"},
		{"thiscontract","this contract"},
		{"reallytogether","really together"},
		{"forsomeone","for someone"},
		{"articlesor","articles or"},
		{"writtento","written to"},
		{"answerin","answer in"},
		{"carryingcargo","carrying cargo"},
		{"inflammationof","inflammation of"},
		{"shavedin","shaved in"},
		{"havingteeth","having teeth"},
		{"excessivedegree","excessive degree"},
		{"ingreat","in great"},
		{"tottedup","totted up"},
		{"touchof","touch of"},
		{"touchsth","touch sth"},
		{"provincesnext","provinces next"},
		{"purposeof","purpose of"},
		{"briefvisit","brief visit"},
		{"commercialarea","commercial area"},
		{"developmentof","development of"},
		{"recordingtape","recording tape"},
		{"sectionof","section of"},
		{"muddytrack","muddy track"},
		{"carpenterby","carpenter by"},
		{"alawyer","a lawyer"},
		{"additionallyof","additionally of"},
		{"agroup","a group"},
		{"alarge","a large"},
		{"saydamaging","say damaging"},
		{"scandalousaffair","scandalous affair"},
		{"dividingtwo","dividing two"},
		{"instructionand","instruction and"},
		{"tramcar","tram car"},
		{"transactionson","transactions on"},
		{"strongfabric","strong fabric"},
		{"meditationand","meditation and"},
		{"pianopiece","piano piece"},
		{"transferredfrom","transferred from"},
		{"interestingpiece","interesting piece"},
		{"accommodationfor","accommodation for"},
		{"holdingpower","holding power"},
		{"transitionalstage","transitional stage"},
		{"translateour","translate our"},
		{"lavatorywindows","lavatory windows"},
		{"allowinglight","allowing light"},
		{"intoanother","into another"},
		{"transmittingradio","transmitting radio"},
		{"appearancedoes","appearance does"},
		{"consecrationinto","consecration into"},
		{"nosides","no sides"},
		{"representativeof","representative of"},
		{"extendacross","extend across"},
		{"somebreakfast","some breakfast"},
		{"betrayinga","betraying a"},
		{"setdown","set down"},
		{"greattreat","great treat"},
		{"philosophicaldoubt","philosophical doubt"},
		{"tremblingfrom","trembling from"},
		{"workmendug","workmen dug"},
		{"oneswho","ones who"},
		{"trendyclothes","trendy clothes"},
		{"withthree","with three"},
		{"behaviourand","behaviour and"},
		{"ofbeing","of being"},
		{"todecorate","to decorate"},
		{"timesthe","times the"},
		{"winningteam","winning team"},
		{"defiantand","defiant and"},
		{"withno","with no"},
		{"frameworksupporting","framework supporting"},
		{"personsuffering","person suffering"},
		{"notsuspicious","not suspicious"},
		{"responseto","response to"},
		{"needsa","needs a"},
		{"resemblingthis","resembling this"},
		{"seawater","sea water"},
		{"roundsharply","round sharply"},
		{"middleear","middle ear"},
		{"winesof","wines of"},
		{"managementof","management of"},
		{"arepresentative","are presentative"},
		{"unidentifiedflying","unidentified flying"},
		{"consistsof","consists of"},
		{"withan","with an"},
		{"wavelengththat","wavelength that"},
		{"leavingmy","leaving my"},
		{"againstwhat","against what"},
		{"unbrokensleep","unbroken sleep"},
		{"interruptedor","interrupted or"},
		{"talkingabout","talking about"},
		{"unbuttonedstyle","unbuttoned style"},
		{"researchis","research is"},
		{"resistedor","resisted or"},
		{"insincerelyearnest","insincerely earnest"},
		{"tookus","took us"},
		{"notenough","not enough"},
		{"undersurface","under surface"},
		{"theirsupports","their supports"},
		{"underclothing","under clothing"},
		{"normallyamong","normally among"},
		{"currentof","current of"},
		{"toolittle","too little"},
		{"undergroundrailway","underground railway"},
		{"littleimportance","little importance"},
		{"toolow","too low"},
		{"continuoustenses","continuous tenses"},
		{"ppunderstood","pp understood"},
		{"beunderstood","be understood"},
		{"understandingon","understanding on"},
		{"ofwrecks","of wrecks"},
		{"paintingis","painting is"},
		{"temporarilywithout","temporarily without"},
		{"betweengood","between good"},
		{"difficultto","difficult to"},
		{"havingor","having or"},
		{"hopedfor","hoped for"},
		{"resemblinga","resembling a"},
		{"awardingdegrees","awarding degrees"},
		{"examinesstudents","examines students"},
		{"againstthe","against the"},
		{"madewithout","made without"},
		{"authorizedby","authorized by"},
		{"unorthodoxteaching","unorthodox teaching"},
		{"behaviourin","behaviour in"},
		{"unprepossessingto","unprepossessing to"},
		{"unquestionedfact","unquestioned fact"},
		{"mysteryunravels","mystery unravels"},
		{"becomeclear","become clear"},
		{"alwayskeep","always keep"},
		{"newsurroundings","new surroundings"},
		{"requiringspecial","requiring special"},
		{"unsparingin","unsparing in"},
		{"feelingno","feeling no"},
		{"unveilnew","unveil new"},
		{"publiclyfor","publicly for"},
		{"inclinedto","inclined to"},
		{"unworthycause","unworthy cause"},
		{"leftwithout","left without"},
		{"higherin","higher in"},
		{"directionfrom","direction from"},
		{"utilizethe","utilize the"},
		{"ofoccupants","of occupants"},
		{"forkeeping","for keeping"},
		{"electriccharge","electric charge"},
		{"capacityof","capacity of"},
		{"validityof","validity of"},
		{"beinglegally","being legally"},
		{"displaygreat","display great"},
		{"jawbone","jaw bone"},
		{"nutritionalvalue","nutritional value"},
		{"bloodsucking","blood sucking"},
		{"variablein","variable in"},
		{"asingle","a single"},
		{"quantitythat","quantity that"},
		{"movewith","move with"},
		{"machinefor","machine for"},
		{"speechverbatim","speech verbatim"},
		{"answeredby","answered by"},
		{"sentencesin","sentences in"},
		{"advantageof","advantage of"},
		{"battlingagainst","battling against"},
		{"thatis","that is"},
		{"somemembers","some members"},
		{"victuala","victual a"},
		{"thisanimal","this animal"},
		{"produceda","produced a"},
		{"viewpoint","view point"},
		{"interpretinga","interpreting a"},
		{"amongthe","among the"},
		{"smallestunit","smallest unit"},
		{"morevim","more vim"},
		{"violatingor","violating or"},
		{"noblemanranking","nobleman ranking"},
		{"unusualbeauty","unusual beauty"},
		{"numberof","number of"},
		{"weakenthe","weaken the"},
		{"advantagesof","advantages of"},
		{"fillingmany","filling many"},
		{"playedon","played on"},
		{"prisonermade","prisoner made"},
		{"orpractice","or practice"},
		{"itstronger","it stronger"},
		{"orobscene","or obscene"},
		{"obsceneword","obscene word"},
		{"hourstime","hours time"},
		{"wetland","wet land"},
		{"groundhog","ground hog"},
		{"paperstrong","paper strong"},
		{"wreckern","wrecker n"},
		{"colourlessand","colourless and"},
		{"thereproductive","the reproductive"},
		{"vehiclewith","vehicle with"},
		{"unsteadilyoff","unsteadily off"},
		{"throughouta","throughout a"},
		{"conferenceheld","conference held"},
		{"theaffirmative","the affirmative"},
		{"acarton","a carton"},
		{"flavouredwith","flavoured with"},
		{"burdensomerestraint","burdensome restraint"},
		{"careand","care and"},
		{"faithfullyto","faithfully to"},
		{"localauthority","local authority"},
		{"childrenin","children in"},
		{"growthin","growth in"},
		{"orrelating","or relating"},
		{"spreadadj","spread adj"},
		{"wayto","way to"},
		{"begat","beg at"},
		{"betweenthe","between the"},
		{"striptease","stript ease"},
		{"ieone","ie one"},
		{"acton","act on"},
		{"inthe","in the"},
		{"liftn","lift n"},
		{"amost","a most"},
		{"lotof","lot of"},
		{"putin","put in"},
		{"areof","are of"},
		{"tothe","to the"},
		{"jobof","job of"},
		{"toend","to end"},
		{"makea","make a"},
		{"onthe","on the"},
		{"atthe","at the"},
		{"ofthe","of the"},
		{"wayof","way of"},
		{"agood","a good"},
		{"toeach","to each"},
		{"beput","be put"},
		{"aword","a word"},
		{"showa","show a"},
		{"rooma","room a"},
		{"whilehallucinating","while hallucinating"},
		{"garmentworn","garment worn"},
		{"aredifficult","are difficult"},
		{"nowbe","now be"},
		{"signboard","sign board"},
		{"wayhe","way he"},
		{"orfor","or for"},
		{"tryto","try to"},
		{"cooka","cook a"},
		{"atrue","a true"},
		{"onhorseback","on horseback"},
		{"uncountablens","uncountable ns"},
		{"anddangerous","and dangerous"},
		{"isapproaching","is approaching"},
		{"togo","to go"},
		{"intercoursewith","intercourse with"},
		{"agas","a gas"},
		{"iein","ie in"},
		{"nota","no ta"},
		{"oron","or on"},
		{"ifhe","if he"},
		{"bythe","by the"},
		{"ieto","ie to"},
		{"tobe","to be"},
		{"meby","me by"},
		{"wassoft","was soft"},
		{"listsof","lists of"},
		{"startedout","started out"},
		{"refusingto","refusing to"},
		{"orvigour","or vigour"},
		{"tocommunicate","to communicate"},
		{"conclusionthat","conclusion that"},
		{"disillusionmentwith","disillusionment with"},
		{"almondsor","almonds or"},
		{"ofexistence","of existence"},
		{"watersof","waters of"},
		{"infectiousdisease","infectious disease"},
		{"precipitationof","precipitation of"},
		{"inprinciple","in principle"},
		{"andend","and end"},
		{"richesof","riches of"},
		{"consequencesof","consequences of"},
		{"soundtrack","sound track"},
		{"alignedwith","aligned with"},
		{"designedto","designed to"},
		{"roughcountry","rough country"},
		{"floodwaters","flood waters"},
		{"sthcompletely","sth completely"},
		{"irregularlywith","irregularly with"},
		{"secondin","second in"},
		{"workand","work and"},
		{"educationaland","educational and"},
		{"oractivity","or activity"},
		{"thoughtabout","thought about"},
		{"growthrough","grow through"},
		{"cleverbut","clever but"},
		{"divisionof","division of"},
		{"behavesensibly","behave sensibly"},
		{"socialconventions","social conventions"},
		{"apension","a pension"},
		{"byphotography","by photography"},
		{"isnot","is not"},
		{"positon","posit on"},
		{"othersin","others in"},
		{"twopieces","two pieces"},
		{"bysb","by sb"},
		{"tosb","to sb"},
		{"lightand","light and"},
		{"accidentson","accidents on"},
		{"byindependent","by independent"},
		{"bumpingis","bumping is"},
		{"occasionsof","occasions of"},
		{"withoutthe","without the"},
		{"plainsong","plain song"},
		{"arrangedclosely","arranged closely"},
		{"arrangementsfor","arrangements for"},
		{"fora","for a"},
		{"unacceptablein","unacceptable in"},
		{"effecton","effect on"},
		{"fieldsthat","fields that"},
		{"toan","to an"},
		{"wherethe","where the"},
		{"overtiring","over tiring"},

		{NULL,NULL}
	};
	int i=0;
	while(1)
	{
		char * word = words[i][0];
		char * rword = words[i][1];
		++i;
		if(word==NULL)
			break;

		if(strcasecmp(word,w)==0)
		{
			printf("%s-->%s\n",word,rword);
			return rword;
		}
	}
	return NULL;
}

int isWordIn(char * w)
{
	if(strlen(w)<=2)
		return 1;
	char * words[]={
		"stript", "synchrony",
		"qui",
		"naivete",
		"flatfish",
		"etc", "pud",
		"bon", "tux",
		"non",
		"militarily",
		"quo",
		"windings",
		"fla",
		"languourously",
		"lingua",
		"app",
		"vox",
		"zee",
		"wih",
		"thy",
		"seq",
		"kru",
		"adv",
		"usu",
		"joc",
		"hoc",
		"fml",
		"aux",
		"def",
		"suff",
		"upto",
		"biro",
		"sera",
		"teen",
		"contexts",
		"blur",
		"soya",
		"vous",
		"fron",
		"rata",
		"erat",
		"thev",
		"oyes",
		"foie",
		"pent",
		"troy",
		"noms",
		"math",
		"lire",
		"faux",
		"emph",
		"hors",
		"iamb",
		"ipso",
		"joie",
		"ebbn",
		"foci",
		"symb",
		"cors",
		"luxe",
		"wavy",
		"jure",
		"deja",
		"cine",
		"pron",
		"rhet",
		"conj",
		"pref",
		"wile",
		"fait",
		"lang",
		"euph",
		"bons",
		"bona",
		"oxen",
		"cite",
		"seur",
		"anno",
		"advs",
		"tempi",
		"vacua",
		"vocab",
		"chert",
		"wroth",
		"yobbo",
		"uteri",
		"privilegesor",
		"ultra",
		"turps",
		"terra",
		"ribonucleic",
		"sotto",
		"tammy",
		"tarsi",
		"swath",
		"facie",
		"biros",
		"quint",
		"teddy",
		"pupae",
		"prima",
		"plasm",
		"dicta",
		"grata",
		"petit",
		"kurus",
		"pilau",
		"orjoc",
		"maria",
		"marge",
		"drumfish",
		"modus",
		"mommy",
		"diode",
		"vivre",
		"shish",
		"derog",
		"genii",
		"trong",
		"saucy",
		"chaam",
		"augur",
		"thata",
		"annum",
		"conjs",
		"faits",
		"femme",
		"ovule",
		"drear",
		"fungi",
		"masse",
		"versa",
		"losse",
		"haute",
		"creme",
		"facto",
		"panicles",
		"maths",
		"fides",
		"prons",
		"illus",
		"propr",
		"indef",
		"carte",
		"infml",
		"beads",
		"presentative",
		"falsetto",
		"weirdie",
		"whilst",
		"wherefores",
		"strawberrries",
		"institutions",
		"vulvae",
		"forego",
		"insighviewfinder",
		"unmannnerly",
		"watercolor",
		"uppity",
		"unvarying",
		"autoworkers",
		"underripe",
		"tyrannous",
		"despot",
		"wordprocessor",
		"stategy",
		"translucency",
		"indifficulties",
		"transferral",
		"transiency",
		"transcendency",
		"trafficking",
		"tranqullizing",
		"taperingjet",
		"juttingout",
		"titmice",
		"trepid",
		"thoraces",
		"thingumabob",
		"thingumajig",
		"thesauri",
		"brasswork",
		"testatrix",
		"tenpins",
		"technicolour",
		"tautologous",
		"horsetaketake",
		"taketake",
		"vivants",
		"vulgaris",
		"processors",
		"antibacterial",
		"substrata",
		"succubi",
		"submersion",
		"stockbroking",
		"statutorily",
		"transmissible",
		"sterna",
		"stewardess",
		"trademark",
		"pressesd",
		"sphagna",
		"spotlit",
		"despitefully",
		"specialty",
		"spectra",
		"southwester",
		"lumiere",
		"dispell",
		"brassware",
		"solaria",
		"esthetically",
		"carbonate",
		"hydroxide",
		"classmate",
		"nonprehensile",
		"ammonium",
		"seashores",
		"roadway",
		"snappily",
		"tenuity",
		"vertebrae",
		"roentgenography",
		"wholeheartedly",
		"unmoving",
		"reallocating",
		"ashiny",
		"simoon",
		"repondez",
		"retentivity",
		"mortis",
		"reticulate",
		"prinicipals",
		"restauranteur",
		"resiliency",
		"addressee",
		"relevancy",
		"storeroom",
		"scratchy",
		"countercharge",
		"razzamatazz",
		"hyphenmay",
		"unselfconscious",
		"scrota",
		"reprocessing",
		"scilicet",
		"schemata",
		"expressons",
		"bathetic",
		"megacycles",
		"raison",
		"overspending",
		"quanta",
		"polyvinyl",
		"unwavering",
		"puffily",
		"postscriptum",
		"magnificient",
		"prophetess",
		"instructors",
		"pimiento",
		"priestess",
		"prepaid",
		"prearrange",
		"talcum",
		"preeminent",
		"polymorphic",
		"cornbread",
		"poofter",
		"premenstrual",
		"reponsible",
		"pickax",
		"pierrette",
		"picnicker",
		"picnicking",
		"patresfamilias",
		"phosphorous",
		"physio",
		"pianoforte",
		"typewriting",
		"philosophic",
		"mescal",
		"phalanges",
		"pharynges",
		"carbolic",
		"centimos",
		"bubonic",
		"pestilential",
		"perfumier",
		"capita",
		"shortsightedly",
		"pelves",
		"pedicle",
		"peepbo",
		"microorganisms",
		"overcook",
		"oxygenise",
		"parentheses",
		"palpilating",
		"papaya",
		"papyri",
		"parapsychology",
		"parellel",
		"paleographic",
		"paleolithic",
		"paleontology",
		"paleography",
		"metatarsal",
		"optimal",
		"orchis",
		"estrogen",
		"oceanographer",
		"alumina",
		"ferric",
		"objets",
		"hazelnut",
		"hazelnuts",
		"nouveaux",
		"normalcy",
		"sequitur",
		"hydroxyl",
		"contestants",
		"federal",
		"unselfconsciously",
		"flattish",
		"mutatis",
		"uncountably",
		"communtiy",
		"bellsand",
		"morris",
		"seafish",
		"monies",
		"syndrome",
		"blurring",
		"mizzenmast",
		"bookbinding",
		"misheard",
		"misanthrope",
		"minima",
		"seabird",
		"micrometre",
		"meteorically",
		"motorcycle",
		"megacycle",
		"meioses",
		"mellifluent",
		"maxima",
		"exiguity",
		"watchdog",
		"papier",
		"masseuse",
		"oleracea",
		"manifestations",
		"ncomical",
		"maestri",
		"lysergic",
		"franca",
		"secondhand",
		"workmate",
		"context",
		"lazuli",
		"protestors",
		"larynges",
		"lassie",
		"shoppers",
		"ladybug",
		"severetropical",
		"reassemble",
		"catsup",
		"kibbutzim",
		"improbablity",
		"isomorphous",
		"inticate",
		"investigatory",
		"whiskey",
		"uniformity",
		"interwoven",
		"interpretive",
		"interlineate",
		"intermezzi",
		"ballistic",
		"intially",
		"innovatory",
		"extrem",
		"highjackers",
		"zincores",
		"hatchery",
		"inasmuch",
		"poliomyelitis",
		"waterborne",
		"inaugurateda",
		"impressionistic",
		"mimicking",
		"unfading",
		"unchangeably",
		"ideograph",
		"illusory",
		"currish",
		"interrog",
		"unrealistic",
		"impluses",
		"nuclei",
		"usuclumsy",
		"humeri",
		"hurley",
		"polloi",
		"suffixe",
		"interj",
		"despotic",
		"attrib",
		"hormones",
		"helices",
		"future",
		"approv",
		"immunodeficiency",
		"revisit",
		"rehang",
		"cerebrum",
		"ritual",
		"fortunately",
		"cheery",
		"beathing",
		"arevolt",
		"halfpence",
		"hackney",
		"habeas",
		"genitalia",
		"jukebox",
		"unsmiling",
		"watercolour",
		"youself",
		"reponsibility",
		"hairdo",
		"angostura",
		"trustworthily",
		"formulae",
		"garrotting",
		"gateleg",
		"motorcyclists",
		"gelatin",
		"unwelcoming",
		"forgone",
		"spokesperson",
		"forbode",
		"floodlit",
		"polynomial",
		"daydream",
		"destine",
		"responsibilites",
		"femmes",
		"femora",
		"tassle",
		"fiancee",
		"wallboard",
		"fibulae",
		"fingernails",
		"mignon",
		"pantomine",
		"finessing",
		"continous",
		"exultaion",
		"exterritorial",
		"extortionist",
		"exogamous",
		"expediency",
		"exoskeleton",
		"officio",
		"gratia",
		"girlfriend",
		"errata",
		"inessence",
		"miniskirt",
		"journalist",
		"doddery",
		"environmentalist",
		"epilog",
		"factuality",
		"embryologist",
		"passant",
		"enfant",
		"enfants",
		"duelist",
		"electroconvulsive",
		"educationalist",
		"exempli",
		"elaborations",
		"misspelt",
		"bagpipe",
		"instructor",
		"daydreams",
		"goalie",
		"compar",
		"repetitive",
		"homemaking",
		"bookkeeping",
		"doyenne",
		"artiste",
		"dissyllabic",
		"divergency",
		"deoxyribonucleic",
		"redecorate",
		"disorient",
		"deviancy",
		"instructs",
		"discontinuance",
		"dishonorably",
		"planners",
		"ideologists",
		"devilry",
		"telltale",
		"motorbike",
		"povery",
		"delftware",
		"funfair",
		"embracing",
		"desiderata",
		"windswept",
		"rituals",
		"absorbers",
		"billiard",
		"bacilli",
		"rigueur",
		"underskirt",
		"centum",
		"reread",
		"criteria",
		"schoolchildren",
		"journalists",
		"cortices",
		"falsify",
		"raccoon",
		"conjunct",
		"cesspool",
		"birthdays",
		"contortingor",
		"ungulate",
		"unchanging",
		"complacence",
		"compos",
		"contestant",
		"guidebook",
		"contests",
		"cognoscenti",
		"hairstyle",
		"chaises",
		"goalposts",
		"cavers",
		"auricle",
		"draughtboard",
		"instruct",
		"leniency",
		"venereal",
		"nailclippers",
		"tonsillectomy",
		"centralist",
		"centralizing",
		"cartwheels",
		"cashable",
		"hurriedly",
		"blanche",
		"criticizingor",
		"geological",
		"garlic",
		"priesthood",
		"motorboat",
		"acetic",
		"electioneer",
		"addenda",
		"adieux",
		"adjacent",
		"hormone",
		"photographer",
		"millimetre",
		"practiser",
		"burglarize",
		"dryness",
		"fortiori",
		"humanoid",
		"inhibitory",
		"children",
		"apices",
		"geometrical",
		"aforesaid",
		"accouterments",
		"citadel",
		"seashore",
		"everyone",
		"aluminum",
		"tablecloth",
		"amidst",
		"amoebae",
		"amongst",
		"classroom",
		"classrooms",
		"firepower",
		"gratin",
		"aureole",
		"aeronautical",
		"prorating",
		"anapest",
		"anapestic",
		"phenomena",
		"toward",
		"antilog",
		"horizontally",
		"footballers",
		"unchanged",
		"unspecified",
		"embraced",
		"outboard",
		"tenpin",
		"noires",
		"setback",
		"birthday",
		"cistern",
		"embrace",
		"underlying",
		"blameworthy",
		"fortunate",
		"blonde",
		"larvae",
		"bribery",
		"bookbinder",
		"bookkeeper",
		"heathland",
		"dyspneic",
		"bricklayer",
		"brilliancy",
		"boyfriend",
		"freelance",
		"reintroduce",
		"darkish",
		"sandcastles",
		"bouyancy",
		"passageway",
		"archduchess",
		"courtroom",
		"unstressed",
		"uncontrolled",
		"appendicectomy",
		"uncontrollably",
		NULL
	};
	int i=0;
	while(1)
	{
		char * word = words[i++];
		if(word==NULL)
			break;
		//printf("%s,%s,",word,w);
		if(strcasecmp(word,w)==0)
			return 1;

		if(endwith(w)) return 1;
	}
	return 0;
}

void addExplainSpace()
{
	//make_db("oxford-gb","/home/libiao/sound/dict.db");//from stardict to sqlite3
	Dict * dict = Dict_new();
	int modify = 0;
	modify = 0;
	modify=1;
	DataBase * db = DataBase_new(decodePath("~/sound/dict.db"));
	dict->name = "oxford-gb";
	int numWords = Dict_getNumWords(dict);
	printf("numWords:%d\r\n",numWords);
	int i = 1;
	i = 17000;
	i = 30000;
	i = 1;
	while(i<numWords)
	{
		Word * word = Dict_getWordByIndex(dict,i);
		char * explain = Dict_getMean(dict,word);

		Array *matched_arr= Array_new();
		int len = regex_search_all(explain,"/ ([a-z]{4,}) /",matched_arr);
		//printf("%d\n",(int)len);
		int a = 0;
		int has = 0;
		while(a<len)
		{
			char * w = Array_getByIndex(matched_arr,a++);
			int l = strlen(w);
			char _w[l];
			memset(_w,0,l);
			sprintf(_w,"%s",w+1);
			_w[l-2]='\0';
			int _index = Dict_getWordIndex(dict,_w);
			if(_index<0)
			{
				if(isWordIn(_w))
					continue;
				if(isOtherFormIn(dict,_w))
					continue;
				if(startWith(dict,_w))
					continue;

				char * s2 = replace_word(_w);
				if(s2==NULL)
				{
					printf("======%d,%s: %s\r\n",i,word->word,explain);
					printf("%s\n",_w);
					has = 1;
				}else{
					explain = regex_replace_all(explain,_w,s2);
					//printf("======%d,%s: %s\r\n",i,word->word,explain);
					if(modify)
						update_one_word(db,word->word,explain);
				}
			}
		}
		regex_matchedarrClear(matched_arr);

		/*
		   int _index = Dict_getWordIndex(dict,__word);
		   if(_index<0)
		   {
		//__word = regex_replace_all(word,"/^[^ ]{1,} /(.*)/.*$/","$1");
		}
		*/

		if(has)
			break;
		++i;
	}
	if(modify)
		make_dict("/home/libiao/sound/dict.db","oxford-gb");// from sqlite3 to stardict
	return;
}

void remend()
{
	DataBase * db = DataBase_new(decodePath("~/dict.db"));
	//char * sql = "select * from dict where explain like \"%<i>US</i>%\";";
	//char * sql = "select * from dict where word like \"%,%\";";
	//char * sql = "select * from dict where word like \"%/ %/%\";";
	char * sql = "select * from dict where word like \"`%\";";
	int rc = DataBase_exec2array(db,sql);
	if(rc){
		printf("ERROR: %s\r\n",db->result_arr);
		return;
	}
	//DataBase_result_print(db);
	Array * wordsArr = NULL;
	Array * explainsArr = NULL;
	if(db->result_arr){
		Array * data = db->result_arr;
		Array * names = Array_getByIndex(data,0);
		if(names==NULL){
			printf("no names Array");
			return ;
		}
		int nCount = names->length;
		int i = 0;
		while(i<nCount)
		{
			char * curName =Array_getByIndex(names,i);
			if(strcmp(curName,"word")==0)
			{
				wordsArr = Array_getByIndex(data,i+1);
				if(wordsArr == NULL || wordsArr->length==0){
					printf("%d:no word\r\n",i);
					return ;
					break;
				}
			}else if(strcmp(curName,"explain")==0){
				explainsArr = Array_getByIndex(data,i+1);
				if(explainsArr == NULL || explainsArr->length==0){
					printf("%d:no explain\r\n",i);
					return ;
					break;
				}
			}
			++i;
		}
	}
	if(wordsArr && explainsArr && wordsArr->length>0 && wordsArr->length == explainsArr->length)
	{
		int i = 0;
		char _explain[160];
		Dict * dict = Dict_new();
		dict->name = "oxford-gb";
		while(i<wordsArr->length){
			//char * explain = NULL;
			char * word = Array_getByIndex(wordsArr,i);
			char * explain = Array_getByIndex(explainsArr,i);
			//if(regex_match(word,"/\\(esp /"))
			if(regex_match(word,"/^`/"))
			{
				//char * _word = regex_replace_all(word,"/ \\(Brit.*$/","");
				//char * _word = regex_replace_all(word,"/^also[ `]*/","");
				//char * _word = regex_replace_all(word,"/^[^,]*, /","");
				//char * _word = regex_replace_all(word,"/^([^ ]{1,}) .*$/","$1");
				char * _word = regex_replace_all(word,"/^`/","");
				//printf("%d:%s\r\n",i,_word);fflush(stdout);

				int _index = Dict_getWordIndex(dict,_word);
				if(_index<0)
				{
					printf("%s->\"%s\" \r\n",word,_word);fflush(stdout);
					//update_one_word(db,_word,explain);
				}

				char * __word = NULL;
				//__word = regex_replace_all(word,"/^.* US (.*)$/","$1");
				__word = regex_replace_all(word,"/^[^ ]{1,} /(.*)/.*$/","$1");
				memset(_explain,0,sizeof(_explain));
				sprintf(_explain,"=> %s",_word);
				_index = Dict_getWordIndex(dict,__word);
				if(_index<0)
				{
					//printf("\"%s\": %s\r\n",__word,_explain);
					//update_one_word(db,__word,_explain);
				}
				//
			}
			/*
			   if(regex_match(tmp,"/ <i>US</i> /"))
			   explain = regex_replace_all(tmp,"/ <i>US</i> /","");
			   if(regex_match(tmp,"/<i>US</i>/"))
			   explain = regex_replace_all(tmp,"/<i>US</i>/","");
			   if(word && explain){
			   update_one_word(db,word,explain);
			   }
			   */
			++i;
		}
	}
}
static void addChineseExplain()
{
	DataBase * db = DataBase_new(decodePath("~/dict.db"));
	Dict * dict = Dict_new();
	dict->name = "oxford-gb";
	Dict * dict2 = Dict_new();
	dict2->name = "langdao";
	//dict2->name = "stardict";
	int i = 0;
	int numWords = Dict_getNumWords(dict);
	int j=0;
	while(i<numWords)
	{
		Word * word = Dict_getWordByIndex(dict,i);
		char * explain = Dict_getMean(dict,word);
		//if(regex_match(explain,"/^[\x01-\x7fːæðŋǀɑɒɔəɜɪʃʊʌʒθ]{2,}$/i"))
		//if(regex_match(explain,"/^[\x01-\x7fːæðŋǀɑɒɔəɜɪʃʊʌʒθ]{2,}$/m"))
		//if(regex_match(explain,"/[\x2f/\\[\\]\x5b\x5d]/m")==0)
		if(strstr(explain,"[")==NULL && strstr(explain,"/")==NULL)
		{
			int _index = Dict_getWordIndex(dict2,word->word);
			if(_index>=0)
			{
				Word * word2 = Dict_getWordByIndex(dict2,_index);
				char * explain2 = Dict_getMean(dict2,word2);
				//printf("%s:(%s)\r\n",word->word,explain);

				if(strstr(explain2,"[")!=NULL)
				{
					int len = strlen(explain2)+strlen(explain)+4;
					char newExplain[len];
					memset(newExplain,0,len);
					sprintf(newExplain,"%s\n%s",explain,explain2);
					printf("====>%s\r\n",newExplain);
					if(strcmp(dict2->name,"strdict")==0)
					{
						//update_one_word(db,word->word,explain2);
					}else{
						update_one_word(db,word->word,newExplain);
					}
				}
				free(explain2);
			}
			free(explain);
			explain = NULL;
			++j;
			//if(j>300) break;
		}
		if(explain)
			free(explain);
		++i;
	}
	printf("a ok\r\n");
}

int main()
{
	/*
	   const char * dict_name = "strdict";
	   char * dict_dir = decodePath(contact_str("~/sound/",dict_name));
	   printf("%s/%s.ifo",dict_dir,dict_name);
	   */
	//remend();
	addExplainSpace();
	//addChineseExplain();
	//return update_word("/home/libiao/dict.db","hello","has long mean");
	//make_db("oxford-gb","/home/libiao/dict.db");//from stardict to sqlite3
	//addirregularverb();
	//select count(*) from dict where explain like "%<i>US</i>%";
	//return make_dict("/home/libiao/dict.db","oxford-gb");// from sqlite3 to stardict
	return 0;
}
