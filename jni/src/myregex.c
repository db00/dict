/**
 *
 gcc -Wall myregex.c mystring.c array.c  -lm -D test_regex && ./a.out
 gcc -Wall mystring.c array.c myregex.c regex.c -lm -D test_regex -D STDC_HEADERS && a
 gcc -Wall myregex.c mystring.c regex.c -lm -D STDC_HEADERS  && ./a.out
 gcc -I"../SDL2/include/" -Wall mystring.c dict.c myregex.c regex.c  -lSDL2 -lm -lSDL2 -Ddebug -DSTDC_HEADERS && ./a.out nude
 gcc -I"../SDL2/include/" -Wall mystring.c dict.c myregex.c regex.c  -lmingw32 -lSDL2main -lSDL2 -lm -lSDL2 -Ddebug -DSTDC_HEADERS && a nude


 ndk_home=~/android-ndk-r10 ; GCC=$ndk_home/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86/bin/arm-linux-androideabi-gcc ; sysroot=$ndk_home/platforms/android-9/arch-arm/usr ; $GCC --sysroot=$sysroot -I"$sysroot/include"  -g -Wall  myregex.c mystring.c array.c -ldl -lm -D test_regex && adb push a.out /sdcard/a.out && adb shell "mount -o remount,rw,exec /mnt/sdcard/" && adb shell /sdcard/a.out


 */

#include "myregex.h"

int regex_error(regex_t * re,int err_no)
{/*{{{*/
	char errbuf [1024];
	//int len = 
	regerror(err_no, re, errbuf, sizeof(errbuf));
	//printf("error: %s\n", errbuf);
	regfree(re);
	free(re);
	return 0;
}/*}}}*/

static int regex_compile(regex_t * re ,const char *regString)
{/*{{{*/
	char * regstr = NULL;
	char * regEnd = NULL;
	int flag = REG_EXTENDED;
	if(*regString=='/'){
		regEnd = strrchr(regString,'/');
		int len = regEnd - (regString+1) ;
		int start = 1;
		regstr = getSubStr((char*)regString,start,len);

		char *flags = regEnd + 1;
		while(*flags){
			switch(*flags)
			{
				/**
				  REG_EXTENDED 以功能更加强大的扩展正则表达式的方式进行匹配。
				  REG_ICASE 匹配字母时忽略大小写。
				  REG_NOSUB 不用存储匹配后的结果。
				  REG_NEWLINE 识别换行符，这样'$'就可以从行尾开始匹配，'^'就可以从行的开头开始匹配。
				  */
				case 'i':
					flag |= REG_ICASE;
					//printf("REG_ICASE,");
					break;
					/*
					   case 'm':
					   printf("REG_NEWLINE,");
					   flag |= REG_NEWLINE;
					   break;
					   case 'e':
					   printf("REG_EXTENDED,");
					   flag |= REG_EXTENDED;
					   break;
					   case 'n':
					   printf("REG_NOSUB,");
					   flag |= REG_NOSUB;
					   break;
					   */
			}
			++flags;
		}
	}
	if(regstr == NULL){
		regstr = getSubStr((char*)regString,0,strlen(regString));
	}
	int err = regcomp(re, regstr, flag);
	free(regstr);
	return err;
}/*}}}*/

int regex_match(const char*s,const char*regString)
{/*{{{*/
	if(s==NULL || strlen(s)==0)
		return 0;
	int err;
	regex_t *re = malloc(sizeof(regex_t));            
	regmatch_t    subs [SUBSLEN];

	err = regex_compile(re,regString);
	if (err)
	{
		regex_error(re,err);
		re = NULL;
		return 0;
	}
	err = regexec(re, s, (size_t) SUBSLEN, subs, 0);
	if (err == REG_NOMATCH)
	{
		regex_error(re,err);
		re = NULL;
		return 0;
	}else if (err){  
		regex_error(re,err);
		re = NULL;
		return 0;
	}
	//printf("regex_match: %d\n", re->re_nsub);
	if(re){
		regfree(re);
		free(re);
	}
	return 1;
}/*}}}*/
/**
 *  
 *
 * @param s	the string to search
 * @param reg	the regex
 * @param callback	0:the first matched,
 * 					1:the first submatch,
 * 					2:the second submatch,
 * 					3:the third submatch,
 * 					...
 *
 * @return  
 */
