
#include "hi3521.h"
#include "sdk/sdk_api.h"
#include "impower/imp_avd_para.h"
#include "impower/imp_algo_type.h"
#include "impower/imp_avd_api.h"
#include "sdk_trace.h"
#include "sdk_common.h"
#include <sys/prctl.h>

#define kSDK_VIN_MAGIC (0xf0f0f0f0)

#if defined(HI3518A)|defined(HI3518C)|defined(HI3516C)|defined(HI3518E)
# define HI_VIN_CH_BACKLOG_REF (1)
#else
# define HI_VIN_CH_BACKLOG_REF (16)
#endif

#define HI_VIN_MD_RECT_BACKLOG_REF (8)

#define HI_VIN_COVER_BACKLOG_REF (COVER_MAX_NUM)
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

	//vi tend
	bool vi_tend_trigger;
	pthread_t vi_tend_tid;
	uint32_t old_int_cnt;
	uint32_t vi_tend_retry_cnt;
	
	
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
		int frame_depth = 0;
		VIDEO_FRAME_INFO_S video_frame_info;
		
		SOC_CHECK(HI_MPI_VI_GetFrameDepth((VI_CHN)vin, (HI_U32*)&frame_depth));
		if(0 == frame_depth){
			HI_MPI_VI_SetFrameDepth(vin, 1);
		}

		if(HI_SUCCESS == HI_MPI_VI_GetFrame(vin, &video_frame_info)){
			if(PIXEL_FORMAT_YUV_SEMIPLANAR_420 == video_frame_info.stVFrame.enPixelFormat){
				
				int const width = video_frame_info.stVFrame.u32Width;
				int const height = video_frame_info.stVFrame.u32Height;
				size_t const stride = video_frame_info.stVFrame.u32Stride[0];
				size_t const yuv420_size = stride * height * 3 / 2;
				const uint8_t *yuv420_data = HI_MPI_SYS_Mmap(video_frame_info.stVFrame.u32PhyAddr[0], yuv420_size);

				if(NULL != yuv420_data){
					// sharpness function
					switch(alg){
						default:
						case kSDK_VIN_FOCUS_MEASURE_ALG_AUTO:
						case kSDK_VIN_FOCUS_MEASURE_ALG_SQUARED_GRADIENT:
							measure_val = focus_measure_squared_gradient(yuv420_data, width, height);
							break;
						case kSDK_VIN_FOCUS_MEASURE_ALG_VOLLATH5:
							measure_val = focus_measure_vollath5(yuv420_data, width, height);
							break;
					}
					SOC_CHECK(HI_MPI_SYS_Munmap((HI_VOID*)yuv420_data, (HI_U32)yuv420_size));
				}
			}
			SOC_CHECK(HI_MPI_VI_ReleaseFrame(vin, &video_frame_info));
		}
		SOC_CHECK(HI_MPI_VI_SetFrameDepth(vin, frame_depth));
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
		int frame_depth = 0;
		VIDEO_FRAME_INFO_S video_frame_info;
		
		SOC_CHECK(HI_MPI_VI_GetFrameDepth((VI_CHN)vin, (HI_U32*)&frame_depth));
		if(0 == frame_depth){
			HI_MPI_VI_SetFrameDepth(vin, 1);
		}

		if(HI_SUCCESS == HI_MPI_VI_GetFrame(vin, &video_frame_info)){
			if(PIXEL_FORMAT_YUV_SEMIPLANAR_420 == video_frame_info.stVFrame.enPixelFormat){
				
				int const width = video_frame_info.stVFrame.u32Width;
				int const height = video_frame_info.stVFrame.u32Height;
				size_t const stride = video_frame_info.stVFrame.u32Stride[0];
				size_t const yuv420_size = stride * height * 3 / 2;
				const uint8_t* yuv420_data = HI_MPI_SYS_Mmap(video_frame_info.stVFrame.u32PhyAddr[0], yuv420_size);
				// allocate the bitmap data
				size_t bitmap24_size = width * height * 3;
				uint8_t* bitmap24_data = alloca(bitmap24_size);

				SOC_DEBUG("Frame(%d) [%dx%d] stride %d", vin, width, height, stride);
				
				if(yuv420_data){
					sdk_yuv420sem_to_bitmap888(yuv420_data, width, height, stride, bitmap24_data);

					if(0 == fseek(bitmap_stream, 0, SEEK_SET)){
						
						BIT_MAP_FILE_HEADER_t bmp_header;
						memset(&bmp_header, 0, sizeof(bmp_header));

						bmp_header.type[0] = 'B';
						bmp_header.type[1] = 'M';
						bmp_header.file_size = sizeof(bmp_header) + bitmap24_size;
						bmp_header.reserved_zero = 0;
						bmp_header.off_bits = sizeof(bmp_header);
						bmp_header.info_size = 40;
						bmp_header.width = width;
						bmp_header.height = height;
						bmp_header.planes = 1;
						bmp_header.bit_count = 24;
						bmp_header.compression = 0;
						bmp_header.size_image = bitmap24_size;
						bmp_header.xpels_per_meter = 0;
						bmp_header.ypels_per_meter = 0;
						bmp_header.clr_used = 0;
						bmp_header.clr_important = 0;

						fwrite(&bmp_header, 1, sizeof(bmp_header), bitmap_stream);
						for(i = 0; i < height; ++i){
							void* bitmap_offset = bitmap24_data + (height - i - 1) * width * 3;
							fwrite(bitmap_offset, 1, width * 3, bitmap_stream);
						}
					}
				}
				SOC_CHECK(HI_MPI_SYS_Munmap((HI_VOID*)yuv420_data, (HI_U32)yuv420_size));
			}
			SOC_CHECK(HI_MPI_VI_ReleaseFrame(vin, &video_frame_info));
		}
		SOC_CHECK(HI_MPI_VI_SetFrameDepth(vin, frame_depth));
	}
	return 0;
}

