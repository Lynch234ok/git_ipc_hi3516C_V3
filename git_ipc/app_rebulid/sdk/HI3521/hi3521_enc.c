
#include "hi3521.h"
#include "sdk/sdk_api.h"
#include "sdk_trace.h"
#include "sdk_common.h"
#include <sys/prctl.h>

#if defined(HI3518A) | defined(HI3518C) | defined(HI3516C) | defined(HI3518E)
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

#if defined(HI3518A) | defined(HI3518C) | defined(HI3516C) | defined(HI3518E)
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
# define HI_AENC_STREAM_BACKLOG_REF (2)
#elif defined(HI3518E)
# define HI_VENC_CH_BACKLOG_REF (1)
# define HI_AENC_CH_BACKLOG_REF (1)
# define HI_VENC_STREAM_BACKLOG_REF (2)
# define HI_AENC_STREAM_BACKLOG_REF (2)
#else
# define HI_VENC_CH_BACKLOG_REF (16)
# define HI_AENC_CH_BACKLOG_REF (16)
# define HI_VENC_STREAM_BACKLOG_REF (2)
#endif

#define HI_VENC_JPEG_MIN_WIDTH (64)
#define HI_VENC_JPEG_MAX_WIDTH (8192)
#define HI_VENC_JPEG_MIN_HEIGHT (64)
#define HI_VENC_JPEG_MAX_HEIGHT (8192)

#define HI_VENC_OVERLAY_BACKLOG_REF (OVERLAY_MAX_NUM)
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

	bool loop_trigger;
	int ref_count;
	pthread_t loop_tid;
	pthread_mutex_t snapshot_mutex;
}stSDK_ENC_ATTR, *lpSDK_ENC_ATTR;

#define STREAM_H264_ISSET(__stream_h264) (strlen((__stream_h264)->name) > 0)
#define STREAM_H264_CLEAR(__stream_h264) do{ memset((__stream_h264)->name, 0, sizeof((__stream_h264)->name)); }while(0)


typedef struct SDK_ENC_HI3521
{
	stSDK_ENC_API api;
	stSDK_ENC_ATTR attr;
}stSDK_ENC_HI3521, *lpSDK_ENC_HI3521;
static stSDK_ENC_HI3521 _sdk_enc;
lpSDK_ENC_API sdk_enc = NULL;

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


static uint32_t enc_stream_limit_fps(uint32_t width, uint32_t height, uint32_t fps, uint32_t chip_id)
{
#define HI3518E_SDK_MAX_ABILITY (1280*720*20)
#define HI3518C_SDK_MAX_ABILITY (1280*960*25)
#define HI3516C_SDK_MAX_ABILITY (1920*1080*30)

	char text[128] = {""};
	uint32_t limit_framerate = 30;

	if(SDK_HI3518C_V100 == chip_id || SDK_HI3518A_V100 == chip_id){//3518C&3518A
		limit_framerate = HI3518C_SDK_MAX_ABILITY/(width*height);		
	
	}else if(SDK_HI3518E_V100 == chip_id){//3518E
		limit_framerate = HI3518E_SDK_MAX_ABILITY/(width*height);	
	
	}else{// fix me:another SOC
		
	}
	printf("%s-%d-%x-limit/curent:%d/%d\r\n", __FUNCTION__, __LINE__, chip_id, limit_framerate, fps);
	if(limit_framerate < fps){
		return limit_framerate;
	}else{
		return fps;
	}
}


