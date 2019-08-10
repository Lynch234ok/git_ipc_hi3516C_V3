
#include "ak3916.h"
#include "sdk/sdk_api.h"
#include "sdk_trace.h"
#include "sdk_common.h"

#if defined(HI3518A) | defined(HI3518C) | defined(HI3516C)
static int _hi3518_vpss_ch_map[] = {
	//VPSS_BSTR_CHN, // for main stream encode
	VPSS_LSTR_CHN, // for main stream encode
	3,
	4,
	5,
	6,
	7,
	VPSS_BYPASS_CHN,
	VPSS_BSTR_CHN, // for snapshot
};
#endif

#if defined(HI3518A) | defined(HI3518C) | defined(HI3516C)
# define HI_VENC_H264_REF_MODE (H264E_REF_MODE_1X)
#elif defined(HI3531)
# define HI_VENC_H264_REF_MODE (H264E_REF_MODE_2X) // because 3531 is lack of memory
#else
# define HI_VENC_H264_REF_MODE (H264E_REF_MODE_4X)
#endif

#if defined(HI3518A)|defined(HI3518C)|defined(HI3516C)
# define HI_VENC_CH_BACKLOG_REF (1)
# define HI_AENC_CH_BACKLOG_REF (1)
# define HI_VENC_STREAM_BACKLOG_REF (3)
#else
# define HI_VENC_CH_BACKLOG_REF (16)
# define HI_AENC_CH_BACKLOG_REF (16)
# define HI_VENC_STREAM_BACKLOG_REF (2)
#endif

#define HI_VENC_JPEG_MIN_WIDTH (64)
#define HI_VENC_JPEG_MAX_WIDTH (8192)
#define HI_VENC_JPEG_MIN_HEIGHT (64)
#define HI_VENC_JPEG_MAX_HEIGHT (8192)

#define HI_VENC_OVERLAY_BACKLOG_REF (4) //(OVERLAY_MAX_NUM)
#define HI_VENC_OVERLAY_CANVAS_STOCK_REF (HI_VENC_CH_BACKLOG_REF * HI_VENC_STREAM_BACKLOG_REF * HI_VENC_OVERLAY_BACKLOG_REF)
#define HI_VENC_OVERLAY_HANDLE_OFFSET (RGN_HANDLE_MAX - HI_VENC_OVERLAY_CANVAS_STOCK_REF) // 0 - 1024

#define __HI_VENC_CH(_vin, _stream) ((_vin)* HI_VENC_STREAM_BACKLOG_REF + (_stream))

#define HI_ENC_ATTR_MAGIC (0xf0f0f0f0)

typedef struct SDK_ENC_VIDEO_OVERLAY_ATTR {
	LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas;
	char name[32];
	int x, y;
	size_t width, height;
	//RGN_HANDLE region_handle; 
}stSDK_ENC_VIDEO_OVERLAY_ATTR, *lpSDK_ENC_VIDEO_OVERLAY_ATTR;

typedef struct SDK_ENC_VIDEO_OVERLAY_ATTR_SET {
	stSDK_ENC_VIDEO_OVERLAY_ATTR attr[HI_VENC_OVERLAY_BACKLOG_REF];
}stSDK_ENC_VIDEO_OVERLAY_ATTR_SET, *lpSDK_ENC_VIDEO_OVERLAY_ATTR_SET;

typedef struct SDK_ENC_ATTR {
	ST_SDK_ENC_STREAM_H264_ATTR h264_attr[HI_VENC_CH_BACKLOG_REF][HI_VENC_STREAM_BACKLOG_REF];
	uint8_t frame_ref_counter[HI_VENC_CH_BACKLOG_REF][HI_VENC_STREAM_BACKLOG_REF];	
	ST_SDK_ENC_STREAM_G711A_ATTR g711_attr[HI_AENC_CH_BACKLOG_REF];
	
	stSDK_ENC_VIDEO_OVERLAY_ATTR_SET video_overlay_set[HI_VENC_CH_BACKLOG_REF][HI_VENC_STREAM_BACKLOG_REF];
	ST_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas_stock[HI_VENC_OVERLAY_CANVAS_STOCK_REF];

	bool loop_trigger;
	pthread_t loop_tid;
	pthread_mutex_t snapshot_mutex;
}stSDK_ENC_ATTR, *lpSDK_ENC_ATTR;

