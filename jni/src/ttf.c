/**
 * @file ttf.c
 gcc -g -D debug_ttf -Wall -I"../SDL2/include/" -I"../SDL2_image" -I"../SDL2_ttf/" -lSDL2_ttf -lSDL2_image -lSDL2 utf8.c myfont.c update.c loading.c zip.c bytearray.c textfield.c httpserver.c array.c filetypes.c urlcode.c dict.c sqlite.c tween.c ease.c sprite.c matrix.c myregex.c ttf.c files.c httploader.c ipstring.c mystring.c base64.c -lz -lssl -lsqlite3 -lpthread -ldl -lcrypto -lm && ./a.out  
 gcc -Wall ttf.c && ./a.out  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2016-06-14
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <iconv.h>

#include "sprite.h"
#include "SDL_image.h"

#include "files.h"

int getUtf8size(char * utf8)
{
	unsigned char c = *utf8;
	if(c<0x80)
		return 1;
	if(c<0xe0)
		return 2;
	if(c<0xf0)
		return 3;
	if(c<0xf8)
		return 4;
	if(c<0xfe)
		return 5;
	if(c<0xff)
		return 6;
	return 1;
}
char * utf2u(char * utf,int * outlen)
{
	int len = getUtf8size(utf);
	char * out = (char*)malloc(len+1);
	printf("len:%d\n",len);
	memset(out,0,len+1);

	unsigned short int c;
	int i = 0;
	c = 0;
	switch(len)
	{
		case 1:
			c = *utf;
			out[i++]=c;
			break;
		case 2:
			c += ((utf[0] & 0x1f)<<6);
			c += (utf[1] & 0x3f);
			out[i++]= *((char*)&c);
			out[i++]=c & 0xff;
			break;
		case 3:
			c += ((utf[0]<<12));
			c += ((utf[1] & 0x3f)<<6);
			c += ((utf[2] & 0x3f));
			out[i++]=(char)(c>>8) & 0xff;
			out[i++]=(char)c & 0xff;
			break;
		default:
			free(out);
			return NULL;
	}
	if(outlen)
		*outlen = i;
	return out;
}


#ifndef _ICONV_H
static short int getUtfQW(char * utf8,unsigned char * q,unsigned char *w)
{
	FILE * file = fopen(decodePath("~/sound/qw.txt"),"rb");
	fseek(file,0,SEEK_END);
	int flen = ftell(file);
	rewind(file);
	char * data = malloc(flen+1);
	data[flen]='\0';
	fread(data,flen,1,file);
	char * pos = strstr(data,utf8);

	*q = atoi(pos+strlen(utf8));
	*w = atoi(pos+strlen(utf8)+3);
	printf("q:%2x,w:%2x\n",*q,*w);
	short qw = 0x100*(*q) + *w + 0xa0a0;
	free(data);
	fclose(file);
	return qw;
}
#else

char * utf2gbk(char* inbuf,size_t * outlen){
	size_t inlen =strlen(inbuf);
	iconv_t cd=iconv_open("GBK","UTF-8");
	char*outbuf=(char*)malloc(inlen*4);
	memset(outbuf,0,inlen*4);
	char*in=inbuf;
	char*out=outbuf;
	*outlen=inlen*4;
	iconv(cd,&in,(size_t*)&inlen,&out,outlen);
	*outlen=strlen(outbuf);
	//printf("%s\n",outbuf);
	//free(outbuf);
	iconv_close(cd);
	return outbuf;
}
#endif

static GLuint textureId;

Sprite * container;
static void showBox(int x,int y,int w)
{
	Sprite*sprite = Sprite_new();
	char sname[] = "earth";
	sprite->name = malloc(sizeof(sname)+1);
	memset(sprite->name,0,sizeof(sname)+1);
	strcpy(sprite->name,sname);
	if(1)
	{
		sprite->surface = IMG_Load(decodePath("~/sound/1.bmp"));
	}else{
		if(textureId){
			sprite->textureId = textureId;
		}else{
			sprite->surface = IMG_Load(decodePath("~/sound/1.bmp"));
			textureId = Sprite_getTextureId(sprite);
			SDL_Log("textureId-------------:%d",textureId);
			sprite->textureId = textureId;
			Sprite_destroySurface(sprite);
			sprite->surface = NULL;
		}
	}
	Data3d*_data3D = sprite->data3d;
	if(_data3D==NULL){
		_data3D = (Data3d*)malloc(sizeof(Data3d));
		memset(_data3D,0,sizeof(Data3d));

		if(_data3D->programObject==0){
			Data3d *data2D = Data3D_init();
			Data3d_set(_data3D,data2D);
		}
		sprite->data3d = _data3D;
		//_data3D->numIndices = esGenSphere ( 20, 0.5f, &_data3D->vertices, &_data3D->normals, &_data3D->texCoords, &_data3D->indices);
		//_data3D->numIndices = esGenSphere ( 20, 0.5f, &_data3D->vertices, NULL, &_data3D->texCoords, &_data3D->indices);
		_data3D->numIndices = esGenCube( w*2.0/stage->stage_w, &_data3D->vertices, &_data3D->normals, &_data3D->texCoords, &_data3D->indices);
	}
	//sprite->filter = 1;
	sprite->x =  - 4*w;
	sprite->y =  - 4*w;
	sprite->scaleZ = 4.0;
	//sprite->alpha = 0.9;
	//sprite->z = -100;
	//sprite->w = stage->stage_w - sprite->x*2;
	//sprite->h = stage->stage_h - sprite->y*2;
	Sprite*sprite2 = Sprite_new();
	sprite2->x = x;
	sprite2->y = y;
	Sprite_addChildAt(container,sprite2,0);
	Sprite_addChild(sprite2,sprite);
}




static void mousehandl(SpriteEvent*e)
{
	Sprite * sprite = (Sprite *)e->target;
	SDL_Event * event = (SDL_Event*)e->e;
	switch(e->type){
		case SDL_MOUSEMOTION:
			//if(e->target->parent) Sprite_addChildAt(e->target->parent,e->target,0);
			if(event->motion.state){
				sprite->rotationX += event->motion.yrel;
				sprite->rotationY += event->motion.xrel;
				Stage_redraw();
			}
			break;
	}
}
int main(int argc,char** argv)
{
	Stage_init();
	container = Sprite_new();
	Sprite_addChild(stage->sprite,container);
	container->mouseChildren = 0;
	container->x = stage->stage_w/2;
	container->y = stage->stage_h/2;
	Sprite_addEventListener(container,SDL_MOUSEMOTION,mousehandl);
	Sprite_addEventListener(container,SDL_MOUSEBUTTONDOWN,mousehandl);
	Sprite_addEventListener(container,SDL_MOUSEBUTTONUP,mousehandl);


	char * s = "字";
	FILE* fphzk = NULL;
	int i, j, k, offset,index=0;
	int flag;
	unsigned char buffer[32];
	unsigned char qu=1 ,wei=1;
	size_t outlen=2;
#ifdef _ICONV_H
	char * word = utf2gbk(s,&outlen);
	printf("outlen:%d\n",(int)outlen);
#else
	getUtfQW(s,&qu,&wei);
	printf("qw:%x,%x\n",qu,wei);
#endif
	while(index<outlen-1){
#ifdef _ICONV_H
		qu = word[index++] - 0xa0;
		wei = word[index++] - 0xa0;
#endif
		printf("q:%2x,w:%2x\n",qu,wei);
		unsigned char key[8] = {
			0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01
		};
		fphzk = fopen(decodePath("~/sound/hzk1216/HZK16"), "rb");
		if(fphzk == NULL){
			fprintf(stderr, "error hzk16\n");
			return 1;
		}
		offset = (94*(unsigned int)(qu-1)+(wei-1))*32;
		fseek(fphzk, offset, SEEK_SET);
		fread(buffer, 1, 32, fphzk);
		for(k=0; k<32; k++){
			printf("%02X ", buffer[k]);
		}
		printf("\n");
		for(k=0; k<16; k++){
			for(j=0; j<2; j++){
				for(i=0; i<8; i++){
					flag = buffer[k*2+j]&key[i];
					printf("%s", flag?"██":"  ");
					if(flag)
					{
						GL_CHECK(gles2.glEnable(GL_DEPTH_TEST));
						showBox((i+j*8)*10*stage->stage_w/240.0,(k)*10*stage->stage_h/320.0,10*stage->stage_w/240.0);
						//GL_CHECK(gles2.glDisable ( GL_DEPTH_TEST));
					}
				}
			}
			printf("\n");
		}
#ifndef _ICONV_H
		break;
#endif
	}
	fclose(fphzk);
	fphzk = NULL;
	Stage_loopEvents();
	return 0;
}

