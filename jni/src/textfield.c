/**
 * @file textfield.c
 gcc -Wall -I"../SDL2/include/"  -I"../SDL2_ttf/" myfont.c loading.c ease.c tween.c array.c utf8.c update.c httploader.c ipstring.c urlcode.c base64.c  bytearray.c zip.c files.c myregex.c sdlstring.c textfield.c matrix.c sprite.c mystring.c  -lSDL2_ttf -lz -lssl -lcrypto -lpthread -lm -lSDL2 -D debugtext  && ./a.out
 gcc -g -Wall -I"../SDL2/include/" -I"../SDL2_image/" -I"../SDL2_ttf/" array.c tween.c ease.c textfield.c matrix.c sprite.c sdlstring.c mystring.c files.c -lSDL2_ttf -lmingw32 -lSDL2main -lSDL2 -D debugtext && a
 *  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2015-07-21
 */
#include "textfield.h"

static void mouseWheels(SpriteEvent*e);

void TextWord_clear(TextWord*textword)
{
	if(textword == NULL)
		return;

	free(textword);
	textword = NULL;
}

void TextLine_clear(TextLine*line)
{
	if(line== NULL)
		return;

	//TextWord* word = line->lastWord;
	//if(word){
	//TextWord_clear(word);
	//free(word);
	//line->lastWord = NULL;
	//}
	if(line->words)
	{
		int i = line->words->length;
		while(i>0)
		{
			--i;
			TextWord* word = Array_getByIndex(line->words,i);
			if(word)
				TextWord_clear(word);
		}
		Array_clear(line->words);
		line->words = NULL;
	}

	if(line->text) {
		free(line->text);
		line->text = NULL;
	}
	free(line);
	line = NULL;
}

static void TextField_clearLines(TextField * textfield)
{
	if(textfield->textLines)
	{
		int i = textfield->textLines->length;
		while(i>0){
			--i;
			TextLine* line = Array_getByIndex(textfield->textLines,i);
			TextLine_clear(line);
		}
		Array_clear(textfield->textLines);
		textfield->textLines = NULL;
	}
}

void TextField_clear(TextField*textfield)
{
	if(textfield == NULL)
		return;

	//SDL_DestroyMutex(textfield->mutex);

	if(textfield->font)
	{
		textfield->font= NULL;
	}

	TextField_clearLines(textfield);

	if(textfield->textColor){
		free(textfield->textColor);
		textfield->textColor = NULL;
	}

	if(textfield->backgroundColor){
		free(textfield->backgroundColor);
		textfield->backgroundColor= NULL;
	}

	if(textfield->text) {
		free(textfield->text);
		textfield->text = NULL;
	}
	if(textfield->sprite) {
		Sprite_removeChildren(textfield->sprite);
		Sprite_destroy(textfield->sprite);
		textfield->sprite = NULL;
	}
	if(textfield->posSprite) {
		Sprite_removeChildren(textfield->posSprite);
		Sprite_destroy(textfield->posSprite);
		textfield->posSprite= NULL;
	}
	free(textfield);
	textfield = NULL;
}


int TextField_getMoreDrawHeight(TextField*textfield)
{
	if(!textfield->staticHeight)
		return 0;
	return stage->stage_h/2;
}

TextWord * Textword_new(TTF_Font * font, Uint8 * curChar)
{
	TextWord* textword = (TextWord*)malloc(sizeof(TextWord));
	memset(textword,0,sizeof(TextWord));

	textword->numbyte = UTF8_numByte((char*)curChar);
	if(('A'<=curChar[0] && curChar[0]<='Z') || ('a'<=curChar[0] && curChar[0]<='z'))
	{//单词不分段
		while(
				('a'<=curChar[textword->numbyte] && curChar[textword->numbyte]<='z')
				|| ('A'<=curChar[textword->numbyte] && curChar[textword->numbyte]<='Z')
			 )
			textword->numbyte++;
	}

	char word[textword->numbyte+1];
	memset(word,0,textword->numbyte + 1);
	memcpy(word,curChar,textword->numbyte);

	TTF_SizeUTF8(font,word,&(textword->w),&(textword->h));
	return textword;
}

