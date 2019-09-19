/*
 *
 gcc -D test_video -g -Wall -I"../SDL2/include/" -I"/usr/include/ffmpeg/" video.c array.c tween.c ease.c sprite.c mystring.c matrix.c -LGLESv2 -lm -lSDL2 -D debug_video -L"/usr/local/lib/" -lm -lavformat -lavcodec -lavutil -lswscale -lswresample -lz -lpthread && ./a.out http://live.cgtn.com/500d/prog_index.m3u8

 gcc video.c sprite.c array.c ease.c matrix.c tween.c -L"lib" -lavdevice -lavcodec -lavformat -lavutil -lswscale -lswresample -D test_video -I"../SDL2/include" -I"include" -lpthread -lmingw32 -lSDL2main -lSDL2 && a
 gcc -D test_video -g -Wall -L"/usr/lib/" -I"/usr/include/ffmpeg/"  -I"../SDL2/include/" video.c array.c tween.c ease.c sprite.c mystring.c matrix.c -LGLESv2 -lm -lSDL2 -D debug_video  -lm -lavformat -lavcodec -lavutil -lswscale -lswresample -lavdevice -lz -lpthread  && ./a.out
 gcc -D test_video -g -Wall  -I"../SDL2/include/" -I"/home/libiao/src/android-ndk-r10/sources/ffmpeg/android/arm/include/" video.c array.c tween.c ease.c sprite.c mystring.c matrix.c -LGLESv2 -lm -lSDL2 -D debug_video  -lm -lavformat -lavcodec -lavutil -lswscale -lswresample  -L"/usr/local/lib/" -lavdevice -lz -lpthread -llzma && ./a.out
 */
#include "video.h"

static AVPacket null_pkt;

/**
 * Check if the given stream matches a stream specifier.
 *
 * @param s  Corresponding format context.
 * @param st Stream from s to be checked.
 * @param spec A stream specifier of the [v|a|s|d]:[\<stream index\>] form.
 *
 * @return 1 if the stream matches, 0 if it doesn't, <0 on error
 */
static int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec)
{
	int ret = avformat_match_stream_specifier(s, st, spec);
	if (ret < 0)
		SDL_Log("Invalid stream specifier: %s.\n", spec);
	return ret;
}

/**
 * Filter out options for given codec.
 *
 * Create a new options dictionary containing only the options from
 * opts which apply to the codec with ID codec_id.
 *
 * @param opts     dictionary to place options in
 * @param codec_id ID of the codec that should be filtered for
 * @param s Corresponding format context.
 * @param st A stream from s for which the options should be filtered.
 * @param codec The particular codec for which the options should be filtered.
 *              If null, the default one is looked up according to the codec id.
 * @return a pointer to the created dictionary
 */
static AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
		AVFormatContext *s, AVStream *st, AVCodec *codec)
{
	AVDictionary    *ret = NULL;
	AVDictionaryEntry *t = NULL;
	int            flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM
		: AV_OPT_FLAG_DECODING_PARAM;
	char          prefix = 0;
	const AVClass    *cc = avcodec_get_class();

	if (!codec)
		codec            = s->oformat ? avcodec_find_encoder(codec_id)
			: avcodec_find_decoder(codec_id);

	switch (st->codec->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			prefix  = 'v';
			flags  |= AV_OPT_FLAG_VIDEO_PARAM;
			break;
		case AVMEDIA_TYPE_AUDIO:
			prefix  = 'a';
			flags  |= AV_OPT_FLAG_AUDIO_PARAM;
			break;
		default:
			break;
	}

	while ((t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX))) {
		char *p = strchr(t->key, ':');

		/* check stream specification in opt name */
		if (p)
			switch (check_stream_specifier(s, st, p + 1)) {
				case  1: *p = 0; break;
				case  0:         continue;
				default:         exit(1);
			}

		if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) ||
				!codec ||
				(codec->priv_class &&
				 av_opt_find(&codec->priv_class, t->key, NULL, flags,
					 AV_OPT_SEARCH_FAKE_OBJ)))
			av_dict_set(&ret, t->key, t->value, 0);
		else if (t->key[0] == prefix &&
				av_opt_find(&cc, t->key + 1, NULL, flags,
					AV_OPT_SEARCH_FAKE_OBJ))
			av_dict_set(&ret, t->key + 1, t->value, 0);

		if (p)
			*p = ':';
	}
	return ret;
}

static void free_picture(Frame *vp)
{
	if (vp->bmp) {
		SDL_DestroyTexture(vp->bmp);
		vp->bmp = NULL;
	}
	if (vp->surface) {
		//GL_CHECK(gles2.glDeleteTextures(1,&(sprite->textureId)));
		SDL_FreeSurface(vp->surface);
		vp->surface= NULL;
	}
}

/**
 * put pkt in a new PacketList , and append the new PacketList to q
 */
static int packet_queue_put_private(PacketQueue *q, AVPacket *pkt)
{
	PacketList *pkt1;

	if (q->abort_request)
		return -1;

	pkt1 = av_malloc(sizeof(PacketList));
	if (!pkt1)
		return -1;
	pkt1->pkt = *pkt;
	pkt1->next = NULL;
	if (pkt == &null_pkt)
		q->serialno++;
	pkt1->index = q->serialno;

	if (!q->last_pkt)
		q->first_pkt = pkt1;
	else
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->numPackets++;
	q->size += pkt1->pkt.size + sizeof(*pkt1);
	/* XXX: should duplicate packet data in DV case */
	SDL_CondSignal(q->cond);
	return 0;
}

static int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
	int ret;

	SDL_LockMutex(q->mutex);
	ret = packet_queue_put_private(q, pkt);
	SDL_UnlockMutex(q->mutex);

	if (pkt != &null_pkt && ret < 0)
		av_packet_unref(pkt);

	return ret;
}

static int packet_queue_put_nullpacket(PacketQueue *q, int stream_index)
{
	AVPacket pkt1, *pkt = &pkt1;
	av_init_packet(pkt);
	pkt->data = NULL;
	pkt->size = 0;
	pkt->stream_index = stream_index;
	return packet_queue_put(q, pkt);
}

/* packet queue handling */
static void packet_queue_init(PacketQueue *q)
{
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
	q->abort_request = 1;
}

static void packet_queue_flush(PacketQueue *q)
{
	PacketList *pkt, *pkt1;

	SDL_LockMutex(q->mutex);
	for (pkt = q->first_pkt; pkt; pkt = pkt1) {
		pkt1 = pkt->next;
		//av_free_packet(&pkt->pkt);
		av_packet_unref(&pkt->pkt);
		av_freep(&pkt);
	}
	q->last_pkt = NULL;
	q->first_pkt = NULL;
	q->numPackets = 0;
	q->size = 0;
	SDL_UnlockMutex(q->mutex);
}

static void packet_queue_destroy(PacketQueue *q)
{
	packet_queue_flush(q);
	SDL_DestroyMutex(q->mutex);
	SDL_DestroyCond(q->cond);
}

static void packet_queue_abort(PacketQueue *q)
{
	SDL_LockMutex(q->mutex);
	q->abort_request = 1;
	SDL_CondSignal(q->cond);
	SDL_UnlockMutex(q->mutex);
}

/**
 *
 * get the first packet of q, save to pkt, save the serial number to serial;
 */
