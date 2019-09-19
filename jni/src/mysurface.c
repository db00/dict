/**
 *
 gcc -Wall -I"../SDL2_image/" -I"../SDL2_ttf" mysurface.c textfield.c utf8.c urlcode.c files.c array.c matrix.c tween.c ease.c base64.c ipstring.c sprite.c httploader.c mystring.c  -lssl -lcrypto  -lSDL2_image -lSDL2_ttf -lSDL2 -I"../SDL2/include/" -lm -D debug_httploader &&./a.out
 gcc -Wall -I"../SDL2_image/" -I"../SDL2_ttf" -I"include" -L"lib" mysurface.c textfield.c files.c array.c matrix.c tween.c ease.c base64.c ipstring.c sprite.c httploader.c mystring.c lib/libeay32.dll.a lib/libssl32.dll.a -lgdi32 -lwsock32 -lssl -lssl32 -lcrypto -lSDL2_image -lSDL2_ttf -lmingw32 -lSDL2main -lSDL2 -I"../SDL2/include/" -lm -D debug_httploader && a
 > a.txt
 */
#include "mysurface.h"
/*
   SDL_Surface * render2surface(SDL_Renderer*renderer)
   {
   SDL_Surface *surface=NULL;
   SDL_Rect viewport;

   if (!renderer) {
   return NULL;
   }

   SDL_RenderGetViewport(renderer, &viewport);
   surface = SDL_CreateRGBSurface(0, viewport.w, viewport.h, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
0x00FF0000, 0x0000FF00, 0x000000FF,
#else
0x000000FF, 0x0000FF00, 0x00FF0000,
#endif
0x00000000);
if (!surface) {
fprintf(stderr, "Couldn't create surface: %s\n", SDL_GetError());
return NULL;
}

if (SDL_RenderReadPixels(renderer, NULL, surface->format->format,
surface->pixels, surface->pitch) < 0) {
fprintf(stderr, "Couldn't read screen: %s\n", SDL_GetError());
SDL_free(surface);
return NULL;
}
return surface;
}
*/


/*
SDL_RWops * SDL_LoadFile_RW(const char * file,const char * mode,SDL_bool autoclose)
{
	FILE *fp = fopen(file, mode);
	return SDL_RWFromFP(fp, SDL_bool autoclose);
}
*/

SDL_Surface * Httploader_loadimg(char * url)
{
	URLRequest * urlrequest = Httploader_load(url);
	SDL_Surface * surface = NULL;
	if(urlrequest && urlrequest->statusCode == 200 && urlrequest->data && urlrequest->respond->contentLength){
		//if(urlrequest->respond->contentLength == strlen(urlrequest->data)) SDL_Log("repond data:\n%s\n",urlrequest->data);
		surface = (SDL_Surface*)IMG_Load_RW(SDL_RWFromConstMem(urlrequest->data, urlrequest->respond->contentLength),0);
		SDL_Log("image size: %d x %d\n",surface->w,surface->h);
		URLRequest_clear(urlrequest);
	}else{
		fprintf(stderr,"%s:%d:%d error: URLNOTFOUND %s\n",__FILE__,__LINE__,1,url);
		//Sprite_alertText("网络错误!");
		URLRequest_clear(urlrequest);
		return NULL;
	}
	return surface;
}



SDL_TimerID timerId;
Uint32 my_callbackfunc(Uint32 interval, void *param)
{
	printf("interval:%d\n",interval);fflush(stdout);
	SDL_RemoveTimer(timerId);
	Sprite * sprite = (Sprite*)param;
	//if(stage && sprite)Sprite_removeChild(sprite->parent,sprite);
	Sprite_destroy(sprite);
	Stage_redraw();
	//SDL_StartTextInput();
	//return interval;
	return 0;
}

void vibrate()
{
#ifdef __ANDROID__
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

	// retrieve the Java instance of the SDLActivity
	jobject activity = (jobject)SDL_AndroidGetActivity();

	// find the Java class of the activity. It should be SDLActivity or a subclass of it.
	//jclass clazz(env->GetObjectClass(activity));
	jclass clazz = (*env)->GetObjectClass(env, activity);

	// find the identifier of the method to call
	jmethodID method_id = (*env)->GetMethodID(env,clazz, "startVibrate", "(I)V");

	// effectively call the Java method
	(*env)->CallVoidMethod(env, activity, method_id, -1);

	// clean up the local references.
	(*env)->DeleteLocalRef(env,activity);
	(*env)->DeleteLocalRef(env,clazz);
#endif
}

