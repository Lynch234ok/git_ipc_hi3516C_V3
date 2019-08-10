#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "minirtsp.h"
#include "media_buf.h"
#include "netsdk.h"
#include "sdk/sdk_enc.h"
#include "usrm.h"
#include "../netsdk_def.h"
#include "global_runtime.h"
#include "base/ja_process.h"

static lpMINIRTSP rtsp = NULL;

typedef struct STREAM_PRIV_CTX
{
	unsigned long long sys_ts;
	char stream_name[64];
	int media_id;
	lpMEDIABUF_USER media_user;
	uint32_t media_speed;
	unsigned long long initUsPts;
}stSTREAM_PRIV_CTX, *lpSTREAM_PRIV_CTX;

int Minirtsp_Server_Get_Venc(char* stream_name,VENC_t* channal_venc){
	printf("liry rtsp_v2 get venc %s \n",stream_name);
	if((NULL == stream_name)||(NULL == channal_venc))
		return -1;
	ST_NSDK_VENC_CH venc_ch;
	ST_NSDK_AENC_CH aenc_ch;
	char* ptr_name = NULL;
	if(ptr_name ==NULL){
		ptr_name = strstr(stream_name,"ch0_0.264");
	}
	if(ptr_name ==NULL){
		ptr_name = strstr(stream_name,"ch1/main/av_stream");
	}
	if(ptr_name ==NULL){
		ptr_name = strstr(stream_name,"ch0_1.264");
	}
	if(ptr_name ==NULL){
		ptr_name = strstr(stream_name,"ch1/sub/av_stream");
	}
	if(ptr_name ==NULL){
		strcpy(stream_name,"ch0_0.264");
		ptr_name = stream_name;
	}

	if(strncmp(ptr_name,"ch0_1.264",sizeof("ch0_1.264")) ==0 || strncmp(ptr_name,"ch1/sub/av_stream",sizeof("ch1/sub/av_stream")) ==0 ){
		NETSDK_conf_venc_ch_get(102,&venc_ch);
	}else{
		NETSDK_conf_venc_ch_get(101,&venc_ch)	;
	}	

	//暂时屏蔽265+类型判断
	//channal_venc->codeType =  ((kNSDK_CODEC_TYPE_H265 == venc_ch.codecType) || (kNSDK_CODEC_TYPE_H265_VIF == venc_ch.codecType)) ? HT_TYPE_H265 : HT_TYPE_H264;
	channal_venc->codeType =  (kNSDK_CODEC_TYPE_H265 == venc_ch.codecType) ? HT_TYPE_H265 : HT_TYPE_H264;
	if (venc_ch.freeResolution == true) {
		channal_venc->resWidth = venc_ch.resolutionWidth;
		channal_venc->resHeight =  venc_ch.resolutionHeight;
	} else {
		channal_venc->resWidth = (venc_ch.resolution >> 16) & 0xffff;
		channal_venc->resHeight =  venc_ch.resolution & 0xffff;
	}

	NETSDK_conf_aenc_ch_get(101, &aenc_ch);

	channal_venc->enaudio = aenc_ch.enabled;
	channal_venc->frameRate = venc_ch.frameRate;
	channal_venc->audiotype = MINIRTSP_MD_TYPE_ALAW;
	channal_venc->rate = 8000;
	channal_venc->channel = 1;

	return 0;
}

int _MINIRTSP_SERVER_SET_IFRAME_(char *stream_name,int code ,void* data){
	if(NULL == stream_name){
		return -1;
	}
	char* ptr_name = NULL;
	if(ptr_name ==NULL){
			ptr_name = strstr(stream_name,"ch0_0.264");
	}
	if(ptr_name ==NULL){
		ptr_name = strstr(stream_name,"ch1/main/av_stream");
	}
	if(ptr_name ==NULL){
		ptr_name = strstr(stream_name,"ch0_1.264");
	}
	if(ptr_name ==NULL){
		ptr_name = strstr(stream_name,"ch1/sub/av_stream");
	}
	if(ptr_name ==NULL){
		strcpy(stream_name,"ch0_0.264");
		ptr_name = stream_name;
	}

	if(strncmp(ptr_name,"ch0_1.264",sizeof("ch0_1.264"))==0 || strncmp(ptr_name,"ch1/sub/av_stream",sizeof("ch1/sub/av_stream"))==0){
		SDK_ENC_request_stream_keyframe(0, 1);
	}else{
		SDK_ENC_request_stream_keyframe(0, 0);
	}

	return rMINIRTSP_OK;
}