static int enc_create_stream_h264(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr)
{
	_sdk_enc.attr.enType[vin][stream] = kSDK_ENC_BUF_DATA_H264;
	if(vin < HI_VENC_CH_BACKLOG_REF && stream < HI_VENC_STREAM_BACKLOG_REF
		&& NULL != h264_attr && STREAM_H264_ISSET(h264_attr)){
		
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
			
		if(0 == enc_lookup_stream_byname(h264_attr->name, NULL, NULL)){
			// the name is overlap
			return -1;
		}
		
		if(!STREAM_H264_ISSET(streamH264Attr)){
			int const venc_group = vin * HI_VENC_STREAM_BACKLOG_REF + stream;
			int const venc_ch = venc_group;
			int const vpssGroup = vin;
#if defined(HI3518A) | defined(HI3518C) | defined(HI3516C) | defined(HI3518E)
			int const vpssChannel = _hi3518_vpss_ch_map[venc_ch];
#else
			int const vpssChannel = (venc_ch % HI_VENC_STREAM_BACKLOG_REF); 
#endif
			int bps_limit = 0;
			const char *venc_mmz = (0 == venc_group % 2) ? HI_MMZ_ZONE_NAME0 : HI_MMZ_ZONE_NAME1;

			// hisilicon structure
			//VPSS_CHN_MODE_S vpssChanneln_mode;
			//VPSS_CHN_ATTR_S vpssChanneln_attr;
			VPSS_CROP_INFO_S stCropInfo;
			MPP_CHN_S mppChannelVPSS;
			MPP_CHN_S mppChannelVENC;
			VENC_CHN_ATTR_S vencChannelAttr;
			VENC_ATTR_H264_REF_MODE_E venc_attr_h264_ref_mode;
			VENC_ATTR_S *const p_venc_attr = &vencChannelAttr.stVeAttr; // the attribute of video encoder
			VENC_RC_ATTR_S *const p_venc_rc_attr = &vencChannelAttr.stRcAttr; // the attribute of rate  ctrl
			VENC_ATTR_H264_S *const p_venc_attr_h264 = &p_venc_attr->stAttrH264e;
			VENC_ATTR_H264_CBR_S *const p_h264_cbr = &p_venc_rc_attr->stAttrH264Cbr;
			VENC_ATTR_H264_VBR_S *const p_h264_vbr = &p_venc_rc_attr->stAttrH264Vbr;
			VENC_ATTR_H264_FIXQP_S *const p_h264_fixqp = &p_venc_rc_attr->stAttrH264FixQp;
			VENC_ATTR_H264_ABR_S *const p_h264_abr = &p_venc_rc_attr->stAttrH264Abr;
			VENC_PARAM_H264_CBRV2_S h264_cbrv2;
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

			uint32_t chip_id;
			HI_MPI_SYS_GetChipId(&chip_id);
			streamH264Attr->fps = enc_stream_limit_fps(streamH264Attr->width, streamH264Attr->height, streamH264Attr->fps, chip_id);
			
			if(streamH264Attr->fps > vin_capture_attr.fps){
				streamH264Attr->fps = vin_capture_attr.fps; // encode framerate must be less than capture framerate
			}
#if defined(HI3518E)
			if(streamH264Attr->fps > 20){				
				streamH264Attr->fps = 20;			
			}
#endif
			SOC_INFO("Create H264 Stream(%d,%d,%s) %dx%d @ %d/%d", vin, stream, streamH264Attr->name, streamH264Attr->width, streamH264Attr->height, streamH264Attr->fps, streamH264Attr->gop);
			
			// Assign MMZ
 			hi3521_mmz_zone_assign(HI_ID_GROUP, venc_group, 0, venc_mmz);
			hi3521_mmz_zone_assign(HI_ID_VENC, 0, venc_ch, venc_mmz);

			// Greate Venc Group
			SOC_CHECK(HI_MPI_VENC_CreateGroup(venc_group));

			// Create Venc Channel
			memset(&vencChannelAttr, 0, sizeof(vencChannelAttr));
			p_venc_attr->enType = PT_H264; // must be h264 for this interface
			p_venc_attr_h264->u32MaxPicWidth = streamH264Attr->width;
			p_venc_attr_h264->u32MaxPicHeight = streamH264Attr->height;
			p_venc_attr_h264->u32PicWidth = p_venc_attr_h264->u32MaxPicWidth;
			p_venc_attr_h264->u32PicHeight = p_venc_attr_h264->u32MaxPicHeight;
			//p_venc_attr_h264->u32BufSize  = p_venc_attr_h264->u32MaxPicWidth * p_venc_attr_h264->u32MaxPicHeight * 3 / 2; // stream buffer size
			p_venc_attr_h264->u32BufSize  = p_venc_attr_h264->u32MaxPicWidth * p_venc_attr_h264->u32MaxPicHeight; // stream buffer size
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
			p_venc_attr_h264->bField = HI_FALSE;  // surpport frame code only for hi3516, bfield = HI_FALSE
			p_venc_attr_h264->bMainStream = HI_TRUE; // surpport main stream only for hi3516, bMainStream = HI_TRUE
			p_venc_attr_h264->u32Priority = 0; // channels precedence level. invalidate for hi3516
			p_venc_attr_h264->bVIField = HI_FALSE; // the sign of the VI picture is field or frame. Invalidate for hi3516

			bps_limit = sdk_venc_bps_limit(streamH264Attr->width, streamH264Attr->height,
				streamH264Attr->fps, streamH264Attr->bps);

			HI_U32 minQp;
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
			printf("!!!!!!!!!!!minQP = %d  quality=%d\n", minQp, h264_attr->quality);
			switch(h264_attr->rc_mode){
				default:
				case kSDK_ENC_H264_RC_MODE_VBR:
				case kSDK_ENC_H264_RC_MODE_AUTO:
				{
					vencChannelAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBRv2;
					p_h264_vbr->u32Gop = streamH264Attr->fps*3; //FIX ME: streamH264Attr->gop;
					p_h264_vbr->u32StatTime = 3;//p_h264_vbr->u32Gop / streamH264Attr->fps;//(streamH264Attr->gop + streamH264Attr->fps - 1) / streamH264Attr->fps;
					p_h264_vbr->u32ViFrmRate = vin_capture_attr.fps;
					p_h264_vbr->fr32TargetFrmRate = (typeof(p_h264_vbr->fr32TargetFrmRate))streamH264Attr->fps;
					//p_h264_vbr->u32MaxBitRate = bps_limit * 4 /3;
					p_h264_vbr->u32MaxBitRate = bps_limit;
					p_h264_vbr->u32MinQp = minQp; //24;
					p_h264_vbr->u32MaxQp = 51; //32;
					break;
				}

				//case kSDK_ENC_H264_RC_MODE_Abr:
				case kSDK_ENC_H264_RC_MODE_CBR:
				{
					p_venc_rc_attr->enRcMode = VENC_RC_MODE_H264CBRv2;
					p_h264_cbr->u32Gop = streamH264Attr->fps*2; //FIX ME: streamH264Attr->gop;
					p_h264_cbr->u32StatTime = 2;//p_h264_cbr->u32Gop / streamH264Attr->fps;
					p_h264_cbr->u32ViFrmRate = vin_capture_attr.fps;
					p_h264_cbr->fr32TargetFrmRate = (typeof(p_h264_vbr->fr32TargetFrmRate))streamH264Attr->fps;
					p_h264_cbr->u32BitRate = bps_limit;
					p_h264_cbr->u32FluctuateLevel = 0;
					break;
				}
				case kSDK_ENC_H264_RC_MODE_ABR:
				{
					p_venc_rc_attr->enRcMode = VENC_RC_MODE_H264ABR;
					p_h264_abr->u32Gop = streamH264Attr->gop;
					p_h264_abr->u32StatTime = (streamH264Attr->gop + streamH264Attr->fps - 1) / streamH264Attr->fps;
					p_h264_abr->u32ViFrmRate = vin_capture_attr.fps;
					p_h264_abr->fr32TargetFrmRate = (typeof(p_h264_vbr->fr32TargetFrmRate))streamH264Attr->fps;
					p_h264_abr->u32AvgBitRate = bps_limit;
					p_h264_abr->u32MaxBitRate = bps_limit * 4 / 3; // 1.5 times of bitrate
					break;
				}
				case kSDK_ENC_H264_RC_MODE_FIXQP:
				{
					p_venc_rc_attr->enRcMode = VENC_RC_MODE_H264FIXQP;
					p_h264_fixqp->u32Gop = streamH264Attr->gop;
					p_h264_fixqp->u32ViFrmRate = vin_capture_attr.fps;
					p_h264_fixqp->fr32TargetFrmRate = (typeof(p_h264_vbr->fr32TargetFrmRate))streamH264Attr->fps;
					p_h264_fixqp->u32IQp = 20;
					p_h264_fixqp->u32PQp = 23;
					break;
				}
			}

			SOC_CHECK(HI_MPI_VENC_CreateChn(venc_ch, &vencChannelAttr));
			SOC_CHECK(HI_MPI_VENC_RegisterChn(venc_ch, venc_group));

			// set vpss chn mode
#if defined(HI3518A)|defined(HI3518C)|defined(HI3516C)|defined(HI3518E)
			do{
				VPSS_CHN_MODE_S vpssChanneln_mode;
				VPSS_CHN_ATTR_S vpssChanneln_attr;
				vpssChanneln_mode.enChnMode = VPSS_CHN_MODE_USER;
				vpssChanneln_mode.bDouble = HI_FALSE;
				vpssChanneln_mode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
				vpssChanneln_mode.u32Width = p_venc_attr_h264->u32MaxPicWidth;
				vpssChanneln_mode.u32Height = p_venc_attr_h264->u32MaxPicHeight;
				SOC_CHECK(HI_MPI_VPSS_SetChnMode(vpssGroup, vpssChannel, &vpssChanneln_mode));  
#if !defined(HI3518E)
				float fsrcScale, fdstScale;
				fsrcScale = (float)vin_capture_attr.height/(float)vin_capture_attr.width;
				fdstScale = (float)streamH264Attr->height/(float)streamH264Attr->width;
				//printf("src=%f   dst=%f\r\n", fsrcScale, fdstScale);
				SOC_CHECK(HI_MPI_VPSS_GetCropCfg(vpssGroup, &stCropInfo));
				if(stream == 0){
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
							/*printf("to min x=%d y=%d w=%d h=%d\r\n", stCropInfo.stCropRect.s32X, stCropInfo.stCropRect.s32Y,
								stCropInfo.stCropRect.u32Width, stCropInfo.stCropRect.u32Height);*/
						}else{//fsrcScale < fdstScale
							stCropInfo.stCropRect.s32X = (vin_capture_attr.width - vin_capture_attr.height/fdstScale)/2; 
							stCropInfo.stCropRect.s32Y = 0; 
							stCropInfo.stCropRect.u32Width = vin_capture_attr.height/fdstScale; 
							stCropInfo.stCropRect.u32Height = vin_capture_attr.height;
							stCropInfo.stCropRect.s32X = (stCropInfo.stCropRect.s32X%2)? stCropInfo.stCropRect.s32X + 1 : stCropInfo.stCropRect.s32X;
							stCropInfo.stCropRect.u32Width = (stCropInfo.stCropRect.u32Width%2)? stCropInfo.stCropRect.u32Width -1: stCropInfo.stCropRect.u32Width;
							/*printf("to max x=%d y=%d w=%d h=%d\r\n", stCropInfo.stCropRect.s32X, stCropInfo.stCropRect.s32Y,
								stCropInfo.stCropRect.u32Width, stCropInfo.stCropRect.u32Height);*/
						}
					}else{//dont need crop
						stCropInfo.bEnable = 0;
					}
					SOC_CHECK(HI_MPI_VPSS_SetCropCfg(vpssGroup, &stCropInfo));
				}
#endif
				
				if(vpssChannel >= VPSS_MAX_PHY_CHN_NUM){
					VPSS_EXT_CHN_ATTR_S vpss_ext_chn_attr;
					vpss_ext_chn_attr.s32BindChn = 1; // bind to vpss 1
					vpss_ext_chn_attr.s32SrcFrameRate = -1;//vin_capture_attr.fps;
					vpss_ext_chn_attr.s32DstFrameRate = -1;//streamH264Attr->fps;
					vpss_ext_chn_attr.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
					vpss_ext_chn_attr.u32Width = p_venc_attr_h264->u32MaxPicWidth;
					vpss_ext_chn_attr.u32Height = p_venc_attr_h264->u32MaxPicHeight;
					SOC_CHECK(HI_MPI_VPSS_SetExtChnAttr(vpssGroup, vpssChannel, &vpss_ext_chn_attr));
				}

				memset(&vpssChanneln_attr, 0, sizeof(vpssChanneln_attr));
				vpssChanneln_attr.bFrameEn = HI_FALSE;
				vpssChanneln_attr.bSpEn = HI_TRUE;
				SOC_CHECK(HI_MPI_VPSS_SetChnAttr(vpssGroup, vpssChannel, &vpssChanneln_attr));
				SOC_CHECK(HI_MPI_VPSS_EnableChn(vpssGroup, vpssChannel));
			}while(0);
			// to reduce the size of IDR
			do{
				VENC_RC_PARAM_S stVencRcPara;
				uint32_t u32ThrdI[12] = {5,5,5,10,10,10,255,255,255,255,255,255};//{7,7,7,9,12,14,18,25,255,255,255,255};
				uint32_t u32ThrdP[12] = {5,5,5,255,255,255,255,255,255,255,255,255};	
				SOC_CHECK(HI_MPI_VENC_GetRcPara(venc_ch, &stVencRcPara));
				memcpy(stVencRcPara.u32ThrdI, u32ThrdI, sizeof(stVencRcPara.u32ThrdI));
				memcpy(stVencRcPara.u32ThrdP, u32ThrdP, sizeof(stVencRcPara.u32ThrdI));
				switch(p_venc_rc_attr->enRcMode){
					default:
					case VENC_RC_MODE_H264VBRv2:
						{
							stVencRcPara.stParamH264VBR.s32DeltaQP = 6;
							//stVencRcPara.stParamH264VBR.u32MinIQP = 16;
							stVencRcPara.stParamH264VBR.u32MaxIprop = 30;
						}
						break;
					case VENC_RC_MODE_H264CBRv2:
						{
							stVencRcPara.stParamH264CbrV2.u32MinIQp = 20;
							stVencRcPara.stParamH264CbrV2.u32MaxIprop = 20;
						}
						break;
					case VENC_RC_MODE_H264FIXQP:
						break;
				}
				
				SOC_CHECK(HI_MPI_VENC_SetRcPara(venc_ch, &stVencRcPara));
			}while(0);
#endif

			memset(&mppChannelVPSS, 0, sizeof(mppChannelVPSS));
			memset(&mppChannelVENC, 0, sizeof(mppChannelVENC));
			// binding venc to vpss
			mppChannelVPSS.enModId = HI_ID_VPSS;
			mppChannelVPSS.s32DevId = vpssGroup;
			mppChannelVPSS.s32ChnId = vpssChannel;
			mppChannelVENC.enModId = HI_ID_GROUP;
			mppChannelVENC.s32DevId = venc_group;
			mppChannelVENC.s32ChnId = venc_ch;
			SOC_CHECK(HI_MPI_SYS_Bind(&mppChannelVPSS, &mppChannelVENC));

			// set h264 reference mode
			SOC_CHECK(HI_MPI_VENC_GetH264eRefMode(venc_ch, &venc_attr_h264_ref_mode));
			venc_attr_h264_ref_mode = HI_VENC_H264_REF_MODE;
			SOC_CHECK(HI_MPI_VENC_SetH264eRefMode(venc_ch, venc_attr_h264_ref_mode));

			// start to recv frame
			//SOC_CHECK(HI_MPI_VENC_StartRecvPic(venc_ch));
			return 0;
		}
	}
	return -1;
}
static int enc_create_stream_h265(int vin, int stream, LP_SDK_ENC_STREAM_H265_ATTR h265_attr)
{
	_sdk_enc.attr.enType[vin][stream] = kSDK_ENC_BUF_DATA_H264;

	return 0;
}

