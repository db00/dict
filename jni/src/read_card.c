/**
 * @file read_card.c
 gcc -g -Wall -D test_readcard -Wall -lpthread music.c utf8.c mysurface.c pinyin.c input.c array.c ease.c tween.c kodi.c base64.c urlcode.c filetypes.c httpserver.c httploader.c readbaidu.c ipstring.c read_card.c dict.c myregex.c files.c mystring.c sprite.c matrix.c textfield.c -lcrypto -lssl -ldl -lm -I"../SDL2_mixer/" -I"../SDL2_image/" -I"../SDL2/include/" -I"../libxml/include/" -I"../SDL2_ttf/" -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_image && ./a.out
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2016-09-18
 */
#include "read_card.h"

typedef struct Card
{
	int index;
	char lang;//c:汉字,e:English
	Sprite * sprite;
	Sprite * img;
	Sprite * txt;//->obj
	char * sound;
	int isRight;//1:right,2:wrong
	void(*complete)(struct Card*);
}Card;

static Array * askArr = NULL;
static Array * allAskArr = NULL;
static int curAskIndex = 0;
static int curKeyIndex=0;

static Card * rightCard;

//
//playPinyins("(hao3 a1)");
int playPinyins(char * s)
{
	if(s==NULL)
		return 1;
	char * pinyins = getStrBtw(s,"[","]",0);
	if(pinyins==NULL)
		return 2;
	Array * arr = string_split(pinyins," ");
	int i = 0;
	while(i<arr->length)
	{
		char * pinyin = Array_getByIndex(arr,i);
		if(pinyin && strlen(pinyin))
			playEasyPinyin(pinyin);
		++i;
	}
	return 0;
}

/*
   int playYinbiaos(char * s)
   {
   if(s==NULL)
   return 1;
   char * pinyins = getStrBtw(s,"[","]",0);
   if(pinyins==NULL)
   return 2;
//Array * arr = string_split(pinyins," ");
Array * arr = UTF8_each(pinyins);
int i = 0;
while(i<arr->length)
{
char * pinyin = Array_getByIndex(arr,i);
if(pinyin && strlen(pinyin))
playYinbiao(pinyin);
++i;
}
free(pinyins);
Array_clear(arr);
return 0;
}
*/

int getNumInLine()
{
	Array * curAsk= Array_getByIndex(askArr,curAskIndex);
	int len = curAsk->length;
	int i = sqrt(len);
	while(i*i<len)
		i=i+1;
	return i;
}


static void playCardSound(Card * card)
{
	//Array * curAsk = Array_getByIndex(askArr,curAskIndex);
	//Array * curLine = Array_getByIndex(curAsk,card->index);
	char * zhuyin = card->sound;

	//playYinbiaos(zhuyin);
	if(card->lang=='e'){
		Sound_playEng(zhuyin,1);//1:uk,2:us
	}else if( card->lang=='c') {
		playPinyins(zhuyin);
	}else if( card->lang=='n') {
		char *num = num2hzs(atoi(zhuyin));
		playHzsPinyin(num);
		free(num);
	}else if( card->lang=='h') {
		playHzsPinyin(zhuyin);
		//playPinyins(zhuyin);
	}else{
		playHzsPinyin(zhuyin);
	}
}

void * readAsk(void*k)
{
	//Array * curAsk = Array_getByIndex(askArr,curAskIndex);
	//Array * curLine = Array_getByIndex(curAsk,curKeyIndex);
	Card * card = k;
	playCardSound(card);

	playHzsPinyin("在哪");
	return NULL;
}

void clicked(SpriteEvent* e)
{
	Sprite * sprite = e->target;
	Card * card = sprite->parent->obj;
	playCardSound(card);

	if(card->isRight==1)
	{
		playHzsPinyin("对");
		//Sprite_removeEvents(sprite);

		makeNewAsk(-1,-1);
	}else if(card->isRight==2){
		playHzsPinyin("错");
		readAsk(rightCard);
	}
	//if(card->complete) card->complete(card);
}


