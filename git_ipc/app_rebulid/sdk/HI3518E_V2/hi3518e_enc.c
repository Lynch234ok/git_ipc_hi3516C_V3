
#include "hi3518e.h"
#include "sdk/sdk_api.h"
#include "sdk_trace.h"
#include "sdk_common.h"

#include "hi_tde_api.h"
#include "hi_tde_type.h"

#include <sys/mount.h>
#include "audio_aac_adp.h"
#include <sys/prctl.h>

#define VPSS_HD_PRE_CHN   0
#define VPSS_MULTI_PRE_CHN   1 
#define VPSS_SD_PRE_CHN   2


HI_BOOL bExitOverlayLoop   = HI_FALSE;
HI_BOOL bExitOverlayRelease = HI_FALSE;

#define OSD_REVERSE_RGN_MAXCNT 64

static int _hi3518_vpss_ch_map[] = {
	
	1,//VPSS_BSTR_CHN, // for main stream encode
	2,//VPSS_MULTI_PRE_CHN ;forSub-stream
	3,//vpss_md_chn
	0,//JPEG
	4,
	5,
	6,


	//VPSS_BYPASS_CHN,
	//VPSS_BSTR_CHN, // for snapshot
};

static int _hi3518_venc_ch_map[] = {
	0,
	0,// for main stream encode
	1,//forSub-stream
	2,//
	3,
};





#define HI_VENC_H264_REF_MODE (H264E_REFSLICE_FOR_2X)
#define HI_VENC_H265_REF_MODE (H264E_REFSLICE_FOR_1X)

#define HI_VENC_CH_BACKLOG_REF (1)
#define HI_AENC_CH_BACKLOG_REF (1)
#define HI_VENC_STREAM_BACKLOG_REF (2)

#define HI_VENC_JPEG_MIN_WIDTH (64)
#define HI_VENC_JPEG_MAX_WIDTH (8192)
#define HI_VENC_JPEG_MIN_HEIGHT (64)
#define HI_VENC_JPEG_MAX_HEIGHT (8192)

#define HI_VENC_OVERLAY_BACKLOG_REF (8)
#define HI_VENC_OVERLAY_CANVAS_STOCK_REF (HI_VENC_CH_BACKLOG_REF * HI_VENC_STREAM_BACKLOG_REF * HI_VENC_OVERLAY_BACKLOG_REF)
#define HI_VENC_OVERLAY_HANDLE_OFFSET (RGN_HANDLE_MAX - HI_VENC_OVERLAY_CANVAS_STOCK_REF) // 0 - 1024

#define __HI_VENC_CH(_vin, _stream) ((_vin)* HI_VENC_STREAM_BACKLOG_REF + (_stream))

#define HI_ENC_ATTR_MAGIC (0xf0f0f0f0)

typedef struct SDK_ENC_VIDEO_OVERLAY_ATTR {
	LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas;
	char name[32];
	int x, y;
	size_t width, height;
	RGN_HANDLE region_handle; 
}stSDK_ENC_VIDEO_OVERLAY_ATTR, *lpSDK_ENC_VIDEO_OVERLAY_ATTR;

typedef struct SDK_ENC_VIDEO_OVERLAY_ATTR_SET {
	stSDK_ENC_VIDEO_OVERLAY_ATTR attr[HI_VENC_OVERLAY_BACKLOG_REF];
}stSDK_ENC_VIDEO_OVERLAY_ATTR_SET, *lpSDK_ENC_VIDEO_OVERLAY_ATTR_SET;

enum SDK_ENC_PRODUCT_MODEL {
    emSDK_ENC_PRODUCT_MODEL_PX = 0,
    emSDK_ENC_PRODUCT_MODEL_CX,
};

typedef struct SDK_ENC_ATTR {
	enSDK_ENC_BUF_DATA_TYPE enType[HI_VENC_CH_BACKLOG_REF][HI_VENC_STREAM_BACKLOG_REF];
	union{	
		ST_SDK_ENC_STREAM_H264_ATTR h264_attr[HI_VENC_CH_BACKLOG_REF][HI_VENC_STREAM_BACKLOG_REF];
		ST_SDK_ENC_STREAM_H265_ATTR h265_attr[HI_VENC_CH_BACKLOG_REF][HI_VENC_STREAM_BACKLOG_REF];
	};
	uint8_t frame_ref_counter[HI_VENC_CH_BACKLOG_REF][HI_VENC_STREAM_BACKLOG_REF];	
	ST_SDK_ENC_STREAM_G711A_ATTR g711_attr[HI_AENC_CH_BACKLOG_REF];
	
	stSDK_ENC_VIDEO_OVERLAY_ATTR_SET video_overlay_set[HI_VENC_CH_BACKLOG_REF][HI_VENC_STREAM_BACKLOG_REF];
	ST_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas_stock[HI_VENC_OVERLAY_CANVAS_STOCK_REF];
    unsigned long long u64encPTS[HI_VENC_CH_BACKLOG_REF][HI_VENC_STREAM_BACKLOG_REF];

	bool loop_trigger;
	int ref_count;
	pthread_t loop_tid;
	pthread_mutex_t snapshot_mutex;
	pthread_mutex_t overlayex_mutex;
	
	lpSDK_ENC_VIDEO_OVERLAY_ATTR  overlay[OVERLAYEX_MAX_NUM_VPSS * 2];

	RGN_HANDLE overlay_handle[OVERLAYEX_MAX_NUM_VPSS * 2];
	int vpss_chn[OVERLAYEX_MAX_NUM_VPSS * 2];
	int overlay_handle_num;
	int product_model;
}stSDK_ENC_ATTR, *lpSDK_ENC_ATTR;

typedef struct hiRGN_OSD_REVERSE_INFO_S
{
    RGN_HANDLE Handle;
    HI_U8 u8PerPixelLumaThrd;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_REGION_INFO_S stLumaRgnInfo;	
}RGN_OSD_REVERSE_INFO_S;




#define STREAM_H264_ISSET(__stream_h264) (strlen((__stream_h264)->name) > 0)
#define STREAM_H264_CLEAR(__stream_h264) do{ memset((__stream_h264)->name, 0, sizeof((__stream_h264)->name)); }while(0)


typedef struct SDK_ENC_HI3521
{
	stSDK_ENC_API api;
	stSDK_ENC_ATTR attr;
}stSDK_ENC_HI3521, *lpSDK_ENC_HI3521;
static stSDK_ENC_HI3521 _sdk_enc;
lpSDK_ENC_API sdk_enc = NULL;

/////test for H265 file
//static FILE *pFile[VENC_MAX_CHN_NUM];



