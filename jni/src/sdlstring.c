/**
 * @file mystring.c
 gcc sdlstring.c mystring.c array.c -lSDL2  -I"../SDL2/include/" -lm -Ddebug_sdlstrings && ./a.out
 gcc sdlstring.c mystring.c array.c -lmingw32 -lSDL2main -lSDL2  -I"../SDL2/include/" -Dsdldebug_strings && a  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-07-22
 */


#include "sdlstring.h"

char*getCompiledVersionString()
{
	char*show_str=NULL;

	SDL_version compiled;
	SDL_VERSION(&compiled);
	show_str=append_str(show_str,
			"%d.%d.%d.%d (%s)\n",
			compiled.major,
			compiled.minor,
			compiled.patch,
			SDL_REVISION_NUMBER,
			SDL_REVISION);
	return show_str;
}

char*getLinkedVersionString()
{
	char*show_str=NULL;
	SDL_version linked;
	SDL_GetVersion(&linked);
	show_str=append_str(show_str,
			"%d.%d.%d.%d (%s)\n",
			linked.major,
			linked.minor,
			linked.patch,
			SDL_GetRevisionNumber(),
			SDL_GetRevision());
	return show_str;
}

const char*getPlatformString()
{
	return SDL_GetPlatform();
}



int setClipboardText(char *s)
{
#ifdef __ANDROID__
	return SDL_SetClipboardText(s);
	//char * cmd = append_str(NULL,"service call clipboard 2 i32 1 i32 %d s16 %s",strlen(s),s);
	//return system(cmd);
#endif
	return SDL_SetClipboardText(s);
}


char * getClipboardText(int showAlert)
{
	//#ifdef __ANDROID__
	//printf("getClipboardText!\n");
	char * s = SDL_GetClipboardText();
	//if(SDL_HasClipboardText())
	if(s!=NULL && strlen(s)>0)
	{
		//printf("hasClipboardText\n");
		return s;
	}else if(showAlert){
		int success = SDL_ShowSimpleMessageBox(
				SDL_MESSAGEBOX_ERROR,
				"Error",
				//"no text in clipboard",
				"剪切版无文本",
				NULL);
		if (success == -1) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error Presenting MessageBox: %s\n", SDL_GetError());
		}
	}
	//char * outStr = malloc(SDL_MAX_LOG_MESSAGE+1);
	//memset(outStr,0,SDL_MAX_LOG_MESSAGE+1);
	//mysystem("service call clipboard 1",outStr,SDL_MAX_LOG_MESSAGE); 
	//return outStr;
	//#endif
	return NULL;
}

void printfStatus()
{
	char * s = NULL;
	s = append_str(s,"hello");
	s = append_str(s,"hello");
	SDL_Log("%s",s);
	SDL_Log("\n");
	SDL_Log("%s",SDL_GetPlatform());
	SDL_Log("\n");
	SDL_Log("cpu cache line size:%d\n",SDL_GetCPUCacheLineSize());
	SDL_Log("cpu count:%d\n",SDL_GetCPUCount());
	SDL_Log("system ram:%d M\n",SDL_GetSystemRAM());
	SDL_Log("cpu has 3dnow:%d\n",SDL_Has3DNow());
	SDL_Log("cpu has avx:%d\n",SDL_HasAVX());
	//SDL_Log("cpu has avx2:%d\n",SDL_HasAVX2());
	SDL_Log("cpu has altivec:%d\n",SDL_HasAltiVec());
	SDL_Log("cpu has mmx:%d\n",SDL_HasMMX());
	SDL_Log("cpu has sse:%d\n",SDL_HasSSE());
	SDL_Log("cpu has sse2:%d\n",SDL_HasSSE2());
	SDL_Log("cpu has sse3:%d\n",SDL_HasSSE3());
	SDL_Log("cpu has sse41:%d\n",SDL_HasSSE41());
	SDL_Log("cpu has sse42:%d\n",SDL_HasSSE42());
	SDL_Log("cpu has rdtsc:%d\n",SDL_HasRDTSC());
	SDL_Log("base path:%s\n",SDL_GetBasePath());
	SDL_Log("pref path:%s\n",SDL_GetPrefPath("org.libsdl.app","files"));
	//SDL_Log("SDL_GetPerformanceCounter:0x%lx\n",SDL_GetPerformanceCounter());
	//SDL_Log("SDL_GetPerformanceFrequency:0x%lx\n",SDL_GetPerformanceFrequency());

	fflush(stdout);
	SDL_Log("clipboardtext:%s\n",getClipboardText(0));

	int secs, pct , powerstatus; 
	powerstatus = SDL_GetPowerInfo(&secs, &pct);
	switch(powerstatus)
	{
		case SDL_POWERSTATE_CHARGING:
			SDL_Log("Battery is charging\n");
			break;
		case SDL_POWERSTATE_CHARGED:
			SDL_Log("Battery is charged\n");
			break;
		case SDL_POWERSTATE_ON_BATTERY:
			SDL_Log("Battery is draining: ");
			if (secs == -1) {
				SDL_Log("(unknown time left)\n");
			} else {
				SDL_Log("(%d seconds left)\n", secs);
			}
			break;
		case SDL_POWERSTATE_UNKNOWN:
			SDL_Log("(SDL_POWERSTATE_UNKNOWN)\n");
			break;
	}

	if (pct == -1) {
		SDL_Log("(powerstatus: unknown percentage left)\n");
	} else {
		SDL_Log("(powerstatus: %d percent left)\n", pct);
	}



	/*
#include "SDL_loadso.h"
	// Variable declaration
	void* myHandle = NULL;
	char* myFunctionName = "myFancyFunction";
	void (*myFancyFunction)(int anInt);
	// Dynamically load mylib.so
	myHandle = SDL_LoadObject("mylib.so");
	// Load the exported function from mylib.so
	// The exported function has the following prototype
	// void myFancyFunction(int anInt);
	myFancyFunction = (void (*)(int))SDL_LoadFunction(myHandle, myFunctionName);
	// Call myFancyFunction with a random integer
	if (myFancyFunction != NULL) {
	myFancyFunction(15);
	} else {
	// Error handling here
	}
	*/
	if(s)
		free(s);
	fflush(stdout);
}


#ifdef debug_sdlstrings

int main(int argc, char *argv[])
{
	printfStatus();

	return 0;
}
#endif