TextLine * TextField_getLastLine(TextField * textfield)
{
	if(textfield==NULL){
		SDL_Log("TextField_getLastLine Error: textfield is NULL!\n");
		return NULL;
	}
	if(textfield->textLines == NULL){
		TextLine*line = TextLine_new();
		textfield->textLines = Array_push(NULL,line);
		//textfield->lastLine = line;
		//textfield->lines = line;
	}
	//return textfield->lastLine;
	return Array_getByIndex(textfield->textLines,textfield->textLines->length-1);
}

TextLine * TextField_appenLine(TextField*textfield)
{
	if(textfield==NULL){
		SDL_Log("TextField_appenLine Error: textfield is NULL!\n");
		return NULL;
	}
	TextLine*line = TextLine_new();
	//if(textfield->lastLine){
	//line->prev = textfield->lastLine;
	//textfield->lastLine->next = line;
	//}else{
	//textfield->lines = line;
	//}
	//textfield->lastLine = line;
	//textfield->numLines ++;
	//line->lineId= textfield->numLines;
	line->lineId= textfield->textLines->length;
	textfield->textLines = Array_push(textfield->textLines,line);
	line->rect.y = textfield->textHeight;
	textfield->textHeight = line->rect.y + line->rect.h;
	return line;
}

SDL_bool TextField_lineFull(TextField*textfield,TextLine*line,TextWord*textword)
{
	if(textfield && line && textword){
		if((textword->w) + (line->rect.w) > (textfield->w)) {
			return SDL_TRUE;
		}

		char *lastWord = line->text + line->numbyte -1;

		//if((line->lastWord) && (line->lastWord->numbyte==1) && ((*(lastWord) == '\r') || (*(lastWord) == '\n' ))) 
		if((line->words) && line->words->length>0 && ((*(lastWord) == '\r') || (*(lastWord) == '\n' ))) 
		{
			return SDL_TRUE;
		}
	}
	return SDL_FALSE;
}


void setLineTexture(TextField*textfield,TextLine*line)
{

	if(!textfield->staticHeight ||
			(textfield->scrollV + line->rect.y - line->rect.h/2 <= textfield->h //bottom
			 && textfield->y + textfield->scrollV + line->rect.y + line->rect.h/2 >= 0)// top
	  ) 
	{
		//printf(" Surface_new ERROR!\n");fflush(stdout);
		SDL_Surface * surface = NULL;
		if(textfield->backgroundColor)
		{
			surface= TTF_RenderUTF8_Shaded(textfield->font, line->text, *(textfield->textColor),*(textfield->backgroundColor));
		}
		else
		{
			//surface= TTF_RenderUTF8_Blended(textfield->font, line->text, *(textfield->textColor));
			surface= TTF_RenderUTF8_Solid(textfield->font, line->text, *(textfield->textColor));
		}

		if(surface){
			//line->rect.w = surface->w;
			//line->rect.h = surface->h;//初始宽高
			{
				//SDL_BlendMode saved_mode;
				SDL_Rect dstrect;
				dstrect.x = line->rect.x;
				if(textfield->staticHeight){
					dstrect.y = line->rect.y + textfield->scrollV ;
				}else{
					dstrect.y = line->rect.y;
				}
				dstrect.w = line->rect.w;
				dstrect.h = line->rect.h;
				//SDL_GetSurfaceBlendMode(textfield->sprite->surface, &saved_mode);
				//SDL_SetSurfaceBlendMode(textfield->sprite->surface, SDL_BLENDMODE_NONE);
				SDL_BlitSurface(surface, NULL, textfield->sprite->surface, &dstrect);
				//SDL_SetSurfaceBlendMode(textfield->sprite->surface, saved_mode);

			}
			SDL_FreeSurface(surface); surface = NULL;
		}
	}
}


