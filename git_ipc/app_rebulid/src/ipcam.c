#include "version.h"
#include "sdk/sdk_api.h"
#include "media_buf.h"

#include <unistd.h>
#include <getopt.h>
#include <sys/reboot.h>

//#include "rtspd.h"
#include "sensor.h"
//#include "spook/spook.h"
#include "sysconf.h"
#include "bsp/watchdog.h"

#include "firmware.h"

#include "ipcam_network.h"
#include "ipcam_timer.h"
#include "ipcam.h"
#include "gpio.h"
#include "app_debug.h"
#include "generic.h"
#include "securedat.h"
#include "bsp/rtc.h"

#include "esee_client.h"
#include "app_overlay.h"
#include "usrm.h"
#include "app_motion_detect.h"
#include "security.h"
#include "global_runtime.h"
#include "netsdk.h"
#include "ptz.h"
#include "hichip_http_cgi.h"
#include "sensor.h"
#include "env_common.h"
#include "app_recover.h"
//#include "conf.h"

#include "app_sdcard.h"
#include "frank_trace.h"
#include "uart_ptz.h"
//#include <app_tfer.h>
#include <app_tfcard.h>
#include <secure_chip.h>
#include "bsp/keytime.h"
#include "sound.h"
#include "p2p/p2pdevice.h"
#include "base/ja_process.h"
#include "mutex_f.h"
#include "bsp/bsp.h"
#include "sound_queue.h"
#include "model_conf.h"
#ifdef REBOOT_ONTIME
#include "app_reboot_ontime.h"
#endif
#ifdef VIDEO_CTRL
#include "app_video_ctrl.h"
#endif
#include "fisheye.h"
#include "custom.h"
#include "production_test.h"
#include "log.h"
#include "key_record.h"
#include "led_pwm.h"
#include "wifi/ja_wifi_seek.h"
#include "wpa_supplicant/include/wpa_status.h"
#include "app_wifi.h"
#include "uart_protocol.h"
#include <sys/prctl.h>

#if defined(TS_RECORD)
#include "tfcard/include/NK_Tfcard.h"
#endif


extern int network_4g_product_test();

#include "app_cloud_rec.h"

//Enviroment Definition
//#define IPCAM_ENV_HOME_DIR "/mnt/flash/nfs/git_ipc/gm_ipc/app_rebulid/bin"
//#define IPCAM_ENV_CONF_DIR "/mnt/flash/conf"
#define IPCAM_ENV_CONF_DIR "/media/conf"
#define IPCAM_ENV_CUSTOM_DIR "/media/custom"
#define IPCAM_ENV_RES_DIR IPCAM_ENV_HOME_DIR"/resource"
#define IPCAM_ENV_WEB_DIR IPCAM_ENV_CUSTOM_DIR"/web"
#define IPCAM_ENV_FONT_DIR IPCAM_ENV_CUSTOM_DIR"/font"
#define IPCAM_ENV_FLASH_MAP IPCAM_ENV_HOME_DIR"/flashmap.ini"
#define IPCAM_ENV_SYSCONF "/dev/mtdblock3"
#define IPCAM_ENV_DEFNETSDK_DIR IPCAM_ENV_RES_DIR"/netsdk"
#define IPCAM_ENV_NETSDK_DIR IPCAM_ENV_CONF_DIR"/netsdk"
#define IPCAM_ENV_ISPCFG_DIR IPCAM_ENV_CUSTOM_DIR"/ispcfg"

// ÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩbufferÔøΩÔøΩ–°
#define QUEUE_BUFFER_SIZE    1024 * 256


static void ipcam_signal_escape(int sig)
{
#if defined(UART_PROTOCOL)
    printf("\r\nStepper_Stopped\r\n");
    //ËÆæÂ§áÂºÇÂ∏∏ÈÄÄÂá∫ÔºåÂøÖÈ°ªÂÅúÊéâÁîµÊú∫ÔºåÈò≤Ê≠¢ÁîµÊú∫ÁÉßÂùè
    APP_MOTOR_Exit();
	APP_TRACE("Hey!");
#endif
	fclose(stdin);
	exit(0);
}

static void ipcam_signal_init()
{
	signal(SIGINT, ipcam_signal_escape);
	signal(SIGQUIT, ipcam_signal_escape);
	signal(SIGPIPE, SIG_IGN);
}

static void ipcam_signal_destroy()
{
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
}

static void delay_to_stop_sw()
{
//	TICKER_add_task(SW_destroy, 1, false);
}

static void _on_user_changed(const char *name, uint32_t user_number, uint32_t user_max)
{
	int vin = 0, stream = 0;
	static uint32_t last_user_number[2] = {0,0};
	if(2 == sscanf(name, "ch%d_%d.264", &vin, &stream)){
		if(sdk_enc && (last_user_number[stream] < user_number)){

			sdk_enc->request_stream_h264_keyframe(vin, stream);
			//APP_TRACE("request_stream_h264_keyframe:ch%d_%d", vin, stream);
			//APP_TRACE("user change from %d to %d", last_user_number[stream], user_number);
			usleep(200000);
		}
		last_user_number[stream] = user_number;
	}
}

static  void media_buf_on_user_changed_hook(const char *name, uint32_t user_number, uint32_t user_max)
{
	static int last_user_num = 0;
	int ch, stream_id;
	sscanf(name, "ch%d_%d.264", &ch, &stream_id);
	if(0 == ch && 0 == stream_id) {
		if(last_user_num > user_number){
			nk_net_adapt_ip_set_pause_flag(time(NULL), 60);
		}
		last_user_num = user_number;
	}
}

