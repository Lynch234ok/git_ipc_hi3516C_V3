

#include "hi3515.h"
#include "sdk/sdk_api.h"
#include "sdk_trace.h"
#include <sys/prctl.h>

#define HI_VENC_CH_BACKLOG_REF (8)
#define HI_VENC_STREAM_BACKLOG_REF (2)
#define HI_AENC_CH_BACKLOG_REF (8)

#define HI_VENC_JPEG_MIN_WIDTH (80)
#define HI_VENC_JPEG_MAX_WIDTH (2048)
#define HI_VENC_JPEG_MIN_HEIGHT (64)
#define HI_VENC_JPEG_MAX_HEIGHT (1536)

#define HI_VENC_OVERLAY_BACKLOG_REF (4)
#define HI_VENC_OVERLAY_CANVAS_STOCK_REF (HI_VENC_OVERLAY_BACKLOG_REF * HI_VENC_OVERLAY_BACKLOG_REF)

#define __HI_AIN_CH (0)
#define __HI_VENC_CH(_vin, _stream) ((_vin)* HI_VENC_STREAM_BACKLOG_REF + (_stream))

#define ENC_H264_STREAM_ATTR_MAGIC (0xf0f0f0f0)

typedef struct SDK_ENC_VIDEO_OVERLAY_ATTR
{
	lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas;
	char name[32];
	int x, y;
	size_t width, height;
	REGION_HANDLE region_handle; 
}stSDK_ENC_VIDEO_OVERLAY_ATTR, *lpSDK_ENC_VIDEO_OVERLAY_ATTR;

typedef struct SDK_ENC_VIDEO_OVERLAY_ATTR_SET
{
	stSDK_ENC_VIDEO_OVERLAY_ATTR attr[HI_VENC_OVERLAY_BACKLOG_REF];
}stSDK_ENC_VIDEO_OVERLAY_ATTR_SET, *lpSDK_ENC_VIDEO_OVERLAY_ATTR_SET;

typedef struct SDK_ENC_ATTR {
	stSDK_ENC_H264_STREAM_ATTR video_stream_attr[HI_VENC_CH_BACKLOG_REF][HI_VENC_STREAM_BACKLOG_REF];
	uint8_t frame_ref_counter[HI_VENC_CH_BACKLOG_REF][HI_VENC_STREAM_BACKLOG_REF];
	stSDK_ENC_G711A_STREAM_ATTR audio_stream_attr[HI_AENC_CH_BACKLOG_REF];

	stSDK_ENC_VIDEO_OVERLAY_ATTR_SET video_overlay_set[HI_VENC_CH_BACKLOG_REF];
	stSDK_ENC_VIDEO_OVERLAY_CANVAS canvas_stock[HI_VENC_OVERLAY_CANVAS_STOCK_REF];
	
	bool loop_trigger;
	pthread_t loop_tid;
	pthread_mutex_t snapshot_mutex;
}stSDK_ENC_ATTR, *lpstSDK_ENC_ATTR;

typedef struct SDK_ENC_HI3515
{
	stSDK_ENC_API api;
	stSDK_ENC_ATTR attr;
}stSDK_ENC_HI3515, *lpSDK_ENC_HI3515;

static stSDK_ENC_HI3515 _sdk_enc;
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
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

static int vimap_hi3515(int vin_id, int* ret_dev, int* ret_ch)
{
	if(vin_id < 8){
		if(ret_dev){
			if(sdk_vin->hw_spec() == SDK_VIN_HW_SPEC_HI35XX_4D1X2_CH0_CH2){
				*ret_dev = (0 == (vin_id / 4)) ? 0 : 2;
			}else if(sdk_vin->hw_spec() == SDK_VIN_HW_SPEC_HI35XX_4D1X2_CH2_CH0){
				*ret_dev = (0 == (vin_id / 4)) ? 2 : 0;
			}else{
				return -1;
			}
		}
		if(ret_ch){
			*ret_ch = vin_id % 4;
		}
		return 0;
	}
	return -1;
}

static int bitrate_regulate(int const enc_width, int const enc_height, int const requested_bps)
{
	int const enc_size = enc_width * enc_height;
	int max_bps = 0;
	if(enc_size <= (180 * 144)){
		max_bps = 256; // kbps
	}else if(enc_size <= (360 * 144)){
		max_bps = 384;
	}else if(enc_size <= (480 * 144)){
		max_bps = 480;
	}else if(enc_size <= (360 * 288)){
		max_bps = 512; // kbps
	}else if(enc_size <= (480 * 288)){
		max_bps = 640;
	}else if(enc_size <= (720 * 288)){
		max_bps = 768;
	}else if(enc_size <= (960 * 288)){
		max_bps = 1024;
	}else if(enc_size <= (720 * 576)){
		max_bps = 1536; // kbps
	}else if(enc_size <= (960 * 576)){
		max_bps = 2048; // kbps
	}
	if(requested_bps < max_bps){
		return requested_bps;
	}
	return max_bps;
}

static int enc_lookup_stream_byname(const char* name, int* ret_vin, int* ret_stream)
{
	int i = 0, ii = 0;
	for(i = 0; i < HI_VENC_CH_BACKLOG_REF; ++i){
		for(ii = 0; ii < HI_VENC_STREAM_BACKLOG_REF; ++ii){
			lpSDK_ENC_H264_STREAM_ATTR stream_attr = &_sdk_enc.attr.video_stream_attr[i][ii];
			if(ENC_H264_STREAM_ATTR_MAGIC == stream_attr->magic){
				// lookup which is init
				if(0 == strcasecmp(stream_attr->name, name)
					&& strlen(stream_attr->name) == strlen(name)){

					if(ret_vin){
						*ret_vin = i;
					}
					if(ret_stream){
						*ret_stream = ii;
					}
					return SDK_SUCCESS;
				}
			}
		}
	}
	return SDK_FAILURE;
}