/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial)
{
	PacketList *pkt1;
	int ret;

	SDL_LockMutex(q->mutex);

	for (;;) {
		if (q->abort_request) {
			ret = -1;
			break;
		}

		pkt1 = q->first_pkt;
		if (pkt1) {
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->numPackets --;
			q->size -= pkt1->pkt.size + sizeof(*pkt1);
			*pkt = pkt1->pkt;
			if (serial)
				*serial = pkt1->index;
			av_free(pkt1);
			ret = 1;
			break;
		} else if (!block) {
			ret = 0;
			break;
		} else {
			SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}

static void decoder_init(Decoder *d, AVCodecContext *avctx, PacketQueue *queue) 
{
	memset(d, 0, sizeof(Decoder));
	d->avctx = avctx;
	d->queue = queue;
	d->start_pts = AV_NOPTS_VALUE;
}

static void toggle_mute(Video * video)
{
	video->muted = !video->muted;
}

static void update_volume(Video *video, int sign, int step)
{
	video->audio_volume = av_clip(video->audio_volume + sign * step, 0, SDL_MIX_MAXVOLUME);
}


static int decoder_decode_frame(Decoder *d, AVFrame *frame) 
{
	int got_frame = 0;

	do {
		int ret = -1;

		if (d->queue->abort_request)
			return -1;

		if (!d->packet_pending || d->queue->serialno != d->pkt_serial) {
			AVPacket pkt;
			do {
				if (packet_queue_get(d->queue, &pkt, 1, &d->pkt_serial) < 0)
					return -1;
				if (pkt.data == null_pkt.data) {
					avcodec_flush_buffers(d->avctx);
					d->finished = 0;
					d->next_pts = d->start_pts;
					d->next_pts_tb = d->start_pts_tb;
				}
			} while (pkt.data == null_pkt.data || d->queue->serialno != d->pkt_serial);
			//av_free_packet(&d->pkt);
			av_packet_unref(&d->pkt);
			d->pkt_temp = d->pkt = pkt;
			d->packet_pending = 1;
		}

		switch (d->avctx->codec_type) {
			case AVMEDIA_TYPE_VIDEO:
				ret = avcodec_decode_video2(d->avctx, frame, &got_frame, &d->pkt_temp);
				if (got_frame) {
					int decoder_reorder_pts = -1;
					if (decoder_reorder_pts == -1) {
						frame->pts = av_frame_get_best_effort_timestamp(frame);
					} else if (decoder_reorder_pts) {
						frame->pts = frame->pkt_pts;
					} else {
						frame->pts = frame->pkt_dts;
					}
				}
				break;
			case AVMEDIA_TYPE_AUDIO:
				ret = avcodec_decode_audio4(d->avctx, frame, &got_frame, &d->pkt_temp);
				if (got_frame) {
					AVRational tb = (AVRational){1, frame->sample_rate};
					if (frame->pts != AV_NOPTS_VALUE)
						frame->pts = av_rescale_q(frame->pts, d->avctx->time_base, tb);
					else if (frame->pkt_pts != AV_NOPTS_VALUE)
						frame->pts = av_rescale_q(frame->pkt_pts, av_codec_get_pkt_timebase(d->avctx), tb);
					else if (d->next_pts != AV_NOPTS_VALUE)
						frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
					if (frame->pts != AV_NOPTS_VALUE) {
						d->next_pts = frame->pts + frame->nb_samples;
						d->next_pts_tb = tb;
					}
				}
				break;
			default:
				break;
		}

		if (ret < 0) {
			d->packet_pending = 0;
		} else {
			d->pkt_temp.dts =
				d->pkt_temp.pts = AV_NOPTS_VALUE;
			if (d->pkt_temp.data) {
				if (d->avctx->codec_type != AVMEDIA_TYPE_AUDIO)
					ret = d->pkt_temp.size;
				d->pkt_temp.data += ret;
				d->pkt_temp.size -= ret;
				if (d->pkt_temp.size <= 0)
					d->packet_pending = 0;
			} else {
				if (!got_frame) {
					d->packet_pending = 0;
					d->finished = d->pkt_serial;
				}
			}
		}
	} while (!got_frame && !d->finished);

	return got_frame;
}

static void decoder_destroy(Decoder *d) 
{
	//av_free_packet(&d->pkt);
	av_packet_unref(&d->pkt);
}

static void frame_queue_unref_item(Frame *vp)
{
	av_frame_unref(vp->avframe);
}

static int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last)
{
	int i;
	memset(f, 0, sizeof(FrameQueue));
	if (!(f->mutex = SDL_CreateMutex()))
		return AVERROR(ENOMEM);
	if (!(f->cond = SDL_CreateCond()))
		return AVERROR(ENOMEM);
	f->pktq = pktq;
	f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
	f->keep_last = !!keep_last;
	for (i = 0; i < f->max_size; i++)
		if (!(f->queue[i].avframe = av_frame_alloc()))
			return AVERROR(ENOMEM);
	return 0;
}

static void frame_queue_destory(FrameQueue *f)
{
	int i;
	for (i = 0; i < f->max_size; i++) {
		Frame *vp = &f->queue[i];
		frame_queue_unref_item(vp);
		av_frame_free(&vp->avframe);
		free_picture(vp);
	}
	SDL_DestroyMutex(f->mutex);
	SDL_DestroyCond(f->cond);
}

static void frame_queue_signal(FrameQueue *f)
{
	SDL_LockMutex(f->mutex);
	SDL_CondSignal(f->cond);
	SDL_UnlockMutex(f->mutex);
}

static Frame *frame_queue_peek(FrameQueue *f)
{
	return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

static Frame *frame_queue_peek_next(FrameQueue *f)
{
	return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}

static Frame *frame_queue_peek_last(FrameQueue *f)
{
	return &f->queue[f->rindex];
}

static Frame *frame_queue_peek_writable(FrameQueue *f)
{
	/* wait until we have space to put a new frame */
	SDL_LockMutex(f->mutex);
	while (f->size >= f->max_size && !f->pktq->abort_request) {
		SDL_CondWait(f->cond, f->mutex);
	}
	SDL_UnlockMutex(f->mutex);

	if (f->pktq->abort_request)
		return NULL;

	return &f->queue[f->windex];
}

static Frame *frame_queue_peek_readable(FrameQueue *f)
{
	/* wait until we have a readable a new frame */
	SDL_LockMutex(f->mutex);
	while (f->size - f->rindex_shown <= 0 && !f->pktq->abort_request) {
		SDL_CondWait(f->cond, f->mutex);
	}
	SDL_UnlockMutex(f->mutex);

	if (f->pktq->abort_request)
		return NULL;

	return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

//write a frame to the queue
static void frame_queue_push(FrameQueue *f)
{
	if (++f->windex == f->max_size)
		f->windex = 0;
	SDL_LockMutex(f->mutex);
	f->size++;
	SDL_CondSignal(f->cond);
	SDL_UnlockMutex(f->mutex);
}

//read a frame from the queue and delete it from the queue
static void frame_queue_next(FrameQueue *f)
{
	if (f->keep_last && !f->rindex_shown) {
		f->rindex_shown = 1;
		return;
	}
	frame_queue_unref_item(&f->queue[f->rindex]);
	if (++f->rindex == f->max_size)
		f->rindex = 0;
	SDL_LockMutex(f->mutex);
	f->size--;
	SDL_CondSignal(f->cond);
	SDL_UnlockMutex(f->mutex);
}

/* jump back to the previous frame if available by resetting rindex_shown */
static int frame_queue_prev(FrameQueue *f)
{
	int ret = f->rindex_shown;
	f->rindex_shown = 0;
	return ret;
}

/* return the number of undisplayed frames in the queue */
static int frame_queue_nb_remaining(FrameQueue *f)
{
	return f->size - f->rindex_shown;
}

/* return last shown position */
static int64_t frame_queue_last_pos(FrameQueue *f)
{
	Frame *fp = &f->queue[f->rindex];
	if (f->rindex_shown && fp->frameIndex == f->pktq->serialno)
		return fp->pos;
	else
		return -1;
}

static void decoder_abort(Decoder *d, FrameQueue *fq)
{
	packet_queue_abort(d->queue);
	frame_queue_signal(fq);
	SDL_WaitThread(d->decoder_tid, NULL);
	d->decoder_tid = NULL;
	packet_queue_flush(d->queue);
}

void Video_clear(Video*video)
{
	if(video->sprite)
	{
		Sprite_destroy(video->sprite);
		video->sprite = NULL;
	}
	if(video->timerId)
	{
		SDL_bool b = SDL_RemoveTimer(video->timerId);
		if(b){
			SDL_Log("SDL_RemoveTimer success!");
		}
		video->timerId = 0;
	}
	/* XXX: use a special url_shutdown call to abort parse cleanly */
	video->abort_request = 1;
	SDL_WaitThread(video->read_tid, NULL);
	packet_queue_destroy(&video->videoq);
	packet_queue_destroy(&video->audioq);

	/* free all pictures */
	frame_queue_destory(&video->pictq);
	frame_queue_destory(&video->sampq);
	SDL_DestroyCond(video->continue_read_thread);
	av_free(video);
}

static double get_clock(Clock *c)
{
	if (*c->queue_serial != c->packet_index)
		return NAN;
	if (c->paused) {
		return c->pts;
	} else {
		double time = av_gettime_relative() / 1000000.0;
		return c->pts_drift + time - (time - c->last_updated_time);
	}
}

static void set_clock_at(Clock *c, double pts, int serial, double time)
{
	c->pts = pts;
	c->last_updated_time = time;
	c->pts_drift = c->pts - time;
	c->packet_index = serial;
}

static void set_clock(Clock *c, double pts, int serial)
{
	double time = av_gettime_relative() / 1000000.0;
	set_clock_at(c, pts, serial, time);
}

static void init_clock(Clock *c, int *queue_serial)
{
	c->paused = 0;
	c->queue_serial = queue_serial;
	set_clock(c, NAN, -1);
}

static void sync_clock_to_slave(Clock *c, Clock *slave)
{
	double clock = get_clock(c);
	double slave_clock = get_clock(slave);
	if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
		set_clock(c, slave_clock, slave->packet_index);
}

/* seek in the stream */
void Video_seek(Video*video, int64_t pos, int64_t rel, int seek_by_bytes)
{
	if (!video->seek_req) {
		video->seek_pos = pos;
		video->seek_rel = rel;
		video->seek_flags &= ~AVSEEK_FLAG_BYTE;
		if (seek_by_bytes)
			video->seek_flags |= AVSEEK_FLAG_BYTE;
		video->seek_req = 1;
		SDL_CondSignal(video->continue_read_thread);
	}
}

/* pause or resume the video */
static void stream_toggle_pause(Video*video)
{
	if (video->paused) {
		video->frame_timer += av_gettime_relative() / 1000000.0 - video->vidclk.last_updated_time;
		if (video->read_pause_return != AVERROR(ENOSYS)) {
			video->vidclk.paused = 0;
		}
		set_clock(&video->vidclk, get_clock(&video->vidclk), video->vidclk.packet_index);
	}
	set_clock(&video->extclk, get_clock(&video->extclk), video->extclk.packet_index);
	video->paused = video->audclk.paused = video->vidclk.paused = video->extclk.paused = !video->paused;
}

void Video_pause(Video * video)
{
	stream_toggle_pause(video);
	video->step = 0;
}

void Video_nextFrame(Video*video)
{
	/* if the stream video paused unpause it, then step */
	if (video->paused)
		stream_toggle_pause(video);
	video->step = 1;
}

static double compute_target_delay(double delay, Video*video)
{
	double sync_threshold, diff = 0;

	/* update delay to follow master synchronisation source */
	//if (get_master_sync_type(video) != AV_SYNC_VIDEO_MASTER) 
	{
		/* if video video slave, we try to correct big delays by
		   duplicating or deleting a frame */
		diff = get_clock(&video->vidclk) - get_clock(&video->extclk);

		/* skip or repeat frame. We take into account the
		   delay to compute the threshold. I still don't know
		   if it video the best guess */
		sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
		if (!isnan(diff) && fabs(diff) < video->max_frame_duration) {
			if (diff <= -sync_threshold)
				delay = FFMAX(0, delay + diff);
			else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
				delay = delay + diff;
			else if (diff >= sync_threshold)
				delay = 2 * delay;
		}
	}
	//SDL_Log( "video: delay=%0.3f A-V=%f\n", delay, -diff);
	return delay;
}

static double frame_duration(Video*video, Frame *frame, Frame *nextframe) 
{
	if (frame->frameIndex == nextframe->frameIndex) {
		double duration = nextframe->pts - frame->pts;
		if (isnan(duration) || duration <= 0 || duration > video->max_frame_duration)
			return frame->duration;
		else
			return duration;
	} 
	return 0.0;
}

static SDL_Surface * avframe2surface(Video * video,AVFrame * pFrame)
{
	int align = 1;
	AVFrame * pFrameRGB = av_frame_alloc();
	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pFrame->width, pFrame->height,align);
	uint8_t * buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	//SDL_Log("%d----------------%dx%d",numBytes,pFrame->width,pFrame->height);
	AVCodecContext	*pCodecCtx = video->viddec.avctx;
	//avpicture_fill((AVPicture*)pFrameRGB,buffer,AV_PIX_FMT_RGB24,pFrame->width, pFrame->height);
	av_image_fill_arrays(pFrameRGB->data,pFrameRGB->linesize,
			buffer,AV_PIX_FMT_RGB24,
			pFrame->width, pFrame->height,align);
	struct SwsContext *img_convert_ctx = sws_getContext(
			pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, //src w h fmt
			pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24,//dest w h fmt
			SWS_BICUBIC,//flags
			NULL, NULL, NULL);//srtfilter destfilter params
	if(img_convert_ctx == NULL)
	{
		fprintf(stderr, "Cannot initialize the conversion context!\n");
		exit(1);
	}
	sws_scale(img_convert_ctx,
			(void*)pFrame->data, pFrame->linesize,//src
			0, pFrame->height,//all lines
			pFrameRGB->data, pFrameRGB->linesize);//dest
	//SDL_Log("%d----------------%dx%dx%d",numBytes,pFrameRGB->linesize[0],pFrameRGB->linesize[1],pFrameRGB->linesize[2]);

	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
#if 1
			(void*)buffer,
#else
			(void*)pFrameRGB->data[0],
#endif
			pFrame->width,
			pFrame->height,
			3 * 8,
			pFrame->width * 3,
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
	sws_freeContext(img_convert_ctx);
	av_frame_free(&pFrameRGB);
	//SDL_Log("%d----------------%dx%d surface",numBytes,surface->w,surface->h);
	return surface;
}

static int queue_picture(Video*video, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial)
{
	Frame *frame;
	//SDL_Log("frame_type=%c pts=%0.3f\n", av_get_picture_type_char(src_frame->pict_type), pts);
	if (!(frame = frame_queue_peek_writable(&video->pictq)))
		return -1;

	frame->pts = pts;
	frame->duration = duration;
	frame->pos = pos;
	frame->frameIndex = serial;

	/* alloc or resize hardware picture buffer */
	if ((frame->bmp == NULL && frame->surface==NULL) ||
			frame->width  != src_frame->width ||
			frame->height != src_frame->height)
	{
		if (video->videoq.abort_request)
		{
			return -1;
		}
		frame->width = src_frame->width;
		frame->height = src_frame->height;
	}
	free_picture(frame);
	SDL_Surface * surface = avframe2surface(video,src_frame);
	if(surface){
		frame->surface = surface;
	}else{
		frame->bmp = SDL_CreateTexture(stage->renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, src_frame->width,src_frame->height);  
		SDL_UpdateYUVTexture(frame->bmp, NULL,\
				src_frame->data[0], src_frame->linesize[0],\
				src_frame->data[1], src_frame->linesize[1],\
				src_frame->data[2], src_frame->linesize[2]\
				);
	}
	frame_queue_push(&video->pictq);
	return 0;
}

/* display the current picture, if any */
static void video_display(Video*video)
{
	if (video->video_st)
	{
		Frame *frame;
		frame = frame_queue_peek(&video->pictq);
		SDL_LockMutex(video->pictq.mutex);
		if (frame->bmp || frame->surface)
		{
			Sprite * sprite = video->sprite;
			video->w = frame->width;
			video->h = frame->height;
			sprite->w = video->w;
			sprite->h = video->h;
			Sprite_fullcenter(sprite,0,0,stage->stage_w,stage->stage_h);
			if(frame->surface){
				Sprite_setSurface(sprite,frame->surface);
				frame->surface->refcount--;
				frame->surface = NULL;
			}else{
				Sprite_destroyTexture(sprite);
				sprite->texture = frame->bmp;
			}
			Stage_redraw();
		}
		SDL_UnlockMutex(video->pictq.mutex);
	}
}
/* called to display each frame */
static void video_refresh(void *opaque, double *remaining_time)
{/*{{{*/
	Video*video = opaque;
	double time;

	if (video->show_mode != SHOW_MODE_VIDEO && video->audio_st) {
		time = av_gettime_relative() / 1000000.0;
		double rdftspeed = 0.02;
		if (video->force_refresh || video->last_vis_time + rdftspeed < time) {
			video_display(video);
			video->last_vis_time = time;
		}
		*remaining_time = FFMIN(*remaining_time, video->last_vis_time + rdftspeed - time);
	}

	if (video->video_st) {
		int redisplay = 0;
		if (video->force_refresh)
			redisplay = frame_queue_prev(&video->pictq);
retry:
		if (frame_queue_nb_remaining(&video->pictq) == 0) {
			// nothing to do, no picture to display in the queue
		} else {
			double last_duration, duration, delay;
			Frame *vp, *lastvp;

			/* dequeue the picture */
			lastvp = frame_queue_peek_last(&video->pictq);
			vp = frame_queue_peek(&video->pictq);

			if (vp->frameIndex != video->videoq.serialno) {
				frame_queue_next(&video->pictq);
				redisplay = 0;
				goto retry;
			}

			if (lastvp->frameIndex != vp->frameIndex && !redisplay)
				video->frame_timer = av_gettime_relative() / 1000000.0;

			if (video->paused)
				goto display;

			/* compute nominal last_duration */
			last_duration = frame_duration(video, lastvp, vp);
			if (redisplay)
				delay = 0.0;
			else
				delay = compute_target_delay(last_duration, video);

			time= av_gettime_relative()/1000000.0;
			if (time < video->frame_timer + delay && !redisplay) {
				*remaining_time = FFMIN(video->frame_timer + delay - time, *remaining_time);
				return;
			}

			video->frame_timer += delay;
			if (delay > 0 && time - video->frame_timer > AV_SYNC_THRESHOLD_MAX)
				video->frame_timer = time;

			SDL_LockMutex(video->pictq.mutex);
			if (!redisplay && !isnan(vp->pts))
			{
				/* update current video pts */
				set_clock(&video->vidclk, vp->pts, vp->frameIndex);
				sync_clock_to_slave(&video->extclk, &video->vidclk);
			}
			SDL_UnlockMutex(video->pictq.mutex);

			if (frame_queue_nb_remaining(&video->pictq) > 1) {
				Frame *nextvp = frame_queue_peek_next(&video->pictq);
				duration = frame_duration(video, vp, nextvp);
				if(!video->step && time > video->frame_timer + duration){
					frame_queue_next(&video->pictq);
					redisplay = 0;
					goto retry;
				}
			}
display:
			/* display picture */
			if (video->show_mode == SHOW_MODE_VIDEO)
				video_display(video);

			frame_queue_next(&video->pictq);

			if (video->step && !video->paused)
				stream_toggle_pause(video);
		}
	}
	video->force_refresh = 0;
}/*}}}*/

static int get_video_frame(Video*video, AVFrame *frame)
{/*{{{*/
	int got_picture;

	if ((got_picture = decoder_decode_frame(&video->viddec, frame)) < 0)
		return -1;

	if (got_picture) {
		double dpts = NAN;

		if (frame->pts != AV_NOPTS_VALUE)
			dpts = av_q2d(video->video_st->time_base) * frame->pts;

		frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(video->avformatctx, video->video_st, frame);

		if (frame->pts != AV_NOPTS_VALUE) {
			double diff = dpts - get_clock(&video->extclk);
			if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
					diff < 0 &&
					video->viddec.pkt_serial == video->vidclk.packet_index &&
					video->videoq.numPackets) {
				av_frame_unref(frame);
				got_picture = 0;
			}
		}
	}
	return got_picture;
}/*}}}*/


static int audio_thread(void *arg)
{/*{{{*/
	Video*video = arg;
	AVFrame *frame = av_frame_alloc();
	Frame *af;
	int got_frame = 0;
	AVRational tb;
	int ret = 0;

	if (!frame)
		return AVERROR(ENOMEM);

	do {
		if ((got_frame = decoder_decode_frame(&video->auddec, frame)) < 0)
			goto the_end;

		if (got_frame) {
			tb = (AVRational){1, frame->sample_rate};
			if (!(af = frame_queue_peek_writable(&video->sampq)))
				goto the_end;

			af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
			af->pos = av_frame_get_pkt_pos(frame);
			af->frameIndex = video->auddec.pkt_serial;
			af->duration = av_q2d((AVRational){frame->nb_samples, frame->sample_rate});

			av_frame_move_ref(af->avframe, frame);
			frame_queue_push(&video->sampq);

		}
	} while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);
the_end:
	av_frame_free(&frame);
	return ret;
}/*}}}*/