static void ipcam_mediabuf_avenc_init()
{
	int i = 0, ii = 0, iii = 0;
	int stream_cnt = NETSDK_venc_get_channels();

	MEDIABUF_init();
	SDK_ENC_init();

#if defined(HI3518E_V2)
    /* ‰∏ªË¶ÅÁî®Êù•Âå∫ÂàÜ18ev200 PxÂíåCxËÆæÁΩÆÁöÑQPÂÄÑ1§7 FIXME*/
    if(false == GLOBAL_sn_front()) {
        SDK_ENC_set_model(1);
    }
    else {
        SDK_ENC_set_model(0);
    }
#endif

	sdk_enc->do_buffer_request = MEDIABUF_in_request;
	sdk_enc->do_buffer_append = MEDIABUF_in_append;
	sdk_enc->do_buffer_commit = MEDIABUF_in_commit;

	for(i = 0; i < 1; ++i){
		// video input
		for(ii = 0; ii < stream_cnt; ++ii){
//		for(ii = stream_cnt - 1; ii >= 0 ; --ii){
			// video h264 encode
			char preferred_name[32] = {""};
			char alternate_name[32] = {""};

			if(stream_cnt > 3){//just for multi channel IP camera
				sprintf(preferred_name, "ch%d_0.264", ii);
			}else{
				sprintf(preferred_name, "ch%d_%d.264", i, ii);
			}
			//fix me
			if(ii ==0){
				//main stream
				sprintf(alternate_name, "720p.264");
			}else if(ii == 1){
				//sub stream 1
				sprintf(alternate_name, "360p.264");
			}else{
				//sub stream 2
				sprintf(alternate_name, "qvga.264");
			}

			int const entry_available = 400;
			int const entry_key_available = 1;
			int user_available;
			switch(stream_cnt){
				default:
				case 3:{
					if(ii == 0){	
						user_available = 5;
					}else{
						user_available = 8;
					}
				}
					break;	
				case 2:{//HI3518E
					if(ii == 0){	
						user_available = 8;
					}else{
						user_available = 8;
					}
				}
					break;
			}
			int mem_size = 0;
			uint32_t sys_mem = GLOBAL_get_sys_mem_mb();
			if(sys_mem < 50){
				if(ii == 0){
					//main stream
#if defined(HI3516E_V1)
					mem_size = 1*1024*1024;//1M
#else
					mem_size = 2*1024*1024;//1M
#endif
				}else{
					//sub stream
					mem_size = 150*1024;//50K
				}
			}else{
				if(ii == 0){
					//main stream
					#if defined(HI3516D)
						mem_size = 8*1024*1024;//8M
                    #else
                        mem_size = 2*1024*1024;//2M
                    #endif
				}else{
					//sub stream
					#if defined(HI3516D)
						mem_size = 800*1024;//800K
                    #else
                        mem_size = 200*1024;//200K
					#endif
				}
			}

			
			if(0 == MEDIABUF_new(mem_size, preferred_name, alternate_name, entry_available, entry_key_available, user_available)){
				// success to new a mediabuf

				
				
				ST_SDK_ENC_STREAM_ATTR stream_attr;
		
				LP_SDK_ENC_STREAM_H264_ATTR stream_h264_attr = &stream_attr.H264_attr;				
				LP_SDK_ENC_STREAM_H265_ATTR stream_h265_attr = &stream_attr.H265_attr;
				
					
				int venc_id = 0;
				int const buf_id = MEDIABUF_lookup_byname(preferred_name);
				
				MEDIABUF_hock_user_changed(buf_id, media_buf_on_user_changed_hook);
		 //   	int stream_rc_mode = kSDK_ENC_H264_RC_MODE_CBR;
				int stream_rc_mode;

		//		strcpy(stream_h264_attr.name, preferred_name);
				MEDIABUF_hock_user_changed(buf_id, _on_user_changed);
				// start video stream
				// FIXME:
				ST_NSDK_VIN_CH vin_ch;
				if(NETSDK_conf_vin_ch_get(1, &vin_ch)&& ii==0){
					netsdk_vin_ch_set(1, &vin_ch);
				}

				
				ST_NSDK_VENC_CH venc_ch;
				if(NETSDK_conf_venc_ch_get((i+1)*100+ii+1, &venc_ch)){ // ËøòÊòØÊ†πÊçÆÈÖçÁΩÆÊñá‰ª∂ÂàõÂª∫ËßÜÈ¢ëÊµÅ
		//			venc_ch.codecType = kNSDK_CODEC_TYPE_H264;  // yang by test
						
					switch(venc_ch.codecType){
						default:
						case kNSDK_CODEC_TYPE_H264:	
							stream_attr.enType = kSDK_ENC_BUF_DATA_H264;
							stream_rc_mode = kSDK_ENC_H264_RC_MODE_CBR;
							strcpy(stream_h264_attr->name, preferred_name);
							if(venc_ch.freeResolution){
								stream_h264_attr->width = venc_ch.resolutionWidth;
								stream_h264_attr->height = venc_ch.resolutionHeight;
							}else{
								stream_h264_attr->width = (venc_ch.resolution >> 16) & 0xffff;
								stream_h264_attr->height = (venc_ch.resolution >> 0) & 0xffff;
							}
							//printf("\n\n\nresolution: %dx%d\n\n\n", stream_h264_attr->width, stream_h264_attr->height);
							stream_h264_attr->fps = venc_ch.frameRate;
							stream_h264_attr->gop = venc_ch.keyFrameInterval;

		//						stream_h264_attr->profile = kSDK_ENC_H264_PROFILE_MAIN;
							stream_h264_attr->profile = venc_ch.h264Profile;
							switch(venc_ch.bitRateControlType){
								case kNSDK_BR_CONTROL_VBR:
									stream_rc_mode = kSDK_ENC_H264_RC_MODE_VBR; break;
								case kNSDK_BR_CONTROL_CBR:
									stream_rc_mode = kSDK_ENC_H264_RC_MODE_CBR; break;
								default:
									stream_rc_mode = kSDK_ENC_H264_RC_MODE_VBR; break;
							}
							stream_h264_attr->rc_mode = stream_rc_mode;
							stream_h264_attr->bps = venc_ch.constantBitRate;
							switch(venc_ch.definitionType){
								case kNSDK_DEFINITION_FLUENCY:
									stream_h264_attr->quality = kSDK_ENC_QUALITY_FLUENCY;
									break;
								case kNSDK_DEFINITION_BD:
									stream_h264_attr->quality = kSDK_ENC_QUALITY_BD;
									break;
								case kNSDK_DEFINITION_HD:
									stream_h264_attr->quality = kSDK_ENC_QUALITY_HD;
								case kNSDK_DEFINITION_AUTO:
								default:
									stream_h264_attr->quality = kSDK_ENC_QUALITY_AUTO;
									break;									
							}
							stream_h264_attr->buf_id = buf_id;

                            /* ‰∏ªË¶ÅÁî®‰∫éÂÖºÂÆπÂØπÊé•NVRÊó∂Ôºå‰∏çÂºÄÂêØÈ´òÁ∫ßË∑≥Â∏ßÂèÇËÄÑ1§7 */
                            if(venc_ch.ImageTransmissionModel == eNSDK_LOW_BPS_MODEL) {
                                stream_h264_attr->refEnable = 1;
                            }
                            else if(venc_ch.ImageTransmissionModel == eNSDK_COMPATIBILITY_MODE) {
                                stream_h264_attr->refEnable = 0;
                            }
                            else {
                                stream_h264_attr->refEnable = 1;
                            }
                            stream_h264_attr->_enable_smartP = false;
						//	sdk_enc->create_stream_h264(i, ii, stream_h264_attr);
						//	sdk_enc->enable_stream_h264(i, ii, true);

							
							SDK_ENC_create_stream(i, ii, &stream_attr);
							SDK_ENC_enable_stream(i, ii, true);

							break;
						case kNSDK_CODEC_TYPE_H265:							
							stream_attr.enType = kSDK_ENC_BUF_DATA_H265;
							stream_rc_mode = kSDK_ENC_H265_RC_MODE_CBR;
							strcpy(stream_h265_attr->name, preferred_name);
							if(venc_ch.freeResolution){
								stream_h265_attr->width = venc_ch.resolutionWidth;
								stream_h265_attr->height = venc_ch.resolutionHeight;
							}else{
								stream_h265_attr->width = (venc_ch.resolution >> 16) & 0xffff;
								stream_h265_attr->height = (venc_ch.resolution >> 0) & 0xffff;
							}
							//printf("\n\n\nresolution: %dx%d\n\n\n", stream_h265_attr->width, stream_h265_attr->height);
							stream_h265_attr->fps = venc_ch.frameRate;
							stream_h265_attr->gop = venc_ch.keyFrameInterval;
#if !defined(HI3516E_V1)
							// FIXME:
							if(stream_h265_attr->gop > 100){
								stream_h265_attr->gop = 100;
							}
#endif
		//						stream_h265_attr->profile = kSDK_ENC_H264_PROFILE_MAIN;
							stream_h265_attr->profile = venc_ch.h264Profile;
							switch(venc_ch.bitRateControlType){
								case kNSDK_BR_CONTROL_VBR:
									stream_rc_mode = kSDK_ENC_H265_RC_MODE_VBR; break;
								case kNSDK_BR_CONTROL_CBR:
									stream_rc_mode = kSDK_ENC_H265_RC_MODE_CBR; break;
								default:
									stream_rc_mode = kSDK_ENC_H265_RC_MODE_VBR; break;
							}
							stream_h265_attr->rc_mode = stream_rc_mode;
							stream_h265_attr->bps = venc_ch.constantBitRate;
							switch(venc_ch.definitionType){
								case kNSDK_DEFINITION_FLUENCY:
									stream_h265_attr->quality = kSDK_ENC_QUALITY_FLUENCY;
									break;
								case kNSDK_DEFINITION_BD:
									stream_h265_attr->quality = kSDK_ENC_QUALITY_BD;
									break;
								case kNSDK_DEFINITION_HD:
									stream_h265_attr->quality = kSDK_ENC_QUALITY_HD;
								case kNSDK_DEFINITION_AUTO:
								default:
									stream_h265_attr->quality = kSDK_ENC_QUALITY_AUTO;
									break;									
							}
							stream_h265_attr->buf_id = buf_id;
                            stream_h265_attr->_enable_smartP = false;
							
							SDK_ENC_create_stream(i, ii, &stream_attr);
							SDK_ENC_enable_stream(i, ii, true);
							break;	
					}					
				}
			}
		}

		ST_NSDK_IMAGE image;
		if(NETSDK_conf_image_get(&image)){
			netsdk_image_changed(&image);
		}

#if defined(HI3516E_V1)
		ST_NSDK_AIN_CH ain_ch;
		if(NETSDK_conf_ain_ch_get(1, &ain_ch)){
			int audioInputGain = 50;
			if(ain_ch.microphoneType == kNSDK_ACTIVE_PICKUP){	
				audioInputGain =  50;
			
			}else if(ain_ch.microphoneType == kNSDK_PASSIVE_MIC){
				audioInputGain =  100;
			}
			
			BSP_Audio_set_volume_val(audioInputGain, 0, -1, -1);
		}
#endif

		ST_NSDK_AENC_CH aenc_ch;
		NETSDK_conf_aenc_ch_get(101, &aenc_ch);
		if(aenc_ch.enabled){
			sdk_enc->create_audio_stream(0, 0, aenc_ch.codecType);
		}
	}
}

static void ipcam_mediabuf_avenc_destroy()
{
	APP_OVERLAY_destroy();
	SDK_ENC_destroy();
	SDK_destroy_vin();//fix me
	MEDIABUF_destroy();
}



