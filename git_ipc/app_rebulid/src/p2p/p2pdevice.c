#include "p2pdevice.h"
#ifdef DANA_P2P
#include "danasdk.h"
#else
#include "p2psdk.h"
#endif

#include "p2p_json_parse.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include  <unistd.h>
#include <secure_chip.h>
//#include "stdinc.h"
#include "media_buf.h"
#include "soup_base.h"

#include "netsdk.h"

#include "app_debug.h"
#include "custom.h"
#include "tfcard.h"
#include "fisheye.h"
#include "sound_queue.h"
#include "sound.h"
#include "app_gsensor.h"
#ifdef VIDEO_CTRL
#include "app_video_ctrl.h"
#endif
#include "model_conf.h"
#include "../tfcard/tfcard_play.h"
#include "global_runtime.h"
#include "../global_runtime.h"
#include "netsdk.h"
#include <sys/prctl.h>
#include "app_upgrade.h"
#include "production_test.h"
#include "base/ja_process.h"
#include "ptz.h"
#include "generic.h"

#if defined(TS_RECORD)
#include "tfcard/include/NK_Tfcard.h"
#endif

#define msleep(x) usleep(x*1000)
#define BUFFER_MAX_SIZE 	(320 * 2)
#define BUFFER_SIZE		(320)

#define P2P_STRUCT_MEMBER_POS(t,m)  ((unsigned long)(&(((t *)(0))->m)))
#define P2P_REF_STREAM_NUM	(64)

#define PACKET_MAX_LENGHT (1024 * 10)

/**
 * P2Pè¿œç¨‹å›žæ”¾å¸§ç¼“å†²
 */
#if defined(HI3518E_V2)
#define P2P_REC_FRAME_BUF_SIZE	(384 * 1024)
#elif defined(HI3516D)
#define P2P_REC_FRAME_BUF_SIZE	(2048 *1024)
#else
#define P2P_REC_FRAME_BUF_SIZE	(512 * 1024)
#endif

#if defined(TS_RECORD)
static char p2pRecFrameBuf[P2P_REC_FRAME_BUF_SIZE];
#endif

#ifdef GSENSOR
#define GSENSOR_INT_FRAME_COUNT     15  // GSENSOR_INT_FRAME_COUNTå¸§è§†é¢‘*66ms(å¸§çŽ‡15)
#endif

enum{
	P2P_BYPASS_FRAME_TYPE_FISHEYE_PARAM = 0,
	P2P_BYPASS_FRAME_TYPE_LENS_PARAM,
	P2P_BYPASS_FRAME_TYPE_GSENSOR_PARAM,
	P2P_BYPASS_FRAME_TYPE_CNT,
};

enum{
	eP2P_IMAGE_FISHEYE_FIX_MODE_WALL=0,
	eP2P_IMAGE_FISHEYE_FIX_MODE_CEIL,
	eP2P_IMAGE_FISHEYE_FIX_MODE_TABLE,
	eP2P_IMAGE_FISHEYE_FIX_MODE_NONE,
};

typedef struct fisheyeParam
{
	struct
    {
        int CenterCoordinateX;
        int CenterCoordinateY;
        int Radius;
        int AngleX;
        int AngleY;
        int AngleZ;
    }param[2];
	int fixMode;
	int version;
	int Reverse;
}ST_FISHEYE_PARAM, *LP_FISHEYE_PARAM;

typedef struct fisheyeParam2
{
	struct
    {
        float CenterCoordinateX;
        float CenterCoordinateY;
        float Radius;
        float AngleX;
        float AngleY;
        float AngleZ;
    }param[2];
	int fixMode;
	int version;
	int Reverse;
}ST_FISHEYE_PARAM2, *LP_FISHEYE_PARAM2;

typedef struct byPassFrame{
	int frameType;
	int dataSize;
	void *data;
}ST_BYPASS_FRAME, *LP_BYPASS_FRAME;

static NK_PVoid g_tfPlayHistoryHandle = NULL;

static inline uint64_t _get_time_ms()
{
	uint64_t time_ms = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_ms = tv.tv_sec;
	time_ms*=1000;
	time_ms += tv.tv_usec/1000;
	return time_ms;
}

int OnAuth(void *ctx, const char *usr, const char *pwd)
{
    printf("Device Auth<%s:%s>\n", usr, pwd);
	
	if(usr || (0 != strlen(usr))){
		if(USRM_GREAT == USRM_check_user(usr, pwd)){
			return 0;
		}
	}
    return -1;
}

static ssize_t OnGetNonce(void *ctx, char *nonce_buff, size_t buff_size)
{
	return 0;

}

static int OnAuth2(void *ctx, const char *auth_info, const char *signature)
{
	return 0;

}

int OnPtzCtrl(void *ctx, uint32_t channel, uint32_t action, uint32_t param1, uint32_t param2)
{
    //printf("PTZ Ctrl:channel%d|%d param1=%d param2=%d\n", channel, action, param1, param2);
    int nChn = 0;
	int speed_flag = NK_FALSE;
    PTZ_COMMAND emCom = PTZ_CMD_NULL;
    unsigned char sendParam = 1;

    if(NULL == ctx)
    {
        return -1;
    }

    if(KP2P_PTZ_CONTROL_ACTION_UP == action)
    {
        emCom = PTZ_CMD_UP;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_DOWN == action)
    {
        emCom = PTZ_CMD_DOWN;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_LEFT == action)
    {
        emCom = PTZ_CMD_LEFT;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_RIGHT == action)
    {
        emCom = PTZ_CMD_RIGHT;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_STOP == action)
    {
        emCom = PTZ_CMD_STOP;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_AUTO == action)
    {
        emCom = PTZ_CMD_AUTOPAN;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_ZOOM_OUT == action)
    {
        emCom = PTZ_CMD_ZOOM_OUT;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_ZOOM_IN == action)
    {
        emCom = PTZ_CMD_ZOOM_IN;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_FOCUS_F == action)
    {
        emCom = PTZ_CMD_FOCUS_FAR;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_FOCUS_N == action)
    {
        emCom = PTZ_CMD_FOCUS_NEAR;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_IRIS_CLOSE == action)
    {
        emCom = PTZ_CMD_IRIS_CLOSE;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_IRIS_OPEN == action)
    {
        emCom = PTZ_CMD_IRIS_OPEN;
		speed_flag = NK_TRUE;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_PRESET_SET == action)
    {
        emCom = PTZ_CMD_SET_PRESET;
        sendParam = param1;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_PRESET_GO == action)
    {
        emCom = PTZ_CMD_GOTO_PRESET;
        sendParam = param1;
    }
    else if(KP2P_PTZ_CONTROL_ACTION_PRESET_CLEAR == action)
    {
        emCom = PTZ_CMD_CLEAR_PRESET;
        sendParam = param1;
    }

#if defined(UART_PROTOCOL)
	if(NK_TRUE == speed_flag){
		//param1å‚æ•°ä¸ºäº‘å°é€Ÿåº¦æ—¶ï¼Œéœ€è¦è¿›è¡Œä¿å­˜ä¸Žè½¬æ¢
		ST_NSDK_PTZ_CFG stPtzConfig;
		memset(&stPtzConfig, 0, sizeof(stPtzConfig));
		NETSDK_conf_ptz_ch_get(&stPtzConfig);
		
		if(param1 > 8){
			param1 = 8;
		}else if(param1 < 1){
			param1 = 1;
		}
		
		stPtzConfig.stPtzExternalConfig.nSpeed = param1;
		NETSDK_conf_ptz_ch_set(&stPtzConfig);
		
		sendParam = ptz_speed_level_switch(param1);
	}
	
    PTZ_Send(nChn, emCom, sendParam);
#endif

    return 0;

}

struct StreamCtx{
	bool isOldTypeRec;
    int mediabuf_id;
    lpMEDIABUF_USER mediabuf_user;
	int chn;
	int streamNo;
	bool bypass_frame_flag;
	struct P2PDeviceDemo *pDevice;
	char byPassFrameBuf[1024];
	void *recFrameBuf;
	size_t recFrameBufSize;
	void *play_rec_search_ctx;
    uint32_t rec_frame_seq;
#ifdef GSENSOR
    int gsensor_frame_flag;
    unsigned int gsensor_int_frame_count;
    uint64_t gsensor_ts_ms;
#endif
#if defined(TS_RECORD)
    unsigned int rec_startTime;
#endif
};

void* OnAttachStream(void *ctx, uint32_t channel, uint32_t stream)
{
    if (!ctx) {
        printf("DeviceAttachStream  failed:nil hooks_ctx\n");
        return NULL;
    }
	int ch_num = channel;
	int stream_num = stream;
    	
    //limited the client use ch0_0.264
    struct P2PDeviceDemo *pDemo = (struct P2PDeviceDemo*)ctx;
    
	struct StreamCtx *pCtx = (struct StreamCtx*)calloc(1,sizeof(struct StreamCtx));    

	if (!pCtx) {
		printf("DeviceAttachStream failed: create StreamCtx\n");
        return NULL;
    }

	char stream_name[64] = {0};
	int stream_num_ex = 0;
	printf("AttachStream:%d-%d\n", ch_num, stream_num);
	stream_num_ex = stream_num;
	if(P2P_REF_STREAM_NUM == stream_num){
		stream_num = 0;
	}
    snprintf(stream_name, sizeof(stream_name), "ch%d_%d.264", ch_num, stream_num);

	printf("Force AttachStream: %s\n", stream_name);
    pCtx->mediabuf_id = MEDIABUF_lookup_byname(stream_name);
    if (pCtx->mediabuf_id < 0) {
        printf("DeviceAttachStream failed:lookup stream %s\n", stream_name);
        free(pCtx);
		pCtx = NULL;
        return NULL;
    }
    pCtx->mediabuf_user = MEDIABUF_attach(pCtx->mediabuf_id);

    if(NULL == pCtx->mediabuf_user) {
        printf("DeviceAttachStream: mediabuf full!\n");
        free(pCtx);
		pCtx = NULL;
        return NULL;
    }

	pCtx->bypass_frame_flag = true;
	pCtx->pDevice = pDemo;
	pCtx->chn = ch_num;
	pCtx->streamNo = stream_num_ex;
#ifdef GSENSOR
    pCtx->gsensor_frame_flag = false;
    pCtx->gsensor_int_frame_count = 0;
    pCtx->gsensor_ts_ms = 0;
#endif
	GLOBAL_enter_live();

    return pCtx;
}

static int recover_bps(void * stream_ctx)
{
#if !defined(ADT)        // ç›®å‰åªç”¨äºŽadityaé¡¹ç›®
    return 0;
#endif

	struct StreamCtx *pCtx = (struct StreamCtx*)stream_ctx;
	ST_NSDK_VENC_CH venc_ch;
	int vin = pCtx->chn;
	int stream = pCtx->streamNo;
	int *pts_bps = NULL;
	ST_SDK_ENC_STREAM_ATTR stream_attr;

	memset(&stream_attr,0,sizeof(ST_SDK_ENC_STREAM_ATTR));
	memset(&venc_ch, 0, sizeof(ST_NSDK_VENC_CH));
	
	NETSDK_conf_venc_ch_get((vin+1)*100+stream+1, &venc_ch);
	
	SDK_ENC_get_stream(vin, stream, &stream_attr);
	switch(stream_attr.enType){
		default:
		case kSDK_ENC_BUF_DATA_H264:
			pts_bps = &stream_attr.H264_attr.bps;
			break;
		case kSDK_ENC_BUF_DATA_H265:
			pts_bps = &stream_attr.H265_attr.bps;
			break;
	}
	*pts_bps = venc_ch.constantBitRate;
	if(*pts_bps > 128){
		SDK_ENC_set_stream(vin, stream, &stream_attr);
	}
	return 0;
}

int OnDetachStream(void *stream_ctx)
{
    struct StreamCtx *pCtx = (struct StreamCtx*)stream_ctx;
	
	if (pCtx && pCtx->mediabuf_user) {
	printf("file=%s, func=%s\n", __FILE__, __FUNCTION__);
		MEDIABUF_detach(pCtx->mediabuf_user);
		pCtx->mediabuf_user = NULL;

		recover_bps(stream_ctx);
		if(pCtx)
		{
			free(pCtx);
			pCtx = NULL;
		}
		GLOBAL_leave_live();
        return 0;
    } else {
        return -1;
    }
}