Card * Card_new(char * lang,char * txt,char * url,char * sound)
{
	Card * card = malloc(sizeof(Card));
	memset(card,0,sizeof(Card));

	card->sprite = Sprite_new();
	card->sprite->obj = card;
	card->lang = lang[0];
	card->sound = sound;
	if((unsigned char)(txt[0])>0x80)
		txt = contact_str(txt,Array_joins(hzs2pinyin(txt),""));
	card->txt = Sprite_newText(txt,stage->stage_h/320*12,0xff0000ff,0xffff00ff);
	if(fileExists(url)){
		card->img = Sprite_newImg(url);
	}else{
		card->img = Sprite_newText(url,stage->stage_h/320*90,0xff0000ff,0xffff00ff);
	}
	card->img->obj = txt;

	int len = getNumInLine();

	int w = stage->stage_w/len-1;
	int h = stage->stage_h/len-1;
	Sprite_fullcenter(card->img,0,0,w,h);
	//Sprite_center(card->txt,0,0,w/2,card->lang->h);
	card->txt->y = h-card->txt->h;

	card->sprite->w = w;
	card->sprite->h = h;

	Sprite_addChild(card->sprite,card->img);
	Sprite_addChild(card->sprite,card->txt);

	//Sprite_addEventListener(card->sprite,SDL_MOUSEBUTTONUP,clicked);
	Sprite_addEventListener(card->txt,SDL_MOUSEBUTTONUP,clicked);
	Sprite_addEventListener(card->img,SDL_MOUSEBUTTONUP,clicked);

	return card;
}
void Card_free(Card*card)
{
	if(card)
	{
		free(card);
	}
}
void removeCardContainer()
{
	if(cardContainer){
		/*
		   int i = cardContainer->children->length;
		   while(i>0)
		   {
		   --i;
		   Sprite * son = Sprite_getChildByIndex(cardContainer,i);
		   if(strcmp(son->name,"card")==0)
		   {
		   if(son->obj)
		   Card_free(son->obj);
		   son->obj=NULL;
		   }
		   }
		   */
		Sprite_removeChildren(cardContainer);
		Sprite_removeChild(stage->sprite,cardContainer);
	}
}

int * makeRadomArr(int n)
{
	if(n<=0)
		return NULL;
	int size = n*sizeof(int);
	int * a = malloc(size);
	memset(a,0,size);
	int i = 0;
	while(i<n)
	{
		a[i] = i;
		++i;
	}
	i=0;
	while(i<n)
	{
		int r = (int)(rand()%(n-i));
		int o1 = a[r];
		int o2 = a[i];
		a[r]=o2;
		a[i]=o1;
		++i;
	}
	i=0;
	while(i<n)
	{
		printf("%d-->%d\n",i,a[i]);
		++i;
	}
	return a;
}

void makeList()
{
	Array * curAsk= Array_getByIndex(askArr,curAskIndex);
	removeCardContainer();
	if(cardContainer == NULL)
	{
		cardContainer = Sprite_new();
		cardContainer->surface = Surface_new(1,1);
		char pixels[4] ={'\0','\0','\0','\xff'};
		memcpy(cardContainer->surface->pixels,(char*)pixels,sizeof(pixels));
		cardContainer->w = stage->stage_w;
		cardContainer->h = stage->stage_h;
	}
	Sprite_addChild(stage->sprite,cardContainer);

	Array * curLine = Array_getByIndex(curAsk,curKeyIndex);
	char * ask = contact_str(Array_getByIndex(curLine,0),"在哪？");
	Sprite * sprite = Sprite_newText(ask,18,0xff0000ff,0xffff00ff);
	free(ask);
	free(sprite->name);
	Sprite_addChild(cardContainer,sprite);
	int i = 0;
	int sq = getNumInLine();
	int w = stage->stage_w/sq;
	int h = stage->stage_h/sq;
	int _y =  (sq*sq-curAsk->length)/sq*stage->stage_h/sq/2;
	int * randomArr = makeRadomArr(curAsk->length);
	while(i<curAsk->length)
	{
		Array * itemArr = Array_getByIndex(curAsk,i);
		char * s1 = Array_getByIndex(itemArr,0);
		char * s2 = Array_getByIndex(itemArr,1);
		char * s3 = Array_getByIndex(itemArr,2);
		char * s4 = Array_getByIndex(itemArr,3);
		if(s1 && s2 && s3 && strlen(s1) && strlen(s2))
		{
			printf("%s,%s,%s,%s\n",s1,s2,s3,s4);
			Card * card = Card_new(s1,s2,s3,s4);
			if(i==curKeyIndex){
				card->isRight = 1;
				rightCard = card;
			}else{
				card->isRight = 2;
			}
			card->index = i;
			if(card->sprite->name)
				free(card->sprite->name);
			card->sprite->name = contact_str("","card");
			Sprite_addChild(cardContainer,card->sprite);

			int index = randomArr[i];
			Sprite_center(card->sprite,(index%sq)*w,(index/sq)*h+_y,w,h);
		}
		//SDL_Log("curKeyIndex:%d,%d",curKeyIndex,i);
		++i;
	}
	free(randomArr);
	Stage_redraw();

	//readAsk(NULL); return NULL;
	pthread_t thread;//创建不同的子线程以区别不同的客户端  
	if(pthread_create(&thread, NULL, readAsk, rightCard)!=0)//创建子线程  
	{  
		perror("pthread_create");  
	}else{
		pthread_detach(thread);
	}
}