char *regex_search(char * s,char * regString , int callback,int *dealed_len)
{
	size_t       len;
	regex_t *re = malloc(sizeof(regex_t));            
	regmatch_t    subs [SUBSLEN];
	int           err;
	if(dealed_len)*dealed_len = 0;

	err = regex_compile(re,regString);
	if (err)
	{
		regex_error(re,err);
		re = NULL;
		if(dealed_len) *dealed_len = strlen(s);
		return NULL;
	}

	//printf("re->re_nsub: %d\n", re->re_nsub);
	char *subsrc = s;
	if(dealed_len) subsrc += *dealed_len;
	err = regexec(re, subsrc, (size_t) SUBSLEN, subs, 0);
	if (err == REG_NOMATCH)
	{
		//printf("Sorry, no more match ...\n");
		if(dealed_len) *dealed_len = strlen(s);
		regex_error(re,err);
		re = NULL;
		return NULL;
	}else if (err){  
		regex_error(re,err);
		re = NULL;
		if(dealed_len) *dealed_len = strlen(s);
		return NULL;
	}

	char * matched = NULL;
	callback = (callback>re->re_nsub?re->re_nsub:callback);
	len = subs[callback].rm_eo - subs[callback].rm_so;
	if(len >0)
	{
		matched = getSubStr(subsrc,subs[callback].rm_so,len);
	}
	if(dealed_len) *dealed_len += subs[0].rm_eo;//当前匹配的结束地址
	if(re){
		regfree(re);
		free(re);
	}
	return matched;
}

/* -----------------------------------------------*/
/**
 *  replace the string with the call back search string array 
 *
 * @param str
 *
 * @return  
 */
char *regex_replace2(char * src,Array* array)
{
	size_t       len;
	regex_t *re = malloc(sizeof(regex_t));            
	regmatch_t    subs [SUBSLEN];
	int           err, i,pos=0;
	char          pattern [] = "\\$[0-9]{1,2}";
	char *ret=NULL;

	//printf("String	: %s\nlen:%d\n", src,strlen(src));
	//printf("Pattern	: %s \n", pattern);


	err = regcomp(re, pattern, REG_EXTENDED);
	if (err)
	{
		regex_error(re,err);
		re = NULL;
		return src;
	}

	//printf("re->re_nsub: %d\n", re->re_nsub);
	do{
		char *subsrc = src + pos;
		err = regexec(re, subsrc, (size_t) SUBSLEN, subs, 0);
		if (err == REG_NOMATCH)
		{
			regex_error(re,err);
			re = NULL;
			break;
		}else if (err){  
			regex_error(re,err);
			re = NULL;
			break;
		}

		len = subs[0].rm_eo - subs[0].rm_so;
		char * matched = getSubStr(subsrc,subs[0].rm_so,len);
		if(matched){
			i = atoi(matched+1);
			free(matched);
		}else{
			return src;
		}
		if(i<array->length){
			char * replace_str = (char*)(Array_getByIndex(array,i));
			//printf("[%d]=%s\n",i,replace_str);
			if(subs[0].rm_so > 0){
				char * tmp = getSubStr(subsrc,0,subs[0].rm_so);
				if(tmp)
					ret = append_str(ret,"%s",tmp);
				free(tmp);
			}
			ret = append_str(ret,"%s",replace_str);
			pos += subs[0].rm_eo;//当前匹配的结束地址
			//printf("cur pos:%d\n",pos);
		}else{
			return src;
		}
	}while(pos < strlen(src));

	if(strlen(src)>pos){
		ret = append_str(ret,"%s",src+pos);
	}
	return ret;
}
/**
 *  
 *
 * @param s
 * @param reg
 * @param replace_str	replace the matched string with the replace_str
 *
 * @return  
 */
