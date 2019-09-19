/**
 *
 gcc -g -D debug_kodi -I"../SDL2/include/" -I"../SDL2_image" -I"../SDL2_ttf/" -lSDL2_ttf -lSDL2_image -lSDL2 utf8.c textfield.c httpserver.c array.c filetypes.c urlcode.c dict.c sqlite.c myfont.c zip.c -lz bytearray.c update.c tween.c ease.c sprite.c matrix.c myregex.c mysurface.c kodi.c jsonrpc.c files.c httploader.c ipstring.c mystring.c cJSON.c base64.c -lssl -lsqlite3 -lpthread -ldl -lcrypto -lm && ./a.out  
 gcc -g -D debug_kodi -Wall -I"../SDL2/include/" -I"../SDL2_image" -I"../SDL2_ttf/" -I"include" -L"lib" textfield.c httpserver.c array.c filetypes.c urlcode.c dict.c tween.c ease.c sprite.c matrix.c myregex.c regex.c -D STDC_HEADERS kodi.c jsonrpc.c files.c httploader.c ipstring.c mystring.c cJSON.c base64.c lib/libssl32.dll.a lib/libeay32.dll.a -lgdi32 -lwsock32 -lssl -lpthread -lopengl32 -lcrypto -lm  -lSDL2_ttf -lSDL2_image -lmingw32 -lSDL2main -lSDL2 && a  
http://kodi.wiki/view/JSON-RPC_API/v6
*/
#include "kodi.h"
#include "update.h"

static Sprite * container = NULL;

void *webThreadk(void *ptr){
	Server*server = Server_new("/",8809);
	Server_recv(server);
	Server_clear(server);
	//pthread_detach(pthread_self());
	pthread_exit(NULL);  
	return NULL;
}

static char * host = NULL;
static char *getHost()
{
	if(host)return host;
	char * file = "~/sound/host";
	char * ip = "192.168.1.118";
	if(!fileExists(file))
		writefile(file,ip,strlen(ip));
	char *url = decodePath("~/sound/host");
	SDL_Log("%s\n",url);
	if(url){
		size_t len;
		char *s = readfile(url,&len);
		//SDL_Log("flen:%ld\n",len);
		SDL_Log("content:%s\n",s);
		//while(s[len-1]=='\r' || s[len-1]=='\n') s[len-1]='\0';
		char * s1 = regex_replace_all(s,"/[^0-9\\.a-z]/im",""); free(s); s = s1;
		SDL_Log("host:(%s)\n",s);
		free(url);
		url = NULL;
		return s;
	}
	return NULL;
}

static int numSending=0;
void * sendUrl(void * _url)
{
	if(_url==NULL)
		return NULL;
	++numSending;
	char * url = _url;
	SDL_Log("%s\n",url);
	URLRequest *urlrequest = URLRequest_new(url);
	if(urlrequest->port == 8080){
		URLRequest_setAuthorization(urlrequest,"kodi","sbhame");
	}else if(urlrequest->port==8809){
		URLRequest_setAuthorization(urlrequest,"test","test");
	}
	urlrequest = Httploader_request(urlrequest);
	if(urlrequest->statusCode == 200){
		if(urlrequest->respond && urlrequest->data && urlrequest->respond->contentLength == strlen(urlrequest->data))
		{
			SDL_Log("repond data:\n%s\n",urlrequest->data);
			//char *s = readfile("c.json",NULL);	
			//printf("%s",s);
			fflush(stdout);
			//vibrate();
		}
	}else{
		SDL_Log("repond code :\n%d\n",urlrequest->statusCode);
	}
	free(url);
	URLRequest_clear(urlrequest);
	//SDL_Delay(1000);
	//pthread_exit(NULL);
	--numSending;
	if(numSending<0)
		numSending = 0;
	return NULL;
}

