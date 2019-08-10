
#include "hi3518e.h"
#include "sdk/sdk_api.h"
#include "sdk_trace.h"
#include "sdk_common.h"
#include "ivs_md.h"
#include <sys/prctl.h>

#define kSDK_VIN_MAGIC (0xf0f0f0f0)

#ifndef HI3518A_VIN_DEV  
#define HI3518A_VIN_DEV (0)
#endif 



#define HI_VIN_CH_BACKLOG_REF (1)

#define HI_VIN_MD_RECT_BACKLOG_REF (8)

#define HI_VIN_COVER_BACKLOG_REF (4)
#define HI_VIN_COVER_HANDLE_OFFSET (RGN_HANDLE_MAX - 256 - HI_VIN_CH_BACKLOG_REF * HI_VIN_COVER_BACKLOG_REF) // 0 - 1024

#define __HI_VIN_COVER_HANDLE(__vin, __blk) (HI_VIN_COVER_HANDLE_OFFSET + __vin * HI_VIN_COVER_BACKLOG_REF + __blk)

#define MD_DETECT_TIME_CORRECTION (2)
#define MD_DETECT_REPEAT_COUNT (3)

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
	bool md_alarm_state;
	bool md_alarm_stop;
	pthread_t md_tid;

	// cover
	ST_SDK_VIN_COVER_ATTR cover_attr[HI_VIN_CH_BACKLOG_REF][HI_VIN_COVER_BACKLOG_REF];
	
}stSDK_VIN_ATTR, *lpSDK_VIN_ATTR;


#define SAMPLE_IVE_MD_IMAGE_NUM 2
#define IVE_ALIGN 16

typedef struct SDK_IVE_MD_S
{
	IVE_SRC_IMAGE_S astImg[SAMPLE_IVE_MD_IMAGE_NUM];	
	IVE_DST_MEM_INFO_S stBlob;
	MD_ATTR_S stMdAttr;		
	HI_U16 u16BaseWitdh;
	HI_U16 u16BaseHeight;
	uint8_t refBlockActiveCnt[72][88];
}stSDK_IVE_MD_S,*lpSDK_IVE_MD_S;



typedef struct SDK_VIN_HI3521 {
	stSDK_VIN_API api;
	stSDK_VIN_ATTR attr;
}stSDK_VIN_HI3521, *lpSDK_VIN_HI3521;

static stSDK_VIN_HI3521 _sdk_vin;
lpSDK_VIN_API sdk_vin = NULL;

static stSDK_IVE_MD_S stMd;