void Sprite_alertText(char * s)//显示弹窗
{
	if(s==NULL)return;

#ifdef __ANDROID__
	// retrieve the JNI environment.
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

	// retrieve the Java instance of the SDLActivity
	jobject activity = (jobject)SDL_AndroidGetActivity();

	// find the Java class of the activity. It should be SDLActivity or a subclass of it.
	//jclass clazz(env->GetObjectClass(activity));
	jclass clazz = (*env)->GetObjectClass(env, activity);

	// find the identifier of the method to call
	jmethodID method_id = (*env)->GetMethodID(env,clazz, "showToast", "(Ljava/lang/String;)V");

	// effectively call the Java method
	jstring string = (*env)->NewStringUTF(env, s);
	(*env)->CallVoidMethod(env, activity, method_id, string);
	(*env)->DeleteLocalRef(env, string);

	// clean up the local references.
	(*env)->DeleteLocalRef(env,activity);
	(*env)->DeleteLocalRef(env,clazz);
#else
	SDL_StopTextInput();
	Sprite * sprite = Sprite_getChildByName(stage->sprite,"__alert___");
	if(sprite){
		Sprite_destroy(sprite);
		sprite = NULL;
	}
	sprite = Sprite_newText(s,12*stage->stage_h/320,0xffffffff,0xff0000ff);
	sprite->filter = 1;
	int slen = strlen("__alert___")+1;
	if(sprite->name)free(sprite->name);
	sprite->name = malloc(slen);
	memset(sprite->name,0,slen);
	sprintf(sprite->name,"__alert___");
	if(sprite->surface){
		sprite->x = stage->stage_w/2 - sprite->surface->w/2;
		sprite->y = stage->stage_h*.2 - sprite->surface->h/2;
	}
	if(stage->sprite)Sprite_addChild(stage->sprite,sprite);
	Stage_redraw();

	SDL_RemoveTimer(timerId);
	timerId = SDL_AddTimer(2000,my_callbackfunc,sprite);
	//SDL_Delay(2000);my_callbackfunc(2000,sprite);
#endif
}

Sprite * Sprite_newText(char *s,int fontSize,Uint32 fontColor,Uint32 bgColor)
{
	Sprite * sprite = Sprite_new();
	SDL_Color *color = uintColor(fontColor);
	SDL_Color *color2 = uintColor(bgColor);
	TTF_Font * font = getDefaultFont(fontSize);
	if(font){
		if(color2->a){
			sprite->surface = TTF_RenderUTF8_Shaded(font,s,*color,*color2);
		}else{
			sprite->surface = TTF_RenderUTF8_Solid(font,s,*color);
		}
		if(sprite->surface)
		{
			sprite->w = sprite->surface->w;
			sprite->h = sprite->surface->h;
		}
		//SDL_Log("text size:%dx%d",sprite->surface->w,sprite->surface->h);
	}else{
		SDL_Log("font ERROR!");
	}
	sprite->obj = s;
	free(color);
	free(color2);
	return sprite;
}

Sprite * Sprite_newImg(char *url)
{
	Sprite * sprite = Sprite_new();
	if(url && strlen(url)>0){
		if(strncmp(url,"http://",7)==0 || strncmp(url,"https://",8)==0 )
		{
			SDL_Surface * surface = Httploader_loadimg(url);
			if(surface)
				sprite->surface = surface;
			else{
				Sprite_destroy(sprite);
				return NULL;
			}
		}else if(url[0]=='~'){
			char * _url = decodePath(url);
			sprite->surface = IMG_Load(_url);
			free(_url);
		}else{
			sprite->surface = IMG_Load(url);
		}
		if(sprite->surface){
			sprite->w = sprite->surface->w;
			sprite->h = sprite->surface->h;
		}
	}
	sprite->obj = contact_str("",url);
	return sprite;
}


