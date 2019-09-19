/**
 *
 gcc update.c tween.c ease.c array.c textfield.c base64.c loading.c utf8.c myregex.c myfont.c urlcode.c bytearray.c zip.c ipstring.c mystring.c sprite.c files.c httploader.c matrix.c -lm -I"../SDL2/include/" -I"../SDL2_image/" -I"../SDL2_ttf/" -lSDL2_ttf -lSDL2_image -lz -lpthread -lSDL2 -lssl -lcrypto -D debug_update && ./a.out
 */
#include "update.h"

void Update_clear(UPDATE*update)
{
	if(update){
		if(update->data)
		{
			free(update->data);
		}
		free(update);
	}
}
UPDATE * Update_decode(UPDATE*update,char *data)
{
	if(data==NULL || strlen(data)==0)
		return update;
	char *p = strtok(data,"\r\n");
	while(p)
	{
		//SDL_Log("strtok:%s\n",p);
		char * s = "version:";
		char * s1 = "apk:";
		char * s2 = "modify:";
		if(strncmp(s,p,strlen(s))==0){
			update->version = p+strlen(s);
			//SDL_Log("%s == %s\n",s,update->version);
		}else if(strncmp(s1,p,strlen(s1))==0){
			update->path= p+strlen(s1);
			//SDL_Log("%s == %s\n",s1,update->path);
		}else if(strncmp(s2,p,strlen(s2))==0){
			update->modify= p+strlen(s2);
			//SDL_Log("%s == %s\n",s2,update->modify);
		}
		p = strtok(NULL,"\r\n");
	}
	return update;
}
void Update_loadApk(UPDATE*update)
{
	char *url = update->path;
	URLRequest * urlrequest = Httploader_load(url);
	if(urlrequest->statusCode == 200){
		char filename[128];
		memset(filename,0,128);
#ifdef __ANDROID__
		sprintf(filename,"/sdcard/update_%s",(char*)(strrchr(update->path,'/')+1));
#else
		sprintf(filename,"../../update_%s",(char*)(strrchr(update->path,'/')+1));
#endif
		if(writefile(filename,urlrequest->data,urlrequest->respond->contentLength)==0)
		{
			char a[1024];
			memset(a,0,1024);
			sprintf(a,"pm install -r %s &",filename);
			SDL_Log("%s\n",a);
#ifdef __ANDROID__
			//system(a);
#endif
		}
	}
}

void Update_init()
{
	char*cur_data = NULL;
#ifdef __ANDROID__
	cur_data = sdlreadfile("version.txt",NULL);
#else
	cur_data = readfile("../../assets/version.txt",NULL);
#endif
	if(cur_data){
		SDL_Log("LOCAL VERSION_DATA:%s",cur_data);
		Update_decode(&cur,cur_data);
		cur.data = cur_data;
	}else{
		SDL_Log("XXXXXXXXXX read version file ERROR!\n");
		//Update_clear(&cur);
		return;
	}
	char *url = "https://git.oschina.net/db0/SDL2apk/raw/master/assets/version.txt";
	URLRequest * urlrequest = Httploader_load(url);
	if(urlrequest && urlrequest->statusCode == 200 && urlrequest->data && strlen(urlrequest->data)){
		if(urlrequest->respond->contentLength == strlen(urlrequest->data))
		{
			if(urlrequest->data)
			{
				int len = strlen(urlrequest->data);
				char * net_data = (char*)malloc(len+1);
				memset(net_data,0,len+1);
				memcpy(net_data,urlrequest->data,len);
				Update_decode(&net,net_data);
				net.data = net_data;
			}else{
				//Update_clear(&cur);
				//Update_clear(&net);
				URLRequest_clear(urlrequest);
				return;
			}
			if(strcmp(cur.version,net.version))
			{
				SDL_Log("has new version:%s , url:%s\n",net.version,net.path);
				Update_loadApk(&net);
			}else{
				SDL_Log("no new version\n");
			}
			//SDL_Log("repond data:\n%s\n",urlrequest->data);
			//SDL_Log("repond data:\n%d\n",urlrequest->respond->contentLength);
			fflush(stdout);
		}
		URLRequest_clear(urlrequest);
		urlrequest = NULL;
	}else{
		SDL_Log("%s ERROR!\n",url);
	}
}




static int compareVersion(char * curVersion,char * newVersion)
{
	if(newVersion==NULL)
		return 0;
	if(curVersion ==NULL)
		return 0;

	Array* curVersionArr = string_split(curVersion,".");
	Array* versionArr = string_split(newVersion,".");
	int i = 0;
	while(i<versionArr->length)
	{
		if(curVersionArr->length<=i)
		{
			return 1;
			break;
		}
		if(atoi(Array_getByIndex(versionArr,i))>atoi(Array_getByIndex(curVersionArr,i)))
		{
			return 1;
			break;
		}
		++i;
	}
	Array_clear(curVersionArr);
	Array_clear(versionArr);
	return 0;
}