int TextField_getMaxScrollV(TextField *textfield)
{
	if(textfield && textfield->font)
		if(textfield->textHeight > textfield->h)
			return (textfield->h - textfield->textHeight);
	return 0;
}
void TextField_setScrollV(TextField* textfield,int i)
{
	textfield->scrollV = i;
	if(!textfield->staticHeight){
		if(textfield->textHeight > textfield->h)
			textfield->sprite->y = i;
	}
	if(textfield->staticHeight){
		if(textfield->scrollV>TextField_getMoreDrawHeight(textfield))
			textfield->scrollV = TextField_getMoreDrawHeight(textfield);
		else if(textfield->textHeight>textfield->h && textfield->scrollV< textfield->h - textfield->textHeight - TextField_getMoreDrawHeight(textfield)){
			textfield->scrollV = textfield->h - textfield->textHeight - TextField_getMoreDrawHeight(textfield);
		}
	}else{
		if(textfield->scrollV>0)
			textfield->scrollV = 0;
		else if(textfield->textHeight>textfield->h && textfield->scrollV< textfield->h - textfield->textHeight){
			textfield->scrollV = textfield->h - textfield->textHeight;
		}
	}
}

void TextField_drawPostionBar(TextField*textfield)
{/*{{{*/
	if(textfield->h < textfield->textHeight && textfield->staticHeight){
		Sprite * sprite = textfield->sprite;
		Sprite * posSprite = textfield->posSprite;
		int moreDrawHeight = TextField_getMoreDrawHeight(textfield);

		float posRate = -(float)(sprite->y+textfield->scrollV)/(textfield->textHeight-(textfield->h-moreDrawHeight*2));
		if(posRate<0)
			posRate = 0.0;
		else if(posRate>1)
			posRate = 1.0;
		if((posRate-textfield->posRate)>=.001 || (posRate-textfield->posRate)<-.01)
		{
			textfield->posRate = posRate;
			//printf("posRate:%f\n",posRate); fflush(stdout);
			if(sprite->parent)
			{
				if(textfield->posSprite==NULL)
				{
					textfield->posSprite = Sprite_new();
					textfield->posSprite->surface = Surface_new(2,2);
					SDL_FillRect(textfield->posSprite->surface,NULL,SDL_MapRGB(textfield->posSprite->surface->format, 255, 0, 0));
				}
				posSprite = textfield->posSprite;
				textfield->posSprite->visible = 1;
				posSprite->w = 2;
				posSprite->h = (textfield->h-moreDrawHeight*2)*(float)(textfield->h-moreDrawHeight*2)/(textfield->textHeight-(textfield->h-moreDrawHeight*2));
				if(posSprite->h<2) posSprite->h = 2;
				posSprite->x = textfield->x + textfield->w -posSprite->w;
				posSprite->y = posRate*(textfield->h-moreDrawHeight*2-posSprite->h);
				Sprite_addChild(sprite->parent,textfield->posSprite);
			}
		}
	}else{
		if(textfield->posSprite)
			textfield->posSprite->visible = 0;
	}
	Stage_redraw();
}/*}}}*/