#define STREAM_H264_ISSET(__stream_h264) (strlen((__stream_h264)->name) > 0)
#define STREAM_H264_CLEAR(__stream_h264) do{ memset((__stream_h264)->name, 0, sizeof((__stream_h264)->name)); }while(0)


typedef struct SDK_ENC_AK3916
{
	stSDK_ENC_API api;
	stSDK_ENC_ATTR attr;
}stSDK_ENC_AK3916, *lpSDK_ENC_AK3916;
static stSDK_ENC_AK3916 _sdk_enc;
lpSDK_ENC_API sdk_enc = NULL;

static inline uint64_t get_time_us()
{
	uint64_t time_us = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_us = tv.tv_sec;
	time_us <<= 32;
	time_us += tv.tv_usec;
	return time_us;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

static int do_buffer_request(int buf_id, ssize_t data_sz, uint32_t keyflag)
{
	if(sdk_enc->do_buffer_request){
		return sdk_enc->do_buffer_request(buf_id, data_sz, keyflag);
	}
	return -1;
}
static int do_buffer_append(int buf_id, const void* item, ssize_t item_sz)
{
	if(sdk_enc->do_buffer_append){
		return sdk_enc->do_buffer_append(buf_id, item, item_sz);
	}
	return -1;
}
static int do_buffer_commit(int buf_id)
{
	if(sdk_enc->do_buffer_commit){
		return sdk_enc->do_buffer_commit(buf_id);
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

static int enc_lookup_stream_byname(const char* name, int* ret_vin, int* ret_stream)
{
	int i = 0, ii = 0;
	if(NULL != name && strlen(name)){
		for(i = 0; i < HI_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < HI_VENC_STREAM_BACKLOG_REF; ++ii){
				LP_SDK_ENC_STREAM_H264_ATTR h264_attr = &_sdk_enc.attr.h264_attr[i][ii];
				
				if(STREAM_H264_ISSET(h264_attr)
					&& 0 == strcasecmp(h264_attr->name, name)
					&& strlen(h264_attr->name) == strlen(name)){

					if(ret_vin){
						*ret_vin = i;
					}
					if(ret_stream){
						*ret_stream = ii;
					}
					return 0;
				}
			}
		}
	}
	return -1;
}

static int enc_create_stream_h264(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr)
{
	if(vin < HI_VENC_CH_BACKLOG_REF && stream < HI_VENC_STREAM_BACKLOG_REF
		&& NULL != h264_attr && STREAM_H264_ISSET(h264_attr)){
		
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
			
		if(0 == enc_lookup_stream_byname(h264_attr->name, NULL, NULL)){
			// the name is overlap
			return -1;
		}
		
		if(!STREAM_H264_ISSET(streamH264Attr)){

		}
	}
	return -1;
}

static int enc_release_stream_h264(int vin, int stream)
{
	LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
	if(vin < HI_VENC_CH_BACKLOG_REF && stream < HI_VENC_STREAM_BACKLOG_REF){
		if(STREAM_H264_ISSET(streamH264Attr)){

		}
	}
	return -1;
}

static int enc_set_stream_h264(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr)
{
	if(vin < HI_VENC_CH_BACKLOG_REF && stream < HI_VENC_STREAM_BACKLOG_REF
		&& NULL != h264_attr && STREAM_H264_ISSET(h264_attr)){
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];

		if(STREAM_H264_ISSET(streamH264Attr)){
			if(1){//if(streamH264Attr->width != h264_attr->width || streamH264Attr->height != h264_attr->height){

				// width / height adjust need to restart the stream
				sdk_enc->release_stream_h264(vin, stream);
				sdk_enc->create_stream_h264(vin, stream, h264_attr);
				sdk_enc->enable_stream_h264(vin, stream, true);
				return 0;
			}else{

			}
			return 0;
		}
	}
	return -1;
}


static int enc_get_stream_h264(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr)
{
	if(vin < HI_VENC_CH_BACKLOG_REF && stream < HI_VENC_STREAM_BACKLOG_REF
		&& NULL != h264_attr){
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
		if(STREAM_H264_ISSET(streamH264Attr)){
			if(NULL != h264_attr){
				memcpy(h264_attr, streamH264Attr, sizeof(ST_SDK_ENC_STREAM_H264_ATTR));
			}
			return 0;
		}
	}
	return -1;
}

static int enc_create_stream_g711a(int ain, int vin_ref)
{
	if(ain < HI_AENC_CH_BACKLOG_REF){
		if(vin_ref < HI_VENC_CH_BACKLOG_REF){
		}
	}
	return -1;
}

static int enc_release_stream_g711a(int ain)
{
	if(ain < HI_AENC_CH_BACKLOG_REF){
	}
	return -1;
}

static int enc_enable_stream_h264(int vin, int stream, bool flag)
{
	LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
	if(STREAM_H264_ISSET(streamH264Attr)){
	}
	return -1;
}

static int enc_request_stream_h264_keyframe(int vin, int stream)
{
	return -1;
}


static int enc_video_proc(int vin, int stream)
{

	LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &(_sdk_enc.attr.h264_attr[vin][stream]);
	uint8_t *const ref_counter = &(_sdk_enc.attr.frame_ref_counter[vin][stream]);
	int const venc_ch = vin * HI_VENC_STREAM_BACKLOG_REF + stream;

	if(STREAM_H264_ISSET(streamH264Attr)){
	}

	return -1;
}

static int enc_audio_proc(int ain)
{
	return -1;
}


static void* enc_loop(void* arg)
{
	int i = 0, ii = 0;
	
	while(_sdk_enc.attr.loop_trigger){
		// audio stream
		for(i = 0; i < HI_AENC_CH_BACKLOG_REF; ++i){
			while(0 == enc_audio_proc(i));
		}
		// main stream first
		for(i = 0; i < HI_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < HI_VENC_STREAM_BACKLOG_REF; ++ii){
				while(0 == enc_video_proc(i, ii));
			}
		}
		//usleep(30000);
		usleep(10000);
	}
	pthread_exit(NULL);
}

