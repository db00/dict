/**
 *
 gcc -Wall ffmpegplayger.c -I"../SDL2/include/" -I"/usr/include/ffmpeg/" -lm -lavformat -lavcodec -lavutil -lswscale -lswresample -lSDL2 && ./a.out http://live.cgtn.com/500d/prog_index.m3u8 a.flv
 gcc -Wall ffmpegplayger.c -I"../SDL2/include/" -I"/home/libiao/src/android-ndk-r10/sources/ffmpeg-2.8.6/android/arm/include/" -lm -lavformat -lavcodec -lavutil -lswscale -lswresample -lSDL2 && ./a.out a.flv
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

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "SDL.h"
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <SDL.h>
#ifdef __cplusplus
};
#endif
#endif

//Output YUV420P data as a file 
#define OUTPUT_YUV420P 0

int main(int argc, char* argv[])
{
	AVFormatContext	*ic;
	int				i, videoindex;
	AVCodecContext	*avctx;
	AVFrame	*src_frame,*pict;
	AVPacket *packet;
#if OUTPUT_YUV420P
	int y_size;
#endif
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;

	//char filepath[]="bigbuckbunny_480x272.h265";
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
	SDL_Window *screen; 
	SDL_Renderer* renderer;
	SDL_Texture* bmp;
	SDL_Rect sdlRect;


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
	for(i=0; i<ic->nb_streams; i++) 
		if(ic->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}
	avctx=ic->streams[videoindex]->codec;
	AVCodec			*codec;
	codec=avcodec_find_decoder(avctx->codec_id);
	if(codec==NULL){
		printf("Codec not found.\n");
		return -1;
	}
	if(avcodec_open2(avctx, codec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}

	src_frame=av_frame_alloc();
	pict=av_frame_alloc();
	uint8_t *out_buffer;
	out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, avctx->width, avctx->height));
	avpicture_fill((AVPicture *)pict, out_buffer, PIX_FMT_YUV420P, avctx->width, avctx->height);
	packet=(AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	//av_dump_format(ic,0,filepath,0);
	printf("-------------------------------------------------\n");
	img_convert_ctx = sws_getContext(avctx->width, avctx->height, avctx->pix_fmt, 
			avctx->width, avctx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 

#if OUTPUT_YUV420P 
	FILE *fp_yuv;
	fp_yuv=fopen("output.yuv","wb+");  
#endif  

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 

	screen_w = avctx->width;
	screen_h = avctx->height;
	//SDL 2.0 Support for multiple windows
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			screen_w, screen_h,
			SDL_WINDOW_OPENGL);

	if(!screen) {  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}

	renderer = SDL_CreateRenderer(screen, -1, 0);  
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,avctx->width,avctx->height);  

	sdlRect.x=0;
	sdlRect.y=0;
	sdlRect.w=screen_w;
	sdlRect.h=screen_h;

	//SDL End----------------------
	while(av_read_frame(ic, packet)>=0){
		if(packet->stream_index==videoindex){
			ret = avcodec_decode_video2(avctx, src_frame, &got_picture, packet);
			if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
			if(got_picture){
				//sws_scale(img_convert_ctx, (const uint8_t* const*)src_frame->data, src_frame->linesize, 0, avctx->height, pict->data, pict->linesize);

#if OUTPUT_YUV420P
				y_size=avctx->width*avctx->height;  
				fwrite(pict->data[0],1,y_size,fp_yuv);    //Y 
				fwrite(pict->data[1],1,y_size/4,fp_yuv);  //U
				fwrite(pict->data[2],1,y_size/4,fp_yuv);  //V
#endif
				if(1){
					//SDL---------------------------
#if 0
					SDL_UpdateTexture( bmp, NULL, pict->data[0], pict->linesize[0] );  
#else
					//SDL_UpdateYUVTexture(bmp, &sdlRect,\
							pict->data[0], pict->linesize[0],\
							pict->data[1], pict->linesize[1],\
							pict->data[2], pict->linesize[2]);
					SDL_UpdateYUVTexture(bmp, NULL,
							src_frame->data[0], src_frame->linesize[0],
							src_frame->data[1], src_frame->linesize[1],
							src_frame->data[2], src_frame->linesize[2]);
#endif	

					SDL_RenderClear( renderer );  
					SDL_RenderCopy( renderer, bmp,  NULL, &sdlRect);  
					SDL_RenderPresent( renderer );  
					//SDL End-----------------------
					//Delay 40ms
				}
				SDL_Delay(40);
			}
		}
		av_free_packet(packet);
	}
	return 0;
	//flush decoder
	//FIX: Flush Frames remained in Codec
	while (1) {
		ret = avcodec_decode_video2(avctx, src_frame, &got_picture, packet);
		if (ret < 0)
			break;
		if (!got_picture)
			break;
		SDL_Log("-----------------remained--------------------------");
		sws_scale(img_convert_ctx, (const uint8_t* const*)src_frame->data, src_frame->linesize, 0, avctx->height, pict->data, pict->linesize);
#if OUTPUT_YUV420P
		int y_size=avctx->width*avctx->height;  
		fwrite(pict->data[0],1,y_size,fp_yuv);    //Y 
		fwrite(pict->data[1],1,y_size/4,fp_yuv);  //U
		fwrite(pict->data[2],1,y_size/4,fp_yuv);  //V
#endif
		//SDL---------------------------
		SDL_UpdateTexture( bmp, &sdlRect, pict->data[0], pict->linesize[0] );  
		SDL_RenderClear( renderer );  
		SDL_RenderCopy( renderer, bmp,  NULL, &sdlRect);  
		SDL_RenderPresent( renderer );  
		//SDL End-----------------------
		//Delay 40ms
		SDL_Delay(40);
	}

	sws_freeContext(img_convert_ctx);

#if OUTPUT_YUV420P 
	fclose(fp_yuv);
#endif 

	SDL_Quit();

	av_frame_free(&pict);
	av_frame_free(&src_frame);
	avcodec_close(avctx);
	avformat_close_input(&ic);

	return 0;
}