void * sendK(void * event)
{
	SpriteEvent * e = event;
	host = getHost();
	//if(e->target->parent) Sprite_addChild(e->target->parent,e->target);
	// "left", "right", "up", "down", "select", "back", "fullscreen", "pause", "stop", "volumeup", "volumedown", "mute", "play",  "volampup", "volampdown", "record",  "showtime",
	//"pageup", "pagedown", "highlight", "parentdir", "parentfolder", "previousmenu", "info", "skipnext", "skipprevious", "aspectratio", "stepforward", "stepback", "bigstepforward", "bigstepback", "chapterorbigstepforward", "chapterorbigstepback", "osd", "showsubtitles", "nextsubtitle", "cyclesubtitle", "codecinfo", "nextpicture", "previouspicture", "zoomout", "zoomin", "playlist", "queue", 
	//"zoomnormal", "zoomlevel1", "zoomlevel2", "zoomlevel3", "zoomlevel4", "zoomlevel5", "zoomlevel6", "zoomlevel7", "zoomlevel8", "zoomlevel9",
	//"number0", "number1", "number2", "number3", "number4", "number5", "number6", "number7", "number8", "number9",
	//"nextcalibration", "resetcalibration", "analogmove", "analogmovex", "analogmovey", "rotate", "rotateccw", "close", "subtitledelayminus", "subtitledelay", "subtitledelayplus", "audiodelayminus", "audiodelay", "audiodelayplus", "subtitleshiftup", "subtitleshiftdown", "subtitlealign", "audionextlanguage", "verticalshiftup", "verticalshiftdown", "nextresolution", "audiotoggledigital", "osdleft", "osdright", "osdup", "osddown", "osdselect", "osdvalueplus", "osdvalueminus", "smallstepback", "fastforward", "rewind", "playpause", "switchplayer", "delete", "copy", "move", "mplayerosd", "hidesubmenu", "screenshot", "rename", "togglewatched", "scanitem", "reloadkeymaps", "backspace", "scrollup", "scrolldown", "analogfastforward", "analogrewind", "moveitemup", "moveitemdown", "contextmenu", "shift", "symbols", "cursorleft", "cursorright", "analogseekforward", "analogseekback", "showpreset", "nextpreset", "previouspreset", "lockpreset", "randompreset", "increasevisrating", "decreasevisrating", "showvideomenu", "enter", "increaserating", "decreaserating", "togglefullscreen", "nextscene", "previousscene", "nextletter", "prevletter", "jumpsms2", "jumpsms3", "jumpsms4", "jumpsms5", "jumpsms6", "jumpsms7", "jumpsms8", "jumpsms9", "filter", "filterclear", "filtersms2", "filtersms3", "filtersms4", "filtersms5", "filtersms6", "filtersms7", "filtersms8", "filtersms9", "firstpage", "lastpage", "guiprofile", "red", "green", "yellow", "blue", "increasepar", "decreasepar", "volumeamplification", "createbookmark", "createepisodebookmark", "settingsreset", "settingslevelchange", "stereomode", "nextstereomode", "previousstereomode", "togglestereomode", "stereomodetomono", "channelup", "channeldown", "previouschannelgroup", "nextchannelgroup", "playpvr", "playpvrtv", "playpvrradio", 
	//"leftclick", "rightclick", "middleclick", "doubleclick", "longclick", "wheelup", "wheeldown", "mousedrag", "mousemove", "tap", "longpress", "pangesture", "zoomgesture", "rotategesture", "swipeleft", "swiperight", "swipeup", "swipedown", "error", "noop",
	char *action = NULL;
	char * url = NULL;
	if(regex_match(e->target->name,"go-previous\\.")){
		action = "left";
	}else if(regex_match(e->target->name,"go-next\\.")){
		action = "right";
	}else if(regex_match(e->target->name,"go-down\\.")){
		action = "down";
	}else if(regex_match(e->target->name,"go-up\\.")){
		action = "up";
	}else if(regex_match(e->target->name,"gtk-ok\\.")){
		action = "select";
	}else if(regex_match(e->target->name,"gtk-no\\.")){
		action = "back";
	}else if(regex_match(e->target->name,"fullscreen\\.")){
		action = "fullscreen";
	}else if(regex_match(e->target->name,"pause\\.")){
		action = "pause";
	}else if(regex_match(e->target->name,"media-playback-stop\\.")){
		action = "stop";
	}else if(regex_match(e->target->name,"start\\.")){
		action = "play";
	}else if(regex_match(e->target->name,"muted\\.")){
		action = "mute";
	}else if(regex_match(e->target->name,"volume-low\\.")){
		action = "volumedown";
	}else if(regex_match(e->target->name,"volume-high\\.")){
		action = "volumeup";
	}else if(regex_match(e->target->name,"open-menu\\.")){
		action = "contextmenu";
	}else if(regex_match(e->target->name,"media-record\\.")){
		action = "contextmenu";
	}else if(regex_match(e->target->name,"system-time\\.")){
		action = "showtime";
	}else if(regex_match(e->target->name,"screenshot")){
		action = "screenshot";
	}else if(regex_match(e->target->name,"go-first\\.")){
		action = "fullscreen";
		url = append_str(NULL,"http://%s:8080/jsonrpc?request={\"method\":\"Input.ExecuteAction\",\"params\":{\"action\":\"%s\"},\"id\":1,\"jsonrpc\":\"2.0\"}",host,action);
		sendUrl(url);
		action = "up";
		url = append_str(NULL,"http://%s:8080/jsonrpc?request={\"method\":\"Input.ExecuteAction\",\"params\":{\"action\":\"%s\"},\"id\":1,\"jsonrpc\":\"2.0\"}",host,action);
		sendUrl(url);
		action = "select";
		url = append_str(NULL,"http://%s:8080/jsonrpc?request={\"method\":\"Input.ExecuteAction\",\"params\":{\"action\":\"%s\"},\"id\":1,\"jsonrpc\":\"2.0\"}",host,action);
		sendUrl(url);
		pthread_exit(NULL);
	}else if(regex_match(e->target->name,"go-last\\.")){
		action = "fullscreen";
		url = append_str(NULL,"http://%s:8080/jsonrpc?request={\"method\":\"Input.ExecuteAction\",\"params\":{\"action\":\"%s\"},\"id\":1,\"jsonrpc\":\"2.0\"}",host,action);
		sendUrl(url);
		action = "down";
		url = append_str(NULL,"http://%s:8080/jsonrpc?request={\"method\":\"Input.ExecuteAction\",\"params\":{\"action\":\"%s\"},\"id\":1,\"jsonrpc\":\"2.0\"}",host,action);
		sendUrl(url);
		action = "select";
		url = append_str(NULL,"http://%s:8080/jsonrpc?request={\"method\":\"Input.ExecuteAction\",\"params\":{\"action\":\"%s\"},\"id\":1,\"jsonrpc\":\"2.0\"}",host,action);
		sendUrl(url);
		pthread_exit(NULL);
	}else if(regex_match(e->target->name,"system-shutdown\\.")){
		url = append_str(NULL,"http://%s:8080/jsonrpc?request={\"method\":\"System.Shutdown\",\"params\":{},\"id\":1,\"jsonrpc\":\"2.0\"}",host,action);
	}else if(regex_match(e->target->name,"process-stop\\.")){
		url = append_str(NULL,"http://%s:8080/jsonrpc?request={\"method\":\"Application.Quit\",\"params\":{},\"id\":1,\"jsonrpc\":\"2.0\"}",host,action);
	}else if(regex_match(e->target->name,"kodi\\.")){
		url = append_str(NULL,"http://%s:8809/?f=nohup&p1=kodi", host);
	}
	if(action)
	{
		url = append_str(NULL,"http://%s:8080/jsonrpc?request={\"method\":\"Input.ExecuteAction\",\"params\":{\"action\":\"%s\"},\"id\":1,\"jsonrpc\":\"2.0\"}",host,action);
	}
	if(url){
		sendUrl(url);
	}
#ifdef __ANDROID__
	//if(action) Sprite_alertText(action);
#endif
	pthread_exit(NULL);
	return NULL;
}
void mouseDown(SpriteEvent * e)
{
	SDL_Log("mouseDown:-----------------------------%s,%d,%d,,\n"
			,e->target->name
			,stage->mouse->x
			,stage->mouse->y
		   );

	pthread_t thread1;
	if(pthread_create(&thread1, NULL, sendK, e)!=0)//创建子线程  
	{  
		perror("pthread_create");  
	}else{
		pthread_detach(thread1);// do not know why uncommit this line , will occur an ERROR !
		//pthread_join(thread1,NULL);
	}
}