static void decoder_start(Decoder *d, int (*fn)(void *), void *arg)
{/*{{{*/
	PacketQueue * q = d->queue;
	SDL_LockMutex(q->mutex);
	q->abort_request = 0;
	packet_queue_put_private(q, &null_pkt);
	SDL_UnlockMutex(q->mutex);
	d->decoder_tid = SDL_CreateThread(fn, NULL,arg);
}/*}}}*/

static int video_thread(void *arg)
{/*{{{*/
	Video*video = arg;
	AVFrame *frame = av_frame_alloc();
	double pts;
	double duration;
	int ret;
	AVRational tb = video->video_st->time_base;
	AVRational frame_rate = av_guess_frame_rate(video->avformatctx, video->video_st, NULL);

	if (!frame) {
		return AVERROR(ENOMEM);
	}

	for (;;) {
		ret = get_video_frame(video, frame);
		if (ret < 0)
			goto the_end;
		if (!ret)
			continue;

		duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);
		pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
		ret = queue_picture(video, frame, pts, duration, av_frame_get_pkt_pos(frame), video->viddec.pkt_serial);
		av_frame_unref(frame);

		if (ret < 0)
			goto the_end;
	}
the_end:
	av_frame_free(&frame);
	return 0;
}/*}}}*/

/* return the wanted number of samples to get better sync if sync_type is video
 * or external master clock */