char * TextField_getWordAtPoint(TextField * textfield,int x, int y)// 在 x 和 y 参数指定的位置返回从零开始的字符索引值。
{

	int lineIndex= TextField_getLineIndexAtPoint(textfield,x,y);
	/*
	   char * lineStr = TextField_getLineText(textfield,lineIndex);
	   if(lineStr==NULL )
	   return NULL;
	   */

	TextLine * line = Array_getByIndex(textfield->textLines,lineIndex);
	Array * words = line->words;
	int i = 0;
	int offset = line->indexInText;
	int w= 0;
	TextWord * word;
	while(i<words->length)
	{
		word = Array_getByIndex(words,i);
		w += word->w;
		if(w>x)
			break;

		offset += word->numbyte;
		++i;
	}
	char * curWord = getSubStr(textfield->text,offset,word->numbyte);
	SDL_Log("curWord: %s",curWord);
	if(curWord==NULL)
	{
		return NULL;
	}
	if(strlen(curWord)==0)
	{
		free(curWord);
		return NULL;
	}
	return curWord;
}
char * TextField_getLineText(TextField * textfield,int lineIndex)// 返回 lineIndex 参数指定的行的文本。
{
	if(textfield->textLines==NULL || lineIndex>=textfield->textLines->length)
		return NULL;
	TextLine * line = Array_getByIndex(textfield->textLines,lineIndex);
	char * lineStr = getSubStr(textfield->text,line->indexInText,line->numbyte);
	if(strlen(lineStr)==0)
	{
		free(lineStr);
		return NULL;
	}
	SDL_Log("line(%d)%s",lineIndex,lineStr);
	return lineStr;
}
int TextField_getLineIndexAtPoint(TextField * textfield,int x, int y)// 在 x 和 y 参数指定的位置返回从零开始的行索引值。
{
	if(textfield->textLines==NULL)
		return 0;
	Sprite * sprite = textfield->sprite;
	int fontheight = TTF_FontHeight(textfield->font);
	int mouseX = x - sprite->Bounds->x;
	int mouseY = y - sprite->Bounds->y - textfield->scrollV;
	int lineIndex= mouseY/fontheight;
	SDL_Log("sprite->Bounds: (%d,%d,%d,%d)",
			mouseX,
			mouseY,
			sprite->Bounds->w,
			sprite->Bounds->h
		   );
	if(lineIndex>=textfield->textLines->length)
		lineIndex= textfield->textLines->length-1;
	return lineIndex;
}

static int stageMouseY;
static int stageMouseX;
static void mouseDown(SpriteEvent*e)
{
	SDL_Event *event  = e->e;
	/*
	   if(event->type == SDL_MOUSEWHEEL)
	   SDL_Log("SDL_MOUSEWHEEL:timestamp:%d,windowID:%d,which:%d,x:%d,y:%d,direction:%d\n",
	   event->wheel.timestamp,
	   event->wheel.windowID,
	   event->wheel.which,
	   event->wheel.x,
	   event->wheel.y
	   ,event->wheel.direction
	   );

*/
	Sprite *sprite = e->target;
	TextField * textfield = (TextField*)(sprite->obj);
	if(textfield->wordSelect){
		char * curWord = NULL;
		switch(event->type)
		{
			case SDL_MOUSEBUTTONDOWN:
				stageMouseY = stage->mouse->y;
				stageMouseX = stage->mouse->x;
				Sprite_addEventListener(sprite,SDL_MOUSEBUTTONUP,mouseDown);
				break;
			case SDL_MOUSEBUTTONUP:
				if(abs(stageMouseY-stage->mouse->y)>5 || abs(stageMouseX - stage->mouse->x)>5)
					return;
				Sprite_removeEventListener(sprite,SDL_MOUSEBUTTONUP,mouseDown);
				if(textfield->textHeight<1)
					return;
				curWord = TextField_getWordAtPoint(textfield,stage->mouse->x,stage->mouse->y);
				if(curWord){
					textfield->wordSelect(curWord);
				}else{
					return;
				}
		}
		if(curWord) free(curWord);
	}
}