int curX = 0;
int curY = 0;
int curW = 0;
int curH = 0;
Sprite * makeBtn(char* _url,char * s)
{
	printf("%s\n",_url);

	Sprite * sprite = Sprite_newText(s,stage->stage_w/7,0xffffffff,0x0);
	if(sprite->surface)
	{
		sprite->w = sprite->surface->w;
		if(sprite->w<= stage->stage_w/8.0)
			sprite->w = stage->stage_w/7.3;
		sprite->h = sprite->w;
		if(curX + sprite->w > stage->stage_w)
		{
			curX = 0;
			curY = curY + sprite->h;
		}
		sprite->x = curX;
		sprite->y = curY;
		curX += sprite->w;
	}
	sprite->name = _url;
	Sprite_addChild(container,sprite);
	Sprite_addEventListener(sprite,SDL_MOUSEBUTTONDOWN,mouseDown);
	return sprite;
}

static int mouseDownX;
static int mouseDownY;
static int deltaX;
static int deltaY;
static int isMoving =0;

static void sendMove()
{
	//char * url = append_str(NULL,"http://%s:8809/bin/xdotool?mousemove_relative&%d&%d",host, deltaX, deltaY);
	//xdotool mousemove_relative 100 100 click 1
	//http://pc:8809/?f=system&p1=xdotool%20mousemove_relative%20%d%20%d
	//sendUrl(url); return;
	char * url = NULL;
	pthread_t thread1;
	if(deltaX >=0 && deltaY>=0)
		url = append_str(NULL,"http://%s:8809/?f=system&p1=xdotool%smousemove_relative%s%d%s%d",host,"%20","%20", deltaX*2,"%20", deltaY*2);
	else
		url = append_str(NULL,"http://%s:8809/?f=system&p1=xdotool%smousemove_relative%s--%s%d%s%d",host,"%20","%20", "%20", deltaX*2,"%20", deltaY*2);
	if(pthread_create(&thread1, NULL, sendUrl, url)!=0)//创建子线程  
	{  
		perror("pthread_create");  
	}else{
		pthread_detach(thread1);// do not know why uncommit this line , will occur an ERROR !
		//pthread_join(thread1,NULL);
	}
	deltaX = 0;
	deltaY = 0;
}
static void mousehandl(SpriteEvent*e)
{
	if(host==NULL)
		return;
	Sprite * sprite = (Sprite *)e->target;
	SDL_Event * event = (SDL_Event*)e->e;
	pthread_t thread1;
	char * url = NULL;
	switch(e->type){
		case SDL_MOUSEBUTTONDOWN:
			mouseDownX = event->button.x;
			mouseDownY = event->button.y;
			deltaX = 0;
			deltaY = 0;
			isMoving = 0;
			break;
		case SDL_MOUSEBUTTONUP:
			if(
					abs(mouseDownX - event->button.x) < 3
					&& abs(mouseDownY - event->button.y)<3
					&& isMoving==0
			  )	
			{
				if(mouseDownX<stage->stage_w*2/3)
					url = append_str(NULL,"http://%s:8809/?f=system&p1=xdotool%sclick%s1",host, "%20","%20");
				else
					url = append_str(NULL,"http://%s:8809/?f=system&p1=xdotool%sclick%s3",host, "%20","%20");
				//char * url = append_str(NULL,"http://%s:8809/bin/xdotool?click&1",host);
				//sendUrl(url); return;
				if(pthread_create(&thread1, NULL, sendUrl, url)!=0)//创建子线程  
				{  
					perror("pthread_create");  
				}else{
					pthread_detach(thread1);// do not know why uncommit this line , will occur an ERROR !
					//pthread_join(thread1,NULL);
				}
			}
			if(deltaX || deltaY){
				sendMove();
			}
			break;
		case SDL_MOUSEMOTION:
			//if(e->target->parent) Sprite_addChildAt(e->target->parent,e->target,0);
			if(event->motion.state){
				sprite->rotationX += event->motion.yrel;
				sprite->rotationY += event->motion.xrel;
				Stage_redraw();

				deltaX += event->motion.xrel;
				deltaY += event->motion.yrel;
				if(abs(mouseDownX - event->motion.x)>=2 ||
						abs(mouseDownY - event->motion.y)>=2)
					isMoving = 1;

				if(numSending<=1 && (deltaX || deltaY)){
					sendMove();
				}
			}
			break;
	}
}