static int vin_avd_algo()
{
	IMAGE3_S image;
	IMP_S32 startFrm = 0;
	VIDEO_FRAME_INFO_S stFrame;
	VI_DEV viDev = 0;
	VI_CHN viChn = 0;
	HI_U32 w, h;
	IMP_HANDLE hAvd = NULL;

//	w = IMG_WIDTH_AVD;
//	h = IMG_HEIGHT_AVD;
	w = 1280, h = 720;

	/** create image of IMP YUV format */
	IMP_Image3Create(&image, w, h, IMAGE_FORMAT_IMP_YUV420, NULL);
	/** create AVD algorithm module */
	hAvd = IMP_AVD_Create(w, h);
	/** configure parameter for the algorithm */
	sleep(5);

	SOC_CHECK(HI_MPI_VI_GetFrame(viDev, &stFrame));

	//IMP_HiImageConvertToYUV420Image(&stFrame.stVFrame, &image);
	//SOC_CHECK(HI_MPI_VI_ReleaseFrame(viDev, &stFrame));

	//IMP_AVD_Process(hAvd,IMAGE3_S * pstImage)


	return 0;
}


static int vin_get_capture_attr(int vin, LP_SDK_VIN_CAPTURE_ATTR capture_attr)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		VI_CHN_ATTR_S vi_chn_attr;
		SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
		if(NULL != capture_attr){
			capture_attr->width = vi_chn_attr.stDestSize.u32Width;
			capture_attr->height = vi_chn_attr.stDestSize.u32Height;
			capture_attr->fps = vi_chn_attr.s32FrameRate;
			return 0;
		}
	}
	return -1;
}