void fill_frame_head(unsigned int headType, kp2p_frame_head_t *pFrameHead, const stSDK_ENC_BUF_ATTR *attr)
{
    pFrameHead->magic = P2PSDK_FRAMEHEAD_MAGIC;
    pFrameHead->magic2 = P2PSDK_FRAMEHEAD_MAGIC2;
    pFrameHead->frame_seq = 0;  // KP2Påªå‰©ä¸‹å¼•ç”¨æ–¹å¼ï¼ŒåŽé¢è¿™éƒ¨åˆ†ä»£ç ä¼šåˆ é™¤
    pFrameHead->version = P2PSDK_FRAMEHEAD_VERSION;
	pFrameHead->headtype = headType;
	pFrameHead->framesize = attr->data_sz;
	pFrameHead->live.channel = attr->p2pFrameHead.live.channel;
	if (headType == P2P_FRAMEHEAD_HEADTYPE_REPLAY)
	{
		if(attr->type == SDK_ENC_BUF_DATA_PCM || attr->type == SDK_ENC_BUF_DATA_G711A || attr->type == SDK_ENC_BUF_DATA_G711U || attr->type == SDK_ENC_BUF_DATA_AAC){
			pFrameHead->live.frametype = P2P_FRAMEHEAD_FRAMETYPE_AUDIO;
	    }
		else if(((attr->type == SDK_ENC_BUF_DATA_H264)&&(attr->h264.keyframe == 0)) || ((attr->type == SDK_ENC_BUF_DATA_H265)&&(attr->h265.keyframe == 0))){
	        pFrameHead->live.frametype = P2P_FRAMEHEAD_FRAMETYPE_PFRAME;
	    }
		else
		{
			 pFrameHead->live.frametype = P2P_FRAMEHEAD_FRAMETYPE_IFRAME;
		}
	}else{
		if( attr->type == SDK_ENC_BUF_DATA_PCM || attr->type == SDK_ENC_BUF_DATA_G711A || attr->type == SDK_ENC_BUF_DATA_G711U || attr->type == SDK_ENC_BUF_DATA_AAC){
	        pFrameHead->live.frametype = P2P_FRAMEHEAD_FRAMETYPE_AUDIO;
	    }else if((0 != attr->h264.keyframe) || (0 != attr->h265.keyframe)){
	        pFrameHead->live.frametype = P2P_FRAMEHEAD_FRAMETYPE_IFRAME;
	    }else{
	        pFrameHead->live.frametype = P2P_FRAMEHEAD_FRAMETYPE_PFRAME;
	    }
	}

	if(headType == P2P_FRAMEHEAD_HEADTYPE_REPLAY)
	{
    	pFrameHead->ts_ms = attr->timestamp_us;
	}
	else
	{
		//fix me:need to get from media buffer
		if(pFrameHead->live.frametype == P2P_FRAMEHEAD_FRAMETYPE_AUDIO){
			//AUDIO
			pFrameHead->ts_ms = attr->timestamp_us/1000;
		}else{
			//VIDEO
			pFrameHead->ts_ms = _get_time_ms();//attr->time_us / 1000;
		}
	}
	
    if(P2P_FRAMEHEAD_FRAMETYPE_IFRAME == pFrameHead->live.frametype || P2P_FRAMEHEAD_FRAMETYPE_PFRAME == pFrameHead->live.frametype){
		if(attr->type == SDK_ENC_BUF_DATA_H264){
			pFrameHead->live.v.width = attr->h264.width;
	        pFrameHead->live.v.height = attr->h264.height;
			snprintf(pFrameHead->live.v.enc, sizeof(pFrameHead->live.v.enc), "%s", "H264");
	        pFrameHead->live.v.fps = attr->h264.fps;
		}else{//if(attr->type == SDK_ENC_BUF_DATA_H265){
			pFrameHead->live.v.width = attr->h265.width;
	        pFrameHead->live.v.height = attr->h265.height;
	        snprintf(pFrameHead->live.v.enc, sizeof(pFrameHead->live.v.enc), "%s", "H265");
	        pFrameHead->live.v.fps = attr->h265.fps;
		}
    }else{
        pFrameHead->live.a.samplerate = attr->g711a.sample_rate;
        pFrameHead->live.a.samplewidth = attr->g711a.sample_width;
        pFrameHead->live.a.channels = 1;
        pFrameHead->live.a.compress = attr->g711a.compression_ratio;
		if(attr->type == SDK_ENC_BUF_DATA_G711A){
			snprintf(pFrameHead->live.a.enc, sizeof(pFrameHead->live.a.enc), "%s", "G711A");
		}else if(attr->type == SDK_ENC_BUF_DATA_AAC){
			snprintf(pFrameHead->live.a.enc, sizeof(pFrameHead->live.a.enc), "%s", "AAC");
		}
    }
    
    //sth else for record stream
    if (headType == P2P_FRAMEHEAD_HEADTYPE_REPLAY) {
        //pFrameHead->rectype = 8;
        //pFrameHead->recchn = 0;
    }
}

int setBypassFrame(kp2p_frame_head_t *pFrameHead, void *raw_frame)
{
	ST_BYPASS_FRAME byPassFrame;
	ST_FISHEYE_PARAM fisheye;
	stFISHEYE_config fisheye_config;

	pFrameHead->magic = P2PSDK_FRAMEHEAD_MAGIC;
    pFrameHead->magic2 = P2PSDK_FRAMEHEAD_MAGIC2;
    pFrameHead->frame_seq = 0; // OOBå¸§ä¸éœ€è¦å¸§åºå·
    pFrameHead->version = P2PSDK_FRAMEHEAD_VERSION;
	pFrameHead->headtype = P2P_FRAMEHEAD_HEADTYPE_LIVE;
	pFrameHead->live.frametype = P2P_FRAMEHEAD_FRAMETYPE_OOB;

	//for fisheye correction
	pFrameHead->framesize = sizeof(ST_FISHEYE_PARAM) + sizeof(ST_BYPASS_FRAME) - sizeof(void *);
	byPassFrame.frameType = P2P_BYPASS_FRAME_TYPE_FISHEYE_PARAM;
	byPassFrame.dataSize = sizeof(ST_FISHEYE_PARAM);

    /* ¸ù¾Ýproduct typeÈ·¶¨ÊÇ·ñÐèÒªÉèÖÃfisheye²ÎÊý */
    /* Ã»ÓÐ¶Áµ½product type£¬ÔòÉèÖÃfisheye²ÎÊý */
    FISHEYE_config_get(&fisheye_config);
	memcpy(fisheye.param, fisheye_config.param, sizeof(fisheye_config.param));

	fisheye.fixMode = FISHEYE_get_fix_mode();
	fisheye.version = 0x01000003;//1.0.0.3
	fisheye.Reverse = 0;

	//copy to p2p buffer
	memcpy(raw_frame, &byPassFrame, P2P_STRUCT_MEMBER_POS(ST_BYPASS_FRAME, data));
	memcpy(raw_frame + P2P_STRUCT_MEMBER_POS(ST_BYPASS_FRAME, data), &fisheye, sizeof(ST_FISHEYE_PARAM));

	return 0;
}

int setBypassFrameEx(void *raw_frame)
{
	ST_BYPASS_FRAME byPassFrame;
	ST_FISHEYE_PARAM fisheye;
	stFISHEYE_config fisheye_config;
	kp2p_frame_head_t pFrameHead;

	pFrameHead.magic = P2PSDK_FRAMEHEAD_MAGIC;
    pFrameHead.magic2 = P2PSDK_FRAMEHEAD_MAGIC2;
    pFrameHead.frame_seq = 0; // OOBå¸§ä¸éœ€è¦å¸§åºå·
	pFrameHead.version = P2PSDK_FRAMEHEAD_VERSION;
	pFrameHead.live.frametype = P2P_FRAMEHEAD_FRAMETYPE_OOB;//OOB
	pFrameHead.live.channel = 0;
	pFrameHead.ts_ms = 0;
	pFrameHead.headtype = P2P_FRAMEHEAD_HEADTYPE_LIVE;

	//for fisheye correction
	pFrameHead.framesize = sizeof(ST_FISHEYE_PARAM) + sizeof(ST_BYPASS_FRAME) - sizeof(void *);
	byPassFrame.frameType = P2P_BYPASS_FRAME_TYPE_FISHEYE_PARAM;
	byPassFrame.dataSize = sizeof(ST_FISHEYE_PARAM);
	FISHEYE_config_get(&fisheye_config);
	memcpy(fisheye.param, fisheye_config.param, sizeof(fisheye_config.param));

	fisheye.fixMode = FISHEYE_get_fix_mode();
	fisheye.version = 0x01000003;//1.0.0.3
	fisheye.Reverse = 0;

	//copy to p2p buffer
	memcpy(raw_frame, &pFrameHead, sizeof(kp2p_frame_head_t));
	memcpy(raw_frame+sizeof(kp2p_frame_head_t), &byPassFrame, P2P_STRUCT_MEMBER_POS(ST_BYPASS_FRAME, data));
	memcpy(raw_frame+sizeof(kp2p_frame_head_t) + P2P_STRUCT_MEMBER_POS(ST_BYPASS_FRAME, data), &fisheye, sizeof(ST_FISHEYE_PARAM));

	return pFrameHead.framesize + sizeof(kp2p_frame_head_t);
}

/* Ö÷Òª´¦ÀíPx-720µÄÔ²ÐÄÐ£Õý²ÎÊýºÍ¾µÍ·»û±ä²ÎÊý */
int setBypassFrame2Ex(void *raw_frame)
{
	ST_BYPASS_FRAME byPassFrame;
	ST_FISHEYE_PARAM2 fisheye;
	stFISHEYE_config fisheye_config;
	kp2p_frame_head_t pFrameHead;
	LP_FISHEYE_LENS_PARAM fisheye_lens_param = NULL;
	int lens_param_len = 0;

	pFrameHead.magic = P2PSDK_FRAMEHEAD_MAGIC;
    pFrameHead.magic2 = P2PSDK_FRAMEHEAD_MAGIC2;
    pFrameHead.frame_seq = 0; // OOBå¸§ä¸éœ€è¦å¸§åºå·
	pFrameHead.version = P2PSDK_FRAMEHEAD_VERSION;
	pFrameHead.live.frametype = P2P_FRAMEHEAD_FRAMETYPE_OOB;//OOB
	pFrameHead.live.channel = 0;
	pFrameHead.ts_ms = 0;
	pFrameHead.headtype = P2P_FRAMEHEAD_HEADTYPE_LIVE;

	//for fisheye correction
	byPassFrame.frameType = P2P_BYPASS_FRAME_TYPE_LENS_PARAM;
	lens_param_len = FISHEYE_lens_param_len_get();
	FISHEYE_config_get(&fisheye_config);
	memcpy(fisheye.param, fisheye_config.param2, sizeof(fisheye_config.param2));

	fisheye.fixMode = FISHEYE_get_fix_mode();
	fisheye.version = 0x01000003;//1.0.0.3
	fisheye.Reverse = 0;

	fisheye_lens_param = FISHEYE_lens_param_get();

	if(fisheye_lens_param != NULL) {
		byPassFrame.dataSize = sizeof(ST_FISHEYE_PARAM2) + lens_param_len * sizeof(ST_FISHEYE_LENS_PARAM);
		pFrameHead.framesize = byPassFrame.dataSize + sizeof(ST_BYPASS_FRAME) - sizeof(void *);
	}
	else {
		byPassFrame.dataSize = sizeof(ST_FISHEYE_PARAM2);
		pFrameHead.framesize = byPassFrame.dataSize + sizeof(ST_BYPASS_FRAME) - sizeof(void *);
	}
	//copy to p2p buffer
	memcpy(raw_frame, &pFrameHead, sizeof(kp2p_frame_head_t));
	memcpy(raw_frame + sizeof(kp2p_frame_head_t), &byPassFrame, P2P_STRUCT_MEMBER_POS(ST_BYPASS_FRAME, data));
	memcpy(raw_frame + sizeof(kp2p_frame_head_t) + P2P_STRUCT_MEMBER_POS(ST_BYPASS_FRAME, data), &fisheye, sizeof(ST_FISHEYE_PARAM2));
	if(fisheye_lens_param != NULL) {
		memcpy(raw_frame + sizeof(kp2p_frame_head_t) + P2P_STRUCT_MEMBER_POS(ST_BYPASS_FRAME, data) + sizeof(ST_FISHEYE_PARAM2), fisheye_lens_param, lens_param_len * sizeof(ST_FISHEYE_LENS_PARAM));
	}

    return pFrameHead.framesize + sizeof(kp2p_frame_head_t);
}

int setGsensorFrame(void *raw_frame, uint64_t ts_ms)
{
#ifdef GSENSOR
    ST_GSENSOR_FRAME gsensorFrame;
    ST_GSENSOR_ANGLES gsensorAngles;
    stGsensor_angles angles;
    kp2p_frame_head_t pFrameHead;

    pFrameHead.magic = P2PSDK_FRAMEHEAD_MAGIC;
    pFrameHead.magic2 = P2PSDK_FRAMEHEAD_MAGIC2;
    pFrameHead.frame_seq = 0; // OOBå¸§ä¸éœ€è¦å¸§åºå·
    pFrameHead.version = P2PSDK_FRAMEHEAD_VERSION;
    pFrameHead.live.frametype = P2P_FRAMEHEAD_FRAMETYPE_OOB;//OOB
    pFrameHead.live.channel = 0;
    pFrameHead.ts_ms = ts_ms;
    pFrameHead.headtype = P2P_FRAMEHEAD_HEADTYPE_LIVE;

    //for gsensor correction
    if(0 != APP_GSENSOR_get_angles_frame(raw_frame + sizeof(kp2p_frame_head_t), &pFrameHead.framesize)) {
        return -1;
    }

    //copy to p2p buffer
    memcpy(raw_frame, &pFrameHead, sizeof(kp2p_frame_head_t));

    return pFrameHead.framesize + sizeof(kp2p_frame_head_t);
#endif
    return 0;
}

int getRefFrame(int stream_num, lpMEDIABUF_USER mediabuf_user, kp2p_frame_head_t *pFrameHead, void *raw_frame, size_t raw_frame_max_sz)
{
	size_t out_size = 0;
	size_t	data_sz = 0;
	const stSDK_ENC_BUF_ATTR *attr = NULL;
	if(0 == MEDIABUF_out(mediabuf_user, (void**)&attr, NULL, &out_size)) {
		if(out_size > raw_frame_max_sz){
			printf("thiz->m_snd_buf_sz too small:%d-%d\n", out_size, raw_frame_max_sz);
			return -1;
		}
		if(P2P_REF_STREAM_NUM == stream_num && 1 == attr->h264.ref_counter){
			data_sz = getRefFrame(stream_num, mediabuf_user, pFrameHead, raw_frame, raw_frame_max_sz);
		}else{
			const void* raw_ptr = (void*)(attr + 1);
			data_sz = attr->data_sz;
			memcpy(raw_frame, raw_ptr, data_sz);
			/*here pack the soup head*/
			fill_frame_head(P2P_FRAMEHEAD_HEADTYPE_LIVE, pFrameHead, attr);
		}
		return data_sz;
	}
	else
	{	 
		//printf("mediabuf out err! %u\n", pMEDIABUF->In_speed(pCtx->mediabuf_id));
		msleep(10);
		return	0;
	}

}