/*
#define IRCUT_LED_GPIO_PINMUX_ADDR 0x200f0120
#define IRCUT_LED_GPIO_DIR_ADDR 0x20140400
#define IRCUT_LED_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_LED_GPIO_PIN 0
#define IRCUT_LED_GPIO_GROUP 0

//new hardware ir-cut control :GPIO0_2
#define NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR 0x200f0128
#define NEW_IRCUT_CTRL_GPIO_DIR_ADDR 0x20140400
#define NEW_IRCUT_CTRL_GPIO_DATA_ADDR 0x201403fc
#define NEW_IRCUT_CTRL_GPIO_PIN 2
#define NEW_IRCUT_CTRL_GPIO_GROUP 0

//old hardware ir-cut control :GPIO0_4
#define IRCUT_CTRL_GPIO_PINMUX_ADDR 0x200f0130
#define IRCUT_CTRL_GPIO_DIR_ADDR 0x20140400
#define IRCUT_CTRL_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_CTRL_GPIO_PIN 4
#define IRCUT_CTRL_GPIO_GROUP 0

//ir-cut photoswitch read:GPIO0_6
#define IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR 0x200f0138
#define IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR 0x20140400
#define IRCUT_PHOTOSWITCH_GPIO_DATA_ADDR 0x201403fc
#define IRCUT_PHOTOSWITCH_GPIO_PIN 6
#define IRCUT_PHOTOSWITCH_GPIO_GROUP 0

//default factory reset:GPIO0_7
#define HW_RESET_GPIO_PINMUX_ADDR 0x200f013c
#define HW_RESET_GPIO_DIR_ADDR 0x20140400
#define HW_RESET_GPIO_DATA_ADDR 0x201403fc
#define HW_RESET_GPIO_PIN 7
#define HW_RESET_GPIO_GROUP 0
*/
#include "ptz/steper_ctrl.h"
static void ipcam_gpio_init()
{
	APP_GPIO_init();
	//GPIO5_1
//	APP_GPIO_add("alarm in 0", 0x200f00b8, 0xffffffff, 0, 0x20190400, 0x2, 0, 0x2, 0x20190008, 0x2);
	//GPIO0_3
//	APP_GPIO_add("alarm out 0", 0x200f012c, 0xffffffff, 0, 0x20140400, 0x8, 0x0, 0x8, 0x20140020, 0x8);
	//GPIO0_2
	APP_GPIO_add("ircut in 0", 0x200f0128, 0xffffffff, 0, 0x20140400, 0x4, 0, 0x4, 0x20140010, 0x4);
	//GPIO0_4
	APP_GPIO_add("ircut in 1", 0x200f0130, 0xffffffff, 0, 0x20140400, 0x10, 0, 0x10, 0x20140040, 0x10);
	//GPIO0_0
	APP_GPIO_add("ir led 0", 0x200f0120, 0xffffffff, 0, 0x20140400, 0x1, 0, 0x1, 0x20140004, 0x1);
	//GPIO0_6
	APP_GPIO_add("ircut out 0", 0x200f0138, 0xffffffff, 0, 0x20140400, 0x40, 0, 0x40, 0x20140100, 0x40);
	//GPIO0_7
	APP_GPIO_add("factory reset", 0x200f013c, 0xffffffff, 1, 0x20140400, 0x80, 0, 0x80, 0x20140200, 0x80);

}

static unsigned int ipcam_user_do_crc32(void* data, ssize_t data_sz)
{
	int i = 0;
	SYS_U32_t* data_32 = data;
	SYS_U32_t data_sz_32 = data_sz / sizeof(SYS_U32_t);
	SYS_U32_t const crc32_origin = 0xfefeef11;
	SYS_U32_t crc32_result = crc32_origin;
	for(i = 0; i < data_sz_32; ++i){
		crc32_result ^= data_32[i];
	}
	return crc32_result;
}

char rand_num[3];

static int ipcam_usrm_init()
{
	char pwd[32];
	
	unsigned int crc;
	char ucode[32];
	if(0 == UC_SNumberGet(ucode))
	{
		struct timespec timetic;
		clock_gettime(CLOCK_MONOTONIC, &timetic);
		srand((unsigned) timetic.tv_nsec);
		rand_num[0] = rand() % 26 + ((0 == rand() % 2) ? 'A' : 'a');
		rand_num[1] = rand() % 26 + ((0 == rand() % 2) ? 'A' : 'a');
		rand_num[2] = 0;
		strcat(ucode, rand_num);
		//printf("ucode=%s\r\n", ucode);
		crc = ipcam_user_do_crc32(ucode, strlen(ucode));
		sprintf(pwd, "%x", crc);
		//printf("crc=%s\r\n", pwd);
		USRM_init("/dev/mtdblock2", "super", pwd);
	}
	else
	{
		USRM_init("/dev/mtdblock2", NULL, NULL);
	}
	
}

static uint32_t ipcam_authentication()
{
	char sn[32];
	uint32_t ret = 0x00010000;
	memset(sn, 0, sizeof(sn));
	if(0 == UC_SNAuthenChk()){
		if(0 == UC_SNumberGet(sn) && 0 == UC_SNumberChk() && 0 == SECURE_CHIP_init()) {
			//if the SN code exist in encryp chip
			APP_TRACE("new chip UC_SNumberGet:%s", sn);
			g_authorized = true;

			//new hardware version with encryp chip
			ret = 0x00010100;//v1.1.0
		}
		g_encryp_chip_type = UC_ENCRYP_CHIP_TYPE_24C02;
	}else{
		if(0 == UC_SNumberGet(sn) && 0 == UC_SNumberChk()) {
			//if the SN code exist in flash
			APP_TRACE("no chip UC_SNumberGet:%s", sn);
			g_authorized = true;
			g_encryp_chip_type = UC_ENCRYP_CHIP_TYPE_NONE;
			//old hardware version without encryption chip
			ret = 0x00010000;//v1.0.0
		}
	}

	if(strlen(sn)> 10){
		if(sn[strlen(sn) - 10] == '0'){
			memcpy(g_esee_id, &sn[strlen(sn) - 10+1], 9);
		}else{
			memcpy(g_esee_id, &sn[strlen(sn) - 10], 10);
		}
	}

	APP_TRACE("hardware version:%x--ID:%s", ret, g_esee_id);
	return ret;
}

int ipcam_ptz_init()
{
	if(!strcmp(PRODUCT_MODEL, "561122")){
		//just for ptz model with rmii
		//PTZ_Init();
	}

	PTZ_t ptzAttr;
	memset(&ptzAttr, 0, sizeof(PTZ_t));

//#define FISHEYE_EPTZ
#if defined(UART_PTZ)
	ptzAttr.Init = uart_ptz_Init;
	ptzAttr.Destroy = uart_ptz_Destroy;
	ptzAttr.Config = uart_ptz_Config;
	ptzAttr.Send = uart_ptz_Send;
#elif defined(FISHEYE_EPTZ)
	ptzAttr.Init = NULL;
	ptzAttr.Destroy = NULL;
	ptzAttr.Config = NULL;
	ptzAttr.Send = SDK_ENC_eptz_ctrl;
#elif defined(UART_PROTOCOL)
	ptzAttr.Init = UART_protocol_ptz_Init; // Âú®‰∏ãÈù¢ÁöÑPTZ_Init(&ptzAttr)‰∏≠ÔºåË∞ÉÁî®Ëøô‰∏™Áî®Êù•ÂàùÂßãÂåñ
	ptzAttr.Destroy = UART_protocol_ptz_Destroy;
	ptzAttr.Config = UART_protocol_ptz_Config;
	ptzAttr.Send = UART_protocol_ptz_Send;
#else
	ptzAttr.Init = NULL;
	ptzAttr.Destroy = NULL;
	ptzAttr.Config = NULL;
	ptzAttr.Send = NULL;
#endif

	PTZ_Init(&ptzAttr);
//	PTZ_Config(0, "PELCOD",  57600, 1);  //nChn: 0 ;  "szProtocol": "PELCOD" or "PELCOP";nBaud: 57600;u8Addr: 1;
	
#if defined(UART_PROTOCOL)

	ST_NSDK_PTZ_CFG stPtzConfig;
	memset(&stPtzConfig, 0, sizeof(stPtzConfig));
	NETSDK_conf_ptz_ch_get(&stPtzConfig);
	
	int baud_rate = stPtzConfig.stPtzExternalConfig.nBaudRate; // Ê≥¢ÁâπÁéá
	int data_bits = stPtzConfig.stPtzExternalConfig.nDateBit;  // Êï∞ÊçÆÁöÑ‰ΩçÊï∞
	char parity = stPtzConfig.stPtzExternalConfig.strParity;   // Â•áÂÅ∂Ê†°È™å
	int stop_bits = stPtzConfig.stPtzExternalConfig.nStopBit;  // ÂÅúÊ≠¢‰Ωç

	printf("baud_rate =%d data_bits=%d  parity =%c stop_bits=%d ",baud_rate, data_bits,parity,stop_bits);
	UART_protocol_reconfig(baud_rate, data_bits, parity, stop_bits); //Âä†ËΩΩÈÖçÁΩÆÊñá‰ª∂‰ø°ÊÅØ

	APP_UART_protocol_init(); // Â∫îÁî®Â±ÇÂõûË∞ÉÂáΩÊï∞
#endif

	
	return 0;
}