static inline uint64_t get_time_us()
{
	uint64_t time_us = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_us = tv.tv_sec;
	time_us *= 1000000;
	time_us += tv.tv_usec;
	return time_us;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

static int do_buffer_request(int buf_id, ssize_t data_sz, uint32_t keyflag, unsigned long long PtsUS)
{
	if(sdk_enc->do_buffer_request){
		return sdk_enc->do_buffer_request(buf_id, data_sz, keyflag, PtsUS);
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

static uint32_t enc_stream_limit_fps(uint32_t width, uint32_t height, uint32_t fps, uint32_t chip_id)
{
#define HI3518EV2_SDK_MAX_ABILITY (1920*1080*15)
#define HI3516CV2_SDK_MAX_ABILITY (1920*1080*35)

	uint32_t limit_framerate = 30;

	if(0x3518e200 == chip_id){	
		limit_framerate = HI3518EV2_SDK_MAX_ABILITY / (width * height);		
	}else if(0x3516c200 == chip_id){
		limit_framerate = HI3516CV2_SDK_MAX_ABILITY / (width * height); 	
	}

	if(limit_framerate < fps){
		return limit_framerate;
	}else{
		return fps;
	}

}
static int hi3518e_enc_resolution(int width, int height);

static int do_enc_resolution_limit(LP_SDK_ENC_STREAM_H264_ATTR streamAttr)
{
	char buf[16] = "";
	char tmp[5] = "";
	int width_old ,height_old;
	int ret = -1;
	uint32_t sensor_width_max,sensor_height_max;

	HI_SDK_ISP_get_sensor_resolution(&sensor_width_max, &sensor_height_max);

	FILE *fid = fopen(MAIN_RESOLUTION, "rb");
	
	if(NULL != fid){
		fread(buf,sizeof(buf),1,fid);
		fclose(fid);
		fid = NULL;
		sscanf(buf,"%d %s %d",&width_old,tmp,&height_old);
		
		if(streamAttr->width != width_old || streamAttr->height != height_old){
			//SOC_INFO("New resolution %d x %d",streamAttr->width,streamAttr->height);
			ret = hi3518e_enc_resolution(streamAttr->width,streamAttr->height);
			if(0 != ret){
				streamAttr->width = sensor_width_max;
				streamAttr->height = sensor_height_max;
				return -1;
			}else{
				return 0;
			}
		}		
	}else{
		SOC_INFO("FILE %s don't exist!",MAIN_RESOLUTION);
		if(streamAttr->width > sensor_width_max || streamAttr->height > sensor_height_max){
			streamAttr->width = sensor_width_max;
			streamAttr->height = sensor_height_max;
		}
	}
	
	return -1;
}

/* È´òÁ∫ßË∑≥Â∏ßÂèÇËÄÉÂºÄÂÖ≥ */
static int enc_ref_enable(int venc_ch, int enable)
{
    VENC_PARAM_REF_S RefParam;
    SOC_CHECK(HI_MPI_VENC_GetRefParam(venc_ch, &RefParam));
    if(enable == 0) {
        RefParam.bEnablePred = HI_TRUE;
        RefParam.u32Base = 1;
        RefParam.u32Enhance = 0;
    }
    else if(enable == 1) {
        RefParam.bEnablePred = HI_TRUE;
        RefParam.u32Base = 1;
        RefParam.u32Enhance = 1;
    }
    SOC_CHECK(HI_MPI_VENC_SetRefParam(venc_ch, &RefParam));

}

static int enc_create_stream_h264(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr)
{
	_sdk_enc.attr.enType[vin][stream] = kSDK_ENC_BUF_DATA_H264;
	if(vin < HI_VENC_CH_BACKLOG_REF && stream < HI_VENC_STREAM_BACKLOG_REF
		&& NULL != h264_attr && STREAM_H264_ISSET(h264_attr)){
		
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
		//printf("name:%s\n", h264_attr->name);	
		if(0 == enc_lookup_stream_byname(h264_attr->name, NULL, NULL)){
			// the name is overlap
			return -1;
		}
		
		if(!STREAM_H264_ISSET(streamH264Attr)){
			int const venc_group = vin * HI_VENC_STREAM_BACKLOG_REF + stream;
			int const venc_ch = venc_group;
			int const vpssGroup = vin;
			int const vpssChannel = _hi3518_vpss_ch_map[venc_ch];

			//printf("vin = %d/%d, venc_group = %d, venc_ch = %d, vpss = %d/%d-%d\r\n", vin, stream, venc_group, venc_ch, vpssGroup, vpssChannel, HI_VENC_STREAM_BACKLOG_REF);
			int bps_limit = 0;
			const char *venc_mmz = (0 == venc_group % 2) ? HI_MMZ_ZONE_NAME0 : HI_MMZ_ZONE_NAME1;

			// hisilicon structure
			//VPSS_CHN_MODE_S vpssChanneln_mode;
			//VPSS_CHN_ATTR_S vpssChanneln_attr;
			VPSS_CROP_INFO_S stCropInfo;
			MPP_CHN_S mppChannelVPSS;
			MPP_CHN_S mppChannelVENC;
			VENC_CHN_ATTR_S vencChannelAttr;
			VENC_ATTR_S *const p_venc_attr = &vencChannelAttr.stVeAttr; // the attribute of video encoder
			VENC_RC_ATTR_S *const p_venc_rc_attr = &vencChannelAttr.stRcAttr; // the attribute of rate  ctrl
			VENC_ATTR_H264_S *const p_venc_attr_h264 = &p_venc_attr->stAttrH264e;
			VENC_ATTR_H264_CBR_S *const p_h264_cbr = &p_venc_rc_attr->stAttrH264Cbr;
			VENC_ATTR_H264_VBR_S *const p_h264_vbr = &p_venc_rc_attr->stAttrH264Vbr;
			VENC_ATTR_H264_FIXQP_S *const p_h264_fixqp = &p_venc_rc_attr->stAttrH264FixQp;
			VENC_ATTR_H264_ABR_S *const p_h264_abr = &p_venc_rc_attr->stAttrH264Abr;
	//		VENC_PARAM_H264_CBR_S h264_cbrv2;
			// vin attributes
			ST_SDK_VIN_CAPTURE_ATTR vin_capture_attr;
			sdk_vin->get_capture_attr(vin, &vin_capture_attr);

			// backup the attributes
			memcpy(streamH264Attr, h264_attr, sizeof(ST_SDK_ENC_STREAM_H264_ATTR));
			// legal check
			streamH264Attr->width = SDK_ALIGNED_LITTLE_ENDIAN(streamH264Attr->width, 2);
			streamH264Attr->height = SDK_ALIGNED_LITTLE_ENDIAN(streamH264Attr->height, 2);
			if(streamH264Attr->width < 160){
				streamH264Attr->width = 160;
			}
			if(streamH264Attr->height < 64){
				streamH264Attr->height = 64;
			}

			if(stream == 0){
				do_enc_resolution_limit(streamH264Attr);//main resolution
			}

			uint32_t chip_id;
			ISP_PUB_ATTR_S stPubAttr;
			HI_U32 u32SrcFrmRate;
			
			HI_MPI_SYS_GetChipId(&chip_id);
			streamH264Attr->fps = enc_stream_limit_fps(streamH264Attr->width, streamH264Attr->height, streamH264Attr->fps, chip_id);
			
			if(streamH264Attr->fps > vin_capture_attr.fps){
				streamH264Attr->fps = vin_capture_attr.fps; // encode framerate must be less than capture framerate
			}


			SOC_CHECK(HI_MPI_ISP_GetPubAttr(0,&stPubAttr));
			if(stPubAttr.f32FrameRate < vin_capture_attr.fps){
				if(stPubAttr.f32FrameRate < streamH264Attr->fps){
					u32SrcFrmRate = streamH264Attr->fps;
				}else{
					u32SrcFrmRate = (HI_U32)stPubAttr.f32FrameRate;
				}
			}else{
				u32SrcFrmRate = vin_capture_attr.fps;
			}

			SOC_INFO("Create H264 Stream(%d,%d,%s) %dx%d @ %d/%d", vin, stream, streamH264Attr->name, streamH264Attr->width, streamH264Attr->height, streamH264Attr->fps, streamH264Attr->gop);
			
			// Assign MMZ
 			hi3521_mmz_zone_assign(HI_ID_GROUP, venc_group, 0, venc_mmz);
			hi3521_mmz_zone_assign(HI_ID_VENC, 0, venc_ch, venc_mmz);

			// Greate Venc Group
			//SOC_CHECK(HI_MPI_VENC_CreateGroup(venc_group));

			// Create Venc Channel
			memset(&vencChannelAttr, 0, sizeof(vencChannelAttr));
			p_venc_attr->enType = PT_H264; // must be h264 for this interface
			p_venc_attr_h264->u32MaxPicWidth = streamH264Attr->width;
			p_venc_attr_h264->u32MaxPicHeight = streamH264Attr->height;
			p_venc_attr_h264->u32PicWidth = p_venc_attr_h264->u32MaxPicWidth;
			p_venc_attr_h264->u32PicHeight = p_venc_attr_h264->u32MaxPicHeight;
			//p_venc_attr_h264->u32BufSize  = p_venc_attr_h264->u32MaxPicWidth * p_venc_attr_h264->u32MaxPicHeight * 3 / 2; // stream buffer size
			p_venc_attr_h264->u32BufSize  = p_venc_attr_h264->u32MaxPicWidth * p_venc_attr_h264->u32MaxPicHeight * 3 / 4; // stream buffer size
			switch(streamH264Attr->profile){
				default:
				case kSDK_ENC_H264_PROFILE_BASELINE:
					p_venc_attr_h264->u32Profile = 0;
					break;
					
				case kSDK_ENC_H264_PROFILE_AUTO:
				case kSDK_ENC_H264_PROFILE_MAIN:
					p_venc_attr_h264->u32Profile = 1;
					break;		
				case kSDK_ENC_H264_PROFILE_HIGH:
					p_venc_attr_h264->u32Profile = 2;
			}
			p_venc_attr_h264->bByFrame = HI_TRUE;// get stream mode is slice mode or frame mode
			p_venc_attr_h264->u32BFrameNum = 0;/* 0: not support B frame; >=1: number of B frames */
			p_venc_attr_h264->u32RefNum = 1;/* 0: default; number of refrence frame*/
			//p_venc_attr_h264->bField = HI_FALSE;  // surpport frame code only for hi3516, bfield = HI_FALSE
			//p_venc_attr_h264->bMainStream = HI_TRUE; // surpport main stream only for hi3516, bMainStream = HI_TRUE
			//p_venc_attr_h264->u32Priority = 0; // channels precedence level. invalidate for hi3516
			//p_venc_attr_h264->bVIField = HI_FALSE; // the sign of the VI picture is field or frame. Invalidate for hi3516

			bps_limit = sdk_venc_bps_limit(streamH264Attr->width, streamH264Attr->height,
				streamH264Attr->fps, streamH264Attr->bps);

			HI_U32 minQp;
            if(_sdk_enc.attr.product_model == emSDK_ENC_PRODUCT_MODEL_CX) {
                if(stream == 0) {
                    switch(h264_attr->quality){
                        case kSDK_ENC_QUALITY_BD:
                            minQp = 33;
                            break;
                        case kSDK_ENC_QUALITY_AUTO:
                        case kSDK_ENC_QUALITY_HD:
                            minQp = 27;
                            break;
                        case kSDK_ENC_QUALITY_FLUENCY:
                            minQp = 38;
                            break;
                        default:
                            minQp = 27;
                            break;
                    }
                }
                else {
                    switch(h264_attr->quality){
                        case kSDK_ENC_QUALITY_BD:
                            minQp = 27;
                            break;
                        case kSDK_ENC_QUALITY_AUTO:
                        case kSDK_ENC_QUALITY_HD:
                            minQp = 22;
                            break;
                        case kSDK_ENC_QUALITY_FLUENCY:
                            minQp = 33;
                            break;
                        default:
                            minQp = 22;
                            break;
                    }
                }
            }
            else {
                switch(h264_attr->quality){
                    case kSDK_ENC_QUALITY_BD:
                        minQp = 27;
                        break;
                    case kSDK_ENC_QUALITY_AUTO:
                    case kSDK_ENC_QUALITY_HD:
                        minQp = 22;
                        break;
                    case kSDK_ENC_QUALITY_FLUENCY:
                        minQp = 33;
                        break;
                    default:
                        minQp = 22;
                        break;
                }
            }
			switch(h264_attr->rc_mode){
				default:
				case kSDK_ENC_H264_RC_MODE_VBR:
				case kSDK_ENC_H264_RC_MODE_AUTO:
				{
					vencChannelAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
					p_h264_vbr->u32Gop = streamH264Attr->gop;
					p_h264_vbr->u32StatTime = p_h264_vbr->u32Gop / streamH264Attr->fps;//(streamH264Attr->gop + streamH264Attr->fps - 1) / streamH264Attr->fps;
					p_h264_vbr->u32SrcFrmRate = u32SrcFrmRate;
					p_h264_vbr->fr32DstFrmRate = (typeof(p_h264_vbr->fr32DstFrmRate))streamH264Attr->fps;
					//p_h264_vbr->u32MaxBitRate = bps_limit * 4 /3;
					p_h264_vbr->u32MaxBitRate = bps_limit;
					p_h264_vbr->u32MinQp = minQp; //24;
					p_h264_vbr->u32MaxQp = 50; //32;
					break;
				}

				//case kSDK_ENC_H264_RC_MODE_Abr:
				case kSDK_ENC_H264_RC_MODE_CBR:
				{
					p_venc_rc_attr->enRcMode = VENC_RC_MODE_H264CBR;
					p_h264_cbr->u32Gop = streamH264Attr->gop;
					p_h264_cbr->u32StatTime = p_h264_cbr->u32Gop / streamH264Attr->fps;
					p_h264_cbr->u32SrcFrmRate = u32SrcFrmRate;
					p_h264_cbr->fr32DstFrmRate = (typeof(p_h264_vbr->fr32DstFrmRate))streamH264Attr->fps;
					p_h264_cbr->u32BitRate = bps_limit;
					p_h264_cbr->u32FluctuateLevel = 0;					
					break;
				}
				case kSDK_ENC_H264_RC_MODE_ABR:
				{
					p_venc_rc_attr->enRcMode = VENC_RC_MODE_H264ABR;
					p_h264_abr->u32Gop = streamH264Attr->gop;
					p_h264_abr->u32StatTime = (streamH264Attr->gop + streamH264Attr->fps - 1) / streamH264Attr->fps;
					p_h264_abr->u32SrcFrmRate = u32SrcFrmRate;
					p_h264_abr->fr32DstFrmRate = (typeof(p_h264_vbr->fr32DstFrmRate))streamH264Attr->fps;
					p_h264_abr->u32AvgBitRate = bps_limit;
					p_h264_abr->u32MaxBitRate = bps_limit * 4 / 3; // 1.5 times of bitrate
					break;
				}
				case kSDK_ENC_H264_RC_MODE_FIXQP:
				{
					p_venc_rc_attr->enRcMode = VENC_RC_MODE_H264FIXQP;
					p_h264_fixqp->u32Gop = streamH264Attr->gop;
					p_h264_fixqp->u32SrcFrmRate = u32SrcFrmRate;
					p_h264_fixqp->fr32DstFrmRate = (typeof(p_h264_vbr->fr32DstFrmRate))streamH264Attr->fps;
					p_h264_fixqp->u32IQp = 20;
					p_h264_fixqp->u32PQp = 23;
					break;
				}
			}

	//		SOC_CHECK(HI_MPI_VENC_CreateChn(venc_ch, &vencChannelAttr));

			// set vpss chn mode
			do{
//#if !defined(HI3518E_V2)		
				float fsrcScale, fdstScale;
				fsrcScale = (float)vin_capture_attr.height/(float)vin_capture_attr.width;
				fdstScale = (float)streamH264Attr->height/(float)streamH264Attr->width;
				printf("src=%f   dst=%f\r\n", fsrcScale, fdstScale);
				//FIX me

				SOC_CHECK(HI_MPI_VPSS_GetChnCrop(vpssGroup, vpssChannel, &stCropInfo));
				int stream_resolu,vin_resolu;
				stream_resolu = streamH264Attr->width * streamH264Attr->height;
				vin_resolu = vin_capture_attr.width * vin_capture_attr.height;				
				if(stream == 0 && stream_resolu <= vin_resolu ){
					if(fsrcScale != fdstScale){					
						stCropInfo.bEnable = 1; 
						stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
						if(fsrcScale > fdstScale){
							stCropInfo.stCropRect.s32X = 0; 
							stCropInfo.stCropRect.s32Y = (vin_capture_attr.height - vin_capture_attr.width*fdstScale)/2; 
							stCropInfo.stCropRect.u32Width = vin_capture_attr.width; 
							stCropInfo.stCropRect.u32Height = vin_capture_attr.width*fdstScale;
							stCropInfo.stCropRect.s32Y = (stCropInfo.stCropRect.s32Y%2)? stCropInfo.stCropRect.s32Y + 1 : stCropInfo.stCropRect.s32Y;
							stCropInfo.stCropRect.u32Height = (stCropInfo.stCropRect.u32Height%2)? stCropInfo.stCropRect.u32Height -1: stCropInfo.stCropRect.u32Height;

						/*	printf("to min x=%d y=%d w=%d h=%d\r\n", stCropInfo.stCropRect.s32X, stCropInfo.stCropRect.s32Y,
								stCropInfo.stCropRect.u32Width, stCropInfo.stCropRect.u32Height);*/
						}else{//fsrcScale < fdstScale
							stCropInfo.stCropRect.s32X = (vin_capture_attr.width - vin_capture_attr.height/fdstScale)/2; 
							stCropInfo.stCropRect.s32Y = 0; 
							stCropInfo.stCropRect.u32Width = vin_capture_attr.height/fdstScale; 
							stCropInfo.stCropRect.u32Height = vin_capture_attr.height;
							stCropInfo.stCropRect.s32X = (stCropInfo.stCropRect.s32X%2)? stCropInfo.stCropRect.s32X + 1 : stCropInfo.stCropRect.s32X;
							stCropInfo.stCropRect.u32Width = (stCropInfo.stCropRect.u32Width%2)? stCropInfo.stCropRect.u32Width -1: stCropInfo.stCropRect.u32Width;
							
						/*	printf("to max x=%d y=%d w=%d h=%d\r\n", stCropInfo.stCropRect.s32X, stCropInfo.stCropRect.s32Y,
								stCropInfo.stCropRect.u32Width, stCropInfo.stCropRect.u32Height);*/
						}
	
					}else{//dont need crop
						stCropInfo.bEnable = 0;
						
					}
					////type VPSS_CROP_ABS_COOR    2   2  4  4
					stCropInfo.stCropRect.s32X = SDK_ALIGNED_LITTLE_ENDIAN(stCropInfo.stCropRect.s32X,2);
					stCropInfo.stCropRect.s32Y = SDK_ALIGNED_LITTLE_ENDIAN(stCropInfo.stCropRect.s32Y,2);
					stCropInfo.stCropRect.u32Height = SDK_ALIGNED_LITTLE_ENDIAN(stCropInfo.stCropRect.u32Height,4);
					stCropInfo.stCropRect.u32Width = SDK_ALIGNED_LITTLE_ENDIAN(stCropInfo.stCropRect.u32Width,4);
					
					SOC_CHECK(HI_MPI_VPSS_SetChnCrop(vpssGroup, vpssChannel, &stCropInfo)); 	
				}
				if(stream == 0 && stream_resolu > vin_resolu){
					
					p_venc_attr_h264->u32PicWidth = vin_capture_attr.width;
					p_venc_attr_h264->u32PicHeight = vin_capture_attr.height;
					streamH264Attr->width = vin_capture_attr.width;
					streamH264Attr->height = vin_capture_attr.height;
					
			     	stCropInfo.bEnable = 0;					
					SOC_CHECK(HI_MPI_VPSS_SetChnCrop(vpssGroup, vpssChannel, &stCropInfo)); 	
					
				}

//#endif

		  		VPSS_CHN_ATTR_S vpssChanneln_attr;
				VPSS_CHN_MODE_S stVpssChnMode;				
				SOC_CHECK(HI_MPI_VPSS_GetChnMode(vpssGroup,vpssChannel,&stVpssChnMode)); 	
				
		        if(stVpssChnMode.u32Height <= streamH264Attr->height && stVpssChnMode.u32Width <= streamH264Attr->width  ){
					stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_USER;
					stVpssChnMode.bDouble		 = HI_FALSE;
					stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
					stVpssChnMode.u32Width		 = streamH264Attr->width;
					stVpssChnMode.u32Height 	 = streamH264Attr->height;
					stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;//COMPRESS_MODE_SEG;
					memset(&vpssChanneln_attr, 0, sizeof(vpssChanneln_attr));
					vpssChanneln_attr.s32SrcFrameRate = -1;
					vpssChanneln_attr.s32DstFrameRate = -1;
					//printf("vpssChannel:%d\r\n", vpssChannel);
					if(vpssChannel >= VPSS_MAX_PHY_CHN_NUM){
						VPSS_EXT_CHN_ATTR_S vpss_ext_chn_attr;
						vpss_ext_chn_attr.s32BindChn = 1; // bind to vpss 1
						vpss_ext_chn_attr.s32SrcFrameRate = vin_capture_attr.fps;
						vpss_ext_chn_attr.s32DstFrameRate = streamH264Attr->fps;
						vpss_ext_chn_attr.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
						vpss_ext_chn_attr.u32Width = p_venc_attr_h264->u32MaxPicWidth;
						vpss_ext_chn_attr.u32Height = p_venc_attr_h264->u32MaxPicHeight;
						vpss_ext_chn_attr.enCompressMode = COMPRESS_MODE_NONE;
						SOC_CHECK(HI_MPI_VPSS_SetExtChnAttr(vpssGroup, vpssChannel, &vpss_ext_chn_attr));
					}else{
						SOC_CHECK(HI_MPI_VPSS_SetChnAttr(vpssGroup, vpssChannel, &vpssChanneln_attr));
						SOC_CHECK(HI_MPI_VPSS_SetChnMode(vpssGroup, vpssChannel, &stVpssChnMode));
					}
					SOC_CHECK(HI_MPI_VPSS_EnableChn(vpssGroup, vpssChannel));
				}	
				
			}while(0);
			
			SOC_CHECK(HI_MPI_VENC_CreateChn(venc_ch, &vencChannelAttr));
			// to reduce the size of IDR
			do{
				VENC_RC_PARAM_S stVencRcPara;
				uint32_t u32ThrdI[12] = {7,7,7,7,7,9,9,9,12,15,18,25};//{7,7,7,9,12,14,18,25,255,255,255,255};
				uint32_t u32ThrdP[12] = {7,7,7,7,7,9,9,9,12,15,18,25};	
				//SOC_CHECK(HI_MPI_VENC_GetRcPara(venc_ch, &stVencRcPara));
				
				SOC_CHECK(HI_MPI_VENC_GetRcParam(venc_ch, &stVencRcPara));
				memcpy(stVencRcPara.u32ThrdI, u32ThrdI, sizeof(stVencRcPara.u32ThrdI));
				memcpy(stVencRcPara.u32ThrdP, u32ThrdP, sizeof(stVencRcPara.u32ThrdI));
				switch(p_venc_rc_attr->enRcMode){
					default:
					case VENC_RC_MODE_H264VBR:
						{							
							stVencRcPara.u32RowQpDelta = 3;
							stVencRcPara.stParamH264VBR.s32IPQPDelta = 2;
						}
						break;
					case VENC_RC_MODE_H264CBR:
						
						break;
					case VENC_RC_MODE_H264FIXQP:
						break;
				}
				//SOC_CHECK(HI_MPI_VENC_SetRcPara(venc_ch, &stVencRcPara));
				
				SOC_CHECK(HI_MPI_VENC_SetRcParam(venc_ch, &stVencRcPara));
			}while(0);

			memset(&mppChannelVPSS, 0, sizeof(mppChannelVPSS));
			memset(&mppChannelVENC, 0, sizeof(mppChannelVENC));
			// binding venc to vpss
			mppChannelVPSS.enModId = HI_ID_VPSS;
			mppChannelVPSS.s32DevId = vpssGroup;
			mppChannelVPSS.s32ChnId = vpssChannel;	
			mppChannelVENC.enModId = HI_ID_VENC;			
			mppChannelVENC.s32DevId = 0;
			mppChannelVENC.s32ChnId = venc_ch;
			SOC_CHECK(HI_MPI_SYS_Bind(&mppChannelVPSS, &mppChannelVENC));
			
#if defined(HI3518E_V2)		
			if(streamH264Attr->width * streamH264Attr->height >= 1920*1080){
				VPSS_LOW_DELAY_INFO_S stLowDelayInfo;
				int s32Ret = -1;
				VPSS_CHN_ATTR_S vpssChanneln_attr2;
				VPSS_CHN_MODE_S stVpssChnMode2; 			
				SOC_CHECK(HI_MPI_VPSS_GetChnMode(vpssGroup,vpssChannel,&stVpssChnMode2));
				
				if(0 == stream){
					s32Ret = HI_MPI_VPSS_GetLowDelayAttr(vpssGroup,vpssChannel,&stLowDelayInfo);
					printf("stLowDelayInfo bEnable = %d,lines=%d\n",stLowDelayInfo.bEnable,stLowDelayInfo.u32LineCnt);
					if(HI_FALSE == stLowDelayInfo.bEnable){
						stLowDelayInfo.bEnable = HI_TRUE;
						stLowDelayInfo.u32LineCnt = stVpssChnMode2.u32Height /2;
						s32Ret = HI_MPI_VPSS_SetLowDelayAttr(vpssGroup,vpssChannel,&stLowDelayInfo);
					}
					s32Ret = HI_MPI_VPSS_GetLowDelayAttr(vpssGroup,vpssChannel,&stLowDelayInfo);
					printf("stLowDelayInfo bEnable = %d,lines=%d\n",stLowDelayInfo.bEnable,stLowDelayInfo.u32LineCnt);
				
				}

			}
			if(0 == stream){
				VENC_CHN_ATTR_S stJpegAttr;
				VENC_CHN vencChannelJPEG = VENC_MAX_CHN_NUM - 2;		
				HI_MPI_VENC_GetChnAttr(vencChannelJPEG, &stJpegAttr);
				if(0 == HI_MPI_VENC_GetChnAttr(vencChannelJPEG, &stJpegAttr)){
					stJpegAttr.stVeAttr.stAttrJpeg.u32PicWidth= streamH264Attr->width;
					stJpegAttr.stVeAttr.stAttrJpeg.u32PicHeight = streamH264Attr->height;				
					HI_MPI_VENC_SetChnAttr(vencChannelJPEG, &stJpegAttr);
				}		

			}
	
#endif
            enc_ref_enable(venc_ch, streamH264Attr->refEnable);
			return 0;
		}
	}
	return -1;
}

static int enc_create_stream_h265(int vin, int stream, LP_SDK_ENC_STREAM_H265_ATTR h265_attr)
{
	return 0;
}
static int enc_creat_snapshot_chn(int vin, int vencChannelJPEG, ssize_t width, ssize_t height)
{

	if(vin < HI_VENC_CH_BACKLOG_REF){				
		int const vpssGroup = vin;
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[0][0];
		int vpssChannel;//_hi3518_vpss_ch_map[vpss_last_ch];	

		VENC_CHN_ATTR_S vencChannelAttr;
		VENC_ATTR_JPEG_S *const vencAttrJPEG = &vencChannelAttr.stVeAttr.stAttrJpeg;
		MPP_CHN_S mppChannelVPSS;
		MPP_CHN_S mppChannelVENC;
		ST_SDK_VIN_CAPTURE_ATTR vin_capture_attr;

		VPSS_CHN_MODE_S stVpssChnMode;				
		VPSS_CHN_ATTR_S vpssChanneln_attr;
		
		memset(&stVpssChnMode,0,sizeof(VPSS_CHN_MODE_S));	
		memset(&vpssChanneln_attr,0,sizeof(VPSS_CHN_ATTR_S));
		

		sdk_vin->get_capture_attr(vin, &vin_capture_attr);
		uint32_t sensor_width_max,sensor_height_max;
		HI_SDK_ISP_get_sensor_resolution(&sensor_width_max, &sensor_height_max);

		// default size
#if defined(HI3518EV2_MODEL) | defined(HI3516CV2_MODEL)
		if(kSDK_ENC_SNAPSHOT_SIZE_AUTO == width || kSDK_ENC_SNAPSHOT_SIZE_AUTO == height){
			width = vin_capture_attr.width / 2;
			height = vin_capture_attr.height / 2;
		}else{
			if(kSDK_ENC_SNAPSHOT_SIZE_MAX == width){
					width = vin_capture_attr.width;//streamH264Attr->width;
			}
			if(kSDK_ENC_SNAPSHOT_SIZE_MAX == height){
					height = vin_capture_attr.height;//streamH264Attr->height;
			}
			if(kSDK_ENC_SNAPSHOT_SIZE_MIN == width){
				width = HI_VENC_JPEG_MIN_WIDTH;
			}
			if(kSDK_ENC_SNAPSHOT_SIZE_MIN == height){
				height = HI_VENC_JPEG_MIN_HEIGHT;
			}
		}

		if(vin_capture_attr.width == vin_capture_attr.height ){
			width = 320;
			height = 320;
		}else{

		}
#else
		if(vin_capture_attr.width == vin_capture_attr.height ){
			width = 320;
			height = 320;
		}else{
			width = 640;
			height = 360;
		}

#endif

		// check the hisilicon size limited
		if(width < HI_VENC_JPEG_MIN_WIDTH){
			width = HI_VENC_JPEG_MIN_WIDTH;
		}
		/*if(width > vin_capture_attr.width){
			width = vin_capture_attr.width;
		}*/
		if(height < HI_VENC_JPEG_MIN_HEIGHT){
			height = HI_VENC_JPEG_MIN_HEIGHT;
		}
		/*if(height > vin_capture_attr.height){
			height = vin_capture_attr.height;
		}*/

		width = SDK_ALIGNED_LITTLE_ENDIAN(width, 4);
		height = SDK_ALIGNED_LITTLE_ENDIAN(height, 4);

		SOC_INFO("Create JPEG CHN %d, %dx%d", vencChannelJPEG, width, height);

		if((VENC_MAX_CHN_NUM - 2) == vencChannelJPEG){ /// main stream jpeg
			if(sensor_width_max * sensor_height_max >= 1920*1080 &&
				vin_capture_attr.width * vin_capture_attr.height >= 1920*1080){
#if defined(HI3518EV2_MODEL) | defined(HI3516CV2_MODEL)
				vpssChannel = 0;
				stVpssChnMode.u32Width		 = vin_capture_attr.width;
				stVpssChnMode.u32Height 	 = vin_capture_attr.height;
				vpssChanneln_attr.s32SrcFrameRate = vin_capture_attr.fps;//-1;
				vpssChanneln_attr.s32DstFrameRate = 1;//-1
#else //PX product
				vpssChannel = 2;
				stVpssChnMode.u32Width		 = width;//streamH264Attr->width;
				stVpssChnMode.u32Height 	 = height;//streamH264Attr->height;
				vpssChanneln_attr.s32SrcFrameRate = -1;
				vpssChanneln_attr.s32DstFrameRate = -1;
#endif
			}else{
				vpssChannel = _hi3518_vpss_ch_map[0];//  vpss main stream chn 1
			}
							
		}else if((VENC_MAX_CHN_NUM - 1) == vencChannelJPEG ){
			if(vin_capture_attr.width != vin_capture_attr.height){
				vpssChannel = 2;
				stVpssChnMode.u32Width		 = width;//streamH264Attr->width;
				stVpssChnMode.u32Height 	 = height;//streamH264Attr->height;
				vpssChanneln_attr.s32SrcFrameRate = -1;
				vpssChanneln_attr.s32DstFrameRate = -1;
			}else{
				vpssChannel = 2;
				stVpssChnMode.u32Width		 = width;//streamH264Attr->width;
				stVpssChnMode.u32Height 	 = height;//streamH264Attr->height;
				vpssChanneln_attr.s32SrcFrameRate = -1;
				vpssChanneln_attr.s32DstFrameRate = -1;
			}

			
		}
		stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_USER;
		stVpssChnMode.bDouble		 = HI_FALSE;
		stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;//COMPRESS_MODE_SEG;
		

		if(vpssChannel != _hi3518_vpss_ch_map[0]){
			SOC_CHECK(HI_MPI_VPSS_SetChnAttr(vpssGroup, vpssChannel, &vpssChanneln_attr));
			SOC_CHECK(HI_MPI_VPSS_SetChnMode(vpssGroup, vpssChannel, &stVpssChnMode));	
			SOC_CHECK(HI_MPI_VPSS_EnableChn(vpssGroup, vpssChannel));
		}else{
			SOC_CHECK(HI_MPI_VPSS_EnableChn(vpssGroup, vpssChannel));
		}

		// create venc group and channel
		vencChannelAttr.stVeAttr.enType = PT_JPEG;

		vencAttrJPEG->u32MaxPicWidth  = width;
		vencAttrJPEG->u32MaxPicHeight = height; 
		vencAttrJPEG->u32PicWidth  = vencAttrJPEG->u32MaxPicWidth;
		vencAttrJPEG->u32PicHeight = vencAttrJPEG->u32MaxPicHeight;
		vencAttrJPEG->u32BufSize = vencAttrJPEG->u32MaxPicWidth * vencAttrJPEG->u32MaxPicHeight;


		vencAttrJPEG->bByFrame = HI_TRUE; // get stream mode is field mode	or frame mode
	//	vencAttrJPEG->bVIField = HI_FALSE; // the sign of the VI picture is field or frame
	//	vencAttrJPEG->u32Priority = 1; // channels precedence level
		vencAttrJPEG->bSupportDCF = HI_FALSE;
		vencAttrJPEG->u32BufSize = SDK_ALIGNED_BIG_ENDIAN(vencAttrJPEG->u32BufSize, 64);

		SOC_CHECK(HI_MPI_VENC_CreateChn(vencChannelJPEG, &vencChannelAttr));

		
		// start to receive the picture from vi unit
		mppChannelVPSS.enModId = HI_ID_VPSS;
		mppChannelVPSS.s32DevId = vpssGroup;	//0
		mppChannelVPSS.s32ChnId = vpssChannel; // attention here	ch 6
		mppChannelVENC.enModId = HI_ID_VENC;//HI_ID_GROUP;	
		mppChannelVENC.s32DevId = 0;//vencGroupJPEG;
		mppChannelVENC.s32ChnId = vencChannelJPEG;	//15
		
		SOC_CHECK(HI_MPI_VPSS_EnableChn(vpssGroup, vpssChannel));
		SOC_CHECK(HI_MPI_SYS_Bind(&mppChannelVPSS, &mppChannelVENC));
		return 0;
		
	}

	return -1;
}


static int enc_release_stream_h264(int vin, int stream)
{
	LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];	
	
	if(vin < HI_VENC_CH_BACKLOG_REF && stream < HI_VENC_STREAM_BACKLOG_REF){
		if(STREAM_H264_ISSET(streamH264Attr)){
			int const venc_ch = __HI_VENC_CH(vin, stream);
	//		int const venc_group = venc_ch;
			int const vpssGroup = vin;
			time_t now;		
			int const vpssChannel = _hi3518_vpss_ch_map[venc_ch];		
			MPP_CHN_S mppChannelVPSS;
			MPP_CHN_S mppChannelVENC;

			memset(&mppChannelVPSS, 0, sizeof(mppChannelVPSS));
			memset(&mppChannelVENC, 0, sizeof(mppChannelVENC));
			// unbind venc to vpss
			mppChannelVPSS.enModId = HI_ID_VPSS;
			mppChannelVPSS.s32DevId = vpssGroup;
			mppChannelVPSS.s32ChnId = vpssChannel;
			mppChannelVENC.enModId = HI_ID_VENC;
			mppChannelVENC.s32DevId = 0;
			mppChannelVENC.s32ChnId = venc_ch;
						
			// clear the magic			
			STREAM_H264_CLEAR(streamH264Attr);		
			time(&now);
			while (_sdk_enc.attr.ref_count > 0){ // wait for idle
				if (( time(NULL) - now) > 5) {
					break;
				}
				usleep(200*1000);
			}
			
			SOC_CHECK(HI_MPI_VENC_StopRecvPic(venc_ch));
			SOC_CHECK(HI_MPI_SYS_UnBind(&mppChannelVPSS, &mppChannelVENC));
			usleep(60000); // wait for the last frame buffering finished
			
			SOC_CHECK(HI_MPI_VENC_DestroyChn(venc_ch));	
			return 0;
		}
	}
	return -1;
}

static int enc_release_stream_h265(int vin, int stream)
{
	return 0;
}

static int enc_release_snapshot_chn(int vin, int vencChannelJPEG)
{
	SOC_INFO("Release JPEG CHN %d ", vencChannelJPEG);

	MPP_CHN_S mppChannelVPSS;
	MPP_CHN_S mppChannelVENC;
	int vpssChannel;

	
	if((VENC_MAX_CHN_NUM - 2) == vencChannelJPEG){ /// main stream jpeg
		vpssChannel = 0;
		
	}else if((VENC_MAX_CHN_NUM - 1) == vencChannelJPEG ){
		vpssChannel = 2;
	}

	mppChannelVPSS.enModId = HI_ID_VPSS;
	mppChannelVPSS.s32DevId = vin;	//0
	mppChannelVPSS.s32ChnId = vpssChannel; // attention here	ch 6
	mppChannelVENC.enModId = HI_ID_VENC;//HI_ID_GROUP;	
	mppChannelVENC.s32DevId = vin;//vencGroupJPEG;
	mppChannelVENC.s32ChnId = vencChannelJPEG;	//15

	SOC_CHECK(HI_MPI_SYS_UnBind(&mppChannelVPSS, &mppChannelVENC));
	SOC_CHECK(HI_MPI_VENC_StopRecvPic(vencChannelJPEG));	
	usleep(50 * 1000); 
	SOC_CHECK(HI_MPI_VENC_DestroyChn(vencChannelJPEG));

	return 0;

}

static int set_stream_h264_fps_gop_rate(int vin, int stream,LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr,
							LP_SDK_ENC_STREAM_H264_ATTR h264_attr){
	int ret = -1;
	if((strcmp(streamH264Attr->name ,h264_attr->name) == 0 ) && (streamH264Attr->width == h264_attr->width) &&  (streamH264Attr->height == h264_attr->height)  &&  (streamH264Attr->profile == h264_attr->profile) 
		&&(streamH264Attr->rc_mode == h264_attr->rc_mode) && (streamH264Attr->quality == h264_attr->quality) &&  (streamH264Attr->buf_id == h264_attr->buf_id) ){

		streamH264Attr->bps = h264_attr->bps;
		streamH264Attr->fps = h264_attr->fps;
		streamH264Attr->gop = h264_attr->gop;

		int s32Ret = -1;
		VENC_CHN_ATTR_S venc_ch_attr;
		int const venc_ch = __HI_VENC_CH(vin, stream);
		s32Ret = HI_MPI_VENC_GetChnAttr(venc_ch, &venc_ch_attr);
		ST_SDK_VIN_CAPTURE_ATTR vin_capture_attr;
		int fps = h264_attr->fps;
		sdk_vin->get_capture_attr(vin, &vin_capture_attr);

		// encode framerate must be less than capture framerate
		if(fps > vin_capture_attr.fps){
			fps = vin_capture_attr.fps;
			streamH264Attr->fps = fps;
		}

		if (s32Ret == HI_SUCCESS){
			if(VENC_RC_MODE_H264CBR == venc_ch_attr.stRcAttr.enRcMode){
				venc_ch_attr.stRcAttr.stAttrH264Cbr.u32BitRate = h264_attr->bps;
				venc_ch_attr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate = fps;
				venc_ch_attr.stRcAttr.stAttrH264Cbr.u32Gop = h264_attr->gop;
				venc_ch_attr.stRcAttr.stAttrH264Cbr.u32StatTime = h264_attr->gop/fps;
			}else if(VENC_RC_MODE_H264VBR == venc_ch_attr.stRcAttr.enRcMode){
				venc_ch_attr.stRcAttr.stAttrH264Vbr.u32MaxBitRate = h264_attr->bps;
				venc_ch_attr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate = fps;
				venc_ch_attr.stRcAttr.stAttrH264Vbr.u32Gop = h264_attr->gop;
				venc_ch_attr.stRcAttr.stAttrH264Vbr.u32StatTime = h264_attr->gop/fps;
			}else if(VENC_RC_MODE_H264ABR == venc_ch_attr.stRcAttr.enRcMode){
				venc_ch_attr.stRcAttr.stAttrH264Abr.u32AvgBitRate = h264_attr->bps;
				venc_ch_attr.stRcAttr.stAttrH264Abr.u32MaxBitRate = h264_attr->bps;
				venc_ch_attr.stRcAttr.stAttrH264Abr.fr32DstFrmRate = fps;
				venc_ch_attr.stRcAttr.stAttrH264Abr.u32Gop = h264_attr->gop;
			}else if(VENC_RC_MODE_H264FIXQP == venc_ch_attr.stRcAttr.enRcMode){
				venc_ch_attr.stRcAttr.stAttrH264FixQp.fr32DstFrmRate = fps;
				venc_ch_attr.stRcAttr.stAttrH264FixQp.u32Gop = h264_attr->gop;
			}
			s32Ret = HI_MPI_VENC_SetChnAttr(venc_ch,&venc_ch_attr);
			if(s32Ret == HI_SUCCESS){
				ret = 0;
			}
		}
	}
	return ret;
}

static int enc_set_stream_h264(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr)
{
	if(vin < HI_VENC_CH_BACKLOG_REF && stream < HI_VENC_STREAM_BACKLOG_REF
		&& NULL != h264_attr && STREAM_H264_ISSET(h264_attr)){
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];

		if(STREAM_H264_ISSET(streamH264Attr)){
			if((strcmp(streamH264Attr->name ,h264_attr->name) == 0 ) && (streamH264Attr->width == h264_attr->width) &&  (streamH264Attr->height == h264_attr->height)  &&  (streamH264Attr->profile == h264_attr->profile) 
				&&(streamH264Attr->rc_mode == h264_attr->rc_mode) && (streamH264Attr->quality == h264_attr->quality) &&  (streamH264Attr->buf_id == h264_attr->buf_id) ){
				set_stream_h264_fps_gop_rate(vin, stream, streamH264Attr, h264_attr);
                if(streamH264Attr->refEnable != h264_attr->refEnable) {
                    streamH264Attr->refEnable = h264_attr->refEnable;
                    enc_ref_enable(stream, h264_attr->refEnable);
                }
			}else{
				// width / height adjust need to restart the stream				
				SDK_ENC_release_stream(vin, stream);
				sdk_enc->create_stream_h264(vin, stream, h264_attr);				
				sdk_enc->enable_stream_h264(vin, stream, true);
			}
			return 0;
		}
	}
	return -1;
}

static int enc_set_stream_h265(int vin, int stream, LP_SDK_ENC_STREAM_H265_ATTR h265_attr)
{
	return 0;
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

static int enc_get_stream_h265(int vin, int stream, LP_SDK_ENC_STREAM_H265_ATTR h265_attr)
{
	return 0;
}

static int enc_create_stream_g711a(int ain, int vin_ref)
{
	if(ain < HI_AENC_CH_BACKLOG_REF){
		if(vin_ref < HI_VENC_CH_BACKLOG_REF){
			enSDK_AUDIO_SAMPLE_RATE ain_samplerate;
			int ain_samplewidth = 0;
			int ain_packet_size = 0;

			SOC_INFO("create g711a @ %d/%d", ain, vin_ref);

			if(sdk_audio && 0 == sdk_audio->get_ain_ch_attr(ain, &ain_samplerate, &ain_samplewidth, &ain_packet_size)){
				int const aenc_ch = ain;
				LP_SDK_ENC_STREAM_G711A_ATTR const stream_g711_attr = &_sdk_enc.attr.g711_attr[ain];

				
				if(0 == stream_g711_attr->packet_size){
					AENC_ATTR_G711_S aenc_attr_g711 = {0};
					AENC_ATTR_AAC_S stAencAac;
					AENC_CHN_ATTR_S aenc_ch_attr = {.enType = PT_G711A, .pValue = &aenc_attr_g711,};
					MPP_CHN_S mpp_ch_ai, mpp_ch_aenc;
#if 0
					aenc_ch_attr.enType = PT_G711A;
					aenc_ch_attr.pValue = &aenc_attr_g711;
					//aenc_ch_attr.u32BufSize = 8;
					//aenc_ch_attr.u32BufSize = MAX_AUDIO_FRAME_NUM; // ver 050 faq 1.6.3
					aenc_ch_attr.u32BufSize = 30;
					aenc_ch_attr.u32PtNumPerFrm = ain_packet_size;
#else
					aenc_ch_attr.enType = PT_AAC;
					aenc_ch_attr.u32BufSize = 30;
					aenc_ch_attr.u32PtNumPerFrm = ain_packet_size;
					aenc_ch_attr.pValue = &stAencAac;
					stAencAac.enAACType = AAC_TYPE_AACLC;
					stAencAac.enBitRate = AAC_BPS_24K;
					stAencAac.enBitWidth = AUDIO_BIT_WIDTH_16;
					stAencAac.enSmpRate = AUDIO_SAMPLE_RATE_8000;
					stAencAac.enSoundMode = AUDIO_SOUND_MODE_MONO;
					stAencAac.enTransType = AAC_TRANS_TYPE_ADTS;
					stAencAac.s16BandWidth = 0;
#endif
					stream_g711_attr->ain = ain;
					stream_g711_attr->vin_ref = vin_ref;
					stream_g711_attr->sample_rate = 8000;
					stream_g711_attr->sample_width = 16;
					// FIXME: remember to check the sample rate and bitwidth
					stream_g711_attr->packet_size = ain_packet_size;

					// sdk operate
					// enable vin
					//SOC_CHECK(HI_MPI_AI_EnableChn(HI_AIN_DEV, ain));
					// create aenc chn
					SOC_CHECK(HI_MPI_AENC_CreateChn(aenc_ch, &aenc_ch_attr));
					// bind AENC to AI channel
					
					mpp_ch_ai.enModId  = HI_ID_AI;
					mpp_ch_ai.s32DevId = HI_AIN_DEV;
					mpp_ch_ai.s32ChnId = ain;
					mpp_ch_aenc.enModId  = HI_ID_AENC;
					mpp_ch_aenc.s32DevId = 0;
					mpp_ch_aenc.s32ChnId = aenc_ch;
					SOC_CHECK(HI_MPI_SYS_Bind(&mpp_ch_ai, &mpp_ch_aenc));

					return 0;	
				}
			}
		}
	}
	return -1;
}

#define AENC_CODEC_TYPE_PCM (1<<0)
#define AENC_CODEC_TYPE_G711A (1<<1)
#define AENC_CODEC_TYPE_G711U (1<<2)
#define AENC_CODEC_TYPE_AAC (1<<3)

static int audio_type;

static int enc_create_audio_stream(int ain, int vin_ref, int type)
{
	if(ain < HI_AENC_CH_BACKLOG_REF){
		if(vin_ref < HI_VENC_CH_BACKLOG_REF){
			enSDK_AUDIO_SAMPLE_RATE ain_samplerate;
			int ain_samplewidth = 0;
			int ain_packet_size = 0;
			audio_type = type;
			if(sdk_audio && 0 == sdk_audio->get_ain_ch_attr(ain, &ain_samplerate, &ain_samplewidth, &ain_packet_size)){
				int const aenc_ch = ain;
				LP_SDK_ENC_STREAM_G711A_ATTR const stream_g711_attr = &_sdk_enc.attr.g711_attr[ain];

				
				if(0 == stream_g711_attr->packet_size){
					AENC_ATTR_G711_S aenc_attr_g711 = {0};
					AENC_ATTR_AAC_S stAencAac;
					AENC_CHN_ATTR_S aenc_ch_attr = {.enType = PT_G711A, .pValue = &aenc_attr_g711,};
					MPP_CHN_S mpp_ch_ai, mpp_ch_aenc;
	          		if(type == AENC_CODEC_TYPE_G711A){
						SOC_INFO("create g711a @ %d/%d", ain, vin_ref);
						aenc_ch_attr.enType = PT_G711A;
						aenc_ch_attr.pValue = &aenc_attr_g711;
						//aenc_ch_attr.u32BufSize = 8;
						//aenc_ch_attr.u32BufSize = MAX_AUDIO_FRAME_NUM; // ver 050 faq 1.6.3
						aenc_ch_attr.u32BufSize = 30;
						aenc_ch_attr.u32PtNumPerFrm = ain_packet_size;
					}else if(type == AENC_CODEC_TYPE_AAC){
						SOC_INFO("create AAC @ %d/%d", ain, vin_ref);
						aenc_ch_attr.enType = PT_AAC;
						aenc_ch_attr.u32BufSize = 30;
						aenc_ch_attr.u32PtNumPerFrm = ain_packet_size;
						aenc_ch_attr.pValue = &stAencAac;
						stAencAac.enAACType = AAC_TYPE_AACLC;
						stAencAac.enBitRate = AAC_BPS_24K;
						stAencAac.enBitWidth = AUDIO_BIT_WIDTH_16;
						stAencAac.enSmpRate = AUDIO_SAMPLE_RATE_8000;
						stAencAac.enSoundMode = AUDIO_SOUND_MODE_MONO;
						stAencAac.enTransType = AAC_TRANS_TYPE_ADTS;
						stAencAac.s16BandWidth = 0;
					}
					stream_g711_attr->ain = ain;
					stream_g711_attr->vin_ref = vin_ref;
					stream_g711_attr->sample_rate = 8000;
					stream_g711_attr->sample_width = 16;
					// FIXME: remember to check the sample rate and bitwidth
					stream_g711_attr->packet_size = ain_packet_size;

					// sdk operate
					// enable vin
					//SOC_CHECK(HI_MPI_AI_EnableChn(HI_AIN_DEV, ain));
					// create aenc chn
					SOC_CHECK(HI_MPI_AENC_CreateChn(aenc_ch, &aenc_ch_attr));
					// bind AENC to AI channel
					
					mpp_ch_ai.enModId  = HI_ID_AI;
					mpp_ch_ai.s32DevId = HI_AIN_DEV;
					mpp_ch_ai.s32ChnId = ain;
					mpp_ch_aenc.enModId  = HI_ID_AENC;
					mpp_ch_aenc.s32DevId = 0;
					mpp_ch_aenc.s32ChnId = aenc_ch;
					SOC_CHECK(HI_MPI_SYS_Bind(&mpp_ch_ai, &mpp_ch_aenc));

					return 0;	
				}
			}
		}
	}
	return -1;
}

static int enc_release_stream_g711a(int ain)
{
	if(ain < HI_AENC_CH_BACKLOG_REF){
		int const aenc_ch = ain;
		LP_SDK_ENC_STREAM_G711A_ATTR const stream_g711_attr = &_sdk_enc.attr.g711_attr[ain];
		if(0 != stream_g711_attr->packet_size){
			// unbind AENC
			MPP_CHN_S mpp_ch_ai, mpp_ch_aenc;

			mpp_ch_ai.enModId  = HI_ID_AI;
			mpp_ch_ai.s32DevId = HI_AIN_DEV;
			mpp_ch_ai.s32ChnId = ain;

			mpp_ch_aenc.enModId  = HI_ID_AENC;
			mpp_ch_aenc.s32DevId = 0;
			mpp_ch_aenc.s32ChnId = aenc_ch;

			SOC_CHECK(HI_MPI_SYS_UnBind(&mpp_ch_ai, &mpp_ch_aenc));
			// destroy aenc chn
			SOC_CHECK(HI_MPI_AENC_DestroyChn(aenc_ch));
			// disable vin
			//SOC_CHECK(HI_MPI_AI_DisableChn(HI_AIN_DEV, ain));

			// clear flag
			stream_g711_attr->packet_size = 0;
			return 0;
		}
	}
	return -1;
}

static int enc_enable_stream_h264(int vin, int stream, bool flag)
{
	LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
	if(STREAM_H264_ISSET(streamH264Attr)){
		if(flag){
			SOC_CHECK(HI_MPI_VENC_StartRecvPic(__HI_VENC_CH(vin, stream)));
		}else{
			SOC_CHECK(HI_MPI_VENC_StopRecvPic(__HI_VENC_CH(vin, stream)));
		}
		return 0;
	}
	return -1;
}


static int enc_enable_stream_h265(int vin, int stream, bool flag)
{
	return 0;
}

static int enc_request_stream_h264_keyframe(int vin, int stream)
{
	SOC_CHECK(HI_MPI_VENC_RequestIDR(__HI_VENC_CH(vin, stream), HI_TRUE));
	return 0;
}
static int enc_request_stream_h265_keyframe(int vin, int stream)
{
	return 0;
}

static inline ssize_t _stream_size(VENC_STREAM_S venc_stream)
{
	int i = 0;
	ssize_t stream_size = 0;
	for(i = 0; i < venc_stream.u32PackCount; ++i){
		stream_size += venc_stream.pstPack[i].u32Len-venc_stream.pstPack[i].u32Offset;
	}
	return stream_size;
}

static int enc_video_proc(int vin, int stream)
{
	
#define HI3521_MAX_VENC_PACK (5)

	LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &(_sdk_enc.attr.h264_attr[vin][stream]);
	LP_SDK_ENC_STREAM_H265_ATTR const streamH265Attr = &(_sdk_enc.attr.h265_attr[vin][stream]);

	uint8_t *const ref_counter = &(_sdk_enc.attr.frame_ref_counter[vin][stream]);
	int const venc_ch = vin * HI_VENC_STREAM_BACKLOG_REF + stream;
	if(STREAM_H264_ISSET(streamH264Attr) || STREAM_H264_ISSET(streamH265Attr) ){
		VENC_CHN_STAT_S venc_chn_stat;
		venc_chn_stat.u32CurPacks = HI3521_MAX_VENC_PACK + 1;
		if(HI_SUCCESS == HI_MPI_VENC_Query(venc_ch, &venc_chn_stat)){
			
			if(venc_chn_stat.u32CurPacks > 0){				
				stSDK_ENC_BUF_ATTR attr;
				VENC_PACK_S venc_pack[HI3521_MAX_VENC_PACK];
				VENC_STREAM_S venc_stream;
				bool is_keyframe = false;
				// get media stream
				memset(&venc_stream, 0, sizeof(venc_stream));
				venc_stream.u32PackCount = HI3521_MAX_VENC_PACK;
				venc_stream.pstPack = venc_pack;

			
				
				SOC_CHECK(HI_MPI_VENC_GetStream(venc_ch, &venc_stream, HI_TRUE));

				switch(_sdk_enc.attr.enType[vin][stream]){	
					default:
					case kSDK_ENC_BUF_DATA_H264:
						//is_keyframe = (HI3521_MAX_VENC_PACK == venc_stream.u32PackCount);
						is_keyframe = (H264E_NALU_ISLICE == venc_pack[venc_chn_stat.u32CurPacks-1].DataType.enH264EType);
			
						if(is_keyframe){					
							*ref_counter = 0;
						}else{
							// h264 enc is x4 reference
							*ref_counter += 1;
							switch(HI_VENC_H264_REF_MODE){
								case H264E_REFSLICE_FOR_1X:
									*ref_counter %= 1;
									break;
								case H264E_REFSLICE_FOR_2X:
									*ref_counter %= 2;
									break;
								case H264E_REFSLICE_FOR_4X:
									*ref_counter %= 4;
									break;
							}
						}
			
						if(1){//if(0 == streamH264Attr->ref_counter || 2 == streamH264Attr->ref_counter){
							// buffering strream
							attr.magic = kSDK_ENC_BUF_DATA_MAGIC;
							attr.type = kSDK_ENC_BUF_DATA_H264;
							attr.timestamp_us = venc_stream.pstPack->u64PTS; // get the first nalu pts
							attr.time_us = get_time_us();
							attr.data_sz = _stream_size(venc_stream);
							attr.h264.keyframe = is_keyframe;
							attr.h264.ref_counter = *ref_counter;
							attr.h264.fps = streamH264Attr->fps;
							attr.h264.width = streamH264Attr->width;
							attr.h264.height = streamH264Attr->height;
							_sdk_enc.attr.u64encPTS[vin][stream] = attr.timestamp_us;

                            /* p2p‘§¿¿ π”√“˝”√ƒ⁄¥Êµƒ∑Ω Ω£¨À˘“‘º”…œp2p÷°Õ∑ */
							attr.p2pFrameHead.magic = 0x4652414d;
                            attr.p2pFrameHead.magic2 = 0x4652414E;
                            attr.p2pFrameHead.frame_seq = venc_stream.u32Seq;
							attr.p2pFrameHead.version = 0x01000000;
                            attr.p2pFrameHead.headtype = 0;
							attr.p2pFrameHead.framesize = attr.data_sz;
                            attr.p2pFrameHead.ts_ms = attr.time_us / 1000;
							attr.p2pFrameHead.live.frametype = is_keyframe ? 1 : 2;
                            attr.p2pFrameHead.live.channel = vin;
                            snprintf(attr.p2pFrameHead.live.v.enc, sizeof(attr.p2pFrameHead.live.v.enc), "%s", "H264");
							attr.p2pFrameHead.live.v.height = streamH264Attr->height;
							attr.p2pFrameHead.live.v.width = streamH264Attr->width;
                            attr.p2pFrameHead.live.v.fps = streamH264Attr->fps;

							if(0 == do_buffer_request(streamH264Attr->buf_id, attr.data_sz + sizeof(attr), attr.h264.keyframe, attr.timestamp_us)){	
								int i = 0;
								// buffer in the attribute
								
								do_buffer_append(streamH264Attr->buf_id, &attr, sizeof(attr));
								// buffer in the payload
								for(i = 0; i < venc_stream.u32PackCount; ++i){
									do_buffer_append(streamH264Attr->buf_id, venc_stream.pstPack[i].pu8Addr + venc_stream.pstPack[i].u32Offset, 
										venc_stream.pstPack[i].u32Len-venc_stream.pstPack[i].u32Offset);

								}
								do_buffer_commit(streamH264Attr->buf_id);
							}
						}
									
						break;
					case kSDK_ENC_BUF_DATA_H265:
						break;
				}
				SOC_CHECK(HI_MPI_VENC_ReleaseStream(venc_ch, &venc_stream));
				return 0;
			}
		}
	}
	return -1;
}

static int enc_audio_proc(int ain)
{
	int i = 0;
	LP_SDK_ENC_STREAM_G711A_ATTR const stream_g711_attr = &_sdk_enc.attr.g711_attr[ain];
	if(0 != stream_g711_attr->packet_size){
		AUDIO_STREAM_S audio_stream = {0};
		if(HI_SUCCESS == HI_MPI_AENC_GetStream(ain, &audio_stream, HI_IO_NOBLOCK)){
			stSDK_ENC_BUF_ATTR attr;
			attr.magic = kSDK_ENC_BUF_DATA_MAGIC;
			if(audio_type == AENC_CODEC_TYPE_AAC){
				attr.type = kSDK_ENC_BUF_DATA_AAC;
				attr.data_sz = audio_stream.u32Len;
			}else if(audio_type == AENC_CODEC_TYPE_G711A){
				attr.type = kSDK_ENC_BUF_DATA_G711A;
				attr.data_sz = audio_stream.u32Len - 4; 
			}
			attr.timestamp_us = audio_stream.u64TimeStamp; // get the first nalu pts
			attr.time_us = get_time_us();
			attr.g711a.sample_rate = stream_g711_attr->sample_rate;
			attr.g711a.sample_width = stream_g711_attr->sample_width;
			attr.g711a.packet = attr.data_sz;
			attr.g711a.compression_ratio = 2.0;

			/* p2p‘§¿¿ π”√“˝”√ƒ⁄¥Êµƒ∑Ω Ω£¨À˘“‘º”…œp2p÷°Õ∑ */
			attr.p2pFrameHead.magic = 0x4652414d;
            attr.p2pFrameHead.magic2 = 0x4652414E;
            attr.p2pFrameHead.frame_seq = 0;    // Èü≥È¢ë‰∏çÈúÄË¶ÅÂ∏ßÂ∫èÂè∑
			attr.p2pFrameHead.version = 0x01000000;
            attr.p2pFrameHead.headtype = 0;
			attr.p2pFrameHead.framesize = attr.data_sz;
            attr.p2pFrameHead.ts_ms = attr.time_us / 1000;
			attr.p2pFrameHead.live.frametype = 0; // audio
			attr.p2pFrameHead.live.channel = ain;
			if(audio_type == AENC_CODEC_TYPE_AAC){
				snprintf(attr.p2pFrameHead.live.a.enc, sizeof(attr.p2pFrameHead.live.a.enc), "%s", "AAC1"); // –¬P2P π”√AAC1«¯∑÷
			}else if(audio_type == AENC_CODEC_TYPE_G711A){
				snprintf(attr.p2pFrameHead.live.a.enc, sizeof(attr.p2pFrameHead.live.a.enc), "%s", "G711A1"); // –¬P2P π”√G711A1«¯∑÷
			}
			attr.p2pFrameHead.live.a.samplerate = stream_g711_attr->sample_rate;
			attr.p2pFrameHead.live.a.samplewidth = stream_g711_attr->sample_width;
            attr.p2pFrameHead.live.a.channels = 1;
            attr.p2pFrameHead.live.a.compress = attr.g711a.compression_ratio;
			// relative to the video stream channel
			// store the audio frame to every relevant video stream
			for(i = 0; i < HI_VENC_STREAM_BACKLOG_REF; ++i){
				LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[stream_g711_attr->vin_ref][i];
				int const buf_id = streamH264Attr->buf_id;
				if(0 == do_buffer_request(buf_id, sizeof(stSDK_ENC_BUF_ATTR) + attr.data_sz, false, attr.timestamp_us)){
					do_buffer_append(buf_id, &attr, sizeof(attr));
					//SOC_DEBUG("audio len = %d", audio_stream.u32Len);
					// if the g711a packet len is 320
					// the former 4bytes == 0x00a00100
					// if the g711a packet len is 480
					// the former 4bytes == 0x00f00100
					//uint32_t* head = audio_stream.pStream;
					//SOC_DEBUG("%08x", *head);
					
					if(audio_type == AENC_CODEC_TYPE_AAC){
						do_buffer_append(buf_id, audio_stream.pStream, audio_stream.u32Len);
					}else if(audio_type == AENC_CODEC_TYPE_G711A){
						do_buffer_append(buf_id, audio_stream.pStream + 4, audio_stream.u32Len - 4);
					}
					do_buffer_commit(buf_id);
				}
			}
			/*
			static FILE *fid = NULL;
			if(fid){
				static int count = 1000;
				if(count --){
					fwrite(audio_stream.pStream + 4, 1, audio_stream.u32Len - 4, fid);
					SOC_INFO("writing %d @ %d", audio_stream.u32Len - 4, 1000 - count);
				}else{
					fclose(fid);
					exit(1);
					//fid = NULL;
				}
			}else{
				fid = fopen("./audio.g711a", "w+b");
			}
			//*/
			
			SOC_CHECK(HI_MPI_AENC_ReleaseStream(ain, &audio_stream));
			return 0;
		}
	}
	return -1;
}


static void* enc_loop(void* arg)
{
	int i = 0, ii = 0;
	prctl(PR_SET_NAME, "enc_loop");
	while(_sdk_enc.attr.loop_trigger){
		// audio stream
		_sdk_enc.attr.ref_count = 1;
		for(i = 0; i < HI_AENC_CH_BACKLOG_REF; ++i){
			while(0 == enc_audio_proc(i));
		}
		// main stream first
		for(i = 0; i < HI_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < HI_VENC_STREAM_BACKLOG_REF; ++ii){
				while(0 == enc_video_proc(i, ii));
			}
		}
		_sdk_enc.attr.ref_count = 0;
		//usleep(30000);
		usleep(10000);
	}
	pthread_exit(NULL);
}

static int enc_start()
{
//	sdk_enc->creat_snapshot_chn(0,VENC_MAX_CHN_NUM - 1,640,360);
	
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

//	sdk_enc->release_snapshot_chn(0,VENC_MAX_CHN_NUM - 1);

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
#if 1
	if(vin < HI_VENC_CH_BACKLOG_REF){ 	
		pthread_mutex_lock(&_sdk_enc.attr.snapshot_mutex);
		int vencChannelJPEG;

		if((kSDK_ENC_SNAPSHOT_SIZE_MAX == width) && (kSDK_ENC_SNAPSHOT_SIZE_MAX == height)){
			vencChannelJPEG = VENC_MAX_CHN_NUM - 2;
		}else if((640 == width) && (360 == height)){
			vencChannelJPEG = VENC_MAX_CHN_NUM - 1;
		}else{
			vencChannelJPEG = VENC_MAX_CHN_NUM - 1;
			width = 640;
			height = 360;
		}
		SOC_INFO(" JPEG %dx%d", width, height);
		SOC_CHECK(HI_MPI_VENC_StartRecvPic(vencChannelJPEG));

		do {
			int i = 0;
			int ret = 0;
			struct timeval select_timeo = { .tv_sec  = 2, .tv_usec = 0, };
			fd_set rfd_set;
			VENC_CHN_STAT_S venc_chn_stat;
			HI_S32 const venc_jpeg_fd = HI_MPI_VENC_GetFd(vencChannelJPEG);

			FD_ZERO(&rfd_set);
			FD_SET(venc_jpeg_fd, &rfd_set);
			ret = select(venc_jpeg_fd + 1, &rfd_set, NULL, NULL, &select_timeo);
			if(ret < 0){
				SOC_DEBUG("Snapshot select failed!");
			}else if (0 == ret){
				SOC_DEBUG("Snapshot select timeout!");
			}else{
				if(FD_ISSET(venc_jpeg_fd, &rfd_set)){
					SOC_CHECK(HI_MPI_VENC_Query(vencChannelJPEG, &venc_chn_stat));
					// here you must keep the u32CurPacks not zero
					if(venc_chn_stat.u32CurPacks > 0){
						VENC_STREAM_S venc_stream;
						venc_stream.u32PackCount = venc_chn_stat.u32CurPacks;
						venc_stream.pstPack = (VENC_PACK_S*)alloca(sizeof(VENC_PACK_S) * venc_stream.u32PackCount);
						SOC_CHECK(HI_MPI_VENC_GetStream(vencChannelJPEG, &venc_stream, HI_IO_NOBLOCK));
						
						if(stream){
						for (i = 0; i < venc_stream.u32PackCount; ++i){							
								fwrite(venc_stream.pstPack[i].pu8Addr, 1, venc_stream.pstPack[i].u32Len, stream) ;								
							}	
							fflush(stream);
						}
						SOC_CHECK(HI_MPI_VENC_ReleaseStream(vencChannelJPEG, &venc_stream));
					}
				}
			}
		}while(0);

		// destruct
		SOC_CHECK(HI_MPI_VENC_StopRecvPic(vencChannelJPEG));
		pthread_mutex_unlock(&_sdk_enc.attr.snapshot_mutex);
		return 0;
	}
#endif
	return -1;
}

static inline uint16_t overlay_pixel_argb4444(stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel)
{
	return ((pixel.alpha>>4)<<12)|((pixel.red>>4)<<8)|((pixel.green>>4)<<4)|((pixel.blue>>4)<<0);
}

static inline stSDK_ENC_VIDEO_OVERLAY_PIXEL overlay_pixel_argb8888(uint16_t pixel)
{
	stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel_8888;
	pixel_8888.alpha = ((pixel>>12)<<4) & 0xff;
	pixel_8888.red = ((pixel>>8)<<4) & 0xff;
	pixel_8888.green = ((pixel>>4)<<4) & 0xff;
	pixel_8888.blue = ((pixel>>0)<<4) & 0xff;
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
				// has not been allocated
				LP_SDK_ENC_VIDEO_OVERLAY_CANVAS const canvas = &canvas_stock[i];

				canvas->width = SDK_ALIGNED_BIG_ENDIAN(width, 2); // aligned to 2 pixel
				canvas->height = SDK_ALIGNED_BIG_ENDIAN(height, 2);
				// hisilicon use argb444 format
				canvas->pixel_format.rmask = 0x0f00;
				canvas->pixel_format.gmask = 0x00f0;
				canvas->pixel_format.bmask = 0x000f;
				canvas->pixel_format.amask = 0xf000;
				// frame buffer
		//  		canvas->pixels = calloc(canvas->width * canvas->height * sizeof(uint16_t), 1);
				HI_MPI_SYS_MmzAlloc(&canvas->phy_addr, (void**)(&canvas->pixels),
					NULL, NULL,canvas->width * canvas->height * sizeof(uint16_t));
				// interfaces
				canvas->put_pixel = overlay_canvas_put_pixel;
				canvas->get_pixel = overlay_canvas_get_pixel;
				canvas->match_pixel = overlay_canvas_match_pixel;
				canvas->put_rect = overlay_canvas_put_rect;
				canvas->fill_rect = overlay_canvas_fill_rect;
				canvas->erase_rect = overlay_canvas_erase_rect;
				return canvas;
			}
		}
	}
	return NULL;
}

