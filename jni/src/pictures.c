/**
 * @file pictures.c
 gcc -Wall -g -I"../SDL2/include/" -I"../SDL2_image/" -I"../SDL2_ttf/" loading.c tween.c ease.c mylist.c matrix.c update.c bytearray.c zip.c utf8.c mysurface.c myfont.c pictures.c sprite.c urlcode.c myregex.c files.c array.c base64.c ipstring.c httploader.c mystring.c -lssl -lcrypto -lm -lpthread -lz -lSDL2 -lSDL2_image -lSDL2_ttf -D debug_pictures &&./a.out
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2017-06-01
 *
 *
 *{
 "thumbURL":"https://ss1.bdstatic.com/70cFuXSh_Q1YnxGkpoWK1HF6hhy/it/u=3570086611,1076431208&fm=23&gp=0.jpg",
 "adType":"0",
 "middleURL":"https://ss1.bdstatic.com/70cFuXSh_Q1YnxGkpoWK1HF6hhy/it/u=3570086611,1076431208&fm=23&gp=0.jpg",
 "largeTnImageUrl":"","hasLarge" :0,"hoverURL":"https://ss1.bdstatic.com/70cFuXSh_Q1YnxGkpoWK1HF6hhy/it/u=3570086611,1076431208&fm=23&gp=0.jpg",
 "pageNum":12,
 "objURL":"http://media-cdn.tripadvisor.com/media/photo-s/07/48/e0/57/ruth-s-chris-steak-house.jpg",
 "fromURL":"ippr_z2C$qAzdH3FAzdH3Fooo_z&e3Bp6trw1etf56_z&e3Bv54AzdH3FL5vwpt5gPi5p5Dt6jvpLtgh-2dlc9d9-18nbldcl-tn9m80nbb-R7pi_f_Ci6tf_Spjwh_H57fj_D7kwt-D7kwt_E4t6wpj_5u_D7kwt_z&e3Bip4sn9m80nbb",
 "fromURLHost":"www.tripadvisor.com",
 "currentIndex":"",
 "width":550,
 "height":367,
 "type":"jpg",
 "filesize":"",
 "bdSrcType":"0",
 "di":"150040500060",
 "is":"0,0",
 "bdSetImgNum":0,
 "spn":0,
 "bdImgnewsDate":"1970-01-01 08:00",
 "fromPageTitle":"steak<\/strong>house",
 "bdSourceName":"",
 "bdFromPageTitlePrefix":"",
 "isAspDianjing":0,"token":"",
 "imgType" : "",
 "adid":"0",
 "pi":"0",
 "cs" : "3570086611,1076431208",
 "os" : "359379483,3493808047",
 "simid" : "3482021890,483765125",
 "source_type":"",
 "personalized":"0",
 "base64": '',
 "adPicId": "0"
 }
 */


#include "pictures.h"
#include "mylist.h"


static Sprite * parent = NULL;
static Sprite * container = NULL;
static Array * urlArr = NULL;
static int picH = 0;
static int numPic = 0;
static int isLoading = 0;

static void addPic();

void removePictures()
{
	if(urlArr)
	{
		Array_freeEach(urlArr);
		urlArr = NULL;
	}
	if(container)
	{
		if(parent && parent == container->parent)
			Sprite_removeChild(parent,container);

		Sprite_destroy(container);
		container = NULL;
	}
	isLoading = 0;
	isPictureMode = 0;
	picH = 0;
	numPic = 0;
	Stage_redraw();
}
static void * load_picture(void * _url)
{
	if(_url)
	{
		Sprite * sprite = Sprite_newImg((char*)_url);
		if(sprite)
		{
			sprite->y = picH;
			if(sprite->w>stage->stage_w)
			{
				float scale = stage->stage_w*1.0/sprite->w;
				sprite->w *= scale;
				sprite->h *= scale;
			}
			picH += sprite->h;
			Sprite_addChild(container,sprite);
			Stage_redraw();
		}
	}
	isLoading = 0;
	addPic();
	return NULL;
}
static void addPic()
{
	if(urlArr==NULL)
		return;


	if(isLoading == 0 && container->y+picH<stage->stage_h)
	{
		isLoading = 1;
		int i = numPic;
		if(i>=urlArr->length)
			return;
		char * _url = Array_getByIndex(urlArr,i);

		pthread_t thread1;
		if(pthread_create(&thread1, NULL,load_picture, _url)!=0)//创建子线程  
		{  
			perror("pthread_create");  
		}else{
			pthread_detach(thread1);
			//pthread_join(thread1,NULL);
		}
		++numPic;
	}

}

static void container_new()
{
	if(parent && container==NULL)
	{
		container = Sprite_new();
		//Sprite_addChildAt(parent,container,0);
		Sprite_addChild(parent,container);
		container->y = stage->stage_h/3;
		container->mouseChildren = SDL_FALSE;
		List_roll(container,addPic);
	}

	if(container && container->dragRect==NULL)
	{
		SDL_Rect * rect = malloc(sizeof(*rect));
		rect->x = container->x;
		rect->w = 0;
		rect->y = -(((unsigned int)-1)/4);
		rect->h = (((unsigned int)-1)/2);
		container->dragRect = rect;
	}
}


static void baiduLoaded(URLRequest * urlrequest)
{
	if(urlrequest==NULL)
		return;
	if(urlrequest->statusCode == 200){
		//char * filename = decodePath("~/sound/pic.txt");
		//if(writefile(filename,urlrequest->data,urlrequest->respond->contentLength)==0)
		{

			Array * matched_arr = Array_new();
			int n = regex_search_all(urlrequest->data,"/\"thumbURL\":\"([^\"]{5,})\"/", matched_arr);
			printf("ok:%d\r\n",n);
			int i = 0;
			while(i<n)
			{
				if(urlArr==NULL)
				{
					urlArr = Array_new();
				}
				char * thumbURL = Array_getByIndex(matched_arr,i);
				char * _url = getStrBtw(thumbURL,":\"","\"",0);
				free(thumbURL);
				printf("%d:%s\r\n",i,_url);
				if(regex_match(_url,"^http"))
					Array_push(urlArr,_url);

				addPic();

				++i;
			}
			Array_clear(matched_arr);
		}
	}
	URLRequest_clear(urlrequest);
	fflush(stdout);
}

void search_pic(Sprite * _container,char * _word)
{
	removePictures();
	isPictureMode = 1;

	parent = _container;

	container_new();


	const char * __url = "https://image.baidu.com/search/index?tn=baiduimage&word=%s";
	int len = strlen(__url)+strlen(_word)+5;
	char baiduurl[len];
	memset(baiduurl,0,len);
	sprintf(baiduurl,__url,_word);
	//URLRequest * urlrequest = Httploader_load(baiduurl); baiduLoaded(urlrequest);
	URLRequest * urlrequest =  Httploader_asyncload(baiduurl,baiduLoaded);
	if(urlrequest)
		printf("%s",baiduurl);
}

#ifdef debug_pictures
int main()
{
	Stage_init();

	search_pic(stage->sprite,"kitten");

	Stage_loopEvents();
	return 0;
}

#endif