int _MINIRTSP_SERVER_GET_PWD_BY_NAME_(const char *name, char *ret_pwd, int ret_size){
	if(NULL == name || NULL == ret_pwd){
		return -1;
	}
	return USRM_get_password(name, ret_pwd, ret_size);
}

extern int SDK_ENC_get_enc_pts(int vin, int stream, unsigned long long *encPts);
int _stream_init(lpMINIRTSP_STREAM s,const char *media_name)
{
	int ret = rMINIRTSP_OK;
	lpSTREAM_PRIV_CTX ctx = NULL;
	ctx = (lpSTREAM_PRIV_CTX)calloc(1, sizeof(stSTREAM_PRIV_CTX));
	if(NULL == ctx){
		return -1;
	}
	printf("liry  _stream_init \n");
	char name_media[128] ={0};
	if((strstr(media_name,"ch0_0.264") !=NULL) ||(strstr(media_name,"ch1/main/av_stream")!=NULL)){
		 strcpy(name_media,"ch0_0.264");
	}else if((strstr(media_name,"ch0_1.264") !=NULL) ||(strstr(media_name,"ch1/sub/av_stream")!=NULL)){
		 strcpy(name_media,"ch0_1.264");
	}else {
		 strcpy(name_media,"ch0_0.264");
		 printf("liry RTSP_V2 init  media_name error %s  \n",media_name);
	}

    // 无线单品打开码流启用nvr兼容模式,关闭跳帧参考,gop改为2秒
#if defined(CX)
    if(false == GLOBAL_sn_front()) {
        ST_NSDK_VENC_CH venc_ch;
        NETSDK_conf_venc_ch_get(101, &venc_ch);
        if(venc_ch.ImageTransmissionModel != eNSDK_COMPATIBILITY_MODE) {
            venc_ch.ImageTransmissionModel = eNSDK_COMPATIBILITY_MODE;
            NETSDK_conf_venc_ch_set(101, &venc_ch);
        }
        NETSDK_conf_venc_ch_get(102, &venc_ch);
        if(venc_ch.ImageTransmissionModel != eNSDK_COMPATIBILITY_MODE) {
            venc_ch.ImageTransmissionModel = eNSDK_COMPATIBILITY_MODE;
            NETSDK_conf_venc_ch_set(102, &venc_ch);
        }
    }
#endif

    /* 打开码流关闭有线和无线dhcp */
    ST_NSDK_NETWORK_INTERFACE lan0, wlan0;

    NETSDK_conf_interface_get(1, &lan0);
    NETSDK_conf_interface_get(4, &wlan0);

    if(lan0.lan.addressingType != kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC)
    {
        lan0.lan.addressingType = kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC;
        NETSDK_conf_interface_set(1, &lan0, eNSDK_CONF_SAVE_JUST_SAVE);
        NK_SYSTEM("kill -9 `pidof udhcpc`");
    }
    if(wlan0.wireless.dhcpServer.dhcpAutoSettingEnabled == true)
    {
        wlan0.wireless.dhcpServer.dhcpAutoSettingEnabled = false;
        NETSDK_conf_interface_set(4, &wlan0, eNSDK_CONF_SAVE_JUST_SAVE);
        NK_SYSTEM("kill -9 `pidof udhcpc`");
    }

	memset(ctx,0,sizeof(stSTREAM_PRIV_CTX));
	ctx->media_id = MEDIABUF_lookup_byname(name_media);
	if(ctx->media_id >= 0){
		ctx->media_user = MEDIABUF_attach(ctx->media_id);
		SDK_ENC_get_enc_pts(0, ctx->media_id, &(ctx->initUsPts));

		if(ctx->media_user){
			ctx->media_speed = MEDIABUF_in_speed(ctx->media_id);
			MEDIABUF_sync(ctx->media_user);

		}
		else
		{
			printf("media attach falied!\n");
			ret = rMINIRTSP_FAIL;
		}
	}else{
		printf("lookup name failed falied!\n");
		ret = rMINIRTSP_FAIL;
	}
	if(rMINIRTSP_OK != ret){
		free(ctx);
		ctx = NULL;
	}
	s->param = ctx;
	printf("liry  _stream_init  end %d\n",ret);
	return ret;
}

int _stream_lock(lpMINIRTSP_STREAM s)
{
	if(NULL == s){
		return rMINIRTSP_FAIL;
	}
	lpSTREAM_PRIV_CTX ctx = (lpSTREAM_PRIV_CTX)s->param;
	if(ctx){
		return MEDIABUF_out_lock(ctx->media_user);
	}
	return rMINIRTSP_FAIL;
}