static int vin_create_md(int vin, size_t ref_hblock, size_t ref_vblock, int ref_frame)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC != md_attr->magic
			&& ref_hblock > 0 && ref_vblock > 0){
			md_attr->magic = kSDK_VIN_MAGIC;
			//md_attr->ref_hblock = ref_hblock;
			//md_attr->ref_vblock = ref_vblock;
			md_attr->ref_hblock = 320 / 16;
			md_attr->ref_vblock = 240 / 16;
			md_attr->bitmap.mask_bytes = ref_hblock * ref_vblock; // align to 8bit
			md_attr->bitmap.mask_bytes = SDK_ALIGNED_BIG_ENDIAN(md_attr->bitmap.mask_bytes, 8) / 8;
			md_attr->bitmap.mask_bitflag = calloc(sizeof(uint8_t) * md_attr->bitmap.mask_bytes, 1);
			md_attr->bitmap.threshold = 1.0; // default to no flag			
			memset(md_attr->bitmap.mask_bitflag, 0xff, md_attr->bitmap.mask_bytes); // default to all mask

///////////////////////////////////////////////////////////////////////////
// Hislicon 
///////////////////////////////////////////////////////////////////////////
			do{
				int const ref_width = md_attr->ref_hblock * 16;
				int const ref_height = md_attr->ref_vblock * 16;
				
				const char *venc_mmz = (0 == vin % 2) ? HI_MMZ_ZONE_NAME0 : HI_MMZ_ZONE_NAME1;
				MPP_CHN_S mpp_chn_vda, mpp_chn_vpss;
				VDA_CHN_ATTR_S vda_chn_attr;

				mpp_chn_vpss.enModId  = HI_ID_VPSS;
				mpp_chn_vpss.s32ChnId = VPSS_BYPASS_CHN;
				mpp_chn_vpss.s32DevId = vin;
				mpp_chn_vda.enModId  = HI_ID_VDA;
				mpp_chn_vda.s32DevId = 0;
				mpp_chn_vda.s32ChnId = vin;

				hi3521_mmz_zone_assign(HI_ID_VDA, mpp_chn_vda.s32DevId, mpp_chn_vda.s32ChnId, venc_mmz);
				
				vda_chn_attr.enWorkMode = VDA_WORK_MODE_MD;
				vda_chn_attr.u32Width = ref_width;
				vda_chn_attr.u32Height = ref_height;

				vda_chn_attr.unAttr.stMdAttr.enVdaAlg = VDA_ALG_REF;
				vda_chn_attr.unAttr.stMdAttr.enMbSize = VDA_MB_16PIXEL;
				vda_chn_attr.unAttr.stMdAttr.enMbSadBits = VDA_MB_SAD_8BIT;
				vda_chn_attr.unAttr.stMdAttr.enRefMode = VDA_REF_MODE_DYNAMIC;
				vda_chn_attr.unAttr.stMdAttr.u32MdBufNum = 16;
				vda_chn_attr.unAttr.stMdAttr.u32VdaIntvl = ref_frame;  
				vda_chn_attr.unAttr.stMdAttr.u32BgUpSrcWgt = 128;
				vda_chn_attr.unAttr.stMdAttr.u32SadTh = 100;
				vda_chn_attr.unAttr.stMdAttr.u32ObjNumMax = 128;
				
				SOC_CHECK(HI_MPI_VDA_CreateChn(vin, &vda_chn_attr));
				SOC_CHECK(HI_MPI_VPSS_EnableChn(vin, VPSS_BYPASS_CHN));
				SOC_CHECK(HI_MPI_SYS_Bind(&mpp_chn_vpss, &mpp_chn_vda));
				SOC_CHECK(HI_MPI_VDA_StartRecvPic(vin));
				
				// set ref frame
				sdk_vin->set_md_ref_frame(vin, ref_frame);
				
			}while(0);
///////////////////////////////////////////////////////////////////////////

			return 0;
		}
	}
	SOC_INFO("MD(%d/%d) create [%d,%d] error!", vin, HI_VIN_CH_BACKLOG_REF, ref_hblock, ref_vblock);
	return -1;
}

static int vin_release_md(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic){
			// FIXME:
///////////////////////////////////////////////////////////////////////////
// Hislicon 
///////////////////////////////////////////////////////////////////////////
			do{
				
				MPP_CHN_S mpp_chn_vpss, mpp_chn_vda;
				mpp_chn_vpss.enModId  = HI_ID_VPSS;
				mpp_chn_vpss.s32ChnId = VPSS_BYPASS_CHN;
				mpp_chn_vpss.s32DevId = vin;
				mpp_chn_vda.enModId  = HI_ID_VDA;
				mpp_chn_vda.s32DevId = 0;
				mpp_chn_vda.s32ChnId = vin;

				SOC_CHECK(HI_MPI_SYS_UnBind(&mpp_chn_vpss, &mpp_chn_vda));
				SOC_CHECK(HI_MPI_VDA_StopRecvPic(vin));
				SOC_CHECK(HI_MPI_VDA_DestroyChn(vin));
			}while(0);
///////////////////////////////////////////////////////////////////////////
			// free
			free(md_attr->bitmap.mask_bitflag);
			md_attr->bitmap.mask_bitflag = NULL;
			md_attr->magic = 0;
			return 0;
		}
	}
	return -1;
}

