/**
 *
 gcc -g -Wall -lpthread music.c main.c utf8.c sidebtns.c testwords.c update.c datas.c loading.c zip.c myfont.c btnlist.c mylist.c explain.c alert.c besier.c pictures.c sdlstring.c wordinput.c bytearray.c searhdict.c mysurface.c pinyin.c input.c array.c ease.c tween.c kodi.c sqlite.c base64.c urlcode.c filetypes.c httpserver.c httploader.c readbaidu.c ipstring.c testime.c dict.c myregex.c files.c mystring.c sprite.c read_card.c matrix.c textfield.c -lcrypto -lssl -ldl -lz -lm -I"../SDL2_mixer/" -I"../SDL2_image/" -I"../SDL2/include/" -I"../libxml/include/" -I"../SDL2_ttf/" -lsqlite3 -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_image && ./a.out
 *  
http://g.hn165.com/hnck/Home/Index
http://ozzmaker.com/2014/06/30/virtual-keyboard-for-the-raspberry-pi/
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2019-03-04
 */
#include "sprite.h"
#include "httploader.h"
#include "httpserver.h"
#include "read_card.h"
#include "update.h"
#include "searhdict.h"
#include "kodi.h"

static enum STATS {
	DICTIONARY,
	CARD,
	KODI,
	ENDS
} stats;


static void * webThread(void *ptr){
	char * s = loadUrl("http://0.0.0.0:8809/quit",NULL);
	while(s){
		printf("quit oldserver: %s\n",s);
		s = loadUrl("http://0.0.0.0:8809/quit",NULL);
	}

	//SDL_Delay(1000);

	pthread_detach(pthread_self());
	SDL_Log("webThread-----\n");
	Server * server = Server_new("/",8809);
	Server_recv(server);
	Server_clear(server);
	SDL_Log("webThread exit-----\n");
	pthread_exit(NULL);  
	return NULL;
}

void showCardTest(int b)
{
	if(cardContainer)
		cardContainer->visible = b;
	if(b){
		makeNewAsk(-1,-1);
	}
}


void droppedFile(SpriteEvent*e){
	SDL_Event* event = (SDL_Event*)(e->e);
	//case (SDL_DROPFILE):
	{      // In case if dropped file
		char * dropped_file = event->drop.file;
		// Shows directory of dropped file
		SDL_ShowSimpleMessageBox(
				SDL_MESSAGEBOX_INFORMATION,
				"File dropped on window",
				dropped_file,
				NULL	
				);

		printf("%s",dropped_file);
		//textfield = TextField_setText(textfield,dropped_file);
		//textfield->sprite->canDrag = 1;
		SDL_free(dropped_file);    // Free dropped_file memory
	}
}


void changeStats(int at)
{
	int index = stats+at;
	if(index>=ENDS)
		index=0;
	else if(index<0)
		index=ENDS-1;
	stats=index;
	printf("stats:%d\n",stats);

	showSearchDict(0);
	Kodi_initBtns(0);
	showCardTest(0);

	switch(stats)
	{
		case KODI:
			Kodi_initBtns(1);
			SDL_SetWindowTitle(stage->window, "kodi");
			break;
		case DICTIONARY:
			showSearchDict(1);
			SDL_SetWindowTitle(stage->window, "词典");
			break;
		case CARD:
			showCardTest(1);
			SDL_SetWindowTitle(stage->window, "card");
			break;
		default:
			break;
	}
}


static void keyupEvent(SpriteEvent* e){
	SDL_Event *event = e->e;
	const char * kname = SDL_GetKeyName(event->key.keysym.sym);
	if(
			strcasecmp(kname,"Menu")==0
			|| strcasecmp(kname,",")==0 
	  ){
		changeStats(1);
	}else{
		switch (event->key.keysym.sym)
		{
			case SDLK_MENU:
				changeStats(1);
				break;
			default:
				break;
		}
	}
	Stage_redraw();
}

int main(int argc, char *argv[]) {
	SDL_SetMainReady();
	Stage_init();
	//if(stage==NULL)return 0;

	pthread_t thread1;
	if(pthread_create(&thread1, NULL, webThread, NULL)!=0){  //创建子线程  
		perror("pthread_create");  
	}else{
		pthread_detach(thread1);// do not know why uncommit this line , will occur an ERROR !
		//pthread_join(thread1,NULL);
	}


	//Sprite_addEventListener(stage->sprite,SDL_KEYUP,keyupEvent); 
	Sprite_addEventListener(stage->sprite,SDL_DROPFILE,droppedFile);
	//Sprite_addEventListener(stage->sprite,SDL_MOUSEMOTION,slideEvent);
	//Sprite_addEventListener(stage->sprite,SDL_MOUSEBUTTONDOWN,slideEvent);
	//Sprite_addEventListener(stage->sprite,SDL_MOUSEBUTTONUP,slideEvent);

	/*
	   Loading_show(1,"start check update");

	   pthread_t thread2;
	   if(pthread_create(&thread2, NULL, update, NULL)!=0){  //创建查找更新子线程  
	   perror("pthread_create");  
	   }else{
	   pthread_detach(thread2);// do not know why uncommit this line , will occur an ERROR !
	//pthread_join(thread2,NULL);
	}
	*/

	SDL_SetWindowTitle(stage->window, "词典");
	//showCardTest(1);
	showSearchDict(1);













	Stage_loopEvents();
	return 0;
}
