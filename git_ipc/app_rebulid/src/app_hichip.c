
#include "app_hichip.h"
#include "hichip.h"
#include "md5sum.h"
#include "securedat.h"
#include "app_debug.h"
#include "gb28181.h"
#include "netsdk.h"

#include "ticker.h"
#include "global_runtime.h"
#include "hichipv10.h"

#define UCODE_SN_SIZE (14)
#define HICHIP_WITH_JUAN_SPEC "IPCAMabcdefghijklmnopqrstuvwxyzj"
#define DEVID_RANDOM_SIZE (7)
#define DEVID_FILE_PATH "/media/conf/devidram"
//#define DEVID_FILE_PATH "/mnt/flash/devidram"


char g_hichip_nonce[36] = {""};
static const char *hichip_nonce()
{
	if(0 == strlen(g_hichip_nonce)){
		int i, ret = 0;
		char random_str[32] = {0};

		struct timespec timetic;
		clock_gettime(CLOCK_MONOTONIC, &timetic);
		srand((unsigned) timetic.tv_nsec);
		for(i = 0; i < 32; ++i){
			g_hichip_nonce[i] = rand() % 26 + ((0 == rand() % 2) ? 'A' : 'a');
		}

		//printf("generate nonce: %s\r\n", g_hichip_nonce);
	}
	return g_hichip_nonce;
}


static const char *hichip_device_id()
{
	static char device_id[36] = {""};

	if(0 == strlen(device_id)){
		// setup once
		char sn_code[128] = {0};
		char sn[32];
		char *p_md5 = NULL;
		memset(sn, 0, sizeof(sn));

		// test ucode rw
		//int ret = UCODE_write(UCODE_SN_MTD, -1, "H1250100001747", strlen("H1250100001747"));


		if(0 == UC_SNumberGet(sn)) {
			p_md5 = md5sum_buffer(sn, UCODE_SN_SIZE);
			snprintf(sn_code, sizeof(sn_code), "IP_camera%s", p_md5);
			memcpy(device_id, sn_code, 32);
		}else{
			memcpy(device_id, HICHIP_WITH_JUAN_SPEC, strlen(HICHIP_WITH_JUAN_SPEC));
		}
		FILE *fp = fopen(DEVID_FILE_PATH, "rb");
		int i, ret = 0;
		char random_str[32] = {0};
		if(fp){
			ret = fread(device_id, 32, 1, fp);	
			fclose(fp);
		}else{
			if(0 == UC_SNumberGet(sn)) {
				p_md5 = md5sum_buffer(sn, UCODE_SN_SIZE);
				snprintf(sn_code, sizeof(sn_code), "IP_camera%s", p_md5);
				memcpy(device_id, sn_code, 32);
			}else{
				memcpy(device_id, HICHIP_WITH_JUAN_SPEC, strlen(HICHIP_WITH_JUAN_SPEC));
			}
			fp = fopen(DEVID_FILE_PATH, "wb+");
			struct timespec timetic;
			clock_gettime(CLOCK_MONOTONIC, &timetic);
			srand((unsigned) timetic.tv_nsec);
			for(i = 32 - DEVID_RANDOM_SIZE; i < 32; ++i){
				device_id[i] = rand() % 26 + ((0 == rand() % 2) ? 'A' : 'a');
			}
			fwrite(device_id, 32, 1, fp);
			fclose(fp);
		}
		//printf("device_id:%s\r\n", device_id);
#if 0

		int i = 0;
		struct timeval tv;
		// random seeds
		gettimeofday(&tv, NULL);
		srand(tv.tv_sec ^tv.tv_usec);
		memset(device_id, 0, sizeof(device_id));
		for(i = 0; i < 32; ++i){
			device_id[i] = rand() % 26 + ((0 == rand() % 2) ? 'A' : 'a');
		}
#endif
		//printf("generate device id: %s\r\n", device_id);
		setenv("HICHIP_ID", device_id , true);
	}

	return device_id;
}

static const char *hichip_device_model()
{
	ST_NSDK_SYSTEM_DEVICE_INFO deviceIinfo;
	static char deviceModel[ sizeof(deviceIinfo.model) ];
	NETSDK_conf_system_get_device_info(&deviceIinfo);
	strcpy(deviceModel, deviceIinfo.model);
	return deviceModel;
}