int getRefFrameEx(struct StreamCtx *pCtx, lpMEDIABUF_USER mediabuf_user, void **ppRawFrame)
{
	size_t out_size = 0;
	size_t	data_sz = 0;
	const stSDK_ENC_BUF_ATTR *attr = NULL;
	if(0 == MEDIABUF_out(mediabuf_user, (void**)&attr, NULL, &out_size)) {
		if(P2P_REF_STREAM_NUM == pCtx->streamNo && 1 == attr->h264.ref_counter){
			data_sz = getRefFrameEx(pCtx, mediabuf_user, ppRawFrame);
		}else{
            data_sz = attr->data_sz + sizeof(kp2p_frame_head_t);
            *ppRawFrame = (void*)&(attr->p2pFrameHead);
#ifdef GSENSOR
            if(APP_GSENSOR_is_support()) {
                if((attr->p2pFrameHead.live.channel == 0) && ((attr->p2pFrameHead.live.frametype == 1) || (attr->p2pFrameHead.live.frametype == 2))) {
                    if(pCtx->gsensor_frame_flag == false) {
                        pCtx->gsensor_frame_flag = true;
                        pCtx->gsensor_int_frame_count = 0;
                    }
                    else {
                        pCtx->gsensor_int_frame_count++;
                        if(pCtx->gsensor_int_frame_count >= GSENSOR_INT_FRAME_COUNT) {
                            pCtx->gsensor_ts_ms = attr->p2pFrameHead.ts_ms; // èŽ·å–è¿™ä¸€æ¬¡çš„è§†é¢‘å¸§æ—¶é—´æˆ³,ä¸‹ä¸€å¸§å‘é€Gsensor oobä½¿ç”¨è¯¥æ—¶é—´æˆ³
                        }
                    }
                }
            }
#endif
		}
		return data_sz;
	}
	else
	{
		//printf("mediabuf out err! %u\n", pMEDIABUF->In_speed(pCtx->mediabuf_id));
		return	0;
	}

}


ssize_t OnReadFrame(void *stream_ctx, kp2p_frame_head_t *pFrameHead, void *raw_frame, size_t raw_frame_max_sz)
{
    struct StreamCtx *pCtx = (struct StreamCtx*)stream_ctx;
	if (!pCtx) {
		printf("DeviceReadFrame failed:nil ctx/pMEDIABUF\n");
		return -1;
	}
    lpMEDIABUF_USER  mediabuf_user = pCtx->mediabuf_user;

	//send bypass frame
	if(NK_False == GLOBAL_sn_front()){
		pCtx->bypass_frame_flag = false;
	}
	
	if(pCtx->bypass_frame_flag){
#if defined(PX_720)
		//setBypassFrame2(pFrameHead, raw_frame);
#else
		setBypassFrame(pFrameHead, raw_frame);
#endif
		pCtx->bypass_frame_flag = false;
		return pFrameHead->framesize;
	}
	int  data_sz = 0;
	if(NULL != mediabuf_user)
	{	
	    if(0 == MEDIABUF_out_try_lock(mediabuf_user)){
	        //const SDK_ENC_BUF_ATTR_t *attr = NULL;
			data_sz = getRefFrame(pCtx->streamNo, mediabuf_user, pFrameHead, raw_frame, raw_frame_max_sz);
			MEDIABUF_out_unlock(mediabuf_user);
			return data_sz;
	    }
		else
		{
			msleep(10);
			printf("DeviceReadFrame failed: mediabuf lock err, user:%p\n", mediabuf_user);
			return 0;
		}
	}
	else
	{
		msleep(10);	
		printf("DeviceReadFrame failed: mediabuf_user:null\n");
		return -1;
	}

}


ssize_t OnReadFrameEx(void *stream_ctx, void **ppRawFrame)
{
    struct StreamCtx *pCtx = (struct StreamCtx*)stream_ctx;
	if (!pCtx) {
		printf("DeviceReadFrame failed:nil ctx/pMEDIABUF\n");
		return -1;
	}
    lpMEDIABUF_USER  mediabuf_user = pCtx->mediabuf_user;

    if(NK_False == GLOBAL_sn_front()) {
        pCtx->bypass_frame_flag = false;
    }

	//send bypass frame
	if(pCtx->bypass_frame_flag){
		int ret_size = 0;
#if defined(PX_720)
		ret_size= setBypassFrame2Ex(pCtx->byPassFrameBuf);
#else
		ret_size= setBypassFrameEx(pCtx->byPassFrameBuf);
#endif

		*ppRawFrame = pCtx->byPassFrameBuf;
		pCtx->bypass_frame_flag = false;
		return ret_size;
	}
#ifdef GSENSOR
    else if(pCtx->gsensor_frame_flag && (pCtx->gsensor_int_frame_count >= GSENSOR_INT_FRAME_COUNT)) {
        int ret_size = 0;
        ret_size= setGsensorFrame(pCtx->byPassFrameBuf, pCtx->gsensor_ts_ms);
        *ppRawFrame = pCtx->byPassFrameBuf;
        pCtx->gsensor_frame_flag = false;
        pCtx->gsensor_int_frame_count = 0;
        return ret_size;
    }
#endif
	int  data_sz = 0;
	if(NULL != mediabuf_user)
	{
	    if(0 == MEDIABUF_out_try_lock(mediabuf_user)){
	        //const SDK_ENC_BUF_ATTR_t *attr = NULL;
			data_sz = getRefFrameEx(pCtx, mediabuf_user, ppRawFrame);
			//MEDIABUF_out_unlock(mediabuf_user);
			return data_sz;
	    }
		else
		{
			printf("DeviceReadFrame failed: mediabuf lock err, user:%p\n", mediabuf_user);
			return 0;
		}
	}
	else
	{
		printf("DeviceReadFrame failed: mediabuf_user:null\n");
		return -1;
	}

}

int OnAfterReadFrame(void *live_ctx, void *frame)
{
	struct StreamCtx *pCtx = (struct StreamCtx*)live_ctx;
	if (!pCtx && NULL != pCtx->mediabuf_user) {
		printf("DeviceReadFrame failed:nil ctx/pMEDIABUF\n");
		return -1;
	}
	MEDIABUF_out_unlock(pCtx->mediabuf_user);

	return 0;
}


int OnDevOnline(void *ctx, const char *id)
{
	if (!ctx || !id) {
		printf("OnDeviceOnline err:nil hooks_ctx/id\n");
		return -1;
	}
	struct P2PDeviceDemo* pDemo = (struct P2PDeviceDemo*)ctx;
	snprintf(pDemo->eseeId, sizeof(pDemo->eseeId), "%s", id);
	if (pDemo->OnDevOnline)
		pDemo->OnDevOnline (id, pDemo->ctx);
	return 0;
}

int OnFetchRecList(void *ctx, uint32_t chn_cnt, const uint8_t *chns, uint8_t type, time_t begin_time, time_t end_time, uint8_t quality, RecList *lists)
{
#if defined(TFCARD)
	stTFCARD_History_List historyList[100] = {0};
	NK_Int historyCnt = sizeof(historyList) / sizeof(historyList[0]);
	int i = 0;
	if(lists == NULL) {
		return -1;
	}
	int start_index = lists->recordIdx;
	//char type[10] ={0};
	//snprintf(type, sizeof(type), "%d", types);

	ST_NSDK_SYSTEM_TIME systime = {0};
	NETSDK_conf_system_get_time(&systime);
	int timezone_second = ((abs(systime.greenwichMeanTime) / 100) * (3600)
							+(abs(systime.greenwichMeanTime) % 100) * 60)
							* (systime.greenwichMeanTime > 0 ? 1 : -1);
	printf("timezone_second = %d\n", timezone_second);
	printf("systime.greenwichMeanTime = %d\n", systime.greenwichMeanTime);
	printf("startTime - timezone_second = %d\n", begin_time - timezone_second);
	printf("endTime - timezone_second = %d\n", end_time - timezone_second);

	/**
	 * ´Ë´¦µ÷ÓÃ»ñÈ¡Â¼ÏñÁÐ±íÊ±,typeÉèÎª¿Õ,Ä¬ÈÏÌáÈ¡ËùÓÐÂ¼ÏñÀàÐÍ
	 */
	if(0 == NK_TFCARD_get_history(begin_time - timezone_second, end_time - timezone_second,
								  NULL, historyList, start_index, &historyCnt))
	{
		//APP_TRACE("start_index : %d, historyCnt : %d",start_index,historyCnt);
		for (i = 0; i < lists->maxRecord && i < historyCnt; ++i)
		{
			lists->pRecord[i].chn = 0;
			lists->pRecord[i].type = type;
			lists->pRecord[i].startTime = historyList[i].beginTm + timezone_second;//+8*60*60;
			lists->pRecord[i].endTime = historyList[i].endTm + timezone_second;//+8*60*60;
			lists->pRecord[i].quality = quality;
		}
		lists->recordCnt = i;
		lists->recordTotal = historyCnt;
		APP_TRACE("__FUNCTION__:%s, LINE:%d, lists->recordIdx:%d, lists->listCnt:%d, lists->maxRecord:%d, history_count:%d\n",
			__FUNCTION__, __LINE__, lists->recordIdx, lists->recordCnt, lists->maxRecord, historyCnt);
		
		return 0;
	}else{
		APP_TRACE("p2p playback VIDEO history is can't search!!!");
	}
#endif
	return -1;
}


void* OnOpenRecFiles(void *ctx, uint32_t chn_cnt, const uint8_t *chns, uint32_t type, time_t begin_time, time_t end_time, uint32_t quality)
{
#if defined(TFCARD)
#if 0
	// play start 
	printf("__FUNCTION__:%s, LINE:%d, startTime:%d\n", __FUNCTION__, __LINE__, startTime);
	Recctx_f *recframe = (Recctx_f *)calloc(1,sizeof(Recctx_f));
	char type[10] ={0};
	int play_ret = -1;
	ST_NSDK_SYSTEM_TIME systime;
	NETSDK_conf_system_get_time(&systime);
	int timezone_second = ((abs(systime.greenwichMeanTime) / 100) * (3600)
							+(abs(systime.greenwichMeanTime) % 100) * 60)
							* (systime.greenwichMeanTime > 0 ? 1 : -1);
	printf("timezone_second = %d\n", timezone_second);
	printf("systime.greenwichMeanTime = %d\n", systime.greenwichMeanTime);

	recframe->rec_attr = (stSDK_ENC_BUF_ATTR*)calloc(1,sizeof(stSDK_ENC_BUF_ATTR));
	recframe->raw_buff = calloc(1, SIZE_BUFF);

	recframe->frame_maxlen = SIZE_BUFF;
	recframe->flag = 0;
		
	snprintf(type, sizeof(type), "%d", recType);
	play_ret = g_pTFer->play(g_pTFer, TRUE, type, startTime-timezone_second, (void*)recframe);

	//printf("__FUNCTION__:%s, LINE:%d, play_ret:%d\n", __FUNCTION__, __LINE__, play_ret);
	recframe->recType = recType;
	recframe->utc_t = startTime-timezone_second;

	return (void*)recframe;
#else
	/**
	 * µ±TF¿¨ÎÞ¹ÒÔØÊ±,ÎÞ·¨½øÐÐÂ¼Ïñ»Ø·Å
	 */
	if(!NK_TFCARD_is_mounted()){
		return NULL;
	}
	/**
	 * ÉÏÏÂÎÄ¶¨ÒåÒ»¸öÕûÐÎ±äÁ¿Ö¸Õë¾ä±ú,ÔÝÊ±²»¿¼ÂÇ¶àÈËÍ¬Ê±»Ø·Å
	 */
	struct StreamCtx *recCtx = (struct StreamCtx*)calloc(1,sizeof(struct StreamCtx));  
	ST_NSDK_SYSTEM_TIME systime;
	NETSDK_conf_system_get_time(&systime);
	NK_Int timezone_second = ((abs(systime.greenwichMeanTime) / 100) * (3600)
							+(abs(systime.greenwichMeanTime) % 100) * 60)
							* (systime.greenwichMeanTime > 0 ? 1 : -1);
	printf("timezone_second = %d\n", timezone_second);
	printf("systime.greenwichMeanTime = %d\n", systime.greenwichMeanTime);

	recCtx->isOldTypeRec = true;
	recCtx->bypass_frame_flag = false;//remove bypass frame in playback in case of avoiding phone app crashing
	if(0 != NK_TFCARD_play_start(begin_time-timezone_second,NULL)){
		APP_TRACE("1111111111111");
		if(recCtx){
			if(recCtx->recFrameBuf) {
				free(recCtx->recFrameBuf);
			}
			free(recCtx);
		}
		return NULL;
	}
	else{
		if(recCtx){
			recCtx->bypass_frame_flag = false;//remove bypass frame in playback in case of avoiding phone app crashing
			recCtx->recFrameBuf = (void *)calloc(1, P2P_REC_FRAME_BUF_SIZE);
			recCtx->recFrameBufSize = P2P_REC_FRAME_BUF_SIZE;
			if(recCtx->recFrameBuf) {
				return (void *)recCtx;
			}
		}
	}
#endif
#endif
	return NULL;
}

int OpenRecFiles(time_t begin_time, time_t end_time)
{
#if defined(TFCARD)
	/**
	 * µ±TF¿¨ÎÞ¹ÒÔØÊ±,ÎÞ·¨½øÐÐÂ¼Ïñ»Ø·Å
	 */
	if(!NK_TFCARD_is_mounted()){
		return NULL;
	}
	/**
	 * ÉÏÏÂÎÄ¶¨ÒåÒ»¸öÕûÐÎ±äÁ¿Ö¸Õë¾ä±ú,ÔÝÊ±²»¿¼ÂÇ¶àÈËÍ¬Ê±»Ø·Å
	 */
	ST_NSDK_SYSTEM_TIME systime;
	NETSDK_conf_system_get_time(&systime);
	NK_Int timezone_second = ((abs(systime.greenwichMeanTime) / 100) * (3600)
							+(abs(systime.greenwichMeanTime) % 100) * 60)
							* (systime.greenwichMeanTime > 0 ? 1 : -1);
	printf("timezone_second = %d\n", timezone_second);
	printf("systime.greenwichMeanTime = %d\n", systime.greenwichMeanTime);

	if(0 != NK_TFCARD_play_start(begin_time-timezone_second,NULL)){
		return -1;
	}
	else{
		return 0;
	}
#endif
	return -1;
}


