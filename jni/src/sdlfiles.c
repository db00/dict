/**
 *
 gcc sdlfiles.c sprite.c array.c tween.c textfield.c ease.c httploader.c -lssl -lcrypto mystring.c ipstring.c base64.c matrix.c files.c -lm -lSDL2 -I"../SDL2_ttf" -lSDL2_ttf -I"../SDL2/include/" -I"../SDL2_image/" -D debug_sdlfiles -lSDL2_image && ./a.out
 gcc sdlfiles.c -I"../SDL2/include/" -lmingw32 -lSDL2main -lSDL2 -D debug_sdlfiles && a
 */
#include "sdlfiles.h"

char *sdlbasepath() 
{
#ifdef _SDL_H
	return SDL_GetBasePath();//当前目录
#endif
	char *cwd = malloc(0x100);
	memset(cwd,0,0x100);
	getcwd(cwd,0x100);
	return cwd;
}
char *sdlprefpath()
{
#ifdef _SDL_H
	return SDL_GetPrefPath("org.libsdl.dict","files");//应用程序数据目录,可读写
#endif
	return sdlbasepath();
}


long sdlfileSize(FILE*stream)
{
#ifdef _SDL_H
	SDL_RWops*file = SDL_RWFromFP(stream, SDL_TRUE);
	return SDL_RWsize(file);
#endif
	long curpos,length;
	curpos=ftell(stream);
	fseek(stream,0L,SEEK_END);
	length=ftell(stream);
	fseek(stream,curpos,SEEK_SET);
	return length;


}

char* sdlreadfile(char * path,size_t * _filesize)
{
#ifdef _SDL_H
	if(_filesize)*_filesize = 0;
	SDL_RWops* file = SDL_RWFromFile(path, "rb");
	if(file==NULL)
		return NULL;
	//Sint64 filesize = SDL_RWseek(file, 0, RW_SEEK_END);
	//Sint64 filesize = SDL_RWtell(struct SDL_RWops* context)
	Sint64 filesize = SDL_RWsize(file);
	char *buffer = (char*)malloc(filesize+1);
	memset(buffer,0,filesize+1);
	SDL_RWseek(file, 0, RW_SEEK_SET);
	SDL_RWread(file, buffer, 1,filesize);
	SDL_RWclose(file);
	if(_filesize)*_filesize = filesize;
	return buffer;
#else
	//if(*path=='/')
	{
		if(_filesize)*_filesize = 0;
		FILE * file = fopen(path,"rb");	
		if(file==NULL)
			return NULL;
		size_t filesize = fileSize(file);
		char *buffer = (char*)malloc(filesize+1);
		memset(buffer,0,filesize+1);
		rewind(file);
		int ret=fread(buffer,1,filesize,file);
		if(ret!=filesize){
			free(buffer);
			buffer = NULL;
		}
		fclose(file);
		if(_filesize)*_filesize = filesize;
		return buffer;
	}
#endif
}
int sdlwritefile(char * path,char *data,size_t data_length)
{
	int ret = 0;
#ifdef _SDL_H
	SDL_RWops* file = SDL_RWFromFile(path, "w+");
	if(file==NULL)
		return -3;
	ret = SDL_RWwrite(file, data, 1, data_length);
	if(ret!=data_length)
		ret = -4;
	SDL_RWclose(file);
#else
	//if(*path=='/')
	{
		FILE * file = fopen(path,"wb");	
		if(file==NULL)
			return -1;
		rewind(file);
		ret=fwrite(data,1,data_length,file);
		if(ret!=data_length){
			printf("write %s ERROR\n",path);
			ret = -2;
		}else{
			ret = 0;
		}
		fclose(file);
		return ret;
	}
#endif
	return ret;
}

#ifdef debug_sdlfiles
#include "SDL_image.h"
#include "sprite.h"
void saveIconFile(char *name,SDL_Surface*surface)
{
	char path[1024];
	memset(path,0,1024);
	sprintf(path,"../../res/%s/ic_launcher.png",name);
	SDL_Surface * _surface = NULL;
	if(strcmp(name,"drawable-mdpi")==0){//48
		_surface = Surface_new(48,48);
	}else if(strcmp(name,"drawable-hdpi")==0){//72
		_surface = Surface_new(72,72);
	}else if(strcmp(name,"drawable-xhdpi")==0){//96
		_surface = Surface_new(96,96);
	}else if(strcmp(name,"drawable-xxhdpi")==0){//144
		_surface = Surface_new(144,144);
	}
	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
	//SDL_BlitSurface(surface, surface->rect, _surface, NULL);
	//SDL_LowerBlit(surface, NULL, _surface, &_rect);
	SDL_BlitScaled(surface, NULL, _surface, NULL);
	IMG_SavePNG(_surface,path);
	SDL_FreeSurface(_surface);
}
void makeIcon(char *pic_path)
{
	if(pic_path==NULL)
		return;
	SDL_Surface *surface = IMG_Load(pic_path);
	if(surface == NULL)
		return;

	saveIconFile("drawable-mdpi",surface);
	saveIconFile("drawable-hdpi",surface);
	saveIconFile("drawable-xhdpi",surface);
	saveIconFile("drawable-xxhdpi",surface);
}
int main(int argc, char *argv[])
{
	makeIcon("/home/db0/SDL2apk/dict.png");
	char* filename="hello/hi/";
	//if( remove(filename) == 0 )
	if( unlink(filename) == 0 )
		printf("Removed %s.", filename);
	else{
		perror("remove");
	}

	printf(sdlreadfile("files.h",NULL));
	sdlwritefile("test","tet",strlen("tet"));
	//printf("creatdir:%d\n",creatdir("hello/hi"));
	return 0;
}
#endif