static int synchronize_audio(Video*video, int nb_samples)
{/*{{{*/
	int wanted_nb_samples = nb_samples;

	double diff, avg_diff;
	int min_nb_samples, max_nb_samples;

	diff = get_clock(&video->audclk) - get_clock(&video->extclk);

	if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {
		video->audio_diff_cum = diff + video->audio_diff_avg_coef * video->audio_diff_cum;
		if (video->audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
			/* not enough measures to have a correct estimate */
			video->audio_diff_avg_count++;
		} else {
			/* estimate the A-V difference */
			avg_diff = video->audio_diff_cum * (1.0 - video->audio_diff_avg_coef);

			if (fabs(avg_diff) >= video->audio_diff_threshold) 
			{
				wanted_nb_samples = nb_samples + (int)(diff * video->audio_src.freq);
				min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
				max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
				wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
			}
			/*
			   SDL_Log( "diff=%f adiff=%f sample_diff=%d apts=%0.3f %f\n",\
			   diff, avg_diff, wanted_nb_samples - nb_samples,\
			   video->audio_clock, video->audio_diff_threshold);
			   */
		}
	} else {
		/* too big difference : may be initial PTS errors, so
		   reset A-V filter */
		video->audio_diff_avg_count = 0;
		video->audio_diff_cum       = 0;
	}

	return wanted_nb_samples;
}/*}}}*/

/**
 * Decode one audio frame and return its uncompressed size.
 *
 * The processed audio frame is decoded, converted if required, and
 * stored in is->audio_buf, with size in bytes given by the return
 * value.
 */