static void release_overlay_caution()
{
	APP_OVERLAY_release_caution(0);
}

static void sdcard_upgrade(bool downgrade)
{
	int rate = 0, err_num = 0, size = 4, ret;
	GLOBAL_before_upgrade_destroy();
	FIRMWARE_import_release_memery();

    FwHeader_t fw_header;
    FILE *fp = NULL;
    int i = 0;
    ssize_t fileSize = 0;
    void *upfile_mem = NULL;
    fp = fopen(FIRMWARE_IMPORT_FILE, "r+");
    if(NULL == fp)
    {
        printf("%s: fopen %s failed\n", __FUNCTION__, FIRMWARE_IMPORT_FILE);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    upfile_mem = FIRMWARE_upgrade_env_prepare(&fileSize);
    printf("%s() line:%d\n", __FUNCTION__, __LINE__);
    if(NULL == upfile_mem)
    {
        printf("%s() error line:%d\n", __FUNCTION__, __LINE__);
        GLOBAL_reboot_system();
    }
    memset(upfile_mem, 0, fileSize);
    printf("upgrade total size = %d\r\n", fileSize);

    ret = fread(upfile_mem, 1, fileSize, fp);
    if(ret != fileSize)
    {
        printf("%s: fread file %s failed, read size %d\n", __FUNCTION__, FIRMWARE_IMPORT_FILE, ret);
        GLOBAL_reboot_system();
    }

    fclose(fp);
    fp = NULL;

    ret = FIRMWARE_upgrade_start2(true, true, 1, upfile_mem, fileSize);
	//ret = FIRMWARE_upgrade_start(false, downgrade, FIRMWARE_IMPORT_FILE);
	if(ret < 0){
		APP_TRACE("start FIRMWARE_upgrade thread failed, exit");
		FIRMWARE_import_recover_memery();
		initLedContrl(DEF_LED_ID,true,LED_DARK_MODE);
		GLOBAL_reboot_system();
	}
	do{
		err_num = FIRMWARE_upgrade_get_errno();
		rate = FIRMWARE_upgrade_get_rate();
		APP_TRACE("upgrade_rate = %d", rate);
		sleep(1);
	}while(100 > rate && FIRMWARE_UPGRADE_ERROR_NONE == err_num);
	FIRMWARE_import_recover_memery();
	if(FIRMWARE_UPGRADE_ERROR_NONE == err_num){
		APP_TRACE("UPGRADE errno:%d", err_num);
		//SearchFileAndPlay(SOUND_Upgrade_completed, NK_True);
	}else{
		SearchFileAndPlay(SOUND_Firmware_update_failed, NK_True);
	}

    if(!IS_FILE_EXIST(FIRMWARE_NOT_MV_FILE)){
		NK_SYSTEM("mv "FIRMWARE_IMPORT_FILE" "FIRMWARE_IMPORT_FILE".used");
	}
 
 	sleep(5);
	GLOBAL_reboot_system();
}

static int ipcam_check_version_and_upgrade()
{
	int ret = 0;
	FwHeader_t *header = NULL;
	FILE *fp = NULL;

	fp = fopen(FIRMWARE_IMPORT_FILE, "r+");
	if(fp != NULL){

        header = calloc(sizeof(FwHeader_t), 1);
        if(NULL == header)
        {
            fclose(fp);
            return -1;
        }

		ret = fread(header, 1, sizeof(FwHeader_t), fp);
		fclose(fp);
		fp = NULL;
		if(ret == sizeof(FwHeader_t)){
			APP_TRACE("upgrade (%d.%d.%d.%s => %d.%d.%d.%s)", SWVER_MAJ, SWVER_MIN, SWVER_REV, SWVER_EXT, header->version.major, header->version.minor, header->version.revision, header->version.extend);
			if(header->version.major != SWVER_MAJ || header->version.minor != SWVER_MIN || header->version.revision != SWVER_REV || strcmp(header->version.extend, SWVER_EXT) != 0){
				if(FIRMWARE_upgrade_import_check(false, FIRMWARE_IMPORT_FILE)){
					APP_TRACE("start upgrade!");
					SearchFileAndPlay(SOUND_Upgrade, NK_True);
					sleep(4);
					sdcard_upgrade(false);

                    if(NULL != header)
                    {
                        free(header);
                        header = NULL;
                    }
					return 0;
				}
				else{
					APP_TRACE("check file failed!");
				}
			}
		}
		else{
			APP_TRACE("read header failed!(%d != %d)", ret, sizeof(FwHeader_t));
		}
	}

    if(NULL != header)
    {
        free(header);
        header = NULL;
    }

	return -1;
}
 
static void key_upgrade(void)
{
#define SDCARD_UPGRADE_DETECT_TIME 10
#define SDCARD_UPGRADE_KEY_PRESS_CNT 3
	static time_t begin_time;
	time_t cur_time = time(NULL);
	static uint8_t press_cnt = 0;
	if(0 == press_cnt){
		begin_time = time(NULL);
	}
	if(cur_time - begin_time > SDCARD_UPGRADE_DETECT_TIME){
		press_cnt = 0;
		begin_time = time(NULL);
	}
	APP_TRACE("%d-%d:%d", cur_time, begin_time, press_cnt);
	if((SDCARD_UPGRADE_KEY_PRESS_CNT-1) <= press_cnt++){
		//start upgrade from sdcard
		if(IS_FILE_EXIST(FIRMWARE_IMPORT_FILE)){
			APP_TRACE("start upgrade!");
			//TFCARD_exit();
			SearchFileAndPlay(SOUND_Upgrade, NK_True);
			sleep(4);
 			sdcard_upgrade(true); 
		}
		press_cnt = 0;
	}
}

static void start_smartlink()
{
#if defined(HI3516D)
    return;
#endif

#if defined(WIFI) && defined(SMART_LINK)
    // ÂêØÂä®smartlink,1ÂàÜÈíüÂêésmartlinkÁ∫øÁ®ãËá™Âä®ÈÄÄÂá∫
    ST_NSDK_NETWORK_INTERFACE n_interface;
    ST_NSDK_SYSTEM_SETTING system;

    NETSDK_conf_system_get_setting_info(&system);
    if(false == system.capabilitySet.pir)
    {
        return;
    }

    if(0 == APP_WIFI_smartlink_is_running()) {
        NETSDK_conf_interface_get(4, &n_interface);
        if(n_interface.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT) {
            ST_PRODUCT_TEST_INFO product_info;
            if(NULL != PRODUCT_TEST_get_info(&product_info))
            {
                APP_TRACE("product testing...");
                return;
            }
            SMART_link_init(NSDK_NETWORK_WIRELESS_MODE_STA_ETH);
#ifdef LED_CTRL
            initLedContrl(DEF_LED_ID,true,LED_MAX_MODE);
#endif
            SearchFileAndPlay(SOUND_Pairing_Mode, NK_True);
        }
    }

#endif

}

static void *key_press(void)
{
    key_upgrade();

}

void *wifi_mode_switch_to_sta(void)
{
	ST_NSDK_NETWORK_INTERFACE n_interface;
	printf("%s\n", __FUNCTION__);
#if defined(P2P)
	//P2PSDKDeinit();
#endif
	NETSDK_conf_interface_get(4, &n_interface);
	n_interface.wireless.wirelessMode = NSDK_NETWORK_WIRELESS_MODE_STATIONMODE;
	sprintf(n_interface.wireless.wirelessStaMode.wirelessApEssId, "111111j");
    //SearchFileAndPlay(SOUND_Switch_to_station_mode, NK_False);
	sleep(1);
	NETSDK_conf_interface_set(4, &n_interface, eNSDK_CONF_SAVE_RESTART);
}

void *wifi_mode_switch_to_monitor(void)
{
	//SearchFileAndPlay(SOUND_Start_soundwave_configuring, NK_True);
#ifdef LED_CTRL
	initLedContrl(DEF_LED_ID,true,LED_MIN_MODE);
#endif

#if defined(WIFI) && defined(SMART_LINK)
	SMART_link_deinit("wlan0");
	SMART_link_init(NSDK_NETWORK_WIRELESS_MODE_STA_ETH);
#endif

#if defined(SOUND_WAVE)
//    SW_init();
#endif

}


void *wifi_mode_switch_to_ap(void)
{
	ST_NSDK_NETWORK_INTERFACE n_interface;
	NETSDK_conf_interface_get(4, &n_interface);
	n_interface.wireless.wirelessMode = NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT;
	printf("%s-%s\n", __FUNCTION__, n_interface.wireless.wirelessApMode.wirelessEssId);

#if defined(P2P)
	//P2PSDKDeinit();
#endif
	char sn_str[32] = {0};
	if(0 == UC_SNumberGet(sn_str)) {
		if(strlen(sn_str)> 10){
			snprintf(n_interface.wireless.wirelessApMode.wirelessEssId, 
				sizeof(n_interface.wireless.wirelessApMode.wirelessEssId), "IPC%s", 
				sn_str);
		}
	}else{
		snprintf(n_interface.wireless.wirelessApMode.wirelessEssId, 
			sizeof(n_interface.wireless.wirelessApMode.wirelessEssId), "IPC123456", 
			sn_str);
	}
	//IPCAM_timer_check_sta_status_stop(); // «–ªªµΩapƒ£ Ωπÿ±’wifi staºÏ≤‚◊¥Ã¨
	//SearchFileAndPlay(SOUND_Switch_to_AP_mode, NK_False);
	NETSDK_conf_interface_set(4, &n_interface, eNSDK_CONF_SAVE_RESTART);
	
}

void *reset_default_factory_by_key(void)
{
	printf("%s\n", __FUNCTION__);
    WPA_stop_connect();
	WPA_resetWifiConnectedFlag();
	SearchFileAndPlay(SOUND_Restore_factory_settings, NK_True);
	//IPCAM_timer_check_sta_status_stop(); // ª÷∏¥≥ˆ≥ß…Ë÷√πÿ±’wifi staºÏ≤‚◊¥Ã¨
	GLOBAL_remove_conf_file();
	USRM_reset_user_by_root();

    /* ª÷∏¥≥ˆ≥ß÷ÿ∆Ùapp«∞£¨∞—wifiœ‡πÿ≥Ã–ÚkillµÙ∫Õremmod wifi«˝∂Ø */
#if defined(WIFI)
    NK_WIFI_adapter_monitor_thread_stop();
    JN_Wifi_Exit();
	//APP_WIFI_exit_wifi();
#endif

#if defined (LED_PWM)
	LED_PWM_reset();
#endif

	//sleep(2);
	exit(0);
}

static void add_wifi_leader_sound()
{
	//SearchFileAndPlay(SOUND_System_starting_completed, NK_True);
#if defined(WIFI)
	ST_NSDK_NETWORK_INTERFACE wlan;
	NETSDK_conf_interface_get(4, &wlan);
	if(APP_WIFI_model_exist()){
		if(wlan.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_STATIONMODE){
			/*if(strcmp(wlan.wireless.wirelessStaMode.wirelessApEssId, "hotspot")){
			//has been setup
			//SearchFileAndPlay(SOUND_WiFi_connecting, NK_False);
			//SearchFileAndPlay(SOUND_Please_wait, NK_False);
			}else{
			//SearchFileAndPlay(SOUND_Please_configure_network, NK_True);
			}*/
			//IPCAM_timer_check_sta_status_start(); // ipc≥ı ºªØ ±£¨wifi≈‰÷√Œ™staƒ£ Ω,ø™∆Ùsta◊¥Ã¨ºÏ≤‚≈–∂œ¡¨Ω”◊¥Ã¨
		}
		else if(wlan.wireless.wirelessMode == NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT){
			SearchFileAndPlay(SOUND_Configuration_mode, NK_True);
#if defined(SOUND_WAVE)
//			SW_init();
#endif
		}
	}
#endif

}

void ipcam_bsp_init(int val, int audioHwSpec)
{
    int bspModelName = em_BSP_MODEL_NAME_PX;
    bool ledEnabled = true;
    if(false == GLOBAL_sn_front()) {
        bspModelName = em_BSP_MODEL_NAME_CX;
    }
    else {
        bspModelName = em_BSP_MODEL_NAME_PX;
    }
#if defined(HI3516E_V1)
#if defined(PIR_ON_EXPAND)
    ST_CUSTOM_SETTING custom;
    if(0 == CUSTOM_get(&custom) && 0 < custom.function.pirEnabled)
    {
        ledEnabled = false;
    }
#endif
#endif

	BSP_ContrlInit(val, audioHwSpec, bspModelName, ledEnabled);
	BSP_GPIO_Init();
	BSP_Speaker_Enable(false);
#if  defined(HI3516E_V1)
    if(SENSOR_MODEL_IMX307 == g_sensor_type)
    {
        LED_PWM_init(true, false, false);
    }
#endif
}

static void ipcam_sensor_init()
{
	stBSPApi api;
	api.BSP_GET_PHOTOSWITCH = BSP_Get_Photo_Val;
	api.BSP_IRCUT_SWITCH = BSP_IRCUT_Switch;
	//api.BSP_SENSOR_RESET = BPS_Senser_reset;
	api.BSP_SET_IR_LED = BSP_IR_Led;
	api.BSP_SET_WHITE_LIGHT_LED = BSP_WHITE_LIGHT_Led;
#if  defined(HI3516E_V1)
	api.BSP_SET_PWM_DUTY_CYCLE = LED_PWM_duty_cycle_set;
#else
    api.BSP_SET_PWM_DUTY_CYCLE = NULL;
#endif
	SENSOR_init(SOC_MODEL, &api);
}

static void ipcam_set_audio_volume()
{
#define CUSTOM_AI_GAIN_FLAG		"/media/conf/old_custom_ai_gain_flag"
	ST_CUSTOM_SETTING custom;
	ST_NSDK_AIN_CH ain;
	char cmd[64];
	NETSDK_conf_ain_ch_get(1, &ain);
	if(0 == CUSTOM_get(&custom)) {
		/* ‘ˆº”“ª∏ˆ¬ﬂº≠£¨÷˜“™Œ™¡ÀºÊ»›æ…∞Ê±æ≤˙≤‚ ±…Ë÷√µƒ ‰»Î‘ˆ“Ê∫Õ“Ù¡ø÷µ */
		/* “ÚŒ™æ…∞Ê±æ÷∏∂®µƒ ‰»Î‘ˆ“Ê∂‘”¶÷µ£¨”Îƒø«∞µƒ‘ˆ“Ê÷µœ‡≤Ó∫‹¥Û
		   æ…∞Ê±æ≥ˆ≥ß ±£¨≤˙≤‚…Ë÷√µƒ ‰»Î‘ˆ“Ê «40£¨ºŸ»Á”√‘⁄◊Ó–¬µƒ‘ˆ“Ê∂‘”¶÷µ£¨æÕ «22db
		   µ´ «πË¬Û∫Õƒ£ƒ‚¬Û≤‚ ‘okµƒ‘ˆ“Ê÷µ «48db£¨“‘œ¬∏˘æ›±Íº«Œƒº˛£¨
		   ∏¸∏ƒ≤˙≤‚∂®÷∆Œƒº˛custom.jsonµƒ ‰»Î‘ˆ“Ê∫Õ“Ù¡ø÷µ£¨ ‰»Î‘ˆ“Ê÷µ¥Û–°∏ƒŒ™90
		*/
		if(!IS_FILE_EXIST(CUSTOM_AI_GAIN_FLAG)) {
			if(CUSTOM_check_int_valid(custom.function.audioInputGain)) {
				if(custom.function.audioInputGain < 80) {
					custom.function.audioInputGain = 90;
					CUSTOM_set(&custom);
				}
			}
			snprintf(cmd, sizeof(cmd), "echo true > %s", CUSTOM_AI_GAIN_FLAG);
			NK_SYSTEM(cmd);
		}

		BSP_Audio_set_volume_val(custom.function.audioInputGain, -1, custom.function.audioOutputGain, -1);
	}else{
		//BSP_Audio_set_volume_val(-1, ain.inputVolume, -1, ain.outputVolume);
	}
}

static enSDK_AUDIO_HW_SPEC ipcam_get_audio_hw_spec()
{
#define AUDIO_HW_SPEC_1X    100      // Hi3518Ev200+38∞Â+πË¬Û
#define AUDIO_HW_SPEC_2X    101      // Hi3518Ev200+38∞Â+ƒ£ƒ‚¬Û
#define AUDIO_HW_SPEC_3X    102      // Hi3518Ev200+≥§Ãı–Œ∞Â+ƒ£ƒ‚¬Û
#define AUDIO_HW_SPEC_4X    200      // Hi3516Dv100+38∞Â+πË¬Û
#define AUDIO_HW_SPEC_5X    300      // Hi3518Ev200+P2_720µ•sensor∞Â+∆’Õ®¬Û

    enSDK_AUDIO_HW_SPEC retVal = kSDK_AUDIO_HW_SPEC_IGNORE;
    int tmpAudioHwSpec = 0;
    ST_CUSTOM_SETTING custom;
    if(0 == CUSTOM_get(&custom)) {
        if(CUSTOM_check_int_valid(custom.function.audioHwSpec)) {
            tmpAudioHwSpec = custom.function.audioHwSpec;
            if(custom.function.audioHwSpec == AUDIO_HW_SPEC_1X) {
                retVal = kSDK_AUDIO_HW_SPEC_1X;
            }
            else if(custom.function.audioHwSpec == AUDIO_HW_SPEC_2X) {
                retVal = kSDK_AUDIO_HW_SPEC_2X;
            }
            else if(custom.function.audioHwSpec == AUDIO_HW_SPEC_3X) {
                retVal = kSDK_AUDIO_HW_SPEC_3X;
            }
            else if(custom.function.audioHwSpec == AUDIO_HW_SPEC_4X) {
                retVal = kSDK_AUDIO_HW_SPEC_4X;
            }
            else if(custom.function.audioHwSpec == AUDIO_HW_SPEC_5X) {
                retVal = kSDK_AUDIO_HW_SPEC_5X;
            }
            else {
                retVal = kSDK_AUDIO_HW_SPEC_IGNORE;
            }
        }
        else {
            retVal = kSDK_AUDIO_HW_SPEC_IGNORE;
        }
    }

    return retVal;
}

#define PIR_GPIO_DEVICE_NAME "PIRTaskModule"
#define PIR_RESULT_GET (1)

int  ReadPIRResult(void)
{
	int Result = 0;

#if defined(PIR_ALARM)

    int  tDev = open("/dev/"PIR_GPIO_DEVICE_NAME, O_RDONLY);
    if (tDev < 0) {
        printf("%s Open Failed\n", PIR_GPIO_DEVICE_NAME);
        return -1;
    }

	if (ioctl(tDev, PIR_RESULT_GET, &Result) < 0) {
		printf("ioctl Read Debug Err !!! \n");
		close(tDev);
		return -1;
	}

    close(tDev);

#endif

	return Result;
}

bool alarm_trigger;
pthread_t alarm_tid;
static void ipcam_get_alarm_in1()
{
#if defined(PIR_ALARM)

	pthread_detach(pthread_self());	
	prctl(PR_SET_NAME, "get_alarm_in1");
	int ret = -1;	
	ret = ReadPIRResult();
	while(alarm_trigger){
		int low_level_cnt = 0;
		ret = ReadPIRResult();
		if(ret == 1){	
			printf("io alarm start\r\n");
			low_level_cnt = 0;
			HICHIP_set_io_alarm_status(true);
			while(alarm_trigger && low_level_cnt++ < (3000 / 250)){
				if (!ReadPIRResult()){
					low_level_cnt ++;
				} else {
					low_level_cnt = 0;
				}
				usleep(250*1000);
			}
			printf("io alarm stop\r\n");
			HICHIP_set_io_alarm_status(false);	
		}else if(0 == ret){
			usleep(500*1000);
		}else {		
			sleep(1);
			printf("alarm return :-1  READ ERRO\r\n");	//-1 == ret    or ret == 0
		}
	}
    alarm_tid = (pthread_t)NULL;

#endif

	pthread_exit(NULL);
}

void ipcam_alarm_in_init()
{
#if defined(PIR_ALARM)

	int pir_gpio_grp = 3, pir_gpio_pin = 3, pir_triger = 0, pir_check = 0;
	char system_cammand[256];
    pthread_attr_t pthread_attr;
    int nRet;

	alarm_trigger = true;
#if defined(HI3516E_V1)
#define IRQF_TRIGGER_NONE		0x00000000
#define IRQF_TRIGGER_RISING		0x00000001
#define IRQF_TRIGGER_FALLING 	0x00000002
#define IRQF_TRIGGER_HIGH 		0x00000004
#define IRQF_TRIGGER_LOW 		0x00000008
#if defined(PIR_ON_EXPAND)
	//Êâ©Â±ïÊùøÁöÑÂÆö‰πâÂíåÊ®°ÁªÑ‰∏äÈù¢ÁöÑÂºïËÑö‰∏ç‰∏ÄÊ†∑
	pir_gpio_grp = 3, pir_gpio_pin = 0;
#else
	pir_gpio_grp = 3, pir_gpio_pin = 3;
#endif
    ST_NSDK_SYSTEM_SETTING sysSet;
    NETSDK_conf_system_get_setting_info(&sysSet);
    if(true == sysSet.capabilitySet.pir)
    {
        if(kNSDK_PIR_MANAGER_TRIGGER_RISING_EDGE == sysSet.pirManager.pirTrigger)
        {
            pir_triger = IRQF_TRIGGER_RISING;
            pir_check = 1;
        }
        else
        {
            pir_triger = IRQF_TRIGGER_FALLING;
            pir_check = 0;
        }
    }
    else
    {
        alarm_trigger = false;
        APP_TRACE("16e start pir failed! not custom pir(%d)", sysSet.capabilitySet.pir);
        return;
    }

	snprintf(system_cammand, sizeof(system_cammand), 
			"rmmod gpio_ext;insmod %s/mpp/juan_ko/gpio_ext.ko pir_gpio_grp=%d pir_gpio_pin=%d pir_triger=%d pir_check=%d",
			IPCAM_ENV_HOME_DIR, pir_gpio_grp, pir_gpio_pin, pir_triger, pir_check);
	NK_SYSTEM(system_cammand);
#endif

    nRet = pthread_attr_init(&pthread_attr);
    if(0 == nRet)
    {
        pthread_attr_setstacksize(&pthread_attr, 131072);
        pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);
        if(NULL == alarm_tid)
        {
            pthread_create(&alarm_tid, &pthread_attr, ipcam_get_alarm_in1, NULL);
        }

        pthread_attr_destroy(&pthread_attr);
    }

#endif

}