static void showEearth()
{
	Sprite*sprite = Sprite_new();
	char sname[] = "earth";
	sprite->name = malloc(sizeof(sname)+1);
	memset(sprite->name,0,sizeof(sname)+1);
	strcpy(sprite->name,sname);
	sprite->is3D = 1;

	char * earthPath = decodePath("~/sound/1.bmp");
	if(!fileExists("~/sound/1.bmp"))
	{
		loadAndunzip("https://git.oschina.net/db0/kodi/raw/master/earth.zip","~/sound/");
	}
	sprite->surface = IMG_Load(earthPath);
	free(earthPath);
	Data3d*_data3D = sprite->data3d;
	if(_data3D==NULL){
		_data3D = (Data3d*)malloc(sizeof(Data3d));
		memset(_data3D,0,sizeof(Data3d));

		if(_data3D->programObject==0){
			Data3d *data2D = Data3D_init();
			Data3d_set(_data3D,data2D);
		}
		sprite->data3d = _data3D;
		_data3D->numIndices = esGenSphere ( 20, 0.5f, &_data3D->vertices, &_data3D->normals, &_data3D->texCoords, &_data3D->indices);
		//_data3D->numIndices = esGenSphere ( 20, 0.5f, &_data3D->vertices, NULL, &_data3D->texCoords, &_data3D->indices);
		//_data3D->numIndices = esGenCube( 0.75f, &_data3D->vertices, &_data3D->normals, &_data3D->texCoords, &_data3D->indices);
	}
	//sprite->filter = 1;
	sprite->x = 0;
	sprite->y = 0;
	sprite->alpha = 0.9;
	//sprite->z = -100;
	sprite->w = stage->stage_w - sprite->x*2;
	sprite->h = stage->stage_h - sprite->y*2;
	Sprite*sprite2 = Sprite_new();
	sprite2->x =stage->stage_w/2;
	sprite2->y =stage->stage_h/2 + 50;
	Sprite_addChildAt(container,sprite2,0);
	Sprite_addChild(sprite2,sprite);
	Sprite_addEventListener(sprite,SDL_MOUSEMOTION,mousehandl);
	Sprite_addEventListener(sprite,SDL_MOUSEBUTTONDOWN,mousehandl);
	Sprite_addEventListener(sprite,SDL_MOUSEBUTTONUP,mousehandl);
}