static int audio_decode_frame(Video*video)
{
	int data_size, resampled_data_size;
	int64_t dec_channel_layout;
	av_unused double audio_clock0;
	int wanted_nb_samples;
	Frame *af;

	if (video->paused)
		return -1;

	do {
		if (!(af = frame_queue_peek_readable(&video->sampq)))
			return -1;
		frame_queue_next(&video->sampq);
	} while (af->frameIndex != video->audioq.serialno);

	data_size = av_samples_get_buffer_size(NULL, av_frame_get_channels(af->avframe),
			af->avframe->nb_samples,
			af->avframe->format, 1);

	dec_channel_layout =
		(af->avframe->channel_layout && av_frame_get_channels(af->avframe) == av_get_channel_layout_nb_channels(af->avframe->channel_layout)) ?
		af->avframe->channel_layout : av_get_default_channel_layout(av_frame_get_channels(af->avframe));
	wanted_nb_samples = synchronize_audio(video, af->avframe->nb_samples);

	if (af->avframe->format        != video->audio_src.fmt            ||
			dec_channel_layout       != video->audio_src.channel_layout ||
			af->avframe->sample_rate   != video->audio_src.freq           ||
			(wanted_nb_samples       != af->avframe->nb_samples && !video->swr_ctx)) {
		swr_free(&video->swr_ctx);
		video->swr_ctx = swr_alloc_set_opts(NULL,
				video->audio_tgt.channel_layout, video->audio_tgt.fmt, video->audio_tgt.freq,
				dec_channel_layout,           af->avframe->format, af->avframe->sample_rate,
				0, NULL);
		if (!video->swr_ctx || swr_init(video->swr_ctx) < 0) {
			SDL_Log( "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
					af->avframe->sample_rate, av_get_sample_fmt_name(af->avframe->format), av_frame_get_channels(af->avframe),
					video->audio_tgt.freq, av_get_sample_fmt_name(video->audio_tgt.fmt), video->audio_tgt.channels);
			swr_free(&video->swr_ctx);
			return -1;
		}
		video->audio_src.channel_layout = dec_channel_layout;
		video->audio_src.channels       = av_frame_get_channels(af->avframe);
		video->audio_src.freq = af->avframe->sample_rate;
		video->audio_src.fmt = af->avframe->format;
	}

	if (video->swr_ctx) {
		const uint8_t **in = (const uint8_t **)af->avframe->extended_data;
		uint8_t **out = &video->audio_buf1;
		int out_count = (int64_t)wanted_nb_samples * video->audio_tgt.freq / af->avframe->sample_rate + 256;
		int out_size  = av_samples_get_buffer_size(NULL, video->audio_tgt.channels, out_count, video->audio_tgt.fmt, 0);
		int len2;
		if (out_size < 0) {
			SDL_Log("av_samples_get_buffer_size() failed\n");
			return -1;
		}
		if (wanted_nb_samples != af->avframe->nb_samples) {
			if (swr_set_compensation(video->swr_ctx, (wanted_nb_samples - af->avframe->nb_samples) * video->audio_tgt.freq / af->avframe->sample_rate,
						wanted_nb_samples * video->audio_tgt.freq / af->avframe->sample_rate) < 0) {
				SDL_Log("swr_set_compensation() failed\n");
				return -1;
			}
		}
		av_fast_malloc(&video->audio_buf1, &video->audio_buf1_size, out_size);
		if (!video->audio_buf1)
			return AVERROR(ENOMEM);
		len2 = swr_convert(video->swr_ctx, out, out_count, in, af->avframe->nb_samples);
		if (len2 < 0) {
			SDL_Log("swr_convert() failed\n");
			return -1;
		}
		if (len2 == out_count) {
			SDL_Log("audio buffer video probably too small\n");
			if (swr_init(video->swr_ctx) < 0)
				swr_free(&video->swr_ctx);
		}
		video->audio_buf = video->audio_buf1;
		resampled_data_size = len2 * video->audio_tgt.channels * av_get_bytes_per_sample(video->audio_tgt.fmt);
	} else {
		video->audio_buf = af->avframe->data[0];
		resampled_data_size = data_size;
	}

	audio_clock0 = video->audio_clock;
	/* update the audio clock with the pts */
	if (!isnan(af->pts))
		video->audio_clock = af->pts + (double) af->avframe->nb_samples / af->avframe->sample_rate;
	else
		video->audio_clock = NAN;
	video->audio_clock_serial = af->frameIndex ;
#ifdef DEBUG
	{
		static double last_clock;
		printf("audio: delay=%0.3f clock=%0.3f clock0=%0.3f\n",
				video->audio_clock - last_clock,
				video->audio_clock, audio_clock0);
		last_clock = video->audio_clock;
	}
#endif
	return resampled_data_size;
}

/* prepare a new audio buffer */
static void sdl_audio_callback(void *opaque, Uint8 *stream, int len)
{
	Video*video = opaque;
	int audio_size, len1;

	while (len > 0) {
		if (video->audio_buf_index >= video->audio_buf_size) {
			audio_size = audio_decode_frame(video);
			if (audio_size < 0) {
				/* if error, just output silence */
				video->audio_buf      = video->silence_buf;
				video->audio_buf_size = sizeof(video->silence_buf) / video->audio_tgt.frame_size * video->audio_tgt.frame_size;
			} else {
				video->audio_buf_size = audio_size;
			}
			video->audio_buf_index = 0;
		}
		len1 = video->audio_buf_size - video->audio_buf_index;
		if (len1 > len)
			len1 = len;
		if (!video->muted && video->audio_volume == SDL_MIX_MAXVOLUME)
			memcpy(stream, (uint8_t *)video->audio_buf + video->audio_buf_index, len1);
		else {
			memset(stream, video->silence_buf[0], len1);
			if (!video->muted)
				SDL_MixAudio(stream, (uint8_t *)video->audio_buf + video->audio_buf_index, len1, video->audio_volume);
		}
		len -= len1;
		stream += len1;
		video->audio_buf_index += len1;
	}
	video->audio_write_buf_size = video->audio_buf_size - video->audio_buf_index;
	/* Let's assume the audio driver that video used by SDL has two periods. */
	if (!isnan(video->audio_clock)) {
		set_clock_at(&video->audclk, video->audio_clock - (double)(2 * video->audio_hw_buf_size + video->audio_write_buf_size) / video->audio_tgt.bytes_per_sec, video->audio_clock_serial, av_gettime_relative()/ 1000000.0);
		sync_clock_to_slave(&video->extclk, &video->audclk);
	}
}

static int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams *audio_hw_params)
{
	SDL_AudioSpec wanted_spec, spec;
	static const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
	static const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};
	int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;

	if (!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout)) {
		wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
		wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
	}
	wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
	wanted_spec.channels = wanted_nb_channels;
	wanted_spec.freq = wanted_sample_rate;
	if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
		SDL_Log("Invalid sample rate or channel count!\n");
		return -1;
	}
	while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
		next_sample_rate_idx--;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.silence = 0;
	wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
	wanted_spec.callback = sdl_audio_callback;
	wanted_spec.userdata = opaque;
	while (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
		SDL_Log( "SDL_OpenAudio (%d channels, %d Hz): %s\n",
				wanted_spec.channels, wanted_spec.freq, SDL_GetError());
		wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
		if (!wanted_spec.channels) {
			wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
			wanted_spec.channels = wanted_nb_channels;
			if (!wanted_spec.freq) {
				SDL_Log("No more combinations to try, audio open failed\n");
				return -1;
			}
		}
		wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
	}
	if (spec.format != AUDIO_S16SYS) {
		SDL_Log( "SDL advised audio format %d video not supported!\n", spec.format);
		return -1;
	}
	if (spec.channels != wanted_spec.channels) {
		wanted_channel_layout = av_get_default_channel_layout(spec.channels);
		if (!wanted_channel_layout) {
			SDL_Log( "SDL advised channel count %d video not supported!\n", spec.channels);
			return -1;
		}
	}

	audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
	audio_hw_params->freq = spec.freq;
	audio_hw_params->channel_layout = wanted_channel_layout;
	audio_hw_params->channels =  spec.channels;
	audio_hw_params->frame_size = av_samples_get_buffer_size(NULL, audio_hw_params->channels, 1, audio_hw_params->fmt, 1);
	audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->channels, audio_hw_params->freq, audio_hw_params->fmt, 1);
	if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0) {
		SDL_Log("av_samples_get_buffer_size failed\n");
		return -1;
	}
	return spec.size;
}

