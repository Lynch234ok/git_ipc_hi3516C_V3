
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <NkUtils/log.h>
#include <OSSCloud/oss_uploader.h>
#include "app_cloud_rec.h"
#include "mutex_f.h"
#include "netsdk.h"
#include "netsdk_private.h"
#include "media_buf.h"
#include "sdk/sdk_enc.h"
#include "base/ja_jobs.h"

//#define NK_CLOUD_REC_COPY_FRAME          (1)
#define NK_CLOUD_REC_STOP_BY_TIME        (1)
#define NK_CLOUD_REC_MAX_DURATION        (3*60*1000*1000)  //us
#define NK_CLOUD_REC_MAX_FRAME_INTERVAL  (2500*1000)  //us

typedef struct NK_CLOUD_REC_MEDIA_INFO
{
	lpMEDIABUF_USER mediabuf_user;
	int chn;
	int stream;
	int rec_type;					//if -1, mean is going to stop
	int is_start;
	unsigned int frame_cnt;
	unsigned long long start_us;
	unsigned long long lastFrame_us;
	unsigned long long duration;
	unsigned long long trigger_pts;
#ifdef NK_CLOUD_REC_COPY_FRAME
	void *streamData;
	unsigned int streamSize;
#else
	int streamIsLock;
#endif
	void *picData;
	unsigned int picSize;
} stNK_CLOUD_REC_MEDIA_INFO, *lpNK_CLOUD_REC_MEDIA_INFO;

typedef struct NK_CLOUD_REC_CON
{
	int inited;
	int max_chn;
	char cloud_id[64];
	LP_MUTEX trigger_lock;
	lpJA_JOB_MGNT ctrl_job;
	lpNK_CLOUD_REC_MEDIA_INFO media_info;
	ST_NSDK_NETWORK_OSSCLD osscld_info;
}stNK_CLOUD_REC_CON, *lpNK_CLOUD_REC_CON;

stNK_CLOUD_REC_CON cloud_rec_ctx = {0, };

static unsigned long long _nkCloudRecGetTimeUS()
{
    unsigned long long ts = 0;
    struct timespec sys_clk;
	
    clock_gettime(CLOCK_MONOTONIC, &sys_clk);
    ts = sys_clk.tv_sec;
    ts *= 1000000;
    ts += (sys_clk.tv_nsec / 1000);
    return ts;
}

int _nkCloudRecCtrl(void *context, JA_UINT32 data, void *user_data)
{
	int channel = (data >> 8) & 0xF, stream = data & 0xF, ctrl = (data >> 16) & 0xF;
	return NK_CLOUD_REC_Stop(channel, stream);
}