int OnCloseRecFiles(void *ctx)
{

#if defined(TFCARD)
#if 0

	// play stop
	Recctx_f *my_frame = (Recctx_f *)ctx;
	char type[10] ={0};
	snprintf(type, sizeof(type), "%d", my_frame->recType);

	if(0 == g_pTFer->play(g_pTFer, FALSE, type, my_frame->utc_t, NULL))
	{
		if(my_frame->rec_attr)
		{
			free(my_frame->rec_attr);
			my_frame->rec_attr = NULL;
		}
		if(my_frame->raw_buff)
		{
			free(my_frame->raw_buff);
			my_frame->raw_buff = NULL;
		}
		if(my_frame)
		{
			free(my_frame);
			my_frame = NULL;
		}
		return 0;
	}
#else
	struct StreamCtx *recCtx = (struct StreamCtx *)ctx;

	if(recCtx != NULL) {

	
		if(recCtx->isOldTypeRec == true) {
			NK_TFCARD_play_stop();
			if(recCtx->recFrameBuf) {
				free(recCtx->recFrameBuf);
				recCtx->recFrameBuf = NULL;
			}
			free(recCtx);
			recCtx = NULL;
			return 0;
		}
	}
	else {
		return -1;
	}
#endif
#endif
	return -1;
}

ssize_t OnReadRecFrame(void *replay_ctx, void **frame)
{
#if defined(TFCARD)
#if 0
	if(NULL == ctx || NULL == g_pTFer)
	{
		usleep(20000);
		return -1;
	}
	Recctx_f *my_frame = (Recctx_f *)ctx;
	if(my_frame->flag == -1)
	{
		printf("__FUNCTION__:%s, LINE:%d, ctx=%p, flag:%d\n", __FUNCTION__, __LINE__, ctx, my_frame->flag);
		return -1;
	}
	else if(my_frame->flag == 1)
	{
		unsigned int data_len = 0;
		if(my_frame && my_frame->raw_buff)
		{
			data_len =  my_frame->rec_attr->data_sz;

			if(data_len > raw_frame_max_sz)
			{
				printf("raw_frame_max_sz = %d", data_len);
				my_frame->flag = 0;
				usleep(20000);
				return -1;
			}
			memcpy(raw_frame, my_frame->raw_buff, data_len);
			fill_frame_head(P2P_FRAMEHEAD_HEADTYPE_REPLAY, pFrameHead, my_frame->rec_attr);
		}
		my_frame->flag = 0;
		usleep(1000);
		return data_len;
	}
#else
	stRecord_Frame_Head frameHead = {0};
	NK_Int dataSize = 0;

	/**
	 * µ±TF¿¨ÎÞ¹ÒÔØÊ±,ÎÞ·¨½øÐÐÂ¼Ïñ»Ø·Å
	 */
	if(!NK_TFCARD_is_mounted()){
		APP_TRACE("TFcard not mounted!");
		return -1;
	}

	struct StreamCtx *pCtx = (struct StreamCtx*)replay_ctx;
	if (!pCtx) {
		printf("DeviceReadFrame failed:nil replay_ctx/pMEDIABUF\n");
		return -1;
	}
	if(pCtx->isOldTypeRec == false) {
		return -1;
	}

	kp2p_frame_head_t *pFrameHead = (kp2p_frame_head_t *)(pCtx->recFrameBuf);
	void *raw_frame = pCtx->recFrameBuf + sizeof(kp2p_frame_head_t);
	size_t raw_frame_max_sz = pCtx->recFrameBufSize - sizeof(kp2p_frame_head_t);
	//send bypass frame
	if(NK_False == GLOBAL_sn_front()){
		pCtx->bypass_frame_flag = false;
	}
	
	if(pCtx->bypass_frame_flag){
#if defined(PX_720)
		//setBypassFrame2(pFrameHead, raw_frame);
#else
		setBypassFrame(pFrameHead, raw_frame);
#endif
		pCtx->bypass_frame_flag = false;
		return pFrameHead->framesize;
	}

	dataSize = NK_TFCARD_play_read_frame(&frameHead,(NK_PByte)raw_frame, raw_frame_max_sz);
	if(dataSize <= 0){
		APP_TRACE("read frame error : dataSize = %d, raw_frame_max_sz = %d",dataSize,raw_frame_max_sz);
		return -1;
	}

	pFrameHead->magic = P2PSDK_FRAMEHEAD_MAGIC;
    pFrameHead->magic2 = P2PSDK_FRAMEHEAD_MAGIC2;
    pFrameHead->frame_seq = 0;  // è¿™éƒ¨åˆ†ä»£ç å·²ç»æ²¡ä½¿ç”¨ï¼Œæ‰€ä»¥å¸§åºå·ç›´æŽ¥å¡«0
	pFrameHead->version = P2PSDK_FRAMEHEAD_VERSION;
	pFrameHead->headtype = P2P_FRAMEHEAD_HEADTYPE_REPLAY;//Êý¾ÝÀàÐÍ 0:Ö±²¥ 1:Â¼Ïñ
	pFrameHead->framesize = frameHead.dataSize;
	pFrameHead->ts_ms = frameHead.sysTime_ms;
	pFrameHead->replay.type = P2P_REC_TYPE_ALL;
	pFrameHead->replay.channel = 0;
	pFrameHead->replay.quality = 0;

	//²âÊÔÓÃ,µÈÎÈ¶¨ºóÔÙÉ¾³ý.
/*	if(frameHead.codec == 96 || frameHead.codec == 97){
	printf("--------message in frame head------- \n\
codec : %s\n\
coderStamp_ms : %lld\n\
sysTime_ms : %lld\n\
dataSize : %u\n\
headSize : %d\n\
headArr : %p\n",
frameHead.codec==96?"264":frameHead.codec==97?"265":"audio",
frameHead.coderStamp_ms,
frameHead.sysTime_ms,
frameHead.dataSize,
sizeof(stRecord_Frame_Head),
&frameHead);
	}
*/
	if(frameHead.codec == NK_TFCARD_VCODEC_H264){
		pFrameHead->replay.frametype = (frameHead.isKeyFrame ? 1 : 2);
		snprintf(pFrameHead->replay.v.enc, sizeof(pFrameHead->replay.v.enc), "%s", "H264");
		pFrameHead->replay.v.fps = frameHead.fps;
		pFrameHead->replay.v.width = frameHead.width;
		pFrameHead->replay.v.height = frameHead.height;
	}else if(frameHead.codec == NK_TFCARD_VCODEC_H265){
		pFrameHead->replay.frametype = (frameHead.isKeyFrame ? 1 : 2);
		snprintf(pFrameHead->replay.v.enc, sizeof(pFrameHead->replay.v.enc), "%s", "H265");
		pFrameHead->replay.v.fps = frameHead.fps;
		pFrameHead->replay.v.width = frameHead.width;
		pFrameHead->replay.v.height = frameHead.height;
	}else if(frameHead.codec == NK_TFCARD_ACODEC_AAC){
		pFrameHead->replay.frametype = 0;
		snprintf(pFrameHead->replay.a.enc, sizeof(pFrameHead->replay.a.enc), "%s", "AAC");
		pFrameHead->replay.a.samplerate = frameHead.sampleRate;
		pFrameHead->replay.a.samplewidth = frameHead.sampleWidth;
		pFrameHead->replay.a.channels = 1;
		pFrameHead->replay.a.compress = frameHead.compressionRatio;
	}else if(frameHead.codec == NK_TFCARD_ACODEC_G711A){
		pFrameHead->replay.frametype = 0;
		snprintf(pFrameHead->replay.a.enc, sizeof(pFrameHead->replay.a.enc), "%s", "G711A");
		pFrameHead->replay.a.samplerate = frameHead.sampleRate;
		pFrameHead->replay.a.samplewidth = frameHead.sampleWidth;
		pFrameHead->replay.a.channels = 1;
		pFrameHead->replay.a.compress = frameHead.compressionRatio;
	}else{
		APP_TRACE("unknow frameHead.codec = %d!", frameHead.codec);
		dataSize = 0;
	}

	*frame = (void *)(pCtx->recFrameBuf);
	return dataSize + sizeof(kp2p_frame_head_t);
#endif
#endif
	return 0;	
}

int OnFetchRecListNew(void *ctx, uint32_t chn_cnt, const uint8_t *chns, uint32_t type, time_t begin_time, time_t end_time, uint32_t quality, RecList *lists)
{
	APP_TRACE("on fetch rec list... ");
#if defined(TFCARD)
    int TotalRecs;
    int i;
    int startIndex;
    size_t recListSize;

    if(lists == NULL) {
        APP_TRACE("lists can't be NULL!");
        return -1;
    }

    if(lists->maxRecord <= 0) {
        APP_TRACE("lists->maxRecord should > 0!");
        return -1;
    }

    recListSize = 100;
    startIndex = lists->recordIdx;
#if !defined(TS_RECORD)
    stTFCARD_History_List historyList[recListSize];

    TotalRecs = REC_PLAY_get_history(begin_time, end_time,
                                     type, startIndex,
                                     historyList, &recListSize);
    if (TotalRecs <= 0) {
		/* ¼ìË÷¾É¸ñÊ½Â¼Ïñ */
		/*if(GLOBAL_isOldTypeRecord()) {
			return OnFetchRecList(ctx, chn_cnt, chns, type, begin_time, end_time, quality, lists);
		}*/

        APP_TRACE("Failed to search record");
        lists->recordCnt = 0;
        lists->recordTotal = 0;
        return -1;
    }
//    APP_TRACE("TotalRecs: %d, recListSize: %lu", TotalRecs, recListSize);

	lists->recordCnt = 0;
    for (i = 0; i < lists->maxRecord && i < recListSize; i++) {
        lists->pRecord[i].chn = 0;
		if(historyList[i].recordType[0] == 'T'){
			lists->pRecord[i].type = NK_REC_TIMER;
		}
		else if(historyList[i].recordType[0] == 'M'){
			lists->pRecord[i].type = NK_REC_MOTION;
		}
		else{
			lists->pRecord[i].type = NK_REC_TIMER;
		}
        lists->pRecord[i].startTime = historyList[i].beginTm;
        lists->pRecord[i].endTime = historyList[i].endTm;
        lists->pRecord[i].quality = quality;
        lists->recordCnt ++;
    }
    lists->recordTotal = TotalRecs;
#else
    NK_Int indexTmp = 0;
    pstTFPLAYFILE pstFile;

    // æœç´¢ä»Žindex=0å¼€å§‹ï¼Œå…ˆç¡®å®šä¸Šä¸€æ¬¡çš„æœç´¢æ˜¯å¦å·²ç»é‡Šæ”¾å®Œæ¯•
    if(0 == lists->recordIdx)
    {
        if(NULL != g_tfPlayHistoryHandle)
        {
            NK_TFPLAY_FreeHistory(g_tfPlayHistoryHandle);
            g_tfPlayHistoryHandle = NULL;
        }

        g_tfPlayHistoryHandle = NK_TFPLAY_CreateHistory(0, begin_time, end_time, NK_REC_TIMER | NK_REC_MOTION, NULL);
        if(NULL == g_tfPlayHistoryHandle)
        {
            APP_TRACE("Failed to search record");
            lists->recordCnt = 0;
            lists->recordTotal = 0;
            return -1;
        }
    }

    if(NULL != g_tfPlayHistoryHandle)
    {
        TotalRecs = NK_TFPLAY_GetHistoryLen(g_tfPlayHistoryHandle);
        lists->recordCnt = 0;
        for(i = 0; (i < lists->maxRecord) && (i < TotalRecs - startIndex); i++)
        {
            indexTmp = startIndex + i;
            if(NK_TRUE == NK_TFPLAY_GetHistoryFile(g_tfPlayHistoryHandle, indexTmp, (NK_PVoid *)&pstFile))
            {
                lists->pRecord[i].chn = pstFile->nChn;
                lists->pRecord[i].type = pstFile->nFileType;
                lists->pRecord[i].startTime = pstFile->nStartUtc;
                lists->pRecord[i].endTime = pstFile->nEndUtc;
                lists->pRecord[i].quality = quality;
                lists->recordCnt++;
            }
        }
        if(indexTmp == TotalRecs - 1)
        {
            NK_TFPLAY_FreeHistory(g_tfPlayHistoryHandle);
            g_tfPlayHistoryHandle = NULL;
        }
        lists->recordTotal = TotalRecs;

    }

#endif
	APP_TRACE("on fetch rec list done, return rec cnt: %d, total: %d, start at index: %d",
			  lists->recordCnt, lists->recordTotal, lists->recordIdx);
    return 0;

#else
    APP_TRACE("firmware do not support tf card!");
    if(lists == NULL) {
        APP_TRACE("pLists can't be NULL!");
        return -1;
    }
    lists->recordCnt = 0;
    lists->recordTotal = 0;
    return -1;
#endif
}