/* open a given stream. Return 0 if OK */
static int stream_component_open(Video*video, int stream_index)
{
	AVFormatContext *ic = video->avformatctx;
	AVCodecContext *avctx;
	AVCodec *codec;
	AVDictionary *opts;
	AVDictionaryEntry *t = NULL;
	int sample_rate, nb_channels;
	int64_t channel_layout;
	int ret = 0;
	int stream_lowres = 0;

	if (stream_index < 0 || stream_index >= ic->nb_streams)
		return -1;
	avctx = ic->streams[stream_index]->codec;
	codec = avcodec_find_decoder(avctx->codec_id);

	switch(avctx->codec_type){
		case AVMEDIA_TYPE_AUDIO   : video->last_audio_stream    = stream_index; break;
		case AVMEDIA_TYPE_VIDEO   : video->last_video_stream    = stream_index; break;
		default:break;
	}
	if (!codec) {
		SDL_Log("No codec could be found with id %d\n", avctx->codec_id);
		return -1;
	}

	avctx->codec_id = codec->id;
	if(stream_lowres > av_codec_get_max_lowres(codec)){
		SDL_Log("The maximum value for lowres supported by the decoder video %d\n",
				av_codec_get_max_lowres(codec));
		stream_lowres = av_codec_get_max_lowres(codec);
	}
	av_codec_set_lowres(avctx, stream_lowres);

	opts = filter_codec_opts(video->codec_opts, avctx->codec_id, ic, ic->streams[stream_index], codec);
	if (!av_dict_get(opts, "threads", NULL, 0))
		av_dict_set(&opts, "threads", "auto", 0);
	if (stream_lowres)
		av_dict_set_int(&opts, "lowres", stream_lowres, 0);
	if (avctx->codec_type == AVMEDIA_TYPE_VIDEO || avctx->codec_type == AVMEDIA_TYPE_AUDIO)
		av_dict_set(&opts, "refcounted_frames", "1", 0);
	if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) {
		goto fail;
	}
	if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
		SDL_Log( "Option %s not found.\n", t->key);
		ret =  AVERROR_OPTION_NOT_FOUND;
		goto fail;
	}

	video->eof = 0;
	ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
	switch (avctx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
			sample_rate    = avctx->sample_rate;
			nb_channels    = avctx->channels;
			channel_layout = avctx->channel_layout;

			/* prepare audio output */
			if ((ret = audio_open(video, channel_layout, nb_channels, sample_rate, &video->audio_tgt)) < 0)
				goto fail;
			video->audio_hw_buf_size = ret;
			video->audio_src = video->audio_tgt;
			video->audio_buf_size  = 0;
			video->audio_buf_index = 0;

			/* init averaging filter */
			video->audio_diff_avg_coef  = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
			video->audio_diff_avg_count = 0;
			/* since we do not have a precise anough audio fifo fullness, we correct audio sync only if larger than this threshold */
			video->audio_diff_threshold = (double)(video->audio_hw_buf_size) / video->audio_tgt.bytes_per_sec;

			video->audio_stream = stream_index;
			video->audio_st = ic->streams[stream_index];

			decoder_init(&video->auddec, avctx, &video->audioq);
			if ((video->avformatctx->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !video->avformatctx->iformat->read_seek) {
				video->auddec.start_pts = video->audio_st->start_time;
				video->auddec.start_pts_tb = video->audio_st->time_base;
			}
			decoder_start(&video->auddec, audio_thread, video);
			SDL_PauseAudio(0);
			break;
		case AVMEDIA_TYPE_VIDEO:
			video->video_stream = stream_index;
			video->video_st = ic->streams[stream_index];
			decoder_init(&video->viddec, avctx, &video->videoq);
			decoder_start(&video->viddec, video_thread, video);
			video->queue_attachments_req = 1;
			break;
		default:
			break;
	}

fail:
	av_dict_free(&opts);
	return ret;
}

static void stream_component_close(Video*video, int stream_index)
{
	AVFormatContext *ic = video->avformatctx;
	AVCodecContext *avctx;

	if (stream_index < 0 || stream_index >= ic->nb_streams)
		return;
	avctx = ic->streams[stream_index]->codec;

	switch (avctx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
			decoder_abort(&video->auddec, &video->sampq);
			SDL_CloseAudio();
			decoder_destroy(&video->auddec);
			swr_free(&video->swr_ctx);
			av_freep(&video->audio_buf1);
			video->audio_buf1_size = 0;
			video->audio_buf = NULL;

			break;
		case AVMEDIA_TYPE_VIDEO:
			decoder_abort(&video->viddec, &video->pictq);
			decoder_destroy(&video->viddec);
			break;
		default:
			break;
	}

	ic->streams[stream_index]->discard = AVDISCARD_ALL;
	avcodec_close(avctx);
	switch (avctx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
			video->audio_st = NULL;
			video->audio_stream = -1;
			break;
		case AVMEDIA_TYPE_VIDEO:
			video->video_st = NULL;
			video->video_stream = -1;
			break;
		default:
			break;
	}
}

static int decode_interrupt_cb(void *ctx)
{
	Video* video = ctx;
	return video->abort_request;
}