static ssize_t _nkCloudRecReadFrame( void *ctx,
                             NK_OSS_UL_PL_TYPE *pOssPlayloadType,
                             uint8_t **pData,
                             uint64_t *pFrameTsMs,
                             int32_t *pVideoFrameRate) {
#if defined(CLOUD_REC)
    int frame_size = 0;
	lpNK_CLOUD_REC_CON thiz = &cloud_rec_ctx;
	lpNK_CLOUD_REC_MEDIA_INFO media_info = ctx;
	lpSDK_ENC_BUF_ATTR attr = NULL;
	JA_UINT32 data = 0;

	int chn = -1;
	if(!thiz->inited){
		NK_Log()->error("%s fail, cloud rec is not init!", __func__);
		return -1;
	}

	if(!media_info){
		NK_Log()->error("%s fail, ctx is null!", __func__);
		return -1;
	}

	chn = media_info->chn;
	if(chn > thiz->max_chn){
		NK_Log()->error("%s fail, media_info->chn %d is invalid!", __func__, chn);
		return -1;
	}

#define __NK_CLOUD_REC_STOP(__IS_STOP_BY_TIME)\
	do{\
		if(thiz->trigger_lock && thiz->trigger_lock->lock(thiz->trigger_lock) == 0){\
			NK_Log()->info("%s ch%d stop rec!", __func__, chn);\
			thiz->media_info[chn].duration = 0;\
			thiz->media_info[chn].rec_type = -1;\
			if(__IS_STOP_BY_TIME){\
				data = thiz->media_info[chn].stream;\
				data |= thiz->media_info[chn].chn << 8;\
				JA_JOB_MGNT_push_job(thiz->ctrl_job, data);\
				usleep(20000);\
			}\
			thiz->trigger_lock->unlock(thiz->trigger_lock);\
		}\
	}while(0)

#ifdef NK_CLOUD_REC_STOP_BY_TIME
#define _NK_CLOUD_REC_STOP()  __NK_CLOUD_REC_STOP(1)
#else
#define _NK_CLOUD_REC_STOP()  __NK_CLOUD_REC_STOP(0)
#endif

	if(thiz->media_info[chn].duration > 0){
		if(thiz->media_info[chn].trigger_pts == 0){
			NK_Log()->info("%s ch%d start rec!", __func__, chn);
			//MEDIABUF_sync2(thiz->media_info[chn].mediabuf_user, 4);
			MEDIABUF_sync(thiz->media_info[chn].mediabuf_user);
			thiz->media_info[chn].start_us = _nkCloudRecGetTimeUS();
			thiz->media_info[chn].trigger_pts = thiz->media_info[chn].start_us;
		}
		thiz->media_info[chn].streamIsLock = 0;
		if(0 == MEDIABUF_out_try_lock(thiz->media_info[chn].mediabuf_user)){
			if(0 == MEDIABUF_out(thiz->media_info[chn].mediabuf_user, (NK_PVoid)&attr, NULL, &frame_size)) {
				if(attr->type == kSDK_ENC_BUF_DATA_H264){
					*pOssPlayloadType = NK_OSS_UL_PL_H264;
					//��ʼʱ����VI֡����
					if(thiz->media_info[chn].frame_cnt == 1){
						thiz->media_info[chn].trigger_pts = attr->timestamp_us;
					}
					//ֹͣ¼��
					if(thiz->media_info[chn].frame_cnt > 1 && attr->h264.keyframe
						&& thiz->media_info[chn].trigger_pts + thiz->media_info[chn].duration < attr->timestamp_us){
						//Beyond the time range
						MEDIABUF_out_unlock(thiz->media_info[chn].mediabuf_user);
						_NK_CLOUD_REC_STOP();
						NK_Log()->warn("%s ch%d new frame(%llu) is beyond the time range", __func__, chn, attr->timestamp_us / 1000000);
						return -1;
					}
					thiz->media_info[chn].lastFrame_us = _nkCloudRecGetTimeUS();
					thiz->media_info[chn].frame_cnt++;
				}
				else if(attr->type == kSDK_ENC_BUF_DATA_AAC){
					*pOssPlayloadType = NK_OSS_UL_PL_AAC;
				}
				else{
					NK_Log()->info("%s fail, know type %d!", __func__, attr->type);
					*pOssPlayloadType = NK_OSS_UL_PL_UNSUPPORT;
					frame_size = 0;
				}
			    *pVideoFrameRate = attr->h264.fps;
			    *pFrameTsMs = attr->timestamp_us/1000;
#ifdef NK_CLOUD_REC_COPY_FRAME
				if(frame_size > thiz->media_info[chn].streamSize){
					if(thiz->media_info[chn].streamData != NULL){
						free(thiz->media_info[chn].streamData);
					}
					thiz->media_info[chn].streamData = malloc(frame_size);
					thiz->media_info[chn].streamSize = frame_size;
					NK_Log()->debug("%s realloc, size %d!", __func__, frame_size);
				}
				if(thiz->media_info[chn].streamData == NULL){
					NK_Log()->error("%s realloc fail, size %d!", __func__, frame_size);
					MEDIABUF_out_unlock(thiz->media_info[chn].mediabuf_user);
					return -1;
				}
				memcpy(thiz->media_info[chn].streamData, (attr+1), frame_size);
				*pData = thiz->media_info[chn].streamData;
				MEDIABUF_out_unlock(thiz->media_info[chn].mediabuf_user);
#else
				*pData = (uint8_t *)(attr+1);
				thiz->media_info[chn].streamIsLock = 1;
#endif
				return frame_size;
			}
			MEDIABUF_out_unlock(thiz->media_info[chn].mediabuf_user);
		}
		if(thiz->media_info[chn].lastFrame_us && _nkCloudRecGetTimeUS() >= thiz->media_info[chn].lastFrame_us + NK_CLOUD_REC_MAX_FRAME_INTERVAL){
			//wait for new frame timeout
			_NK_CLOUD_REC_STOP();
			NK_Log()->warn("%s ch%d wait for new frame too long, last time is %llu", __func__, chn, thiz->media_info[chn].lastFrame_us / 1000000);
			return -1;
		}
		usleep(10 * 1000);
	}
	else{
		usleep(20 * 1000);
		return 0;
	}
#endif
	return 0;
}