static void ipcam_alarm_in_destroy()
{	
#if defined(PIR_ALARM)

	alarm_trigger = false;

#endif

}

static int ipcam_producttestSetCustom()
{
    ST_PRODUCT_TEST_INFO info;
    ST_CUSTOM_SETTING custom;
    int setFlag = 0;

    if(0 != CUSTOM_get(&custom))
    {
        return -1;
    }

    /**
     *  pir custom
     */
    if(0 == PRODUCT_TEST_getCustom(&info))
    {
        if(1 == info.pir)
        {
            custom.function.pirEnabled = 1;
            custom.function.pirTrigger = (info.pirTrigger == 1) ? kNSDK_PIR_MANAGER_TRIGGER_RISING_EDGE : kNSDK_PIR_MANAGER_TRIGGER_FALLING_EDGE;
        }
        else
        {
            custom.function.pirEnabled = 0;
            custom.function.pirTrigger = info.pirTrigger;
        }
        setFlag = 1;
    }

    if(1 == setFlag)
    {
        CUSTOM_set(&custom);
        custom_conf_match();
    }

    return 0;

}

int IPCAM_init()
{
	int i = 0, ii = 0, ret = 0;
	SYSCONF_t *sysconf = NULL;

	NK_Log()->setLogLevel(NK_LOG_LV_INFO);
	NK_Log()->setTerminalLevel(NK_LOG_LV_ALERT);
	//NK_Log()->onFlush = onFlush;
	
	enSDK_AUDIO_HW_SPEC audioHwSpec;
	ipcam_signal_init();

	ret = IPCAM_network_init_for_upgrade();
	if(!ret){
		return 0;
	}

	//APP_RECOVER_init("192.168.1.169", IPCAM_ENV_DEFNETSDK_DIR, IPCAM_ENV_NETSDK_DIR);
	
	g_hardware_version = ipcam_authentication();

    audioHwSpec = ipcam_get_audio_hw_spec();
    if(g_hardware_version == 0x00010100) {
        ipcam_bsp_init(1, audioHwSpec);
    }
    else if(g_hardware_version == 0x00010000){
        ipcam_bsp_init(0, audioHwSpec);
    }
    else {
        ipcam_bsp_init(0, audioHwSpec);
    }
	TICKER_init();

#if defined(TS_RECORD)
    NK_TFCARD_Init(TFCARD_MMCBLK0P1_PATH, TFCARD_MOUNT_PATH);//-----------------Âü∫Êú¨ÂèØ‰ª•ÔºåÈô§‰∫ÜÈáåÈù¢Á∫øÁ®ãÁöÑÊìç‰Ωú
#else
#if defined(TFCARD)
	TFCARD_init(0, NULL);
#endif//defined(TFERCARD)
#endif

	SYSCONF_init(SOC_MODEL, PRODUCT_CLASS, "/dev/mtdblock3",
		SWVER_MAJ, SWVER_MIN, SWVER_REV, SWVER_EXT);


	//firmware
	FIRMWARE_init(getenv("FLASHMAP"), NULL);
	ST_CUSTOM_SETTING custom;
	if(0 == CUSTOM_get(&custom) && CUSTOM_check_string_valid(custom.model.oemNumber)){
		FIRMWARE_upgrade_set_oem_number(custom.model.oemNumber);
	}
	
	TICKER_init();

	ipcam_usrm_init();

	WATCHDOG_init(120);
	
	printf("%s\r\n", PRODUCT_CLASS);
	
	ipcam_sensor_init(SOC_MODEL);

	PRODUCT_TEST_init();

	APP_NETSDK_init();

	ipcam_producttestSetCustom();

	// init audio 
	SDK_init_audio(audioHwSpec, BSP_Speaker_Enable);
	ST_NSDK_AENC_CH aenc_ch;
	NETSDK_conf_aenc_ch_get(101, &aenc_ch);
	sdk_audio->init_ain(kSDK_AUDIO_SAMPLE_RATE_8000, 16, aenc_ch.codecType);
	//sdk_audio->set_aout_loop(0);
	//SearchFileAndPlay(SOUND_System_booting, NK_False);
    stSOUNG_QUEUE_API sqApi;
    memset(&sqApi, 0, sizeof(stSOUNG_QUEUE_API));
    sqApi.playSound = sdk_audio->playSound;
    sqApi.playG711A= sdk_audio->playG711A;
    sqApi.isAoBufFree= sdk_audio->isAoBufFree;
    sqApi.speakerEnable = BSP_Speaker_Enable;
    SOUND_initQueue(&sqApi, QUEUE_BUFFER_SIZE);
    ipcam_set_audio_volume();

	IPCAM_network_init2();
	start_smartlink();

	// sdk init video input
	SDK_init_vin(kSDK_VIN_HW_SPEC_IGNORE);
	for(i = 0; i < 1; ++i){
		ST_NSDK_VIN_CH vin_ch;
		NETSDK_conf_vin_ch_get(i + 1, &vin_ch);
		for(i = 0; ii < 4; ++ii){
			LP_NSDK_VIN_PRIVACY_MASK_REGION region = &vin_ch.privacyMask[ii];
			ST_SDK_VIN_COVER_ATTR cover;
			cover.enable = region->enabled;
			cover.x = region->regionX;
			cover.y = region->regionY;
			cover.width = region->regionWidth;
			cover.height = region->regionHeight;
			cover.color = region->regionColor | 0xff000000;

			//APP_TRACE("cover[%d] @ (%f,%f) %fx%f %s",
			//	ii, cover.x, cover.y, cover.width, cover.height, cover.enable ? "show" : "hide");
			
			
			sdk_vin->set_cover(i, ii, &cover);
		}
	}
	
	ipcam_mediabuf_avenc_init();
	
	APP_OVERLAY_init();
	
	// motion detection
	APP_MD_init(1); // only one channel motion detection

	//ipcam_gpio_init();
#if !defined(PIR_ON_EXPAND)
	if((0 == CUSTOM_get(&custom)) && (0 >= custom.function.pirEnabled)) // PIRÂíåPTZ‰∫íÊñ•
#else
    if(1) // PIRÂíåLEDÊåáÁ§∫ÁÅØ‰∫íÊñ•
#endif
	{
		ipcam_ptz_init(); //‰∫ëÂè∞
	}

#if defined(UART_PROTOCOL)
	APP_MOTOR_Init();
#endif
	//set upgrade env prepare function
	FIRMWARE_upgrade_env_callback_set(sdk_enc->upgrade_env_prepare);

	IPCAM_network_protocol_init();

	sdk_enc->start();
	sdk_vin->start();
	
	sdk_vin->set_md_trap(0, IPCAM_network_md_handle);
	sdk_vin->set_md_ref_freq(0, 1);
	//sdk_audio->set_aout_loop(0);
	//sdk_audio->set_aout_play(0);
	
#if defined(SDCARD)
	// SD-Card ready
	SDCARD_init("/dev/mmcblk0p1", "/media/sdcard", "/tmp/sdcard.cache", APP_SDCARD_recorder);
#endif//defined(SDCARD)

#ifdef REBOOT_ONTIME
	//–Ë“™‘⁄tfcard÷Æ«∞∆Ù∂Ø£¨ª÷∏¥ ±º‰
	REBOOT_ONTIME_init(-1);//-1 : hourNum π”√ƒ¨»œ ±º‰,º¥Œ™¡Ë≥ø¡Ωµ„’˚÷ÿ∆Ù…Ë±∏
#endif

#if defined(TS_RECORD)
    NK_TFRECORD_Start(0, EN_RECORD_TYPE_TIMER);
#else
#if defined(TFCARD)
    TFCARD_start_record_thread();
#endif//defined(TFERCARD)
#endif

funBackCall callBackFour = NULL;
#if defined (PX_720)
    callBackFour = KEY_REC_control;
#endif
	ipcam_check_version_and_upgrade();
	initKeyTime(key_press, reset_default_factory_by_key, NULL, NULL, callBackFour);

	IPCAM_timer_init();

	ipcam_alarm_in_init();

#ifdef VIDEO_CTRL
    if(true == GLOBAL_sn_front()) {
        VIDEO_CTRL_init();
    }
#endif

#if defined (LED_PWM)
    LED_PWM_init();
#endif

	GLOBAL_setOldTypeRecordFlag();

#if !defined(TS_RECORD)
#if defined(TFCARD)
    NK_TFCARD_turn_on_status_check();
#endif//defined(TFERCARD)
#endif

	network_4g_product_test();

    GLOBAL_cpuDetectInit();

#if defined(CLOUD_REC)
    char sn_str[32] = {0};
    ST_NSDK_NETWORK_OSSCLD osscld;
    if (NETSDK_conf_osscloud_get(&osscld)) {
        if(true == osscld.channel[0].stream[0].enable)
        {
            if(0 == GLOBAL_GetID(sn_str)) {
                NK_CLOUD_REC_Init(sn_str, strlen(sn_str), 1);
            }

            NK_CLOUD_REC_Update(&osscld);
        }
    }
#endif

#if defined(MSG)
    ESEE_msg_push_init();
#endif

	return 0;
}