/* this thread gets the stream from the disk or the network */
static int read_thread(void *arg)
{
	Video*video = arg;
	AVFormatContext *ic = NULL;
	int err, i, ret;
	int st_index[AVMEDIA_TYPE_NB];
	AVPacket pkt1, *pkt = &pkt1;
	int64_t stream_start_time;
	int pkt_in_play_range = 0;
	SDL_mutex *wait_mutex = SDL_CreateMutex();
	int64_t pkt_ts;

	memset(st_index, -1, sizeof(st_index));
	video->last_video_stream = video->video_stream = -1;
	video->last_audio_stream = video->audio_stream = -1;
	video->eof = 0;

	ic = avformat_alloc_context();
	if (!ic) {
		SDL_Log("Could not allocate context.\n");
		ret = AVERROR(ENOMEM);
		goto fail;
	}
	ic->interrupt_callback.callback = decode_interrupt_cb;
	ic->interrupt_callback.opaque = video;
	err = avformat_open_input(&ic, video->filename, video->iformat, NULL);
	if (err < 0) {
		ret = -1;
		SDL_Log("%s: could not open file!\n", video->filename);
		goto fail;
	}
	video->avformatctx = ic;

	av_format_inject_global_side_data(ic);
	err = avformat_find_stream_info(ic, NULL);
	if (err < 0) {
		SDL_Log("%s: could not find codec parameters\n", video->filename);
		ret = -1;
		goto fail;
	}

	if (ic->pb)
		ic->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end

	if (video->seek_by_bytes < 0)
		video->seek_by_bytes = !!(ic->iformat->flags & AVFMT_TS_DISCONT) && strcmp("ogg", ic->iformat->name);

	video->max_frame_duration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

	for (i = 0; i < ic->nb_streams; i++) {
		AVStream *st = ic->streams[i];
		st->discard = AVDISCARD_ALL;
	}

	st_index[AVMEDIA_TYPE_VIDEO] =
		av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
				st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
	st_index[AVMEDIA_TYPE_AUDIO] =
		av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
				st_index[AVMEDIA_TYPE_AUDIO],
				st_index[AVMEDIA_TYPE_VIDEO],
				NULL, 0);

	video->show_mode = SHOW_MODE_NONE;

	/* open the streams */
	if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
		stream_component_open(video, st_index[AVMEDIA_TYPE_AUDIO]);
	}
	ret = -1;
	if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
		ret = stream_component_open(video, st_index[AVMEDIA_TYPE_VIDEO]);
	}
	if (video->show_mode == SHOW_MODE_NONE)
		video->show_mode = ret >= 0 ? SHOW_MODE_VIDEO : SHOW_MODE_RDFT;

	if (video->video_stream < 0 && video->audio_stream < 0) {
		SDL_Log( "Failed to open file '%s' or configure filtergraph\n",
				video->filename);
		ret = -1;
		goto fail;
	}

	while(1) {
		if (video->abort_request)
			break;
		if (video->paused != video->last_paused) {
			video->last_paused = video->paused;
			if (video->paused)
				video->read_pause_return = av_read_pause(ic);
			else
				av_read_play(ic);
		}
		if (video->seek_req) {
			int64_t seek_target = video->seek_pos;
			int64_t seek_min    = video->seek_rel > 0 ? seek_target - video->seek_rel + 2: INT64_MIN;
			int64_t seek_max    = video->seek_rel < 0 ? seek_target - video->seek_rel - 2: INT64_MAX;
			// FIXME the +-2 video due to rounding being not done in the correct direction in generation
			//      of the seek_pos/seek_rel variables

			ret = avformat_seek_file(video->avformatctx, -1, seek_min, seek_target, seek_max, video->seek_flags);
			if (ret < 0) {
				SDL_Log( "%s: error while seeking\n", video->avformatctx->filename);
			} else {
				if (video->audio_stream >= 0) {
					packet_queue_flush(&video->audioq);
					packet_queue_put(&video->audioq, &null_pkt);
				}
				if (video->video_stream >= 0) {
					packet_queue_flush(&video->videoq);
					packet_queue_put(&video->videoq, &null_pkt);
				}
				if (video->seek_flags & AVSEEK_FLAG_BYTE) {
					set_clock(&video->extclk, NAN, 0);
				} else {
					set_clock(&video->extclk, seek_target / (double)AV_TIME_BASE, 0);
				}
			}
			video->seek_req = 0;
			video->queue_attachments_req = 1;
			video->eof = 0;
			if (video->paused)
				Video_nextFrame(video);
		}
		if (video->queue_attachments_req) {
			if (video->video_st && video->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC) {
				AVPacket copy;
				if ((ret = av_copy_packet(&copy, &video->video_st->attached_pic)) < 0)
					goto fail;
				packet_queue_put(&video->videoq, &copy);
				packet_queue_put_nullpacket(&video->videoq, video->video_stream);
			}
			video->queue_attachments_req = 0;
		}

		/* if the queue are full, no need to read more */
		if ((video->audioq.numPackets > MIN_FRAMES || video->audio_stream < 0 || video->audioq.abort_request)
				&& (video->videoq.numPackets > MIN_FRAMES || video->video_stream < 0 || video->videoq.abort_request
					|| (video->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC))
		   ) {
			/* wait 10 ms */
			SDL_LockMutex(wait_mutex);
			SDL_CondWaitTimeout(video->continue_read_thread, wait_mutex, 10);
			SDL_UnlockMutex(wait_mutex);
			continue;
		}
		if (!video->paused &&
				(!video->audio_st || (video->auddec.finished == video->audioq.serialno && frame_queue_nb_remaining(&video->sampq) == 0)) &&
				(!video->video_st || (video->viddec.finished == video->videoq.serialno && frame_queue_nb_remaining(&video->pictq) == 0))) {
			//SDL_Log("finished");
			ret = AVERROR_EOF;
			goto fail;
		}
		ret = av_read_frame(ic, pkt);
		if (ret < 0) {
			if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !video->eof) {
				if (video->video_stream >= 0)
					packet_queue_put_nullpacket(&video->videoq, video->video_stream);
				if (video->audio_stream >= 0)
					packet_queue_put_nullpacket(&video->audioq, video->audio_stream);
				video->eof = 1;
			}
			if (ic->pb && ic->pb->error)
				break;
			SDL_LockMutex(wait_mutex);
			SDL_CondWaitTimeout(video->continue_read_thread, wait_mutex, 10);
			SDL_UnlockMutex(wait_mutex);
			continue;
		} else {
			video->eof = 0;
		}
		/* check if packet video in play range specified by user, then queue, otherwise discard */
		stream_start_time = ic->streams[pkt->stream_index]->start_time;
		pkt_ts = pkt->pts == AV_NOPTS_VALUE ? pkt->dts : pkt->pts;
		int64_t duration = AV_NOPTS_VALUE;
		pkt_in_play_range = duration == AV_NOPTS_VALUE ||
			(pkt_ts - (stream_start_time != AV_NOPTS_VALUE ? stream_start_time : 0)) * av_q2d(ic->streams[pkt->stream_index]->time_base) <= ((double)duration / 1000000);
		if (pkt->stream_index == video->audio_stream && pkt_in_play_range) {
			packet_queue_put(&video->audioq, pkt);
		} else if (pkt->stream_index == video->video_stream && pkt_in_play_range
				&& !(video->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
			packet_queue_put(&video->videoq, pkt);
		} else {
			//av_free_packet(pkt);
			av_packet_unref(pkt);
		}
	}
	/* wait until the end */
	while (!video->abort_request) {
		SDL_Delay(10);
	}

	ret = 0;
fail:
	/* close each stream */
	if (video->audio_stream >= 0)
		stream_component_close(video, video->audio_stream);
	if (video->video_stream >= 0)
		stream_component_close(video, video->video_stream);
	if (ic) {
		avformat_close_input(&ic);
		video->avformatctx = NULL;
	}

	if (ret != 0) {
		exit(0);
	}
	SDL_DestroyMutex(wait_mutex);
	return 0;
}


static void stream_cycle_channel(Video *video, int codec_type)
{
	AVFormatContext *ic = video->avformatctx;
	int start_index, stream_index;
	int old_index;
	AVStream *st;
	AVProgram *p = NULL;
	int nb_streams = video->avformatctx->nb_streams;

	if (codec_type == AVMEDIA_TYPE_VIDEO) {
		start_index = video->last_video_stream;
		old_index = video->video_stream;
	} else if (codec_type == AVMEDIA_TYPE_AUDIO) {
		start_index = video->last_audio_stream;
		old_index = video->audio_stream;
	} else {
	}
	stream_index = start_index;

	if (codec_type != AVMEDIA_TYPE_VIDEO && video->video_stream != -1) {
		p = av_find_program_from_stream(ic, NULL, video->video_stream);
		if (p) {
			nb_streams = p->nb_stream_indexes;
			for (start_index = 0; start_index < nb_streams; start_index++)
				if (p->stream_index[start_index] == stream_index)
					break;
			if (start_index == nb_streams)
				start_index = -1;
			stream_index = start_index;
		}
	}

	while(1) {
		if (++stream_index >= nb_streams)
		{
			if (start_index == -1)
				return;
			stream_index = 0;
		}
		if (stream_index == start_index)
			return;
		st = video->avformatctx->streams[p ? p->stream_index[stream_index] : stream_index];
		if (st->codec->codec_type == codec_type) {
			/* check that parameters are OK */
			switch (codec_type) {
				case AVMEDIA_TYPE_AUDIO:
					if (st->codec->sample_rate != 0 &&
							st->codec->channels != 0)
						goto the_end;
					break;
				case AVMEDIA_TYPE_VIDEO:
					goto the_end;
				default:
					break;
			}
		}
	}
the_end:
	if (p && stream_index != -1)
		stream_index = p->stream_index[stream_index];
	SDL_Log( "Switch %s stream from #%d to #%d\n",
			av_get_media_type_string(codec_type),
			old_index,
			stream_index);

	stream_component_close(video, old_index);
	stream_component_open(video, stream_index);
}