ssize_t _nkCloudRecReleaseFrame(void * ctx, uint8_t *pData)
{
#ifndef NK_CLOUD_REC_COPY_FRAME
	lpNK_CLOUD_REC_CON thiz = &cloud_rec_ctx;
	lpNK_CLOUD_REC_MEDIA_INFO media_info = ctx;
	int chn = -1;
	if(!thiz->inited){
		NK_Log()->error("%s fail, cloud rec is not init!", __func__);
		return -1;
	}

	if(!media_info){
		NK_Log()->error("%s fail, ctx is null!", __func__);
		return -1;
	}

	chn = media_info->chn;
	if(chn > thiz->max_chn){
		NK_Log()->error("%s fail, media_info->chn %d is invalid!", __func__, chn);
		return -1;
	}

	if(thiz->media_info[chn].streamIsLock){
		MEDIABUF_out_unlock(thiz->media_info[chn].mediabuf_user);
	}
#endif
	return 0;
}

static ssize_t _nkCloudRecGetCoverPic(void * ctx, NK_OSS_UL_PL_TYPE *pPayloadType, uint8_t **pData)
{
#if defined(CLOUD_REC)
    lpNK_CLOUD_REC_CON thiz = &cloud_rec_ctx;
    lpNK_CLOUD_REC_MEDIA_INFO media_info = ctx;
    int chn = -1;
    char buf[102400] = {0};
    int nLen = sizeof(buf);


    if(!thiz->inited){
        NK_Log()->error("%s fail, cloud rec is not init!", __func__);
        return -1;
    }

    if(!media_info){
        NK_Log()->error("%s fail, ctx is null!", __func__);
        return -1;
    }

    chn = media_info->chn;
    if(chn > thiz->max_chn){
        NK_Log()->error("%s fail, media_info->chn %d is invalid!", __func__, chn);
        return -1;
    }

    if(thiz->media_info[chn].duration > 0){
    if(0 == sdk_enc->snapshotToBuf(0, kSDK_ENC_SNAPSHOT_QUALITY_HIGHEST, 640, 360, buf, &nLen))
    {
        *pPayloadType = NK_OSS_UL_PL_JPG;
        if(nLen > thiz->media_info[chn].picSize){
            if(thiz->media_info[chn].picData != NULL){
                free(thiz->media_info[chn].picData);
            }
            thiz->media_info[chn].picData = malloc(nLen);
            thiz->media_info[chn].picSize = nLen;
            NK_Log()->debug("%s realloc, size %d!", __func__, nLen);
        }
        if(thiz->media_info[chn].picData == NULL){
            NK_Log()->error("%s realloc fail, size %d!", __func__, nLen);
            return -1;
        }
        memcpy(thiz->media_info[chn].picData, buf, nLen);
        *pData = thiz->media_info[chn].picData;
        return nLen;
    }
        usleep(10000);
        return 0;
    }
    else{
        usleep(20000);
        return 0;
    }
#endif
    return 0;
}

