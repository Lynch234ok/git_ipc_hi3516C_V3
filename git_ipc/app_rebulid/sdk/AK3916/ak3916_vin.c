
#include "ak3916.h"
#include "sdk/sdk_api.h"
#include "sdk_trace.h"
#include "sdk_common.h"

#define kSDK_VIN_MAGIC (0xf0f0f0f0)

#if defined(HI3518A)|defined(HI3518C)|defined(HI3516C)
# define HI_VIN_CH_BACKLOG_REF (1)
#else
# define HI_VIN_CH_BACKLOG_REF (16)
#endif

#define HI_VIN_MD_RECT_BACKLOG_REF (8)

//#define HI_VIN_COVER_BACKLOG_REF (COVER_MAX_NUM)
#define HI_VIN_COVER_BACKLOG_REF (4)
//#define HI_VIN_COVER_HANDLE_OFFSET (RGN_HANDLE_MAX - 256 - HI_VIN_CH_BACKLOG_REF * HI_VIN_COVER_BACKLOG_REF) // 0 - 1024
#define HI_VIN_COVER_HANDLE_OFFSET (0) // 0 - 1024

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


static int focus_measure_vollath5(const uint8_t *yuv420, size_t width, size_t height)
{
	int i = 0, ii = 0;
	// calc the y mean
	int measure_val = 0;
	int y_sum = 0;
	int y_mean = 0;
	int y_square_sum = 0;
	for(i = 0; i < height; ++i){
		for(ii = 0; ii < width - 1; ++ii){
			const uint8_t *y0_offset = yuv420 + i * width + ii;
			const uint8_t *y1_offset = yuv420 + i * width + ii + 1;
			// y sum for mean calculation
			y_sum += (int)(*y0_offset);
			// y square sum
			y_square_sum += (int)(*y0_offset) * (int)(*y1_offset);
		}
	}
	y_mean = y_sum / i * ii;

	measure_val = y_square_sum - i * i * y_mean * y_mean;
	return measure_val;
}


static int focus_measure_squared_gradient(const uint8_t *yuv420, size_t width, size_t height)
{
	int i = 0, ii = 0;
	int measure_val = 0;
	for(i = 0; i < height - 1; ++i){
		for(ii = 0; ii < width; ++ii){
			const uint8_t *y0_offset = yuv420 + (i + 0) * width + ii;
			const uint8_t *y1_offset = yuv420 + (i + 1) * width + ii;
			
			int y0 = (int)(*y0_offset);
			int y1 = (int)(*y1_offset);
			int y_diff = y1 - y0;
			int y_variance = y_diff * y_diff;
			
			measure_val += y_variance;
		}
	}
	return measure_val;
}

static int vin_focus_measure(int vin, enSDK_VIN_FOCUS_MEASURE_ALG alg)
{
	int measure_val = 0;
	//int i = 0, ii = 0;
	if(vin < HI_VIN_CH_BACKLOG_REF){
	}
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
	int i = 0;
	if(vin < HI_VIN_CH_BACKLOG_REF){
	}
	return 0;
}

static int vin_get_capture_attr(int vin, LP_SDK_VIN_CAPTURE_ATTR capture_attr)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
	}
	return -1;
}

static int vin_create_md(int vin, size_t ref_hblock, size_t ref_vblock, int ref_frame)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
	}
	SOC_INFO("MD(%d/%d) create [%d,%d] error!", vin, HI_VIN_CH_BACKLOG_REF, ref_hblock, ref_vblock);
	return -1;
}

static int vin_release_md(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){

	}
	return -1;
}

static int vin_start_md(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
	}
	return -1;
}

static int vin_stop_md(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
	}
	return -1;
}

static void vin_set_md_bitmap_mode(int vin, bool flag)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		md_attr->bitmap_mode = flag;
	}
}


static int vin_set_md_bitmap_threshold(int vin, float threshold)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic
			&& threshold < 1.0){
			md_attr->bitmap.threshold = threshold;
			return 0;
		}
	}
	return -1;
}

static int vin_set_md_bitmap_one_mask(int vin, int x, int y, bool mask_flag)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic
			&& x < md_attr->ref_hblock && y < md_attr->ref_vblock){

			int const mask_offset = y * md_attr->ref_hblock + x;
			int const mask_byte = mask_offset / 8;
			int const mask_bit = mask_offset % 8;

			if(mask_flag){
				md_attr->bitmap.mask_bitflag[mask_byte] |= (1<<mask_bit);
			}else{
				md_attr->bitmap.mask_bitflag[mask_byte] &= ~(1<<mask_bit);
			}
			return 0;
		}
	}
	return -1;
}