int drawLines(TextField*textfield)
{
	Sprite *sprite = textfield->sprite;

	if(sprite==NULL)
	{
		printf("no sprite\n");fflush(stdout);
		return 0;
	}
	Sprite_addEventListener(sprite,SDL_MOUSEBUTTONDOWN,mouseDown);

	if(textfield->mouseWheelEnabled){
		Sprite_addEventListener(sprite,SDL_MOUSEWHEEL,mouseWheels);
	}
	if(sprite->canDrag){
		Sprite_addEventListener(sprite,SDL_MOUSEMOTION,mouseWheels);
		if(textfield->staticHeight) Sprite_addEventListener(sprite,SDL_MOUSEBUTTONUP,mouseWheels);
	}

	//SDL_SpinLock lock = 0;
	//SDL_AtomicLock(&lock);

	SDL_Surface * surface = sprite->surface;
	if(textfield->staticHeight || 
			(surface && (surface->w != textfield->textWidth || surface->h != textfield->textHeight))){
		Sprite_destroySurface(sprite);
		Sprite_destroyTexture(sprite);
	}

	int fontheight = TTF_FontHeight(textfield->font);

	if((stage->renderer && sprite->surface == NULL && sprite->texture==NULL)// 2d 初始化
			|| (stage->GLEScontext && (sprite->surface==NULL && sprite->data3d==NULL)) //3d 初始化
	  )
	{
		if(textfield->staticHeight){
			textfield->h = TextField_getMoreDrawHeight(textfield)*2 + stage->stage_h;
			sprite->surface = Surface_new(textfield->w,textfield->h);
			if(sprite->surface==NULL) {
				//printf("%s\n Surface_new ERROR!\n",textfield->text);fflush(stdout);
			}
			//SDL_FillRect(sprite->surface,NULL,SDL_MapRGB(sprite->surface->format, 255, 0, 0));
			sprite->w = textfield->w;
			sprite->h = textfield->h;
		}else{
			sprite->surface = Surface_new(textfield->textWidth,textfield->textHeight);
			if(sprite->surface==NULL) {
				//printf("%s\n Surface_new ERROR!\n",textfield->text);fflush(stdout);
			}
			if(sprite->w != textfield->textWidth) sprite->w = textfield->textWidth;
			if(sprite->h != textfield->textHeight) sprite->h = textfield->textHeight;
		}

		if(textfield->staticHeight){
			//int old_scroll = textfield->scrollV;
			textfield->scrollV += sprite->y-textfield->y+TextField_getMoreDrawHeight(textfield);
			sprite->y = textfield->y-TextField_getMoreDrawHeight(textfield);
		}
		int i = 0;

		while(i<textfield->textLines->length)
		{
			//TextLine * line = textfield->lines;
			TextLine * line = Array_getByIndex(textfield->textLines,i);
			if(line && sprite->surface){
				if(textfield->staticHeight){
					int curY = line->rect.y + textfield->scrollV;
					int curH = fontheight;
					if(0 <= curY+curH && curY < textfield->h)
						setLineTexture(textfield,line);
					if(curY >= textfield->h){
						break;
					}
				}else{
					setLineTexture(textfield,line);
				}
				//line= line->next;
				++i;
			}
		}
	}


	if(!textfield->staticHeight){
		Sprite * target = sprite;
		SDL_Rect *rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
		rect->x = textfield->x;
		rect->y = textfield->y - textfield->textHeight + fontheight;
		rect->w = 0;
		rect->h = textfield->textHeight - fontheight;
		if(target->dragRect)
			free(target->dragRect);
		target->dragRect = rect;
		Sprite_limitPosion(target,target->dragRect);
	}

	//SDL_AtomicUnlock(&lock);
	TextField_drawPostionBar(textfield);
	return 0;
}


