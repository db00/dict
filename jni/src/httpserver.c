/**
 *
 gcc -g -Wall -lpthread httpserver.c array.c myregex.c filetypes.c dict.c files.c mystring.c  urlcode.c base64.c -ldl -lm -D DEBUG -D debug_httpserver -o ~/a && ~/a
 gcc -g -Wall -I"." httpserver.c array.c myregex.c regex.c filetypes.c dict.c files.c mystring.c urlcode.c base64.c -lpthread -lwsock32 -lm -DSTDC_HEADERS -D DEBUG -D debug_httpserver && a
 gdb ~/a
 gcc -g -Wall -lpthread httpserver.c filetypes.c mystring.c urlcode.c base64.c -ldl -lm -D debug_httpserver -o ~/a && ~/a

 ndk_home=~/android-ndk-r10 ; GCC=$ndk_home/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86/bin/arm-linux-androideabi-gcc ; sysroot=$ndk_home/platforms/android-9/arch-arm/usr ; $GCC --sysroot=$sysroot -I"$sysroot/include"  -g -Wall  httpserver.c array.c myregex.c filetypes.c dict.c files.c mystring.c  urlcode.c base64.c -ldl -lm -D DEBUG -D debug_httpserver && adb push a.out /sdcard/a.out && adb shell /sdcard/a.out

 curl -F Filedata=@localfilename -F Filename=localfilename localhost:8809 -u test:test

 curl -F Filedata=@/home/libiao/icon.png -F Filename=android.png localhost:8809/home/libiao/test/ -u test:test
 将/home/libiao/icon.png 上传到 /home/libiao/test/android.png

 curl -F Filedata=@/home/libiao/screenshot.png -F Filename=screenshot.png 192.168.1.67:8809/sdcard/ -u test:test
 curl -F Filedata=@/home/libiao/new/kof/retropie-4.4-rpi1_zero.img.gz -F Filename=retropie-4.4-rpi1_zero.img.gz 192.168.1.47:8809/sdcard/ -u test:test

http://0.0.0.0:8809/?f=mean&p1=test

zip -r c.zip *.c
http://www.macromedia.com/support/documentation/en/flashplayer/help/settings_manager04.html

 * @author: db0@qq.com
 * @version 1.0.1
 * @date 2013-08-27
 */

#include "httpserver.h"
//#include "sqlite.h"//如不需数据支持,注释此行,否则不注释此行,windows下加-lwsock32,linux加-lpthread编译参数,并加sqlite3.c
#include "dict.h"//字典

int close_sock(int new_fd)
{
#ifdef __WIN32
	if(new_fd >0)closesocket(new_fd);
	//WSACleanup();
#else
	if(new_fd >0)close(new_fd);
#endif
	return 0;
}


Parameter *Parameter_new(char*name,char *value)
{
	Parameter*parameter = (Parameter*)malloc(sizeof(*parameter));
	memset(parameter,0,sizeof(*parameter));
	if(name)parameter->name = name;
	if(value)parameter->value = value;
	return parameter;
}

void Parameter_clear(Parameter*parameter)
{
	if(parameter)
	{
		//if(parameter->name) free(parameter->name);
		//if(parameter->value) free(parameter->value);
		if(parameter->next) Parameter_clear(parameter->next);
		free(parameter);
	}
}


void Client_clear(Client*client)
{
	if(client)
	{
		if(client->fd)
			close_sock(client->fd);
		if(client->head)
			free(client->head);
		if(client->data)
			free(client->data);
		if(client->real_path)
			free(client->real_path);
		if(client->get_param_str)
			free(client->get_param_str);
		if(client->boundary_str)
			free(client->boundary_str);
		if(client->addr)
			free(client->addr);
		if(client->path)
			free(client->path);
		if(client->host)
			free(client->host);
		if(client->auth_usr)
			free(client->auth_usr);
		if(client->auth_pswd)
			free(client->auth_pswd);
		if(client->parameters)
			Parameter_clear(client->parameters);
		client->num_paras = 0;
		free(client);
	}
}

void Server_clear(Server*server)
{
	if(server)
	{
		if(server->sock_fd)
		{
			close_sock(server->sock_fd);
		}
		free(server);
	}
}
char * getHome(){
	int l=100;
#ifdef __ANDROID__
	char * home = mysystem("echo /sdcard",&l);
#else
	char * home = mysystem("echo $HOME",&l);
#endif
	while(*(home+strlen(home)-1)=='\n')
		home[strlen(home)-1]='\0';
	//printf("home: %s\n",home);
	return home;
}
char * home_resolve(char *s)
{
	char * home = getHome();
	if(s==NULL || strlen(s)==0)
		return home;
	int l=strlen(home)+strlen(s)+2;
	char * path = malloc(l);
	memset(path,0,l);
	sprintf(path,"%s/%s",home,s);
	//printf("home_resolve --------%s\n",path);
	free(home);
	return path;
}

char* getmetavalue(char*head,const char*meta)
{
	char *curpos = strstr(head,meta);
	if(curpos == NULL)return NULL;
	char *pos;
	int len=0;
	curpos += strlen(meta);
	while(*curpos == ' ' || *curpos == ':' )curpos ++;
	char* end1= strstr(curpos,"\r\n");
	if(strcmp("Authorization",meta))
	{
		char* end2= strstr(curpos," ");
		len = (end1>end2?end2:end1) - curpos;
	}else if(end1){
		len = end1 - curpos;
	}
	if(len<=0)
		return NULL;
	pos = malloc(len+1);
	if(pos == NULL)return NULL;
	memset(pos,0,len + 1);
	memcpy(pos,curpos,len);
	return pos;
}