static int enc_create_h264_stream(const char* name, int vin, int stream, lpSDK_ENC_H264_STREAM_ATTR stream_attr)
{
	if(vin < HI_VENC_CH_BACKLOG_REF){
		if(stream < HI_VENC_STREAM_BACKLOG_REF){
			lpSDK_ENC_H264_STREAM_ATTR const main_stream_attr = &_sdk_enc.attr.video_stream_attr[vin][0];
			lpSDK_ENC_H264_STREAM_ATTR const this_stream_attr = &_sdk_enc.attr.video_stream_attr[vin][stream];
			if(0 == this_stream_attr->magic){
				VENC_ATTR_H264_REF_MODE_E venc_attr_h264_ref_mode;
				VENC_CHN_ATTR_S venc_chn_attr;
				VENC_ATTR_H264_S venc_attr_h264;
				VENC_ATTR_H264_RC_S venc_attr_h264_rc = {0};
				int const venc_group = vin;
				int const venc_ch = vin * HI_VENC_STREAM_BACKLOG_REF + stream;
				bool is_main = (0 == stream) ? true : false;
				
				// only magic is null could be init
				// init this stream attribute;
				memcpy(this_stream_attr, stream_attr, sizeof(stSDK_ENC_H264_STREAM_ATTR));
				strncpy(this_stream_attr->name, name, sizeof(this_stream_attr->name));
				this_stream_attr->magic = ENC_H264_STREAM_ATTR_MAGIC; // mark it
				this_stream_attr->vin = vin;
				this_stream_attr->stream = stream;
				if(stream > 0){
					// the size of stream depands on the main stream size
					if((this_stream_attr->width != main_stream_attr->width && this_stream_attr->width * 2 != main_stream_attr->width)
						|| (this_stream_attr->height != main_stream_attr->height && this_stream_attr->height * 2 != main_stream_attr->height)){

						this_stream_attr->width = main_stream_attr->width / 2;
						this_stream_attr->height = main_stream_attr->height / 2;
					}
				}
				this_stream_attr->start = false; // very important, declare the encode status
				
				SDK_ZERO_VAL(venc_chn_attr);
				venc_chn_attr.enType = PT_H264;	// both h264
				venc_chn_attr.pValue = &venc_attr_h264;

				SDK_ZERO_VAL(venc_attr_h264);
				venc_attr_h264.bMainStream = is_main ? HI_TRUE : HI_FALSE;
				venc_attr_h264.bField = HI_FALSE;
				venc_attr_h264.bVIField = HI_FALSE;
				venc_attr_h264.u32PicWidth = this_stream_attr->width;
				venc_attr_h264.u32PicHeight = this_stream_attr->height;
				venc_attr_h264.u32ViFramerate = this_stream_attr->vin_fps;
				venc_attr_h264.u32TargetFramerate = this_stream_attr->fps;
				venc_attr_h264.u32Gop = this_stream_attr->gop;
				venc_attr_h264.u32MaxDelay = 100;
				venc_attr_h264.u32Priority = 0;
				venc_attr_h264.bByFrame = HI_TRUE;
				venc_attr_h264.u32BufSize = venc_attr_h264.u32PicWidth * venc_attr_h264.u32PicHeight * 3 / 2;
				venc_attr_h264.enRcMode = RC_MODE_CBR;
				venc_attr_h264.u32PicLevel = 0;
				//venc_attr_h264.u32Bitrate = this_stream_attr->bps;
				venc_attr_h264.u32Bitrate = bitrate_regulate(this_stream_attr->width, this_stream_attr->height, this_stream_attr->bps);

				SOC_DEBUG("Creating h264 %d,%d", vin, stream);

				// create video encode group
				if(is_main){
					int vi_dev = 0;
					int vi_chn = 0;
					vimap_hi3515(vin, &vi_dev, &vi_chn);
					SOC_DEBUG("Binding to (%d,%d)", vi_dev, vi_chn);
					SOC_CHECK(HI_MPI_VENC_CreateGroup(venc_group));
					SOC_CHECK(HI_MPI_VENC_BindInput(venc_group, vi_dev, vi_chn));
				}

				// create video encode channel and register it
				SOC_CHECK(HI_MPI_VENC_CreateChn(venc_ch, &venc_chn_attr, HI_NULL));
				SOC_CHECK(HI_MPI_VENC_RegisterChn(venc_group, venc_ch));

				// set h264 rc parameter
				SOC_CHECK(HI_MPI_VENC_GetH264eRcPara(venc_ch, &venc_attr_h264_rc));
				venc_attr_h264_rc.s32IdrQpMax = 42; // 050 version description point 9
				venc_attr_h264_rc.bFrameLostAllow = HI_TRUE;
				venc_attr_h264_rc.s32OsdProtectEn = HI_FALSE;
				venc_attr_h264_rc.bSceneChangeClip = HI_FALSE; // 050 version description point 13
				venc_attr_h264_rc.bVbrQpDownAllowed = HI_TRUE;
				SOC_CHECK(HI_MPI_VENC_SetH264eRcPara(venc_ch, &venc_attr_h264_rc));

				// set h264 reference mode
				SOC_CHECK(HI_MPI_VENC_GetH264eRefMode(venc_ch, &venc_attr_h264_ref_mode));
				venc_attr_h264_ref_mode = H264E_REF_MODE_4X;
				SOC_CHECK(HI_MPI_VENC_SetH264eRefMode(venc_ch, venc_attr_h264_ref_mode));
				return SDK_SUCCESS;
			}
			
		}
	}
	return SDK_FAILURE;
}