static int enc_start()
{
	if(!_sdk_enc.attr.loop_tid){
		int ret = 0;
		_sdk_enc.attr.loop_trigger = true;
		ret = pthread_create(&_sdk_enc.attr.loop_tid, NULL, enc_loop, NULL);
		SOC_ASSERT(0 == ret, "AV encode do loop create thread failed!");
		return 0;
	}
	return -1;
}

static int enc_stop()
{
	if(_sdk_enc.attr.loop_tid){
		_sdk_enc.attr.loop_trigger = false;
		pthread_join(_sdk_enc.attr.loop_tid, NULL);
		_sdk_enc.attr.loop_tid = (pthread_t)NULL;
		return 0;
	}
	return -1;
}

static int enc_snapshot(int vin, enSDK_ENC_SNAPSHOT_QUALITY quality, ssize_t width, ssize_t height, FILE* stream)
{
	if(vin < HI_VENC_CH_BACKLOG_REF){

	}
	return -1;
}

static inline uint16_t overlay_pixel_argb4444(stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel)
{
	return 0;
}

static inline stSDK_ENC_VIDEO_OVERLAY_PIXEL overlay_pixel_argb8888(uint16_t pixel)
{
	stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel_8888;
	return pixel_8888;
}


static int overlay_canvas_put_pixel(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas, int x, int y, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel)
{
	if(canvas){
		if(x < canvas->width && y < canvas->height){
			if(NULL != canvas->pixels){
				uint16_t *const pixels = (uint16_t*)canvas->pixels;
				*(pixels + y * canvas->width + x) = overlay_pixel_argb4444(pixel);
				return 0;
			}
		}
	}
	return -1;
}

static int overlay_canvas_get_pixel(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas, int x, int y, stSDK_ENC_VIDEO_OVERLAY_PIXEL* ret_pixel)
{
	if(canvas){
		if(x < canvas->width && y < canvas->height){
			if(NULL != canvas->pixels){
				uint16_t *const pixels = (uint16_t*)canvas->pixels;
				if(ret_pixel){
					*ret_pixel = overlay_pixel_argb8888(*(pixels + y * canvas->width + x));
					return 0;
				}
			}
		}
	}
	return -1;
}

static bool overlay_canvas_match_pixel(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel1, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel2)
{
	return overlay_pixel_argb4444(pixel1) == overlay_pixel_argb4444(pixel2);
}

static int overlay_canvas_put_rect(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas, int x, int y, size_t width, size_t height,stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel)
{
	if(canvas){
		if(x < canvas->width && y < canvas->height){
			if(NULL != canvas->pixels){
				int i, ii;
				uint16_t *const pixels = (uint16_t*)(canvas->pixels);
				uint16_t const pixel_4444 = overlay_pixel_argb4444(pixel);
				
				if(x + width >= canvas->width){
					width = canvas->width - x;
				}
				if(y + height >= canvas->height){
					height = canvas->height - y;
				}
				
				for(i = 0; i < height; ++i){
					uint16_t* pixel_pos = pixels + i * canvas->width;
					if(0 == i || height - 1 == i){
						// put one line
						for(ii = 0; ii < width; ++ii){
							*pixel_pos++ = pixel_4444;
						}
					}else{
						// put 2 dots
						pixel_pos[0] = pixel_pos[width - 1] = pixel_4444;
					}
				}
				return 0;
			}
		}
	}
	return -1;
}