int getparas(Client*client,char* str)//get 以 '&' 分割的参数
{
	if(str==NULL)return 1;
	//printf("getparas:%s\n",str);
	client->get_param_str = malloc(strlen(str)+1);
	memset(client->get_param_str,0,strlen(str)+1);
	strcpy(client->get_param_str,str);

	Parameter * parameter = Parameter_new(NULL,NULL);
	char *cur = strtok(client->get_param_str,"&");
	while(cur){
		parameter = Parameter_new(NULL,NULL);
		parameter->name = cur;
		parameter->value = strstr(cur,"=");
		if(parameter->value){
			*(parameter->value)='\0';
			parameter->value +=1;

			if(client->parameters == NULL)
			{
				client->num_paras = 1;
				client->parameters = parameter;
			}else{
				client->num_paras += 1;
				client->lastParameter->next = parameter;
			}

			client->lastParameter= parameter;
			printf("parameter: %s ===== %s\n",parameter->name,parameter->value);
		}else{
			Parameter_clear(parameter);
			break;
		}
		cur = strtok(NULL,"&");
	}
	return 0;
}

int Client_sends(Client * client,const char *fmt, ...) 
{
	if(fmt==NULL)
		return -1;
	int sock = client->fd;
	if(strlen(fmt)==0)
		return -1;
	int n, size = getpagesize();
	char *p;
	va_list ap;
	if ((p = (char *) malloc(size)) == NULL)
		return -1;
	memset(p,0,size);
	while (1) {
		/* 尝试在申请的空间中进行打印操作 */
		va_start(ap, fmt);
		n = vsnprintf (p, size-1, fmt, ap);
		va_end(ap);
		/* 如果vsnprintf调用成功，返回该字符串 */
		if (n > 0 && n < size-1){
			break;
		}
		/* vsnprintf调用失败(n<0)，或者p的空间不足够容纳size大小的字符串(n>=size)，尝试申请更大的空间*/
		size += getpagesize(); /* 再申请一个内存页 */
		if ((p = (char *)realloc(p, size)) == NULL)
			return -1;
	}
	int len = strlen(p);
	int sended_len = 0;
	while(sended_len < len){
		int r = send(sock,p+sended_len,len-sended_len,0);
		if(r<0)return -2;
		sended_len += r;
	}
	//printf(p);
	if(p)free(p);
	return sended_len;
}


int send_file(Client*client)
{/*{{{*/
	int ret;
	unsigned int len;
	FILE *fp =fopen(client->real_path,"rb");
	if(fp == NULL){
		return 1;
	}
	fseek(fp, 0L, SEEK_END);
	len = ftell(fp);

	rewind(fp);
	char *filebuf=malloc(0x100);
	fflush(stdout);
	if(filebuf){
		memset(filebuf,0, 0x100);
		ret =fread(filebuf, 1,0x100,fp);
		if(ret >0){
		}else{
			free(filebuf);
			filebuf=NULL;
			return 2;
		}
	}else{
		return 3;
	}

	if(strncmp(filebuf,"\x7f\x45\x4c\x46",4)==0)
	{//exec respond
		int cmdlen = strlen(client->real_path)+strlen(client->path)+4;
		char cmd[cmdlen];
		memset(cmd,0,cmdlen);
		strcat(cmd,client->real_path);
		char * end = strstr(client->path,"?");
		if(end)
		{
			strcat(cmd," ");
			strcat(cmd,end);
		}
		char * _cmd = regex_replace_all(cmd,"[&\\?=]"," ");
		char *output= NULL;
		int len =0;
		if(_cmd){
			output = mysystem(_cmd,&len);
			free(_cmd);
		}
		if(output){
			//Client_sends(client, "Content-Type: %s\r\n","text/html");
			Client_sends(client,"Content-Length: %d\r\n",len);
			Client_sends(client,"Connection: close\r\n\r\n");
			Client_sends(client,output);
			if(output){
				free(output); output = NULL;
			}
			fclose(fp);
			return 0;
		}
	}

	Client_sends(client,"Connection: keep-alive\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\n",len);
	char * type = (char*)type_by_head(filebuf);
	if(type == NULL){
		type=strrchr(client->real_path,'.');
		printf("other type:-------------%s\n",type);
	}
	if(type==NULL)type = "*";
	Client_sends(client, "Content-Type: %s",type2mine(type));
	Client_sends(client,"\r\n\r\n");

	rewind(fp);
	int send_once_size=0x100000;
	char *p = malloc(send_once_size);
	if(p){
		memset(p,0, send_once_size);
		unsigned int remain_size = len;
		unsigned int cur_size = send_once_size;
		if(remain_size<send_once_size)cur_size = remain_size;
		while((ret =fread(p, 1,cur_size,fp))>0){
			send(client->fd,p,ret,0);
			memset(p,0, ret);
			remain_size=len - ftell(fp);
			if(remain_size<send_once_size)cur_size = remain_size;
			if(remain_size<=0)break;
		}
		fclose(fp);
		if(p)free(p);
		p = NULL;
	}
	if(filebuf)free(filebuf);
	filebuf = NULL;
	return 0;
}/*}}}*/


char * path_conact(char*cur_dir,...) 
{//return current directory's real path + the file name 
	int ret_size = strlen(cur_dir) + 4;
	char* ret=(char*)malloc(ret_size);
	memset(ret,0,ret_size);
	char *p = cur_dir;
	if(cur_dir){
		while(*p=='/')p++;
		memcpy(ret,p,strlen(p));
	}

	if(strlen(ret)>0){//delete the "/" at end of the cur_dir;
		while(*(ret+strlen(ret)-1)=='/' || *(ret+strlen(ret)-1)=='\\')
			*(ret+strlen(ret)-1)='\0';
	}
	va_list argptr;
	va_start(argptr, cur_dir);
	while(1){
		char*file_name = va_arg(argptr,char*);
		if(file_name && strlen(file_name)){
			if(strlen(ret)+strlen(file_name)>=ret_size-4) {
				ret_size += strlen(file_name)+4;
				ret = (char*)realloc(ret,ret_size);
			}
			if(strcmp(file_name,"..")==0){
				while(strlen(ret)>0 && *(ret+strlen(ret)-1)!='/')
					*(ret+strlen(ret)-1)='\0';
				while(strlen(ret)>0 && *(ret+strlen(ret)-1)=='/')
					*(ret+strlen(ret)-1)='\0';
			}else if(strcmp(file_name,".")==0){
			}else{
				while(*(ret+strlen(ret)-1)=='/' || *(ret+strlen(ret)-1)=='\\')
					*(ret+strlen(ret)-1)='\0';
				if(strlen(ret)>0)
					sprintf(ret+strlen(ret),"/");
				sprintf(ret+strlen(ret),"%s",file_name);
			}
		}else{
			break;
		}
	}
	va_end(argptr);
	return ret;
}