static Tween * tween = NULL;
void setTweenToNull(void *p){
	tween = NULL;
}

int Kodi_initBtns(int v)
{
	if(host==NULL){
		host = getHost();
		container = Sprite_new();
		container->surface = Surface_new(1,1);
		char pixels[4] ={'\0','\0','\0','\xff'};
		memcpy(container->surface->pixels,(char*)pixels,sizeof(pixels));
		container->w= stage->stage_w;
		container->h= stage->stage_h;

		//↑↓←→
		//⇐⇑⇒⇓
		//↑↓←→↖↗↘↙↔↕➻➼➽➸➳➺➻➴➵➶➷➹▶►▷◁◀◄«»➩➪➫➬➭➮➯➱⏎➲➾➔➘➙➚➛➜➝➞➟➠➡➢➣➤➥➦➧➨↚↛↜↝↞↟↠↠↡↢↣↤↤↥↦↧↨⇄⇅⇆⇇⇈⇉⇊⇋⇌⇍⇎⇏⇐⇑⇒⇓⇔⇖⇗⇘⇙⇜↩↪↫↬↭↮↯↰↱↲↳↴↵↶↷↸↹☇☈↼↽↾↿⇀⇁⇂⇃⇞⇟⇠⇡⇢⇣⇤⇥⇦⇧⇨⇩⇪↺↻⇚⇛
		//▂▃▄▅▆▇█▉▊▋▌▍▎▏
		//♚　♛　♝　♞　♜　♟　♔　♕　♗　♘　♖　♙
		//㊣㊎㊍㊌㊋㊏㊐㊊㊚㊛㊤㊥㊦㊧㊨㊒㊞㊑㊓㊔㊕㊖㊗㊘㊜㊝㊟㊠㊡㊢㊩㊪㊫㊬㊭㊮㊯㊰㊙㉿囍



		makeBtn("media-playback-pause.png","║");
		makeBtn("go-up.png","⇧");
		makeBtn("gtk-no.png","×");
		makeBtn("audio-volume-muted.png","♬");
		makeBtn("open-menu.png","▤");
		makeBtn("preferences-system-time.png","Ⅷ");
		makeBtn("system-shutdown.png","×");
		makeBtn("go-previous.png","↞");
		makeBtn("gtk-ok.png","√");
		makeBtn("go-next.png","↠");
		makeBtn("audio-volume-low.png","➷");
		makeBtn("media-playback-start.png","▶");
		makeBtn("media-record.png","✍");
		makeBtn("applets-screenshooter.png","✄");
		makeBtn("media-playback-stop.png","◎");
		makeBtn("go-down.png","⇩");
		makeBtn("view-fullscreen.png","✠");
		makeBtn("audio-volume-high.png","➹");
		makeBtn("kodi.png","㊣");
		makeBtn("process-stop.png","✘");
		makeBtn("go-first.png","⇤");
		makeBtn("go-last.png","⇥");

		showEearth();
	}
	if(!v && stage->sprite == container->parent){
		/*
		   if(tween)
		   Tween_kill(tween,1);
		   TweenObj * tweenObj = (TweenObj*)TweenObj_new(container);
		   tweenObj->end->x=stage->stage_w;
		   tweenObj->start->x=0;
		   tween = tweenlite_to(container,500 ,tweenObj);
		   tween->ease = easeInOut_linear;
		   */

		Sprite_removeChild(stage->sprite,container);
		return 0;
	}else{
		container->alpha = 0;
		container->visible = 0;
		Sprite_addChild(stage->sprite,container);

		if(tween)
			Tween_kill(tween,1);
		TweenObj * tweenObj = (TweenObj*)TweenObj_new(container);
		tweenObj->start->alpha=0;
		tweenObj->start->x=stage->stage_w;
		tweenObj->end->x=0;
		tweenObj->end->alpha=1.0;
		tween= tween_to(container,500 ,tweenObj);
		tween->onComplete = setTweenToNull;
		tween->ease = easeInOut_quint;
		container->visible = 1;
	}
	return 1;
}

