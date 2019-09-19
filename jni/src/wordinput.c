#include "wordinput.h"
#include "alert.h"

static Sprite * curlistSprite = NULL;
static Word * curSelectWord;

static void * showLoading(void * params)
{
	Loading_show(atoi(params),"show loading");
	return NULL;
}

static void open_dict()
{
	if(ec_dict==NULL)
	{
		ec_dict = Dict_new();
		ec_dict->name = "oxford-gb";
		if(!fileExists("~/sound/oxford-gb/"))
		{
			pthread_t thread2;
			if(pthread_create(&thread2, NULL, showLoading, "1")!=0)//创建查找更新子线程  
			{  
				perror("pthread_create");  
			}else{
				pthread_detach(thread2);// do not know why uncommit this line , will occur an ERROR !
				//pthread_join(thread2,NULL);
			}

			//Loading_show(1,"loading oxford ......");
			loadAndunzip("https://git.oschina.net/db0/kodi/raw/master/oxford.zip","~/sound/");
			if(pthread_create(&thread2, NULL, showLoading, "0")!=0)//创建查找更新子线程  
			{  
				perror("pthread_create");  
			}else{
				pthread_detach(thread2);// do not know why uncommit this line , will occur an ERROR !
				//pthread_join(thread2,NULL);
			}
		}
	}
	//Loading_show(0,NULL);
}

int isCE(char * value)
{
	//if(value && 0==regex_match(value,"/^[a-z0-9()\\\/-]*[\r\n]*$/i"))
	if(value && strlen(value) && 0==regex_match(value,"/^[\x1-\x7f]*[\r\n]*$/i"))
	{
		if(ce_dict==NULL)
		{
			ce_dict = Dict_new();
			ce_dict->name = "ce";
			if(!fileExists("~/sound/ce/langdao-ce-gb.ifo"))
			{
				pthread_t thread2;
				if(pthread_create(&thread2, NULL, showLoading, "1")!=0)//创建查找更新子线程  
				{  
					perror("pthread_create");  
				}else{
					pthread_detach(thread2);// do not know why uncommit this line , will occur an ERROR !
					//pthread_join(thread2,NULL);
				}
				//Loading_show(1,"loading ce ......");
				loadAndunzip("https://git.oschina.net/db0/kodi/raw/master/ce.zip","~/sound/");
				if(pthread_create(&thread2, NULL, showLoading, "0")!=0)//创建查找更新子线程  
				{  
					perror("pthread_create");  
				}else{
					pthread_detach(thread2);// do not know why uncommit this line , will occur an ERROR !
					//pthread_join(thread2,NULL);
				}
			}
		}
		return 1;
	}
	return 0;
}

int Wordinput_getHeight()
{
	if(input)
		return input->sprite->h;
	return 0;
}


void Wordinput_setText(char * s)
{
	if(input)
	{
		if(s==NULL)
			s = "";
		Input_setText(input,s);
	}
}

Sprite * makeWordBtn(Word * word,void (*selectedEvent)(SpriteEvent *))
{
	int fontSize = 20*stage->stage_h/320;
	//printf("fontSize: (%d===%d)", fontSize,h);
	Sprite * sprite = Sprite_newText(word->word,fontSize,0x0,0xffffffff);
	sprite->obj= word;
	//sprite->filter = 0;
	if(sprite->name)
		free(sprite->name);
	sprite->name = contact_str("",word->word);
	Sprite_addEventListener(sprite,SDL_MOUSEBUTTONDOWN,selectedEvent);
	Sprite_addEventListener(sprite,SDL_MOUSEBUTTONUP,selectedEvent);
	return sprite;
}

void List_removeOuts(Sprite * curlistSprite)
{
	if(curlistSprite->children && curlistSprite->children->length>0)
	{
		Sprite * lastSprite = NULL;
		lastSprite = Sprite_getChildByIndex(curlistSprite,0);
		while(lastSprite->y + lastSprite->h + curlistSprite->y<0)
		{
			//Word * _word = lastSprite->obj;
			Sprite_removeChildAt(curlistSprite,0);
			Sprite_destroy(lastSprite);
			lastSprite = Sprite_getChildByIndex(curlistSprite,0);
		}

		lastSprite = Sprite_getChildByIndex(curlistSprite,curlistSprite->children->length-1);
		while(lastSprite->y + curlistSprite->y > stage->stage_h)
		{
			//Word * _word = lastSprite->obj;
			Sprite_removeChild(curlistSprite,lastSprite);
			Sprite_destroy(lastSprite);
			lastSprite = Sprite_getChildByIndex(curlistSprite,curlistSprite->children->length-1);
		}
	}
	Stage_redraw();
}