char * web2realpath(Server*server,char*path)
{
	int path_len = strlen(path)+strlen(server->root)+3;
	char* ret=(char*)malloc(path_len);
	memset(ret,0,path_len);
	sprintf(ret,"%s",server->root);
	while(ret[strlen(ret)-1]=='/')
		*(ret+strlen(ret)-1)='\0';
	if(path!=NULL){
		if(*path!='/')
			sprintf(ret+strlen(ret),"/");
		sprintf(ret+strlen(ret),"%s",path);
	}
	return ret;
}

int send_dir(Server*server,Client *client)
{
	DIR *dir;
	struct dirent *dirent;
	char * real_path = web2realpath(server,client->path);
	printf("\n path: %s ==> real_path:%s\n",client->path,real_path);
	dir = opendir(real_path);
	free(real_path);
	real_path = NULL;
	Client_sends(client,"Connection: close\r\n");
	Client_sends(client,"Content-Type: text/html\r\n\r\n");
	Client_sends(client, "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html\" charset=\"UTF-8\" /><title>");
	if(dir == 0){
		Client_sends(client,strerror(errno));
		return 1;
	}

	Client_sends(client,client->path);
	Client_sends(client, "</title></head><body>");


	struct stat info;
	char* reletivepath = NULL;//相对server->root的路径
	while ((dirent = readdir(dir)) != 0) {
		if(strlen(dirent->d_name)==0)
			continue;
		reletivepath = path_conact(client->path,dirent->d_name,NULL);
		if(reletivepath==NULL)
			continue;

		//Client_sends(client, "<a href=\"http://%s/%s\">%s",client->host,reletivepath,dirent->d_name);
		char*encoded_url = url_encode(reletivepath,strlen(reletivepath),NULL,1);
		Client_sends(client, "<a href=\"http://%s/%s\">%s",client->host,encoded_url,dirent->d_name);
		if(encoded_url){
			free(encoded_url);
			encoded_url = NULL;
		}

		real_path = web2realpath(server,reletivepath);
		//printf("\n host:%s , reletivepath: %s, real_path:%s\n",client->host,reletivepath,real_path);
		if(reletivepath){
			free(reletivepath);
			reletivepath = NULL;
		}
		memset(&info,0,sizeof(info));
		if(stat(real_path, &info)!=-1) {//if the file is a directory , append a "/" ;
			if(S_ISDIR(info.st_mode)){
				Client_sends(client,"/");
			}
		}
		if(real_path)
		{
			free(real_path);
			real_path = NULL;
		}

		Client_sends(client, "</a>");
		Client_sends(client, "<br/>");
	}
	Client_sends(client, "</body></html>");
	return 0;
}

int send_upload(Client*client)
{
	char *html = 
		"<!DOCTYPE HTML>"
		"<html> "
		"	<head>"
		"		<meta charset=\"UTF-8\"/>"
		"		<title>upload</title>"
		"		<script> function changeFileName(val) { document.uploadForm.Filename.value = val;}</script>"
		"	</head> "
		"    <body > "
		"      <form name=\"uploadForm\" method=\"POST\" "
		"        enctype=\"MULTIPART/FORM-DATA\" "
		"        action=\"http://%s/%s/\"> "
		"		<p>Upload: <input type=\"file\" name=\"Filedata\" onchange=\"changeFileName(this.value.substr(this.value.lastIndexOf('\\\\')+1))\" size=\"30\"/></p>"
		"		<p>save: <input type=\"text\" name=\"Filename\" value=\"\" size=\"30\"/></p>"
		"        <input type=\"submit\" name=\"submit\" value=\"upload\"/> "
		"        <input type=\"reset\" name=\"reset\" value=\"reset\"/> "
		"      </form> "
		"    </body> "
		"</html>";
	char s[2048];
	memset(s,0,sizeof(s));
	char * dir = getHome();
	sprintf(s,html,client->host,dir);
	Client_sends(client,"Connection: close\r\n\r\n");
	Client_sends(client,s);
	return 0;
}

/**
 * 原样返回
 */
int send_echo(Client*client)
{
	char *xml = client->head;
	Client_sends(client,"Content-Type: text/text\r\n");
	Client_sends(client,"Connection: keep-alive\r\nAccept-Ranges: bytes\r\n");
	if(xml)
		Client_sends(client,"Content-Length: %d\r\n",strlen(xml));
	Client_sends(client,"\r\n");
	if(xml)
	{
		Client_sends(client,"%s",xml);
		printf("%s",xml);
		fflush(stdout);
	}
	return 0;
}

static char * crossdomain_xml =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
"<cross-domain-policy>"
"  <site-control permitted-cross-domain-policies=\"all\"/>"
"  <allow-access-from domain=\"*\" secure=\"false\"/>"
"  <allow-http-request-headers-from domain=\"*\" headers=\"*\"/>"
"</cross-domain-policy>"
;
int send_crossdomain(Client*client)
{
	Client_sends(client,"Content-Type: text/xml\r\n");
	Client_sends(client,"Connection: keep-alive\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\n",strlen(crossdomain_xml));
	Client_sends(client,"\r\n");
	Client_sends(client,"%s",crossdomain_xml);
	return 0;
}