// mask_flag length = (ref_hblock x ref_vblock) / 32, bit hot flag
static int vin_set_md_bitmap_mask(int vin, uint8_t *mask_bitflag)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic){
			int const flag_bytes = (md_attr->ref_hblock * md_attr->ref_vblock + 7) / 8; // align to 8bits
			memcpy(md_attr->bitmap.mask_bitflag, mask_bitflag, flag_bytes);
			return 0;
		}
	}
	return -1;
}

static int vin_md_rect_lookup(int vin, const char *name)
{
	int i = 0;
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic){
			for(i = 0; i < HI_VIN_MD_RECT_BACKLOG_REF; ++i){
				lpSDK_VIN_MD_RECT_MODE md_rect = &md_attr->rect[i];
				if(NULL == name){ // find an empty place
					if(!md_rect->occupation){
						return i;
					}
				}else if(md_rect->occupation
					&& strlen(name) == strlen(md_rect->name)
					&& 0 == strcasecmp(name, md_rect->name)){
					return i;
				}
			}
		}
	}
	return -1;
}
	

static int vin_add_md_rect(int vin, const char *name, float x_ratio, float y_ratio, float width_ratio, float height_ratio, float threshold)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic){
			if(NULL != name 
				&& strlen(name) > 0 && strlen(name) < sizeof(md_attr->rect[0].name)
				&& threshold < 1.0){
				int i_rect = vin_md_rect_lookup(vin, name); // check overwrite first
				if(i_rect >= 0){
					// overwrite the old rect
					lpSDK_VIN_MD_RECT_MODE md_rect = &md_attr->rect[i_rect];
					md_rect->x_ratio = x_ratio;
					md_rect->y_ratio = y_ratio;
					md_rect->width_ratio = width_ratio;
					md_rect->height_ratio = height_ratio;
					md_rect->threshold = threshold;
					// keep the old trap
					return 0;
				}else{
					i_rect = vin_md_rect_lookup(vin, NULL); // find the free place
					if(i_rect >= 0){
						// a new rect
						lpSDK_VIN_MD_RECT_MODE md_rect = &md_attr->rect[i_rect];
						md_rect->x_ratio = x_ratio;
						md_rect->y_ratio = y_ratio;
						md_rect->width_ratio = width_ratio;
						md_rect->height_ratio = height_ratio;
						md_rect->threshold = threshold;
						strcpy(md_rect->name, name);
						md_rect->occupation = true;
						return 0;
					}
				}
			}
		}
	}
	return -1;
}

static int vin_del_md_rect(int vin, const char *name)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic
			&& NULL != name){
			int const i_rect = vin_md_rect_lookup(vin, name);
			if(i_rect >= 0){
				lpSDK_VIN_MD_RECT_MODE md_rect = &md_attr->rect[i_rect];
				md_rect->occupation = false;
				return 0;
			}
		}
	}
	return -1;
}

static bool vin_check_md_rect(int vin, const char *name,
	float *x_ratio, float *y_ratio, float *width_ratio, float *height_ratio, float *threshold)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic
			&& NULL != name && strlen(name)){
			int const i_rect = vin_md_rect_lookup(vin, name);
			lpSDK_VIN_MD_RECT_MODE const md_rect = &md_attr->rect[i_rect];
			if(i_rect >= 0){
				if(x_ratio){
					*x_ratio = md_rect->x_ratio;
				}
				if(y_ratio){
					*y_ratio = md_rect->y_ratio;
				}
				if(width_ratio){
					*width_ratio = md_rect->width_ratio;
				}
				if(height_ratio){
					*height_ratio = md_rect->height_ratio;
				}
				if(threshold){
					*threshold = md_rect->threshold;
				}
				return true;
			}
		}
	}
	return false;
}

static void vin_clear_md_rect(int vin)
{
	int i = 0;
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic){
			for(i = 0; i < HI_VIN_MD_RECT_BACKLOG_REF; ++i){
				lpSDK_VIN_MD_RECT_MODE md_rect = &md_attr->rect[i];
				md_rect->occupation = false;
			}
		}
	}
}