//free mmz 
#define IVE_MMZ_FREE(phy,vir)\
do{\
	if ((0 != (phy)) && (NULL != (vir)))\
	{\
		 HI_MPI_SYS_MmzFree((phy),(vir));\
		 (phy) = 0;\
		 (vir) = NULL;\
	}\
}while(0)




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

		if(HI_SUCCESS == HI_MPI_VI_GetFrame(vin, &video_frame_info, 2000)){//fix me: the last param
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

		if(HI_SUCCESS == HI_MPI_VI_GetFrame(vin, &video_frame_info, 2000)){//fix me: the last param
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
			capture_attr->fps = vi_chn_attr.s32DstFrameRate;
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
			md_attr->ref_hblock = 352 / 4;
			md_attr->ref_vblock = 288 / 4;
			md_attr->bitmap.mask_bytes = md_attr->ref_hblock * md_attr->ref_vblock; //ref_hblock * ref_vblock; // align to 8bit
			md_attr->bitmap.mask_bytes = SDK_ALIGNED_BIG_ENDIAN(md_attr->bitmap.mask_bytes, 8) / 8;
			md_attr->bitmap.mask_bitflag = calloc(sizeof(uint8_t) * md_attr->bitmap.mask_bytes, 1);
			md_attr->bitmap.threshold = 1.0; // default to no flag			
			memset(md_attr->bitmap.mask_bitflag, 0xff, md_attr->bitmap.mask_bytes); // default to all mask

///////////////////////////////////////////////////////////////////////////
// Hislicon 
///////////////////////////////////////////////////////////////////////////
			do{
				
				int const ref_width = md_attr->ref_hblock * 4;
				int const ref_height = md_attr->ref_vblock * 4;
				
				HI_U8 u8WndSz;	
				
				VPSS_CHN_MODE_S stVpssChnMode;	
				VPSS_CHN_ATTR_S stVpssChnAttr;
				HI_S32  VpssGrp = 0;
				HI_S32 vpss_md_chn = 3;      //for md chn				
				HI_U32 u32Depth = 4;				
				MD_CHN MdChn = 0;
				HI_U32 u32Size = 0;
				HI_U32 i;
				ST_SDK_VIN_CAPTURE_ATTR vin_capture_attr;
			
				sdk_vin->get_capture_attr(vin, &vin_capture_attr);



				stVpssChnMode.u32Width		 = ref_width;
                stVpssChnMode.u32Height 	 = ref_height;	
				stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_USER;
				stVpssChnMode.bDouble		 = HI_FALSE;
				stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
				stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
				memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
				stVpssChnAttr.s32SrcFrameRate = vin_capture_attr.fps;//vin_capture_attr.fps;//-1;
				stVpssChnAttr.s32DstFrameRate = vin_capture_attr.fps/2;//-1;
				if(stVpssChnAttr.s32DstFrameRate < 12){
					stVpssChnAttr.s32DstFrameRate = 12;
				}
				SOC_CHECK(HI_MPI_VPSS_SetChnAttr(VpssGrp, vpss_md_chn, &stVpssChnAttr));
				SOC_CHECK(HI_MPI_VPSS_SetChnMode(VpssGrp, vpss_md_chn, &stVpssChnMode));	
				
				SOC_CHECK(HI_MPI_VPSS_SetDepth(VpssGrp,vpss_md_chn,u32Depth));
				SOC_CHECK(HI_MPI_VPSS_EnableChn(vin,vpss_md_chn));

				memset(&stMd,0,sizeof(stSDK_IVE_MD_S));

				stMd.stMdAttr.enAlgMode = MD_ALG_MODE_BG;
				stMd.stMdAttr.enSadMode = IVE_SAD_MODE_MB_4X4;//IVE_SAD_MODE_MB_16X16;			
				stMd.stMdAttr.enSadOutCtrl = IVE_SAD_OUT_CTRL_THRESH;
				stMd.stMdAttr.u16SadThr = 100;//18;//100 * (1 << 1);//100 * (1 << 2);
				stMd.stMdAttr.u16Height = ref_height;				
				stMd.stMdAttr.u16Width  = ref_width;
				stMd.stMdAttr.stAddCtrl.u0q16X = 32768;
				stMd.stMdAttr.stAddCtrl.u0q16Y= 32768;			
				u8WndSz = ( 1 << (2 + stMd.stMdAttr.enSadMode));
				stMd.stMdAttr.stCclCtrl.u16InitAreaThr = 10;//u8WndSz * u8WndSz;
				stMd.stMdAttr.stCclCtrl.u16Step = u8WndSz;
			
				SOC_CHECK(HI_IVS_MD_Init());
				SOC_CHECK(HI_IVS_MD_CreateChn(MdChn,&stMd.stMdAttr));
				
				for (i = 0;i < SAMPLE_IVE_MD_IMAGE_NUM;i++){
					stMd.astImg[i].enType = IVE_IMAGE_TYPE_U8C1;				
					stMd.astImg[i].u16Width = ref_width;
					stMd.astImg[i].u16Height = ref_height;
					stMd.astImg[i].u16Stride[0] = ref_width + (IVE_ALIGN - ref_width%IVE_ALIGN)%IVE_ALIGN ;

					u32Size = stMd.astImg[i].u16Stride[0] * stMd.astImg[i].u16Height;
					SOC_CHECK(HI_MPI_SYS_MmzAlloc(&stMd.astImg[i].u32PhyAddr[0],(void**)&stMd.astImg[i].pu8VirAddr[0],NULL, HI_NULL, u32Size));
				}
		
				u32Size = sizeof(IVE_CCBLOB_S);				
				stMd.stBlob.u32Size = u32Size;
				SOC_CHECK(HI_MPI_SYS_MmzAlloc(&stMd.stBlob.u32PhyAddr,(void**)&stMd.stBlob.pu8VirAddr,NULL,HI_NULL, u32Size));
				
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
///////////////////////////////////////////////////////////////////////////
// Hislicon 
///////////////////////////////////////////////////////////////////////////
			do{	

				HI_S32 MdChn = 0;
				HI_S32 i;
				SOC_CHECK(HI_IVS_MD_DestroyChn(MdChn));

				for (i = 0; i < SAMPLE_IVE_MD_IMAGE_NUM; i++)
				{	
					IVE_MMZ_FREE(stMd.astImg[i].u32PhyAddr[0],stMd.astImg[i].pu8VirAddr[0]);
				}
				IVE_MMZ_FREE(stMd.stBlob.u32PhyAddr,stMd.stBlob.pu8VirAddr);		
				SOC_CHECK(HI_IVS_MD_Exit());
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
{return 0;
	if(vin < HI_VIN_CH_BACKLOG_REF){
		SOC_CHECK(HI_MPI_VDA_StartRecvPic(vin));
		return 0;
	}
	return -1;
}

static int vin_stop_md(int vin)
{return 0;
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
		int radioX = 4, radioY = 4, row = 0;
		if(kSDK_VIN_MAGIC == md_attr->magic
			&& x < md_attr->ref_hblock && y < md_attr->ref_vblock){

			for(row = 0; row < radioY; row++){
				int const mask_offset = (y * radioY + row) * md_attr->ref_hblock + x*radioX;
				int const mask_byte = mask_offset / 8;
				int const mask_bit = mask_offset % 8;

				if(mask_flag){
					md_attr->bitmap.mask_bitflag[mask_byte] |= (0xF<<mask_bit);
				}else{
					md_attr->bitmap.mask_bitflag[mask_byte] &= ~(0xF<<mask_bit);
				}
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
{return 0;
		if(vin < HI_VIN_CH_BACKLOG_REF){
		VDA_CHN_ATTR_S vda_chn_attr;
		if(ref_frame > 0){
			VI_CHN_ATTR_S vi_chn_attr;
			uint32_t detect_intl;
			// get vi framerate for reference
			SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
			detect_intl = 1000000 * ref_frame / (-1 == vi_chn_attr.s32DstFrameRate ? 25 : vi_chn_attr.s32DstFrameRate);
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
{return 0;
	if(vin < HI_VIN_CH_BACKLOG_REF){
		VDA_CHN_ATTR_S vda_chn_attr;
		SOC_CHECK(HI_MPI_VDA_GetChnAttr(vin, &vda_chn_attr));
		return vda_chn_attr.unAttr.stMdAttr.u32VdaIntvl;
	}
	return -1;
}

static int vin_set_md_ref_freq(int vin, int freq)
{return 0;
	if(vin < HI_VIN_CH_BACKLOG_REF){
		if(freq > 0){
			VI_CHN_ATTR_S vi_chn_attr;
			int ref_frame = 0;
			// get vi framerate for reference
			SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
			ref_frame = vi_chn_attr.s32DstFrameRate / freq;
			return sdk_vin->set_md_ref_frame(vin, ref_frame);
		}
	}	
	return -1;
}

static int vin_get_md_ref_freq(int vin)
{return 0;
	if(vin < HI_VIN_CH_BACKLOG_REF){
		VI_CHN_ATTR_S vi_chn_attr;
		int const ref_frame = sdk_vin->get_md_ref_frame(vin);
		SOC_CHECK(HI_MPI_VI_GetChnAttr(vin, &vi_chn_attr));
		return (vi_chn_attr.s32DstFrameRate + ref_frame - 1) / ref_frame;
	}
	return -1;
}

static int vin_get_md_alarm_state(bool* md_alarm_state)
{
	*md_alarm_state = _sdk_vin.attr.md_alarm_state;
	return 0;
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

	//SOC_CHECK(HI_MPI_RGN_DetachFrmChn(rgn_handle, &mpp_chn_viu));
	SOC_CHECK(HI_MPI_RGN_Destroy(rgn_handle));
}

static int vin_set_md_alarm_stop(bool *md_alarm_stop)
{
	_sdk_vin.attr.md_alarm_stop = *md_alarm_stop;
	return 0;
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
			mpp_chn_viu.enModId  = HI_ID_VPSS;
			mpp_chn_viu.s32DevId = 0;
			mpp_chn_viu.s32ChnId = vin;
			printf("vin =%d",vin);
			

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
			cover_chn_attr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
			cover_chn_attr.unChnAttr.stCoverChn.u32Color = cover_attr->color;
			cover_chn_attr.unChnAttr.stCoverChn.u32Layer = id;
			cover_chn_attr.unChnAttr.stCoverChn.stRect.s32X = x;
			cover_chn_attr.unChnAttr.stCoverChn.stRect.s32Y = y;
			cover_chn_attr.unChnAttr.stCoverChn.stRect.u32Width  = width;
			cover_chn_attr.unChnAttr.stCoverChn.stRect.u32Height = height;

			//SOC_INFO("VIN(%d) cover(%d) @ (%d,%d) [%dx%d] in #%08X ", vin, id,
			//	cover_chn_attr.unChnAttr.stCoverChn.stRect.s32X, cover_chn_attr.unChnAttr.stCoverChn.stRect.s32Y,
			//	cover_chn_attr.unChnAttr.stCoverChn.stRect.u32Width, cover_chn_attr.unChnAttr.stCoverChn.stRect.u32Height,
			//	cover_chn_attr.unChnAttr.stCoverChn.u32Color);
			//printf("rgn_new = %d \n",rgn_new);
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


HI_S32 vin_dma_image(VIDEO_FRAME_INFO_S *pstFrameInfo,IVE_DST_IMAGE_S *pstDst,HI_BOOL bInstant)
{
	HI_S32 s32Ret;
	IVE_HANDLE hIveHandle;
	IVE_SRC_DATA_S stSrcData;
	IVE_DST_DATA_S stDstData;
	IVE_DMA_CTRL_S stCtrl = {IVE_DMA_MODE_DIRECT_COPY,0};
	HI_BOOL bFinish = HI_FALSE;
	HI_BOOL bBlock = HI_TRUE;

	//fill src
	stSrcData.pu8VirAddr = (HI_U8*)pstFrameInfo->stVFrame.pVirAddr[0];
	stSrcData.u32PhyAddr = pstFrameInfo->stVFrame.u32PhyAddr[0];
	stSrcData.u16Width   = (HI_U16)pstFrameInfo->stVFrame.u32Width;
	stSrcData.u16Height  = (HI_U16)pstFrameInfo->stVFrame.u32Height;
	stSrcData.u16Stride  = (HI_U16)pstFrameInfo->stVFrame.u32Stride[0];

	//fill dst
	stDstData.pu8VirAddr = pstDst->pu8VirAddr[0];
	stDstData.u32PhyAddr = pstDst->u32PhyAddr[0];
	stDstData.u16Width   = pstDst->u16Width;
	stDstData.u16Height  = pstDst->u16Height;
	stDstData.u16Stride  = pstDst->u16Stride[0];

	s32Ret = HI_MPI_IVE_DMA(&hIveHandle,&stSrcData,&stDstData,&stCtrl,bInstant);	
	if (HI_SUCCESS != s32Ret)
	{
        printf("HI_MPI_IVE_DMA fail,Error!\n");
       return s32Ret;
    }
	
	if (HI_TRUE == bInstant)
	{
		s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);			
		while(HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
		{
			usleep(100);					
			s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);	
		}
		if (HI_SUCCESS != s32Ret)
		{
			printf("HI_MPI_IVE_Query fail,Error!\n");
		   return s32Ret;
		}
	}

	return HI_SUCCESS;
}

static void dump_map(uint8_t *array, int w, int h)
{
	int i, j;
	printf("dump_map:%d*%d\r\n", w, h);
	for(i = 0; i < h; i++){
		for(j = 0; j < w; j++){
			printf("%01d", *(array+i*w+j));
		}
		printf("\n");
	}
	printf("end\r\n");
}

static void md_loop(void *arg)
{ 
	int i = 0;
	VIDEO_FRAME_INFO_S stExtFrmInfo;
	struct timeval last_active_time;
	struct timeval cur_active_time;
	struct timeval last_detect_time;
    unsigned long long cur_active_time_us, last_active_time_us;
    unsigned long long last_detect_time_us;

	HI_S32 s32GetFrameMilliSec = 2000;//-1
	VPSS_GRP VpssGrp  = 0;
	VPSS_CHN VpssChn = 3;
	MD_CHN MdChn = 0;
	HI_S32 s32CurIdx = 0;

	HI_BOOL bFirstFrm = HI_TRUE;
	HI_BOOL bInstant = HI_TRUE;
	
	HI_U16 u16SrcWidth,u16SrcHeight,u16DstWidth,u16DstHeight;

	u16SrcWidth = stMd.stMdAttr.u16Width;
	u16SrcHeight = stMd.stMdAttr.u16Height;
	bool md_actived = HI_FALSE; 
	
	POINT_S astPoint[4];
	IVE_CCBLOB_S *pstBlob;

	HI_U16 u16Num ;
	HI_U16 j,k,m,z,g;
	int x_cnt = 0, y_cnt = 0;
	int h_offset = 0, v_offset = 0 ;  //line row

	int i_rect = 0;
	bool rect_actived[HI_VIN_MD_RECT_BACKLOG_REF]; // the motion symbol, to avoid the scan repeated

	int md_detect_cnt = 0;

	gettimeofday(&last_active_time, NULL);
	gettimeofday(&cur_active_time, NULL);
	gettimeofday(&last_detect_time, NULL);
    prctl(PR_SET_NAME, "md_loop");
	SDK_clean_array(stMd.refBlockActiveCnt, sizeof(stMd.refBlockActiveCnt));
	while(_sdk_vin.attr.md_trigger){
		for(i = 0; i < HI_VIN_CH_BACKLOG_REF; ++i){
			lpSDK_VIN_MD_ATTR const md_attr = &_sdk_vin.attr.md_attr[i];
///////////////////////////////////////////////////////////////////////////
// Hislicon 

			HI_U16 u16Thr= 0;
			HI_U16 threshold = 0;

			HI_MPI_VPSS_GetChnFrame(VpssGrp,VpssChn,&stExtFrmInfo,s32GetFrameMilliSec);	

			if (HI_TRUE != bFirstFrm){
				vin_dma_image(&stExtFrmInfo,&stMd.astImg[s32CurIdx],bInstant);  //base astImg[1]
			}else{
				vin_dma_image(&stExtFrmInfo,&stMd.astImg[s32CurIdx],bInstant);// base  astImg[0]
				HI_MPI_VPSS_ReleaseChnFrame(VpssGrp,VpssChn,&stExtFrmInfo);	
				bFirstFrm = HI_FALSE;
				goto CHANGE_IDX;
			}

			HI_IVS_MD_Process(MdChn,&stMd.astImg[s32CurIdx],&stMd.astImg[1 - s32CurIdx],NULL,&stMd.stBlob); 

			pstBlob = (IVE_CCBLOB_S *)stMd.stBlob.pu8VirAddr;
			u16DstWidth = stExtFrmInfo.stVFrame.u32Width;
			u16DstHeight = stExtFrmInfo.stVFrame.u32Height;

			HI_MPI_VPSS_ReleaseChnFrame(VpssGrp,VpssChn,&stExtFrmInfo);
			if(_sdk_vin.attr.md_alarm_stop){
				continue;
			}

			gettimeofday(&cur_active_time, NULL);
			cur_active_time_us = (unsigned long long)cur_active_time.tv_sec * 1000000ULL + (unsigned long long)cur_active_time.tv_usec;
			last_active_time_us = (unsigned long long)last_active_time.tv_sec * 1000000ULL + (unsigned long long)last_active_time.tv_usec;

			if(cur_active_time_us - last_active_time_us > 1000000 ||
				(last_active_time.tv_sec - MD_DETECT_TIME_CORRECTION > 0 &&
				last_active_time.tv_sec - MD_DETECT_TIME_CORRECTION > cur_active_time.tv_sec)){

				if(md_attr->bitmap_mode){
					// bitmap mode
					bool bitmap_actived = false; // the motion symbol, to avoid the scan repeated

					if( md_attr->bitmap.threshold <= 0.03 ){
						md_attr->bitmap.threshold = 0.025;
					}
					u16Thr = (HI_U16)(pstBlob->u16CurAreaThr * (md_attr->bitmap.threshold + 0.1) * 400);

					for(j = 0;j < pstBlob->u8RegionNum && HI_FALSE == bitmap_actived;j++){
						if( pstBlob->astRegion[j].u32Area > u16Thr ){
							astPoint[0].s32X = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Left / (HI_FLOAT)u16SrcWidth * (HI_FLOAT)u16DstWidth) & (~1) ;
							astPoint[0].s32Y = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Top / (HI_FLOAT)u16SrcHeight * (HI_FLOAT)u16DstHeight) & (~1);

							astPoint[1].s32X = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Right/ (HI_FLOAT)u16SrcWidth * (HI_FLOAT)u16DstWidth) & (~1);
							astPoint[1].s32Y = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Top / (HI_FLOAT)u16SrcHeight * (HI_FLOAT)u16DstHeight) & (~1);

							astPoint[2].s32X = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Right / (HI_FLOAT)u16SrcWidth * (HI_FLOAT)u16DstWidth) & (~1);
							astPoint[2].s32Y = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Bottom / (HI_FLOAT)u16SrcHeight * (HI_FLOAT)u16DstHeight) & (~1);

							astPoint[3].s32X = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Left / (HI_FLOAT)u16SrcWidth * (HI_FLOAT)u16DstWidth) & (~1);
							astPoint[3].s32Y = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Bottom / (HI_FLOAT)u16SrcHeight * (HI_FLOAT)u16DstHeight) & (~1);

							h_offset = astPoint[0].s32X / 4;//astPoint[0].s32X / 16;
							v_offset = astPoint[0].s32Y / 4;//astPoint[0].s32Y / 16;
							x_cnt = (astPoint[1].s32X - astPoint[0].s32X)/4;
							y_cnt = (astPoint[3].s32Y - astPoint[0].s32Y)/4;
							for(z = 0; z < y_cnt; z ++){
								for(g = 0; g < x_cnt; g ++ ){
									// check the mask
									if(sdk_vin->get_md_bitmap_one_mask){
										if(sdk_vin->get_md_bitmap_one_mask(i, h_offset, v_offset)){
											if(SDK_is_array_empty(stMd.refBlockActiveCnt, sizeof(stMd.refBlockActiveCnt))){
												last_detect_time = cur_active_time;
											}
											SDK_set_array(stMd.refBlockActiveCnt, sizeof(stMd.refBlockActiveCnt), h_offset, v_offset, md_attr->ref_hblock);
											last_detect_time_us = (unsigned long long)last_detect_time.tv_sec * 1000000ULL + (unsigned long long)last_detect_time.tv_usec;
											if(cur_active_time_us - last_detect_time_us > 500000){
												SDK_clean_array(stMd.refBlockActiveCnt, sizeof(stMd.refBlockActiveCnt));
												continue;
											}
										}
									}
									h_offset ++;
								}

								v_offset ++;
								h_offset = astPoint[0].s32X / 4;
							}

							if(SDK_is_array_active(stMd.refBlockActiveCnt, sizeof(stMd.refBlockActiveCnt), MD_DETECT_REPEAT_COUNT)){
								bitmap_actived = HI_TRUE;
								//dump_map(stMd.refBlockActiveCnt, 88, 72);
								SDK_clean_array(stMd.refBlockActiveCnt, sizeof(stMd.refBlockActiveCnt));
							}

							for(m = 0; m < 3;m++){
								for (k = m + 1; k < 4;k++){
									if ((astPoint[m].s32X == astPoint[k].s32X)
									 &&(astPoint[m].s32Y == astPoint[k].s32Y)){
										bitmap_actived = HI_FALSE;
										md_detect_cnt = 0;
										printf("MD rect is zero!!!\n");
										SDK_clean_array(stMd.refBlockActiveCnt, sizeof(stMd.refBlockActiveCnt));
										break;
									 }
								}
							}

							if(bitmap_actived == HI_TRUE){
								break;
							}
						}
					}

					_sdk_vin.attr.md_alarm_state = bitmap_actived;
					if(bitmap_actived){
						if(md_attr->do_trap){
							md_attr->do_trap(i, NULL);
							last_active_time = cur_active_time;
						}
					}
				}else{
					// rect mode
					//////////////////////////////////////FIXME

					for(i_rect = 0; i_rect < HI_VIN_MD_RECT_BACKLOG_REF; ++i_rect){
						rect_actived[i_rect] = false;
					}

					for(j = 0;j < 254;j++){
						if( pstBlob->astRegion[j].u32Area > u16Thr){

							astPoint[0].s32X = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Left / (HI_FLOAT)u16SrcWidth * (HI_FLOAT)u16DstWidth) & (~1) ;
							astPoint[0].s32Y = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Top / (HI_FLOAT)u16SrcHeight * (HI_FLOAT)u16DstHeight) & (~1);

							astPoint[1].s32X = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Right/ (HI_FLOAT)u16SrcWidth * (HI_FLOAT)u16DstWidth) & (~1);
							astPoint[1].s32Y = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Top / (HI_FLOAT)u16SrcHeight * (HI_FLOAT)u16DstHeight) & (~1);

							astPoint[2].s32X = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Right / (HI_FLOAT)u16SrcWidth * (HI_FLOAT)u16DstWidth) & (~1);
							astPoint[2].s32Y = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Bottom / (HI_FLOAT)u16SrcHeight * (HI_FLOAT)u16DstHeight) & (~1);

							astPoint[3].s32X = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Left / (HI_FLOAT)u16SrcWidth * (HI_FLOAT)u16DstWidth) & (~1);
							astPoint[3].s32Y = (HI_U16)((HI_FLOAT)pstBlob->astRegion[j].u16Bottom / (HI_FLOAT)u16SrcHeight * (HI_FLOAT)u16DstHeight) & (~1);

							md_actived = HI_TRUE;

							for(m = 0; m < 3;m++)
								{
									for (k = m + 1; k < 4;k++)
									{
									  if ((astPoint[m].s32X == astPoint[k].s32X)
										 &&(astPoint[m].s32Y == astPoint[k].s32Y))
										{
											md_actived = HI_FALSE;
											break;
										 }

									}
								}
							if(HI_TRUE == md_actived){
								for(i_rect = 0; i_rect < HI_VIN_MD_RECT_BACKLOG_REF; ++i_rect){
									lpSDK_VIN_MD_RECT_MODE const md_rect = &md_attr->rect[i_rect];
									if(!rect_actived[i_rect] && md_rect->occupation){

										float const x_ratio = (float)astPoint[0].s32X / (float)u16SrcWidth;
										float const y_ratio = (float)astPoint[0].s32Y / (float)u16SrcHeight;

										if(x_ratio >= md_rect->x_ratio && y_ratio >= md_rect->y_ratio
											&& x_ratio < (md_rect->x_ratio + md_rect->width_ratio)
											&& y_ratio < (md_rect->y_ratio + md_rect->height_ratio)){
											// bingo!!
											threshold = (HI_U16)(pstBlob->u16CurAreaThr * md_rect->threshold);
											if(pstBlob->astRegion[j].u32Area > threshold){
												rect_actived[i_rect] = true;
											}
										}
									}
								}
							}
						}
					}

					for(i_rect = 0; i_rect < HI_VIN_MD_RECT_BACKLOG_REF; ++i_rect){
						if(rect_actived[i_rect]){
							lpSDK_VIN_MD_RECT_MODE const md_rect = &md_attr->rect[i_rect];								
							if(md_attr->do_trap ){
								md_attr->do_trap(i, md_rect->name);
							}
						}
					}
				}
			}
		}

///////////////////////////////////////////////////////////////////////////
		CHANGE_IDX:
		//Change reference and current frame index
		    s32CurIdx = 1 - s32CurIdx;
			usleep(10000);

	}
	usleep(_sdk_vin.attr.md_intl);
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
		.get_md_alarm_state = vin_get_md_alarm_state,
		.set_md_alarm_stop = vin_set_md_alarm_stop,
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
	.attr = {
		.md_alarm_state = false,
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