static void seek_chapter(Video *video, int incr)
{/*{{{*/
	int64_t pos = get_clock(&video->extclk) * AV_TIME_BASE;
	int i;

	if (!video->avformatctx->nb_chapters)
		return;

	/* find the current chapter */
	for (i = 0; i < video->avformatctx->nb_chapters; i++) {
		AVChapter *ch = video->avformatctx->chapters[i];
		if (av_compare_ts(pos, AV_TIME_BASE_Q, ch->start, ch->time_base) < 0) {
			i--;
			break;
		}
	}

	i += incr;
	i = FFMAX(i, 0);
	if (i >= video->avformatctx->nb_chapters)
		return;

	SDL_Log("Seeking to chapter %d.\n", i);
	Video_seek(video, av_rescale_q(video->avformatctx->chapters[i]->start, video->avformatctx->chapters[i]->time_base,
				AV_TIME_BASE_Q), 0, 0);
}/*}}}*/

static void onKeydown(SpriteEvent * e)
{/*{{{*/
	double incr, pos ;
	SDL_Event * event = e->e;
	Video * video = e->target->obj;
	switch (event->key.keysym.sym) {
		case SDLK_p:
		case SDLK_SPACE:
			Video_pause(video);
			break;
		case SDLK_m:
			toggle_mute(video);
			break;
		case SDLK_KP_MULTIPLY:
		case SDLK_0:
			update_volume(video, 1, SDL_VOLUME_STEP);
			break;
		case SDLK_KP_DIVIDE:
		case SDLK_9:
			update_volume(video, -1, SDL_VOLUME_STEP);
			break;
		case SDLK_s: // S: Step to next frame
			Video_nextFrame(video);
			break;
		case SDLK_a:
			stream_cycle_channel(video, AVMEDIA_TYPE_AUDIO);
			break;
		case SDLK_v:
			stream_cycle_channel(video, AVMEDIA_TYPE_VIDEO);
			break;
		case SDLK_c:
			stream_cycle_channel(video, AVMEDIA_TYPE_VIDEO);
			stream_cycle_channel(video, AVMEDIA_TYPE_AUDIO);
			break;
		case SDLK_PAGEUP:
			if (video->avformatctx->nb_chapters <= 1) {
				incr = 600.0;
				goto do_seek;
			}
			seek_chapter(video, 1);
			break;
		case SDLK_PAGEDOWN:
			if (video->avformatctx->nb_chapters <= 1) {
				incr = -600.0;
				goto do_seek;
			}
			seek_chapter(video, -1);
			break;
		case SDLK_LEFT:
			incr = -10.0;
			goto do_seek;
		case SDLK_RIGHT:
			incr = 10.0;
			goto do_seek;
		case SDLK_UP:
			incr = 60.0;
			goto do_seek;
		case SDLK_DOWN:
			incr = -60.0;
do_seek:
			if (video->seek_by_bytes) {
				pos = -1;
				if (pos < 0 && video->video_stream >= 0)
					pos = frame_queue_last_pos(&video->pictq);
				if (pos < 0 && video->audio_stream >= 0)
					pos = frame_queue_last_pos(&video->sampq);
				if (pos < 0)
					pos = avio_tell(video->avformatctx->pb);
				if (video->avformatctx->bit_rate)
					incr *= video->avformatctx->bit_rate / 8.0;
				else
					incr *= 180000.0;
				pos += incr;
				Video_seek(video, pos, incr, 1);
			} else {
				pos = get_clock(&video->extclk);
				if (isnan(pos))
					pos = (double)video->seek_pos / AV_TIME_BASE;
				pos += incr;
				if (video->avformatctx->start_time != AV_NOPTS_VALUE && pos < video->avformatctx->start_time / (double)AV_TIME_BASE)
					pos = video->avformatctx->start_time / (double)AV_TIME_BASE;
				Video_seek(video, (int64_t)(pos * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
			}
			break;
	}
}/*}}}*/
static void onMouseMove(SpriteEvent * e)
{/*{{{*/
	double x;
	double frac;
	SDL_Event * event = e->e;
	Video * video = e->target->obj;
	if (event->type == SDL_MOUSEBUTTONDOWN) {
		x = event->button.x;
	} else {
		if (event->motion.state != SDL_PRESSED)
			return;
		x = event->motion.x;
	}
	if (video->seek_by_bytes || video->avformatctx->duration <= 0) {
		uint64_t size =  avio_size(video->avformatctx->pb);
		Video_seek(video, size*x/video->w, 0, 1);
	} else {
		int64_t ts;
		int ns, hh, mm, ss;
		int tns, thh, tmm, tss;
		tns  = video->avformatctx->duration / 1000000LL;
		thh  = tns / 3600;
		tmm  = (tns % 3600) / 60;
		tss  = (tns % 60);
		frac = x / video->w;
		ns   = frac * tns;
		hh   = ns / 3600;
		mm   = (ns % 3600) / 60;
		ss   = (ns % 60);
		SDL_Log( "Seek to %2.0f%% (%2d:%02d:%02d) of total duration (%2d:%02d:%02d)       \n", frac*100,
				hh, mm, ss, thh, tmm, tss);
		ts = frac * video->avformatctx->duration;
		if (video->avformatctx->start_time != AV_NOPTS_VALUE)
			ts += video->avformatctx->start_time;
		Video_seek(video, ts, 0, 0);
	}
}/*}}}*/

static Uint32 enterFrame(Uint32 interval, void *param) 
{/*{{{*/
	Video * video = param;
	double remaining_time = REFRESH_RATE;
	if (video->show_mode != SHOW_MODE_NONE && (!video->paused || video->force_refresh))
		video_refresh(video, &remaining_time);
	return 1.0/REFRESH_RATE;
}/*}}}*/

static void show(Video * video)
{/*{{{*/
	video->timerId = SDL_AddTimer((Uint32)(1.0/REFRESH_RATE), enterFrame, video);
}/*}}}*/



Video*Video_new(const char *filename,const char *ifmt)
{
	Video *video;
	video = av_mallocz(sizeof(Video));
	if (!video)
		return NULL;
	av_strlcpy(video->filename, filename, sizeof(video->filename));
	video->sprite = Sprite_new();
	video->sprite->obj = video;
	if(ifmt)
		video->iformat =av_find_input_format(ifmt);

	av_init_packet(&null_pkt);
	null_pkt.data = (uint8_t *)&null_pkt;

	/* start video display */
	if (frame_queue_init(&video->pictq, &video->videoq, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0)
		goto fail;
	if (frame_queue_init(&video->sampq, &video->audioq, SAMPLE_QUEUE_SIZE, 1) < 0)
		goto fail;

	packet_queue_init(&video->videoq);
	packet_queue_init(&video->audioq);

	video->continue_read_thread = SDL_CreateCond();

	init_clock(&video->vidclk, &video->videoq.serialno);
	init_clock(&video->audclk, &video->audioq.serialno);
	init_clock(&video->extclk, &video->extclk.packet_index);

	video->audio_clock_serial = -1;
	video->audio_volume = SDL_MIX_MAXVOLUME;
	video->muted = 0;
	video->read_tid = SDL_CreateThread(read_thread,NULL, video);
	if (!video->read_tid) {
fail:
		Video_clear(video);
		return NULL;
	}
	Sprite_addEventListener(video->sprite,SDL_KEYDOWN,onKeydown);
	Sprite_addEventListener(video->sprite,SDL_MOUSEMOTION,onMouseMove);
	show(video);
	return video;
}

#ifdef test_video
/* Called from the main */
int main(int argc, char **argv)
{
	av_register_all();
	avformat_network_init();
#if CONFIG_AVDEVICE
	avdevice_register_all();
#endif
	char * input_filename = argv[1];
	if (!input_filename) {
#ifdef __ANDROID__
		input_filename = "/sdcard/a.flv";
#else
		input_filename = "a.flv";
#endif
	}

	Stage_init();

	Video * video = Video_new(input_filename,NULL);
	//Video * video = Video_new("/dev/video0","video4linux2");
	//Video * video = Video_new(":0.0","x11grab");
	Sprite_addChild(stage->sprite,video->sprite);
	Stage_loopEvents();
	exit(0);
}
#endif