static int _nkCloudRecTypeChange(tNK_CLOUD_REC_TYPE nkType)
{
	int ossType = 0;

	if(nkType & NK_CLOUD_REC_TYPE_TIME){
		ossType |= OSS_REC_TYPE_TIME;
	}
	if(nkType & NK_CLOUD_REC_TYPE_MOTION){
		ossType |= OSS_REC_TYPE_MOTION;
	}
	if(nkType & NK_CLOUD_REC_TYPE_ALARM){
		ossType |= OSS_REC_TYPE_ALARM;
	}
	if(nkType & NK_CLOUD_REC_TYPE_MANU){
		ossType |= OSS_REC_TYPE_MANU;
	}
	return ossType;
}

/*
 *  @param id 101表示主码流，102表示子码流
 */
static void _nkCloudRecEncToH264(int id)
{
    ST_NSDK_VENC_CH venc;

    NETSDK_conf_venc_ch_get(id, &venc);
    if(kNSDK_CODEC_TYPE_H264 != venc.codecType)
    {
        venc.codecType = kNSDK_CODEC_TYPE_H264;
        NETSDK_conf_venc_ch_set2(id, &venc, true);
        if(NULL != netsdk->videoEncodeChannelChanged)
        {
            NK_Log()->info("%s change to h264", __func__);
            netsdk->videoEncodeChannelChanged(id, &venc);
        }
    }

}

int NK_CLOUD_REC_Deinit()
{
#if defined(CLOUD_REC)
	lpNK_CLOUD_REC_CON thiz = &cloud_rec_ctx;
	int i = 0;

    if(!thiz->inited){
        NK_Log()->error("%s fail, cloud rec is not init!", __func__);
        return -1;
    }

	thiz->inited = 0;
	NK_OSS_UploaderDeinit();
#ifdef NK_CLOUD_REC_STOP_BY_TIME
	if(thiz->ctrl_job){
		JA_JOB_MGNT_stop2(thiz->ctrl_job, JA_TRUE, JA_TRUE);
		JA_JOB_MGNT_del(thiz->ctrl_job);
	}
#endif
	for(i = 0; i < thiz->max_chn; i++){
		if(thiz->media_info[i].mediabuf_user){
			MEDIABUF_detach(thiz->media_info[i].mediabuf_user);
			thiz->media_info[i].mediabuf_user = NULL;
		}
#ifdef NK_CLOUD_REC_COPY_FRAME
		if(thiz->media_info[i].streamData != NULL){
			free(thiz->media_info[i].streamData);
			thiz->media_info[i].streamData = NULL;
		}
#endif
		if(thiz->media_info[i].picData != NULL){
			free(thiz->media_info[i].picData);
			thiz->media_info[i].picData = NULL;
		}
	}
	if(thiz->media_info){
		free(thiz->media_info);
		thiz->media_info = NULL;
	}
	if(thiz->trigger_lock){
		MUTEX_release(thiz->trigger_lock);
		thiz->trigger_lock = NULL;
	}
#endif
    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);

	return 0;
}