static int isInputRegexp()
{
	if(input && input->value && regex_match(input->value,"/^\\/.*\\/[img]*$/"))
		return 1;
	return 0;
}


static Word * getWordFromDict(char * _word)
{
	if(isCE(_word)){
		return Dict_getWord(ce_dict,_word);
	}
	return Dict_getWord(ec_dict,_word);
}
static int add_history(char *word)
{
	if(history_db && strlen(word)>1 && regex_match(word,"/^[a-z-]*$/i")) 
	{
		add_new_word(word,time(NULL));
	}
	return 0;
}

static int getMean(Word * word)
{
	removePictures();

	if(word==NULL)
		return 0;
	Dict * dict = ec_dict;
	if(isCE(word->word)){
		dict = ce_dict;
	}

	Wordinput_hideList();

	if(word->word)
	{
		if(!isInputRegexp())
			Input_setText(input,word->word);
		add_history(word->word);
	}
	Sidebtns_hide();

	curSelectWord = word;

	Explain_show(dict,word);
	return 0;
}


static Word * _curWord;
static void * callbackFunc(void * v)
{
	AlertItem * item = v;

	if(strcmp("加入熟词",item->str)==0){
		SDL_Log("%s,%s",item->str,_curWord->word);
		add_remembered_word(_curWord->word,1);
	}else if(strcmp("加入生词",item->str)==0){
		SDL_Log("%s,%s",item->str,_curWord->word);
		add_remembered_word(_curWord->word,0);
	}
	return NULL;
}


static int button_messagebox(Word * _word)
{
	_curWord = _word;
	Array * middleBtns = AlertItem_push(NULL,AlertItem_new("加入熟词",callbackFunc));
	AlertItem_push(middleBtns,AlertItem_new("加入生词",callbackFunc));
	Array * bottomBtns = NULL;
	bottomBtns = AlertItem_push(NULL,AlertItem_new("取消",callbackFunc));
	//Alert_show("hello",middleBtns,bottomBtns,5000);
	Alert_show(_word->word,middleBtns,bottomBtns,0);



	/*

	   char * word = _word->word;
	   if (word == NULL || strlen(word)==0) {
	   return 1;
	   }
	   const SDL_MessageBoxButtonData buttons[] = {
	   {
	   (STATS==NEW)?SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT:SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
	   0,
	//"remembered"
	"加入熟词"
	},
	{
	(STATS==REMEMBERED)?SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT:SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
	1,
	//"not remebered"
	"加入生词"
	},
	};

	SDL_MessageBoxData data = {
	SDL_MESSAGEBOX_INFORMATION,
	NULL, 
	//"change rememeber status",
	"改变单词状态",
	word,//data.message = word;
	2,
	buttons,
	NULL 
	};

	int button = -1;
	int success = 0;

	success = SDL_ShowMessageBox(&data, &button);
	if (success == -1) {
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error Presenting MessageBox: %s\n", SDL_GetError());
	return 1;
	}

	SDL_Log("Pressed button: %d, %s\n", button, button == 0 ? "熟" : "生");

	if(button==0){
	add_remembered_word(word,1);
	}else if(button==1){
	add_remembered_word(word,0);
	}
	return button;
	*/

	return 0;
}

static int stageMouseY =0;
static int timestamp = 0;
static void selectedEvent(SpriteEvent*e)
{
	if(dictContainer==NULL || dictContainer->visible==0)
		return;
	if(e==NULL)
		return;

	Sprite*sprite = e->target;
	SDL_Event* event = e->e;

	if(sprite==NULL || sprite->parent!=curlistSprite)
		return;

	Word * word = sprite->obj;
	if(word==NULL)
		return;
	//printf("---------------------%s\n",word->word);
	switch(e->type)
	{
		case SDL_MOUSEBUTTONDOWN:
			timestamp = event->button.timestamp;
			//printf("timestamp:%d\n",timestamp);fflush(stdout);
			stageMouseY = event->button.y;
			sprite->parent->obj= NULL;
			return;
			break;
		case SDL_MOUSEBUTTONUP:
			if(sprite->parent->obj)
				return;
			if(abs(stageMouseY - event->button.y)>20)
				return;
			break;
	}
	int time_diff = event->button.timestamp-timestamp;
	if(time_diff<500)
	{
		Sprite_removeEventListener(sprite,e->type,selectedEvent);
		getMean(word);
	}else{
		//printf("time_diff:%d\n",time_diff);fflush(stdout);
		button_messagebox(word);
	}
}


