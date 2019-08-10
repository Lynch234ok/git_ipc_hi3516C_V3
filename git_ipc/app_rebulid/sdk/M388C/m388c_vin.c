
#include "sdk/sdk_api.h"
#include "sdk_trace.h"
#include "sdk_common.h"
#include <sys/prctl.h>

#define kSDK_VIN_MAGIC (0xf0f0f0f0)

#ifndef HI3518A_VIN_DEV  
#define HI3518A_VIN_DEV (0)
#endif 



#if defined(HI3518A)|defined(HI3518C)|defined(HI3516C)|defined(HI3516A)|defined(HI3516D)
# define HI_VIN_CH_BACKLOG_REF (1)
#else
# define HI_VIN_CH_BACKLOG_REF (16)
#endif

#define HI_VIN_MD_RECT_BACKLOG_REF (8)

#define HI_VIN_COVER_BACKLOG_REF (4)
#define HI_VIN_COVER_HANDLE_OFFSET (RGN_HANDLE_MAX - 256 - HI_VIN_CH_BACKLOG_REF * HI_VIN_COVER_BACKLOG_REF) // 0 - 1024

#define __HI_VIN_COVER_HANDLE(__vin, __blk) (HI_VIN_COVER_HANDLE_OFFSET + __vin * HI_VIN_COVER_BACKLOG_REF + __blk)


typedef struct SDK_VIN_MD_BITMAP_MODE {
	size_t mask_bytes;
	uint8_t *mask_bitflag;
	float threshold;
}stSDK_VIN_MD_BITMAP_MODE, *lpSDK_VIN_MD_BITMAP_MODE;

typedef struct SDK_VIN_MD_RECT_MODE {
	bool occupation;
	char name[32];
	float x_ratio, y_ratio, width_ratio, height_ratio;
	float threshold;
}stSDK_VIN_MD_RECT_MODE, *lpSDK_VIN_MD_RECT_MODE;

typedef struct SDK_VIN_MD_ATTR {
	uint32_t magic; // = kSDK_MD_CH_ATTR_MAGIC
	size_t ref_hblock, ref_vblock;
	// bitmap mode flag, describe using bitmap mode or rect mode
	bool bitmap_mode;
	
	stSDK_VIN_MD_BITMAP_MODE bitmap;
	stSDK_VIN_MD_RECT_MODE rect[HI_VIN_MD_RECT_BACKLOG_REF];
	fSDK_MD_DO_TRAP do_trap;
	
}stSDK_VIN_MD_ATTR, *lpSDK_VIN_MD_ATTR;

/*
typedef struct SDK_VIN_COVER_ATTR {
	uint32_t magic; // = kSDK_MD_CH_ATTR_MAGIC
	char name[32]; // the name of this cover
	float x_ratio, y_ratio, width_ratio, height_ratio;
	uint32_t color;
}stSDK_VIN_COVER_ATTR, *lpSDK_VIN_COVER_ATTR;
*/

typedef struct SDK_VIN_ATTR {
	enSDK_VIN_HW_SPEC hw_spec;
	
	// motion detection
	stSDK_VIN_MD_ATTR md_attr[HI_VIN_CH_BACKLOG_REF];
	uint32_t md_intl;
	bool md_trigger;
	pthread_t md_tid;

	// cover
	ST_SDK_VIN_COVER_ATTR cover_attr[HI_VIN_CH_BACKLOG_REF][HI_VIN_COVER_BACKLOG_REF];
	
}stSDK_VIN_ATTR, *lpSDK_VIN_ATTR;

typedef struct SDK_VIN_HI3521 {
	stSDK_VIN_API api;
	stSDK_VIN_ATTR attr;
}stSDK_VIN_HI3521, *lpSDK_VIN_HI3521;

static stSDK_VIN_HI3521 _sdk_vin;
lpSDK_VIN_API sdk_vin = NULL;


static int vin_focus_measure(int vin, enSDK_VIN_FOCUS_MEASURE_ALG alg)
{
	int measure_val = 0;

	return measure_val;
}



typedef struct BIT_MAP_FILE_HEADER
{
	char type[2]; // "BM" (0x4d42)
    uint32_t file_size;
    uint32_t reserved_zero;
    uint32_t off_bits; // data area offset to the file set (unit. byte)
	uint32_t info_size;
	uint32_t width;
	uint32_t height;
	uint16_t planes; // 0 - 1
	uint16_t bit_count; // 0 - 1
	uint32_t compression; // 0 - 1
	uint32_t size_image; // 0 - 1
	uint32_t xpels_per_meter;
	uint32_t ypels_per_meter;
	uint32_t clr_used;
	uint32_t clr_important;
}__attribute__((packed)) BIT_MAP_FILE_HEADER_t; //

static int vin_capture(int vin, FILE* bitmap_stream)
{
	return 0;
}



static int vin_get_capture_attr(int vin, LP_SDK_VIN_CAPTURE_ATTR capture_attr)
{
	return -1;
}

static int vin_create_md(int vin, size_t ref_hblock, size_t ref_vblock, int ref_frame)
{	
	return -1;
}

static int vin_release_md(int vin)
{
	return -1;
}

static int vin_start_md(int vin)
{
	return -1;
}

static int vin_stop_md(int vin)
{
	return -1;
}

static void vin_set_md_bitmap_mode(int vin, bool flag)
{
}


static int vin_set_md_bitmap_threshold(int vin, float threshold)
{
	return -1;
}

static int vin_set_md_bitmap_one_mask(int vin, int x, int y, bool mask_flag)
{
	return -1;
}

// mask_flag length = (ref_hblock x ref_vblock) / 32, bit hot flag
static int vin_set_md_bitmap_mask(int vin, uint8_t *mask_bitflag)
{
	return -1;
}

