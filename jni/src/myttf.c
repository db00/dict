/**
 * @file myttf.c
 gcc -Wall -I"../SDL2/include/"  -I"../SDL2_ttf/" myfont.c array.c utf8.c update.c httploader.c ipstring.c urlcode.c base64.c loading.c tween.c ease.c  bytearray.c zip.c files.c myregex.c sdlstring.c myttf.c matrix.c sprite.c mystring.c  -lSDL2_ttf -lz -lssl -lcrypto -lpthread -lm -lSDL2 -D debugtext  && ./a.out
 gcc -g -Wall -I"../SDL2/include/" -I"../SDL2_image/" -I"../SDL2_ttf/" array.c tween.c ease.c myttf.c matrix.c sprite.c sdlstring.c mystring.c files.c -lSDL2_ttf -lmingw32 -lSDL2main -lSDL2 -D debugtext && a
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-07-21
 */
#include "myttf.h"

//单个字符
typedef struct TextWord{
	char * word;
	int w;
	int h;
	Sprite * sprite;
}TextWord;

typedef struct TextLine{
	int start;//文本开始位置
	int numbyte;//文本行字节数
	Sprite * sprite;
}TextLine;

static void TextWord_clear(TextWord*textword)
{
	if(textword == NULL)
		return;
	if(textword->word)
		free(textword->word);
	if(textword->sprite){
		Sprite_destroy(textword->sprite);
	}
	free(textword);
}

static void TextLine_clear(TextLine*line)
{
	if(line== NULL)
		return;

	if(line->sprite)
	{
		while(line->sprite->children && line->sprite->children->length>0)
		{
			Sprite * child =Sprite_getChildByIndex(line->sprite,0);
			TextWord * textword = child->obj;
			TextWord_clear(textword);
			child->obj = NULL;
		}
		if(line->sprite->parent)
		{
			Sprite_removeChild(line->sprite->parent,line->sprite);
		}
		Sprite_destroy(line->sprite);
		line->sprite = NULL;
	}

	free(line);
	line = NULL;
}


static void Text_clearLines(Text*textfield)
{
	if(textfield->lines)
	{
		int i = 0;
		while(i<textfield->lines->length){
			TextLine * line = Array_getByIndex(textfield->lines,i);
			TextLine_clear(line);
			++i;
		}
		Array_clear(textfield->lines);
		textfield->lines = NULL;
	}
	if(textfield->sprite)
	{
		textfield->sprite->w = 0;
		textfield->sprite->h = 0;
	}
}
void Text_clear(Text*textfield)
{
	if(textfield == NULL)
		return;

	if(textfield->font){
	}

	Text_clearLines(textfield);

	if(textfield->textColor){
		free(textfield->textColor);
	}

	if(textfield->backgroundColor){
		free(textfield->backgroundColor);
	}

	if(textfield->text) {
		free(textfield->text);
	}
	if(textfield->sprite) {
		Sprite_removeChildren(textfield->sprite);
		Sprite_destroy(textfield->sprite);
	}
	free(textfield);
}

