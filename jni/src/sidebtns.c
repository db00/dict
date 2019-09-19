#include "sidebtns.h"
static Sprite * sideBtns = NULL;
static void read_out(SpriteEvent*e)
{
	if(Sprite_getVisible(sideBtns)==0)
		return;
	Sprite*target = e->target;
	if(target==NULL || target->obj==NULL)
		return;
	//SDL_Event* event = e->e;


	if(strcmp(target->obj,"粘贴")==0){
		char * s = getClipboardText(1);
		//printf("\n------%s,",s);fflush(stdout);
		if(s && strlen(s)>0)
		{
			Input_setText(input,s);
			//searchWord(s);
		}
		return;
	}else if(strcmp(target->obj,"测试")==0){
		Test_start(dictContainer,ec_dict);
		return;
	}else if(strcmp(target->obj,"剪切版词")==0){
		char * s = getClipboardText(1);
		if(s && strlen(s)>0){
			Array * matched_arr = Array_new();
			int n = regex_search_all(s,"/[a-z-]{3,100}/i", matched_arr);
			int i = 0;
			while(i<n){
				char * word = Array_getByIndex(matched_arr,i);
				int _index = Dict_getWordIndex(ec_dict,word);
				if(_index>=0)
				{
					//printf("%d:%s\r\n",i,word);
					if(get_word_id(word)<=0)
					{
						//printf("%d:%s\r\n",_index,word);
						add_remembered_word(word,0);
						//printf("%d:%s\r\n",i,word);
					}
				}
				++i;
			}
			fflush(stdout);
			Array_clear(matched_arr);
			Input_setText(input,"");
			/*
			STATS = NEW;
			SDL_SetWindowTitle(stage->window, "生词");
			Btnlist_showByValue("");
			*/
		}
		return;
	}

	char * word = input->value;
	if(word==NULL || strlen(word)<=0)
		return;

	//printf("read %s\n",word);
	if(strcmp(target->obj,"清除")==0){
		if(strlen(word)==0)
			return;
		Input_setText(input,"");
	}
	//Stage_redraw();
}

static Sprite * makeSideBtn(char * name,int y, void (*func)(SpriteEvent*))
{
	Sprite * btn = Sprite_newText(name,45*stage->stage_h/800,0x0,0xffffffff);
	btn->y = y;
	Sprite_addEventListener(btn,SDL_MOUSEBUTTONUP,func);
	//printf("btn:(%s)\n",btn->obj);
	btn->x = stage->stage_w - btn->w;
	Sprite_addChild(sideBtns,btn);
	return btn;
}
void Sidebtns_hide()
{
	if(sideBtns)
		sideBtns->visible = SDL_FALSE;
}
static void show_history_list(SpriteEvent*e)
{
	if(Sprite_getVisible(sideBtns)==0)
		return;
	Sprite*target = e->target;
	//SDL_Event* event = e->e;


	//Array * data = NULL;
	if(strcmp(target->obj,"历史")==0){
		if(STATS!=HISTORY)
		{
			STATS = HISTORY;
		}
		SDL_SetWindowTitle(stage->window, "历史");
	}else if(strcmp(target->obj,"熟词")==0){
		if(STATS!=REMEMBERED)
		{
			STATS = REMEMBERED;
		}
		SDL_SetWindowTitle(stage->window, "熟词");
	}else if(strcmp(target->obj,"生词")==0){
		if(STATS!=NEW)
		{
			STATS = NEW;
		}
		SDL_SetWindowTitle(stage->window, "生词");
	}else{
		return;
	}

	Btnlist_showByValue(input->value);
}
Sprite * Sidebtns_show()
{
	if(sideBtns==NULL)
	{
		sideBtns = Sprite_new();
		Sprite_addChild(dictContainer,sideBtns);

		int gap =2;
		int y = input->sprite->h;
		//int y = 0;
		//enBtn = makeSideBtn("清除",y,read_out);
		//y = enBtn->y + enBtn->h + gap;
		Sprite * enBtn = makeSideBtn("粘贴",y,read_out);
		y = enBtn->y + enBtn->h + gap;
		enBtn = makeSideBtn("历史",y,show_history_list);
		y = enBtn->y + enBtn->h + gap;
		enBtn = makeSideBtn("生词",y,show_history_list);
		y = enBtn->y + enBtn->h + gap;
		enBtn = makeSideBtn("熟词",y,show_history_list);
		y = enBtn->y + enBtn->h + gap;
		enBtn = makeSideBtn("测试",y,read_out);
		y = enBtn->y + enBtn->h + gap;
		enBtn = makeSideBtn("剪切版词",y,read_out);
		y = enBtn->y + enBtn->h + gap;
	}
	sideBtns->visible = SDL_TRUE;
	return sideBtns;
}
int Sidebtns_getWidth()
{
	if(sideBtns && sideBtns->Bounds)
		return sideBtns->Bounds->w;
	return 0;
}