static LP_SDK_ENC_VIDEO_OVERLAY_CANVAS enc_load_overlay_canvas(const char *bmp24_path)
{
	int i = 0, ii = 0;
	int ret = 0;
	typedef struct BIT_MAP_FILE_HEADER	{
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
	
	FILE *bmp_fid = NULL;

	bmp_fid = fopen(bmp24_path, "rb");
	if(NULL != bmp_fid){
		BIT_MAP_FILE_HEADER_t bmp_hdr;
		ret = fread(&bmp_hdr, 1, sizeof(bmp_hdr), bmp_fid);

		if(sizeof(bmp_hdr) == ret){
			if('B' == bmp_hdr.type[0]
				&& 'M' == bmp_hdr.type[1]
				&& 24 == bmp_hdr.bit_count){
				
				int const bmp_width = bmp_hdr.width;
				int const bmp_height = bmp_hdr.height;
				char *canvas_cache = calloc(bmp_hdr.size_image, 1);
				
				LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas = NULL;
				stSDK_ENC_VIDEO_OVERLAY_PIXEL canvas_pixel;

				SOC_INFO("IMAGE %dx%d size=%d offset=%d info=%d", bmp_width, bmp_height, bmp_hdr.size_image, bmp_hdr.off_bits, bmp_hdr.info_size);


				// load image to buf
				if(0 == fseek(bmp_fid, bmp_hdr.off_bits, SEEK_SET)){
					ret = fread(canvas_cache, 1, bmp_hdr.size_image, bmp_fid);
				}
				fclose(bmp_fid);
				bmp_fid = NULL;

				// load to canvas
				//canvas_pixel.argb8888 = 0xffffffff;
				canvas = sdk_enc->create_overlay_canvas(bmp_width, bmp_height);
				for(i = 0; i < bmp_height; ++i){
					char *const line_offset = canvas_cache + SDK_ALIGNED_BIG_ENDIAN(3 * bmp_width, 4) * (bmp_height - 1 - i) + 2;
					for(ii = 0; ii < bmp_width; ++ii){
						char *const column_offset = line_offset + 3 * ii;

						canvas_pixel.alpha = 0xff;
						canvas_pixel.red = column_offset[0];
						canvas_pixel.green = column_offset[1];
						canvas_pixel.blue = column_offset[2];

						canvas->put_pixel(canvas, ii, i, canvas_pixel);
					}
				}
				
				//canvas->fill_rect(canvas, 0, 0, bmp_width, bmp_height, canvas_pixel);

				// free the canvas cache
				free(canvas_cache);
				canvas_cache = NULL;

				return canvas;
			}
		}

		fclose(bmp_fid);
		bmp_fid = NULL;
	}
	return NULL;
	
}


static void enc_release_overlay_canvas(LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas)
{
	if(canvas){
		canvas->width = 0;
		canvas->height = 0;
		if(canvas->pixels){
		//	free(canvas->pixels);		
			SOC_CHECK(HI_MPI_SYS_MmzFree(canvas->phy_addr,canvas->pixels));
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

HI_S32 enc_rgn_add_reverse_color_task(TDE_HANDLE handle, 
    TDE2_SURFACE_S *pstForeGround, TDE2_RECT_S *pstForeGroundRect, 
    TDE2_SURFACE_S *pstBackGround, TDE2_RECT_S *pstBackGroundRect)
{
    HI_S32 s32Ret;
    TDE2_OPT_S stOpt = {0};
    
    HI_ASSERT(NULL != pstForeGround);
    HI_ASSERT(NULL != pstForeGroundRect);
    HI_ASSERT(NULL != pstBackGround);
    HI_ASSERT(NULL != pstBackGroundRect);

    stOpt.enAluCmd        = TDE2_ALUCMD_ROP;
    stOpt.enRopCode_Alpha = TDE2_ROP_COPYPEN;
    stOpt.enRopCode_Color = TDE2_ROP_NOT;
    
    s32Ret =  HI_TDE2_Bitblit(handle, pstBackGround, pstBackGroundRect, pstForeGround, 
          pstForeGroundRect, pstBackGround, pstBackGroundRect, &stOpt);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_TDE2_Bitblit fail! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
    
}


   
HI_S32 enc_rgn_reverse_osd_color_tde(TDE2_SURFACE_S *pstSrcSurface, TDE2_SURFACE_S *pstDstSurface, 
		const VPSS_REGION_INFO_S *pstRgnInfo)

{
	HI_S32 i;
	HI_S32 s32Ret;
	TDE_HANDLE handle;
	TDE2_RECT_S stRect;


	HI_ASSERT(NULL != pstSrcSurface);
	HI_ASSERT(NULL != pstDstSurface);
	HI_ASSERT(NULL != pstRgnInfo);

	s32Ret = HI_TDE2_Open();
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_TDE2_Open fail! s32Ret: 0x%x.\n", s32Ret);
		return s32Ret;
	}

	handle = HI_TDE2_BeginJob();
	if (handle < 0)
	{
		printf("HI_TDE2_BeginJob fail!\n");
		return HI_FAILURE;
	}

	stRect.s32Xpos = 0;
	stRect.s32Ypos = 0;
	stRect.u32Width  = pstSrcSurface->u32Width;
	stRect.u32Height = pstSrcSurface->u32Height;
	s32Ret = HI_TDE2_QuickCopy(handle, pstSrcSurface, &stRect, pstDstSurface, &stRect);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_TDE2_QuickCopy fail! s32Ret: 0x%x.\n", s32Ret);
		HI_TDE2_CancelJob(handle);
		return s32Ret;
	}

	for (i = 0; i < pstRgnInfo->u32RegionNum; ++i)
	{
		stRect.s32Xpos	 = pstRgnInfo->pstRegion[i].s32X;
		stRect.s32Ypos	 = pstRgnInfo->pstRegion[i].s32Y;
		stRect.u32Width  = pstRgnInfo->pstRegion[i].u32Width;
		stRect.u32Height = pstRgnInfo->pstRegion[i].u32Height;

		s32Ret = enc_rgn_add_reverse_color_task(handle, pstSrcSurface, &stRect, pstDstSurface, &stRect);
		if (HI_SUCCESS != s32Ret)
		{
			printf("enc_rgn_add_reverse_color_task fail! s32Ret: 0x%x.\n", s32Ret);
			HI_TDE2_CancelJob(handle);
			return s32Ret;
		}
	}

	s32Ret = HI_TDE2_EndJob(handle, HI_FALSE, HI_FALSE, 5);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_TDE2_EndJob fail! s32Ret: 0x%x.\n", s32Ret);
		HI_TDE2_CancelJob(handle);
		return s32Ret;
	}

	s32Ret = HI_TDE2_WaitForDone(handle);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_TDE2_WaitForDone fail! s32Ret: 0x%x.\n", s32Ret);
		return s32Ret;
	}

	return HI_SUCCESS;
}