static Sprite * appendWordBtn(Word * word,int end)
{
	Sprite * sprite = makeWordBtn(word,selectedEvent);
	return List_AddItem(curlistSprite,sprite,end);
}

static Word * _getWordByIndex(char * curWord,int index)
{
	if(isCE(curWord)){
		return Dict_getWordByIndex(ce_dict,index); 
	}
	return Dict_getWordByIndex(ec_dict,index); 
}

void Wordinput_changeWordList()
{
	if(ec_dict==NULL)
	{
		open_dict();
	}
	int isReg = isInputRegexp();
	char * curWord = input->value;
	int isC_E= isCE(curWord);
	if(curlistSprite->children==NULL || curlistSprite->children->length==0)
	{//first match
		Word * word = NULL;
		int curIsNotReg = 1;
		if(isReg){
			if(curSelectWord && regex_match(curSelectWord->word,input->value)){
				word = curSelectWord;
				curIsNotReg = 0;
			}else if(isC_E){
				word = Dict_getWordByRegIndex(ce_dict,input->value,0); 
			}else{
				word = Dict_getWordByRegIndex(ec_dict,input->value,0); 
			}
		}else{
			word = getWordFromDict(curWord);
		}
		if(word){
			Sprite * first = appendWordBtn(word,1);
			if(isReg && curIsNotReg) first->name = append_str(NULL,"first");
		}else{
			return;
		}
	}
	if(curlistSprite->children && curlistSprite->children->length>0)
	{
		Sprite * lastSprite = NULL;
		lastSprite = Sprite_getChildByIndex(curlistSprite,0);
		if(strcmp(lastSprite->name,"only")==0) return;
		while(lastSprite->y + curlistSprite->y>0)
		{//first button
			Word * _word = lastSprite->obj;
			int index = _word->index;
			if(index<=1)
				break;
			Word * word = NULL;
			if(isReg){
				if(strcmp(lastSprite->name,"first")==0) break;
				Word * p;
				p = _getWordByIndex(curWord,index-1); 
				if(isC_E){
					word = Dict_getWordByRegWordPrev(ce_dict,input->value,p); 
				}else{
					word = Dict_getWordByRegWordPrev(ec_dict,input->value,p); 
				}
			}else{
				word = _getWordByIndex(curWord,index-1); 
			}
			if(word){
				appendWordBtn(word,0);
				lastSprite = Sprite_getChildByIndex(curlistSprite,0);
			}else{
				if(isReg) lastSprite->name = append_str(NULL,"first");
				break;
			}
		}
		lastSprite = Sprite_getChildByIndex(curlistSprite,curlistSprite->children->length-1);

		while(lastSprite && lastSprite->y + curlistSprite->y + lastSprite->h < stage->stage_h)
		{//last button
			Word * _word = lastSprite->obj;
			int index = _word->index + 1;
			if(isC_E){
				if(index>= ce_dict->wordcount)
					break;
			}else{
				if(index>= ec_dict->wordcount)
					break;
			}
			Word * word = NULL;
			if(isReg){
				if(lastSprite->name && strcmp(lastSprite->name,"last")==0) break;
				if(isC_E){
					word = Dict_getWordByRegIndex(ce_dict,input->value,index-1); 
				}else{
					word = Dict_getWordByRegIndex(ec_dict,input->value,index-1); 
				}
				if(word==NULL){
					if(strcmp(lastSprite->name,"first")==0){
						lastSprite->name = append_str(NULL,"only"); break;
					}else{
						lastSprite->name = append_str(NULL,"last"); break;
					}
				}
			}else{
				word = _getWordByIndex(curWord,index);

			}
			if(word){
				appendWordBtn(word,1);
				lastSprite = Sprite_getChildByIndex(curlistSprite,curlistSprite->children->length-1);
			}else{
				break;
			}
		}
	}
	List_showOptical(curlistSprite);
}

static void onListMove(void * p)
{
	if(STATS == DICT)
	{
		Btnlist_hide();
		Wordinput_changeWordList();
	}else{
		Wordinput_hideList();
		Btnlist_changeHistoryList();
	}
}