static int overlay_canvas_fill_rect(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas, int x, int y, size_t width, size_t height, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel)
{
	if(canvas){
		if(!width){
			width = canvas->width;
		}
		if(!height){
			height = canvas->height;
		}
		
		if(x < canvas->width && y < canvas->height){
			if(NULL != canvas->pixels){	
				int i, ii;
				uint16_t *const pixels = (uint16_t*)(canvas->pixels);
				uint16_t const pixel_4444 = overlay_pixel_argb4444(pixel);
				
				if(x + width >= canvas->width){
					width = canvas->width - x;
				}
				if(y + height >= canvas->height){
					height = canvas->height - y;
				}
				
				for(i = 0; i < height; ++i){
					uint16_t* pixel_pos = pixels + i * canvas->width;
					for(ii = 0; ii < width; ++ii){
						*pixel_pos++ = pixel_4444;
					}
				}
				return 0;
			}
		}
	}
	return -1;
}

static int overlay_canvas_erase_rect(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas, int x, int y, size_t width, size_t height)
{
	stSDK_ENC_VIDEO_OVERLAY_PIXEL erase_pixel;
	erase_pixel.alpha = 0;
	erase_pixel.red = 0;
	erase_pixel.green = 0;
	erase_pixel.blue = 0;
	return canvas->fill_rect(canvas, x, y, width, height, erase_pixel);
}


static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS enc_create_overlay_canvas(size_t width, size_t height)
{
	int i = 0;
	if(width > 0 && height > 0){
		LP_SDK_ENC_VIDEO_OVERLAY_CANVAS const canvas_stock = _sdk_enc.attr.canvas_stock;
		for(i = 0; i < HI_VENC_OVERLAY_CANVAS_STOCK_REF; ++i){
			LP_SDK_ENC_VIDEO_OVERLAY_CANVAS const canvas = canvas_stock + i;
			if(!canvas->pixels){
				
				return canvas;
			}
		}
	}
	return NULL;
}

static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS enc_load_overlay_canvas(const char *bmp24_path)
{
	return NULL;
}


static void enc_release_overlay_canvas(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas)
{
	if(canvas){
		canvas->width = 0;
		canvas->height = 0;
		if(canvas->pixels){
			free(canvas->pixels);
			canvas->pixels = NULL; // very important
		}
		// baccaus the canvas is created from the stock
		// so it's needless to be free
	}
}

static lpSDK_ENC_VIDEO_OVERLAY_ATTR enc_lookup_overlay_byname(int vin, int stream, const char* name)
{
	int i = 0;
	if(vin < HI_VENC_CH_BACKLOG_REF
		&& stream < HI_VENC_STREAM_BACKLOG_REF){
		lpSDK_ENC_VIDEO_OVERLAY_ATTR_SET const overlay_set = &_sdk_enc.attr.video_overlay_set[vin][stream];
		// check name override
		for(i = 0; i < HI_VENC_OVERLAY_BACKLOG_REF; ++i){
			lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = &overlay_set->attr[i];
			//SOC_DEBUG("Looking up \"%s\"/\"%s\"", name, overlay->name);
			if(overlay->canvas && 0 == strcmp(overlay->name, name)){
				// what's my target
				return overlay;
			}
		}
	}
	return NULL;
}
	

static int enc_create_overlay(int vin, int stream, const char* overlay_name,
		float x, float y, LP_SDK_ENC_VIDEO_OVERLAY_CANVAS const canvas)
{
	int i = 0;
	if(NULL != canvas
		&& vin < HI_VENC_CH_BACKLOG_REF
		&& stream < HI_VENC_STREAM_BACKLOG_REF){

	}
	return -1;
}

static int enc_release_overlay(int vin, int stream, const char* overlay_name)
{
	if(vin < HI_VENC_CH_BACKLOG_REF
		&& stream < HI_VENC_STREAM_BACKLOG_REF){
		lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = enc_lookup_overlay_byname(vin, stream, overlay_name);
		if(NULL != overlay){

		}
	}
	return -1;
}

