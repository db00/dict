/**
 * 最简单的基于FFmpeg的AVDevice例子（读取摄像头）
 gcc camera.c -I"../SDL2/include" -I"/usr/include/ffmpeg/" -L"/usr/lib/" -lm -lpthread -lSDL2 -lavdevice -lavcodec -lavformat -lavutil -lswscale && ./a.out
 gcc camera.c sprite.c array.c ease.c matrix.c tween.c -L"lib" -lavdevice -lavcodec -lavformat -lavutil -lswscale -lswresample -D test_video -I"../SDL2/include" -I"include" -lpthread -lmingw32 -lSDL2main -lSDL2 && a
 gcc camera.c -I"../SDL2/include" -L"/usr/local/lib/" -lm -lpthread -lSDL2 -lavdevice -lavcodec -lavformat -lavutil -lswscale && ./a.out
 * Simplest FFmpeg Device (Read Camera)
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序实现了本地摄像头数据的获取解码和显示。是基于FFmpeg
 * 的libavdevice类库最简单的例子。通过该例子，可以学习FFmpeg中
 * libavdevice类库的使用方法。
 * 本程序在Windows下可以使用2种方式读取摄像头数据：
 *  1.VFW: Video for Windows 屏幕捕捉设备。注意输入URL是设备的序号，
 *          从0至9。
 *  2.dshow: 使用Directshow。注意作者机器上的摄像头设备名称是
 *         “Integrated Camera”，使用的时候需要改成自己电脑上摄像头设
 *          备的名称。
 * 在Linux下则可以使用video4linux2读取摄像头设备。
 *
 * This software read data from Computer's Camera and play it.
 * It's the simplest example about usage of FFmpeg's libavdevice Library. 
 * It's suiltable for the beginner of FFmpeg.
 * This software support 2 methods to read camera in Microsoft Windows:
 *  1.gdigrab: VfW (Video for Windows) capture input device.
 *             The filename passed as input is the capture driver number,
 *             ranging from 0 to 9.
 *  2.dshow: Use Directshow. Camera's name in author's computer is 
 *             "Integrated Camera".
 * It use video4linux2 to read Camera in Linux.
 * 
 */


#include <stdio.h>
#include <signal.h>

#ifdef cpp
extern "C"
{
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
	//#include "libavutil/pixfmt.h"
	//SDL
#include "SDL.h"
#include "SDL_thread.h"
#ifdef cpp
};
#endif


//'1' Use Dshow 
//'0' Use VFW
#define USE_DSHOW 0


//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

int thread_exit=0;