HI_S32 enc_rgn_conv_osd_cavas_to_tde_surface(TDE2_SURFACE_S *pstSurface, const RGN_CANVAS_INFO_S *pstCanvasInfo)
	
{
    HI_ASSERT((NULL != pstSurface) && (NULL != pstCanvasInfo));
    
    switch (pstCanvasInfo->enPixelFmt)
    {
        case PIXEL_FORMAT_RGB_4444:
        {
            pstSurface->enColorFmt = TDE2_COLOR_FMT_ARGB4444;
            break ;
        }
        case PIXEL_FORMAT_RGB_1555:
        {
            pstSurface->enColorFmt = TDE2_COLOR_FMT_ARGB1555;
            break ;
        }
        case PIXEL_FORMAT_RGB_8888:
        {
            pstSurface->enColorFmt = TDE2_COLOR_FMT_ARGB8888;
            break ;
        }
        default :
        {
            printf("[Func]:%s [Line]:%d [Info]:invalid Osd pixel format(%d)\n", 
                __FUNCTION__, __LINE__, pstCanvasInfo->enPixelFmt);
            return HI_FAILURE;
        }
    }

    pstSurface->bAlphaExt1555 = HI_FALSE;
    pstSurface->bAlphaMax255  = HI_TRUE;
    pstSurface->u32PhyAddr    = pstCanvasInfo->u32PhyAddr;
    pstSurface->u32Width      = pstCanvasInfo->stSize.u32Width;
    pstSurface->u32Height     = pstCanvasInfo->stSize.u32Height;
    pstSurface->u32Stride     = pstCanvasInfo->u32Stride;

    return HI_SUCCESS;
}