static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS enc_get_overlay_canvas(int vin, int stream, const char* overlay_name)
{
	lpSDK_ENC_VIDEO_OVERLAY_ATTR overlay = enc_lookup_overlay_byname(vin, stream, overlay_name);
	if(overlay){
		return overlay->canvas;
	}
	return NULL;
}

static int enc_show_overlay(int vin, int stream, const char* overlayName, bool showFlag)
{
	if(vin < HI_VENC_CH_BACKLOG_REF
		&& stream < HI_VENC_STREAM_BACKLOG_REF){
		lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = enc_lookup_overlay_byname(vin, stream, overlayName);
		if(NULL != overlay){
		}
	}
	return -1;
}

static int enc_update_overlay(int vin, int stream, const char* overlay_name)
{
	if(vin < HI_VENC_CH_BACKLOG_REF
		&& stream < HI_VENC_STREAM_BACKLOG_REF){
		lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = enc_lookup_overlay_byname(vin, stream, overlay_name);
		if(NULL != overlay){
		}
	}
	return -1;
}

static stSDK_ENC_AK3916 _sdk_enc =
{
	// init the interfaces
	.api = {
		// h264 stream
		.create_stream_h264 = enc_create_stream_h264,
		.release_stream_h264 = enc_release_stream_h264,
		.enable_stream_h264 = enc_enable_stream_h264,
		.set_stream_h264 = enc_set_stream_h264,
		.get_stream_h264 = enc_get_stream_h264,
		.request_stream_h264_keyframe = enc_request_stream_h264_keyframe,
		
		.create_stream_g711a = enc_create_stream_g711a,
		.release_stream_g711a = enc_release_stream_g711a,

		// snapshot a picture
		.snapshot = enc_snapshot,
		// overlay
		.create_overlay_canvas = enc_create_overlay_canvas,
		.load_overlay_canvas = enc_load_overlay_canvas,
		.release_overlay_canvas = enc_release_overlay_canvas,
		.create_overlay = enc_create_overlay,
		.release_overlay = enc_release_overlay,
		.get_overlay_canvas = enc_get_overlay_canvas,
		.show_overlay = enc_show_overlay,
		.update_overlay = enc_update_overlay,

		// encode start / stop
		.start = enc_start,
		.stop = enc_stop,
	},
};


int SDK_ENC_init()
{
	int i = 0, ii = 0, iii = 0;
	// only 'sdk_enc' pointer is NULL could be init
	if(NULL == sdk_enc){
		// set handler pointer
		sdk_enc = (lpSDK_ENC_API)(&_sdk_enc);
		
		// clear the buffering callback
		sdk_enc->do_buffer_request = NULL;
		sdk_enc->do_buffer_append = NULL;
		sdk_enc->do_buffer_commit = NULL;
		
		// init the internal attribute value
		// clear the stream attrubutes
		// clear the frame counter
		for(i = 0; i < HI_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < HI_VENC_STREAM_BACKLOG_REF; ++ii){
				LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[i][ii];
				uint8_t *const frame_ref_counter = &_sdk_enc.attr.frame_ref_counter[i][ii];

				STREAM_H264_CLEAR(streamH264Attr);
				*frame_ref_counter = 0;
			}
		}
		
		// init the snapshot mutex
		pthread_mutex_init(&_sdk_enc.attr.snapshot_mutex, NULL);
		// start
		//sdk_enc->start();
		// success to init
		return 0;
	}
	return -1;
}

int SDK_ENC_destroy()
{
	if(sdk_enc){
		int i = 0, ii = 0;
		// destroy the snapshot mutex
		pthread_mutex_destroy(&_sdk_enc.attr.snapshot_mutex);
		// stop encode firstly
		sdk_enc->stop();
		// release the canvas stock
		for(i = 0; i < HI_VENC_OVERLAY_CANVAS_STOCK_REF; ++i){
			sdk_enc->release_overlay_canvas(_sdk_enc.attr.canvas_stock + i);
		}
		
		// release the audio encode
		for(i = 0; i < HI_AENC_CH_BACKLOG_REF; ++i){
			sdk_enc->release_stream_g711a(i);
		}
		// release the video encode
		for(i = 0; i < HI_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < HI_VENC_STREAM_BACKLOG_REF; ++ii){
				// destroy sub stream firstly
				sdk_enc->release_stream_h264(i, ii);
			}
		}
		// clear handler pointer
		sdk_enc = NULL;
		// success to destroy
		return 0;
	}
	return -1;
}

