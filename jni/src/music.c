/**
 *
 gcc -Wall -D debug_music music.c httploader.c files.c array.c myregex.c mystring.c base64.c ipstring.c urlcode.c -I"../SDL2_mixer/" -I"../SDL2/include/" -lpthread -lSDL2 -lSDL2_mixer -lssl -lcrypto && ./a.out ~/sound/pinyin/bang3.ogg
 */
#include "music.h"

#if 0
static void print_init_flags(int flags)
{
#define PFLAG(a) if(flags&MIX_INIT_##a) printf(#a " ")
	PFLAG(FLAC);
	PFLAG(MOD);
	PFLAG(MP3);
	PFLAG(OGG);
	if(!flags)
		SDL_Log("None");
	SDL_Log("\n");
}
#endif

Sound * Sound_new(int audio_rate)
{
	Sound * sound = malloc(sizeof(Sound));
	memset(sound,0,sizeof(Sound));
	sound->volume = SDL_MIX_MAXVOLUME;
#ifdef __ANDROID__
	sound->audio_rate = audio_rate;
#else
	sound->audio_rate = 44100;
#endif

	//sound->initted=Mix_Init(0);
	//SDL_Log("Before Mix_Init SDL_mixer supported: ");
	//print_init_flags(sound->initted);
	sound->initted=Mix_Init(~0);
	//SDL_Log("After  Mix_Init SDL_mixer supported: ");
	//print_init_flags(sound->initted);
	while(Mix_Init(0))
		Mix_Quit();

	/*
	   while(Mix_PlayingMusic())
	   SDL_Delay(1500);
	   Mix_CloseAudio();
	   SDL_Delay(100);
	   */
	//if(Mix_OpenAudio(16000,MIX_DEFAULT_FORMAT,2,1024)<0)
	//if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,1024)<0)
	if(Mix_OpenAudio(sound->audio_rate,MIX_DEFAULT_FORMAT,2,4096)<0)
	{
		SDL_Log("Error initializing SDL_mixer: %s\n", Mix_GetError());
		Sound_free(sound);
		return NULL;
	}
	/* we play no samples, so deallocate the default 8 channels... */
	SDL_Log("number of channels :%d",Mix_AllocateChannels(0));
#if 0
	/* print out some info on the formats this run of SDL_mixer supports */
	{
		int i,n=Mix_GetNumChunkDecoders();
		SDL_Log("There are %d available chunk(sample) decoders:\n",n);
		for(i=0; i<n; ++i)
			printf("	%s", Mix_GetChunkDecoder(i));
		SDL_Log("\n");
		n = Mix_GetNumMusicDecoders();
		SDL_Log("There are %d available music decoders:\n",n);
		for(i=0; i<n; ++i)
			printf("	%s", Mix_GetMusicDecoder(i));
		SDL_Log("\n");
	}
#endif

	/* print out some info on the audio device and stream */
	int numtimesopened = Mix_QuerySpec(&sound->audio_rate, &sound->audio_format, &sound->audio_channels);
	//sample_size=bits/8+audio_channels;
	if(numtimesopened) {
		SDL_Log("Opened audio %d times at %d Hz %d bit %s, %d bytes audio buffer\n",numtimesopened, sound->audio_rate,
				sound->audio_format&0xff, sound->audio_channels>1?"stereo":"mono", 1024);
	}else{
		SDL_Log("Mix_QuerySpec: %s\n",Mix_GetError());
	}

	return sound;
}
int Sound_playUrl(Sound*sound,char * url)
{
	SDL_Log("Sound_playUrl:%s",url);
	size_t data_length=0;
	char * data = loadUrl(url,&data_length);
	if(data){
		int i = Sound_playData(sound,data,data_length);
		free(data);
		return i;
	}
	return 1;
}

int playSound(Sound * sound)
{
	if(sound==NULL)
		return 1;
	if(sound->music)
	{
		Mix_MusicType type=Mix_GetMusicType(sound->music);
		SDL_Log("Music type: %s\n",
				type==MUS_NONE?"MUS_NONE":
				type==MUS_CMD?"MUS_CMD":
				type==MUS_WAV?"MUS_WAV":
				//type==MUS_MOD_MODPLUG?"MUS_MOD_MODPLUG":
				type==MUS_MOD?"MUS_MOD":
				type==MUS_MID?"MUS_MID":
				type==MUS_OGG?"MUS_OGG":
				type==MUS_MP3?"MUS_MP3":
				type==MUS_FLAC?"MUS_FLAC":
				"Unknown");
	}
	/* wait for escape key of the quit event to finish */
	if(Mix_PlayMusic(sound->music, 1)==-1)
	{
		SDL_Log("Mix_PlayMusic: %s\n",Mix_GetError());
		Sound_free(sound);
		return 2;
	}
	if(Mix_VolumeMusic(sound->volume)<0)
	{
		SDL_Log("Mix_VolumeMusic: %s\n",Mix_GetError());
		Sound_free(sound);
		return 3;
	}
	while((Mix_PlayingMusic() || Mix_PausedMusic()))
	{
		SDL_Delay(100);
		//SDL_Log("--------------------");
	}
	//Sound_free(sound);
	return 0;
}