static void mouseMoves(SpriteEvent*e)
{
	if(Sprite_getVisible(dictContainer)==0)
		return;
	SDL_StopTextInput();
	Sprite*target = e->target;
	if(target==NULL)
		return;

	if(target!=curlistSprite)
		return;

	SDL_Event* event = e->e;
	if(event->type!= SDL_MOUSEMOTION)
		return;

	if(event->motion.state){
		//if(abs(event->motion.xrel)<20 && abs(event->motion.yrel)<20)
		target->x += event->motion.xrel;
		target->y += event->motion.yrel;

		int stageSize = stage->stage_w>stage->stage_h?stage->stage_w:stage->stage_h;
		if(abs(stageMouseY-event->motion.y)>stageSize/320 || abs(event->motion.xrel)>stageSize/320){
			target->obj= e;
		}
		Sprite_limitPosion(target,target->dragRect);
		if(target==curlistSprite)
		{
			onListMove(NULL);
		}
	}
}
static void list_show()
{
	if(curlistSprite==NULL){
		int numWords = 10;
		int fontSize = 16;
		curlistSprite = Sprite_new();
		SDL_Rect * rect = malloc(sizeof(*rect));
		rect->x = curlistSprite->x;
		rect->w = 0;
		rect->y = -(((unsigned int)-1)/4);
		rect->h = (((unsigned int)-1)/2);
		curlistSprite->dragRect = rect;
		curlistSprite->surface = Surface_new(stage->stage_w,numWords*fontSize*1.5);
		Sprite_addChildAt(dictContainer,curlistSprite,0);
		Sprite_addEventListener(curlistSprite,SDL_MOUSEMOTION,mouseMoves);

		//List_roll(curlistSprite,onListMove);
	}
	if(curlistSprite){
		Sprite_removeChildren(curlistSprite);
		curlistSprite->x = 0;
		curlistSprite->y = Wordinput_getHeight();
	}
	curlistSprite->visible = 1;

	Explain_hide();
	Stage_redraw();
}

static void textChangFunc(Input * input){
	if(STATS!=DICT)
	{
		Btnlist_hide();
		STATS=DICT;
		SDL_SetWindowTitle(stage->window, "词典");
	}
	if(dictContainer->visible)
	{
		SDL_Log("text input changed!");
		list_show();
		Wordinput_changeWordList();
	}
}
static void show_list(SpriteEvent* e){
	if(Sprite_getVisible(dictContainer)==0)
		return;
	//Sprite*target = e->target;
	SDL_Event* event = e->e;
	switch(event->type)
	{
		case SDL_MOUSEBUTTONDOWN:
			if(event->button.x>stage->stage_w-40*1.6)
			{
				Input_setText(input,"");
			}
			break;
	}

	if(input)
	{
		Sidebtns_hide();
		if(STATS==DICT){
			Btnlist_hide();
			textChangFunc(input);
		}else{
			Wordinput_hideList();
			Btnlist_show();
		}
	}
}
static void keyupEvent(SpriteEvent* e){
	if(Sprite_getVisible(dictContainer)==0)
		return;
	SDL_Event *event = e->e;
	if(event->key.keysym.sym == SDLK_RETURN)
	{
		if(strlen(input->value)>1){
			int l= strlen(input->value);
			char v[l];
			memset(v,0,l);
			memcpy(v,input->value,l-1);
			while( v[strlen(v)-1]=='\r'|| v[strlen(v)-1]=='\n')
				v[strlen(v)-1]='\0';
			if(strlen(v)>0){
				Word * word = getWordFromDict(v);
				getMean(word);
			}
		}
	}
}

void Wordinput_hideList()
{
	if(curlistSprite)
	{
		curlistSprite->visible = SDL_FALSE;
	}
}

void Wordinput_show()
{
	Btnlist_hide();
	open_dict();
	if(input==NULL)
	{
		input = Input_new(stage->stage_w,min(stage->stage_h/10,50));
		input->textChangFunc = textChangFunc;
		Sprite_addEventListener(input->sprite,SDL_MOUSEBUTTONDOWN,show_list);//click to show a list
		Sprite_addChild(dictContainer,input->sprite);

		Sprite * clearbtn = Sprite_newText("⊗",input->sprite->h*.7,0x0,0x00000000);
		clearbtn->x = stage->stage_w-clearbtn->w*1.3;
		Sprite_addChild(input->sprite,clearbtn);
		Sprite_addEventListener(stage->sprite,SDL_KEYUP,keyupEvent); 
	}
	stage->focus = input->sprite;
}

int Wordinput_searchWord(char * v)
{
	Word * word = getWordFromDict(v);
	if(word)
		getMean(word);
	return 0;
}