Array * getAskArr(int numAsks)
{
	if(allAskArr)
	{
		if(numAsks>allAskArr->length){
			return allAskArr;
		}

		return allAskArr;
	}
	size_t data_len = 0;
	char * data = NULL;
	data = readfile("~/sound/test.txt",&data_len);
	Array * array0 = string_split(data,"\n\n");
	if(array0 == NULL || array0->length == 0)
	{
		if(array0)
			Array_clear(array0);
		if(data)
			free(data);
		return NULL;
	}

	allAskArr = Array_new();
	int i = 0;
	while(i<array0->length)
	{
		char * s = Array_getByIndex(array0,i);
		if(s==NULL || strlen(s)==0)
		{
			++i;
			continue;
		}
		Array * line = string_split(s,"\n");
		Array * lineArr = NULL;
		int l = 0;
		while(l<line->length)
		{
			char * sl = Array_getByIndex(line,l);
			if(sl && strlen(sl))
			{
				Array * arr = string_split(sl,",");
				if(arr && arr->length>=3)
				{
					if(lineArr==NULL){
						lineArr = Array_new();
						Array_push(allAskArr,lineArr);
					}
					Array_push(lineArr,arr);
				}
			}
			++l;
		}
		Array_clear(line);
		++i;
	}
	return allAskArr;
}

void makeNewAsk(int askIndex,int keyIndex)
{
	if(askArr == NULL)
		askArr = getAskArr(9);
	if(askArr == NULL)
		return;
	if(askIndex<0){
		srand((unsigned)time(NULL));  
		askIndex =(int)(rand()%askArr->length);
	}
	if(askIndex>=askArr->length)
		askIndex = askArr->length-1;
	curAskIndex = askIndex;
	Array * curAsk= Array_getByIndex(askArr,curAskIndex);

	if(keyIndex<0)
	{
		srand((unsigned)time(NULL));  
		keyIndex = (int)(rand()%curAsk->length);
	}
	if(keyIndex>=curAsk->length)
		keyIndex = curAsk->length-1;
	curKeyIndex = keyIndex;

	SDL_Log("curAskIndex: %d,curKeyIndex: %d\n",curAskIndex,curKeyIndex);

	makeList();
}

#ifdef test_readcard
int main()
{
	Stage_init();
	if(stage==NULL)return 1;
	stage->sound = Sound_new(16000);

	/*
	   Card * card = Card_new("e","one","~/sound/img/1.jpg",null);
	//card->isRight = 1;
	Sprite_center(card->sprite,0,0,stage->stage_w,stage->stage_h);
	Sprite_addChild(stage->sprite,card->sprite);
	*/
	//playPinyins("(hao3 a1)");

	//makeNewAsk(-1,-1);
	makeNewAsk(0,-1);

	Stage_loopEvents();
	return 0;
}
#endif