static char * curWord;
static int curMouseY;
static unsigned int timestamp;
static void showCurWord(SpriteEvent * e)
{
	Sprite * sprite = e->target;
	TextWord * textword = sprite->obj;
	Text * textfield = sprite->other;
	SDL_Event * event = e->e;
	switch(e->type)
	{
		case SDL_MOUSEMOTION:
			if(curWord != textword->word || abs(curMouseY-event->motion.y)>10){
				Sprite_removeEventListener(sprite,SDL_MOUSEBUTTONUP,showCurWord);
				Sprite_removeEventListener(sprite,SDL_MOUSEMOTION,showCurWord);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			Sprite_addEventListener(sprite,SDL_MOUSEBUTTONUP,showCurWord);
			Sprite_addEventListener(sprite,SDL_MOUSEMOTION,showCurWord);
			curMouseY = event->button.y;
			curWord = textword->word;
			timestamp = event->button.timestamp;
			break;
		case SDL_MOUSEBUTTONUP:
			Sprite_removeEventListener(sprite,SDL_MOUSEBUTTONUP,showCurWord);
			Sprite_removeEventListener(sprite,SDL_MOUSEMOTION,showCurWord);
			if(curWord == textword->word && event->button.timestamp-timestamp>1000)
			{
				SDL_Log("selected: %s",textword->word);
				if(textfield->wordSelect)
					textfield->wordSelect(textword->word);
			}
			break;
	}
}

static TextWord * Textword_new(Text * textfield, Uint8 * curChar)
{
	TextWord* textword = (TextWord*)malloc(sizeof(TextWord));
	memset(textword,0,sizeof(TextWord));

	int numBytes = UTF8_numByte((char*)curChar);
	if(('A'<=curChar[0] && curChar[0]<='Z') || ('a'<=curChar[0] && curChar[0]<='z'))
	{//单词不分段
		while(
				('a'<=curChar[numBytes] && curChar[numBytes]<='z')
				|| ('A'<=curChar[numBytes] && curChar[numBytes]<='Z')
			 )
			numBytes++;
	}

	char * word = malloc(numBytes+1);
	memset(word,0,numBytes + 1);
	memcpy(word,curChar,numBytes);
	textword->word = word;

	textword->sprite = Sprite_new();
	textword->sprite->obj = textword;
	textword->sprite->other = textfield;

	TTF_Font * font = textfield->font;
	SDL_Surface * surface=NULL;
	SDL_Color * backgroundColor = textfield->backgroundColor;
	SDL_Color * textColor = textfield->textColor;
	if(backgroundColor){
		surface= TTF_RenderUTF8_Shaded(font, word, *(textColor),*(backgroundColor));
	}else{
		//surface= TTF_RenderUTF8_Blended(font, word, *(textColor));
		surface= TTF_RenderUTF8_Solid(font, word, *(textColor));
	}
	if(surface)
	{
		textword->sprite->surface = surface;
		//TTF_SizeUTF8(font,word,&(textword->w),&(textword->h));
		textword->w = surface->w;
		textword->h = surface->h;
		Sprite_addEventListener(textword->sprite,SDL_MOUSEBUTTONDOWN,showCurWord);
	}
	return textword;
}

static TextLine * TextLine_new()
{
	TextLine * textline = (TextLine*)malloc(sizeof(TextLine));
	memset(textline,0,sizeof(TextLine));
	textline->sprite = Sprite_new();
	return textline;
}

static TextLine * Text_getLastLine(Text * textfield)
{
	if(textfield==NULL){
		SDL_Log("Text_getLastLine Error: textfield is NULL!\n");
		return NULL;
	}
	if(textfield->lines == NULL){
		TextLine * line = TextLine_new();
		line->start = textfield->dealedlen;
		textfield->lines = Array_new();
		Array_push(textfield->lines,line);
		Sprite_addChild(textfield->sprite,line->sprite);
	}
	return Array_getByIndex(textfield->lines,textfield->lines->length-1);
}

static TextLine * Text_appenNewLine(Text * textfield)
{
	TextLine * line = TextLine_new();
	line->start = textfield->dealedlen;
	Sprite_addChild(textfield->sprite,line->sprite);
	Array_push(textfield->lines,line);
	line->sprite->y = textfield->sprite->h;
	return line;
}

static SDL_bool Text_lineFull(Text * textfield,TextLine * line,TextWord*textword)
{
	if(textfield && line && textword){
		if((textword->w) + (line->sprite->w) > (textfield->w)) 
		{
			return SDL_TRUE;
		}

		if(textfield->text && strlen(textfield->text)) {
			char * lastWord = textfield->text + textfield->dealedlen-1;
			if(*lastWord == '\r' || *lastWord == '\n' )
			{
				//SDL_Log("----------------------------r-n---------------");
				return SDL_TRUE;
			}
		}
	}
	return SDL_FALSE;
}



static void Text_more(Text * textfield)
{
	TextLine * line = Text_getLastLine(textfield);//最后一行
	while(textfield->dealedlen < strlen(textfield->text))
	{
		TextWord * textword = Textword_new(textfield,(Uint8*)(textfield->text+textfield->dealedlen));

		if(Text_lineFull(textfield,line,textword))
		{//行满,另起一行
			line = Text_appenNewLine(textfield);
			if(line->sprite->y + textfield->sprite->y > textfield->h)
				break;
		}

		textword->sprite->x = line->sprite->w;
		line->sprite->w += textword->w;

		Sprite_addChild(line->sprite,textword->sprite);

		line->numbyte += strlen(textword->word);
		if(line->sprite->h < textword->h){
			line->sprite->h = textword->h;//line h
		}
		if(textfield->sprite->h < line->sprite->y + line->sprite->h)
			textfield->sprite->h = line->sprite->y + line->sprite->h;

		textfield->dealedlen += strlen(textword->word);
	}

	Sprite * sprite = textfield->sprite;
	int fontheight = TTF_FontHeight(textfield->font);
	if(sprite->dragRect==NULL || sprite->dragRect->h != textfield->sprite->h - fontheight)
	{
		SDL_Rect * rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
		rect->x = textfield->x;
		rect->y = textfield->y - textfield->sprite->h + fontheight;
		rect->w = 0;
		rect->h = textfield->sprite->h - fontheight;
		if(sprite->dragRect)
			free(sprite->dragRect);
		sprite->dragRect = rect;
		Sprite_limitPosion(sprite,sprite->dragRect);
	}
}

static void mouseWheels(SpriteEvent*e)
{
	SDL_Event *event  = e->e;
	Sprite *sprite = e->target;
	if(sprite->h<1)
		return;

	Text * textfield = (Text*)(sprite->obj);

	int lineHeight = TTF_FontHeight(textfield->font);
	if(textfield && textfield->lines){
		switch(event->type)
		{
			case SDL_MOUSEWHEEL:
				if(event->wheel.y > 0){
					sprite->y += lineHeight;
				}else if(event->wheel.y < 0){
					sprite->y -= lineHeight;
				}
				break;
			case SDL_MOUSEMOTION:
				if(event->motion.state){
					sprite->y += event->motion.yrel;
				}
				break;
		}
		if(sprite->dragRect) Sprite_limitPosion(sprite,sprite->dragRect);
		Text_more(textfield);
		Stage_redraw();
	}

}


Text * Text_appendText(Text*textfield,char*s)
{
	if(s == NULL || strlen(s)==0)
		return textfield;
	if(textfield == NULL)
		textfield = Text_new();

	textfield->text = contact_str(textfield->text,s);

	Text_more(textfield);

	Sprite_addEventListener(textfield->sprite,SDL_MOUSEWHEEL,mouseWheels);
	if(textfield->sprite->canDrag)
	{
		Sprite_addEventListener(textfield->sprite,SDL_MOUSEMOTION,mouseWheels);
	}

	return textfield;
}

Text * Text_new()
{
	if(!TTF_WasInit())
	{
		if ( TTF_Init() < 0 ) {
			fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
			SDL_Quit();
			return NULL;
		}
	}

	Text* textfield = (Text*)malloc(sizeof(Text));
	memset(textfield,0,sizeof(Text));
	textfield->w = 100;
	textfield->h = 100;
	textfield->sprite = Sprite_new();
	//textfield->sprite->mouseChildren = SDL_FALSE;
	textfield->sprite->obj = textfield;

	if(textfield->textColor==NULL)
		textfield->textColor = uintColor(0x00ff0000);

	if(textfield->font==NULL){
		int stageSize = max(stage->stage_w,stage->stage_h);
		int textSize = 6 * stageSize/320;
		if(textSize<12)
			textSize = 12;
		textfield->font = getDefaultFont(textSize);
	}

	return textfield;
}



Text * Text_setText(Text*textfield,char *s)
{
	if(textfield==NULL)
		textfield = Text_new();


	Text_clearLines(textfield);

	if(textfield->text) {
		free(textfield->text);
		textfield->length = 0;
		textfield->text = NULL;
	}
	textfield->dealedlen = 0;

	if(textfield->sprite) {
		textfield->sprite->canDrag = 1;
	}else{
		textfield->sprite = Sprite_new();
		textfield->w = 100;
		textfield->h = 100;
		//textfield->sprite->mouseChildren = SDL_FALSE;
		textfield->sprite->obj = textfield;
	}
	textfield->sprite->y = textfield->y;
	textfield->sprite->x = textfield->x;


	if(s){
		Text_appendText(textfield,s);
	}
	return textfield;
}


#ifdef debugtext
#include "sdlstring.h"
int main(int argc, char *argv[])
{
	Stage_init();
	Text * txt = Text_new();//txt = Text_setText(txt,getLinkedVersionString());
	//txt->x = stage->stage_w/4;
	//txt->y = stage->stage_h/4;
	//txt->font = getFontByPath("DroidSansFallback.ttf",24);
	txt->font = getDefaultFont(24);
	txt->w = stage->stage_w;
	txt->h = stage->stage_h;
	txt->sprite->canDrag = 1;


	/*
	//char *s =NULL;
	//txt = Text_setText(txt,SDL_GetBasePath());
	txt = Text_setText(txt,"hello");
	txt=Text_appendText(txt,"\n");
	txt = Text_appendText(txt,SDL_GetBasePath());
	txt=Text_appendText(txt,"\n");
	*/
	//txt = Text_setText(txt,"hello");
	//SDL_Log("basepath:%s\n",s);
	//Text_setText(txt,"");
	//Text_appendText(txt,"\n");
	//Text_appendText(txt,"\n");
	//Text_appendText(txt,"1234567890abcdefghijklmnopqrst:" );
	//txt = Text_appendText(txt,"1234567890\nabcdefghijklmnopqrstuvwxwz\nABCDEFGHIJKLMNOPQRSTUVWXYZ:" );
	//txt = Text_appendText(txt,"1234567890:" );
	//txt = Text_setText(txt,getLinkedVersionString());
	//txt = Text_appendText(txt,"abcdefghijklmnopqrstuvwxyz:" );
	txt = Text_appendText(txt,"ABCD\rEFG\nHIJKLMN\nOPQRSTUVWXYZ_:" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = Text_appendText(txt,"一二三四五六七八九十一二三四五六七八九十end\n");
	//txt = Text_setText(txt,"一二三四五六七八九十一二三四五六七八九十end\n");
	//txt = Text_setText(txt,"hello");
	txt = Text_appendText(txt,"一\r二\n三\r四\n五六七八九十一二三四五六七八九十end\n");
	Sprite_addChild(stage->sprite,txt->sprite); 
	Stage_loopEvents(); return 0;
	//Text_appendText(txt,"\npref path:\n" );
	//Text_appendText(txt,SDL_GetPrefPath("test", "subsystem2"));

}
#endif