HI_VOID *enc_rgn_vpss_osd_reverse_thread()
{	
	pthread_detach(pthread_self());
	
	usleep(1000*1000);
	int handle_num = 0;
	
	while (HI_FALSE == bExitOverlayLoop){		
		while((handle_num < _sdk_enc.attr.overlay_handle_num) && (HI_FALSE == bExitOverlayRelease) ){			
			if(NULL == _sdk_enc.attr.overlay[handle_num] ){
					handle_num ++ ;				
					continue ;			
			}else{	
				if(0 == strcmp("clock",_sdk_enc.attr.overlay[handle_num]->name)){				
					handle_num ++ ;
					continue;
				}
			}
			RGN_ATTR_S stRgnAttrSet;
			RGN_CHN_ATTR_S stChnAttr;
			HI_S32 i,VpssChn,VpssChn_sub;
			HI_S32 k = 0, j = 0;

			RGN_HANDLE Handle;
			RGN_HANDLE Handle_sub;

			HI_U32 u32OsdRectCnt;
			SIZE_S stSize;
			RGN_OSD_REVERSE_INFO_S stOsdReverseInfo;    

			RECT_S astOsdLumaRect[64];	 
			RECT_S astOsdRevRect[OSD_REVERSE_RGN_MAXCNT];
			HI_U32 au32LumaData[OSD_REVERSE_RGN_MAXCNT];			
			HI_S32 s32Ret = HI_SUCCESS;
					
			TDE2_SURFACE_S stRgnOrignSurface = {0};
			TDE2_SURFACE_S stRgnSurface = {0};
			RGN_CANVAS_INFO_S stCanvasInfo;
			VPSS_REGION_INFO_S stReverseRgnInfo;
			
			MPP_CHN_S stChn;
			MPP_CHN_S stMppChn =  {0};			
			RGN_CHN_ATTR_S stOsdChnAttr = {0};
			lpSDK_ENC_VIDEO_OVERLAY_ATTR  overlay;
		
			int width_max;			
			VPSS_CHN_MODE_S stVpssMode;
			VPSS_CROP_INFO_S stCropInfo;
			int stream_width;
			HI_U32 per_width;
			
			memset(&stOsdReverseInfo, 0, sizeof(stOsdReverseInfo));
		
			Handle                                   = _sdk_enc.attr.overlay_handle[handle_num];
			VpssChn                                  = _sdk_enc.attr.vpss_chn[handle_num];			
			u32OsdRectCnt							 = 16;//32
			stOsdReverseInfo.Handle 				 = Handle;
			stOsdReverseInfo.VpssGrp				 = 0;
			stOsdReverseInfo.VpssChn				 = 2;//VpssChn;
			stOsdReverseInfo.u8PerPixelLumaThrd 	 = 128;
			stOsdReverseInfo.stLumaRgnInfo.u32RegionNum   = u32OsdRectCnt;
			stOsdReverseInfo.stLumaRgnInfo.pstRegion = astOsdLumaRect;


			if(1 == VpssChn){//main stream
				VpssChn_sub = VpssChn + 1;
				Handle_sub = _sdk_enc.attr.overlay_handle[handle_num + 1];
			}else if (2 == VpssChn){//sub stream  
				VpssChn_sub = VpssChn;
				Handle_sub = Handle;
			}
			if(0 == Handle_sub){
				return -1;
			}
			pthread_mutex_lock(&_sdk_enc.attr.overlayex_mutex);
			SOC_CHECK(HI_MPI_RGN_GetAttr(Handle_sub, &stRgnAttrSet));
			stSize.u32Width  = stRgnAttrSet.unAttr.stOverlay.stSize.u32Width;
			stSize.u32Height = stRgnAttrSet.unAttr.stOverlay.stSize.u32Height;

			stChn.enModId  = HI_ID_VENC;
			stChn.s32DevId = 0;
			stChn.s32ChnId = _hi3518_venc_ch_map[VpssChn_sub]; // 1
			SOC_CHECK(HI_MPI_RGN_GetDisplayAttr(Handle_sub, &stChn, &stChnAttr));
			
			SOC_CHECK(HI_MPI_VPSS_GetChnCrop(0,VpssChn_sub,&stCropInfo));
			SOC_CHECK(HI_MPI_VPSS_GetChnMode(0,VpssChn_sub,&stVpssMode));

			if(stCropInfo.bEnable == 1 ){
				stream_width = stCropInfo.stCropRect.u32Width;				
			}else{
				stream_width = stVpssMode.u32Width ;
			}
			
			per_width = stSize.u32Height;
			u32OsdRectCnt = stSize.u32Width / per_width;
			stOsdReverseInfo.stLumaRgnInfo.u32RegionNum   = u32OsdRectCnt;	
			
			for (i=0;i < u32OsdRectCnt; i++)
			{			  
				width_max = (per_width * (i+1)) + stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X;
			    if( width_max > stream_width){		
			   		stOsdReverseInfo.stLumaRgnInfo.u32RegionNum = i;
					break;			   	
			   	}
				
				astOsdLumaRect[i].s32X = (per_width * i) + stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X;
				astOsdLumaRect[i].s32Y = stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y;
				astOsdLumaRect[i].u32Width  = per_width;
				astOsdLumaRect[i].u32Height = stSize.u32Height;
			}
			
			overlay = _sdk_enc.attr.overlay[handle_num];
			

			SOC_CHECK(HI_MPI_RGN_GetCanvasInfo(Handle, &stCanvasInfo));
						
			if(NULL == overlay || (NULL != overlay)&&(NULL == overlay->canvas)){
				SOC_CHECK(HI_MPI_RGN_UpdateCanvas(Handle));
				handle_num ++ ;
				pthread_mutex_unlock(&_sdk_enc.attr.overlayex_mutex);
				continue ;

			}else{
				memcpy(stCanvasInfo.u32VirtAddr,overlay->canvas->pixels,
				overlay->canvas->width * overlay->canvas->height * sizeof(uint16_t));
				stCanvasInfo.stSize.u32Height = overlay->canvas->height;
				stCanvasInfo.stSize.u32Width  = overlay->canvas->width;
			}

			SOC_CHECK(enc_rgn_conv_osd_cavas_to_tde_surface(&stRgnSurface, &stCanvasInfo));
		    
			stRgnOrignSurface.enColorFmt = TDE2_COLOR_FMT_ARGB4444;
		    stRgnOrignSurface.bAlphaExt1555 = HI_FALSE;
		    stRgnOrignSurface.bAlphaMax255  = HI_TRUE;
		    stRgnOrignSurface.u32PhyAddr    = overlay->canvas->phy_addr;
		    stRgnOrignSurface.u32Width      = stCanvasInfo.stSize.u32Width;
		    stRgnOrignSurface.u32Height     = stCanvasInfo.stSize.u32Height;
		    stRgnOrignSurface.u32Stride     = stCanvasInfo.u32Stride;

			/* 3.get the  display attribute of OSD attached to vpss*/

			stMppChn.enModId  = HI_ID_VENC;
			stMppChn.s32DevId = 0;
			stMppChn.s32ChnId = _hi3518_venc_ch_map[VpssChn];//1;

			s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle, &stMppChn, &stOsdChnAttr);

			stReverseRgnInfo.pstRegion = (RECT_S *)astOsdRevRect;	
	
			HI_U32 interval_size;

			/* 4.get the sum of luma of a region specified by user*/
			s32Ret = HI_MPI_VPSS_GetRegionLuma(stOsdReverseInfo.VpssGrp, stOsdReverseInfo.VpssChn, &(stOsdReverseInfo.stLumaRgnInfo), au32LumaData, -1);
	        if (HI_SUCCESS != s32Ret)
		    {
				printf("[Func]:%s [Line]:%d [Info]:HI_MPI_VPSS_GetRegionLuma VpssGrp=%d failed, s32Ret: 0x%x,overlay name:%s.\n",
					   __FUNCTION__, __LINE__, stOsdReverseInfo.VpssGrp, s32Ret,_sdk_enc.attr.overlay[handle_num]->name);
			}	
			/* 5.decide which region to be reverse color according to the sum of the region*/
			for (k = 0, j = 0; k < stOsdReverseInfo.stLumaRgnInfo.u32RegionNum; ++k)
			{	
		
				if (au32LumaData[k] > (stOsdReverseInfo.u8PerPixelLumaThrd * 
					stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].u32Width * 
					stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].u32Height))
				{
					/* 6.get the regions to be reverse color */					
					if(1 == VpssChn){		
						SOC_CHECK(HI_MPI_RGN_GetAttr(Handle, &stRgnAttrSet));
						stSize.u32Width  = stRgnAttrSet.unAttr.stOverlay.stSize.u32Width;
						stSize.u32Height = stRgnAttrSet.unAttr.stOverlay.stSize.u32Height;
						interval_size = stSize.u32Height ;
				
						stReverseRgnInfo.pstRegion[j].s32X = interval_size *k;
						stReverseRgnInfo.pstRegion[j].s32Y = 0;
						stReverseRgnInfo.pstRegion[j].u32Width = interval_size; 	
						stReverseRgnInfo.pstRegion[j].u32Height = stSize.u32Height; 
			//			printf("main s32X=%d s32Y=%d u32Width=%d u32Height=%d\n",stReverseRgnInfo.pstRegion[j].s32X,
			//				stReverseRgnInfo.pstRegion[j].s32Y,stReverseRgnInfo.pstRegion[j].u32Width,
			//				stReverseRgnInfo.pstRegion[j].u32Height);
						++j;
					}else if (2 == VpssChn){
						stReverseRgnInfo.pstRegion[j].s32X = stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].s32X 
							- stOsdChnAttr.unChnAttr.stOverlayChn.stPoint.s32X;
						stReverseRgnInfo.pstRegion[j].s32Y = stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].s32Y 
							- stOsdChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y;
						stReverseRgnInfo.pstRegion[j].u32Width = stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].u32Width;
						stReverseRgnInfo.pstRegion[j].u32Height = stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].u32Height;	
			//			printf("sub s32X=%d s32Y=%d u32Width=%d u32Height=%d\n",stReverseRgnInfo.pstRegion[j].s32X,
			//				stReverseRgnInfo.pstRegion[j].s32Y,stReverseRgnInfo.pstRegion[j].u32Width,
			//				stReverseRgnInfo.pstRegion[j].u32Height);
						++j;
					}

				}
			}
			
			stReverseRgnInfo.u32RegionNum = j;
			if(NULL == overlay || (NULL != overlay)&&(NULL == overlay->canvas)){
				SOC_CHECK(HI_MPI_RGN_UpdateCanvas(Handle));
				handle_num ++ ;
				pthread_mutex_unlock(&_sdk_enc.attr.overlayex_mutex);
				continue ;
			}else{
				/* 8.reverse color */
#if 0				
				enc_rgn_reverse_osd_color_tde(&stRgnOrignSurface, &stRgnSurface, &stReverseRgnInfo);
#endif
			}
			// 9.update OSD 	
			SOC_CHECK(HI_MPI_RGN_UpdateCanvas(Handle));
			pthread_mutex_unlock(&_sdk_enc.attr.overlayex_mutex);
			handle_num++;
		}
		handle_num = 0;		
		usleep(500*1000);
	}
	pthread_exit(NULL);
	return 0;
}