int send_exec_end(Client*client)
{
	Parameter *paras = client->parameters;
	int num_paras= client->num_paras;
	Client_sends(client,"Accept-Ranges: bytes\r\n");
#ifdef DEBUG
	printf("has paras========\n");
#endif
	int i=0;
	char *value[0x100];
	while(i<0x100){
		value[i]=NULL;
		++i;
	}
	i=0;
	while(i<num_paras)
	{
		url_decode(paras->value,strlen(paras->value));
		if(strcmp(paras->name,"f")==0){
			value[0] = paras->value;
#ifdef DEBUG
			printf("function:%s\n",value[0]);
#endif
		}else if(strncmp(paras->name,"p",1)==0 && *(paras->name+1)!=0 && atoi(paras->name +1)>0){
			value[atoi(paras->name+1)] = paras->value;
			//printf("value[%d]:%s\n",atoi(paras->name+1),paras->value);
		}
		paras = paras->next;
		++i;
	}
	int ret = 0;
	int handle = 0;
	if(value[0]!=NULL && value[1] !=NULL){
		if(strcmp(value[0],"fwrite")==0 && value[2]!=NULL && value[3]!=NULL && value[4]!=NULL){//fwrite,file_name,opentype,data,data_byte_len
			FILE *file=fopen(value[1],value[2]);
			if(file == NULL){
				return 1;
			}
			if(atoi(value[3])<=0){
				return 3;
			}
			char *data = base64_decode(value[3],strlen(value[3]),NULL);
			ret = fwrite(data,1,atoi(value[4]),file);
			fflush(file);
			if(ret <0 || ret < atoi(value[4])){
				return 2;
			}
			/*ret=lseek(handle,0L,SEEK_END);*/
			ret= ftell(file);
			fclose(file);

			char b[16];
			memset(b,0,sizeof(b));
			vspf(b,"%d",ret);
			Client_sends(client,"Content-Length: %d\r\n",strlen(b));
			Client_sends(client,"Connection: close\r\n\r\n");
			Client_sends(client,"%d",ret);
			/*Client_sends(client,"\"}\n");*/
			if(data){
				free(data);
				data = NULL;
			}
		}else if(strcmp(value[0],"rename" )==0 && value[1]!=NULL && value[2]!=NULL){
#ifdef DEBUG
			printf("rename:\n");
#endif
			ret=unlink(value[2]);//override the file;
			ret=rename(value[1],value[2]);
			Client_sends(client,"Connection: close\r\n\r\n");
			Client_sends(client,"{\"rename\":%d}\n",ret);
		}else if(strcmp(value[0],"remove" )==0){
			Client_sends(client,"Connection: close\r\n\r\n");
			Client_sends(client,"{\"remove\":%d}\n",remove(value[1]));
		}else if(strcmp(value[0],"unlink" )==0){
			Client_sends(client,"Connection: close\r\n\r\n");
			Client_sends(client,"{\"unlink\":%d}\n",unlink(value[1]));
		}else if(strcmp(value[0],"filelength")==0){
			handle=open(value[1],O_RDONLY);
			if(handle<=0){
				ret = 0;
			}else{
				ret=lseek(handle,0L,SEEK_END);
				close(handle);
			}
			Client_sends(client,"Connection: close\r\n\r\n");
			Client_sends(client,"%d",ret);

#ifdef dict_h
		}else if(strcmp(value[0],"mean" )==0 && value[1]!=NULL){
			Dict * dict = Dict_new();
			dict->name = "oxford-gb";
			char *mean = Dict_explain(dict,value[1]);
			if(mean){
				Client_sends(client,"Content-Length: %d\r\n",strlen(mean));
				Client_sends(client,"Connection: close\r\n\r\n");
				//Client_sends(client, "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html\" charset=\"UTF-8\" /></head><body>");
				//int sended_len = 
				Client_sends(client,mean);
				//if(sended_len == strlen(mean)){ printf("========sended complete=========="); }else{ printf("========not send complete=========="); }
				free(mean);
				//Client_sends(client, "</body>");
			}else{
				mean = "not exist";
				Client_sends(client,"Content-Length: %d\r\n",strlen(mean));
				Client_sends(client,"Connection: close\r\n\r\n");
				Client_sends(client,mean);
			}
#ifdef DEBUG
			printf("%s,%d\n",mean,(int)strlen(mean));
#endif
			Dict_free(dict);
			fflush(stdout);
#endif
		}else if(strcmp(value[0],"system" )==0){
			url_decode(value[1],strlen(value[1]));
			printf("%s\n",value[1]);
			char *output=mysystem(value[1],&ret);
			if(output){
				Client_sends(client,"Content-Length: %d\r\n",ret);
			}
			Client_sends(client,"Connection: close\r\n\r\n");
			Client_sends(client,output);
			if(output){
				free(output); output = NULL;
			}
		}else if(strcmp(value[0],"nohup" )==0){
			url_decode(value[1],strlen(value[1]));
			printf("%s\n",value[1]);
			char * nohup = NULL;
#ifdef __ANDROID__
			nohup = append_str(NULL,"HOME=/sdcard nohup %s &",value[1]);
#else
			nohup = append_str(NULL,"HOME=/home/`whoami` nohup %s 2>/dev/null &",value[1]);
#endif
			if(nohup){
				ret = system(nohup);
				free(nohup);
			}
			char * output = append_str(NULL,"%d",ret);
			if(output){
				Client_sends(client,"Content-Length: %d\r\n",strlen(output));
				Client_sends(client,"Connection: close\r\n\r\n");
				Client_sends(client,output);
				free(output); output = NULL;
			}else{
				Client_sends(client,"Connection: close\r\n\r\n");
			}
#ifdef _SQLITE3_H_
		}else if(strcmp(value[0],"sql")==0){
			char * dbpath = home_resolve("test.db");
			DataBase * db = DataBase_new(dbpath);
			if(db){
				DataBase_exec(db,value[1]);
				//if(0 == DataBase_exec(db,value[1]))
				{
					Client_sends(client,"Content-Length: %d\r\n",strlen(db->result_str));
					Client_sends(client,"Connection: close\r\n\r\n");
					Client_sends(client, db->result_str);
				}
				//else{ Client_sends(client,"Connection: close\r\n\r\n"); }
				DataBase_clear(db);
				db = NULL;
			}else{
				Client_sends(client,"Connection: close\r\n\r\n");
			}
			free(dbpath);
#endif
		}else{
			Client_sends(client, "{\"c_result\":\"no such function\"}\r\n");
		}
		return 3;
	}
	return 0;
}