void* OnOpenRecFile2(void *ctx, uint32_t chn_cnt, const uint8_t *chns, uint32_t type, time_t begin_time, time_t end_time, uint32_t quality, uint32_t open_type)
{
	void *ret = NULL;
	struct StreamCtx *recCtx = NULL;
	int oldRet = -1;
	APP_TRACE("on open rec... (begin_time: %ld, end_time: %ld)", begin_time, end_time);

#if defined(TFCARD)

	if(GLOBAL_enter_playback() == 0){
#if defined(TS_RECORD)
        if(NULL != g_tfPlayHistoryHandle)
        {
            NK_TFPLAY_FreeHistory(g_tfPlayHistoryHandle);
            g_tfPlayHistoryHandle = NULL;
        }
        ret = NK_TFPLAY_OpenEx(0, begin_time, end_time, EN_RECORD_TYPE_MOTION | EN_RECORD_TYPE_TIMER, NK_Nil);
#else
		ret = REC_PLAY_start(begin_time, end_time, type, open_type);
#endif
		if(NULL == ret) {
			/*if(GLOBAL_isOldTypeRecord()) {
				oldRet = OpenRecFiles(begin_time, end_time);
			}*/
			GLOBAL_leave_playback();
			APP_TRACE("open rec failed");
		}
		else {
			recCtx = (struct StreamCtx*)calloc(1,sizeof(struct StreamCtx));
			if(recCtx != NULL) {
				recCtx->isOldTypeRec = false;
				recCtx->bypass_frame_flag = false; // remove bypass frame in playback in case of avoiding phone app crashing
#if !defined(TS_RECORD)
				recCtx->recFrameBuf = (void *)calloc(1, P2P_REC_FRAME_BUF_SIZE);
				if(NULL == recCtx->recFrameBuf) {
					recCtx->recFrameBufSize = 0;
				}
				else {
					recCtx->recFrameBufSize = P2P_REC_FRAME_BUF_SIZE;
				}
#else
                memset(p2pRecFrameBuf, 0, P2P_REC_FRAME_BUF_SIZE);
                recCtx->recFrameBuf = (void *)p2pRecFrameBuf;
                recCtx->recFrameBufSize = P2P_REC_FRAME_BUF_SIZE;
#endif
				recCtx->play_rec_search_ctx = ret;
                recCtx->rec_frame_seq = 0;
#if defined(TS_RECORD)
                recCtx->rec_startTime = begin_time;
#endif
				APP_TRACE("on open rec done (rec ctx: %p)", ret);
			}

		}
	}
	else{
		APP_TRACE("open rec failed, user is full");
	}

	return recCtx;
#else
	APP_TRACE("firmware do not support tf card!");
    return NULL;
#endif
}

int OnCloseRecFilesNew(void *ctx)
{
	int ret = -1;

    APP_TRACE("on close rec... (rec ctx: %p)", ctx);

#if defined(TFCARD)
	if(!ctx){
		return -1;
	}
	GLOBAL_leave_playback();
	struct StreamCtx *recCtx = (struct StreamCtx *)ctx;

	if(recCtx) {
#if defined(TS_RECORD)
        NK_TFPLAY_Close(recCtx->play_rec_search_ctx);
        recCtx->play_rec_search_ctx = NULL;
        APP_TRACE("on close rec done (rec ctx: %p)", ctx);
#else
	    ret = REC_PLAY_stop(recCtx->play_rec_search_ctx);
	    if (0 == ret) {
	        APP_TRACE("on close rec done (rec ctx: %p)", ctx);
	    } else {
			/*if(GLOBAL_isOldTypeRecord()) {
				if(0 == OnCloseRecFiles(ctx)) {
					APP_TRACE("on close old rec done (rec ctx: %p)", ctx);
					//return 0;
				}
			}
			else {
				APP_TRACE("close rec failed");
			}
			APP_TRACE("close rec failed");*/
			APP_TRACE("close rec failed");
	    }
			//APP_TRACE("on close rec done (rec ctx: %p)", ctx);
#endif

		if(recCtx->recFrameBuf) {
			free(recCtx->recFrameBuf);
			recCtx->recFrameBuf = NULL;
		}
		free(recCtx);
		recCtx = NULL;
	}

	return ret;
#else
    APP_TRACE("firmware do not support tf card!");
    return -1;
#endif
}

ssize_t OnReadRecFrameNew(void *replay_ctx, void **frame)
{
	//APP_TRACE("on read frame... (rec ctx: %p)", replay_ctx);
#if defined(TFCARD)
    ssize_t ret, dataSize = 0;
    stRecord_Frame_Head frameHead;
    unsigned int seek_time = 0;

	if(!replay_ctx){
		APP_TRACE("on read frame, replay_ctx is null !");
		return -1;
	}
	//return OnReadRecFrame(replay_ctx, frame);

	struct StreamCtx *recCtx = (struct StreamCtx *)replay_ctx;
	kp2p_frame_head_t *pFrameHead = (kp2p_frame_head_t *)(recCtx->recFrameBuf);
	void *raw_frame = NULL;
	size_t raw_frame_max_sz = recCtx->recFrameBufSize - sizeof(kp2p_frame_head_t);

	if(NULL == pFrameHead) {
		APP_TRACE("on read frame failed, pFrameHead: %p)", pFrameHead);
		return -1;
	}

#if defined(TS_RECORD)

    if(recCtx->rec_startTime != 0)
    {
        seek_time = recCtx->rec_startTime;
        if(NK_FALSE == NK_TFPLAY_ReadSeek(recCtx->play_rec_search_ctx,&frameHead,(void **)&raw_frame ,seek_time))
        {
            APP_TRACE("[%s:%d]Fixed point playback failure",__func__,__LINE__);
            return -1;

        }
        recCtx->rec_startTime = 0;
    }
    else
    {
        if(false == NK_TFPLAY_Read(recCtx->play_rec_search_ctx, 0,&frameHead,(void **)&raw_frame))
        {
            APP_TRACE("read frame error : dataSize = %d",frameHead.dataSize);
            return -1;
        }
        recCtx->rec_startTime = 0;
    }

    if(NULL == raw_frame)
    {
        if((frameHead.codec == NK_TFCARD_VCODEC_H264) ||
            (frameHead.codec == NK_TFCARD_VCODEC_H265))
        {
            usleep(15000);
        }
        printf("raw_frame failed:nil \n");
        usleep(5000);
        return 0;
    }

#else
    raw_frame = recCtx->recFrameBuf + sizeof(kp2p_frame_head_t);
    if(NULL == raw_frame) {
        APP_TRACE("on read frame failed, raw_frame: %p)", raw_frame);
        return -1;
    }
    ret = REC_PLAY_read_frame(recCtx->play_rec_search_ctx, &frameHead, (uint8_t *)raw_frame, raw_frame_max_sz);
    if (ret < 0) {
		/*if(GLOBAL_isOldTypeRecord()) {
			return OnReadRecFrame(replay_ctx, frame);
		}*/
		APP_TRACE("on read frame, failed to read frame, ret: %ld (rec replay_ctx: %p)", ret, replay_ctx);
        return -1;
    }
#endif

    pFrameHead->magic = P2PSDK_FRAMEHEAD_MAGIC;
    pFrameHead->magic2 = P2PSDK_FRAMEHEAD_MAGIC2;
    pFrameHead->frame_seq = 0;
    pFrameHead->version = P2PSDK_FRAMEHEAD_VERSION;
    pFrameHead->headtype = P2P_FRAMEHEAD_HEADTYPE_REPLAY;//Êý¾ÝÀàÐÍ 0:Ö±²¥ 1:Â¼Ïñ
    pFrameHead->framesize = frameHead.dataSize;

#if defined(TS_RECORD)
	ST_NSDK_SYSTEM_TIME systime;
	NETSDK_conf_system_get_time(&systime);
	NK_Int timezone_second = ((abs(systime.greenwichMeanTime) / 100) * (3600)
							+(abs(systime.greenwichMeanTime) % 100) * 60)
							* (systime.greenwichMeanTime > 0 ? 1 : -1);
    pFrameHead->ts_ms = frameHead.sysTime_ms - timezone_second * 1000;
#else
    pFrameHead->ts_ms = frameHead.sysTime_ms;
#endif
    dataSize = frameHead.dataSize;
	pFrameHead->replay.type = P2P_REC_TYPE_ALL;  // Ä¿Ç°»¹Ã»×öÂ¼ÏñÀàÐÍÇø·Ö
	pFrameHead->replay.channel = 0;
	pFrameHead->replay.quality = 0;
//    APP_TRACE("pFrameHead->ts_ms: %llu", pFrameHead->ts_ms);

    if(frameHead.codec == NK_TFCARD_VCODEC_H264){
        pFrameHead->frame_seq = recCtx->rec_frame_seq++;  // video frame seq
		pFrameHead->replay.frametype = (frameHead.isKeyFrame ? 1 : 2);
		snprintf(pFrameHead->replay.v.enc, sizeof(pFrameHead->replay.v.enc), "%s", "H264");
		pFrameHead->replay.v.fps = frameHead.fps;
		pFrameHead->replay.v.width = frameHead.width;
		pFrameHead->replay.v.height = frameHead.height;
    }else if(frameHead.codec == NK_TFCARD_VCODEC_H265){
        pFrameHead->frame_seq = recCtx->rec_frame_seq++;  // video frame seq
		pFrameHead->replay.frametype = (frameHead.isKeyFrame ? 1 : 2);
		snprintf(pFrameHead->replay.v.enc, sizeof(pFrameHead->replay.v.enc), "%s", "H265");
		pFrameHead->replay.v.fps = frameHead.fps;
		pFrameHead->replay.v.width = frameHead.width;
		pFrameHead->replay.v.height = frameHead.height;
    }else if(frameHead.codec == NK_TFCARD_ACODEC_G711A){
		pFrameHead->replay.frametype = 0;
		snprintf(pFrameHead->replay.a.enc, sizeof(pFrameHead->replay.a.enc), "%s", "G711A");
		pFrameHead->replay.a.samplerate = frameHead.sampleRate;
		pFrameHead->replay.a.samplewidth = frameHead.sampleWidth;
		pFrameHead->replay.a.channels = 1;
		pFrameHead->replay.a.compress = frameHead.compressionRatio;
    }else if(frameHead.codec == NK_TFCARD_ACODEC_AAC){
		pFrameHead->replay.frametype = 0;
		snprintf(pFrameHead->replay.a.enc, sizeof(pFrameHead->replay.a.enc), "%s", "AAC");
		pFrameHead->replay.a.samplerate = frameHead.sampleRate;
		pFrameHead->replay.a.samplewidth = frameHead.sampleWidth;
		pFrameHead->replay.a.channels = 1;
		pFrameHead->replay.a.compress = frameHead.compressionRatio;
    }else if(frameHead.codec == NK_TFCARD_CODEC_DATA){
		pFrameHead->replay.frametype = P2P_FRAMEHEAD_FRAMETYPE_OOB;
    }else{
        APP_TRACE("unknow frameHead.codec = %d!", frameHead.codec);
        dataSize = 0;
    }

	//APP_TRACE("on read frame done, frame size: %lu, type: %d (rec ctx: %p)",
	//		  frameHead.dataSize, frameHead.codec, replay_ctx);
#if defined(TS_RECORD)
    memcpy(recCtx->recFrameBuf + sizeof(kp2p_frame_head_t) ,raw_frame , pFrameHead->framesize);
#endif
	*frame = (void *)(recCtx->recFrameBuf);
    return dataSize + sizeof(kp2p_frame_head_t);
#else
    APP_TRACE("firmware do not support tf card!");
    return -1;
#endif
}

int OnAfterReadRecFrame(void *replay_ctx, void *frame)
{
	//printf("%s\n", __FUNCTION__);

	return 0;
}

void *OnFindFileStart(void *ctx, kp2p_find_file_cond_t *cond)
{
	return NULL;
	
}

int OnFindNextFile(void *find_ctx, kp2p_find_file_data_t *file_data)
{
	return 0;
}

int OnFindFileStop(void *find_ctx)
{
	return 0;
}

ssize_t OnRemoteSetup(void *ctx, const char *req_data, size_t req_data_size, void *rsp_buf, size_t rsp_buf_size)
{
	ssize_t respSz = 0;
    if (!rsp_buf) {
        printf("OnRemoteSetup err\n");
        return 0;
    }
    printf("OnRemoteSetup--->Req%s\n", (char*)req_data);
	if(NULL != req_data)
	{
		respSz = p2p_parse((void *)req_data, rsp_buf, rsp_buf_size);
	}
	if(0 == respSz)
	{
		sprintf((char *)rsp_buf, "{\r\n\"option\" :\"failed\"\r\n}\r\n");
	}
	else if(-1 == respSz){
		sprintf((char *)rsp_buf, "{\r\n\"option\" :\"Authorization failed\"\r\n}\r\n");
	}
	else if(-2 == respSz){
		printf("OnRemoteSetup--->buf size [%d] too small \n", rsp_buf_size);
		return -1;
	}

	respSz = strlen((char *)rsp_buf);
	printf("OnRemoteSetup--->Resp:%s, respSz=%d\n", (char*)rsp_buf, respSz);

    return respSz;
}

void*  OnAuP2PCall(void *ctx, void *session, uint32_t chn, int *errNo)
{
	void * ret = NULL;
#if defined(VOICE_TALK)
    printf("FUNC:OnAuP2PCall1......seesion:%p, chn:%d\n", session, chn);
	if(GLOBAL_enter_twowaytalk() != 0) {
		printf("OnAuP2PCall busy!\n");
		*errNo = -1;
		return NULL;
	}

	ret = SOUND_QueueisInit();
	if(!ret){
		GLOBAL_leave_twowaytalk();
		*errNo = -2;
	}

	return ret;
#endif
	*errNo = -2;
	return NULL;
}