#ifdef debug_kodi
int main(int argc, char *argv[])
{
	Stage_init();


	pthread_t thread1;
	if(pthread_create(&thread1, NULL, webThreadk, NULL)!=0)//创建子线程  
	{  
		perror("pthread_create");  
	}else{
		pthread_detach(thread1);// do not know why uncommit this line , will occur an ERROR !
		//pthread_join(thread1,NULL);
	}
	//char *s = readfile("c.json",NULL);	printf("%s",s); //
	/*
	   Method * methodlist = NULL;
	   char * _url = append_str(NULL,"http://%s:8080/jsonrpc",host);
	   URLRequest *urlrequest = URLRequest_new(_url);
	   free(_url);
	   URLRequest_setAuthorization(urlrequest,"kodi","sbhame");
	   urlrequest = Httploader_request(urlrequest);
	   if(urlrequest->statusCode == 200){
	   if(urlrequest->respond->contentLength == strlen(urlrequest->data))
	   {
	   printf("repond data:\n%s\n",urlrequest->data);
	   cJSON* pRoot = cJSON_Parse(urlrequest->data);
	   if(pRoot){
	   cJSON *child = pRoot->child;
	   while(child){
	   methodlist = Jsonrpc_print_node(methodlist,child);
	   child = child->next;
	   }
	   cJSON_Delete(pRoot);
	   pRoot = NULL;
	   }
	   printf("%s",cJSON_Print(pRoot));
	   printf("repond data:\n%d\n",urlrequest->respond->contentLength);
	   fflush(stdout);
	   Method_clear(methodlist);
	   methodlist = NULL;
	   }
	   }
	   URLRequest_clear(urlrequest);
	   urlrequest = NULL;
	   */
	Kodi_initBtns(1);

	Stage_loopEvents();
	exit(0);
	return 0;
}
#endif