void Client_respond(Client*client)
{
	Server *server = client->server;
	//Parameter *paras = client->parameters;
	int num_paras= client->num_paras;
	Client_sends(client,"HTTP/1.1 200 OK\r\nServer: %s\r\n",client->host);

	url_decode(client->path,strlen(client->path));
	printf("\nclient->path:%s\n---",client->path);

	if(num_paras> 0 && (client->path[1]=='?' || strlen(client->path)==1)){//根目录请求
		send_exec_end(client); return;
	}else if(strcmp(client->path,"/")==0){
		if(server->defaulPage && strlen(server->defaulPage)>1){
			strcpy(client->path,server->defaulPage);
		}
	} else if(strcmp(client->path,"/upload")==0){
		send_upload(client);
		return;
	} else if(strcmp(client->path,"/crossdomain.xml")==0){
		send_crossdomain(client);
		return;
	} else if(strcmp(client->path,"/echo")==0){
		send_echo(client);
		return;
	}

#ifdef _SQLITE3_H_
	if(strncmp(client->path,"/sql:",4)==0){
		char * dbpath = home_resolve("test.db");
		DataBase * db = DataBase_new(dbpath);
		if(db)
		{
			db = exec_sql(db,client->path);
			if(db){
				Client_sends(client,"Content-Length: %d\r\n",strlen(db->result_str));
				Client_sends(client,"Connection: close\r\n\r\n");
				Client_sends(client, db->result_str);
				DataBase_clear(db);
			}else{
				Client_sends(client,"Connection: close\r\n\r\n");
			}
			db = NULL;
		}else{
			Client_sends(client,"Connection: close\r\n\r\n");
		}
		free(dbpath);
		return;
	}
#endif

	struct stat info;
	int len;

	len = strlen(server->root) + strlen(client->path) + 1;
	client->real_path = malloc(len + 1);
	memset(client->real_path,0, len + 1);
	if(strcmp(server->root,"/")==0){//linux abs path
		sprintf(client->real_path, "%s", client->path);
	}else{
		sprintf(client->real_path, "%s", server->root);
		while(((client->real_path[strlen(client->real_path)-1])=='\\'|| (client->real_path[strlen(client->real_path)-1])=='/') && strlen(client->real_path)>0)client->real_path[strlen(client->real_path)-1]='\0';
		sprintf(client->real_path+strlen(client->real_path), "%s", client->path);
		while(((client->real_path[strlen(client->real_path)-1])=='\\'|| (client->real_path[strlen(client->real_path)-1])=='/') && strlen(client->real_path)>0)client->real_path[strlen(client->real_path)-1]='\0';
	}

	char *end = strstr(client->real_path,"?");
	if(end) *end = '\0';

	if(strlen(client->real_path)==0) sprintf(client->real_path, "%s", ".");
#ifdef DEBUG
	//printf("\nclient->real_path:%s,path:%s\n",client->real_path,client->path);
	fflush(stdout);
#endif


	//https://ffmpeg.org/ffmpeg-devices.html#fbdev
	//ffmpeg -f gdigrab -i desktop out.mpg 
	//xdpyinfo
	//ffmpeg -f x11grab -i :0.0 -video_size 1440x900 out.mpg 
	if(strcmp(client->path,"/tmpshot.png") == 0) {
		char cmd[128];
		memset(cmd,0,sizeof(cmd));
		free(client->real_path);
		client->real_path = home_resolve(client->path);
#ifdef __ANDROID__
		sprintf(cmd,"screencap %s",client->real_path);
#else
		sprintf(cmd,"scrot %s",client->real_path);
#endif
		system(cmd);
	}
	//screencap /mnt/sdcard/tmpshot.png

	if (stat(client->real_path, &info) < 0) {//failed
		Client_sends(client,"Connection: close\r\n\r\n");
		Client_sends(client,"Error:%s path:%s errno:%d\r\n",strerror(errno),client->path,errno);
		return;
	}

	if (S_ISREG(info.st_mode)) {//file
		send_file(client);
	} else if (S_ISDIR(info.st_mode)) {//dir
		//printf("isdir");
		send_dir(server,client);
	} else {//other forbid to access
		Client_sends(client,"Conten-Type: text/text\r\n");
		Client_sends(client,"Connection: close\r\n\r\n");
		Client_sends(client,"%s forbid!",client->path);
	}
}

int open_with_browser(Server*server)
{
	char *cmd = NULL;
	char * host = "http://127.0.0.1";
#ifdef WIN32
	cmd = append_str(cmd,"cmd /c start \"\" \"%s:%d\" &",host,server->port);
#endif
#ifdef __ANDROID__
	cmd = append_str(cmd,"am start -a android.intent.action.VIEW -d %s:%d ",host,server->port);
#else
	cmd = append_str(cmd,"xdg-open '%s:%d/' &",host,server->port);
#endif
	if(cmd){
		if(strlen(cmd))
			system(cmd);
		free(cmd);
	}
	return 0;
}