int NK_CLOUD_REC_Init(char *esee_id, int id_len, int max_chn)
{
	lpNK_CLOUD_REC_CON thiz = &cloud_rec_ctx;
	NK_OSS_UploaderInitParam param;
	int ret = -1, i = 0;
	int meidabuf_id = -1;
	char stream_name[32];

#if defined(CLOUD_REC)
	do{
        if(1 == thiz->inited)
        {
            NK_Log()->info("cloud rec initialization");
            break;
        }

		if(!(esee_id && strlen(esee_id) > 0)){
			break;
		}
		
		if(id_len > sizeof(thiz->cloud_id)){
			break;
		}

		memcpy(thiz->cloud_id, esee_id, id_len);
		thiz->max_chn = max_chn;

		thiz->media_info = calloc(thiz->max_chn, sizeof(stNK_CLOUD_REC_MEDIA_INFO));
		if(!thiz->media_info){
			break;
		}
		thiz->trigger_lock = MUTEX_create();
		if(!thiz->trigger_lock){
			break;
		}
#ifdef NK_CLOUD_REC_STOP_BY_TIME
		thiz->ctrl_job = JA_JOB_MGNT_new("MATCH_CODE_JOB", thiz->max_chn * 4, 1, 1024*1024, (fJA_JOB_HANDLE)_nkCloudRecCtrl, NULL, NULL, 0);
		if(!thiz->ctrl_job){
			break;
		}
		JA_JOB_MGNT_start(thiz->ctrl_job);
#endif
	    param.device_esee_id = thiz->cloud_id;
	    param.esee_server = "sts.dvr163.com";
	    param.tz = NK_TZ_GMT;
	    param.read_frame_cb = _nkCloudRecReadFrame;
	    param.uploader_notify_cb = NK_Nil;
	    param.get_corjson_cb = NK_Nil;
	    param.get_coverpic_cb = _nkCloudRecGetCoverPic;
		param.after_read_frame_cb = _nkCloudRecReleaseFrame;

	    if (0 != NK_OSS_UploaderInit_V2(&param, thiz->max_chn)) {
			break;
	    }

		for(i = 0; i < thiz->max_chn; i++){
			snprintf(stream_name, sizeof(stream_name), "ch%01d_0.264", i);
			meidabuf_id = MEDIABUF_lookup_byname(stream_name);
			thiz->media_info[i].mediabuf_user = MEDIABUF_attach(meidabuf_id);
		}

		thiz->inited = 1;
		ret = 0;
	}
	while(0);

	if(!thiz->inited){
		NK_CLOUD_REC_Deinit();
	}
#endif

	return ret;
}

int NK_CLOUD_REC_Update(LP_NSDK_NETWORK_OSSCLD osscld)
{
	lpNK_CLOUD_REC_CON thiz = &cloud_rec_ctx;
	int ret = -1;

	do{
		if(!thiz->inited){
			NK_Log()->error("%s fail, cloud rec is not init!", __func__);
			break;
		}

		if(!osscld){
			NK_Log()->error("%s fail, osscld is null!", __func__);
			break;
		}

		memcpy(&thiz->osscld_info, osscld, sizeof(ST_NSDK_NETWORK_OSSCLD));

        /*
         * 因为云存没支持H265包，假如开通云存时当前视频编码是H265就转换为H264编码
         */
        if(true == thiz->osscld_info.channel[0].stream[0].enable)
        {
            _nkCloudRecEncToH264(101);
            _nkCloudRecEncToH264(102);
        }
		ret = 0;
	}
	while(0);
	return ret;
}

int NK_CLOUD_REC_Trigger(int chn, int rec_type, int rec_duration)
{
#if defined(CLOUD_REC)
	lpNK_CLOUD_REC_CON thiz = &cloud_rec_ctx;
	unsigned long long duration = 0;
	do{
		if(!thiz->inited){
			NK_Log()->error("%s fail, cloud rec is not init!", __func__);
			break;
		}

		if(chn >= thiz->max_chn){
			NK_Log()->error("%s fail, chn %d is exceed max chn %d!", __func__, chn, thiz->max_chn);
			break;
		}

		if(!thiz->osscld_info.isBound || !thiz->osscld_info.channel[chn].stream[0].enable){
			NK_Log()->debug("%s fail, cloud is not buy!", __func__);
			break;
		}

		if(!thiz->media_info[chn].is_start){
			NK_Log()->error("%s fail, chn %d is not start!", __func__, chn);
			break;
		}

		if(thiz->trigger_lock && thiz->trigger_lock->lock(thiz->trigger_lock) == 0){
			if(thiz->media_info[chn].rec_type != -1){
				rec_type = _nkCloudRecTypeChange(rec_type);
				if(rec_type > thiz->media_info[chn].rec_type){
					//�л�¼��
				}
				if(thiz->media_info[chn].trigger_pts == 0){
					thiz->media_info[chn].duration = rec_duration;
					thiz->media_info[chn].duration *= 1000000;
				}
				else{
					//˳��¼��
					duration = rec_duration;
					duration *= 1000000;
					duration += _nkCloudRecGetTimeUS();
					thiz->media_info[chn].duration = (duration - thiz->media_info[chn].start_us);
					if(thiz->media_info[chn].duration > NK_CLOUD_REC_MAX_DURATION){
						thiz->media_info[chn].duration = NK_CLOUD_REC_MAX_DURATION;
					}
				}
			}
			else{
				NK_Log()->info("NK_CLOUD_REC_Trigger fail, chn %d is going to stop!", chn);
			}
			thiz->trigger_lock->unlock(thiz->trigger_lock);
			return 0;
		}
		else{
			NK_Log()->error("NK_CLOUD_REC_Trigger fail, lock is not inited!");
		}
	}
	while(0);
#endif

	return -1;
}

