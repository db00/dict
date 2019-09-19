/**
 *
 gcc -g -Wall -D debug_ime -Wall utf8.c update.c ipstring.c base64.c urlcode.c zip.c httploader.c bytearray.c myregex.c myfont.c files.c array.c ease.c tween.c input.c mystring.c sprite.c matrix.c textfield.c -lm -I"../SDL2/include/"  -I"../SDL2_ttf/" -lpthread -lssl -lz -lcrypto -lSDL2 -lSDL2_ttf  && ./a.out
 */

#include "input.h"

static void Input_redraw(Input *input) {
	input->textfield = TextField_setText(input->textfield,input->value);
	TextField_setScrollV(input->textfield,TextField_getMaxScrollV(input->textfield));
	input->rect->x = input->sprite->x;
	input->rect->y = input->sprite->y;
	input->rect->w = input->sprite->w;
	input->rect->h = input->sprite->h;
#if 0
	/*
	//printf("rect:%d,%d,%d,%d\n",\
	input->rect->x,\
	input->rect->y,\
	input->rect->w,\
	input->rect->h\
	);
	*/
#endif
	SDL_SetTextInputRect(input->rect);
	Stage_redraw();
}

static void textChangFunc(Input * input)
{
	if(input->textChangFunc)
		input->textChangFunc(input);
}

char * Input_setText(Input * input,char * s)
{
	s = contact_str("",s);
	free(input->value);
	input->value = s;
	Input_redraw(input);
	return input->value;
}

static unsigned int timestamp;
static void keydownEvent(SpriteEvent* e){
	SDL_Event *event = e->e;
	if(Sprite_getVisible(e->target)==0)
		return;
	Input* input = e->target->obj;
	char * _value;
	if(event->key.timestamp-timestamp<20){
		return;
	}


	//SDL_Log("-----keydownEvent: %08X,%08X\n",event->key.keysym.sym,event->key.keysym.scancode);
	switch (event->key.keysym.sym)
	{
		case SDLK_RETURN:
			//if(strlen(input->value)) memset(input->value,0,strlen(input->value));
			_value = contact_str(input->value,"\n");
			free(input->value);
			input->value = _value;
			textChangFunc(input);
			break;
		case SDLK_BACKSPACE:
			timestamp = event->key.timestamp;
			SDL_Log("--------------timestamp:%d,%d",timestamp,event->type);
			//printf("SDLK_BACKSPACE\n");
			if (input->value && strlen(input->value)>0)
			{
				Array * arr = UTF8_each(input->value);
				if(arr && arr->length>0)
				{
					arr = Array_removeByIndex(arr,arr->length-1);
					if(arr && arr->length>0)
					{
						_value = Array_joins(arr,"");
						free(input->value);
						input->value = _value;
					}else{
						memset(input->value,0,strlen(input->value));
					}
				}
				Array_freeEach(arr);
				textChangFunc(input);
			}
			break;
		default:
			if(strlen(input->value)==0)
				SDL_StartTextInput();
	}
	Input_redraw(input);
	return;
}


static void textinputEvent(SpriteEvent*e)
{
	SDL_Event * event = (SDL_Event *)(e->e);
	if(Sprite_getVisible(e->target)==0)
		return;
	if (event->text.text[0] == '\0'){
		return;
	}

	//SDL_Log("------Keyboard: text input \"%s\"\n", event->text.text);
	Input* input = e->target->obj;
	if(input->value){
		char * _value = contact_str(input->value,event->text.text);
		free(input->value);
		input->value = _value;
		textChangFunc(input);
	}
	//SDL_Log("-----text inputed: %s\n", input->value);

	Input_redraw(input);
}

static void texteditingEvent(SpriteEvent*e){
	//SDL_Event* event = (SDL_Event*)(e->e);
	Input* input = e->target->obj;
	//SDL_Log("-------text editing \"%s\", selected range (%d, %d)\n", event->edit.text, event->edit.start, event->edit.length);

	Input_redraw(input);
}

static void Input_fingerEvent(SpriteEvent*e)
{
	if(e){
		stage->focus = e->target;
		SDL_StartTextInput();
	}else{
		stage->focus = NULL;
		SDL_StopTextInput();
	}
}

Input * Input_new(int w,int h)
{
	Input * input = malloc(sizeof(Input));
	memset(input,0,sizeof(Input));
	TextField * textfield = TextField_new();
	textfield->font = getDefaultFont(h);
	//textfield->textColor = uintColor(0x330000ff);
	input->textfield = textfield;
	textfield->w = w;
	textfield->h = h;
	input->value = malloc(2);
	memset(input->value,0,2);
	TextField_setText(textfield,input->value);
	Sprite * sprite = Sprite_new();
	input->sprite = sprite;
	sprite->w = w;
	sprite->h = h;
	Sprite * bg = Sprite_new();
	bg->surface = Surface_new(1,1);
	char pixels[4] ={'\xff','\0','\0','\xff'};
	memcpy(bg->surface->pixels,(char*)pixels,sizeof(pixels));
	bg->w= w;
	bg->h= h;
	Sprite_addChild(sprite,bg);
	Sprite_addChild(sprite,textfield->sprite);
	sprite->mouseChildren = SDL_FALSE;
	input->rect = malloc(sizeof(SDL_Rect));
	input->rect->x = sprite->x;
	input->rect->y = sprite->y;
	input->rect->w = sprite->w;
	input->rect->h = sprite->h;

	sprite->obj = input;
	Sprite_addEventListener(sprite,SDL_MOUSEBUTTONDOWN,Input_fingerEvent); 
	//Sprite_addEventListener(sprite,SDL_FINGERDOWN,Input_fingerEvent); 
	Sprite_addEventListener(sprite,SDL_KEYDOWN,keydownEvent); 
	Sprite_addEventListener(sprite,SDL_TEXTINPUT,textinputEvent); 
	Sprite_addEventListener(sprite,SDL_TEXTEDITING,texteditingEvent); 
	return input;
}

void Input_clear(Input * input)
{
	if(input)
	{
		if(stage->focus==input->sprite)
			stage->focus = NULL;
		TextField_clear(input->textfield);
		Sprite_destroy(input->sprite);
		free(input->rect);
		free(input->value);
		free(input);
	}
}

#ifdef debug_ime
int main(int argc, char *argv[]) {
	Stage_init();
	if(stage==NULL)return 0;

	SDL_StartTextInput();

	Input * input = Input_new(100,100);
	Sprite_addChild(stage->sprite,input->sprite);
	input->sprite->x = 100;
	input->sprite->y = 100;

	Input * input2 = Input_new(100,100);
	Sprite_addChild(stage->sprite,input2->sprite);
	input2->sprite->x = 0;
	input2->sprite->y = 0;

	Stage_loopEvents();
	exit(0);
	return 0;
}

#endif