void IPCAM_destroy()
{
	APP_TRACE("at IPCAM_destroy...");

#if defined(UART_PROTOCOL)
    APP_MOTOR_Exit();
#endif

	ipcam_alarm_in_destroy();
	PTZ_Destroy();

	// SD-Card finished
#if defined(SDCARD)
	SDCARD_destroy();
#endif//defined(SDCARD)

#if defined(TS_RECORD)
    NK_TFRECORD_Stop(0);
#else
#if defined(TFCARD)
	//TFER_destroy();
	TFCARD_destroy();
#endif//defined(TFERCARD)
#endif

#if defined(CLOUD_REC)
    NK_CLOUD_REC_Deinit();
#endif

    SOUND_releaseQueue();

	IPCAM_network_destroy();
	IPCAM_timer_destroy();
	TICKER_destroy();
	FIRMWARE_destroy();
	ipcam_mediabuf_avenc_destroy();
	sdk_audio->release_ain_ch(0);
	sdk_audio->destroy_ain();
	SDK_destroy_audio();
	//LVIEW_destroy();
	SENSOR_destroy();
	SDK_destroy_sys();
	WATCHDOG_destroy();
	ipcam_signal_destroy();
	//SDK_destroy_vin();//fix me :call alert
	APP_TRACE("IPCAM_destroy done");
}