int sfp_refresh_thread(void *opaque)
{
	while (thread_exit==0) {
		SDL_Event event;
		event.type = SFM_REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	return 0;
}


//Show Device
void show_dshow_device(){
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options,"list_devices","true",0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	printf("Device Info=============\n");
	avformat_open_input(&pFormatCtx,"video=dummy",iformat,&options);
	printf("========================\n");
}

//Show Device Option
void show_dshow_device_option(){
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options,"list_options","true",0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	printf("Device Option Info======\n");
	avformat_open_input(&pFormatCtx,"video=Integrated Camera",iformat,&options);
	printf("========================\n");
}

//Show VFW Device
void show_vfw_device(){
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVInputFormat *iformat = av_find_input_format("vfwcap");
	printf("VFW Device Info======\n");
	avformat_open_input(&pFormatCtx,"list",iformat,NULL);
	printf("=====================\n");
}


static void sigterm_handler(int sig)
{
	exit(123);
}

int main(int argc, char* argv[])
{
	signal(SIGINT , sigterm_handler); /* Interrupt (ANSI).    */
	signal(SIGTERM, sigterm_handler); /* Termination (ANSI).  */

	int ret, got_picture,align = 1;
	AVFormatContext *pFormatCtx;
	int             i, videoindex;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;

	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	//Register Device
	avdevice_register_all();
	int r=1;
	AVDictionary* options = NULL;  
	AVInputFormat *ifmt=NULL;

	//Windows
#ifdef _WIN32

	//Show Dshow Device
	show_dshow_device();
	//Show Device Options
	show_dshow_device_option();
	//Show VFW Options
	show_vfw_device();


	ifmt=av_find_input_format("vfwcap");
	r = avformat_open_input(&pFormatCtx,"0",ifmt,NULL);

	if(r!=0){
		printf("Couldn't open input stream.（无法打开输入流）\n");
		ifmt=av_find_input_format("dshow");
		r = avformat_open_input(&pFormatCtx,"video=Integrated Camera",ifmt,NULL);
	}
#endif
	//Linux
#ifdef linux
	/*
	 * //must run as root
	 AVInputFormat *ifmt=av_find_input_format("fbdev");
	 if(avformat_open_input(&pFormatCtx,"/dev/fb0",ifmt,&options)!=0){  
	 printf("Couldn't open input stream.（无法打开输入流）\n");  
	 return -1;  
	 }  
	 */
	//摄像头
	ifmt = av_find_input_format("video4linux2");
	r = avformat_open_input(&pFormatCtx,"/dev/video0",ifmt,NULL);
	if(r!=0){
		printf("Couldn't open input stream.（无法打开输入流）\n");
		//录制屏幕
		ifmt=av_find_input_format("x11grab");
		AVDictionary* options = NULL;  
		//grabbing frame rate  
		//av_dict_set(&options,"framerate","5",0);  
		//Make the grabbed area follow the mouse  
		//av_dict_set(&options,"follow_mouse","centered",0);  
		//Video frame size. The default is to capture the full screen  
		//av_dict_set(&options,"video_size","1024x768",0);
		//r = avformat_open_input(&pFormatCtx,":0.0+10,20",ifmt,&options);
		r = avformat_open_input(&pFormatCtx,":0.0",ifmt,&options);
	}
#endif
	if(r!=0){
		printf("Couldn't open input stream.（无法打开输入流）\n");
		return -1;
	}


	if(avformat_find_stream_info(pFormatCtx,NULL)<0)
	{
		printf("Couldn't find stream information.（无法获取流信息）\n");
		return -1;
	}
	videoindex=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++) 
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
		{
			videoindex=i;
			break;
		}
	if(videoindex==-1)
	{
		printf("Couldn't find a video stream.（没有找到视频流）\n");
		return -1;
	}
	pCodecCtx=pFormatCtx->streams[videoindex]->codec;
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL)
	{
		printf("Codec not found.（没有找到解码器）\n");
		return -1;
	}
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
	{
		printf("Could not open codec.（无法打开解码器）\n");
		return -1;
	}
	AVFrame *pFrame,*pFrameYUV;
	pFrame= av_frame_alloc();
	pFrameYUV= av_frame_alloc();
	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height,align);
	//uint8_t *out_buffer=(uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height));
	uint8_t *out_buffer=(uint8_t *)av_malloc(numBytes);
	//avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	av_image_fill_arrays(pFrameYUV->data,pFrameYUV->linesize,out_buffer,AV_PIX_FMT_RGB24,pCodecCtx->width, pCodecCtx->height,align);

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 
	int screen_w,screen_h;
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;

	SDL_Window * window = SDL_CreateWindow("Camera", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			screen_w, screen_h,
			0);
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Surface *bmp = SDL_CreateRGBSurface(0, screen_w, screen_h, 24,
#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			0xff000000, /* Red bit mask. */
			0x00ff0000, /* Green bit mask. */
			0x0000ff00, /* Blue bit mask. */
			0x000000ff  /* Alpha bit mask. */
#else
			0x000000ff, /* Red bit mask. */
			0x0000ff00, /* Green bit mask. */
			0x00ff0000, /* Blue bit mask. */
			0xff000000  /* Alpha bit mask. */
#endif
			);
	SDL_Texture * texture = NULL;


	/**
	 *
	 int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pFrame->width, pFrame->height,align);
	 uint8_t * buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	//SDL_Log("%d----------------%dx%d",numBytes,pFrame->width,pFrame->height);
	AVCodecContext	*pCodecCtx = video->viddec.avctx;
	//avpicture_fill((AVPicture*)pFrameRGB,buffer,AV_PIX_FMT_RGB24,pFrame->width, pFrame->height);
	av_image_fill_arrays(pFrameRGB->data,pFrameRGB->linesize,buffer,AV_PIX_FMT_RGB24,pFrame->width, pFrame->height,align);
	*/
	AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Information-----------------------------
	printf("File Information（文件信息）---------------------\n");
	av_dump_format(pFormatCtx,0,NULL,0);
	printf("-------------------------------------------------\n");

	struct SwsContext *img_convert_ctx;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
			pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, 
			SWS_BICUBIC, NULL, NULL, NULL); 
	//------------------------------
	SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread,NULL,NULL);
	//Event Loop
	SDL_Event event;

	for (;;) {
		//Wait
		SDL_WaitEvent(&event);
		if(event.type==SFM_REFRESH_EVENT){
			//------------------------------
			if(av_read_frame(pFormatCtx, packet)>=0){
				if(packet->stream_index==videoindex){
					ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
					if(ret < 0){
						printf("Decode Error.（解码错误）\n");
						return -1;
					}
					if(got_picture){
						sws_scale(img_convert_ctx, 
								(const uint8_t* const*)pFrame->data, pFrame->linesize, 
								0, pCodecCtx->height,
								pFrameYUV->data, pFrameYUV->linesize);
						SDL_LockSurface(bmp);
						SDL_UnlockSurface(bmp);
						bmp->pixels = pFrameYUV->data[0];
						SDL_RenderClear(renderer);
						if(texture)
							SDL_DestroyTexture(texture);
						texture = SDL_CreateTextureFromSurface(renderer,bmp);
						SDL_RenderCopy(renderer, texture, NULL, NULL);
						SDL_RenderPresent(renderer);

					}
				}
				//av_free_packet(packet);
				av_packet_unref(packet);
			}else{
				//Exit Thread
				thread_exit=1;
				break;
			}
		}else if(event.type==SDL_QUIT){
			thread_exit=1;
			break;
		}

	}


	sws_freeContext(img_convert_ctx);

	SDL_Quit();

	av_free(out_buffer);
	av_free(pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}