#ifdef linux
void sig_handler(int sig)
{
	int s=0;
	pid_t i;
	switch(sig){
		case SIGCHLD:
			i= wait(&s);
			printf("pid:%d SIGCHLD stats:%d!\n",i,s);
			break;
		case SIGINT:
			printf("ctrl+c has been keydownd\n");
			printf("pid:%d exit!\n",getpid());
			exit(0);
			break;
	}
}
void signal_ex(int signo, void* func)
{
	struct sigaction act, old_act;
	act.sa_handler = func;
	sigemptyset(&act.sa_mask); //清空此信号集
	act.sa_flags = 0;
	if (sigaction(signo, &act, &old_act) < 0)
	{
		printf("sig err!\n");
	}
	sigaction(signo, &old_act, NULL); //恢复成原始状态
	return;
}
#endif
typedef struct Item
{
	char *start;
	char *end;
	char *name;
	char *data;
	unsigned int data_len;
	struct Item* next;
	//Content-Disposition: form-data; name="Filedata"; filename="test.png"
	//Content-Type: application/octet-stream
} Item;

int post_upload(Client*client)
{
	char * p = client->data;
	char * split = "\r\n\r\n";
	//printf("\n boundary ========== %s\n%s\n,client->content_length:%d\n",client->boundary_str,p,client->content_length);
	char *end = client->data + client->content_length;
	p = memstr(p,client->content_length,client->boundary_str);
	Array * list = NULL;
	unsigned int list_len=0;
	char *Filename =NULL;
	char *Filedata=NULL;
	unsigned int Filelen=0;
	int result=0;
	while(p && p < end){
		p += strlen(client->boundary_str);
		char *pend = memstr(p,end-p,client->boundary_str);
		if(pend){
			Item * item = (Item*)malloc(sizeof(Item));
			memset(item,0,sizeof(Item));
			list = Array_push(list,item);

			item->start = p;
			item->end= pend - strlen(split);
			*(item->end) = '\0';
			item->data= strstr(p,split);
			if(item->data){
				*(item->data)='\0';
				item->data+= strlen(split);
				item->data_len = item->end - item->data;
			}
			//printf("\nhead%d:%s",list_len,item->start);
			printf("\n(data_len:%d)",item->data_len);
			/*printf("\ndata:%s \n",item->data);*/
			char* names =" name=\"";
			char *name = strstr(p,names);
			if(name){
				item->name = name + strlen(names);
				char *name_end = strstr(name+strlen(names),"\"");
				if(name_end)*name_end='\0';

				if(strcmp(item->name,"Filename")==0){// 获取 Filename 参数值
					Filename=item->data;
					/*printf("\nFilename:%s",Filename);*/
				}else if(strcmp(item->name,"Filedata") ==0){// 获取 Filedata 参数值
					Filedata = item->data;
					Filelen = item->data_len;
					/*printf("\nFilelen:%d",Filelen);*/
					/*printf("\n Filedata:%s",Filedata);*/
				}
			}
			list_len++;
		}
		p = pend;
	}

	if( Filename && Filedata && Filelen >0)
	{
		int filenameLen = strlen(Filename)+strlen(client->path)+4;
		char filename[filenameLen];
		memset(filename,0,filenameLen);
		sprintf(filename,"%s",client->path);
		char * end = strstr(filename,"?");
		if(end)*end = '\0';
		strcat(filename,Filename);
		printf("\n write to file : %s,%d\n",filename,Filelen);
		FILE *wfile=fopen(filename,"wb");
		if(wfile){
			int len = fwrite(Filedata,1,Filelen,wfile);
			if(len==Filelen)
			{
				printf("write success!\n");
				fflush(wfile);
				Client_sends(client,"(%s) OK\n",filename);
			}else{
				printf("write failed!\n");
				Client_sends(client,"write ERRR! (%s)\n",filename);
				result=1;
			}
			fclose(wfile);
			wfile = NULL;
		}else{
			//printf("open file to write Error! (%s) \n",filename);
			Client_sends(client,"open file to write ERRR! (%s)\n",filename);
			result=2;
		}
	}
	if(list)
	{
		int i = 0;
		while(i<list->length)
		{
			Item * cur = Array_getByIndex(list,i);
			if(cur)
				free(cur);
			++i;
		}
		Array_clear(list);
		list = NULL;
	}
	//printf("post upload end !\n");
	return result;
}

Client *Client_new(Server*server)
{
	Client *client = (Client*)malloc(sizeof(Client));
	memset(client,0,sizeof(Client));
	client->addrlen = sizeof(struct sockaddr_in);
	client->addr = (struct sockaddr_in*)malloc(client->addrlen);
	memset(client->addr,0,client->addrlen);
	client->fd= accept(server->sock_fd,(struct sockaddr*)client->addr, (socklen_t *)&client->addrlen);
	if (client->fd < 0) {
		perror("accept()");
		Client_clear(client);
		return NULL;
	}
	client->server = server;
	printf("client: %s:%d\n", inet_ntoa(client->addr->sin_addr), ntohs(client->addr->sin_port));
	return client;
}

Client *Client_getAuth(Client*client,char *authorization)
{
	int Unauthorized = 1;
	if(authorization==NULL){
		Unauthorized = 1;
	}else{
		printf("Authorization ===> %s\n",authorization);
		char *Authorization = base64_decode(authorization+strlen("Basic "),strlen(authorization+strlen("Basic ")),NULL);
		if(Authorization){
			char *split = strstr(Authorization,":");
			if(split){
				*split = '\0';
				client->auth_usr = append_str(NULL,"%s",Authorization);
				client->auth_pswd = append_str(NULL,"%s",split+1);
				printf("auth_usr ===> %s\n",client->auth_usr);
				printf("auth_pswd ===> %s\n",client->auth_pswd);
				if(client->auth_usr == NULL || client->auth_pswd==NULL){
					Unauthorized = 1;
				}else if(strcmp(client->auth_usr,"test") || strcmp(client->auth_pswd,"test")){//user and password
					Unauthorized = 1;
				}else{
					Unauthorized = 0;
				}
			}else{
				Unauthorized = 1;
			}
			free(Authorization);
		}else{
			Unauthorized = 1;
		}
	}
	if(Unauthorized) {
		Client_sends(client,"HTTP/1.1 401 Unauthorized\r\n");
		Client_sends(client,"WWW-Authenticate: Basic realm=\"host\"\r\n");
		Client_sends(client,"\r\n");
		Client_clear(client);
		client = NULL;
	}
	return client;
}