static int enc_release_stream_h264(int vin, int stream)
{
	LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
	if(vin < HI_VENC_CH_BACKLOG_REF && stream < HI_VENC_STREAM_BACKLOG_REF){
		if(STREAM_H264_ISSET(streamH264Attr)){
			int const venc_ch = __HI_VENC_CH(vin, stream);
			int const venc_group = venc_ch;
			int const vpssGroup = vin;
			time_t now;
#if defined(HI3518A) | defined(HI3518C) | defined(HI3516C) | defined(HI3518E)
			int const vpssChannel = _hi3518_vpss_ch_map[venc_ch];
#else
			int const vpssChannel = (venc_ch % HI_VENC_STREAM_BACKLOG_REF); 
#endif
			
			MPP_CHN_S mppChannelVPSS;
			MPP_CHN_S mppChannelVENC;

			memset(&mppChannelVPSS, 0, sizeof(mppChannelVPSS));
			memset(&mppChannelVENC, 0, sizeof(mppChannelVENC));
			// unbind venc to vpss
			mppChannelVPSS.enModId = HI_ID_VPSS;
			mppChannelVPSS.s32DevId = vpssGroup;
			mppChannelVPSS.s32ChnId = vpssChannel;
			mppChannelVENC.enModId = HI_ID_GROUP;
			mppChannelVENC.s32DevId = venc_group;
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
			SOC_CHECK(HI_MPI_VENC_UnRegisterChn(venc_ch));
			SOC_CHECK(HI_MPI_VENC_DestroyChn(venc_ch));
			SOC_CHECK(HI_MPI_VENC_DestroyGroup(venc_group));

			return 0;
		}
	}
	return -1;
}