int NK_CLOUD_REC_Stop(int chn, int stream)
{
#if defined(CLOUD_REC)
	lpNK_CLOUD_REC_CON thiz = &cloud_rec_ctx;
	if(!thiz->inited){
		NK_Log()->error("%s fail, cloud rec is not init!", __func__);
		return -1;
	}
	if(NK_OSS_StopUpload(chn, stream) != 0){
		NK_Log()->error("NK_CLOUD_REC_Stop fail, ch%d NK_OSS_StopUpload fail!", chn);
		return -1;
	}
#ifdef NK_CLOUD_REC_COPY_FRAME
	if(thiz->media_info[chn].streamData != NULL){
		free(thiz->media_info[chn].streamData);
		thiz->media_info[chn].streamData = NULL;
	}
#endif
	if(thiz->media_info[chn].picData != NULL){
		free(thiz->media_info[chn].picData);
		thiz->media_info[chn].picData = NULL;
	}
	thiz->media_info[chn].frame_cnt = 0;
	thiz->media_info[chn].trigger_pts = 0;
	thiz->media_info[chn].lastFrame_us = 0;
	thiz->media_info[chn].start_us = 0;
	thiz->media_info[chn].rec_type = 0;
	thiz->media_info[chn].is_start = 0;
	NK_Log()->info("NK_CLOUD_REC_Stop ch%d succeed!", chn);
#endif
	return 0;
}

int NK_CLOUD_REC_Start(int chn, int stream, int rec_type)
{
	lpNK_CLOUD_REC_CON thiz = &cloud_rec_ctx;
	int ret = -1;
#if defined(CLOUD_REC)
	do{
		if(!thiz->inited){
			NK_Log()->error("%s fail, cloud rec is not init!", __func__);
			break;
		}

		if(chn >= thiz->max_chn || stream > 2){
			NK_Log()->error("%s fail, chn %d is exceed max chn %d!", __func__, chn, thiz->max_chn);
			break;
		}

		if(!thiz->osscld_info.isBound || !thiz->osscld_info.channel[chn].stream[stream].enable){
			NK_Log()->debug("%s fail, cloud is not buy!", __func__);
			break;
		}

		if(thiz->media_info[chn].is_start){
			return 0;
		}

#ifdef NK_CLOUD_REC_COPY_FRAME
		thiz->media_info[chn].streamSize = 10240;
		if((thiz->media_info[chn].streamData = malloc(thiz->media_info[chn].streamSize)) == NULL){
			NK_Log()->error("%s malloc fail!", __func__);
			break;
		}
#endif
		thiz->media_info[chn].picSize = 5120;
		if((thiz->media_info[chn].picData = malloc(thiz->media_info[chn].picSize)) == NULL){
			NK_Log()->error("%s malloc picture fail!", __func__);
			break;
		}

		thiz->media_info[chn].chn = chn;
		thiz->media_info[chn].stream = stream;
		rec_type = _nkCloudRecTypeChange(rec_type);
		thiz->media_info[chn].rec_type = rec_type;
		if(0 != NK_OSS_StartUpload((void*)&thiz->media_info[chn], chn, stream, rec_type)){
			NK_Log()->error("NK_CLOUD_REC_Start fail, ch%d NK_OSS_StartUpload fail!", chn);
			break;
		}
		thiz->media_info[chn].is_start = 1;
		NK_Log()->info("NK_CLOUD_REC_Start ch%d succeed!", chn);
		ret = 0;
	}
	while(0);
#endif

	return ret;
}