#define P2P_BANDWIDTH_STEP (50)//bps
void OnBindwidthChanged(void *ctx, float c_recv_bw)
{
#if 0
	struct StreamCtx *pCtx = (struct StreamCtx*)ctx;
	if (!pCtx) {
		printf("OnBindwidthChanged failed:nil ctx\n");
		return;
	}

	int bw = 0;
	int vin = pCtx->chn;
	int stream = pCtx->streamNo;
	int ret = 0;
	int *pts_bps = NULL;
	ST_SDK_ENC_STREAM_ATTR stream_attr;
	ST_NSDK_VENC_CH venc_ch;
	memset(&stream_attr,0,sizeof(ST_SDK_ENC_STREAM_ATTR));
	
	bw = (int)c_recv_bw*8*85/100;//85%bandwidth

	ret = SDK_ENC_get_stream(vin, stream, &stream_attr);
	switch(stream_attr.enType){
		default:
		case kSDK_ENC_BUF_DATA_H264:
			pts_bps = &stream_attr.H264_attr.bps;
			break;
		case kSDK_ENC_BUF_DATA_H265:
			pts_bps = &stream_attr.H265_attr.bps;
			break;
	}
	
	printf("bw = %d(%f) now:%d\n", bw, c_recv_bw*8, *pts_bps);
	
	if(bw < *pts_bps && bw > 128){
		//need to reduce bandwidth
		*pts_bps = bw;
		SDK_ENC_set_stream (vin, stream, &stream_attr);
	}else if(bw - P2P_BANDWIDTH_STEP > *pts_bps){
		//need to raise bandwidth
		memset(&venc_ch, 0, sizeof(ST_NSDK_VENC_CH));
		NETSDK_conf_venc_ch_get((vin+1)*100+stream+1, &venc_ch);
		
		bw = bw < venc_ch.constantBitRate ? bw : venc_ch.constantBitRate;
		*pts_bps = bw;
		SDK_ENC_set_stream (vin, stream, &stream_attr);
	}
#endif
}

void OnConnected(void *ctx, void *session)
{

}

void OnClosed(void *ctx, void *session)
{

}

void OnBandwidthChanged(void *ctx, void *session, uint32_t bandwitdh)
{

}

ssize_t OnAuP2PTalkRecv(void *talk_ctx, kp2p_frame_head_t *frame_head, const void *voice_data, size_t voice_data_size)
{
	int ret = -1;
#if defined(VOICE_TALK)
	if(talk_ctx){
		ret = SOUND_writeQueue((unsigned char *)voice_data, voice_data_size, emSOUND_DATA_TYPE_P2P_G711A, emSOUND_PRIORITY_ZERO);
	}
#endif
	return ret;
}

int OnAuP2PHangup(void *talkCtx)
{
#if defined(VOICE_TALK)
    printf("VoP2P Hangup......%p\n", talkCtx);
	if(talkCtx){
		GLOBAL_leave_twowaytalk();
	    SOUND_releasePriority();
	}
	return 0;
#endif
}

int OnPullAlarmMsg(void *ctx, void *session, uint32_t chn, time_t ticket, uint32_t type)
{
	return 0;
}

int OnForceIFrame(void *ctx)
{
	return 0;
}

void OnVconRecv(void *ctx, void *session, uint32_t authed, uint32_t channel, const void *data, uint32_t data_size)
{
	size_t write_count = 0;
	uint32_t renddatasize = 0;
	uint32_t errorflag = 0;
	int fseekret = 0;
	unsigned int fileoffset = 0;
	static char dst_file_path[128] = {0};
	static uint32_t packagecount = 0;
    static uint32_t packetsize = 0;
	FILE *fp_srcfile = NULL;
	static unsigned int PackFileType = 0;
	vconRendDataHead_t *vDemo_r = (vconRendDataHead_t *)malloc(1 * sizeof(vconRendDataHead_t));
	vDemo_r->magic = MAGIC_NUMBER_RETRANSMIT;

	if(!data)
	{
		printf("OnVconRecv recv data package error !!\n");
		return NULL;
	}

	struct vconDataHead *vDemo = (struct vconDataHead *)data;

	if(!vDemo)
	{
		printf("OnVconRecv recv data struct error !!\n");
		return NULL;
	}

	char *src_file_buf = (char *)malloc(PACKET_MAX_LENGHT * sizeof(char));
/*
	printf("magic       = 0x%08x\n", vDemo->magic);
	printf("version     = %u\n", vDemo->version);
	printf("FileType    = %u\n", vDemo->FileType);
	printf("packedFlag  = %u\n", vDemo->packedFlag);
	printf("nFileLen    = %u\n", vDemo->nFileLen);
	printf("npackedLen  = %u\n", vDemo->npackedLen);
	printf("npackedNo   = %u\n", vDemo->npackedNo);
	printf("sizeof stru = %d\n", sizeof(vconDataHead_t));
	printf("------------------------------------------\n");
	printf("vDemo  addr = %p\n", (vDemo + 1));  // æœ¬åœ°å­˜æ”¾æ•°æ®çš„èµ·å§‹åœ°å€(æ­£ç¡®ï¼Œé€šè¿‡åç§»é‡å–æ•°æ®)
*/
	// å¤„ç†ä¼ è¿‡æ¥çš„æ•°æ®
	// magic number
	if(MAGIC_NUMBER != vDemo->magic)
	{
		printf("--- %s: Magic number error (NO.%d pack) : 0x%08x!\n", __FUNCTION__, vDemo->npackedNo, vDemo->magic);

		vDemo_r->version = 1;
		vDemo_r->FileType = vDemo->FileType;
		vDemo_r->npackedNo = vDemo->npackedNo;
		vDemo_r->endflag = 1;
		renddatasize = sizeof(vconRendDataHead_t);
		P2PSDKVconSend(session, 0, (void *)vDemo_r, renddatasize);

		errorflag = 1;
	}

	// ç‰ˆæœ¬ä¿¡æ¯
	if(1 != vDemo->version)
	{
		printf("--- %s: Vcon version error (NO.%d pack) : %d!\n", __FUNCTION__, vDemo->npackedNo, vDemo->version);

		vDemo_r->version = 1;
		vDemo_r->FileType = vDemo->FileType;
		vDemo_r->npackedNo = vDemo->npackedNo;
		vDemo_r->endflag = 1;
		renddatasize = sizeof(vconRendDataHead_t);
		P2PSDKVconSend(session, 0, (void *)vDemo_r, renddatasize);

		errorflag = 1;
	}

	// CRC åˆ¤æ–­
	if(0 != vDemo->crc)
	{

	}
	// first
	if(VCON_PACKED_FLAG_FIRST == vDemo->packedFlag)
	{
		packagecount = 0;
		packetsize = vDemo->npackedLen;
		switch(vDemo->FileType)
		{
			case VCON_FILE_TYPE_DEBUG:  // .txt
				{
					snprintf(dst_file_path, sizeof(dst_file_path), "%s", DST_FILE_PATH_DEBUG);
				}
				break;

			case VCON_FILE_TYPE_PCM:    // PCM
				{
					snprintf(dst_file_path, sizeof(dst_file_path), "%s", DST_FILE_PATH_CST_SOUND);
				}
				break;

			case VCON_FILE_TYPE_G711A:  // G711A
				{
					//
				}
				break;

			case VCON_FILE_TYPE_JPEG:  // JPEG
				{
					//
				}
				break;

			case VCON_FILE_TYPE_JNP:  // JNP
				{
					//
				}
				break;

			default:break;
		}

	}
	// åˆ¤æ–­åŒ…æ ‡è¯†
	if(VCON_PACKED_FLAG_REND == vDemo->packedFlag) // é‡å‘åŒ…
	{
		memset(src_file_buf, 0, sizeof(src_file_buf));
		if(1 != errorflag)
		{
			memcpy(src_file_buf, vDemo + 1, vDemo->npackedLen); // å–åç§»é‡ï¼Œåç§»ä¸€ä¸ªç»“æž„ä½“çš„é•¿åº¦ï¼Œ40ä¸ªå­—èŠ‚
		}

		fp_srcfile = fopen(dst_file_path,"rb+");
		if (NULL == fp_srcfile)
		{
			printf("--- %s: Failed to open file of retransmit (NO.%d pack): %s!\n", __FUNCTION__, vDemo->npackedNo, dst_file_path);
			free(vDemo_r);
			vDemo_r = NULL;
			free(src_file_buf);
			src_file_buf = NULL;

			return NULL;
		}

		if(packetsize > vDemo->npackedLen)
		{
            fseek(fp_srcfile, 0, SEEK_END); // é‡å‘åŒ…åˆšå¥½æ˜¯åºå·æœ€å¤§çš„åŒ…
		}
		else
		{
		    fileoffset = vDemo->npackedLen * vDemo->npackedNo;
			fseek(fp_srcfile, fileoffset, SEEK_SET); // è®¾ç½®åç§»é‡
		}

		write_count = fwrite(src_file_buf, 1, vDemo->npackedLen, fp_srcfile);
		if(write_count != vDemo->npackedLen)
		{
			printf("--- %s: Failed to write file of retransmit (NO.%d pack): expected to write: %zu, actually written: %zu!\n", __FUNCTION__, vDemo->npackedNo, sizeof(data), fp_srcfile);

			fileoffset = vDemo->npackedLen * vDemo->npackedNo;
			fseek(fp_srcfile, fileoffset, SEEK_SET);       // è®¾ç½®åç§»é‡
			memset(src_file_buf, 0, sizeof(src_file_buf));  // æ¸…ç©ºæ•°æ®
			fwrite(src_file_buf, 1, vDemo->npackedLen, fp_srcfile);

			vDemo_r->version = 1;
			vDemo_r->FileType = vDemo->FileType;
			vDemo_r->npackedNo = vDemo->npackedNo;
            vDemo_r->endflag = 1;
			renddatasize = sizeof(vconRendDataHead_t);
			P2PSDKVconSend(session, 0, (void *)vDemo_r, renddatasize);

			free(vDemo_r);
			vDemo_r = NULL;
			free(src_file_buf);
			src_file_buf = NULL;

			return NULL;
		}

		if(1 != errorflag)
		{
			packagecount += vDemo->npackedLen;
			// printf("===> packagecount = %d\n", packagecount);
		}
		if((1 != errorflag) && (packagecount == vDemo->nFileLen))
		{
			vDemo_r->version = 1;
			vDemo_r->FileType = vDemo->FileType;
			vDemo_r->npackedNo = vDemo->npackedNo;
			vDemo_r->endflag = 2;
			renddatasize = sizeof(vconRendDataHead_t);
			P2PSDKVconSend(session, 0, (void *)vDemo_r, renddatasize);

			memset(dst_file_path, 0, sizeof(dst_file_path));

			printf("===> The length of the custom file = %d\n", packagecount);

			sleep(3);
			if(IS_FILE_EXIST(DST_FILE_PATH_CST_SOUND))
			{
				printf("===> Ready to play custom sound\n");
				SearchFileAndPlay(SOUND_Alarm_custom, NK_True);
				printf("===> End of the play custom sound\n");
			}
			else
			{
				printf("===> Can't open custom sound\n");
			}
		}

		free(vDemo_r);
		vDemo_r = NULL;
		free(src_file_buf);
		src_file_buf = NULL;

	}

	if((VCON_PACKED_FLAG_CONTINUS == vDemo->packedFlag) || (VCON_PACKED_FLAG_FIRST == vDemo->packedFlag)) // continus || first
	{
		memset(src_file_buf, 0, sizeof(src_file_buf));
		if(1 != errorflag) // åŒ…æ²¡é”™å°±æ­£å¸¸ä¼ è¾“ï¼Œæœ‰é”™çš„è¯å°±å†™0
		{
			memcpy(src_file_buf, vDemo + 1, vDemo->npackedLen); // å–åç§»é‡ï¼Œåç§»ä¸€ä¸ªç»“æž„ä½“çš„é•¿åº¦ï¼Œ40ä¸ªå­—èŠ‚
		}

		if(VCON_PACKED_FLAG_FIRST == vDemo->packedFlag)
		{
			fp_srcfile = fopen(dst_file_path, "w+");
		}
		else
		{
			//fp_srcfile = fopen(dst_file_path, "a+"); // "a+"è®¾ç½®ä¸äº†åç§»é‡
			fp_srcfile = fopen(dst_file_path, "rb+");
		}

		if (NULL == fp_srcfile)
		{
			printf("--- %s: Failed to open file (NO.%d pack): %s!\n", __FUNCTION__, vDemo->npackedNo, dst_file_path);
			free(vDemo_r);
			vDemo_r = NULL;
			free(src_file_buf);
			src_file_buf = NULL;

			return NULL;
		}

		fileoffset = vDemo->npackedLen * vDemo->npackedNo;
		fseek(fp_srcfile, fileoffset, SEEK_SET);       // è®¾ç½®åç§»é‡
		write_count = fwrite(src_file_buf, 1, vDemo->npackedLen, fp_srcfile);
		if(write_count != vDemo->npackedLen)
		{
			printf("--- %s: Failed to write file (NO.%d pack): expected to write: %zu, actually written: %zu!\n", __FUNCTION__, vDemo->npackedNo, sizeof(data), fp_srcfile);

			fileoffset = vDemo->npackedLen * vDemo->npackedNo;
			fseek(fp_srcfile, fileoffset, SEEK_SET);       // è®¾ç½®åç§»é‡
			memset(src_file_buf, 0, sizeof(src_file_buf));  // æ¸…ç©ºæ•°æ®
			fwrite(src_file_buf, 1, vDemo->npackedLen, fp_srcfile);

			vDemo_r->version = 1;
			vDemo_r->FileType = vDemo->FileType;
			vDemo_r->npackedNo = vDemo->npackedNo;
			vDemo_r->endflag = 1;
			renddatasize = sizeof(vconRendDataHead_t);
			P2PSDKVconSend(session, 0, (void *)vDemo_r, renddatasize);

            free(vDemo_r);
			vDemo_r = NULL;
			free(src_file_buf);
			src_file_buf = NULL;

			return NULL;
		}

		if(1 != errorflag)
		{
			packagecount += vDemo->npackedLen;
			// printf("===> packagecount = %d\n", packagecount);
		}

		free(vDemo_r);
		vDemo_r = NULL;
		free(src_file_buf);
		src_file_buf = NULL;
	}

	if(VCON_PACKED_FLAG_END == vDemo->packedFlag) // end
	{
		memset(src_file_buf, 0, sizeof(src_file_buf));
		if(1 != errorflag)
		{
			memcpy(src_file_buf, vDemo + 1, vDemo->npackedLen); // å–åç§»é‡ï¼Œåç§»ä¸€ä¸ªç»“æž„ä½“çš„é•¿åº¦ï¼Œ40ä¸ªå­—èŠ‚
		}

		fp_srcfile = fopen(dst_file_path, "rb+");
		if (NULL == fp_srcfile)
		{
			printf("--- %s: Failed to open file (NO.%d end pack): %s!\n", __FUNCTION__, vDemo->npackedNo, dst_file_path);
			free(vDemo_r);
			vDemo_r = NULL;
			free(src_file_buf);
			src_file_buf = NULL;

			return NULL;
		}

		//fileoffset = vDemo->npackedLen * vDemo->npackedNo;
		fseek(fp_srcfile, 0, SEEK_END);       // è®¾ç½®åç§»é‡
		write_count = fwrite(src_file_buf, 1, vDemo->npackedLen, fp_srcfile);
		if(write_count != vDemo->npackedLen)
		{
			printf("--- %s: Failed to write sound file (NO.%d end pack): expected to write: %zu, actually written: %zu!\n", __FUNCTION__, vDemo->npackedNo, vDemo->npackedLen, write_count);

			fileoffset = vDemo->npackedLen * vDemo->npackedNo;
			fseek(fp_srcfile, fileoffset, SEEK_SET);       // è®¾ç½®åç§»é‡
			memset(src_file_buf, 0, sizeof(src_file_buf));  // æ¸…ç©ºæ•°æ®
			fwrite(src_file_buf, 1, vDemo->npackedLen, fp_srcfile);

			vDemo_r->version = 1;
			vDemo_r->FileType = vDemo->FileType;
			vDemo_r->npackedNo = vDemo->npackedNo;
			vDemo_r->endflag = 1;
			renddatasize = sizeof(vconRendDataHead_t);
			P2PSDKVconSend(session, 0, (void *)vDemo_r, renddatasize);

			free(vDemo_r);
			vDemo_r = NULL;
			free(src_file_buf);
			src_file_buf = NULL;

			return NULL;
		}

		if(1 != errorflag)
		{
			packagecount += vDemo->npackedLen;
			// printf("===> packagecount = %d\n", packagecount);
		}
		if((1 != errorflag) && (packagecount == vDemo->nFileLen))
		{
			vDemo_r->version = 1;
			vDemo_r->FileType = vDemo->FileType;
			vDemo_r->npackedNo = vDemo->npackedNo;
			vDemo_r->endflag = 2;
			renddatasize = sizeof(vconRendDataHead_t);
			P2PSDKVconSend(session, 0, (void *)vDemo_r, renddatasize);

			memset(dst_file_path, 0, sizeof(dst_file_path));

			printf("===> The length of the custom file = %d\n", packagecount);

			sleep(3);
			if(IS_FILE_EXIST(DST_FILE_PATH_CST_SOUND))
			{
				printf("===> Ready to play custom sound\n");
				SearchFileAndPlay(SOUND_Alarm_custom, NK_True);
				printf("===> End of the play custom sound\n");
			}
			else
			{
				printf("===> Can't open custom sound\n");
			}
		}

		free(vDemo_r);
		vDemo_r = NULL;
		free(src_file_buf);
		src_file_buf = NULL;
	}

	fclose(fp_srcfile);
}