static int vin_add_md_rect(int vin, const char *name, float x_ratio, float y_ratio, float width_ratio, float height_ratio, float threshold)
{
	return -1;
}

static int vin_del_md_rect(int vin, const char *name)
{
	return -1;
}

static bool vin_check_md_rect(int vin, const char *name,
	float *x_ratio, float *y_ratio, float *width_ratio, float *height_ratio, float *threshold)
{
	return false;
}

static void vin_clear_md_rect(int vin)
{
}

static int vin_set_md_rect_threshold(int vin, const char *name, float threshold)
{
	return -1;
}

static float vin_get_md_rect_threshold(int vin, const char *name)
{
	return 1.0;
}

static int vin_set_md_trap(int vin, fSDK_MD_DO_TRAP do_trap)
{
	return -1;
}


static int vin_set_md_ref_frame(int vin, int ref_frame)
{
	return -1;
}

static int vin_get_md_ref_frame(int vin)
{
	return -1;
}

static int vin_set_md_ref_freq(int vin, int freq)
{	
	return -1;
}

static int vin_get_md_ref_freq(int vin)
{
	return -1;
}

static float vin_get_md_bitmap_threshold(int vin)
{
	return 1.0;
}

static bool vin_get_md_one_mask(int vin, int x, int y)
{
	return false;
}

static const uint8_t *vin_get_md_bitmap_mask(int vin, size_t *ref_hblock, size_t *ref_vblock)
{
	return NULL;
}

static void vin_clear_md_bitmap_mask(int vin)
{
}

static void vin_dump_md(int vin)
{
}

static int vin_set_cover(int vin, int id, LP_SDK_VIN_COVER_ATTR cover)
{
	return -1;
}

static int vin_get_cover(int vin, int id, LP_SDK_VIN_COVER_ATTR cover)
{
	return -1;
}

static void md_loop(void *arg)
{
	prctl(PR_SET_NAME, "md_loop");
	while(1){
		usleep(10000);
	}
}

static int md_start()
{
	int ret = 0;
	if(!_sdk_vin.attr.md_tid){
		_sdk_vin.attr.md_trigger = true;
		ret = pthread_create(&_sdk_vin.attr.md_tid, NULL, (void*)md_loop, NULL);
		SOC_ASSERT(0 == ret, "MD create thread failed!");
		return 0;
	}
	return -1;
}

static int md_stop()
{
	if(_sdk_vin.attr.md_tid){
		_sdk_vin.attr.md_trigger = false;
		pthread_join(_sdk_vin.attr.md_tid, NULL);
		_sdk_vin.attr.md_tid = (pthread_t)NULL;
		return 0;
	}
	return -1;
}

static int vin_start()
{
	return md_start();
}

static int vin_stop()
{
	return md_stop();
}


static stSDK_VIN_HI3521 _sdk_vin = {
	.api = {
		.focus_measure = vin_focus_measure,
		.capture = vin_capture,

		// capture attributes
		.get_capture_attr = vin_get_capture_attr,

		// motion detection about
		.create_md = vin_create_md,
		.release_md = vin_release_md,

		.start_md = vin_start_md,
		.stop_md = vin_stop_md,

		.set_md_bitmap_mode = vin_set_md_bitmap_mode,

		.set_md_bitmap_threshold = vin_set_md_bitmap_threshold,
		.get_md_bitmap_threshold = vin_get_md_bitmap_threshold,
		.set_md_bitmap_one_mask = vin_set_md_bitmap_one_mask,
		.get_md_bitmap_one_mask = vin_get_md_one_mask,
		.set_md_bitmap_mask = vin_set_md_bitmap_mask,
		.get_md_bitmap_mask = vin_get_md_bitmap_mask,
		.clear_md_bitmap_mask = vin_clear_md_bitmap_mask,
		
		.add_md_rect = vin_add_md_rect,
		.del_md_rect = vin_del_md_rect,
		.check_md_rect = vin_check_md_rect,
		.clear_md_rect = vin_clear_md_rect,
		.set_md_rect_threshold = vin_set_md_rect_threshold,
		.get_md_rect_threshold = vin_get_md_rect_threshold,
		
		.set_md_trap = vin_set_md_trap,
		
		.set_md_ref_frame = vin_set_md_ref_frame,
		.get_md_ref_frame = vin_get_md_ref_frame,
		.set_md_ref_freq = vin_set_md_ref_freq,
		.get_md_ref_freq = vin_get_md_ref_freq,
		.dump_md = vin_dump_md,

		.set_cover = vin_set_cover,
		.get_cover = vin_get_cover,

		.start = vin_start,
		.stop = vin_stop,

		
	},
};

int SDK_init_vin(enSDK_VIN_HW_SPEC hw_spec)
{
	int i = 0;
	if(NULL == sdk_vin){
		// hardware spec
		_sdk_vin.attr.hw_spec = hw_spec;
		
		// clear
		memset(&_sdk_vin.attr, 0, sizeof(_sdk_vin.attr));
		// init the handle pointer
		sdk_vin = (lpSDK_VIN_API)(&_sdk_vin);
		// setup
		for(i = 0; i < HI_VIN_CH_BACKLOG_REF; ++i){
			sdk_vin->set_md_bitmap_mode(i, true); // default use the bitmap mode
		}
		return 0;
	}
	return -1;
}

int SDK_destroy_vin()
{
	int i;
	if(sdk_vin){
		// stop the loop
		sdk_vin->stop();
		// free the video attr
		for(i = 0; i < HI_VIN_CH_BACKLOG_REF; ++i){
			// release all the motion detectoin
			sdk_vin->release_md(i);

		}
		sdk_vin = NULL;
		return 0;
	}
	return -1;
}