static int vin_start_md(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		SOC_CHECK(HI_MPI_VDA_StartRecvPic(vin));
		return 0;
	}
	return -1;
}

static int vin_stop_md(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		SOC_CHECK(HI_MPI_VDA_StopRecvPic(vin));
		return 0;
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
		lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[vin];
		if(kSDK_VIN_MAGIC == md_attr->magic
			&& NULL != name){
			int const i_rect = vin_md_rect_lookup(vin, name);
			if(i_rect >= 0){
				return md_attr->rect[i_rect].threshold;
			}
		}
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
		VDA_CHN_ATTR_S vda_chn_attr;
		if(ref_frame > 0){
			VI_CHN_ATTR_S vi_chn_attr;
			uint32_t detect_intl;
			// get vi framerate for reference
			SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
			detect_intl = 1000000 * ref_frame / vi_chn_attr.s32FrameRate;
			if(!_sdk_vin.attr.md_intl || detect_intl < _sdk_vin.attr.md_intl){
				_sdk_vin.attr.md_intl = detect_intl;
			}
			SOC_CHECK(HI_MPI_VDA_GetChnAttr(vin, &vda_chn_attr));
			if(ref_frame != vda_chn_attr.unAttr.stMdAttr.u32VdaIntvl){
				vda_chn_attr.unAttr.stMdAttr.u32VdaIntvl = ref_frame;
				SOC_CHECK(HI_MPI_VDA_SetChnAttr(vin, &vda_chn_attr));
				return 0;
			}
		}
	}
	return -1;
}

static int vin_get_md_ref_frame(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		VDA_CHN_ATTR_S vda_chn_attr;
		SOC_CHECK(HI_MPI_VDA_GetChnAttr(vin, &vda_chn_attr));
		return vda_chn_attr.unAttr.stMdAttr.u32VdaIntvl;
	}
	return -1;
}

static int vin_set_md_ref_freq(int vin, int freq)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		if(freq > 0){
			VI_CHN_ATTR_S vi_chn_attr;
			int ref_frame = 0;
			// get vi framerate for reference
			SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
			ref_frame = vi_chn_attr.s32FrameRate / freq;
			return sdk_vin->set_md_ref_frame(vin, ref_frame);
		}
	}
	return -1;
}