static int toBrowser(char * title,char * content,char * url)
{
	const SDL_MessageBoxButtonData buttons[] = {
		{
			SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
			0,
			"下载"
		},
		{
			SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
			1,
			"取消"
		},
	};

	SDL_MessageBoxData data = {
		SDL_MESSAGEBOX_INFORMATION,
		NULL, /* no parent window */
		//"change rememeber status",
		//"发现新版本",
		title,
		//"发现新版本内容",//data.message = word;
		content,
		2,
		buttons,
		NULL /* Default color scheme */
	};

	int button = -1;
	int success = 0;

	success = SDL_ShowMessageBox(&data, &button);
	if (success == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error Presenting MessageBox: %s\n", SDL_GetError());
		return -1;
	}
	SDL_Log("Pressed button: %d\n", button);

	if(button==0){//下载
		char * cmd = NULL;
#if defined(__ANDROID__)
		cmd = append_str(NULL,"am start --user 0 -a android.intent.action.VIEW -d ");
#elif defined(linux)
		cmd = append_str(NULL,"xdg-open ");
#elif defined(__MACOS__)
		cmd = append_str(NULL,"open ");
#elif defined(__WIN32__) || defined(WIN64)
		cmd = append_str(NULL,"cmd /c start ");
#else
		cmd = append_str(NULL,"cmd /c start ");
#endif
		cmd = append_str(cmd,url);
		system(cmd);
		free(cmd);
	}
	return 0;
}

int loadAndunzip(char * url,char * toDir)
{
	int len = 0;
	char * dict_zip = loadUrl(url,(size_t*)&len);
	if(dict_zip)
	{
		ByteArray * bytearray = ByteArray_new(len);
		bytearray->data = dict_zip;

		char * dir = decodePath(toDir);
		ZipFile_free(ZipFile_unzipAll(bytearray,dir));
		//free(dir);
		ByteArray_free(bytearray);
		//free(dict_zip);//bytearray 已经free掉了 dict_zip
		return 0;
	}
	return 1;
}

void compareWebAndLocal(char * url , char * local_path, char * update_url)
{
	char * path = decodePath(local_path);
	char * f = readfile(path,NULL);
	free(path);
	if(f==NULL){
		return;
	}

	char * oxford_info = loadUrl(url,NULL);
	if(oxford_info)
	{
		char * newVersion = getStrBtw(oxford_info,"version=","\x0a",0);
		free(oxford_info);
		SDL_Log("oxford version: %s\r\n",newVersion);
		int hasNewVersion = 0;
		char * curVersion = getStrBtw(f,"version=","\x0a",0);
		free(f);
		if(curVersion && strlen(curVersion)>0)
		{
			while(curVersion[strlen(curVersion)-1]=='\x0d'||curVersion[strlen(curVersion)-1]=='\x0a')
				curVersion[strlen(curVersion)-1]='\0';
		}else{
			if(newVersion)
				free(newVersion);
			if(curVersion)
				free(curVersion);
			return;
		}
		hasNewVersion = compareVersion(curVersion,newVersion);

		if(hasNewVersion){
			SDL_Log("has new oxford version: %s\r\n",newVersion);
			loadAndunzip(update_url,"~/sound/");
		}else{
			SDL_Log("no newer than oxford version: %s\r\n",curVersion);
		}
		if(newVersion)
			free(newVersion);
		if(curVersion)
			free(curVersion);
	}
}

static void * showLoading(void * params)
{
	Loading_show(atoi(params),"show loading");
	return NULL;
}

void * update(void *ptr)
{
	//Loading_show(1,"start check update");
	//https://git.oschina.net/db0/kodi/raw/master/oxford.zip
	//compareWebAndLocal("https://raw.githubusercontent.com/db00/sdl2apk/master/oxford-gb.ifo","~/sound/oxford-gb/oxford-gb.ifo","https://git.oschina.net/db0/kodi/raw/master/oxford.zip");
	compareWebAndLocal("https://git.oschina.net/db0/kodi/raw/master/oxford-gb.ifo","~/sound/oxford-gb/oxford-gb.ifo","https://git.oschina.net/db0/kodi/raw/master/oxford.zip");
	//compareWebAndLocal("https://raw.githubusercontent.com/db00/sdl2apk/master/langdao-ce-gb.ifo","~/sound/ce/langdao-ce-gb.ifo","https://git.oschina.net/db0/kodi/raw/master/ce.zip");
	compareWebAndLocal("https://git.oschina.net/db0/kodi/raw/master/oxford-gb.ifo","~/sound/oxford-gb/oxford-gb.ifo","https://git.oschina.net/db0/kodi/raw/master/oxford.zip");

	char * s = loadUrl("https://raw.githubusercontent.com/db00/sdl2apk/master/AndroidManifest.xml",NULL);
	//char * s = loadUrl("https://git.oschina.net/db0/kodi/raw/master/AndroidManifest.xml",NULL);
	if(s)
	{
		char * versionName = getStrBtw(s,"android:versionName=\"","\"",0);
		SDL_Log("\r\n app versionName:%s\r\n",versionName);
		if(versionName)
		{
			char * curVersion = "1.0.1";
			int hasNewVersion = compareVersion(curVersion,versionName);
			if(!hasNewVersion)
			{
				printf("\r\n no newer than version: %s\r\n",versionName);
				fflush(stdout);
				return NULL;
			}else{
				toBrowser("发现新版本","发现新版本,点击下载进入下载页面","https://www.pgyer.com/jEjl");
			}
			free(versionName);
		}
		free(s);
	}
	fflush(stdout);
	//Loading_show(0,"updated");
	pthread_t thread2;
	if(pthread_create(&thread2, NULL, showLoading, "0")!=0)//创建查找更新子线程  
	{  
		perror("pthread_create");  
	}else{
		pthread_detach(thread2);// do not know why uncommit this line , will occur an ERROR !
		//pthread_join(thread2,NULL);
	}
	return NULL;
}

#ifdef debug_update
int main()
{
	Update_init();
	return 0;
}
#endif