Array * authed_ips = NULL; 
int Client_isAuthed(Client * client)
{
	if(authed_ips == NULL || client==NULL)
		return 0;
	//printf("client: %s:%d\n", inet_ntoa(client->addr->sin_addr), ntohs(client->addr->sin_port));
	int i=0;
	while(i< authed_ips->length)
	{
		char * ip = inet_ntoa(client->addr->sin_addr);
		char * _ip = Array_getByIndex(authed_ips,i);
		if(ip && _ip && strlen(ip)>0 && strlen(_ip)>0 && strcmp(ip,_ip)==0)
			return 1;
		++i;
	}
	return 0;
}
void Client_setAuthed(Client * client)
{
	if(client == NULL || client->addr==NULL)
		return;
	char * ip = inet_ntoa(client->addr->sin_addr);
	if(ip)
	{
		char * _ip = malloc(strlen(ip)+1);
		memset(_ip,0,strlen(ip)+1);
		strcpy(_ip,ip);
		authed_ips = Array_push(authed_ips,_ip);
	}
}


void *Client_recv(void *_client)  
{
	Client*client = (Client*)_client;
	pthread_detach(pthread_self());
	if(client==NULL)
	{
		printf("no client\n");
		pthread_exit(NULL);  
		return NULL;
	}

	int page_size = getpagesize();

	/*int i = 1; setsockopt( client->fd, IPPROTO_TCP, TCP_NODELAY, (void *)&i, sizeof(i));//立即发送*/

	int len=0;
	int num_page = 1;
	client->head = (char*)malloc(page_size*num_page);
	memset(client->head,0,page_size*num_page);
	if(client->head==NULL)
	{
		printf("not enough memory!\n");
		Client_clear(client);
		return NULL;
	}

	//printf("-\n");fflush(stdout); return 0;
	while((client->head) && strstr(client->head,"\r\n\r\n") == NULL)
	{
		if(((len=recv(client->fd,client->head+client->recved_len,page_size*num_page-client->recved_len, 0)) > 0)){
			printf("recv %d bytes!\n",len);
		}else{
			printf("recv Error\n");
			break;
		}
		client->recved_len +=len;
		if(client->recved_len == page_size*num_page){//所有页面已满,还有更多,再分配
			char*tmp = (char*)realloc(client->head,(page_size)*(++num_page));
			if(tmp != NULL){//分配成功
				client->head = tmp;
			}else{//失败
				if(client->head){
					free(client->head);
					client->head= NULL;
				}
				printf("realloc Error\n");
				break;
			}
		}else if(client->recved_len > page_size*num_page){//overflow
			client->recved_len = 0;
			Client_clear(client);
			printf("recv head overflow\n");
			break;
		}else{//recv end
			printf("recv head ok\n");
			break;
		}
		if(client->recved_len > 0x100000){//http头超过1M,退出,以防止大数据包攻击
			printf("recv head over 1 M\n");
			len = -1;
			break;
		}
	}
	if(len <= 0)
	{
		Client_clear(client);
		printf("len<=0\n");
		return NULL;
	}
	if(client->recved_len==0)
	{
		Client_clear(client);
		printf("recved_len==0\n");
		return NULL;
	}
	if(client->head== NULL){//closed
		Client_clear(client);
		printf("no head\n");
		pthread_exit(NULL);  
		return NULL;
	}
	printf("head getted\n");
#ifdef DEBUG
	printf("\nrecv:=============\n%s %d\n ======================= \n",client->head,client->recved_len);
	/*printf("\nrecv:=============\n%d,,,%d\n =======================%d \n",num_page,page_size,client->recved_len);*/
#endif
	if(strcmp(client->head,"<policy-file-request/>")==0)
	{
		Client_sends(client,crossdomain_xml);
		Client_clear(client);
		//printf("send crossdomain\n");
		pthread_exit(NULL);  
		return NULL;
	}

	char *head_end = strstr(client->head,"\r\n\r\n");
	int head_len = 0;
	if(head_end ==NULL){//头部信息接收未完
		Client_clear(client);
		printf("head not end\n");
		pthread_exit(NULL);  
		return NULL;
	}else{
		head_len = head_end - client->head+ 4;
		printf("head_len:%d\n",head_len);
	}

	if(0 == strncmp(client->head,"GET ",4)){
		client->type = 0;
		client->path = getmetavalue(client->head,"GET");
		//#ifdef DEBUG
		printf("is GET %s\n",client->path);
		//#endif
		if(strcmp(client->path,"/quit")==0)
		{
			pthread_exit(client->path);  
			return NULL;
		}
	}else if(0 == strncmp(client->head,"POST ",5)){
		client->type = 1;
		client->path = getmetavalue(client->head,"POST");
#ifdef DEBUG
		printf("is POST %s\n",client->path);
#endif
	}

	char *content_length = getmetavalue(client->head,"Content-Length"); 
	if(content_length){
		client->content_length = atoi(content_length);
		free(content_length);
	}
	printf("Content-Length:%d\n",client->content_length);



	//#ifndef DEBUG
	// Authorization: Basic a29kaTpzYmhhbWU=\r\n",
	if(Client_isAuthed(client)==0){
		char *authorization = getmetavalue(client->head,"Authorization");
		client = Client_getAuth(client,authorization);
		if(authorization) {
			free(authorization); authorization = NULL;
		}
		if(client==NULL)
		{//Unauthorized
			pthread_exit(NULL);  
			return NULL;
		}
		Client_setAuthed(client);
	}else{
		printf("authed\n");
	}
	//#endif

	if(client->content_length >0){//数据区
		int recved_data_len = client->recved_len - head_len;
		int remain_size = client->content_length - recved_data_len;
		client->data = malloc(client->content_length +1);
		memset(client->data,0,client->content_length+1);
		if(client->data == NULL){
			Client_clear(client);
			printf("data recv Error\n");
			pthread_exit(NULL);  
			return NULL;
		}
		memset(client->data,0,client->content_length +1);
		memcpy(client->data,client->head+ head_len,recved_data_len);
		while(remain_size > 0){
			if((len=recv(client->fd,client->data+recved_data_len,remain_size,0))>0){
				recved_data_len += len;
				remain_size = client->content_length - recved_data_len;
			}else{
				break;
			}
		}
		if(len <= 0 && remain_size >0 ){
			Client_clear(client);
			printf("data recv Error 2\n");
			pthread_exit(NULL);  
			return NULL;
		}
		//printf("data--------------->%s\n",client->data);

		char *_boundary = "boundary=";
		char *boundary = strstr(client->head,_boundary);
		if(boundary)
		{
			Client_sends(client,"HTTP/1.1 200 OK\r\n\r\n");

			boundary = boundary + strlen(_boundary);
			// Content-Type: multipart/form-data; boundary=----------Ij5ae0ae0KM7GI3KM7 
			char *boundary_end = boundary;
			while(isgraph(*boundary_end)){
				boundary_end++;
			}
			int boundary_len = boundary_end - boundary;
			client->boundary_str=(char*)malloc(boundary_len+1);
			if(client->boundary_str){
				memset(client->boundary_str,0,boundary_len+1);
				strncpy(client->boundary_str,boundary,boundary_len);
				//printf("boundary is %s\n",client->boundary_str);
				/*printf("data len is %d\n",client->content_length);*/
				post_upload(client);
			}
			Client_clear(client);
			pthread_exit(NULL);  
			printf("post end\n");
			return NULL;
		}
	}
	client->num_paras=0;

	if(client->path){//head 传入的参数
		char *paras_start = strstr(client->path,"?");
		if(paras_start){
			++paras_start;
			getparas(client,paras_start);
		}
		printf("head num_paras:%d\n",client->num_paras);
	}
	if(client->content_length >0 && client->boundary_str == NULL){//post数据区传入的参数
		/*printf("data:%s\n",data);*/
		getparas(client,client->data);
		printf("all num_paras:%d\n",client->num_paras);
	}
	client->host = getmetavalue(client->head,"Host");
	if(client->host)
		printf("Host:%s\n",client->host);
	if(client->path && strlen(client->path)>0)
		Client_respond(client);


	Client_clear(client);
	printf("Client_recv() end\n");
	pthread_exit(NULL);  
	return NULL;
}