static int enc_rgn_vpss_osd_reverse(int handle_num_id)
{	

	int handle_num = handle_num_id;
	RGN_ATTR_S stRgnAttrSet;
	RGN_CHN_ATTR_S stChnAttr;
	HI_S32 i,VpssChn,VpssChn_sub;
	HI_S32 k = 0, j = 0;

	RGN_HANDLE Handle;
	RGN_HANDLE Handle_sub;
	
	HI_U32 u32OsdRectCnt;
	SIZE_S stSize;
	RGN_OSD_REVERSE_INFO_S stOsdReverseInfo;	

	RECT_S astOsdLumaRect[64];	 
	RECT_S astOsdRevRect[OSD_REVERSE_RGN_MAXCNT];
	HI_U32 au32LumaData[OSD_REVERSE_RGN_MAXCNT];			
	HI_S32 s32Ret = HI_SUCCESS;
			
	TDE2_SURFACE_S stRgnOrignSurface = {0};
	TDE2_SURFACE_S stRgnSurface = {0};
	RGN_CANVAS_INFO_S stCanvasInfo;
	VPSS_REGION_INFO_S stReverseRgnInfo;

	MPP_CHN_S stChn;
	MPP_CHN_S stMppChn =  {0};			
	RGN_CHN_ATTR_S stOsdChnAttr = {0};
	lpSDK_ENC_VIDEO_OVERLAY_ATTR  overlay;

	int width_max;			
	VPSS_CHN_MODE_S stVpssMode;
	VPSS_CROP_INFO_S stCropInfo;
	int stream_width;
	HI_U32 per_width;

	if(NULL == _sdk_enc.attr.overlay[handle_num]){
		return -1 ;			
	}else{			
		overlay = _sdk_enc.attr.overlay[handle_num];
	}
	
	memset(&stOsdReverseInfo, 0, sizeof(stOsdReverseInfo));

	Handle									 = _sdk_enc.attr.overlay_handle[handle_num];
	VpssChn 								 = _sdk_enc.attr.vpss_chn[handle_num];			
	u32OsdRectCnt							 = 16;//32;
	stOsdReverseInfo.Handle 				 = Handle;
	stOsdReverseInfo.VpssGrp				 = 0;
	stOsdReverseInfo.VpssChn				 = 2;
	stOsdReverseInfo.u8PerPixelLumaThrd 	 = 128;
	stOsdReverseInfo.stLumaRgnInfo.u32RegionNum   = u32OsdRectCnt;
	stOsdReverseInfo.stLumaRgnInfo.pstRegion = astOsdLumaRect;
	
	if(1 == VpssChn){//main stream
		VpssChn_sub = VpssChn + 1;
		Handle_sub = _sdk_enc.attr.overlay_handle[handle_num + 1];
	}else if (2 == VpssChn){//sub stream  
		VpssChn_sub = VpssChn;
		Handle_sub = Handle;
	}
	if(0 == Handle_sub){
		return -1;
	}
	
	
	pthread_mutex_lock(&_sdk_enc.attr.overlayex_mutex);
	SOC_CHECK(HI_MPI_RGN_GetAttr(Handle_sub, &stRgnAttrSet));
	stSize.u32Width  = stRgnAttrSet.unAttr.stOverlay.stSize.u32Width;
	stSize.u32Height = stRgnAttrSet.unAttr.stOverlay.stSize.u32Height;

	stChn.enModId  = HI_ID_VENC;
	stChn.s32DevId = 0;
	stChn.s32ChnId = _hi3518_venc_ch_map[VpssChn_sub]; // 1
	SOC_CHECK(HI_MPI_RGN_GetDisplayAttr(Handle_sub, &stChn, &stChnAttr));

	SOC_CHECK(HI_MPI_VPSS_GetChnCrop(0,VpssChn_sub,&stCropInfo));
	SOC_CHECK(HI_MPI_VPSS_GetChnMode(0,VpssChn_sub,&stVpssMode));

	if(stCropInfo.bEnable == 1 ){
		stream_width = stCropInfo.stCropRect.u32Width;				
	}else{
		stream_width = stVpssMode.u32Width ;
	}
	
	per_width = stSize.u32Height;
	u32OsdRectCnt = stSize.u32Width / per_width;
	stOsdReverseInfo.stLumaRgnInfo.u32RegionNum   = u32OsdRectCnt;
						
	for (i=0;i < u32OsdRectCnt; i++)
	{			  
		width_max = (per_width * (i+1)) + stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X;
		if( width_max > stream_width){		
			stOsdReverseInfo.stLumaRgnInfo.u32RegionNum = i;
			break;				
		}
		astOsdLumaRect[i].s32X = (per_width * i) + stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X;
		astOsdLumaRect[i].s32Y = stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y;
		astOsdLumaRect[i].u32Width	= per_width;
		astOsdLumaRect[i].u32Height = stSize.u32Height;
	//	printf("[%d] s32X=%d ,s32Y=%d ,u32Width=%d,u32Height=%d \n",i,astOsdLumaRect[i].s32X,astOsdLumaRect[i].s32Y,
	//		astOsdLumaRect[i].u32Width,astOsdLumaRect[i].u32Height);
	}

	SOC_CHECK(HI_MPI_RGN_GetCanvasInfo(Handle, &stCanvasInfo));
				
	if(NULL == overlay || (NULL != overlay)&&(NULL == overlay->canvas)){
		SOC_CHECK(HI_MPI_RGN_UpdateCanvas(Handle));	
		pthread_mutex_unlock(&_sdk_enc.attr.overlayex_mutex);
		return -1 ;
	}else{
		memcpy(stCanvasInfo.u32VirtAddr,overlay->canvas->pixels,
		overlay->canvas->width * overlay->canvas->height * sizeof(uint16_t));
		stCanvasInfo.stSize.u32Height = overlay->canvas->height;
		stCanvasInfo.stSize.u32Width  = overlay->canvas->width;
	}

	SOC_CHECK(enc_rgn_conv_osd_cavas_to_tde_surface(&stRgnSurface, &stCanvasInfo));

	stRgnOrignSurface.enColorFmt = TDE2_COLOR_FMT_ARGB4444;
	stRgnOrignSurface.bAlphaExt1555 = HI_FALSE;
	stRgnOrignSurface.bAlphaMax255	= HI_TRUE;
	stRgnOrignSurface.u32PhyAddr	= overlay->canvas->phy_addr;
	stRgnOrignSurface.u32Width		= stCanvasInfo.stSize.u32Width;
	stRgnOrignSurface.u32Height 	= stCanvasInfo.stSize.u32Height;
	stRgnOrignSurface.u32Stride 	= stCanvasInfo.u32Stride;

	/* 3.get the  display attribute of OSD attached to vpss*/
	stMppChn.enModId  = HI_ID_VENC;
	stMppChn.s32DevId = 0;
	stMppChn.s32ChnId = _hi3518_venc_ch_map[VpssChn];//1;
	s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle, &stMppChn, &stOsdChnAttr);

	stReverseRgnInfo.pstRegion = (RECT_S *)astOsdRevRect;	

	HI_U32 interval_size;
	
	/* 4.get the sum of luma of a region specified by user*/
	s32Ret = HI_MPI_VPSS_GetRegionLuma(stOsdReverseInfo.VpssGrp, stOsdReverseInfo.VpssChn, &(stOsdReverseInfo.stLumaRgnInfo), au32LumaData, -1);
	if (HI_SUCCESS != s32Ret)			
	{
		printf("[Func]:%s [Line]:%d [Info]:HI_MPI_VPSS_GetRegionLuma VpssGrp=%d failed, s32Ret: 0x%x,overlay name:%s.\n",
			   __FUNCTION__, __LINE__, stOsdReverseInfo.VpssGrp, s32Ret,_sdk_enc.attr.overlay[handle_num]->name);
	}
	/* 5.decide which region to be reverse color according to the sum of the region*/
	for (k = 0, j = 0; k < stOsdReverseInfo.stLumaRgnInfo.u32RegionNum; ++k)
	{	

		if (au32LumaData[k] > (stOsdReverseInfo.u8PerPixelLumaThrd * 
			stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].u32Width * 
			stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].u32Height))
		{
			/* 6.get the regions to be reverse color */
			
			if(1 == VpssChn){		
				SOC_CHECK(HI_MPI_RGN_GetAttr(Handle, &stRgnAttrSet));
				stSize.u32Width  = stRgnAttrSet.unAttr.stOverlay.stSize.u32Width;
				stSize.u32Height = stRgnAttrSet.unAttr.stOverlay.stSize.u32Height;
				interval_size = stSize.u32Height;				

				stReverseRgnInfo.pstRegion[j].s32X = interval_size *k;
				stReverseRgnInfo.pstRegion[j].s32Y = 0;
				stReverseRgnInfo.pstRegion[j].u32Width = interval_size;		
				stReverseRgnInfo.pstRegion[j].u32Height = stSize.u32Height; 
	//			printf("main s32X=%d s32Y=%d u32Width=%d u32Height=%d\n",stReverseRgnInfo.pstRegion[j].s32X,
	//				stReverseRgnInfo.pstRegion[j].s32Y,stReverseRgnInfo.pstRegion[j].u32Width,
	//				stReverseRgnInfo.pstRegion[j].u32Height);
				++j;
			}else if (2 == VpssChn){
				stReverseRgnInfo.pstRegion[j].s32X = stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].s32X 
					- stOsdChnAttr.unChnAttr.stOverlayChn.stPoint.s32X;
				stReverseRgnInfo.pstRegion[j].s32Y = stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].s32Y 
					- stOsdChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y;
				stReverseRgnInfo.pstRegion[j].u32Width = stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].u32Width;
				stReverseRgnInfo.pstRegion[j].u32Height = stOsdReverseInfo.stLumaRgnInfo.pstRegion[k].u32Height;	
	//			printf("sub s32X=%d s32Y=%d u32Width=%d u32Height=%d\n",stReverseRgnInfo.pstRegion[j].s32X,
	//				stReverseRgnInfo.pstRegion[j].s32Y,stReverseRgnInfo.pstRegion[j].u32Width,
	//				stReverseRgnInfo.pstRegion[j].u32Height);
				++j;
			}

		}
	}

	stReverseRgnInfo.u32RegionNum = j;
	if(NULL == overlay || (NULL != overlay)&&(NULL == overlay->canvas)){
		SOC_CHECK(HI_MPI_RGN_UpdateCanvas(Handle));		
		pthread_mutex_unlock(&_sdk_enc.attr.overlayex_mutex);
		return -1 ;

	}else{
		/* 8.reverse color */
#if 0 
		enc_rgn_reverse_osd_color_tde(&stRgnOrignSurface, &stRgnSurface, &stReverseRgnInfo);		
#endif
	}
	// 9.update OSD 
	SOC_CHECK(HI_MPI_RGN_UpdateCanvas(Handle));
	pthread_mutex_unlock(&_sdk_enc.attr.overlayex_mutex);

	return 0;
}