#if defined(DANA_P2P)
int OnSmartlinkSetWifi(void *ctx, const char *ssid, const char *password)
{
    ST_NSDK_NETWORK_INTERFACE wlan;

    if(ssid && password) {
        NETSDK_conf_interface_get(4, &wlan);
        if(strcmp(ssid, wlan.wireless.wirelessStaMode.wirelessApEssId) == 0 && 
        strcmp(password, wlan.wireless.wirelessStaMode.wirelessApPsk) == 0 &&
        wlan.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_STATIONMODE) {
            APP_TRACE("same essid (%s) (%s) <-> (%s) (%s)", ssid, password,
            wlan.wireless.wirelessStaMode.wirelessApEssId, wlan.wireless.wirelessStaMode.wirelessApPsk);
            return 0;
        }
        APP_TRACE("setup essid(%s) psk(%s)", ssid, password);
        snprintf(wlan.wireless.wirelessStaMode.wirelessApEssId,
        sizeof(wlan.wireless.wirelessStaMode.wirelessApEssId),"%s",
        ssid);
        snprintf(wlan.wireless.wirelessStaMode.wirelessApPsk, 
        sizeof(wlan.wireless.wirelessStaMode.wirelessApPsk), "%s",
        password);
        wlan.wireless.wirelessMode = NSDK_NETWORK_WIRELESS_MODE_STATIONMODE;
        wlan.wireless.dhcpServer.dhcpAutoSettingEnabled = true;
        wlan.lan.addressingType = kNSDK_NETWORK_LAN_ADDRESSINGTYPE_DYNAMIC;

        SearchFileAndPlay(SOUND_WiFi_setting, NK_False);
        SearchFileAndPlay(SOUND_Please_wait, NK_False);

        NETSDK_conf_interface_set(4, &wlan, eNSDK_CONF_SAVE_RESTART);

        return 0;
    }
    return -1;
}

#define P2P_UPGRADE_ROM    "/media/tf/upgrade_danale.rom"

typedef struct DANA_ATTR
{
    pthread_t upgrade_tid;
    pthread_t cloud_tid;
    bool cloud_trigger;
    bool motion_check;
}stDANA_ATTR;
static stDANA_ATTR dana_attr = {
    .upgrade_tid = (pthread_t)NULL,
    .cloud_tid = (pthread_t)NULL,
    .cloud_trigger = false,
    .motion_check = false,
};

/**
 * @param ctx               ç”¨æˆ·ä¸Šä¸‹æ–‡
 * @param rom_size          å›ºä»¶æ–‡ä»¶å¤§å°
 * @param rom_ver           å›ºä»¶ç‰ˆæœ¬
 * @param save_pathname     å›ºä»¶å­˜æ”¾çš„è·¯å¾„ï¼Œè¾“å‡ºå‚æ•°
 *                          æˆåŠŸ  è¿”å›ž0
 *                          å¤±è´¥  å…¶ä»–ä»£ç 
 */
uint32_t OnNewRomCome(void *ctx, const uint64_t rom_size, const char *rom_ver, char save_pathname[512])
{
    if(!NK_TFCARD_is_mounted()) {
        APP_TRACE("TFcard not mounted!");
        return -1;
    }

    sprintf(save_pathname, "%s", P2P_UPGRADE_ROM);
    APP_TRACE("New rom version %s(size %d)", rom_size, rom_size);

    return 0;

}

/**
 * @param ctx               ç”¨æˆ·ä¸Šä¸‹æ–‡
 * @param code              çŠ¶æ€ç ï¼Œ0æˆåŠŸ
 * @param rom_pathname      å›ºä»¶å­˜æ”¾è·¯å¾„
 */
void OnRomDownloadComplete(void *ctx, const uint32_t code, const char *rom_pathname)
{
    APP_TRACE("Download rom path name:%s(code %d)", rom_pathname, code);

}

static void *p2p_upgradeRom(void *arg)
{
    char *rom_pathname = (char *)arg;

    prctl(PR_SET_NAME, "p2p_upgradeRom");

    APP_TRACE("upgrade rom start");

    APP_UPGRADE_start(rom_pathname, false);

    dana_attr.upgrade_tid = (pthread_t)NULL;
    pthread_exit(NULL);

}

/**
 * @param ctx               ç”¨æˆ·ä¸Šä¸‹æ–‡
 * @param rom_pathname      å›ºä»¶å­˜æ”¾è·¯å¾„
 * @param upgrade_timeout   å›ºä»¶æ›´æ–°è¶…æ—¶ï¼Œå•ä½ç§’
 */
void OnUpgradeConfirm(void *ctx, const char *rom_pathname, uint32_t *upgrade_timeout)
{
    if(!dana_attr.upgrade_tid) {
        if(pthread_create(&dana_attr.upgrade_tid, NULL, p2p_upgradeRom, (void *)rom_pathname) == 0) {
            *upgrade_timeout = 300;   // 5åˆ†é’Ÿ
        }
        else {
            APP_TRACE("Create pthread failed.p2p upgrade rom failed!");
        }
    }
    else {
        APP_TRACE("Upgrade confirm failed");
    }

}

// get current monotonic ms. p_cur_ms can't be NULL
static int get_cur_mono_ms(int64_t *p_cur_ms)
{
    int ret;
    struct timespec tp;

    ret = clock_gettime(CLOCK_MONOTONIC, &tp);
    if (ret != 0) {
        APP_TRACE("%s: clock_gettime failed! ret: %d, errno: %d",
                  __FUNCTION__,
                  ret,
                  errno);
        return -1;
    } else {
        *p_cur_ms = ((int64_t)tp.tv_sec) * 1000 + ((int64_t)tp.tv_nsec) / 1000000;
        return 0;
    }
}

void p2p_fillRecordFrameHead(unsigned int recordType, kp2p_frame_head_t *pFrameHead, const stSDK_ENC_BUF_ATTR *attr)
{
    pFrameHead->magic = P2PSDK_FRAMEHEAD_MAGIC;
    pFrameHead->version = P2PSDK_FRAMEHEAD_VERSION;
    pFrameHead->headtype = P2P_FRAMEHEAD_HEADTYPE_REPLAY;
    pFrameHead->framesize = attr->data_sz;
    pFrameHead->ts_ms = attr->time_us / 1000;

    pFrameHead->replay.type = recordType;
    pFrameHead->replay.channel = 0;
    pFrameHead->replay.quality = 0;

    if(attr->type == SDK_ENC_BUF_DATA_H264) {
        pFrameHead->replay.frametype = (attr->h264.keyframe ? 1 : 2);
        snprintf(pFrameHead->replay.v.enc, sizeof(pFrameHead->replay.v.enc), "%s", "H264");
        pFrameHead->replay.v.fps = attr->h264.fps;
        pFrameHead->replay.v.width = attr->h264.width;
        pFrameHead->replay.v.height = attr->h264.height;
    }
    else if(attr->type == SDK_ENC_BUF_DATA_H265) {
        pFrameHead->replay.frametype = (attr->h264.keyframe ? 1 : 2);
        snprintf(pFrameHead->replay.v.enc, sizeof(pFrameHead->replay.v.enc), "%s", "H265");
        pFrameHead->replay.v.fps = attr->h264.fps;
        pFrameHead->replay.v.width = attr->h264.width;
        pFrameHead->replay.v.height = attr->h264.height;
    }
    else if(attr->type == SDK_ENC_BUF_DATA_G711A) {
        pFrameHead->replay.frametype = 0;
        snprintf(pFrameHead->replay.a.enc, sizeof(pFrameHead->replay.a.enc), "%s", "G711A");
        pFrameHead->replay.a.samplerate = attr->g711a.sample_rate;
        pFrameHead->replay.a.samplewidth = attr->g711a.sample_width;
        pFrameHead->replay.a.channels = 1;
        pFrameHead->replay.a.compress = attr->g711a.compression_ratio;
    }
    else if(attr->type == SDK_ENC_BUF_DATA_AAC) {
        pFrameHead->replay.frametype = 0;
        snprintf(pFrameHead->replay.a.enc, sizeof(pFrameHead->replay.a.enc), "%s", "AAC");
        pFrameHead->replay.a.samplerate = attr->g711a.sample_rate;
        pFrameHead->replay.a.samplewidth = attr->g711a.sample_width;
        pFrameHead->replay.a.channels = 1;
        pFrameHead->replay.a.compress = attr->g711a.compression_ratio;
    }
    else {
        APP_TRACE("unknow frameHead.codec = %d!", attr->type);
    }

}

void *cloudRecord(void *arg)
{
#define DANA_CLOUD_REC_UPLOAD_TIMEOUT_US    (5 * 1000 * 1000)
#define DANA_CLOUD_REC_MOTION_TIME_S   10

    lpMEDIABUF_USER  mediabuf_user = NULL;
    size_t data_sz = 0;
	int ch_num = 0;
	int stream_num = 0;
	char stream_name[64] = {0};
    int mediabuf_id = 0;

    int64_t motion_cur_ms = 0;
    int64_t motion_end_ms = 0;

    bool in_motion = false;

    APP_TRACE("DANALE cloud record start");
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "cloudRecord");

    snprintf(stream_name, sizeof(stream_name), "ch%d_%d.264", ch_num, stream_num);

    do {
    	printf("Force AttachStream: %s\n", stream_name);
        mediabuf_id = MEDIABUF_lookup_byname(stream_name);
        if (mediabuf_id < 0) {
            printf("DeviceAttachStream failed:lookup stream %s\n", stream_name);
        }
        mediabuf_user = MEDIABUF_attach(mediabuf_id);
        if(NULL == mediabuf_user) {
            printf("DeviceAttachStream: mediabuf full!\n");
            msleep(10);
        }
    }while(NULL == mediabuf_user);

    while(dana_attr.cloud_trigger == true) {
        if(0 == MEDIABUF_out_try_lock(mediabuf_user)) {
            const stSDK_ENC_BUF_ATTR *attr = NULL;
            if(0 == MEDIABUF_out(mediabuf_user, (void**)&attr, NULL, &data_sz)) {
                void *raw_data = (void*)(attr + 1);
                data_sz = attr->data_sz;
                kp2p_frame_head_t pFrameHead;
                int recType = P2P_REC_TYPE_TIME;

                if(dana_attr.motion_check == true) {
                    dana_attr.motion_check = false;
                    get_cur_mono_ms(&motion_cur_ms);
                    motion_end_ms = motion_cur_ms + 1000 * DANA_CLOUD_REC_MOTION_TIME_S;
                    in_motion = true;
                }
                else if(in_motion) {
                    get_cur_mono_ms(&motion_cur_ms);
                }

                if(in_motion && motion_cur_ms < motion_end_ms) {
                    recType = P2P_REC_TYPE_MOTION;
                }
                else {
                    recType = P2P_REC_TYPE_TIME;
                    in_motion = false;
                }

                p2p_fillRecordFrameHead(recType, &pFrameHead, attr);
                P2PSDKCloudRecordUpload(&pFrameHead, recType, raw_data, data_sz, DANA_CLOUD_REC_UPLOAD_TIMEOUT_US);
            }
            else
            {
                msleep(10);
                continue;
            }

            MEDIABUF_out_unlock(mediabuf_user);
        }
        else
        {
            msleep(10);
            printf("DeviceReadFrame failed: mediabuf lock err, user:%p\n", mediabuf_user);
            continue;
        }
    }

    return NULL;
}

