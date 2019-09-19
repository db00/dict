/**
 *
 gcc -Wall testffplay.c -I"../SDL2/include/" -lSDL2 -I"/usr/include/ffmpeg/" -lm -lavformat -lavcodec -lavutil -lswscale -lswresample && ./a.out a.flv

 gcc -Wall testffplay.c -I"../SDL2/include/" -I"/home/libiao/src/android-ndk-r10/sources/ffmpeg-2.8.6/android/arm/include/" -lm -lavformat -lavcodec -lavutil -lswscale -lswresample -lSDL2 && ./a.out a.flv
 gcc ffmpegplayger.c -I"../SDL2/include/" -I"include/" -L"../ffmpeg/lib/" -lmingw32 -lSDL2main -lSDL2  -lavformat -lavcodec  -lavutil -lswscale && a
 * 最简单的基于FFmpeg的视频播放器 2
 * Simplest FFmpeg Player 2
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 第2版使用SDL2.0取代了第一版中的SDL1.2
 * Version 2 use SDL 2.0 instead of SDL 1.2 in version 1.
 *
 * 本程序实现了视频文件的解码和显示(支持HEVC，H.264，MPEG2等)。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * This software is a simplest video player based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 *
 */
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <SDL.h>
#ifdef __cplusplus
};
#endif

int main(int argc, char* argv[])
{
	AVFormatContext	*ic;
	int				i, videoindex;
	AVCodecContext	*avctx;
	int got_picture;

	char * filepath = NULL;
	if(argc>1) filepath = argv[1];
	if(filepath==NULL)
	{
#ifdef __ANDROID__
		filepath="/sdcard/a.flv";
#else
		filepath = "a.flv";
#endif
	}
	//SDL---------------------------
	int screen_w=0,screen_h=0;


	av_register_all();
	avformat_network_init();
	ic = avformat_alloc_context();

	if(avformat_open_input(&ic,filepath,NULL,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return -1;
	}
	if(avformat_find_stream_info(ic,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}
	videoindex=-1;
	for(i=0; i<ic->nb_streams; i++) {
		if(ic->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	}
	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}
	avctx=ic->streams[videoindex]->codec;
	AVCodec	*codec;
	codec = avcodec_find_decoder(avctx->codec_id);
	if(codec==NULL){
		printf("Codec not found.\n");
		return -1;
	}
	if(avcodec_open2(avctx, codec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}

	AVFrame	*src_frame,*pict;
	src_frame=av_frame_alloc();
	pict=av_frame_alloc();
	uint8_t *out_buffer;
	out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, avctx->width, avctx->height));
	avpicture_fill((AVPicture *)pict, out_buffer, PIX_FMT_YUV420P, avctx->width, avctx->height);
	screen_w = avctx->width;
	screen_h = avctx->height;


	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 
	SDL_Window *window; 
	SDL_Renderer* renderer;
	window = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			screen_w, screen_h,
			SDL_WINDOW_OPENGL);
	renderer = SDL_CreateRenderer(window, -1, 0);  

	AVPacket *packet;
	packet=(AVPacket *)av_malloc(sizeof(AVPacket));
	while(av_read_frame(ic, packet)>=0){
		if(packet->stream_index==videoindex){
			avcodec_decode_video2(avctx, src_frame, &got_picture, packet);
			if(got_picture){
				SDL_Texture * bmp;
				AVFrame * frame = src_frame;
				bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,frame->width,frame->height);  
				SDL_UpdateYUVTexture(bmp, NULL,
						frame->data[0], frame->linesize[0],
						frame->data[1], frame->linesize[1],
						frame->data[2], frame->linesize[2]);

				SDL_RenderCopy( renderer, bmp,  NULL, NULL);  
				SDL_DestroyTexture(bmp);
				SDL_RenderPresent( renderer );  
				SDL_Delay(4);
			}
		}
		av_free_packet(packet);
	}
	exit(0);
	return 0;
}