static int enc_release_stream_h265(int vin, int stream)
{
return 0;
}

static int enc_set_stream_h264(int vin, int stream, LP_SDK_ENC_STREAM_H264_ATTR h264_attr)
{
	if(vin < HI_VENC_CH_BACKLOG_REF && stream < HI_VENC_STREAM_BACKLOG_REF
		&& NULL != h264_attr && STREAM_H264_ISSET(h264_attr)){
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];

		if(STREAM_H264_ISSET(streamH264Attr)){
			if(1){//if(streamH264Attr->width != h264_attr->width || streamH264Attr->height != h264_attr->height){

				// width / height adjust need to restart the stream
			//	sdk_enc->release_stream_h264(vin, stream);			
				SDK_ENC_release_stream(vin, stream);
				sdk_enc->create_stream_h264(vin, stream, h264_attr);
				sdk_enc->enable_stream_h264(vin, stream, true);
				return 0;
			}else{
				VENC_CHN_ATTR_S venc_ch_attr;
				int const venc_ch = __HI_VENC_CH(vin, stream);

				SOC_CHECK(HI_MPI_VENC_GetChnAttr(venc_ch, &venc_ch_attr));
				// FIXME:
				SOC_CHECK(HI_MPI_VENC_SetChnAttr(venc_ch, &venc_ch_attr));
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
					AENC_CHN_ATTR_S aenc_ch_attr = {.enType = PT_G711A, .pValue = &aenc_attr_g711,};
					MPP_CHN_S mpp_ch_ai, mpp_ch_aenc;

					aenc_ch_attr.enType = PT_G711A;
					aenc_ch_attr.pValue = &aenc_attr_g711;
					//aenc_ch_attr.u32BufSize = 8;
					//aenc_ch_attr.u32BufSize = MAX_AUDIO_FRAME_NUM; // ver 050 faq 1.6.3
					aenc_ch_attr.u32BufSize = 5;

					stream_g711_attr->ain = ain;
					stream_g711_attr->vin_ref = vin_ref;
					stream_g711_attr->sample_rate = 8000;
					stream_g711_attr->sample_width = 16;
					// FIXME: remember to check the sample rate and bitwidth
					stream_g711_attr->packet_size = ain_packet_size;

					// sdk operate
					// enable vin
					SOC_CHECK(HI_MPI_AI_EnableChn(HI_AIN_DEV, ain));
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
			SOC_CHECK(HI_MPI_AI_DisableChn(HI_AIN_DEV, ain));

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
	SOC_CHECK(HI_MPI_VENC_RequestIDR(__HI_VENC_CH(vin, stream)));
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
		stream_size += venc_stream.pstPack[i].u32Len[0];
		stream_size += venc_stream.pstPack[i].u32Len[1];
	}
	return stream_size;
}

static int enc_video_proc(int vin, int stream)
{
#define HI3521_MAX_VENC_PACK (4)

	LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &(_sdk_enc.attr.h264_attr[vin][stream]);
	LP_SDK_ENC_STREAM_H265_ATTR const streamH265Attr = &(_sdk_enc.attr.h265_attr[vin][stream]);

	uint8_t *const ref_counter = &(_sdk_enc.attr.frame_ref_counter[vin][stream]);
	int const venc_ch = vin * HI_VENC_STREAM_BACKLOG_REF + stream;

	if(STREAM_H264_ISSET(streamH264Attr) || STREAM_H264_ISSET(streamH265Attr)){
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
						is_keyframe = (HI3521_MAX_VENC_PACK == venc_stream.u32PackCount);

						if(is_keyframe){
							*ref_counter = 0;
						}else{
							// h264 enc is x4 reference
							*ref_counter += 1;
							switch(HI_VENC_H264_REF_MODE){
								case H264E_REF_MODE_1X:
									*ref_counter %= 1;
									break;
								case H264E_REF_MODE_2X:
									*ref_counter %= 2;
									break;
								case H264E_REF_MODE_4X:
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

							if(0 == do_buffer_request(streamH264Attr->buf_id, attr.data_sz + sizeof(attr), attr.h264.keyframe)){	
								int i = 0;
								// buffer in the attribute
								do_buffer_append(streamH264Attr->buf_id, &attr, sizeof(attr));
								// buffer in the payload
								for(i = 0; i < venc_stream.u32PackCount; ++i){
									do_buffer_append(streamH264Attr->buf_id, venc_stream.pstPack[i].pu8Addr[0], venc_stream.pstPack[i].u32Len[0]);
									if(venc_stream.pstPack[i].u32Len[1] > 0){
										do_buffer_append(streamH264Attr->buf_id, venc_stream.pstPack[i].pu8Addr[1], venc_stream.pstPack[i].u32Len[1]);
									}
								}
								do_buffer_commit(streamH264Attr->buf_id);
							}
						}
						
						break;
						case kSDK_ENC_BUF_DATA_H265:
							//FIX ME
							
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
			attr.type = kSDK_ENC_BUF_DATA_G711A;
			attr.timestamp_us = audio_stream.u64TimeStamp;; // get the first nalu pts
			attr.time_us = get_time_us();
			attr.data_sz = audio_stream.u32Len - 4; // remove the heading 4bytes of hisilicon g711a
			attr.g711a.sample_rate = stream_g711_attr->sample_rate;
			attr.g711a.sample_width = stream_g711_attr->sample_width;
			attr.g711a.packet = attr.data_sz;
			attr.g711a.compression_ratio = 2.0;
			// relative to the video stream channel
			// store the audio frame to every relevant video stream
			for(i = 0; i < HI_AENC_STREAM_BACKLOG_REF; ++i){
				LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[stream_g711_attr->vin_ref][i];
				int const buf_id = streamH264Attr->buf_id;
				if(0 == do_buffer_request(buf_id, sizeof(stSDK_ENC_BUF_ATTR) + attr.data_sz, false)){
					do_buffer_append(buf_id, &attr, sizeof(attr));
					//SOC_DEBUG("audio len = %d", audio_stream.u32Len);
					// if the g711a packet len is 320
					// the former 4bytes == 0x00a00100
					// if the g711a packet len is 480
					// the former 4bytes == 0x00f00100
					//uint32_t* head = audio_stream.pStream;
					//SOC_DEBUG("%08x", *head);
					do_buffer_append(buf_id, audio_stream.pStream + 4, audio_stream.u32Len - 4);
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
		
		int const vencGroupJPEG = VENC_MAX_GRP_NUM - 1; // the last venc group
		int const vencChannelJPEG = VENC_MAX_CHN_NUM - 1; // the last venc channel
		int const vpssGroup = vin;
		VPSS_CROP_INFO_S stCropInfo;
		LP_SDK_ENC_STREAM_H264_ATTR streamH264Attr = &_sdk_enc.attr.h264_attr[0][0];
#if defined(HI3518A) | defined(HI3518C) | defined(HI3516C) | defined(HI3518E)
		int const vpss_last_ch = (sizeof(_hi3518_vpss_ch_map) / sizeof(_hi3518_vpss_ch_map[0])) - 1;
		int const vpssChannel = VPSS_LSTR_CHN;//_hi3518_vpss_ch_map[vpss_last_ch];
#else
		int const vpssChannel = VPSS_BYPASS_CHN; 
#endif
		VENC_CHN_ATTR_S vencChannelAttr;
		VENC_ATTR_JPEG_S *const vencAttrJPEG = &vencChannelAttr.stVeAttr.stAttrJpeg;
		MPP_CHN_S mppChannelVPSS;
		MPP_CHN_S mppChannelVENC;
		ST_SDK_VIN_CAPTURE_ATTR vin_capture_attr;

		sdk_vin->get_capture_attr(vin, &vin_capture_attr);

		pthread_mutex_lock(&_sdk_enc.attr.snapshot_mutex);
		
		// default size
		SOC_CHECK(HI_MPI_VPSS_GetCropCfg(vpssGroup, &stCropInfo));
		if(kSDK_ENC_SNAPSHOT_SIZE_AUTO == width || kSDK_ENC_SNAPSHOT_SIZE_AUTO == height){
			width = vin_capture_attr.width / 2;
			height = vin_capture_attr.height / 2;
		}else{
			if(kSDK_ENC_SNAPSHOT_SIZE_MAX == width){
				width = streamH264Attr->width;
				//width = vin_capture_attr.width;
			}
			if(kSDK_ENC_SNAPSHOT_SIZE_MAX == height){
				height = streamH264Attr->height;
				//height = vin_capture_attr.height;
			}
			if(kSDK_ENC_SNAPSHOT_SIZE_MIN == width){
				width = HI_VENC_JPEG_MIN_WIDTH;
			}
			if(kSDK_ENC_SNAPSHOT_SIZE_MIN == height){
				height = HI_VENC_JPEG_MIN_HEIGHT;
			}
		}

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


		/*if(height > 360){
			height = 360;
		}
		if(width > 640){
			width = 640;
		}*/

		width = SDK_ALIGNED_LITTLE_ENDIAN(width, 4);
		height = SDK_ALIGNED_LITTLE_ENDIAN(height, 4);

		// create venc group and channel
		vencChannelAttr.stVeAttr.enType = PT_JPEG;
		vencAttrJPEG->u32MaxPicWidth  = width;
		vencAttrJPEG->u32MaxPicHeight = height;
		vencAttrJPEG->u32PicWidth  = vencAttrJPEG->u32MaxPicWidth;
		vencAttrJPEG->u32PicHeight = vencAttrJPEG->u32MaxPicHeight;
		vencAttrJPEG->u32BufSize = vencAttrJPEG->u32MaxPicWidth * vencAttrJPEG->u32MaxPicHeight * 3 / 2;
		vencAttrJPEG->bByFrame = HI_TRUE; // get stream mode is field mode	or frame mode
		vencAttrJPEG->bVIField = HI_FALSE; // the sign of the VI picture is field or frame
		vencAttrJPEG->u32Priority = 1; // channels precedence level
		vencAttrJPEG->u32BufSize = SDK_ALIGNED_BIG_ENDIAN(vencAttrJPEG->u32BufSize, 64);
		
		SOC_CHECK(HI_MPI_VENC_CreateGroup(vencGroupJPEG));
		SOC_CHECK(HI_MPI_VENC_CreateChn(vencChannelJPEG, &vencChannelAttr));
		SOC_CHECK(HI_MPI_VENC_RegisterChn(vencGroupJPEG, vencChannelJPEG));

		if(vpssChannel >= VPSS_MAX_PHY_CHN_NUM){
			VPSS_EXT_CHN_ATTR_S vpss_ext_chn_attr;
			vpss_ext_chn_attr.s32BindChn = 1; // bind to vpss 1
			vpss_ext_chn_attr.s32SrcFrameRate = -1;
			vpss_ext_chn_attr.s32DstFrameRate = -1;
			vpss_ext_chn_attr.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
			vpss_ext_chn_attr.u32Width = width;
			vpss_ext_chn_attr.u32Height = height;
			SOC_CHECK(HI_MPI_VPSS_SetExtChnAttr(vpssGroup, vpssChannel, &vpss_ext_chn_attr));
		}


		// start to receive the picture from vi unit
		mppChannelVPSS.enModId = HI_ID_VPSS;
		mppChannelVPSS.s32DevId = vpssGroup;
		mppChannelVPSS.s32ChnId = vpssChannel; // attention here
		mppChannelVENC.enModId = HI_ID_GROUP;
		mppChannelVENC.s32DevId = vencGroupJPEG;
		mppChannelVENC.s32ChnId = vencChannelJPEG;
#if defined(HI3518A)|defined(HI3518C)|defined(HI3516C) | defined(HI3518E)
		SOC_CHECK(HI_MPI_VPSS_EnableChn(vpssGroup, vpssChannel));
#endif
		SOC_CHECK(HI_MPI_SYS_Bind(&mppChannelVPSS, &mppChannelVENC));
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
								fwrite(venc_stream.pstPack[i].pu8Addr[0], 1, venc_stream.pstPack[i].u32Len[0], stream);
								if(venc_stream.pstPack[i].u32Len[1]){
									fwrite(venc_stream.pstPack[i].pu8Addr[1], 1, venc_stream.pstPack[i].u32Len[1], stream);
								}
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
		SOC_CHECK(HI_MPI_SYS_UnBind(&mppChannelVPSS, &mppChannelVENC));
		SOC_CHECK(HI_MPI_VENC_UnRegisterChn(vencChannelJPEG));
		SOC_CHECK(HI_MPI_VENC_DestroyChn(vencChannelJPEG));
		SOC_CHECK(HI_MPI_VENC_DestroyGroup(vencGroupJPEG));

		pthread_mutex_unlock(&_sdk_enc.attr.snapshot_mutex);
		return 0;
	}
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
				canvas->pixels = calloc(canvas->width * canvas->height * sizeof(uint16_t), 1);
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
				&& 32 == bmp_hdr.bit_count){
				
				int const bmp_width = bmp_hdr.width;
				int const bmp_height = bmp_hdr.height;
				char *canvas_cache = calloc(bmp_hdr.size_image, 1);
				
				LP_SDK_ENC_VIDEO_OVERLAY_CANVAS canvas = NULL;
				stSDK_ENC_VIDEO_OVERLAY_PIXEL canvas_pixel;

				printf("IMAGE %dx%d size=%d offset=%d info=%d\n", bmp_width, bmp_height, bmp_hdr.size_image, bmp_hdr.off_bits, bmp_hdr.info_size);


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
					#if 0//fix me: use 24bit BMP
					char *const line_offset = canvas_cache + SDK_ALIGNED_BIG_ENDIAN(3 * bmp_width, 4) * (bmp_height - 1 - i) + 2;
					for(ii = 0; ii < bmp_width; ++ii){
						char *const column_offset = line_offset + 3 * ii;

						canvas_pixel.alpha = 0xff;
						canvas_pixel.red = column_offset[0];
						canvas_pixel.green = column_offset[1];
						canvas_pixel.blue = column_offset[2];

						canvas->put_pixel(canvas, ii, i, canvas_pixel);
					}	
					#else //fix me: use 32bit BMP
					char *const line_offset = canvas_cache + SDK_ALIGNED_BIG_ENDIAN(4 * bmp_width, 4) * (bmp_height - 1 - i) + 2;
					for(ii = 0; ii < bmp_width; ++ii){
						char *const column_offset = line_offset + 4 * ii;

						canvas_pixel.alpha = column_offset[0];
						canvas_pixel.red = column_offset[1];
						canvas_pixel.green = column_offset[2];
						canvas_pixel.blue = column_offset[3];

						canvas->put_pixel(canvas, ii, i, canvas_pixel);		
					}
					#endif
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
		int canvas_x = 0, canvas_y = 0;
		size_t canvas_width = 0, canvas_height = 0;
		LP_SDK_ENC_STREAM_H264_ATTR const streamH264Attr = &_sdk_enc.attr.h264_attr[vin][stream];
		lpSDK_ENC_VIDEO_OVERLAY_ATTR_SET const overlay_set = &_sdk_enc.attr.video_overlay_set[vin][stream];
		// check name override
		if(NULL != enc_lookup_overlay_byname(vin, stream, overlay_name)){
			SOC_DEBUG("Overlay name %s override", overlay_name);
			return -1;
		}
		
		// width /height
		canvas_x = (typeof(canvas_x))(x * (float)streamH264Attr->width);
		canvas_y = (typeof(canvas_y))(y * (float)(streamH264Attr->height));
		canvas_width = canvas->width;
		canvas_height = canvas->height;

		if(canvas_x >= streamH264Attr->width){
			canvas_x = streamH264Attr->width - 16;
		}
		if(canvas_y + canvas_height > streamH264Attr->height){
			canvas_y = streamH264Attr->height - canvas_height;
		}

		// alignment
		canvas_x = SDK_ALIGNED_LITTLE_ENDIAN(canvas_x, 16);
		canvas_y = SDK_ALIGNED_LITTLE_ENDIAN(canvas_y, 16);
		canvas_width = SDK_ALIGNED_BIG_ENDIAN(canvas_width, 2);
		canvas_height = SDK_ALIGNED_BIG_ENDIAN(canvas_height, 2);
		// adjustment
		if(canvas_y + canvas_height > streamH264Attr->height){
			canvas_y = streamH264Attr->height - canvas_height;
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

				if(1){//if(overlay->region_handle >= 0){
					RGN_HANDLE const region_handle = overlay->region_handle;
					RGN_ATTR_S region_attr;
					RGN_CHN_ATTR_S region_ch_attr;
					MPP_CHN_S mppChannelVENC;
//					BITMAP_S bitmap;

					memset(&region_attr, 0, sizeof(region_attr));
					region_attr.enType = OVERLAY_RGN;
					region_attr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_4444;
					
					region_attr.unAttr.stOverlay.stSize.u32Width = overlay->width;
					region_attr.unAttr.stOverlay.stSize.u32Height = overlay->height;
					region_attr.unAttr.stOverlay.u32BgColor = 0;
					SOC_CHECK(HI_MPI_RGN_Create(region_handle, &region_attr));

					memset(&mppChannelVENC, 0, sizeof(mppChannelVENC));
				    mppChannelVENC.enModId = HI_ID_GROUP;
			        mppChannelVENC.s32DevId = __HI_VENC_CH(vin, stream);
			        mppChannelVENC.s32ChnId = 0;

					memset(&region_ch_attr,0,sizeof(region_ch_attr));
			        region_ch_attr.bShow = HI_FALSE;
			        region_ch_attr.enType = OVERLAY_RGN;
			        region_ch_attr.unChnAttr.stOverlayChn.stPoint.s32X = overlay->x;// & (0xFFFFFFFF << (overlay->height / 16) + 3);
			        region_ch_attr.unChnAttr.stOverlayChn.stPoint.s32Y = overlay->y;// & (0xFFFFFFFF << (overlay->height / 16) + 3);
			        region_ch_attr.unChnAttr.stOverlayChn.u32BgAlpha = 0;
			        region_ch_attr.unChnAttr.stOverlayChn.u32FgAlpha = 64;
			        region_ch_attr.unChnAttr.stOverlayChn.u32Layer = 0;

			        region_ch_attr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
			        region_ch_attr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;

					region_ch_attr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = overlay->height;
					region_ch_attr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = overlay->height;
					region_ch_attr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;
					region_ch_attr.unChnAttr.stOverlayChn.stInvertColor.enChgMod = 1;
					if(!strcmp(overlay->name, "pic")){
						region_ch_attr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_FALSE;
						region_ch_attr.unChnAttr.stOverlayChn.stPoint.s32X = overlay->x;
			        	region_ch_attr.unChnAttr.stOverlayChn.stPoint.s32Y = overlay->y;
					}else{
						region_ch_attr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_TRUE;
						region_ch_attr.unChnAttr.stOverlayChn.stPoint.s32X = overlay->x ;//& (0xFFFFFFFF << (overlay->height / 16) + 3);
						region_ch_attr.unChnAttr.stOverlayChn.stPoint.s32Y = overlay->y ;//& (0xFFFFFFFF << (overlay->height / 16) + 3);
					}

					SOC_CHECK(HI_MPI_RGN_AttachToChn(region_handle, &mppChannelVENC, &region_ch_attr));

					SOC_DEBUG("Create overlay(%d,%d) @ %d [%dx%d] [%dx%d] [%dx%d] named \"%s\"",
						vin, stream, overlay->region_handle, overlay->width, overlay->height, region_ch_attr.unChnAttr.stOverlayChn.stPoint.s32X, 
						region_ch_attr.unChnAttr.stOverlayChn.stPoint.s32Y, overlay->x, overlay->y, overlay->name);

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
		&& stream < HI_VENC_STREAM_BACKLOG_REF){
		lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = enc_lookup_overlay_byname(vin, stream, overlay_name);
		if(NULL != overlay){
			MPP_CHN_S mppChannelVENC;

			// only clear the canvas is ok
			// about the canvas release is not my business
			overlay->canvas = NULL;
			memset(&mppChannelVENC, 0, sizeof(mppChannelVENC));
			mppChannelVENC.enModId = HI_ID_GROUP;
			mppChannelVENC.s32DevId = __HI_VENC_CH(vin, stream);
			mppChannelVENC.s32ChnId = 0;
			SOC_CHECK(HI_MPI_RGN_DetachFrmChn(overlay->region_handle, &mppChannelVENC));
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

		    mppChannel.enModId = HI_ID_GROUP;
		    mppChannel.s32DevId = __HI_VENC_CH(vin, stream);
		    mppChannel.s32ChnId = 0;
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

static int hi3518_enc_eptz_ctrl(int vin, int stream, int cmd, int param)
{
	return 0;
}

static int hi3518_enc_usr_mode(int vin, int stream, int fix_mode, int show_mode)
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
**描述:内存映射，根据长度在mmz映射相同长度的内存给用户空间
**返回:虚拟地址
***************************************************************************************/
static void  *hi3518_enc_sdk_mmap(void *param)
{
	VB_BLK VbBlk;
	HI_U8* pVirAddr;
	HI_U32 u32PhyAddr;
	VB_POOL VbPool;

	SDK_ENC_destroy();
	SDK_ISP_destroy();
	SDK_destroy_vin();

	sdk_audio->release_ain_ch(0);
	sdk_audio->destroy_ain();
	sdk_audio->release_ain_ch(0);
	sdk_audio->destroy_ain();

	SDK_destroy_audio();
	SDK_destroy_sys();
	//OVERLAY_destroy();
	ssize_t length = *((ssize_t *)param);

	if(HI_SUCCESS != hi3518_mpi_init())
	{
		printf("MPI_Init err\n");
		return -1;
	}

	/* create a video buffer pool*/
	VbPool = HI_MPI_VB_CreatePool(length,2,"anonymous");

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

	//返回地址
	return pVirAddr;
}
		
int  SDK_unmmap(void *virmem, int length)
{
	return HI_MPI_SYS_Munmap(virmem, length);
}

static int hi3521_update_overlay_by_text(int vin, int stream, const char* text)
{
	return -1;
}

static int hi3521_enc_resolution(int width, int height)
{	
	return -1;
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

		//fish eye
		.eptz_ctrl = hi3518_enc_eptz_ctrl,
		.enc_mode = hi3518_enc_usr_mode,

		//upgrade
		.upgrade_env_prepare = 	hi3518_enc_sdk_mmap,	
		.update_overlay_bytext = hi3521_update_overlay_by_text,

		//switch resolution
		.enc_resolution = hi3521_enc_resolution,
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
		
		// init the snapshot mutex
		pthread_mutex_init(&_sdk_enc.attr.snapshot_mutex, NULL);
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
			for(ii = 0; ii < HI_VENC_STREAM_BACKLOG_REF; ++ii){
				// destroy sub stream firstly
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
		return 0;
	}
	return -1;
}


int SDK_ENC_create_stream(int vin, int stream, LP_SDK_ENC_STREAM_ATTR stream_attr)
{	
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
}



int SDK_ENC_release_stream(int vin, int stream)
{
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
}



int SDK_ENC_set_stream(int vin, int stream,LP_SDK_ENC_STREAM_ATTR stream_attr)
{
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
}


int SDK_ENC_get_stream(int vin, int stream, LP_SDK_ENC_STREAM_ATTR stream_attr)
{
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
}

int SDK_ENC_enable_stream(int vin, int stream, bool flag)
{
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
}

int	SDK_ENC_request_stream_keyframe(int vin, int stream)
{	
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
}