static const char *hichip_device_name()
{
	ST_NSDK_SYSTEM_DEVICE_INFO deviceIinfo;
	static char deviceName[ sizeof(deviceIinfo.deviceName) ];
	NETSDK_conf_system_get_device_info(&deviceIinfo);
	strcpy(deviceName, deviceIinfo.deviceName);
	return deviceName;
}

static const char *hichip_ether_lan()
{
	const char *eth = getenv("DEF_ETHER");
	return NULL == eth ? "eth0" : eth;
}

static const char *hichip_ether_vlan()
{
	return "eth0:1";
}

static void hichip_ip_adapt_pause(int timeout)
{
	nk_net_adapt_ip_set_pause_flag(time(NULL), timeout);
}

static int hichip_get_video_cnt(void)
{
	int channel_num;
	channel_num = NETSDK_venc_get_channels();

	//for fisheye
	ST_NSDK_VENC_CH venc_ch;
	if(channel_num > 3){
		NETSDK_conf_venc_ch_get(101, &venc_ch);
		if(venc_ch.resolution > kNSDK_RES_960X720){
			venc_ch.resolution = kNSDK_RES_960X720;
			venc_ch.resolutionWidth = 960;
			venc_ch.resolutionHeight = 720;
			//netsdk_venc_ch_changed(101, &venc_ch);
			NETSDK_conf_venc_ch_set(101, &venc_ch);
		}
	}
	
	
	return channel_num;
}

static int hichip_get_stream_name(int index, char *name)
{
	if(0 == MEDIABUF_get_username_by_id(index, name)){
		return 0;
	}else{
		return -1;
	}
}

int APP_HICHIP_init()
{
	stHICHIP_CONF_FUNC conf_func;
	memset(&conf_func, 0, sizeof(conf_func));
	// init callback interface
	conf_func.nonce = hichip_nonce;
	conf_func.device_id = hichip_device_id;
	conf_func.device_model = hichip_device_model;
	conf_func.device_name = hichip_device_name;
	conf_func.ether_lan = hichip_ether_lan;
	conf_func.ether_vlan = hichip_ether_vlan;
#if defined(GB28181)
	conf_func.gb28181_conf = GB28181_configure;
#else
    conf_func.gb28181_conf = NULL;
#endif
	conf_func.ip_adapt_pause = hichip_ip_adapt_pause;
	conf_func.get_video_count = hichip_get_video_cnt;
	conf_func.get_stream_name_by_index = hichip_get_stream_name;
	APP_TRACE("HICHIP INIT:%s",conf_func.ether_lan());

	
	//generate a nonce at first
	conf_func.nonce();
	conf_func.device_id();

	// web server hichip v1.0 cgi interfaces
	int stream_cnt = NETSDK_venc_get_channels();
	int i, vin, stream;
	char stream_name[64], url[64];


	for(i = 0; i < stream_cnt; i++){
		if(0 == MEDIABUF_get_username_by_id(i, stream_name)){
			if(2 == sscanf(stream_name, "ch%d_%d.264", &vin, &stream)){
				sprintf(url, "/livestream/%d%d", vin+1, stream+1);
				WEBS_add_cgi(url, HICHIP_live_stream, kH_METH_GET);
			}
		}
	}
	
	// FIXME: need to replace all the compat interfaces
	WEBS_add_cgi("/cgi-bin/hi3510/getidentify.cgi", HICHIPV10_get_identify, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/param.cgi", HICHIPV10_compat, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/getvdisplayattr.cgi", HICHIPV10_get_video_displayattr, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/getvencattr.cgi", HICHIPV10_compat, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/ptzctrl.cgi", HICHIPV10_compat, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/preset.cgi", HICHIPV10_compat, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/ptzup.cgi", HICHIPV10_compat, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/ptzdown.cgi", HICHIPV10_compat, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/ptzleft.cgi", HICHIPV10_compat, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/ptzright.cgi", HICHIPV10_compat, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/ptzzoomin.cgi", HICHIPV10_compat, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/ptzzoomout.cgi", HICHIPV10_compat, kH_METH_GET);
	WEBS_add_cgi("/cgi-bin/hi3510/getnonce.cgi", HICHIPV10_compat, kH_METH_GET);

	return HICHIP_init(conf_func);
}

void APP_HICHIP_destroy()
{
	HICHIP_destroy();
	printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);
}

int APP_HICHIP_Lock_init()
{
	return HICHIP_Lock_init();
}

int APP_HICHIP_Lock_destroy()
{
	return HICHIP_Lock_destroy();
}