static int enc_create_overlay(int vin, int stream, const char* overlay_name,
		float x, float y, LP_SDK_ENC_VIDEO_OVERLAY_CANVAS const canvas)
{
	int i = 0;	
	static pthread_t stOsdReverseThread = NULL;
	
	if(NULL != canvas
		&& vin < HI_VENC_CH_BACKLOG_REF
		&& stream < HI_VENC_STREAM_BACKLOG_REF ){
		int canvas_x = 0, canvas_y = 0;
		size_t canvas_width = 0, canvas_height = 0;
		LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
		lpSDK_ENC_VIDEO_OVERLAY_ATTR_SET const overlay_set = &_sdk_enc.attr.video_overlay_set[vin][stream];
		
		// check name override
		if(NULL != enc_lookup_overlay_byname(vin, stream, overlay_name)){
			SOC_DEBUG("Overlay name %s override", overlay_name);
			return -1;
		}
		//printf("rect: %f/%f %d/%d\n", x, y, streamH264Attr->width, streamH264Attr->height);

		
		VPSS_CHN_MODE_S stVpssMode;
		VPSS_CROP_INFO_S stCropInfo;	
		HI_S32 VpssChn = _hi3518_vpss_ch_map[__HI_VENC_CH(vin, stream)];
		
		SOC_CHECK(HI_MPI_VPSS_GetChnCrop(0,VpssChn,&stCropInfo));
		SOC_CHECK(HI_MPI_VPSS_GetChnMode(vin,VpssChn,&stVpssMode));

		canvas_width = canvas->width;
		canvas_height = canvas->height;
		
		// width /height	
		if(stCropInfo.bEnable == 1){
			canvas_x = (typeof(canvas_x))(x * (float)(streamH264Attr->width) * stCropInfo.stCropRect.u32Width / stVpssMode.u32Width);			
			canvas_y = (typeof(canvas_y))(y * (float)(streamH264Attr->height) * stCropInfo.stCropRect.u32Height / stVpssMode.u32Height);

			if((canvas_x + canvas->width) > streamH264Attr->width){
				canvas_x = streamH264Attr->width - canvas->width;
			}			
			if((canvas_y + canvas_height) > streamH264Attr->height){
				canvas_y = streamH264Attr->height - canvas->height;
			}
		}else {
			canvas_x = (typeof(canvas_x))(x * (float)(streamH264Attr->width));			
			canvas_y = (typeof(canvas_y))(y * (float)(streamH264Attr->height));
			if((canvas_x + canvas->width) > streamH264Attr->width){
				if(stream == 0){
					canvas_x = canvas_x - 100;
				}else if(stream == 1){
					canvas_x = canvas_x - 60;
				}										 
			}
			if((canvas_y + canvas_height) > streamH264Attr->height){
				canvas_y = streamH264Attr->height - canvas->height;
			}
		}

		// alignment
		canvas_x = SDK_ALIGNED_BIG_ENDIAN(canvas_x, 4);
		canvas_y = SDK_ALIGNED_BIG_ENDIAN(canvas_y, 4);
		canvas_width = SDK_ALIGNED_BIG_ENDIAN(canvas_width, 2);
		canvas_height = SDK_ALIGNED_BIG_ENDIAN(canvas_height, 2);

		if(stCropInfo.bEnable == 1){
			if((canvas_x + canvas_width > stCropInfo.stCropRect.u32Width) && (canvas_x > 4)){
				canvas_x = canvas_x -4;
			}
			if((canvas_y + canvas_height > stCropInfo.stCropRect.u32Height) && (canvas_y > 4)){
				canvas_y = canvas_y - 4;
			}
		}else{			
			if((canvas_x + canvas_width > stVpssMode.u32Width) && (canvas_x > 4)){
				canvas_x = canvas_x -4;
			}
			if((canvas_y + canvas_height > stVpssMode.u32Height) && (canvas_y > 4)){
				canvas_y = canvas_y - 4;
			}
		}

		if(canvas_x < 0){
			canvas_x = 0;	
		}
		if(canvas_y < 0){
			canvas_y = 0;
		}
		
		for(i = 0; i < HI_VENC_OVERLAY_BACKLOG_REF; ++i){
			lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = &overlay_set->attr[i];
			if(!overlay->canvas){
				overlay->canvas = canvas;
				snprintf(overlay->name, sizeof(overlay->name), "%s", overlay_name);
				overlay->x = canvas_x;
				overlay->y = canvas_y;
				overlay->width = canvas_width;
				overlay->height = canvas_height;
				//printf("rect: %d/%d %d/%d\n", overlay->x, overlay->y, overlay->width, overlay->height);
				if(1){//if(overlay->region_handle >= 0){
					RGN_HANDLE const region_vpss_handle = overlay->region_handle;
					
					RGN_ATTR_S region_attr;
					RGN_CHN_ATTR_S region_ch_attr;
					MPP_CHN_S mppChannelVENC;

					///Attach To venc chn
					MPP_CHN_S stVpssChn;
					HI_S32 VpssChn;				
					int handle_num;
					VpssChn = _hi3518_vpss_ch_map[__HI_VENC_CH(vin, stream)];
			
					memset(&region_attr, 0, sizeof(region_attr));
					region_attr.enType = OVERLAY_RGN;
					region_attr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_4444;
					region_attr.unAttr.stOverlay.stSize.u32Width = overlay->width;
					region_attr.unAttr.stOverlay.stSize.u32Height = overlay->height;
					region_attr.unAttr.stOverlay.u32BgColor = 0;
					SOC_CHECK(HI_MPI_RGN_Create(region_vpss_handle, &region_attr));
					
					memset(&mppChannelVENC, 0, sizeof(mppChannelVENC));
					mppChannelVENC.enModId  = HI_ID_VENC;
					mppChannelVENC.s32DevId = 0;
					mppChannelVENC.s32ChnId = _hi3518_venc_ch_map[VpssChn];//VpssChn;  
					memset(&region_ch_attr,0,sizeof(region_ch_attr));
						
					region_ch_attr.bShow = HI_TRUE;
					region_ch_attr.enType = OVERLAY_RGN;
					
					region_ch_attr.unChnAttr.stOverlayChn.stPoint.s32X = overlay->x;   //0
					region_ch_attr.unChnAttr.stOverlayChn.stPoint.s32Y = overlay->y;   //680
					region_ch_attr.unChnAttr.stOverlayChn.u32BgAlpha = 0;
					region_ch_attr.unChnAttr.stOverlayChn.u32FgAlpha = 64;
					region_ch_attr.unChnAttr.stOverlayChn.u32Layer = 0;	
					

			        region_ch_attr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
			        region_ch_attr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;					
				//	region_ch_attr.unChnAttr.stOverlayChn.stQpInfo.bQpDisable  = HI_FALSE;
					region_ch_attr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_FALSE;

					SOC_CHECK(HI_MPI_RGN_AttachToChn(region_vpss_handle, &mppChannelVENC, &region_ch_attr));


				}
								return 0;
			}
		}
	}
	return -1;
}

static int enc_release_overlay(int vin, int stream, const char* overlay_name)
{
	if(vin < HI_VENC_CH_BACKLOG_REF
		&& stream < HI_VENC_STREAM_BACKLOG_REF ){
		lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = enc_lookup_overlay_byname(vin, stream, overlay_name);
		if(NULL != overlay){			
			overlay->canvas = NULL;
		
		//	SOC_CHECK(HI_MPI_RGN_DetachFrmChn(overlay->region_handle, &mppChannelVPSS));			
			SOC_CHECK(HI_MPI_RGN_Destroy(overlay->region_handle));
			return 0;
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
			RGN_HANDLE const regionHandle = overlay->region_handle;
			MPP_CHN_S mppChannel;
			RGN_CHN_ATTR_S regionChannelAttr;
	/*		
			mppChannel.enModId =  HI_ID_VPSS;
		    mppChannel.s32DevId = 0;
		    mppChannel.s32ChnId =  _hi3518_vpss_ch_map[__HI_VENC_CH(vin, stream)];
	*/		
			mppChannel.enModId =  HI_ID_VENC;
		    mppChannel.s32DevId = 0;
		    mppChannel.s32ChnId =  __HI_VENC_CH(vin, stream);
			SOC_CHECK(HI_MPI_RGN_GetDisplayAttr(regionHandle, &mppChannel, &regionChannelAttr));
			if(0 != showFlag){
				regionChannelAttr.bShow = HI_TRUE;
			}else{
				regionChannelAttr.bShow = HI_FALSE;
			}
			//SOC_NOTICE("region_ch_attr.bShow = %x/%x", showFlag, regionChannelAttr.bShow);
		    SOC_CHECK(HI_MPI_RGN_SetDisplayAttr(regionHandle, &mppChannel, &regionChannelAttr));			
			return 0;
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
			RGN_HANDLE const region_handle = overlay->region_handle;
			BITMAP_S overlay_bitmap;
			overlay_bitmap.enPixelFormat = PIXEL_FORMAT_RGB_4444;
			overlay_bitmap.u32Width = overlay->canvas->width;
			overlay_bitmap.u32Height = overlay->canvas->height;
			overlay_bitmap.pData = overlay->canvas->pixels;
			SOC_CHECK(HI_MPI_RGN_SetBitMap(region_handle, &overlay_bitmap));
			return 0;
		}
	}
	return -1;
}

