/*
 *
 gcc -D test_video -g -Wall  -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/" video.c array.c tween.c ease.c base64.c ipstring.c sprite.c httploader.c mystring.c textfield.c files.c matrix.c -lSDL2_image -lssl -lcrypto -lSDL2_ttf -LGLESv2 -lm -lSDL2 -D debug_video  -I"/usr/include/ffmpeg/" -lm -lavformat -lavcodec -lavutil -lswscale -lswresample -lavdevice && ./a.out
 gcc -D test_video -g -Wall  -I"../SDL2/include/" -I"../SDL2_ttf/" -I"../SDL2_image/" video.c array.c tween.c ease.c base64.c ipstring.c sprite.c httploader.c mystring.c textfield.c files.c matrix.c -lSDL2_image -lssl -lcrypto -lSDL2_ttf -LGLESv2 -lm -lSDL2 -D debug_video -L"/usr/local/lib/" -I"/home/libiao/src/android-ndk-r10/sources/ffmpeg/android/arm/include/"  -lm -lavformat -lavcodec -lavutil -lswscale -lswresample -lavdevice -lz -lpthread -llzma && ./a.out
 */
#ifndef video_h
#define video_h

#ifdef __cplusplus
extern "C"
{
#endif
#include "sprite.h"
#include <SDL.h>
#include <signal.h>
//#ifdef CONFIG_AVDEVICE
#include "libavdevice/avdevice.h"
//#endif
#include "libavutil/avstring.h"
#include "libavutil/time.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#ifdef __cplusplus
};
#endif
#define MIN_FRAMES 25
/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0
/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10
/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20
/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, VIDEO_PICTURE_QUEUE_SIZE)

/* Step size for volume control */
#define SDL_VOLUME_STEP (SDL_MIX_MAXVOLUME / 50)

typedef struct PacketList 
{
	AVPacket pkt;
	struct PacketList *next;
	int index;
} PacketList;

typedef struct PacketQueue 
{
	PacketList *first_pkt, *last_pkt;
	int numPackets;
	int size;
	int abort_request;
	int serialno;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;

typedef struct AudioParams 
{
	int freq;
	int channels;
	int64_t channel_layout;
	enum AVSampleFormat fmt;
	int frame_size;
	int bytes_per_sec;
} AudioParams;

typedef struct Clock 
{
	double pts;           /* clock base */
	double pts_drift;     /* clock base minus time at which we updated the clock */
	double last_updated_time;
	int packet_index;           /* clock is based on a packet with this serial */
	int paused;
	int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
} Clock;


typedef struct UserData
{
	GLuint programObject;
	GLint mvpLoc;

	GLint positionLoc;
	GLint textureLoc;
	GLint yloc;
	GLint uloc;
	GLint vloc;

	GLuint textyId;
	GLuint textuId;
	GLuint textvId;
} UserData;


/* Common struct for handling all types of decoded data and allocated render buffers. */
typedef struct Frame
{
	AVFrame *avframe;
	int frameIndex;
	double pts;           /* presentation timestamp for the frame */
	double duration;      /* estimated duration of the frame */
	int64_t pos;          /* byte position of the frame in the input file */
	SDL_Texture *bmp;
	SDL_Surface * surface;
	int width;
	int height;
} Frame;

typedef struct FrameQueue 
{
	Frame queue[FRAME_QUEUE_SIZE];
	int rindex;//read index
	int windex;//write index
	int size;
	int max_size;
	int keep_last;
	int rindex_shown;
	SDL_mutex *mutex;
	SDL_cond *cond;
	PacketQueue *pktq;
} FrameQueue;

typedef struct Decoder 
{
	AVPacket pkt;
	AVPacket pkt_temp;
	PacketQueue *queue;
	AVCodecContext *avctx;
	int pkt_serial;
	int finished;
	int packet_pending;
	int64_t start_pts;
	AVRational start_pts_tb;
	int64_t next_pts;
	AVRational next_pts_tb;
	SDL_Thread *decoder_tid;
} Decoder;

typedef struct Video
{
	char filename[1024];
	Sprite * sprite;
	SDL_TimerID timerId;
	int seek_by_bytes;// = -1;
	AVDictionary *codec_opts;
	int w, h;

	int audio_volume;
	int muted;

	SDL_Thread *read_tid;
	AVInputFormat *iformat;
	int abort_request;
	int force_refresh;
	int paused;
	int last_paused;
	int queue_attachments_req;
	int seek_req;
	int seek_flags;
	int64_t seek_pos;
	int64_t seek_rel;
	int read_pause_return;
	AVFormatContext * avformatctx;

	Clock audclk;
	Clock vidclk;
	Clock extclk;

	FrameQueue pictq;
	FrameQueue sampq;

	Decoder auddec;
	Decoder viddec;

	int audio_stream;
	double audio_clock;
	int audio_clock_serial;
	double audio_diff_cum; /* used for AV difference average computation */
	double audio_diff_avg_coef;
	double audio_diff_threshold;
	int audio_diff_avg_count;
	AVStream *audio_st;
	PacketQueue audioq;
	int audio_hw_buf_size;
	uint8_t silence_buf[SDL_AUDIO_MIN_BUFFER_SIZE];
	uint8_t *audio_buf;
	uint8_t *audio_buf1;
	unsigned int audio_buf_size; /* in bytes */
	unsigned int audio_buf1_size;
	int audio_buf_index; /* in bytes */
	int audio_write_buf_size;
	struct AudioParams audio_src;
	struct AudioParams audio_tgt;
	struct SwrContext *swr_ctx;

	enum ShowMode {
		SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_RDFT 
	} show_mode;
	double last_vis_time;

	double frame_timer;
	int video_stream;
	AVStream *video_st;
	PacketQueue videoq;
	double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
	int eof;

	int step;
	int last_video_stream, last_audio_stream;

	SDL_cond *continue_read_thread;
} Video;


Video*Video_new(const char *filename,const char *ifmt);
void Video_clear(Video*video);
void Video_pause(Video * video);
void Video_nextFrame(Video*video);
void Video_seek(Video*video, int64_t pos, int64_t rel, int seek_by_bytes);

#endif