int Server_recv(Server * server)
{
	if(server==NULL)
		return -1;

	while (1) {
		Client *client = NULL;
		client = Client_new(server);
		if(client==NULL){
			perror("no client!");
			continue;
		}

		pthread_t thread;//创建不同的子线程以区别不同的客户端  
		if(pthread_create(&thread, NULL, Client_recv, client)!=0)//创建子线程  
		{  
			perror("pthread_create");  
			break;
		}
		//pthread_detach(thread);
		char * thread_return;
		pthread_join(thread, (void**)&thread_return);
		if(thread_return)
		{
			break;
		}
	}
	return 0;
}

Server * Server_new(char * root,int port)
{
	Server *server = (Server*)malloc(sizeof(*server));
	memset(server,0,sizeof(Server));
	server->port = port;
	server->root = root;
	server->numClient = 1;
	char cwd[0x100];
	memset(cwd,0,0x100);
#ifdef __ANDROID__
	sprintf(cwd,"/sdcard/");
#else
	getcwd(cwd,0x100);
#endif
	if(strlen(server->root)==0)
		strcpy(server->root,cwd);
	printf("@author:db0@qq.com\n");
	printf ("port=%d back=%d dirroot=%s cwd=%s(pID:%d)\n", server->port , server->numClient, server->root,cwd, getpid());
#ifdef WIN32
	system("ipconfig");
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested=MAKEWORD(2, 2);
	int ret=WSAStartup(wVersionRequested, &wsaData);
	if(ret!=0)
	{
		printf("WSAStartup() failed!\n");
		Server_clear(server);
		return NULL;
	}
#else
	signal(SIGPIPE, SIG_IGN);
	signal_ex(SIGINT, sig_handler);
	signal_ex(SIGCHLD, sig_handler);
#endif
	if ((server->sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()");
		Server_clear(server);
		return NULL;
	}
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	//addr.sin_port = htons(0);
	//addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_addr.s_addr = INADDR_ANY;
	int addrlen = sizeof(addr);

	int opt = SO_REUSEADDR;
	setsockopt(server->sock_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));// 端口复用,有可能收到非期望数据,windows下最好注释此行

	if (bind(server->sock_fd, (struct sockaddr *) &addr, addrlen) < 0) {
		perror("bind()");
		Server_clear(server);
		return NULL;
	}
	if (listen(server->sock_fd, server->numClient) < 0) {
		perror("listen()");
		Server_clear(server);
		return NULL;
	}
	return server;
}

#ifdef debug_httpserver

void *webThread(void *ptr){

	Server*server = Server_new("/",8809);
	Server_recv(server);

	//open_with_browser(server);
	Server_clear(server);
	pthread_exit(NULL);  
	return NULL;
}
int main(int argc,char **argv)
{
#ifdef linux
	system("ip a s");
#endif
	/*
	   Server*server = Server_new("/",8809);
	   Server_recv(server);
	   Server_clear(server);
	   server=NULL;
	   */



	pthread_t thread1;
	if(pthread_create(&thread1, NULL, webThread, NULL)!=0)//创建子线程  
	{  
		perror("pthread_create");  
	}else{
		pthread_join(thread1,NULL);
	}
	return 0;
}
#endif