static int hi3518e_enc_eptz_ctrl(int vin, int stream, int cmd, int param)
{
	return 0;
}

static int hi3518e_enc_usr_mode(int vin, int stream, int fix_mode, int show_mode)
{
	return 0;
}

static int hi3518_mpi_init()
{
    HI_S32 s32Ret;
    VB_CONF_S struVbConf;
    MPP_SYS_CONF_S struSysConf;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    memset(&struVbConf, 0, sizeof(VB_CONF_S));
    s32Ret = HI_MPI_VB_SetConf(&struVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VB_SetConf fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }
    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VB_Init fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_SYS_Init fail,Error(%#x)\n", s32Ret);
        (HI_VOID)HI_MPI_VB_Exit();
        return s32Ret;
    }

    return HI_SUCCESS;
}

/**************************************************************************************
**√Ë ˆ:ƒ⁄¥Ê”≥…‰£¨∏˘æ›≥§∂»‘⁄mmz”≥…‰œ‡Õ¨≥§∂»µƒƒ⁄¥Ê∏¯”√ªßø’º‰
**∑µªÿ:–Èƒ‚µÿ÷∑
***************************************************************************************/
static void  *hi3518e_enc_sdk_mmap(void *param)
{
	umount2("/media/custom", MNT_DETACH);// like 'umount -l'
	VB_BLK VbBlk;
	HI_U8* pVirAddr;
	HI_U32 u32PhyAddr;
	VB_POOL VbPool;
	//APP_OVERLAY_destroy();
	SDK_ENC_destroy();
/*	
	SDK_ISP_destroy();
	SDK_destroy_vin();
	sdk_audio->release_ain_ch(0);
	sdk_audio->destroy_ain();
	sdk_audio->release_ain_ch(0);
	sdk_audio->destroy_ain();
	SDK_destroy_audio();
	SDK_destroy_sys();
*/
	//OVERLAY_destroy();
	ssize_t length = *((ssize_t *)param);

	if(HI_SUCCESS != hi3518_mpi_init())
	{
		printf("MPI_Init err\n");
		return -1;
	}

	/* create a video buffer pool*/
	VbPool = HI_MPI_VB_CreatePool(length,1,"anonymous");

	if ( VB_INVALID_POOLID == VbPool )
	{
		printf("create vb err\n");
		return NULL;
	}

	VbBlk = HI_MPI_VB_GetBlock(VbPool, length, "anonymous");
	if (VB_INVALID_HANDLE == VbBlk)
	{
	    printf("HI_MPI_VB_GetBlock err! size:%d\n", length);
	    return NULL;
	}
	u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(VbBlk);
	if (0 == u32PhyAddr)
	{
	    printf("HI_MPI_VB_Handle2PhysAddr error\n");
	    return NULL;
	}
	pVirAddr = HI_MPI_SYS_Mmap(u32PhyAddr, length);
	if(0 == pVirAddr)
	{
		return NULL;
		printf("HI_MPI_SYS_Mmap error\n");
	}

	//∑µªÿµÿ÷∑
	return pVirAddr;
}
		
int  SDK_unmmap(void *virmem, int length)
{
	return HI_MPI_SYS_Munmap(virmem, length);
}

static int hi3518e_update_overlay_by_text(int vin, int stream, const char* text)
{
	return -1;
}

static void *enc_resolution_reboot(void *arg)
{
	pthread_detach(pthread_self());		
	sleep(3);
	SOC_INFO("MAIN_RESOLUTION changed,restart!");
#ifdef F5

#else
	exit(0);
#endif
}

static int hi3518e_enc_resolution(int width, int height)//main resolution
{	

	uint32_t sensor_width_max,sensor_height_max;

	HI_SDK_ISP_get_sensor_resolution(&sensor_width_max, &sensor_height_max);

	if(width >= 720 && width <= sensor_width_max &&
		height >= 576 && height <= sensor_height_max){

		FILE *fid0 = fopen(MAIN_RESOLUTION, "rb");
		char buf[16] = "";
		char tmp[5] = "";
		int width_old ,height_old;
		pthread_t pid = NULL;
		bool flag_restart;
		
		if(NULL != fid0){
			fread(buf,sizeof(buf),1,fid0);
			fclose(fid0);
			fid0 = NULL;
			sscanf(buf,"%d %s %d",&width_old,tmp,&height_old);
		}else{
			SOC_INFO("Open %s failed", MAIN_RESOLUTION);
			return -1;	
		}


		if(width_old * height_old >= 1920*1080 || width * height >= 1920*1080){
			flag_restart = true;
		}else{
			flag_restart = false;
		}
				
		FILE *fid = fopen(MAIN_RESOLUTION, "w+b");
		
		memset(buf,0,sizeof(buf));		
		sprintf(buf,"%d x %d",width,height);
		if(NULL != fid){
			fwrite(buf,1,strlen(buf),fid);
			fclose(fid);
			fid = NULL;			
			SOC_INFO("cur resolution: %dx%d ; new resolution: %dx%d \n",width_old,height_old,width,height);
			if(flag_restart){
				pthread_create(&pid,NULL,enc_resolution_reboot,NULL);				
			}
			return 0;		
		}else{
			SOC_INFO("Open %s failed", MAIN_RESOLUTION);
		}
		return -1;	
	}else{
		SOC_INFO("Resolution size exceeds limit!!");
		return -1;
	}

}

static stSDK_ENC_HI3521 _sdk_enc =
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

		//h265 stream
		.create_stream_h265 = enc_create_stream_h265,
		.release_stream_h265 = enc_release_stream_h265,
		.enable_stream_h265 = enc_enable_stream_h265,
		.set_stream_h265 = enc_set_stream_h265,
		.get_stream_h265 = enc_get_stream_h265,
		.request_stream_h265_keyframe = enc_request_stream_h265_keyframe,
		
		.create_stream_g711a = enc_create_stream_g711a,
		.create_audio_stream = enc_create_audio_stream,
		.release_stream_g711a = enc_release_stream_g711a,

		// snapshot a picture
		.creat_snapshot_chn = enc_creat_snapshot_chn,
		.release_snapshot_chn = enc_release_snapshot_chn,
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

		//fish eye
		.eptz_ctrl = hi3518e_enc_eptz_ctrl,
		.enc_mode = hi3518e_enc_usr_mode,

		//upgrade
		.upgrade_env_prepare = 	hi3518e_enc_sdk_mmap,	
		.update_overlay_bytext = hi3518e_update_overlay_by_text,

		.enc_resolution = hi3518e_enc_resolution,
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
				LP_SDK_ENC_STREAM_H265_ATTR const streamH265Attr = &_sdk_enc.attr.h265_attr[i][ii];
				
				uint8_t *const frame_ref_counter = &_sdk_enc.attr.frame_ref_counter[i][ii];

				STREAM_H264_CLEAR(streamH264Attr);
				STREAM_H264_CLEAR(streamH265Attr);
				*frame_ref_counter = 0;
			}
		}
		// init the overlay set handl
		for(i = 0; i < HI_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < HI_VENC_STREAM_BACKLOG_REF; ++ii){
				lpSDK_ENC_VIDEO_OVERLAY_ATTR_SET const overlay_set = &_sdk_enc.attr.video_overlay_set[i][ii];
				for(iii = 0; iii < HI_VENC_OVERLAY_BACKLOG_REF; ++iii){
					lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = &overlay_set->attr[iii];

					overlay->canvas = NULL;
					memset(overlay->name, 0, sizeof(overlay->name));
					overlay->x = 0;
					overlay->y = 0;
					overlay->width = 0;
					overlay->height = 0;
					// very important, pre-alloc the handle number
					overlay->region_handle = HI_VENC_OVERLAY_HANDLE_OFFSET;
					overlay->region_handle += i * HI_VENC_STREAM_BACKLOG_REF * HI_VENC_OVERLAY_BACKLOG_REF;
					overlay->region_handle += ii * HI_VENC_OVERLAY_BACKLOG_REF;
					overlay->region_handle += iii;
				}
			}

		}
//		_sdk_enc.attr.overlay_handle_num = 0;
		
		// init the snapshot mutex
		pthread_mutex_init(&_sdk_enc.attr.snapshot_mutex, NULL);

		
		sdk_enc->creat_snapshot_chn(0,VENC_MAX_CHN_NUM - 2,-1,-1);
		sdk_enc->creat_snapshot_chn(0,VENC_MAX_CHN_NUM - 1,640,360);
		// start
		//sdk_enc->start();
		// success to init
		return 0;
	}
	return -1;
}


int SDK_ENC_wdr_destroy()
{	
	if(sdk_enc){
		int i = 0, ii = 0;
	   // release the video encode
		for(i = 0; i < HI_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < HI_VENC_STREAM_BACKLOG_REF; ++ii){	// destroy sub stream firstly
				switch(_sdk_enc.attr.enType[i][ii]){
				default:
				case kSDK_ENC_BUF_DATA_H264:					
					sdk_enc->release_stream_h264(i, ii);
					break;
				case kSDK_ENC_BUF_DATA_H265:
					sdk_enc->release_stream_h265(i, ii);
					break;	
			    }				
		    }
		}
		return 0;
	}
	return -1;

}

int SDK_ENC_vpss_destroy()
{  
	MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
	HI_U32 VpssGrp = 0;
	HI_U32 VpssChn;

	// vpss chn 0,1,2,3
	for(VpssChn = 0; VpssChn <=3; VpssChn++){
		HI_MPI_VPSS_DisableChn(VpssGrp,VpssChn);
		usleep(500);
	}

	stSrcChn.enModId = HI_ID_VIU;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 0;

	stDestChn.enModId = HI_ID_VPSS;
	stDestChn.s32DevId = VpssGrp;
	stDestChn.s32ChnId = 0;
	
	HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
	HI_MPI_VPSS_StopGrp(VpssGrp);
	HI_MPI_VPSS_DestroyGrp(VpssGrp);
	return 0;
}


int SDK_ENC_destroy()
{	
	SDK_destroy_vin();
	if(sdk_enc){
		int i = 0, ii = 0;
//		bExitOverlayLoop = HI_TRUE;
		
		// destroy the snapshot mutex
		pthread_mutex_destroy(&_sdk_enc.attr.snapshot_mutex);	
		// stop encode firstly
		sdk_enc->stop();
		
		//release the sanapshot chn
		sdk_enc->release_snapshot_chn(0,VENC_MAX_CHN_NUM - 2);	//main JPEG
		sdk_enc->release_snapshot_chn(0,VENC_MAX_CHN_NUM - 1);  //sub JPEG
				
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
				switch(_sdk_enc.attr.enType[i][ii]){
					default:
					case kSDK_ENC_BUF_DATA_H264:					
						sdk_enc->release_stream_h264(i, ii);
						break;
					case kSDK_ENC_BUF_DATA_H265:
						sdk_enc->release_stream_h265(i, ii);
						break;	
				}	
			}
		}		
		// clear handler pointer
		sdk_enc = NULL;		
		// success to destroy
	}
	//destroy vpss	
	SDK_ENC_vpss_destroy();
//	SDK_destroy_vin();
	if(sdk_audio){
		sdk_audio->release_ain_ch(0);
	}
	SDK_destroy_audio();
	SENSOR_destroy();
	usleep(1000);
	SDK_destroy_sys();

	return 0;
}


int SDK_ENC_create_stream(int vin, int stream, LP_SDK_ENC_STREAM_ATTR stream_attr)
{
	if(sdk_enc){
		int ret;
		_sdk_enc.attr.enType[vin][stream] = stream_attr->enType;

		switch(stream_attr->enType){
			default:
			case kSDK_ENC_BUF_DATA_H264:
				ret = sdk_enc->create_stream_h264(vin, stream,&stream_attr->H264_attr);
				break;
			case kSDK_ENC_BUF_DATA_H265:
				ret = sdk_enc->create_stream_h265(vin, stream,&stream_attr->H265_attr);
				break;	
		}
		return ret;
	}else{
		return -1;

	}

}



int SDK_ENC_release_stream(int vin, int stream)
{
	if(sdk_enc){
		int ret;
		switch(_sdk_enc.attr.enType[vin][stream]){
			default:
			case kSDK_ENC_BUF_DATA_H264:
				ret = sdk_enc->release_stream_h264(vin,stream);
				break;
			case kSDK_ENC_BUF_DATA_H265:
				ret = sdk_enc->release_stream_h265(vin,stream);
				break;	
		}
		return ret;
	}else{
		return -1;
	}


}



int SDK_ENC_set_stream(int vin, int stream,LP_SDK_ENC_STREAM_ATTR stream_attr)
{	
	if(sdk_enc){
		int ret;
		switch(stream_attr->enType){
			default:
			case kSDK_ENC_BUF_DATA_H264:
				ret = sdk_enc->set_stream_h264(vin, stream,&stream_attr->H264_attr);
				break;
			case kSDK_ENC_BUF_DATA_H265:
				ret = sdk_enc->set_stream_h265(vin, stream,&stream_attr->H265_attr);
				break;	
		}	
		return ret;
	}else{
		return -1;
	}

}


int SDK_ENC_get_stream(int vin, int stream, LP_SDK_ENC_STREAM_ATTR stream_attr)
{
	if(sdk_enc){
		int ret;
		switch(_sdk_enc.attr.enType[vin][stream]){
			default:
			case kSDK_ENC_BUF_DATA_H264:
				ret = sdk_enc->get_stream_h264(vin, stream,&stream_attr->H264_attr);
				stream_attr->enType = kSDK_ENC_BUF_DATA_H264;		
				break;
			case kSDK_ENC_BUF_DATA_H265:
				ret = sdk_enc->get_stream_h265(vin, stream,&stream_attr->H265_attr);			
				stream_attr->enType = kSDK_ENC_BUF_DATA_H265;
				break;	
		}	
		return ret;
	}else{
		return -1;
	}

}

int SDK_ENC_enable_stream(int vin, int stream, bool flag)
{
	if(sdk_enc){
		int ret;
		switch(_sdk_enc.attr.enType[vin][stream]){
			default:
			case kSDK_ENC_BUF_DATA_H264:
				ret = sdk_enc->enable_stream_h264(vin, stream, flag);
				break;
			case kSDK_ENC_BUF_DATA_H265:
				ret = sdk_enc->enable_stream_h265(vin, stream, flag);
				break;	
		}
		return ret;
	}else{
		return -1;
	}

}

int	SDK_ENC_request_stream_keyframe(int vin, int stream)
{
	if(sdk_enc){
		switch(_sdk_enc.attr.enType[vin][stream]){
			default:
			case kSDK_ENC_BUF_DATA_H264:
				sdk_enc->request_stream_h264_keyframe(vin, stream);
				break;
			case kSDK_ENC_BUF_DATA_H265:
				sdk_enc->request_stream_h265_keyframe(vin, stream);
				break;	
		}
		return 0;
	}else{
		return -1;
	}	

}

PAYLOAD_TYPE_E  SDK_ENC_request_venc_type(int vin, int stream)
{
	VENC_CHN_ATTR_S venc_ch_attr;
	enSDK_ENC_BUF_DATA_TYPE enc_type;
	int const venc_ch = __HI_VENC_CH(vin, stream);
	SOC_CHECK(HI_MPI_VENC_GetChnAttr(venc_ch, &venc_ch_attr));
	switch(venc_ch_attr.stVeAttr.enType){
		default:
		case PT_H264:
			enc_type = kSDK_ENC_BUF_DATA_H264;
			break;
		case PT_H265:
			enc_type = kSDK_ENC_BUF_DATA_H265;
			break;	
	}
	return enc_type;
}

int SDK_ENC_get_enc_pts(int vin, int stream, unsigned long long *encPts)
{
	if(encPts){
		*encPts = _sdk_enc.attr.u64encPTS[vin][stream];
		return 0;
	}
	return -1;
}

/*
* @brief ‰∏ªË¶ÅÁî®Êù•ÁªôÂà∞sdkÂ±ÇÂΩìÂâçÁöÑËÆæÂ§áÂûãÂè∑ÔºåÂå∫ÂàÜPxÂíåCxÁöÑQPÂÄºÔºå
         Âú®SDK_ENC_initÂêéË∞ÉÁî®Ôºå‰∏çÂª∫ËÆÆÁî®‰∫éÂÖ∂ÂÆÉÂäüËÉΩ
* @param model emSDK_ENC_PRODUCT_MODEL_PX = 0,
               emSDK_ENC_PRODUCT_MODEL_CX,
*/
void SDK_ENC_set_model(int model)
{
    _sdk_enc.attr.product_model = model;

}