static void mouseWheels(SpriteEvent*e)
{
	SDL_Event *event  = e->e;
	/*
	   if(event->type == SDL_MOUSEWHEEL)
	   SDL_Log("SDL_MOUSEWHEEL:timestamp:%d,windowID:%d,which:%d,x:%d,y:%d,direction:%d\n",
	   event->wheel.timestamp,
	   event->wheel.windowID,
	   event->wheel.which,
	   event->wheel.x,
	   event->wheel.y
	   ,event->wheel.direction
	   );

*/
	Sprite *sprite = e->target;
	TextField * textfield = (TextField*)(sprite->obj);
	if(textfield->textHeight<1)
		return;

	int moreDrawHeight = TextField_getMoreDrawHeight(textfield);

	if(event->type == SDL_MOUSEBUTTONUP && (sprite->y+moreDrawHeight!=textfield->y)){
		//SDL_Log("mouse---------------\n");fflush(stdout);
		drawLines(textfield);
		return;
	}


	int fontheight = TTF_FontHeight(textfield->font);
	if(textfield && textfield->textLines && textfield->textLines->length>0){
		if(textfield->staticHeight){
			//SDL_Log("drag staticHeight---------------\n");fflush(stdout);
			sprite->canDrag = 0;
			int deltaY = 0;
			if(event->type == SDL_MOUSEWHEEL){
				if(event->wheel.y > 0){
					deltaY = fontheight;
				}else if(event->wheel.y < 0){
					deltaY = -fontheight;
				}
				int old_scroll = textfield->scrollV;
				TextField_setScrollV(textfield,textfield->scrollV + deltaY);
				if(old_scroll!=textfield->scrollV)drawLines(textfield);
			} else if(event->type == SDL_MOUSEMOTION){
				if(event->motion.state){
					if(abs(event->motion.xrel)<20 && abs(event->motion.xrel)<20)
					{
						//TextField_setScrollV(textfield,textfield->scrollV + event->motion.yrel);
						//textfield->scrollV = event->motion.yrel;
						sprite->y += event->motion.yrel;
						if(sprite->y + textfield->scrollV > textfield->y)//top
							sprite->y = textfield->y - textfield->scrollV;
						else if((sprite->y + textfield->scrollV) +(textfield->textHeight-TextField_getMoreDrawHeight(textfield)*1.0) < textfield->y)//bottom
							sprite->y = textfield->y - (textfield->textHeight-TextField_getMoreDrawHeight(textfield)*1.0) - textfield->scrollV;

						TextField_drawPostionBar(textfield);
					}
				}
			}
		}else {
			if(event->type == SDL_MOUSEWHEEL){
				if(event->wheel.y > 0){
					//sprite->y += textfield->lines->rect.h;
					sprite->y += fontheight;
				}else if(event->wheel.y < 0){
					//sprite->y -= textfield->lines->rect.h;
					sprite->y -= fontheight;
				}
				drawLines(textfield);
			}else if(event->motion.state){
				if(abs(event->motion.xrel)<20 && abs(event->motion.xrel)<20)
				{
					sprite->y += event->motion.yrel;
					if(sprite->y + textfield->scrollV > textfield->y)//top
						sprite->y = textfield->y - textfield->scrollV;
					else if((sprite->y + textfield->scrollV) +(textfield->textHeight-TextField_getMoreDrawHeight(textfield)*2) < textfield->y)//bottom
						sprite->y = textfield->y - (textfield->textHeight-TextField_getMoreDrawHeight(textfield)*2) - textfield->scrollV;

					TextField_drawPostionBar(textfield);
				}
			}
		}
	}

}

char * getTextLineText(TextField*textfield,TextLine*line)
{
	if(line){
		if(line->text){
			free(line->text);
			line->text = NULL;
		}
		if(line->numbyte > 0){
			line->text = malloc(line->numbyte+1);
			memset(line->text,0,line->numbyte+1);
			memcpy(line->text,textfield->text+line->indexInText,line->numbyte);
		}
		return line->text;
	}
	return NULL;
}

