#include "btnlist.h"

static Sprite * curlistSprite=NULL;
static Array * history_str_arr;

static Sprite * removeWordBtn(Word * word)
{
	printf("\r\n remembered:%s\r\n",word->word);fflush(stdout);
	if(curlistSprite->children && curlistSprite->children->length>0)
	{
		int i = 0;
		while(i<curlistSprite->children->length)
		{
			Sprite * lastSprite = Sprite_getChildByIndex(curlistSprite,i);
			if(lastSprite->obj==word)
			{
				int n = Array_getIndexByStringValue(history_str_arr,word->word);
				history_str_arr = Array_removeByIndex(history_str_arr,n);

				return List_removeItem(curlistSprite,lastSprite);
			}
			++i;
		}
	}
	return NULL;
}

static int button_messagebox(Word * _word)
{
	char * word = _word->word;
	if (word == NULL || strlen(word)==0) {
		return 1;
	}
	const SDL_MessageBoxButtonData buttons[] = {
		{
			(STATS==NEW)?SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT:SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
			0,
#ifndef __ANDROID__
			"remembered"
#else
			"加入熟词"
#endif
		},
		{
			(STATS==REMEMBERED)?SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT:SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
			1,
#ifndef __ANDROID__
			"raw"
#else
			"加入生词"
#endif
		},
		{
			(1)?SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT:SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
			2,
#ifndef __ANDROID__
			"Delete"
#else
			"删除词"
#endif
		},
	};

	SDL_MessageBoxData data = {
		SDL_MESSAGEBOX_INFORMATION,
		NULL, /* no parent window */
		//"change rememeber status",
		"改变单词状态",
		word,//data.message = word;
		3,
		buttons,
		NULL /* Default color scheme */
	};

	int button = -1;
	int success = 0;

	success = SDL_ShowMessageBox(&data, &button);
	if (success == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error Presenting MessageBox: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Log("Pressed button: %d, %s\n", button, button==2?"删除":(button == 0 ? "熟" : "生"));

	if(button==0){
		add_remembered_word(word,1);
		if(STATS==NEW){
			Sprite_destroy(removeWordBtn(_word));
		}
	}else if(button==1){
		add_remembered_word(word,0);
		if(STATS==REMEMBERED){
			Sprite_destroy(removeWordBtn(_word));
		}
	}else if(button==2){
		del_word(word);
		if(STATS==REMEMBERED || STATS==NEW){
			Sprite_destroy(removeWordBtn(_word));
		}
	}

	return button;
}
static int getMean(Word * word)
{
	if(word==NULL)
		return 0;
	Btnlist_hide();
	Sidebtns_hide();

	Input_setText(input,word->word);
	Explain_show(ec_dict,word);
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

static int getIndexByValue(Array * array,char * word)
{
	if(array && array->length>0)
	{
		int i = 0;
		while(i<array->length)
		{
			char * s = Array_getByIndex(array,i);
			//printf("%s - %s\r\n",s,word);
			if(s && strlen(word)>0 && strcmp(s,word)==0)
				return i;
			++i;
		}
	}else{
		if(array==NULL){
			printf("array==NULL\n");
		}else{
			printf("array.length=%d\n",array->length);
		}
		fflush(stdout);
	}
	//printf("s=%s,%d\n",word,strlen(word));
	return -1;
}



static int noMoreAppend=0;
static int noMorePreppend=0;
static Array * get_more_list(char * word,int isAppend,int isFromCenter)
{
	int num = 20;
	char * compare = ">";
	if(isAppend)
	{
		printf(" isAppend\r\n");
		compare = "<";
	}
	if(isFromCenter || word==NULL)
	{
		if(history_str_arr)
		{
			Array_freeEach(history_str_arr);
		}
		history_str_arr = NULL;
	}
	if(word && strlen(word)==0)
		word = NULL;

	if(isFromCenter==0)
	{
		if(noMoreAppend && isAppend)
			return history_str_arr;
		if(noMorePreppend && (!isAppend))
			return history_str_arr;
	}
	if(isFromCenter){
		if(strcmp(compare,"<")==0)
			compare = "<=";
		else
			compare = ">=";
		noMoreAppend=0;
		noMorePreppend=0;
	}

	Array * data = NULL;
	if(STATS==HISTORY){
		if(word && strlen(word)>0)
			data = get_history_list(num,word,compare);
		if(isFromCenter && data==NULL)
			data = get_history_list(num,NULL,compare);
	}else if(STATS==REMEMBERED){
		if(word && strlen(word)>0)
			data = get_remembered_list(1,num,word,compare);
		if(isFromCenter && data==NULL)
			data = get_remembered_list(1,num,NULL,compare);
	}else if(STATS==NEW){
		if(word && strlen(word)>0)
			data = get_remembered_list(0,num,word,compare);
		if(isFromCenter && data==NULL)
			data = get_remembered_list(0,num,NULL,compare);
	}

	if(data)
	{
		Array * names = Array_getByIndex(data,0);
		if(names==NULL){
			printf("no names Array");
			return history_str_arr;
		}
		int nCount = names->length;
		int i = 0;
		while(i<nCount)
		{
			char * curName =Array_getByIndex(names,i);
			if(strcmp(curName,"word")==0)
			{
				Array * wordsArr = Array_getByIndex(data,i+1);
				if(wordsArr && wordsArr->length>0)
				{
					Array * a = Array_new();
					int j = 0;
					while(j<wordsArr->length)
					{
						if(isAppend){
							Array_push(a,append_str(NULL,"%s",Array_getByIndex(wordsArr,j)));
						}else{
							Array_push(a,append_str(NULL,"%s",Array_getByIndex(wordsArr,wordsArr->length-1-j)));
						}
						++j;
					}

					if(isAppend){
						history_str_arr = Array_concat(history_str_arr,a);
					}else{
						Array * tmpArr = NULL;
						tmpArr = Array_concat(a,history_str_arr);
						Array_clear(history_str_arr);
						history_str_arr = tmpArr;
					}
				}
				if(wordsArr->length < num)
				{
					if(isAppend)
						noMoreAppend = 1;
					else
						noMorePreppend = 1;
				}
				//printf("\r\n column_name:%s:%d,length:%d\r\n",curName,i+1,wordsArr->length);
				break;
			}
			++i;
		}
	}else{
		if(isAppend)
			noMoreAppend = 1;
		else
			noMorePreppend = 1;
	}
	return history_str_arr;
}

static Word * getWordFromDict(char * _word)
{
	return Dict_getWord(ec_dict,_word);
}

void Btnlist_changeHistoryList()
{
	Wordinput_hideList();
	if(history_str_arr==NULL || history_str_arr->length<=0)
	{
		printf("\n no words \n");fflush(stdout);
		return;
	}

	char * curWord;
	if(curlistSprite->children ==NULL || curlistSprite->children->length<=0)
	{// first word
		int index = getIndexByValue(history_str_arr,input->value);
		if(index<0)
		{
			if(strlen(input->value)>0)
			{
				history_str_arr = get_more_list(input->value,1,1);
				if(history_str_arr==NULL)
					return;
				index = getIndexByValue(history_str_arr,input->value);
			}
		}
		if(index>=0)
		{
			curWord = Array_getByIndex(history_str_arr,index);
		}else{
			curWord = Array_getByIndex(history_str_arr,0);
		}
		if(curWord==NULL)
			return;
		//printf("\r\n curWord:%s\r\n",curWord);

		Word * word = getWordFromDict(curWord);
		if(word==NULL)
			return;
		appendWordBtn(word,0);
	}
	//fflush(stdout); return;
	Sprite * lastSprite = NULL;
	if(curlistSprite->children && curlistSprite->children->length>0)
	{
		lastSprite = Sprite_getChildByIndex(curlistSprite,0);
		while(lastSprite->y + curlistSprite->y>0)
		{// prepend
			Word * _word = lastSprite->obj;
			int index = getIndexByValue(history_str_arr,_word->word);
			if(index==0)
			{
				//printf ("index==0 history_str_arr length: %d ->",history_str_arr->length);
				history_str_arr = get_more_list(_word->word,0,0);
				index = getIndexByValue(history_str_arr,_word->word);
				//printf (" history_str_arr ---->%d (%d)\r\n",history_str_arr->length,index);
				fflush(stdout);
			}
			if(index<0)
			{
				printf("getIndexByValue--- %d\n",index);
				break;
			}
			curWord = Array_getByIndex(history_str_arr,index-1);
			if(curWord){
				Word * word = getWordFromDict(curWord);
				//Word * word = Array_getByIndex(history_str_arr,index-1); 
				lastSprite = appendWordBtn(word,0);
			}else{
				//printf("getIndexByValue---%d\n",index);
				break;
			}
		}

		if(curlistSprite->children && curlistSprite->children->length>0)
		{
			lastSprite = Sprite_getChildByIndex(curlistSprite,curlistSprite->children->length-1);
			while(lastSprite->y + curlistSprite->y + lastSprite->h < stage->stage_h)
			{//append
				Word * _word = lastSprite->obj;
				int index = getIndexByValue(history_str_arr,_word->word);
				//printf("\n-------------%s:%d/%d\n",_word->word,index,history_str_arr->length);fflush(stdout);
				if(index==history_str_arr->length-1)
				{
					//printf ("history_str_arr length: %d =======>",history_str_arr->length);
					history_str_arr = get_more_list(_word->word,1,0);
					index = getIndexByValue(history_str_arr,_word->word);
					//printf ("%d (%d)\r\n",history_str_arr->length,index);
				}
				//printf("\n-------------%s:%d/%d\n",_word->word,index,history_str_arr->length);fflush(stdout);
				//int index = curlistSprite->children->length;
				if(index<0 || index>= history_str_arr->length-1)
				{
					printf("getIndexByValue--- %s %d\n",_word->word,index); fflush(stdout);
					break;
				}

				char * curWord = Array_getByIndex(history_str_arr,index+1);
				//printf("\r\n curWord:%s\r\n",curWord);
				//if(curWord==NULL) return;
				Word * word = getWordFromDict(curWord);

				lastSprite = appendWordBtn(word,1);
			}
		}
	}
	List_showOptical(curlistSprite);
	//List_removeOuts();
}

static void mouseMoves(SpriteEvent*e)
{
	if(Sprite_getVisible(curlistSprite)==0)
		return;
	Sprite*target = e->target;
	if(target!=curlistSprite)
		return;

	SDL_Event* event = e->e;
	if(event->type!= SDL_MOUSEMOTION)
		return;

	SDL_StopTextInput();

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
			Btnlist_changeHistoryList();
		}
	}
}


void Btnlist_show()
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
	}
	if(curlistSprite){
		Sprite_removeChildren(curlistSprite);
		curlistSprite->x = 0;
		curlistSprite->y = Wordinput_getHeight();
	}
	curlistSprite->visible = SDL_TRUE;


	Explain_hide();
	Btnlist_changeHistoryList();
	Stage_redraw();
}
void Btnlist_hide()
{
	if(curlistSprite){
		curlistSprite->visible = SDL_FALSE;
	}
}

void Btnlist_showByValue(char * value)
{
	get_more_list(value,1,1);
	Btnlist_show();
}