static int enc_release_h264_stream(int vin, int stream)
{
	lpSDK_ENC_H264_STREAM_ATTR stream_attr = &_sdk_enc.attr.video_stream_attr[vin][stream];
	if(vin < HI_VENC_CH_BACKLOG_REF && stream < HI_VENC_STREAM_BACKLOG_REF){
		if(ENC_H264_STREAM_ATTR_MAGIC == stream_attr->magic){
			int const venc_group = vin;
			int const venc_ch = vin * HI_VENC_STREAM_BACKLOG_REF + stream;

			SOC_DEBUG("Release video encode (%d,%d)", vin, stream);
			SOC_CHECK(HI_MPI_VENC_StopRecvPic(venc_ch));
			SOC_CHECK(HI_MPI_VENC_UnRegisterChn(venc_ch));
			SOC_CHECK(HI_MPI_VENC_DestroyChn(venc_ch));

			if(0 == stream){
				SOC_CHECK(HI_MPI_VENC_UnbindInput(venc_group));
				SOC_CHECK(HI_MPI_VENC_DestroyGroup(venc_group));
			}

			// clear the magic
			stream_attr->magic = 0;
			return SDK_SUCCESS;
		}
	}
	return SDK_FAILURE;
}

static int enc_create_g711a_stream(int ain, int vin_ref, lpSDK_ENC_G711A_STREAM_ATTR stream_attr)
{
	if(ain < HI_AENC_CH_BACKLOG_REF){
		if(vin_ref < HI_VENC_CH_BACKLOG_REF){
			int const aenc_ch = ain;
			lpSDK_ENC_G711A_STREAM_ATTR const this_stream_attr = &_sdk_enc.attr.audio_stream_attr[ain];
		
			if(0 == this_stream_attr->magic){
				AENC_ATTR_G711_S aenc_attr_g711 = {0};
				AENC_CHN_ATTR_S aenc_ch_attr = {.enType = PT_G711A, .pValue = &aenc_attr_g711,};

				aenc_ch_attr.enType = PT_G711A;
				aenc_ch_attr.pValue = &aenc_attr_g711;
				//aenc_ch_attr.u32BufSize = 8;
				//aenc_ch_attr.u32BufSize = MAX_AUDIO_FRAME_NUM; // ver 050 faq 1.6.3
				aenc_ch_attr.u32BufSize = 5;

				memcpy(this_stream_attr, stream_attr, sizeof(stSDK_ENC_G711A_STREAM_ATTR));
				this_stream_attr->magic = ENC_H264_STREAM_ATTR_MAGIC; // mark it
				this_stream_attr->ain = ain;
				this_stream_attr->vin_ref = vin_ref;
				// FIXME: remember to check the sample rate and bitwidth
				this_stream_attr->sample_rate = kSDK_ENC_AUDIO_SAMPLERATE_8KBPS;
				this_stream_attr->bit_width = kSDK_ENC_AUDIO_BITWIDTH_16BITS;
				this_stream_attr->packet_size = 320;

				// sdk operate
				// enable vin
				SOC_CHECK(HI_MPI_AI_EnableChn(__HI_AIN_CH, ain));
				// create aenc chn
				SOC_CHECK(HI_MPI_AENC_CreateChn(aenc_ch, &aenc_ch_attr));
				// bind AENC to AI channel
				SOC_CHECK(HI_MPI_AENC_BindAi(aenc_ch, __HI_AIN_CH, ain));

				return SDK_SUCCESS;
			}
		}
	}
	return SDK_FAILURE;
}

static int enc_release_g711a_stream(int ain)
{
	if(ain < HI_AENC_CH_BACKLOG_REF){
		int const aenc_ch = ain;
		lpSDK_ENC_G711A_STREAM_ATTR const this_stream_attr = &_sdk_enc.attr.audio_stream_attr[ain];
		
		// unbind AENC
		SOC_CHECK(HI_MPI_AENC_UnBindAi(aenc_ch, __HI_AIN_CH, ain));
		// destroy aenc chn
		SOC_CHECK(HI_MPI_AENC_DestroyChn(aenc_ch));
		// disable vin
		SOC_CHECK(HI_MPI_AI_DisableChn(__HI_AIN_CH, ain));

		// clear flag
		this_stream_attr->magic = 0;
		return SDK_SUCCESS;
	}

	return SDK_FAILURE;
}



static int enc_start_h264_stream(int vin, int stream)
{
	lpSDK_ENC_H264_STREAM_ATTR const stream_attr = &_sdk_enc.attr.video_stream_attr[vin][stream];
	if(!stream_attr->start){
		SOC_CHECK(HI_MPI_VENC_StartRecvPic(__HI_VENC_CH(vin, stream)));
		stream_attr->start = true;
		return SDK_SUCCESS;
	}
	return SDK_FAILURE;
}