TextField *TextField_appendText(TextField*textfield,char*s)
{
	if(s == NULL || strlen(s)==0)
		return textfield;
	if(textfield == NULL)
		textfield = TextField_new();

	/*
	   if (SDL_LockMutex(textfield->mutex) < 0) {
	   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock mutex: %s", SDL_GetError());
	   exit(1);
	   }
	   */

	int dealedlen = 0;
	if(textfield->text) {
		dealedlen = strlen(textfield->text);
	}

	textfield->text = contact_str(textfield->text,s);
	//printf("textfield:%s\n",textfield->text);fflush(stdout);
	//return textfield;

	TextLine*line = TextField_getLastLine(textfield);//最后一行

	while(dealedlen < strlen(textfield->text))
	{
		TextWord* textword = Textword_new(textfield->font,(Uint8*)(textfield->text+dealedlen));

		if(TextField_lineFull(textfield,line,textword)){//行满,另起一行
			//SDL_Log("--------------------------------------------------TextField_lineFull\n");
			//if(line) SDL_Log("-line:%d:%s\n",line->lineId,line->text);
			line = TextField_appenLine(textfield);
			line->indexInText = dealedlen;
		}
		//if(line->lastWord){
		//free(line->lastWord);
		//}
		//line->lastWord = textword;
		/*
		   TextWord * lastWord = Array_getByIndex(line->words,line->words->length-1);
		   if(lastWord!=NULL)
		   {
		   free(lastWord);
		   Array_setByIndex(line->words,line->words->length-1,NULL);
		   }
		   */
		line->words = Array_push(line->words,textword);
		line->numbyte += textword->numbyte;
		line->rect.w += textword->w;// line w
		if(line->rect.h < textword->h){
			line->rect.h = textword->h;//line h
		}
		if(textfield->textWidth < line->rect.w){
			textfield->textWidth = line->rect.w;
		}
		if(textfield->textHeight < line->rect.y + line->rect.h)
			textfield->textHeight = line->rect.y + line->rect.h;//textHeight

		dealedlen += textword->numbyte;
		getTextLineText(textfield,line);
		//setLineTexture(textfield,line);
	}
	//SDL_Log("dealedlen:%d %d\n",dealedlen,strlen(textfield->text));fflush(stdout);

	if(textfield->textWidth > stage->stage_h*3 || textfield->textHeight> stage->stage_h*3)
		textfield->staticHeight = SDL_TRUE;
	else
		textfield->staticHeight = 0;

	//printf("textfield:%s\n",textfield->text);fflush(stdout);
	SDL_Log("textfield->textWidth:%d textfield->textHeight:%d \n",textfield->textWidth,textfield->textHeight);fflush(stdout);
	drawLines(textfield);


	/*
	   if (SDL_UnlockMutex(textfield->mutex) < 0) {
	   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't unlock mutex: %s", SDL_GetError());
	   exit(1);
	   }
	   */
	return textfield;
}

TextField* TextField_new()
{
	if(!TTF_WasInit())
	{
		if ( TTF_Init() < 0 ) {
			fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
			SDL_Quit();
			return NULL;
		}
	}

	TextField* textfield = (TextField*)malloc(sizeof(TextField));
	memset(textfield,0,sizeof(TextField));
	textfield->w = 100;
	textfield->h = 100;
	textfield->sprite = Sprite_new();
	textfield->sprite->mouseChildren = SDL_FALSE;
	textfield->sprite->obj = textfield;
	textfield->mouseWheelEnabled = 1;

	if(stage==NULL)
		Stage_init();

	if(textfield->textColor==NULL)
		textfield->textColor = uintColor(0x00ff0000);

	if(textfield->font==NULL){
		int stageSize = max(stage->stage_w,stage->stage_h);
		int textSize = 6 * stageSize/320;
		if(textSize<12)
			textSize = 12;
		textfield->font = getDefaultFont(textSize);
	}


	/*
	   if ((textfield->mutex = SDL_CreateMutex()) == NULL) {
	   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create mutex: %s\n", SDL_GetError());
	   exit(1);
	   }
	   */


	return textfield;
}

TextLine * TextLine_new()
{
	if(!TTF_WasInit())
	{
		if ( TTF_Init() < 0 ) {
			fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
			SDL_Quit();
			return NULL;
		}
	}

	TextLine* textline = (TextLine*)malloc(sizeof(TextLine));
	memset(textline,0,sizeof(TextLine));
	memset(&(textline->rect) ,0 ,sizeof(SDL_Rect));
	if(stage==NULL)
		Stage_init();
	return textline;
}