static int vin_set_md_rect_threshold(int vin, const char *name, float threshold)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic
			&& NULL != name 
			&& threshold < 1.0){
			int const i_rect = vin_md_rect_lookup(vin, name);
			if(i_rect >= 0){
				lpSDK_VIN_MD_RECT_MODE md_rect = &md_attr->rect[i_rect];
				md_rect->threshold = threshold;
				return 0;
			}
		}
	}
	return -1;
}

static float vin_get_md_rect_threshold(int vin, const char *name)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
	}
	return 1.0;
}

static int vin_set_md_trap(int vin, fSDK_MD_DO_TRAP do_trap)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		md_attr->do_trap = do_trap;
		return 0;
	}
	return -1;
}


static int vin_set_md_ref_frame(int vin, int ref_frame)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
	}
	return -1;
}

static int vin_get_md_ref_frame(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
	}
	return -1;
}

static int vin_set_md_ref_freq(int vin, int freq)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
	}
	return -1;
}

static int vin_get_md_ref_freq(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
	}
	return -1;
}

static float vin_get_md_bitmap_threshold(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic){
			return md_attr->bitmap.threshold;
		}
	}
	return 1.0;
}

static bool vin_get_md_one_mask(int vin, int x, int y)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic
			&& x < md_attr->ref_hblock && y < md_attr->ref_vblock){

			int const mask_offset = y * md_attr->ref_hblock + x;
			int const mask_byte = mask_offset / 8;
			int const mask_bit = mask_offset % 8;
			
			return (md_attr->bitmap.mask_bitflag[mask_byte] & (1<<mask_bit)) ? true : false;
		}
	}
	return false;
}

static const uint8_t *vin_get_md_bitmap_mask(int vin, size_t *ref_hblock, size_t *ref_vblock)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic){

			if(ref_hblock){
				*ref_hblock = md_attr->ref_hblock;
			}
			if(ref_vblock){
				*ref_vblock = md_attr->ref_vblock;
			}
			return md_attr->bitmap.mask_bitflag;
		}
	}
	return NULL;
}

static void vin_clear_md_bitmap_mask(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic){
			int const flag_bytes = (md_attr->ref_hblock * md_attr->ref_vblock + 7) / 8; // align to 8bits
			memset(md_attr->bitmap.mask_bitflag, 0, flag_bytes);
		}
	}
}

static void vin_dump_md(int vin)
{
	int i = 0, ii = 0;
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];

		printf("CH: %d\r\n", vin);
		printf("MODE: %s\r\n", md_attr->bitmap_mode ? "bitmap" : "rect");
		printf("THRESHOLD: %.3f\r\n", md_attr->bitmap.threshold);
		printf("SIZE: %dx%d\r\n", md_attr->ref_hblock, md_attr->ref_vblock);
		if(md_attr->bitmap_mode){
			// bitmap mode
			printf("MASK:\r\n");
			for(i = 0; i < md_attr->ref_vblock; ++i){
				for(ii = 0; ii < md_attr->ref_hblock; ++ii){
					bool const one_mask = sdk_vin->get_md_bitmap_one_mask(vin, ii, i);
					printf("%c", one_mask ? '#' : '.');
				}
				printf("\r\n");
			}
		}else{
			// rect mode
		}
	}
}

static int vin_set_cover(int vin, int id, LP_SDK_VIN_COVER_ATTR cover)
{
	return -1;
}

static int vin_get_cover(int vin, int id, LP_SDK_VIN_COVER_ATTR cover)
{
	int i = 0 ;
	if(vin < HI_VIN_CH_BACKLOG_REF && id < HI_VIN_COVER_BACKLOG_REF){
		LP_SDK_VIN_COVER_ATTR const cover_attr = &_sdk_vin.attr.cover_attr[vin][i];
		if(cover){
			memcpy(cover, cover_attr, sizeof(ST_SDK_VIN_COVER_ATTR));
			return 0;
		}
	}
	return -1;
}


static void md_loop(void *arg)
{
	int i = 0;
	while(_sdk_vin.attr.md_trigger){

		sleep(1);
	}
	pthread_exit(NULL);
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
	int i = 0, ii = 0;
	if(sdk_vin){
		// stop the loop
		sdk_vin->stop();
		// free the video attr
		for(i = 0; i < HI_VIN_CH_BACKLOG_REF; ++i){
			// release all the motion detectoin
			sdk_vin->release_md(i);

			for(ii = 0; ii < HI_VIN_COVER_BACKLOG_REF; ++ii){
				
			}
		}
		sdk_vin = NULL;
		return 0;
	}
	return -1;
}