int Sound_playFile(Sound*sound,char * file)
{
	SDL_Log("Sound_playFile :%s",file);

	Sound * _sound = sound;

	if(sound==NULL){
		sound = Sound_new(16000);
	}
	if(sound==NULL)
		return 2;

	char * f= decodePath(file);

	//if(sound->music) Mix_FreeMusic(sound->music);
	sound->music = Mix_LoadMUS(f);
	//sound->music = Mix_LoadMUS_RW(SDL_RWFromFile(f, "rb"), SDL_TRUE);

	if(sound->music)
	{
		int i = playSound(sound);
		if(i==0 && _sound==NULL)//play successfully,
			Sound_free(sound);
		return i;
	}else{
		SDL_Log("Mix_LoadMUS: %s\n",Mix_GetError());
		Sound_free(sound);
		return 3;
	}

	/*
	   size_t data_length=0;
	   char * data = readfile(file,&data_length);
	   if(data){
	   int i = Sound_playData(sound,data,data_length);
	   free(data);
	   return i;
	   }
	   */
	return 0;
}

int Sound_playData(Sound * sound,char * data,size_t data_length)
{
#ifdef __x86_64__
	SDL_Log("Sound_playData:0x%llx,%d",(unsigned long long int)data,(int)data_length);
#elif __i386__
	SDL_Log("Sound_playData:0x%llx,%d",(unsigned int)data,(int)data_length);
#endif
	if(data==NULL || data_length==0)
		return 1;
	if(sound==NULL){
		sound = Sound_new(16000);
	}
	if(sound==NULL)
		return 2;

	//if(sound->music) Mix_FreeMusic(sound->music);
	/* load the song */
	sound->music = Mix_LoadMUS_RW(SDL_RWFromConstMem(data,data_length), SDL_TRUE);
	//sound->music=Mix_LoadMUS(file);

	if(sound->music){
		return playSound(sound);
	}else{
		SDL_Log("Mix_LoadMUS_RW : %s\n",Mix_GetError());
		Sound_free(sound);
		return 3;
	}

	return 0;
}



void Sound_rewind(Sound * sound,int mod)
{
	if(mod)
	{
		Mix_RewindMusic();
		sound->position=0;
	}
	else
	{
		int pos=sound->position/sound->audio_rate-1;
		if(pos<0)
			pos=0;
		Mix_SetMusicPosition(pos);
		sound->position=pos*sound->audio_rate;
	}
}

void fast_forward(Sound * sound)
{
	switch(Mix_GetMusicType(NULL))
	{
		case MUS_MP3:
			Mix_SetMusicPosition(+5);
			sound->position+=5* sound->audio_rate;
			break;
		case MUS_OGG:
		case MUS_FLAC:
			/*case MUS_MOD_MODPLUG:*/
			Mix_SetMusicPosition(sound->position/sound->audio_rate+1);
			sound->position+=sound->audio_rate;
			break;
		default:
			SDL_Log("cannot fast-forward this type of music\n");
			break;
	}
}
void Sound_pause()
{
	if(Mix_PausedMusic())
		Mix_ResumeMusic();
	else
		Mix_PauseMusic();
}
void volumeDown(Sound*sound)
{
	sound->volume>>=1;
	Mix_VolumeMusic(sound->volume);
}
void volumeUp(Sound*sound)
{
	sound->volume=(sound->volume+1)<<1;
	if(sound->volume>SDL_MIX_MAXVOLUME)
		sound->volume=SDL_MIX_MAXVOLUME;
	Mix_VolumeMusic(sound->volume);
}

void Sound_free(Sound *sound)
{
	//SDL_Log("Sound_free: ");
	if( Mix_PlayingMusic() ) {
		if(!Mix_FadeOutMusic(1500))
			SDL_Log("Mix_FadeOutMusic: %s\n",Mix_GetError());
		SDL_Delay(1500);
	}
	if(sound){
		if(sound->music)
			Mix_FreeMusic(sound->music);
		free(sound);
	}
	Mix_CloseAudio();
	//SDL_Log("successfully!");
}

#ifdef debug_music
int main(int argc,char**argv)
{
	/*
	   ffmpeg -i /home/db0/sound/pinyin/bei4.mp3 /home/db0/sound/pinyin/bei4.ogg
	   */

	//if(argc>1) Sound_playFile(NULL,argv[1]); else
	Sound_playFile(NULL,"~/sound/uk/fuck.mp3");
	Sound_playFile(NULL,"~/sound/pinyin/cao4.ogg");
	int i=0x12345678;
	char * p =  (char*)(&i);
	printf("%x",*(p+3));
	printf("%x",*(p+2));
	printf("%x",*(p+1));
	printf("%x",*(p+0));
	return 0;
}
#endif