void SDK_destroy()
{
#if defined(TFCARD)
    //TFCARD_destroy();
#endif
	IPCAM_timer_sdk_destroy();	
	APP_OVERLAY_destroy(); 	 
	SDK_ENC_destroy();
}


int IPCAM_exec()
{
	usleep(200000);
	return 0;
	char inbuf[128];
#define _IAPP_EXEC_POLL_INPUT() \
	(memset(inbuf, 0, sizeof(inbuf)), fgets(inbuf, sizeof(inbuf) - 1, stdin))
#define _IAPP_EXEC_MATCH_INPUT(cmd) \
	(0 == strncasecmp(inbuf, (cmd), strlen(cmd)))

	if(_IAPP_EXEC_POLL_INPUT()){
		if(_IAPP_EXEC_MATCH_INPUT("quit")){
			return -1;
		}
		
		
		return 0;
	}
	printf("stdin error\n");
	exit(1);
	return -1;
}

void IPCAM_Info()
{
	char sn_string[UC_SN_MAX_LEN] = {0};
	UC_SNumberGet(sn_string);

	SYSCONF_init(SOC_MODEL, PRODUCT_CLASS, "/dev/mtdblock3",
		SWVER_MAJ, SWVER_MIN, SWVER_REV, SWVER_EXT);
	SYSCONF_t *sysconf = SYSCONF_dup();
	printf("Device Name: %s\r\n", sysconf->ipcam.info.device_name);
	printf("Device Model: %s\r\n", sysconf->ipcam.info.device_model);
	printf("Device ID: %s\r\n", sn_string);
	printf("Device Software Version: %d.%d.%d %s\r\n", SWVER_MAJ, SWVER_MIN, SWVER_REV, SWVER_EXT);
	printf("\r\n");
	printf("Camera: %d\r\n", sysconf->ipcam.spec.vin);
	printf("Audio: %d\r\n", sysconf->ipcam.spec.ain);
	printf("Sensor: %d\r\n", sysconf->ipcam.spec.io_sensor);
	printf("Alarm: %d\r\n", sysconf->ipcam.spec.io_alarm);
	printf("Hard Disk Driver: 0\r\n");
	printf("Series Code: %s\r\n", SERISE_CODE);
	printf("\r\n\r\n");
}