TextField * TextField_setText(TextField*textfield,char *s)
{
	if(textfield==NULL)
		textfield = TextField_new();

	/*
	   if (SDL_LockMutex(textfield->mutex) < 0) {
	   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock mutex: %s", SDL_GetError());
	   exit(1);
	   }
	   */

	//textfield->numLines = 0;
	//textfield->lines = NULL;
	//textfield->lastLine = NULL;
	TextField_clearLines(textfield);
	textfield->textWidth = 0;
	textfield->textHeight = 0;
	textfield->length = 0;
	textfield->scrollV= 0;
	textfield->staticHeight = 0;

	//TextLine* line = textfield->lines;
	//while(line){
	//TextLine_clear(line);
	//line = line->next;
	//}
	//textfield->lines = NULL;

	if(textfield->text) {
		free(textfield->text);
	}
	textfield->text = NULL;

	if(textfield->sprite) {
		Sprite_destroySurface(textfield->sprite);
		Sprite_destroyTexture(textfield->sprite);
		textfield->sprite->canDrag = 1;
		/*
		   Sprite_removeChildren(textfield->sprite);
		   Sprite_destroy(textfield->sprite);
		   textfield->sprite = NULL;
		   */
	}
	if(textfield->sprite==NULL){
		textfield->sprite = Sprite_new();
		textfield->w = 100;
		textfield->h = 100;
		textfield->sprite->mouseChildren = SDL_FALSE;
		textfield->sprite->obj = textfield;
		textfield->mouseWheelEnabled = 1;

		if(stage==NULL)
			Stage_init();
	}
	textfield->sprite->y = textfield->y;
	/*
	   if (SDL_UnlockMutex(textfield->mutex) < 0) {
	   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't unlock mutex: %s", SDL_GetError());
	   exit(1);
	   }
	   */

	if(s){
		if(textfield->posSprite)
			textfield->posSprite->visible = 0;

		textfield->text = contact_str(textfield->text,s);
		//printf("textfield->text = %s\n",textfield->text); fflush(stdout);
		TextField_appendText(textfield,textfield->text);
	}
	return textfield;
}


#ifdef debugtext
#include "sdlstring.h"
int main(int argc, char *argv[])
{
	Stage_init();
	TextField* txt = TextField_new();//txt = TextField_setText(txt,getLinkedVersionString());
	//txt->x = stage->stage_w/4;
	//txt->y = stage->stage_h/4;
	//txt->font = getFontByPath("DroidSansFallback.ttf",24);
	txt->font = getDefaultFont(82);
	TTF_SetFontOutline(txt->font,1);
	//TTF_SetFontHinting(textfield->font,TTF_HINTING_LIGHT);
	//TTF_SetFontHinting(textfield->font,TTF_HINTING_MONO);
	txt->w = stage->stage_w;
	txt->h = stage->stage_h;
	txt->sprite->canDrag = 1;


	//char *s =NULL;
	txt = TextField_setText(txt,SDL_GetBasePath());
	txt=TextField_appendText(txt,"\n");
	txt = TextField_appendText(txt,SDL_GetBasePath());
	//SDL_Log("basepath:%s\n",s);
	//TextField_setText(txt,"");
	//TextField_appendText(txt,"\n");
	//TextField_appendText(txt,"\n");
	//TextField_appendText(txt,"1234567890abcdefghijklmnopqrst:" );
	//txt = TextField_appendText(txt,"1234567890\nabcdefghijklmnopqrstuvwxwz\nABCDEFGHIJKLMNOPQRSTUVWXYZ:" );
	//txt = TextField_appendText(txt,"1234567890:" );
	//txt = TextField_setText(txt,getLinkedVersionString());
	//txt = TextField_appendText(txt,"abcdefghijklmnopqrstuvwxyz:" );
	txt = TextField_appendText(txt,"ABCDEFGHIJKLMNOPQRSTUVWXYZ_:āáǎàōóǒòēéěèīíǐìūúǔùǖǘǚǜ" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
#if 0
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"01234567890abcdefghijklmnopqrstuvwxyz,< >中间:ABCDEFGHIJKLMNOPQRSTUVWXYZ==" );
	txt = TextField_appendText(txt,"一二三四五六七八九十一二三四五六七八九十end\n");
#endif
	//txt = TextField_setText(txt,"一二三四五六七八九十一二三四五六七八九十end\n");
	txt = TextField_appendText(txt,"一二三四五六七八九十一二三四五六七八九十end\n");
	Sprite_addChild(stage->sprite,txt->sprite); 
	Stage_loopEvents(); return 0;
	//TextField_appendText(txt,"\npref path:\n" );
	//TextField_appendText(txt,SDL_GetPrefPath("test", "subsystem2"));

}
#endif