char *regex_replace(char * s, const char * regString , const char * replace_str,int *dealed_len)
{
	if(s==NULL)
		return NULL;
	size_t       len;
	regex_t *re = malloc(sizeof(regex_t));            
	regmatch_t    subs [SUBSLEN];
	int           err, i;
	if(dealed_len) *dealed_len = 0;
	int bufsize= 0;
	//printf("src:		%s\n",s);
	//printf("regString:	%s\n",regString);
	//printf("replace_str:	%s\n",replace_str);

	err = regex_compile(re,regString);
	if (err)
	{
		regex_error(re,err);
		re = NULL;
		if(dealed_len) *dealed_len = strlen(s);
		return s;
	}

	char *ret = NULL;
	do{
		char *subsrc = s;
		if(dealed_len)
			subsrc += *dealed_len;

		err = regexec(re, subsrc, (size_t) SUBSLEN, subs, 0);
		if (err == REG_NOMATCH){
			regex_error(re,err);
			re = NULL;
			break;
		}else if (err){  
			regex_error(re,err);
			re = NULL;
			if(ret){
				free(ret);
				ret = NULL;
			}
			if(dealed_len) *dealed_len = strlen(s);
			return s;
		}

		//printf("re_nsub:%d\n",re->re_nsub);
		Array * matched_arr = Array_resize(NULL,re->re_nsub+1);
		for (i = 0; i <= re->re_nsub; i++)
		{
			len = subs[i].rm_eo - subs[i].rm_so;
			Array_setByIndex(matched_arr,i,getSubStr(subsrc,subs[i].rm_so,len));
			//printf("matched_arr[%d]: %s\n",i, (char*)Array_getByIndex(matched_arr,i));
		}
		bufsize += (subs[0].rm_so+1);
		char * tmp = getSubStr(subsrc,0,subs[0].rm_so);
		if(tmp)
			ret = append_str(ret,"%s",tmp);
		free(tmp);
		char * copy_str = NULL;
		if(re->re_nsub>0 && regex_match(replace_str,"/\\$[0-9]{1,2}/")>0){//替换字符串中含有回调,回调号功能
			copy_str = regex_replace2((char*)replace_str, matched_arr);
			//printf("copy_str:%s\n",copy_str);
			if(copy_str && strlen(copy_str)>0){
				ret = append_str(ret,"%s",copy_str);
				if(copy_str != replace_str){
					free(copy_str);
					copy_str = NULL;
				}
			}
		}else{
			ret = append_str(ret,"%s",(char*)replace_str);
		}
		if(matched_arr){
			for (i = 0; i <= re->re_nsub; i++)
				free(Array_getByIndex(matched_arr,i));
			Array_clear(matched_arr);matched_arr = NULL;
		}
		if(dealed_len)
			*dealed_len += subs[0].rm_eo;//当前匹配的结束地址
		else
			break;
	}while(*dealed_len < strlen(s));

	if(dealed_len){
		if(strlen(s)>(int)(*dealed_len)) 
			ret = append_str(ret,"%s", s + (*dealed_len));
	} else {
		ret = append_str(ret,"%s", s + subs[0].rm_eo);
	}

	if(re){
		regfree(re);
		free(re);
	}
	//printf("result:%s\n",ret);
	return ret;
}
int regex_search_all(char * s,char * reg , Array*matched_arr)
{
	int num = 0;
	int dealed_len=0;
	int pos = 0;
	//printf("%s(%d)\n",s,strlen(s));
	//printf("%s\n",reg);
	while(pos<strlen(s))
	{
		char * p = s + pos;
		Array_setByIndex(matched_arr,num, regex_search(p,reg , 0, &dealed_len));
		if(Array_getByIndex(matched_arr,num) && strlen(Array_getByIndex(matched_arr,num))>0){
			//printf(" matched:%s,dealed_len:%d\n",Array_getByIndex(matched_arr,num),pos);
			//free(Array_getByIndex(matched_arr,num)); Array_setByIndex(matched_arr,num,NULL);
			num++;
		}else{
			break;
		}
		pos += dealed_len; 
		//printf("dealed_len:%d\n",pos);
	}

	return num;
}

char *regex_replace_all(char * src, const char * reg , const char * replace_str)
{
	int dealed_len=0;
	return  regex_replace(src,reg,replace_str,&dealed_len);
}

void regex_matchedarrClear(Array* matched_arr)
{
	if(matched_arr)
		Array_freeEach(matched_arr);
}

#ifdef test_regex 
int main()
{
	printf("%d\n",regex_match("teststests","/TESTS/i"));

	size_t       len;
	regmatch_t    subs [SUBSLEN];
	int           err, i,pos=0;
	char          src    [] = "++<title>12345+9876</title>";

	printf("%s\n",regex_replace_all(src,"/<([^<>]+)>/im",""));

	printf("\ntest regex_search_all:\n");
	Array *matched_arr= Array_new();
	len = regex_search_all(src,"/<([^<>]*)>/",matched_arr);
	printf("%d\n",(int)len);
	regex_matchedarrClear(matched_arr);

	printf("\ntest regex_search:\n");
	int dealed_len=0;
	char *replaced = regex_search(src,"/([0-9]+)/",0,&dealed_len);
	printf("%s,%d\n",replaced,dealed_len);
	if(src != replaced)free(replaced);//free
	printf("\n-----------\n");

	printf("\ntest regex_replace:\n");
	replaced = regex_replace(src,"/([0-9]+)/","/($1)/",&dealed_len);
	printf("regex_replace:%s,%d",replaced,dealed_len);
	if(src != replaced)free(replaced);//free
	printf("\n-----------\n");


	regex_t *re = malloc(sizeof(regex_t));            
	regex_compile(re,"/(\\+)/");
	//printf("re->re_nsub: %d\n", re->re_nsub);
	do{
		char *subsrc = src+pos;
		err = regexec(re, subsrc, (size_t) SUBSLEN, subs, 0);
		if (err == REG_NOMATCH)
		{
			regex_error(re,err);
			re = NULL;
			break;
		}else if (err){  
			regex_error(re,err);
			re = NULL;
			break;
		}

		for (i = 0; i <= re->re_nsub; i++)
		{
			len = subs[i].rm_eo - subs[i].rm_so;
			if (i == 0)
			{//当前匹配
				printf ("first match : %d -> %d , len : %d , ",(int)(pos + subs[i].rm_so),(int)(pos + subs[i].rm_eo),(int)len);
			}
			else
			{//子匹配
				printf("sub match %d : %d -> %d , len : %d , ", i, (int)(pos + subs[i].rm_so),(int)(pos + subs[i].rm_eo), (int)len);
			}

			char * matched = getSubStr(subsrc,subs[i].rm_so,len);
			printf("match: %s\n", matched);
			if(matched)free(matched);
		}
		pos += subs[0].rm_eo;//当前匹配的结束地址
		//printf("cur pos:%d\n",pos);
	}while(pos < strlen(src));

	if(re){
		regfree(re);
		free(re);
	}
	return (0);
}
#endif