static void ipcam_process_arg(int argc, char *argv[])
{
	int i = 0, c = 0;
	struct option long_opt[] = {
		{
			.name = 	"webdir",
			.has_arg = 1,
			.flag = NULL,
			.val = 'w',
		},
		{
			.name = 	"fontdir",
			.has_arg = 1,
			.flag = NULL,
			.val = 'f',
		},
		{
			.name = 	"flashmap",
			.has_arg = 1,
			.flag = NULL,
			.val = 'm',
		},
		{
			.name = 	"sysconf",
			.has_arg = 1,
			.flag = NULL,
			.val = 's',
		},
		{
			.name = 	"defnetsdk",
			.has_arg = 1,
			.flag = NULL,
			.val = 'd',
		},
		{
			.name = 	"netsdk",
			.has_arg = 1,
			.flag = NULL,
			.val = 'n',
		},
		{
			.name = 	"ispcfg",
			.has_arg = 1,
			.flag = NULL,
			.val = 'i',
		},
		// end
		{
			.name = 	NULL,
			.has_arg = 0,
			.flag = NULL,
			.val = 0,
		},
	};
	char short_opt[(sizeof(long_opt) / sizeof(long_opt[0])) * 2] = {""};
	
	// make the short option string
	for(i = 0; i < sizeof(long_opt) / sizeof(long_opt[0]); ++i){
		struct option *l_opt = long_opt + i;
		if(!l_opt->name){
			break;
		}

		strncat(short_opt, (char*)(&l_opt->val), 1);
		if(1 == l_opt->has_arg){
			strcat(short_opt, ":");
		}
	}

	// set the default environment
	setenv("WEBDIR", IPCAM_ENV_WEB_DIR, 1);
	setenv("FONTDIR", IPCAM_ENV_FONT_DIR, 1);
	setenv("FLASHMAP", IPCAM_ENV_FLASH_MAP, 1);
	setenv("SYSCONF", IPCAM_ENV_SYSCONF, 1);
	setenv("DEFNETSDK", IPCAM_ENV_DEFNETSDK_DIR, 1);
	setenv("NETSDK", IPCAM_ENV_NETSDK_DIR, 1);
	setenv("ISPCFG", IPCAM_ENV_ISPCFG_DIR, 1);

	// read the options
	while((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1){
		switch(c){
		case 'w':
			setenv("WEBDIR", optarg, 1);
			break;
		case 'f':
			setenv("FONTDIR", optarg, 1);
			break;
		case 'm':
			setenv("FLASHMAP", optarg, 1);
			break;
		case 's':
			setenv("SYSCONF", optarg, 1);
			break;
		case 'd':
			setenv("DEFNETSDK", optarg, 1);
			break;
		case 'n':
			setenv("NETSDK", optarg, 1);
			break;
		case 'i':
			setenv("ISPCFG", optarg, 1);
			break;
		default:
			;
		}
	}

	
	//system("printenv");exit(1);
	
}

static int ipcam_snumber_init()
{
	ST_MODEL_CONF model_conf;
	g_sensor_type = SDK_ISP_sensor_check();
	MODEL_CONF_init(g_sensor_type);
	
	if(NULL != MODEL_CONF_get(&model_conf)){
		//read SN check param from config file
		UC_SNumberInit(model_conf.snumber.chipModel, model_conf.snumber.productModel);
	}else{
		//use default decription
		if(0 == strcmp(SOC_MODEL, "HI3518E_V2")) {
			UC_SNumberInit(0x3518E200, 0x00000200);
		}
		if(0 == strcmp(SOC_MODEL, "HI3516C_V2")) {
			UC_SNumberInit(0x3518E200, 0x00000200);
		}
		if((0 == strcmp(SOC_MODEL, "HI3516A"))
		|| (0 == strcmp(SOC_MODEL, "HI3516D"))) {
			UC_SNumberInit(0x3516D100, 0x00000400);
		}
		if((0 == strcmp(SOC_MODEL, "HI3518E"))
		|| (0 == strcmp(SOC_MODEL, "HI3518C"))) {
			UC_SNumberInit(0x3518E100, 0x00000200);
		}
		if((0 == strcmp(SOC_MODEL, "M388C"))) {
			UC_SNumberInit(0x388C0000, 0x00000200);
		}
	}
	if(0 == UC_SNGetODMNumber(&g_encryp_chip_odm1, &g_encryp_chip_odm2)){
		printf("sn odm=%x-%x\n", g_encryp_chip_odm1, g_encryp_chip_odm2);
	}

    if(NK_False == GLOBAL_sn_front()) {
        MODEL_CONF_destory();
        MODEL_CONF_init(g_sensor_type);
    }

	return 0;
}

int IPCAM_main(int argc, char** argv)
{
	ipcam_process_arg(argc, argv);
	SDK_init_sys(PRODUCT_CLASS);


    //custom conf:need to do first because of OEM number(PRODUCT_MODEL) for firmware upgrade
	CUSTOM_init();

	ipcam_snumber_init();
	FISHEYE_config_init();
	
	GLOBAL_event_lock_init();

	if(strstr(argv[0], "READ_INFO") != NULL)
	{
		IPCAM_Info();
		exit(0);
	}
	else if(strstr(argv[0], "WSERIAL_NUM") != NULL)
	{
		UC_SNumberSet(argv[1], strlen(argv[1]));
		printf("write serial number finish\n");
		exit(0);
	}
	else if(strstr(argv[0], "RSERIAL_NUM") != NULL)
	{
		char ucode[32];
		if(0 == UC_SNumberGet(ucode))
		{
			printf("serial number:%s\n", ucode);
		}
		else
		{
			printf("serial number NOT init\n");
		}
		exit(0);
	}
	else if(strstr(argv[0], "RRTC") != NULL)
	{
		rtc_time_t rtc_tm;
		RTC_gettime(&rtc_tm);
	}
	else if(strstr(argv[0], "WRTC") != NULL && argv[1] != NULL)
	{
		struct timeval tv;
		//printf("%s\r\n", argv[1]);		
		time_t curtime = (time_t)atoi(argv[1]);
		tv.tv_sec = curtime;
		settimeofday(&tv,NULL);
		printf("int:%d\r\n", curtime);
		RTC_settime((time_t)tv.tv_sec);
	}
	else
	{
		printf("enter main application\r\n");
		//IPCAM_Info();
		GLOBAL_setTimeFromFile();   // ≥Ã–Ú∆Ù∂Ø ±÷¥––“ª¥Œª÷∏¥ ±º‰µƒ≤Ÿ◊˜
        RTC_sync_to_system();
		atexit(SDK_destroy);
		atexit(GLOBAL_exit);
		IPCAM_init();
		//atexit(IPCAM_destroy);
		while(0 == IPCAM_exec());
		IPCAM_destroy();
	}

	exit(0);
}