static int enc_stop_h264_stream(int vin, int stream)
{
	lpSDK_ENC_H264_STREAM_ATTR const stream_attr = &_sdk_enc.attr.video_stream_attr[vin][stream];
	if(stream_attr->start){
		SOC_CHECK(HI_MPI_VENC_StopRecvPic(__HI_VENC_CH(vin, stream)));
		stream_attr->start = false;
		return SDK_SUCCESS;
	}
	return SDK_FAILURE;
}

static int enc_request_h264_keyframe(int vin, int stream)
{
	SOC_CHECK(HI_MPI_VENC_RequestIDR(__HI_VENC_CH(vin, stream)));
	return SDK_SUCCESS;
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
#define HI3515_MAX_VENC_PACK (4)
	VENC_STREAM_S venc_stream;
	VENC_CHN_STAT_S venc_chn_stat;
	VENC_PACK_S venc_pack[HI3515_MAX_VENC_PACK];
	int venc_ch = __HI_VENC_CH(vin, stream);
	lpSDK_ENC_H264_STREAM_ATTR const stream_attr = &_sdk_enc.attr.video_stream_attr[vin][stream];
	uint8_t* const frame_ref_counter = &_sdk_enc.attr.frame_ref_counter[vin][stream];
	int const buf_id = stream_attr->buf_id;

//	SOC_DEBUG("Querying venc ch %d", venc_id);
	if(HI_SUCCESS == HI_MPI_VENC_Query(venc_ch, &venc_chn_stat)){
		if(venc_chn_stat.u32CurPacks > 0){
			venc_stream.pstPack = venc_pack;
			venc_stream.u32PackCount = venc_chn_stat.u32CurPacks;
			SOC_ASSERT(venc_stream.u32PackCount <= HI3515_MAX_VENC_PACK, "Invalid sdk packet %d", venc_stream.u32PackCount);
			SOC_CHECK(HI_MPI_VENC_GetStream(venc_ch, &venc_stream, HI_IO_NOBLOCK));
			///////////////////////////////////////////////////////
			// do something to get the frame
			if(buf_id >= 0){
				ssize_t in_data_size = 0;
				bool is_keyframe = (HI3515_MAX_VENC_PACK == venc_stream.u32PackCount) ? true : false;
				stSDK_ENC_BUF_ATTR attr;
				
				if(is_keyframe){
					*frame_ref_counter = 0;
				}else{
					++*frame_ref_counter;
					// ref mode x4
					*frame_ref_counter %= 4;
				}
				attr.magic = kSDK_ENC_BUF_DATA_MAGIC;
				attr.type = kSDK_ENC_BUF_DATA_H264;
				attr.timestamp_us = venc_stream.pstPack[0].u64PTS; // get the first nalu pts
				attr.time_us = get_time_us();
				attr.data_sz = _stream_size(venc_stream);
				attr.h264.keyframe = is_keyframe ? true : false;
				attr.h264.ref_counter = *frame_ref_counter;
				attr.h264.fps = stream_attr->fps;
				attr.h264.width = stream_attr->width;
				attr.h264.height = stream_attr->height;

				
				in_data_size = sizeof(stSDK_ENC_BUF_ATTR) + attr.data_sz;
				//SOC_DEBUG("get data size(%d) from ref(%d) venc(%d) to buf(%d)", attr.h264.ref_counter, in_data_size, venc_id, buf_id);
				if(1){//if(0 == attr.h264.ref_counter || 2 == attr.h264.ref_counter){
					if(1){//if(in_data_size < stream_attr->frame_limit){
						
						if(0 == do_buffer_request(buf_id, in_data_size, attr.h264.keyframe)){	
							int i = 0;
							// buffer in the attribute
							do_buffer_append(buf_id, &attr, sizeof(attr));
							// buffer in the payload
							for(i = 0; i < venc_stream.u32PackCount; ++i){
								do_buffer_append(buf_id, venc_stream.pstPack[i].pu8Addr[0], venc_stream.pstPack[i].u32Len[0]);
								if(venc_stream.pstPack[i].u32Len[1] > 0){
									do_buffer_append(buf_id, venc_stream.pstPack[i].pu8Addr[1], venc_stream.pstPack[i].u32Len[1]);
								}
							}
							do_buffer_commit(buf_id);
							
						}
					}else{
						// request a new idr
						SOC_DEBUG("drop a frame size = %d", in_data_size);
						SOC_CHECK(HI_MPI_VENC_RequestIDR(venc_ch));
					}
				}
			}
			///////////////////////////////////////////////////////
			SOC_CHECK(HI_MPI_VENC_ReleaseStream(venc_ch, &venc_stream));
			return 0;
		}
	}
	return -1;
}