static int vin_get_md_ref_freq(int vin)
{
	if(vin < HI_VIN_CH_BACKLOG_REF){
		VI_CHN_ATTR_S vi_chn_attr;
		int const ref_frame = sdk_vin->get_md_ref_frame(vin);
		SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
		return (vi_chn_attr.s32FrameRate + ref_frame - 1) / ref_frame;
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

static void hi3521_destroy_cover(int vin, int blk)
{
	RGN_HANDLE const rgn_handle = __HI_VIN_COVER_HANDLE(vin, blk);
	MPP_CHN_S mpp_chn_viu;

	mpp_chn_viu.enModId  = HI_ID_VIU;
	mpp_chn_viu.s32DevId = 0;
	mpp_chn_viu.s32ChnId = vin;

	SOC_CHECK(HI_MPI_RGN_DetachFrmChn(rgn_handle, &mpp_chn_viu));
	SOC_CHECK(HI_MPI_RGN_Destroy(rgn_handle));
}

static int vin_set_cover(int vin, int id, LP_SDK_VIN_COVER_ATTR cover)
{
	if(vin < HI_VIN_CH_BACKLOG_REF && id < HI_VIN_COVER_BACKLOG_REF){
		LP_SDK_VIN_COVER_ATTR const cover_attr = &_sdk_vin.attr.cover_attr[vin][id];
		bool cover_changed = false;
		
		if(!cover){
			// full fill the screen
			cover_attr->x = cover_attr->y = 0.0;
			cover_attr->width = cover_attr->height = 1.0;
			cover_changed = true;
		}else{
			if(cover->x < 1.0 && cover->y < 1.0
				&& cover->width >= 0.0 && cover->width <= 1.0
				&& cover->height >= 0.0 && cover->height <= 1.0){
				// pass
				if(0 != memcmp(cover_attr, cover, sizeof(ST_SDK_VIN_COVER_ATTR))){
					memcpy(cover_attr, cover, sizeof(ST_SDK_VIN_COVER_ATTR));
					cover_changed = true;

					//cannot be zero
					if(cover_attr->width == 0.0 || cover_attr->height == 0.0)
					{
						if(cover->enable == false)
						{
							cover_attr->width = cover->width==0.0?0.5:cover->width;
							cover_attr->height = cover->height==0.0?0.5:cover->height;
						}
						else
							return -1;
						
					}
					
				}
			}else{
				return -1;
			}
		}

		if(cover_changed){
			RGN_ATTR_S rgn_attr;
			MPP_CHN_S mpp_chn_viu;
			RGN_CHN_ATTR_S cover_chn_attr;
			RGN_HANDLE const rgn_handle = __HI_VIN_COVER_HANDLE(vin, id);
			int x = 0, y = 0, width = 0, height = 0;
			bool rgn_new = true;
			ST_SDK_VIN_CAPTURE_ATTR vin_capture_attr;

			// calc the absolute size
			sdk_vin->get_capture_attr(vin, &vin_capture_attr);

			rgn_attr.enType = COVER_RGN;
			mpp_chn_viu.enModId  = HI_ID_VIU;
			mpp_chn_viu.s32DevId = 0;
			mpp_chn_viu.s32ChnId = vin;

			if(HI_SUCCESS == HI_MPI_RGN_GetDisplayAttr(rgn_handle, &mpp_chn_viu, &cover_chn_attr)){
				// configurate the ready region
				rgn_new = false;
			}else{
				// create a new region
				hi3521_mmz_zone_assign(HI_ID_RGN, 0, 0, (0 == vin) ? HI_MMZ_ZONE_NAME0 : HI_MMZ_ZONE_NAME1);
				SOC_CHECK(HI_MPI_RGN_Create(rgn_handle, &rgn_attr));
			}
				
			x = (int)(cover_attr->x * (float)vin_capture_attr.width);
			y = (int)(cover_attr->y * (float)vin_capture_attr.height);
			width = (int)(cover_attr->width * (float)vin_capture_attr.width);
			height = (int)(cover_attr->height * (float)vin_capture_attr.height);

			x = SDK_ALIGNED_BIG_ENDIAN(x, 2);
			y = SDK_ALIGNED_BIG_ENDIAN(y, 4);
			width = SDK_ALIGNED_BIG_ENDIAN(width, 2);
			height = SDK_ALIGNED_BIG_ENDIAN(height, 4);
				
			cover_chn_attr.bShow = cover_attr->enable ? HI_TRUE : HI_FALSE;
			cover_chn_attr.enType = COVER_RGN;
			cover_chn_attr.unChnAttr.stCoverChn.u32Color = cover_attr->color;
			cover_chn_attr.unChnAttr.stCoverChn.u32Layer = id;
			cover_chn_attr.unChnAttr.stCoverChn.stRect.s32X = x;
			cover_chn_attr.unChnAttr.stCoverChn.stRect.s32Y = y;
			cover_chn_attr.unChnAttr.stCoverChn.stRect.u32Width  = width;
			cover_chn_attr.unChnAttr.stCoverChn.stRect.u32Height = height;

			SOC_INFO("VIN(%d) cover(%d) @ (%d,%d) [%dx%d] in #%08X ", vin, id,
				cover_chn_attr.unChnAttr.stCoverChn.stRect.s32X, cover_chn_attr.unChnAttr.stCoverChn.stRect.s32Y,
				cover_chn_attr.unChnAttr.stCoverChn.stRect.u32Width, cover_chn_attr.unChnAttr.stCoverChn.stRect.u32Height,
				cover_chn_attr.unChnAttr.stCoverChn.u32Color);
			if(rgn_new){
				SOC_CHECK(HI_MPI_RGN_AttachToChn(rgn_handle, &mpp_chn_viu, &cover_chn_attr));
			}else{
				SOC_CHECK(HI_MPI_RGN_SetDisplayAttr(rgn_handle, &mpp_chn_viu, &cover_chn_attr));
			}
			return 0;
		}
	}
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
		for(i = 0; i < HI_VIN_CH_BACKLOG_REF; ++i){
			lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[i];
			uint8_t threshold = (uint8_t)(255.0 * md_attr->bitmap.threshold);
			VDA_DATA_S vda_data;
			int v_offset = 0, h_offset = 0;
			
			prctl(PR_SET_NAME, "md_loop");

///////////////////////////////////////////////////////////////////////////
// Hislicon 
///////////////////////////////////////////////////////////////////////////
			while(HI_SUCCESS == HI_MPI_VDA_GetData(i, &vda_data, HI_FALSE)){
				if(md_attr->bitmap_mode){
					// bitmap mode
					bool bitmap_actived = false; // the motion symbol, to avoid the scan repeated
					for(v_offset = 0; v_offset < vda_data.u32MbHeight && !bitmap_actived; ++v_offset){
						for(h_offset = 0; h_offset < vda_data.u32MbWidth && !bitmap_actived; ++h_offset){
							const uint8_t *detect_offset = (uint8_t*)((HI_U32)vda_data.unData.stMdData.stMbSadData.pAddr);
							uint8_t detect_val = 0;
							// offset
							detect_offset += v_offset * vda_data.unData.stMdData.stMbSadData.u32Stride + h_offset;
							// detect value
							detect_val = *detect_offset;
							// check the threshold
							if(detect_val >= threshold){
								// check the mask
								if(sdk_vin->get_md_bitmap_one_mask){
									if(sdk_vin->get_md_bitmap_one_mask(i, h_offset, v_offset)){
										bitmap_actived = true;
									}
								}
							}
						}
					}
					if(bitmap_actived){
						if(md_attr->do_trap){
							md_attr->do_trap(i, NULL);
						}
					}
				}else{
					// rect mode
					int i_rect = 0;
					bool rect_actived[HI_VIN_MD_RECT_BACKLOG_REF]; // the motion symbol, to avoid the scan repeated

					for(i_rect = 0; i_rect < HI_VIN_MD_RECT_BACKLOG_REF; ++i_rect){
						rect_actived[i_rect] = false;
					}
				
					for(v_offset = 0; v_offset < vda_data.u32MbHeight; ++v_offset){
						for(h_offset = 0; h_offset < vda_data.u32MbWidth; ++h_offset){
							const uint8_t *detect_offset = (uint8_t*)((HI_U32)vda_data.unData.stMdData.stMbSadData.pAddr);
							uint8_t detect_val = 0;
							// offset
							detect_offset += v_offset * vda_data.unData.stMdData.stMbSadData.u32Stride + h_offset;
							// detect value
							detect_val = *detect_offset;
							// foreach all the rects
							for(i_rect = 0; i_rect < HI_VIN_MD_RECT_BACKLOG_REF; ++i_rect){
								lpSDK_VIN_MD_RECT_MODE const md_rect = &md_attr->rect[i_rect];
								if(!rect_actived[i_rect] && md_rect->occupation){
									// check the mask
									if(detect_val >= md_rect->threshold){
										// check mask
										float const x_ratio = (float)h_offset / (float)vda_data.u32MbWidth;
										float const y_ratio = (float)v_offset / (float)vda_data.u32MbHeight;
										if(x_ratio >= md_rect->x_ratio && y_ratio >= md_rect->y_ratio
											&& x_ratio < (md_rect->x_ratio + md_rect->width_ratio)
											&& y_ratio < (md_rect->y_ratio + md_rect->height_ratio)){
											// bingo!!
											rect_actived[i_rect] = true;
										}
									}
								}
							}
						}
					}

					for(i_rect = 0; i_rect < HI_VIN_MD_RECT_BACKLOG_REF; ++i_rect){
						if(rect_actived[i_rect]){
							lpSDK_VIN_MD_RECT_MODE const md_rect = &md_attr->rect[i_rect];
							if(md_attr->do_trap){
								md_attr->do_trap(i, md_rect->name);
							}
						}
					}
				}
				HI_MPI_VDA_ReleaseData(i, &vda_data);
			}
///////////////////////////////////////////////////////////////////////////
			
		}

		usleep(_sdk_vin.attr.md_intl);
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

//vi tend
#define VI_TEND_RETRY_CNT (20)
#define VI_TEND_FILE "/tmp/Interrupts_reset"
#define VI_TEND_FILE_LENGTH (1*1024*1024)//1M
static int vi_tend_loop(void *arg)
{
	VI_CHN_STAT_S viStat;
	prctl(PR_SET_NAME, "vi_tend_loop");
	while(_sdk_vin.attr.vi_tend_trigger){
		HI_MPI_VI_Query(0, &viStat);
		if(_sdk_vin.attr.old_int_cnt == viStat.u32IntCnt){
			_sdk_vin.attr.vi_tend_retry_cnt++;
			printf("there is no VI Interrupts-%d\r\n", _sdk_vin.attr.vi_tend_retry_cnt);
		}else{
			_sdk_vin.attr.vi_tend_retry_cnt = 0;
			_sdk_vin.attr.old_int_cnt = viStat.u32IntCnt;
		}
		
		//printf("Int cnt:%d-%d\r\n", _sdk_vin.attr.old_int_cnt, viStat.u32IntCnt);
		if(_sdk_vin.attr.vi_tend_retry_cnt > VI_TEND_RETRY_CNT){
			printf("there is no VI Interrupts----Need to reset sensor!\r\n");
			//write log file
			FILE *fd;
			char buf[128] = {0};
			time_t timet;
			int length = 0;
			fd = fopen(VI_TEND_FILE, "a+");
			if(fd){
				fseek(fd,0L,SEEK_END);
				length=ftell(fd);
				printf("file length:%d\n", length);
				if(length > VI_TEND_FILE_LENGTH){
					fclose(fd);
					fd = fopen(VI_TEND_FILE, "wb+");
					if(fd ){
						fclose(fd);
					}
					continue;
				}			
				timet = time(NULL);
				sprintf(buf, "%d:%d-%d\n", timet, _sdk_vin.attr.old_int_cnt, viStat.u32IntCnt);				
				fwrite(buf, 1, sizeof(buf),fd);	
				fclose(fd);
			}
			exit(0);
		}
		sleep(2);
	}
}


static int vi_tend_start()
{
	int ret = 0;
	if(!_sdk_vin.attr.vi_tend_tid){
		_sdk_vin.attr.vi_tend_trigger = true;
		_sdk_vin.attr.old_int_cnt = 0;
		_sdk_vin.attr.vi_tend_retry_cnt = 0;
		ret = pthread_create(&_sdk_vin.attr.vi_tend_tid, NULL, (void*)vi_tend_loop, NULL);
		SOC_ASSERT(0 == ret, "vi tend create thread failed!");
		return 0;
	}
	return -1;
}

static int vi_tend_stop()
{
	if(_sdk_vin.attr.vi_tend_tid){
		_sdk_vin.attr.vi_tend_trigger= false;
		_sdk_vin.attr.old_int_cnt = 0;
		_sdk_vin.attr.vi_tend_retry_cnt = 0;
		pthread_join(_sdk_vin.attr.vi_tend_tid, NULL);
		_sdk_vin.attr.vi_tend_tid = (pthread_t)NULL;
		return 0;
	}
	return -1;
}


static int vin_start()
{
	vi_tend_start();
	return md_start();
}

static int vin_stop()
{
	vi_tend_stop();
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
				hi3521_destroy_cover(i, ii);
			}
		}
		sdk_vin = NULL;
		return 0;
	}
	return -1;
}