int _stream_unlock(lpMINIRTSP_STREAM s)
{
	if(NULL == s){
		return rMINIRTSP_FAIL;
	}
	lpSTREAM_PRIV_CTX ctx = (lpSTREAM_PRIV_CTX)s->param;
	if(ctx){
		MEDIABUF_out_unlock(ctx->media_user);
		return rMINIRTSP_OK;
	}
	return rMINIRTSP_FAIL;
}


int _stream_next(lpMINIRTSP_STREAM s) {
	if(NULL == s){
		return rMINIRTSP_FAIL;
	}
	lpSTREAM_PRIV_CTX ctx = (lpSTREAM_PRIV_CTX)s->param;

	const lpSDK_ENC_BUF_ATTR attr = NULL;
	uint32_t out_size = 0;

	if(0 == MEDIABUF_out(ctx->media_user, (void **)&attr, NULL, &out_size)){
		void* raw_ptr = (void*)(attr + 1);
		uint32_t raw_size = attr->data_sz;
		uint64_t mb_timestamp = attr->timestamp_us;
		if(attr->timestamp_us < ctx->initUsPts){
			MEDIABUF_out_unlock(ctx->media_user);
			MEDIABUF_sync(ctx->media_user);
			return rMINIRTSP_DATA_NOT_AVAILABLE;
		}
		s->data = raw_ptr;
		s->size = raw_size;
		s->timestamp = mb_timestamp;
		if(kSDK_ENC_BUF_DATA_H264 == attr->type){
			s->isKeyFrame = attr->h264.keyframe;
			s->type = MINIRTSP_MD_TYPE_H264;
		}else if(kSDK_ENC_BUF_DATA_H265 == attr->type){
			s->type = MINIRTSP_MD_TYPE_H265;
			s->isKeyFrame = attr->h265.keyframe;
		}else if((kSDK_ENC_BUF_DATA_G711A == attr->type)){
			s->type = MINIRTSP_MD_TYPE_ALAW;
			s->isKeyFrame = FALSE;
		}else{
			return rMINIRTSP_DATA_NOT_AVAILABLE;
		}
		return rMINIRTSP_OK;

	}
	return rMINIRTSP_DATA_NOT_AVAILABLE;
}

int _stream_destroy(lpMINIRTSP_STREAM s)
{
	if(NULL == s){
		return rMINIRTSP_FAIL;
	}
	lpSTREAM_PRIV_CTX ctx = (lpSTREAM_PRIV_CTX)s->param;
	printf("liry  _stream_destroy\n");
	if(ctx && ctx->media_user){
		MEDIABUF_detach(ctx->media_user);
		ctx->media_user = NULL;
	}
	free(ctx);
	s->param = NULL;
	return rMINIRTSP_OK;
}
int _stream_reset(lpMINIRTSP_STREAM s)
{
	return rMINIRTSP_OK;
}


void rtsp_init_run(){
	if(rtsp){
		return ;
	}
	stMINIRTSP_STREAM_FUNC funcs;
	lpMINIRTSP thiz = MINIRTSP_server_new();

	funcs.open = (fMINIRTSP_STREAM_OPEN)_stream_init;
	funcs.lock = (fMINIRTSP_STREAM_LOCK)_stream_lock;
	funcs.next = (fMINIRTSP_STREAM_NEXT)_stream_next;
	funcs.unlock = (fMINIRTSP_STREAM_UNLOCK)_stream_unlock;
	funcs.close = (fMINIRTSP_STREAM_CLOSE)_stream_destroy;
	funcs.get_avc = NULL;
	funcs.reset = (fMINIRTSP_STREAM_RESET)_stream_reset;
	MINIRTSP_server_set_stream_func(funcs);

	MINIRTSP_set_loglevel(thiz, 5);
	MINIRTSP_server_venc_hook((MINIRTSP_SERVER_GET_VENC)Minirtsp_Server_Get_Venc);

	MINIRTSP_server_IFrame_hook(_MINIRTSP_SERVER_SET_IFRAME_);
	MINIRTSP_server_auth_hook(_MINIRTSP_SERVER_GET_PWD_BY_NAME_);
	MINIRTSP_server_start(554);
	rtsp = thiz;
}

void rtsp_init_stop(){
	if(rtsp){
		MINIRTSP_server_stop();
		MINIRTSP_delete(rtsp);
		rtsp = NULL;
	}
}