void OnCloudRecordStart(void *ctx, uint32_t channel)
{
    if(NULL == dana_attr.cloud_tid) {
        dana_attr.cloud_trigger = true;
        if(pthread_create(&dana_attr.cloud_tid, NULL, cloudRecord, NULL) != 0) {
            APP_TRACE("cloud record pthread create failed!");
            dana_attr.cloud_trigger = false;
        }
    }

}

void P2P_danaRecordMotion(bool motion)
{
    dana_attr.motion_check = motion;

}

#endif

void *P2PDeviceStart(void *arg)
{
	if(NULL == arg)
	{
		printf("P2PStart create DeviceCtx failed\n");
		return NULL;
	}
	struct P2PDeviceDemo *pDemo = (struct P2PDeviceDemo *)arg;

	//P2PSDKSetLogPath("/media/tf/p2p/log/p2p.log");
	//PSDKSetLogLevel(P2PSDK_EMERG);

	struct P2PSDKDevice device = { 0 };
    snprintf(device.info.sn, sizeof(device.info.sn), "%s", pDemo->serialNo);
    snprintf(device.info.model, sizeof(device.info.model), "%s", pDemo->model);
    device.info.install_type = pDemo->install_type;
    snprintf(device.info.version, sizeof(device.info.version), "%s", pDemo->version);
	snprintf(device.info.hwcode, sizeof(device.info.hwcode), "%s", pDemo->hwcode);
    snprintf(device.info.vendor, sizeof(device.info.vendor), "%s", pDemo->vendor);
	device.info.cloud_record = pDemo->cloud_record;
	snprintf(device.info.area, sizeof(device.info.area), pDemo->area);
    device.info.max_ch = pDemo->max_ch;
    device.ctx = pDemo;

    device.OnAuth = OnAuth;
	device.OnGetNonce = OnGetNonce;
	device.OnAuth2 = OnAuth2;
    device.OnPtzCtrl = OnPtzCtrl;
    //device.OnSettingsReadStreamInfo = OnSettingsReadStreamInfo;
    device.OnAttachStream = OnAttachStream;
    device.OnDetachStream = OnDetachStream;
    device.OnReadFrame = OnReadFrameEx;
	//device.OnReadFrameEx = OnReadFrameEx;
	device.OnAfterReadFrame = OnAfterReadFrame;
	device.OnDevOnline = OnDevOnline;
	//device.OnDevConnectReq = NULL;
#if 0
    device.OnFetchRecList = OnFetchRecList;//OnFetchRecListNew;
    device.OnOpenRecFile = OnOpenRecFiles;
	device.OnOpenRecFile2 = NULL;//OnOpenRecFile2;
    device.OnCloseRecFile = OnCloseRecFiles;//OnCloseRecFilesNew;
    device.OnReadRecFrame = OnReadRecFrame;//OnReadRecFrameNew;
#else
	device.OnFetchRecList = OnFetchRecListNew;
    device.OnOpenRecFile = NULL;
	device.OnOpenRecFile2 = OnOpenRecFile2;
    device.OnCloseRecFile = OnCloseRecFilesNew;
    device.OnReadRecFrame = OnReadRecFrameNew;

#endif
	device.OnAfterReadRecFrame = OnAfterReadRecFrame;
	device.OnFindFileStart = OnFindFileStart;
	device.OnFindNextFile = OnFindNextFile;
	device.OnFindFileStop = OnFindFileStop;
    device.OnRemoteSetup = OnRemoteSetup;
	//device.OnBindwidthChanged = OnBindwidthChanged;
	device.OnBandwidthChanged = OnBandwidthChanged;
	device.OnConnected = OnConnected;
	device.OnClosed = OnClosed;

    device.OnVoP2PCall = OnAuP2PCall;
    device.OnVoP2PTalkRecv = OnAuP2PTalkRecv;
    device.OnVoP2PHangup = OnAuP2PHangup;
	device.OnPullAlarmMsg = OnPullAlarmMsg;
	device.OnForceIFrame = OnForceIFrame;
#if !defined(DANA_P2P)
    device.OnVconRecv = OnVconRecv;
    //if(NK_TFCARD_is_mounted()) {
        //P2PSDKSetLogPath2("/media/tf", 1024 * 10000);
    //}
#else
    if(NK_TFCARD_is_mounted()) {
        P2PSDKSetLogPath("/media/tf", 1024 * 10000);
    }
#endif

#if defined(DANA_P2P)
    device.OnSmartlinkSetWifi = OnSmartlinkSetWifi;
    device.OnNewRomCome = OnNewRomCome;
    device.OnRomDownloadComplete = OnRomDownloadComplete;
    device.OnUpgradeConfirm = OnUpgradeConfirm;
    device.OnCloudRecordStart = OnCloudRecordStart;
    device.info.type = KP2P_DEVICE_IPC;
#endif

#if defined(DANA_P2P)
#define DANALE_CONF_PATH    "/media/conf/"
    char agent_user[16] = "venkee";
    char agent_pass[64] = "_1venkee2_";
    char chip_code[8] = "H18E";
    char scheme_code[8] = "HXWS";
    char product_code[16] = "1000B1";
    ST_PRODUCT_TEST_INFO product_info;

    if(device.info.install_type == eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL) {
        snprintf(product_code, sizeof(product_code), "%s", "1000B1");
	}
	else if(device.info.install_type == eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL) {
        snprintf(product_code, sizeof(product_code), "%s", "10009Y");
	}
    P2PSDKInit2(&device, DANALE_CONF_PATH, agent_user, agent_pass, chip_code, scheme_code, product_code);
    if(pDemo->apMode == 1) {
        if(NULL == PRODUCT_TEST_get_info(&product_info)) {
            P2PSDKSmartlinkStart("wlan0");
        }
    }
#else

    APP_TRACE("KP2P device capabilities %s", pDemo->capabilities);
    P2PSDKInitV2(&device, pDemo->capabilities);
#endif
//    P2PSDKSetEseeServerAddr("115.28.0.173", 60101);
//    P2PSDKSetEseeServerAddr("113.105.223.77", 60101);

	//if(NULL != MODEL_CONF_get(&model_conf)){
	P2PSDKSetCamDes(0, device.info.model);
	//}

	ST_CUSTOM_SETTING custom;
	if((0 == CUSTOM_get(&custom)) && (true == CUSTOM_check_string_valid(custom.function.p2pServerIp))){
#if !defined(DANA_P2P)
		P2PSDKSetEseeServerAddr(custom.function.p2pServerIp, 60101);
#endif
	}

    APP_TRACE("P2PSDK version:%s", P2PSDKGetVersion());
	return 0;
}
#if 0
/* P2PDevice TEst!!! */
void OnDevOnline(const char *eseeid, void *ctx)
{
    struct P2PDeviceDemo *pDemo = (struct P2PDeviceDemo*)ctx;
    snprintf(pDemo->eseeId, sizeof(pDemo->eseeId), "%s", eseeid);
    printf("Device %s logined Server!!!\n", pDemo->eseeId);
}
#endif

int SpookAddrGenerator(struct sockaddr *app_service_addr)
{
    struct sockaddr_in* pAddr = (struct sockaddr_in*)app_service_addr;
    pAddr->sin_family = AF_INET;
    pAddr->sin_addr.s_addr = inet_addr("192.168.28.5");
    pAddr->sin_port = htons(80);
    return 0;
}

int P2P_sdkdestroy()
{
	return P2PSDKDeinit();

}


int P2PStart(const char *sn, const char *version, const char *vend, const char *hwcode)
{
	// init p2pDevice
	static struct P2PDeviceDemo Pdevice;
	pthread_t P2Ptid;
	int result=-1;
	ST_MODEL_CONF model_conf;
    ST_NSDK_SYSTEM_SETTING sInfo;
    ST_NSDK_NETWORK_INTERFACE n_interface;

    memset(&Pdevice, 0, sizeof(struct P2PDeviceDemo));

#if defined(ADT)
	char tutk_uid[33];

	if (!SECURE_CHIP_is_init_success()) {
		printf("%s:%d Secure chip has not inited, P2P not start.\n",
			   __FUNCTION__, __LINE__);
		return -1;
	}

	if (0 != SECURE_CHIP_get_data(SECURE_CHIP_DATA_UID, tutk_uid, sizeof(tutk_uid))) {
		printf("%s:%d Get tutk uid from secure chip failed, P2P not start.\n",
			   __FUNCTION__, __LINE__);
		return -1;
	}

	snprintf(Pdevice.serialNo, sizeof(Pdevice.serialNo), "%s", tutk_uid);
#else
    snprintf(Pdevice.serialNo, sizeof(Pdevice.serialNo), "%s", sn);
#endif
	snprintf(Pdevice.version, sizeof(Pdevice.version), "%s", version);
	snprintf(Pdevice.hwcode, sizeof(Pdevice.hwcode), "%s", hwcode);
    snprintf(Pdevice.vendor, sizeof(Pdevice.vendor), "%s", vend);
	if(NULL != MODEL_CONF_get(&model_conf)) {
		snprintf(Pdevice.model, sizeof(Pdevice.model), "%s", model_conf.modelName);
		if(model_conf.fixmode == eNSDK_IMAGE_FISHEYE_FIX_MODE_WALL) {
			Pdevice.install_type = 0;
		}
		else if(model_conf.fixmode == eNSDK_IMAGE_FISHEYE_FIX_MODE_CELL) {
			Pdevice.install_type = 1;
		}
        else if(model_conf.fixmode == eNSDK_IMAGE_FISHEYE_FIX_MODE_TABLE) {
			Pdevice.install_type = 2;
		}
		else {
			Pdevice.install_type = 0;
		}
	}
	else {
		snprintf(Pdevice.model, sizeof(Pdevice.model), "IPCAM");
		Pdevice.install_type = 0;
	}
    Pdevice.cloud_record = 0;

    NETSDK_conf_system_get_setting_info(&sInfo);
    if(!strncmp(sInfo.area, "default", 7)) {
        snprintf(Pdevice.area, sizeof(Pdevice.area), "");
    }
    else {
        snprintf(Pdevice.area, sizeof(Pdevice.area), sInfo.area);
    }
    Pdevice.max_ch = 1;
	Pdevice.OnDevOnline = OnDevOnline;
	Pdevice.ctx = &Pdevice;

    NETSDK_conf_interface_get(4, &n_interface);//wlan
    if(n_interface.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT) {
        Pdevice.apMode = 1;
    }
    else {
        Pdevice.apMode = 0;
    }

    struct json_object *arrObj = json_object_new_array();
    if(NULL != arrObj)
    {
        struct json_object *tmpObj = NULL;
#if defined(CLOUD_REC)
#if defined(HI3516E_V1) // åŽé¢ä½¿ç”¨è®¾å¤‡é…ç½®çš„èƒ½åŠ›é›†åˆ¤æ–­
        tmpObj = json_object_new_int(KP2P_DEV_CAPABILITY_CLOUD_RECORD_ALY);    // é˜¿é‡Œäº‘å­˜
        if(NULL != tmpObj)
        {
            json_object_array_add(arrObj, tmpObj);
        }
#endif
#endif
        snprintf(Pdevice.capabilities, sizeof(Pdevice.capabilities), "%s", json_object_to_json_string(arrObj));
        json_object_put(arrObj);
    }
    else
    {
        snprintf(Pdevice.capabilities, sizeof(Pdevice.capabilities), "[]");
    }

    printf("P2PDeviceDemo version:%s\n", Pdevice.version);

	result = pthread_create(&P2Ptid, NULL, P2PDeviceStart, (void *)&Pdevice);
	if(0 == result)
	{
		printf("P2P pthread start sucess\n");
	}
	else
	{
		printf("P2P pthread create failed\n");
		return -1;	
	}

#if defined(DANA_P2P)  // danaä¸å¯åŠ¨IOTDaemon
    char str[64];
    memset(str, '\0', sizeof(str));
    sprintf(str, "killall daemon_start.sh iot.Daemon");
    NK_SYSTEM(str);
#endif

	return 0;
}

/*
* @brief  å¤§æ‹¿æŠ¥è­¦æŽ¨é€
* @return æˆåŠŸ  å‘é€å‡ºåŽ»çš„æ•°æ®é•¿åº¦
*         å¤±è´¥    -1
* å¤‡æ³¨:æŽ¥å£å‚æ•°åŽé¢å†æ ¹æ®éœ€æ±‚åšè°ƒæ•´
*/
#ifdef DANA_P2P
int P2P_danaAlarmMsgSend()
{
    int ret;
    kp2p_frame_head_t pFrameHead;
    uint64_t time_ms = 0;
	struct timeval tv;
    char alarm_msg[32] = {0};

    memset(&pFrameHead, 0, sizeof(kp2p_frame_head_t));

	gettimeofday(&tv, NULL);
	time_ms = tv.tv_sec;
	time_ms *= 1000;
	time_ms += tv.tv_usec / 1000;
    pFrameHead.ts_ms = time_ms;
    pFrameHead.alarm.channel = 0;

    sprintf(alarm_msg, "%s", "motion detection");

    ret = P2PSDKSendAlarmMsg(NULL, &pFrameHead, (void *)alarm_msg, strlen(alarm_msg));
    if(ret > 0) {
        APP_TRACE("P2PSDKSendAlarmMsg success");
    }
    else {
        APP_TRACE("P2PSDKSendAlarmMsg failed");
    }

    return ret;

}
#endif