static int enc_audio_proc(int ain)
{
	int i = 0;
	lpSDK_ENC_G711A_STREAM_ATTR const audio_stream_attr = &_sdk_enc.attr.audio_stream_attr[ain];
	if(ENC_H264_STREAM_ATTR_MAGIC == audio_stream_attr->magic){
		AUDIO_STREAM_S audio_stream = {0};
		if(HI_SUCCESS == HI_MPI_AENC_GetStream(ain, &audio_stream, HI_IO_NOBLOCK)){
			stSDK_ENC_BUF_ATTR attr;
			attr.magic = kSDK_ENC_BUF_DATA_MAGIC;
			attr.type = kSDK_ENC_BUF_DATA_G711A;
			attr.timestamp_us = audio_stream.u64TimeStamp;
			attr.time_us = get_time_us();
			attr.data_sz = audio_stream.u32Len - 4; // remove the heading 4bytes of hisilicon g711a
			attr.g711a.sample_rate = 8000; // FIXME:
			attr.g711a.sample_width = 16; // FIXME:
			attr.g711a.packet = attr.data_sz;
			attr.g711a.compression_ratio = 2.0;
	
			// relative to the video stream channel
			// store the audio frame to every relevant video stream
			for(i = 0; i < HI_VENC_STREAM_BACKLOG_REF; ++i){
				lpSDK_ENC_H264_STREAM_ATTR const video_stream_attr = &_sdk_enc.attr.video_stream_attr[audio_stream_attr->vin_ref][i];
				if(0 == do_buffer_request(video_stream_attr->buf_id, sizeof(stSDK_ENC_BUF_ATTR) + attr.data_sz, false)){
					do_buffer_append(video_stream_attr->buf_id, &attr, sizeof(attr));
					//SOC_DEBUG("audio len = %d", audio_stream.u32Len);
					// if the g711a packet len is 320
					// the former 4bytes == 0x00a00100
					// if the g711a packet len is 480
					// the former 4bytes == 0x00f00100
					//uint32_t* head = audio_stream.pStream;
					//SOC_DEBUG("%08x", *head);
					do_buffer_append(video_stream_attr->buf_id, audio_stream.pStream + 4, audio_stream.u32Len - 4);
					do_buffer_commit(video_stream_attr->buf_id);			
				}
			}
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
		for(i = 0; i < HI_AENC_CH_BACKLOG_REF; ++i){
			while(0 == enc_audio_proc(i));
		}
		// main stream first
		for(i = 0; i < HI_VENC_CH_BACKLOG_REF; ++i){
			for(ii = 0; ii < HI_VENC_STREAM_BACKLOG_REF; ++ii){
				while(0 == enc_video_proc(i, ii));
			}
		}
		// between 30
		usleep(30000);
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
		return SDK_SUCCESS;
	}
	return SDK_FAILURE;
}

static int enc_stop()
{
	if(_sdk_enc.attr.loop_tid){
		_sdk_enc.attr.loop_trigger = false;
		pthread_join(_sdk_enc.attr.loop_tid, NULL);
		_sdk_enc.attr.loop_tid = (pthread_t)NULL;
		return SDK_SUCCESS;
	}
	return SDK_FAILURE;
}

static int enc_snapshot(int vin, enSDK_ENC_SNAPSHOT_QUALITY quality, ssize_t width, ssize_t height, FILE* stream)
{
	if(vin < HI_VENC_CH_BACKLOG_REF){
		lpSDK_ENC_H264_STREAM_ATTR const video_stream_attr = &_sdk_enc.attr.video_stream_attr[vin][0];
		int i = 0;
		int ret = -1;
		VENC_ATTR_JPEG_S venc_attr_jpeg;
		VENC_CHN_ATTR_S venc_chn_attr;
		VENC_STREAM_S venc_stream;

		int venc_fd = -1;
		fd_set read_fd_set;
		struct timeval time_out = {0};

		int const snap_grp = VENC_MAX_GRP_NUM - 1;
		int const snap_chn = VENC_MAX_CHN_NUM - 1;
		int vi_dev = 0;
		int vi_chn = 0;

		int const width_min = video_stream_attr->width / 4;
		int const width_max = video_stream_attr->width;
		int const height_min = video_stream_attr->height / 4;
		int const height_max = video_stream_attr->height;
		
		// default size
		if(kSDK_ENC_SNAPSHOT_SIZE_AUTO == width || kSDK_ENC_SNAPSHOT_SIZE_AUTO == height){
			width = video_stream_attr->width / 2;
			height = video_stream_attr->height / 2;
		}else{
			if(kSDK_ENC_SNAPSHOT_SIZE_MAX == width){
				width = width_max;
			}
			if(kSDK_ENC_SNAPSHOT_SIZE_MAX == height){
				height = height_max;
			}
			if(kSDK_ENC_SNAPSHOT_SIZE_MIN == width){
				width = HI_VENC_JPEG_MIN_WIDTH;
			}
			if(kSDK_ENC_SNAPSHOT_SIZE_MIN == height){
				height = HI_VENC_JPEG_MIN_HEIGHT;
			}
		}

		// check the hisilicon size limited
		if(width < width_min){
			width = width_min;
		}
		if(width > width_max){
			width = width_max;
		}
		if(height < height_min){
			height = height_min;
		}
		if(height > height_max){
			height = height_max;
		}

		pthread_mutex_lock(&_sdk_enc.attr.snapshot_mutex);
		
		////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////
		// snapshot code body
		SDK_ZERO_VAL(venc_attr_jpeg);
		venc_attr_jpeg.u32Priority = 0;
		venc_attr_jpeg.u32PicWidth = ((width + 15) / 16) * 16;
		venc_attr_jpeg.u32PicHeight = ((height + 15) / 16) * 16;
		venc_attr_jpeg.u32BufSize = venc_attr_jpeg.u32PicWidth * venc_attr_jpeg.u32PicHeight * 2;
		venc_attr_jpeg.bVIField = HI_FALSE;
		venc_attr_jpeg.bByFrame = HI_TRUE;
		venc_attr_jpeg.u32MCUPerECS = 0;
		switch(quality){
			case kSDK_ENC_SNAPSHOT_QUALITY_HIGHEST:
			case kSDK_ENC_SNAPSHOT_QUALITY_HIGH:
			case kSDK_ENC_SNAPSHOT_QUALITY_MEDIUM: 
			case kSDK_ENC_SNAPSHOT_QUALITY_LOW:
				venc_attr_jpeg.u32ImageQuality = quality;
				break;
			default:
				venc_attr_jpeg.u32ImageQuality = venc_attr_jpeg.u32ImageQuality;
				break;
		}
		SDK_ZERO_VAL(venc_chn_attr);
		venc_chn_attr.enType = PT_JPEG; // both h264
		venc_chn_attr.pValue = &venc_attr_jpeg;

		// jpeg encode init
		SOC_CHECK(HI_MPI_VENC_CreateGroup(snap_grp));
		SOC_CHECK(HI_MPI_VENC_CreateChn(snap_chn, &venc_chn_attr, HI_NULL));
		//SOC_CHECK(HI_MPI_VENC_SetMaxStreamCnt(snap_chn, 1));
		// start capture
		vimap_hi3515(vin, &vi_dev, &vi_chn);
		SOC_CHECK(HI_MPI_VENC_BindInput(snap_chn, vi_dev, vi_chn));
		SOC_CHECK(HI_MPI_VENC_RegisterChn(snap_grp, snap_chn));
		SOC_CHECK(HI_MPI_VENC_StartRecvPic(snap_chn));

		// waiting for snapshot
		venc_fd = HI_MPI_VENC_GetFd(snap_chn);
		SDK_ASSERT(venc_fd);
			
		time_out.tv_sec  = 1;
		time_out.tv_usec = 0;

		FD_ZERO(&read_fd_set);
		FD_SET(venc_fd, &read_fd_set);
		ret = select(venc_fd + 1, &read_fd_set, NULL, NULL, &time_out);
		if(ret > 0){
			if(FD_ISSET(venc_fd, &read_fd_set)){
				VENC_CHN_STAT_S venc_chn_stat;
				SOC_CHECK(HI_MPI_VENC_Query(snap_chn, &venc_chn_stat));
				if(venc_chn_stat.u32CurPacks > 0){
					const uint8_t jpeg_soi[] = {0xff, 0xd8};
					const uint8_t jpeg_eoi[] = {0xff, 0xd9};
					venc_stream.pstPack = (VENC_PACK_S*)alloca(sizeof(VENC_PACK_S) * venc_chn_stat.u32CurPacks);
					venc_stream.u32PackCount = venc_chn_stat.u32CurPacks;
					SOC_CHECK(HI_MPI_VENC_GetStream(snap_chn, &venc_stream, HI_IO_NOBLOCK));
					if(stream){
						fwrite(jpeg_soi, 1, sizeof(jpeg_soi), stream);
						for (i = 0; i < venc_stream.u32PackCount; ++i){
							fwrite(venc_stream.pstPack[i].pu8Addr[0], venc_stream.pstPack[i].u32Len[0], 1, stream);
							if(venc_stream.pstPack[i].u32Len[1]){
								fwrite(venc_stream.pstPack[i].pu8Addr[1], venc_stream.pstPack[i].u32Len[1], 1, stream);
							}
						}
						fwrite(jpeg_eoi, 1, sizeof(jpeg_soi), stream);
					}
					SOC_CHECK(HI_MPI_VENC_ReleaseStream(snap_chn, &venc_stream));
				}
			}
		}

		// de-init
		SOC_CHECK(HI_MPI_VENC_StopRecvPic(snap_chn));
		SOC_CHECK(HI_MPI_VENC_UnRegisterChn(snap_chn));
		SOC_CHECK(HI_MPI_VENC_UnbindInput(snap_chn));
		SOC_CHECK(HI_MPI_VENC_DestroyChn(snap_chn));
		SOC_CHECK(HI_MPI_VENC_DestroyGroup(snap_chn));

		SOC_DEBUG("Snapshot @ %d (%d, %d)!", vin, venc_attr_jpeg.u32PicWidth, venc_attr_jpeg.u32PicHeight);
		pthread_mutex_unlock(&_sdk_enc.attr.snapshot_mutex);
		return SDK_SUCCESS;
	}
	return SDK_FAILURE;
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


static int overlay_canvas_put_pixel(lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas, int x, int y, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel)
{
	if(canvas){
		if(x < canvas->width && y < canvas->height){
			uint16_t* const pixels = (uint16_t*)canvas->pixels;
			*(pixels + y * canvas->width + x) = overlay_pixel_argb4444(pixel);
			return SDK_SUCCESS;
		}
	}
	return SDK_FAILURE;
}

static int overlay_canvas_get_pixel(lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas, int x, int y, stSDK_ENC_VIDEO_OVERLAY_PIXEL* ret_pixel)
{
	if(canvas){
		if(x < canvas->width && y < canvas->height){
			uint16_t* const pixels = (uint16_t*)canvas->pixels;
			if(ret_pixel){
				*ret_pixel = overlay_pixel_argb8888(*(pixels + y * canvas->width + x));
				return SDK_SUCCESS;
			}
		}
	}
	return SDK_FAILURE;
}

static bool overlay_canvas_match_pixel(lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel1, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel2)
{
	return overlay_pixel_argb4444(pixel1) == overlay_pixel_argb4444(pixel2);
}

static int overlay_canvas_put_rect(lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas, int x, int y, size_t width, size_t height,stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel)
{
	if(canvas){
		if(x < canvas->width && y < canvas->height){
			int i, ii;
			uint16_t* const pixels = (uint16_t*)(canvas->pixels);
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
			return SDK_SUCCESS;
		}
	}
	return SDK_FAILURE;
}

static int overlay_canvas_fill_rect(lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas, int x, int y, size_t width, size_t height, stSDK_ENC_VIDEO_OVERLAY_PIXEL pixel)
{
	if(canvas){
		if(!width){
			width = canvas->width;
		}
		if(!height){
			height = canvas->height;
		}
		
		if(x < canvas->width && y < canvas->height){
			int i, ii;
			uint16_t* const pixels = (uint16_t*)(canvas->pixels);
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
			return SDK_SUCCESS;
		}
	}
	return SDK_FAILURE;
}

static int overlay_canvas_erase_rect(lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas, int x, int y, size_t width, size_t height)
{
	stSDK_ENC_VIDEO_OVERLAY_PIXEL erase_pixel;
	erase_pixel.alpha = 0;
	erase_pixel.red = 0;
	erase_pixel.green = 0;
	erase_pixel.blue = 0;
	return canvas->fill_rect(canvas, x, y, width, height, erase_pixel);
}


static lpSDK_ENC_VIDEO_OVERLAY_CANVAS enc_create_overlay_canvas(size_t width, size_t height)
{
	int i = 0;
	if(width > 0 && height > 0){
		lpSDK_ENC_VIDEO_OVERLAY_CANVAS const canvas_stock = _sdk_enc.attr.canvas_stock;
		for(i = 0; i < HI_VENC_OVERLAY_CANVAS_STOCK_REF; ++i){
			lpSDK_ENC_VIDEO_OVERLAY_CANVAS const canvas = canvas_stock + i;
			if(!canvas->pixels){
				// has not been allocated
				lpSDK_ENC_VIDEO_OVERLAY_CANVAS const canvas = &canvas_stock[i];

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

static void enc_release_overlay_canvas(lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas)
{
	if(canvas){
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
		&& 0 == stream){
		stSDK_ENC_VIDEO_OVERLAY_ATTR_SET* const overlay_set = &_sdk_enc.attr.video_overlay_set[vin];
		// check name override
		for(i = 0; i < HI_VENC_OVERLAY_BACKLOG_REF; ++i){
			lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = &overlay_set->attr[i];
			if(overlay->canvas && 0 == strcmp(overlay->name, name)){
				// what's my target
				return overlay;
			}
		}
	}
	return NULL;
}
	

static int enc_create_overlay(int vin, int stream, const char* overlay_name,
		int x, int y, size_t width, size_t height, lpSDK_ENC_VIDEO_OVERLAY_CANVAS const canvas)
{
	int i = 0;
	if(NULL != canvas && vin < HI_VENC_CH_BACKLOG_REF && 0 == stream){
		stSDK_ENC_VIDEO_OVERLAY_ATTR_SET* const overlay_set = &_sdk_enc.attr.video_overlay_set[vin];
		// check name override
		if(NULL != enc_lookup_overlay_byname(vin, stream, overlay_name)){
			SOC_DEBUG("Overlay name %s override", overlay_name);
			return SDK_FAILURE;
		}
		// create
		for(i = 0; i < HI_VENC_OVERLAY_BACKLOG_REF; ++i){
			lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = &overlay_set->attr[i];
			VENC_GRP const venc_group = vin;
			REGION_ATTR_S region_attr;
			
			if(!overlay->canvas){

				
				overlay->canvas = canvas;
				snprintf(overlay->name, sizeof(overlay->name), "%s", overlay_name);
				overlay->x = SDK_ALIGNED_BIG_ENDIAN(x, 8);
				overlay->y = SDK_ALIGNED_BIG_ENDIAN(y, 2);
				overlay->width = width;
				overlay->height = height;
				overlay->region_handle = 0; // clear it
				SOC_DEBUG("Create overlay @ %d name %s", overlay->region_handle, overlay->name);

				region_attr.enType = OVERLAY_REGION;
				region_attr.unAttr.stOverlay.bPublic = HI_FALSE;
				region_attr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_4444;
				region_attr.unAttr.stOverlay.stRect.s32X = overlay->x;
				region_attr.unAttr.stOverlay.stRect.s32Y = overlay->y;
				region_attr.unAttr.stOverlay.stRect.u32Width = overlay->width;
				region_attr.unAttr.stOverlay.stRect.u32Height = overlay->height;
				region_attr.unAttr.stOverlay.u32BgAlpha = 128;
				region_attr.unAttr.stOverlay.u32FgAlpha = 128;
				region_attr.unAttr.stOverlay.u32BgColor = 0;
				region_attr.unAttr.stOverlay.VeGroup = venc_group;

				SOC_CHECK(HI_MPI_VPP_CreateRegion(&region_attr, &overlay->region_handle));
				
				SOC_DEBUG("Create overlay @ %d name %s", overlay->region_handle, overlay->name);
				return SDK_SUCCESS;
			}
		}
	}
	return SDK_FAILURE;
}

static int enc_release_overlay(int vin, int stream, const char* overlay_name)
{
	if(vin < HI_VENC_CH_BACKLOG_REF && 0 == stream){
		lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = enc_lookup_overlay_byname(vin, stream, overlay_name);
		if(NULL != overlay){
			// only clear the canvas is ok
			// about the canvas release is not my business
			overlay->canvas = NULL;
			SOC_CHECK(HI_MPI_VPP_DestroyRegion(overlay->region_handle));
			overlay->region_handle = 0; // clear it
			return SDK_SUCCESS;
		}
	}
	return SDK_FAILURE;
}

static lpSDK_ENC_VIDEO_OVERLAY_CANVAS enc_get_overlay_canvas(int vin, int stream, const char* overlay_name)
{
	lpSDK_ENC_VIDEO_OVERLAY_ATTR overlay = enc_lookup_overlay_byname(vin, stream, overlay_name);
	if(overlay){
		return overlay->canvas;
	}
	return NULL;
}

static int enc_show_overlay(int vin, int stream, const char* overlay_name, bool show)
{
	if(vin < HI_VENC_CH_BACKLOG_REF && 0 == stream){
		lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = enc_lookup_overlay_byname(vin, stream, overlay_name);
		if(NULL != overlay){
			REGION_HANDLE const region_handle = overlay->region_handle;
			SOC_CHECK(HI_MPI_VPP_ControlRegion(region_handle, show ? REGION_SHOW : REGION_HIDE, HI_NULL));
			return SDK_SUCCESS;
		}
	}
	return SDK_FAILURE;
}

static int enc_update_overlay(int vin, int stream, const char* overlay_name)
{
	if(vin < HI_VENC_CH_BACKLOG_REF && 0 == stream){
		lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = enc_lookup_overlay_byname(vin, stream, overlay_name);
		if(NULL != overlay){
			REGION_HANDLE const region_handle = overlay->region_handle;
			REGION_CTRL_PARAM_U region_ctrl_param;
			memset(&region_ctrl_param, 0, sizeof(region_ctrl_param));
			region_ctrl_param.stBitmap.enPixelFormat = PIXEL_FORMAT_RGB_4444;
			region_ctrl_param.stBitmap.u32Width = overlay->canvas->width;
			region_ctrl_param.stBitmap.u32Height = overlay->canvas->height;
			region_ctrl_param.stBitmap.pData = overlay->canvas->pixels;
			SOC_CHECK(HI_MPI_VPP_ControlRegion(region_handle, REGION_SET_BITMAP, &region_ctrl_param));
			return SDK_SUCCESS;
		}
	}
	return SDK_FAILURE;
}

static stSDK_ENC_HI3515 _sdk_enc =
{
	// init the interfaces
	.api = {
		.lookup_stream_byname = enc_lookup_stream_byname,
		.create_h264_stream = enc_create_h264_stream,
		.release_h264_stream = enc_release_h264_stream,
		.start_h264_stream = enc_start_h264_stream,
		.stop_h264_stream = enc_stop_h264_stream,
		.request_h264_keyframe = enc_request_h264_keyframe,
		.create_g711a_stream = enc_create_g711a_stream,
		.release_g711a_stream = enc_release_g711a_stream,
		.start = enc_start,
		.stop = enc_stop,
		.snapshot = enc_snapshot,
		// overlay
		.create_overlay_canvas = enc_create_overlay_canvas,
		.release_overlay_canvas = enc_release_overlay_canvas,
		.create_overlay = enc_create_overlay,
		.release_overlay = enc_release_overlay,
		.get_overlay_canvas = enc_get_overlay_canvas,
		.show_overlay = enc_show_overlay,
		.update_overlay = enc_update_overlay,
	},
};

int SDK_init_enc()
{
	int i = 0, ii = 0, iii = 0;
	// only 'sdk_enc' pointer is NULL could be init
	if(NULL == sdk_enc){
		// alloc memory
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
				lpSDK_ENC_H264_STREAM_ATTR const stream_attr = &_sdk_enc.attr.video_stream_attr[i][ii];
				uint8_t* const frame_ref_counter = &_sdk_enc.attr.frame_ref_counter[i][ii];

				stream_attr->magic = 0;
				stream_attr->start = false;
				*frame_ref_counter = 0;
			}
		}
		// init the overlay set handl
		for(i = 0; i < HI_VENC_CH_BACKLOG_REF; ++i){
			stSDK_ENC_VIDEO_OVERLAY_ATTR_SET* const overlay_set = &_sdk_enc.attr.video_overlay_set[i];
			for(iii = 0; iii < HI_VENC_OVERLAY_BACKLOG_REF; ++iii){
				lpSDK_ENC_VIDEO_OVERLAY_ATTR const overlay = &overlay_set->attr[iii];

				overlay->canvas = NULL;
				memset(overlay->name, 0, sizeof(overlay->name));
				overlay->x = 0;
				overlay->y = 0;
				overlay->width = 0;
				overlay->height = 0;
				// very important, pre-alloc the handle number
				overlay->region_handle = -1;
			}
		}
		
		// init the snapshot mutex
		pthread_mutex_init(&_sdk_enc.attr.snapshot_mutex, NULL);
		// success to init
		return SDK_SUCCESS;
	}
	return SDK_FAILURE;
}

int SDK_destroy_enc()
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
			sdk_enc->release_g711a_stream(i);
		}
		// release the video encode
		for(i = 0; i < HI_VENC_CH_BACKLOG_REF; ++i){
			for(ii = HI_VENC_STREAM_BACKLOG_REF - 1; ii >= 0; --ii){
				// destroy sub stream firstly
				sdk_enc->release_h264_stream(i, ii);
			}
		}
		// clear handler pointer
		sdk_enc = NULL;
		// success to destroy
		return SDK_SUCCESS;
	}
	return SDK_FAILURE;
}

