#include "danavideo.h"
#include "danavideo_cmd.h"
#include "debug.h"
#include "mediatyp.h"
#include "errdefs.h"

#include "dana_lib.h"
#include "generic.h"
#include "securedat.h"
#include "socket_tcp.h"
#include "http_util.h"
#include "sdk/sdk_api.h"
#include "netsdk.h"
#include "netsdk_def.h"
#include "media_buf.h"
#include "sdk/sdk_enc.h"
#include "netsdk_private.h"
#include "generic.h"
#include <sys/prctl.h>


#define DANACONF_DEFPATH "/media/conf/"
#define DANACONF_PATH "/tmp/"
#define DANACONF_FILENAME "danale.conf"
#define DANASCHEMA "8HJA"
#define DANAPRODUCT "8H8C01"
#define DANAIDQRCODE "/tmp/dana_id.bmp"

typedef struct _MyData {
	pdana_video_conn_t *danavideoconn;	
    volatile bool run_media;
    volatile bool exit_media;
	volatile bool change_media;
    pthread_t thread_media;
    uint32_t chan_no;    
    char appname[32];
	volatile bool run_audio;
	uint32_t video_quality;
}MYDATA;


uint32_t danavideoconn_created(void *arg);
void danavideoconn_aborted(void *arg);
void danavideoconn_command_handler(void *arg, dana_video_cmd_t cmd, uint32_t trans_id, void *cmd_arg, uint32_t cmd_arg_len);
void th_media(void *arg);
int dana_pic_media( uint32_t ch_no,void *arg);
void th_audio(void *arg);
void Dana_back_handle();
static int dana_get_alarm_status(int ch_no);
void danavideo_autoset(const uint32_t power_freq, const int64_t now_time, const char *time_zone, const char *ntp_server1, const char *ntp_server2);
static boolean Dana_Settime(const int64_t now_time, const char *time_zone);
static int dana_SetVideoResolution(int id,uint32_t video_quality, MYDATA *mydata);
static char local_time_zone[32] = {"CST(China)"};


static bool lib_danavideo_inited = false, lib_danavideo_started = false;
static int alarm_status =0;

dana_video_callback_funs_t ipc_dana_callbackfuns ={
	.danavideoconn_created = danavideoconn_created,
	.danavideoconn_aborted = danavideoconn_aborted,
	.danavideoconn_command_handler = danavideoconn_command_handler,
};

void PRINTF_DANA(){
	dbg("/////////////////////////////////////////////////////////////////////////////////////////////////\n");
	dbg("//                                         DANA                                                //\n");
	dbg("/////////////////////////////////////////////////////////////////////////////////////////////////\n");
}

static int esee_dana_get_nonce(char *nonce, int nonce_length, char *server_ip)
{
	int ret;
	int receive_file_size = 0;
	char text[2048];
	LP_HTTP_HEAD_FIELD snd_header = NULL;
	LP_HTTP_HEAD_FIELD rcv_header = NULL;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp_r(&sockTCP);
	//set socket time out
	//tcp->set_send_timeout(tcp, 3, 0);
	//tcp->set_recv_timeout(tcp, 3, 0);

	snd_header = HTTP_UTIL_new_request_header("http", "1.1", "GET", "/ESeeViaDanale/getNonce", NULL);
	snd_header->add_tag_text(snd_header, "Authorization", "Basic admin:", true);
	//snd_header->add_tag_text(snd_header, "User-Agent", "JUAN SREACHER", true);
	snd_header->add_tag_text(snd_header, "Host", server_ip, true);
	snd_header->add_tag_text(snd_header, "Connection", "Keep-Alive", true);
	snd_header->add_tag_text(snd_header, "Accept", "*/*", true);
	snd_header->to_text(snd_header, text, sizeof(text));	


	//printf("REQUEST size %d:\n%s\r\n", strlen(text), text);

	ret = tcp->connect(tcp, server_ip, 80);
	if(ret < 0){
		printf("connect %s failed!\r\n", server_ip);
		goto ERR_EXIT1;
	}

	ret = tcp->send2(tcp, text, strlen(text), 0);
	if(ret < 0){
		printf("send recover.cgi failed!\r\n");
		goto ERR_EXIT1;
	}

	if(-1 == HTTP_UTIL_recv_response_message(tcp->sock, nonce, nonce_length)){
		goto ERR_EXIT1;
	}
	/*if(NULL == (rcv_header = HTTP_UTIL_recv_response_header(tcp->sock))){
		printf("receive recover.cgi failed!\r\n");
		goto ERR_EXIT1;
	}

	if(200 == rcv_header->status_code){
		char *pRev_length = rcv_header->read_tag(rcv_header, "Content-Length");
		if(NULL != pRev_length){
			// receive the content of http request
			receive_file_size = atoi(pRev_length);
			ret = tcp->recv(tcp, text, receive_file_size, 0);
			if(ret == receive_file_size){
				text[32] = 0;
				memcpy(nonce, text, strlen(text));
			}else{
				goto ERR_EXIT1;
			}
		}else{
			//printf("pRev_length %d!\r\n", pRev_length);
			goto ERR_EXIT1;
		}
	}else{
		printf("status code error %d!\r\n", rcv_header->status_code);
		goto ERR_EXIT1;
	}*/

	tcp->close(tcp);
	if(snd_header){
		snd_header->free(snd_header);
	}
	if(rcv_header){
		rcv_header->free(rcv_header);
	}
	snd_header = NULL;
	rcv_header = NULL;

	//pthread_exit(NULL);
	return 0;
		
ERR_EXIT1:
	if(snd_header) snd_header->free(snd_header);
	if(rcv_header) rcv_header->free(rcv_header);
	if(tcp->sock > 0) tcp->close(tcp);
	
	return -1;	


}


static int esee_dana_get_config_file(char *nonce, int nonce_length, char *server_ip)
{
	int ret;
	int receive_size = 0, receive_size_cnt = 0, receive_file_size = 0;
	char config_file[64];
	char text[2048];
	char query_buf[256];
	char sn_id[32];
	char chip[8];
	char *signature = NULL;
	LP_HTTP_HEAD_FIELD snd_header = NULL;
	LP_HTTP_HEAD_FIELD rcv_header = NULL;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp_r(&sockTCP);
	//set socket time out
	//tcp->set_send_timeout(tcp, 3, 0);
	//tcp->set_recv_timeout(tcp, 3, 0);

	char sn_str[32] = {0};
	char esee_id[32] = {0};
	if(0 == UC_SNumberGet(sn_id)) {
	}

	//get soc
	ST_NSDK_SYSTEM_DEVICE_INFO device_info;
	NETSDK_conf_system_get_device_info(&device_info);
	char hi3516c_id[16],hi3518a_id[16], hi3518c_id[16], hi3518e_id[16], hi3516a_id[16];
	
	sprintf(hi3516c_id, "%x", SDK_HI3516C_V100);
	sprintf(hi3518a_id, "%x", SDK_HI3518A_V100);
	sprintf(hi3518c_id, "%x", SDK_HI3518C_V100);
	sprintf(hi3518e_id, "%x", SDK_HI3518E_V100);
	sprintf(hi3516a_id, "%x", SDK_HI3516A_V100);
	

	if(!strcmp(device_info.soc, hi3516c_id)){
		sprintf(chip, "H16C");
	}else if(!strcmp(device_info.soc,hi3518a_id)){
		sprintf(chip, "H18C");
	}else if(!strcmp(device_info.soc, hi3518c_id)){
		sprintf(chip, "H18C");
	}else if(!strcmp(device_info.soc, hi3518e_id)){
		sprintf(chip, "H18E");
	}else if(!strcmp(device_info.soc, hi3516a_id)){
		sprintf(chip, "H16A");
	}else{
		sprintf(chip, "H18C");
	}

	sprintf(text, "JA%s%s%s", sn_id, chip, nonce);
	printf("signature:%s\n", text);
	signature = md5sum_buffer(text, strlen(text));
	//printf("signatureMD:%s\n", signature);
	snprintf(query_buf, sizeof(query_buf), "schema=%s&chip=%s&product=%s&id=JA%s&signature=%s", 
		DANASCHEMA,chip, DANAPRODUCT, sn_id, signature);
	//printf("query_buf:%s\n", query_buf);
	snd_header = HTTP_UTIL_new_request_header("http", "1.1", "GET", "/ESeeViaDanale/getLicense", query_buf);
	//snd_header->add_tag_text(snd_header, "Authorization", "Basic admin:", true);
	//snd_header->add_tag_text(snd_header, "User-Agent", "JUAN SREACHER", true);
	snd_header->add_tag_text(snd_header, "Host", server_ip, true);
	snd_header->add_tag_text(snd_header, "Connection", "Keep-Alive", true);
	snd_header->add_tag_text(snd_header, "Accept", "*/*", true);
	snd_header->to_text(snd_header, text, sizeof(text));	


	//printf("REQUEST size %d:\n%s\r\n", strlen(text), text);

	ret = tcp->connect(tcp, server_ip, 80);
	if(ret < 0){
		printf("connect %s failed!\r\n", server_ip);
		goto ERR_EXIT1;
	}

	ret = tcp->send2(tcp, text, strlen(text), 0);
	if(ret < 0){
		printf("send recover.cgi failed!\r\n");
		goto ERR_EXIT1;
	}

	sprintf(config_file, "%s%s", DANACONF_DEFPATH, DANACONF_FILENAME);
	if(-1 == HTTP_UTIL_recv_response_file(tcp->sock, config_file)){
		goto ERR_EXIT1;
	}
	
	tcp->close(tcp);
	if(snd_header){
		snd_header->free(snd_header);
	}
	if(rcv_header){
		rcv_header->free(rcv_header);
	}
	snd_header = NULL;
	rcv_header = NULL;

	//pthread_exit(NULL);
	return 0;
		
ERR_EXIT1:
	if(snd_header) snd_header->free(snd_header);
	if(rcv_header) rcv_header->free(rcv_header);
	if(tcp->sock > 0) tcp->close(tcp);
	
	return -1;	


}


static int dana_config_file_init()
{
	int ret = 0;
	char filepath[128], def_filepath[128];
	char nonce[64];
	char server_ip[32] = "";
	sprintf(filepath, "%s%s", DANACONF_PATH, DANACONF_FILENAME);
	if(!IS_FILE_EXIST(filepath)){
		printf("file:%s is not exist\r\n", filepath);
		sprintf(def_filepath, "%s%s", DANACONF_DEFPATH, DANACONF_FILENAME);
		if(!IS_FILE_EXIST(def_filepath)){
			struct hostent *host = NULL;	
			while(1){
				if(NULL == host){
					host = gethostbyname("alias.msndvr.com");		
					if(NULL == host)
					{
						//printf("Get Esee's addr failed\n");	
						sleep(1);
						continue;
					}
					sprintf(server_ip, "%s",inet_ntoa(*(struct in_addr*)host->h_addr));
					printf("dana config file server:%s\n", server_ip);
				}
				//sprintf(server_ip, "192.168.1.2");
				if(0 == esee_dana_get_nonce(nonce,sizeof(nonce), server_ip)){
					if(0 == esee_dana_get_config_file(nonce, sizeof(nonce), server_ip)){
						//get dana config file from esee server successfully
						break;
					}
				}
				sleep(10);
			}
		}
		COPY_FILE(def_filepath, filepath);
	}
}

static void generate_QRcode(char *pInput, char* pOutput_file)
{
	char shell_cmd[256];
	if(pInput&&pOutput_file&&!IS_FILE_EXIST(pOutput_file)){
		///usr/share/ipcam/ja_tools/zint -o ./resource/web/images/dana_esee.bmp -b 58 --border=1 --scale=2.0 -d http://www.dvr163.com/download/mdanale.php
		snprintf(shell_cmd, sizeof(shell_cmd), "%s/ja_tools/zint -o %s -b 58 --border=1 --scale=2.0 -d \"%s\"",
			IPCAM_ENV_HOME_DIR, pOutput_file, pInput);
		NK_SYSTEM(shell_cmd);
	}
}

static void dana_init(){  
		//dbg_on();
		//PRINTF_DANA();	
//		lib_danavideo_set_hb_is_ok(danavideo_hb_is_ok);
//		lib_danavideo_set_hb_is_not_ok(danavideo_hb_is_not_ok);
	
		lib_danavideo_set_autoset(danavideo_autoset);
	
		 char *danale_path =  DANACONF_PATH;    
		 char *agent_user =NULL;    
		 char *agent_pass = NULL;
		 char *chip_code = NULL; 	
		 char *schema_code = NULL;  
		 char *product_code = NULL;
		 uint32_t libdanavideo_startup_timeout_sec = 30;

		 lib_danavideo_set_startup_timeout(libdanavideo_startup_timeout_sec);
//		 dbg("testdanavideo lib_danavideo_deviceid: %s\n", lib_danavideo_deviceid_from_conf(danale_path));
		while (lib_danavideo_init(danale_path, agent_user, agent_pass, chip_code, schema_code, product_code, &ipc_dana_callbackfuns) == false) {
			while(!lib_danavideo_create_on_check_conf()) { 
				dbg("testdanavideo lib_danavideo_create_on_check_conf failed, try again\n"); 
				printf("lib_danavideo_init failed\n");
				return ;
			}  
//			dbg("testdanavideo lib_danavideo_create_on_check_conf succeeded\n");	
		}
		lib_danavideo_inited = true;	
		//ESEE_CLIENT_restart_by_dana(lib_danavideo_deviceid());
		// lib_danavideo_init成功之后可以调用lib_danavideo_deviceid()获取设备标识码    
		dbg(" lib_danavideo_deviceid: %s\n", lib_danavideo_deviceid());
		generate_QRcode(lib_danavideo_deviceid(), DANAIDQRCODE);
		/*if (!lib_danavideo_start()) {		 
			printf(" lib_danavideo_start failed\n");		
			lib_danavideo_deinit(); 
			lib_danavideo_started = false;
			lib_danavideo_inited = false;	
			return ;	  
		}	 */
		/*if (!lib_danavideo_start()) {		 
			printf("%d-lib_danavideo_start failed\n", __LINE__);		
			lib_danavideo_deinit(); 
			lib_danavideo_started = false;
			lib_danavideo_inited = false;	
			return ;	  
		}*/
		while(!lib_danavideo_start()){
			printf("lib_danavideo_start failed\n");
			//lib_danavideo_set_startup_timeout(libdanavideo_startup_timeout_sec);
			sleep(1);
		}
		dbg(" lib_danavideo_start succeed\n");	 
		lib_danavideo_started = true;
	
		danavideo_device_type_t device_type = DANAVIDEO_DEVICE_IPC;    
		uint32_t channel_num = 0;	 
		char *rom_ver = "IPCAM";	  
		char *api_ver = "IPCAM";	 
		char *rom_md5 = "7efab09e5ce7542f99930fb20e4862f9";//"rom_md5";    
		if (lib_danavideo_util_setdeviceinfo(device_type, channel_num, rom_ver, api_ver, rom_md5)) {		
			printf("\x1b[32m  IPC lib_danavideo_util_setdeviceinfo success\x1b[0m\n");	  
		} else {		
			printf("\x1b[34m IPC lib_danavideo_util_setdeviceinfo failed\x1b[0m\n");   
		}
		
		alarm_status = dana_get_alarm_status(1);

	while(1){
		dbg("11111111111111111111111111111111111111111\n");
		sleep(200000);
	}
 	
}

static dana_proc()
{
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, "dana_proc");
	//dana file init
	dana_config_file_init();
	dana_init();
}

void DanaLib_init(){
	setenv("DANA_STREAM", "720p.264", true);
	pthread_t dana_pthread;
	pthread_create(&dana_pthread,NULL,dana_proc,NULL);
}

void DanaLib_destroy(){
    if (lib_danavideo_started) {
		lib_danavideo_stop();        
		lib_danavideo_started = false;    
	}    
		usleep(800*1000);    
	if (lib_danavideo_inited) { 
		lib_danavideo_deinit();        
		lib_danavideo_inited = false;    
	}   
		usleep(800*1000);
	dbg("testdanavideo exit\n");
}

MYDATA mydata_liry ;
uint32_t danavideoconn_created(void *arg){
	printf("danavideoconn_created\n");
	pdana_video_conn_t *danavideoconn = (pdana_video_conn_t *)arg; 
	if(NULL == danavideoconn){
		return -1;
	}
	MYDATA *mydata = &mydata_liry; 
	strncpy(mydata->appname, "juan ipc", strlen("juan ipc"));
	if (NULL == mydata) {        
		dbg(" danavideoconn_created failed calloc\n");       
		return 0;    
	}
	mydata->thread_media = -1;
	mydata->danavideoconn = danavideoconn;
	mydata->chan_no = -1;	
	mydata->run_audio = false;
	mydata->run_media = false;
	mydata->exit_media = true;
	if (0 != lib_danavideo_set_userdata(danavideoconn, mydata)) {        
		dbg(" danavideoconn_created lib_danavideo_set_userdata failed\n");        
		mydata = NULL;		
		return -1;    
	}
	 return 0; 
}

void danavideoconn_aborted(void *arg){
	printf("danavideoconn_aborted\n");	
	pdana_video_conn_t *danavideoconn = (pdana_video_conn_t *)arg;
		MYDATA *mydata;
		if (0 != lib_danavideo_get_userdata(danavideoconn, (void **)&mydata)) {
			dbg("danavideoconn_aborted lib_danavideo_get_userdata failed, mem-leak\n");
			return;
		}
		
		if (NULL != mydata) {
			// stop video thread
			mydata->run_media = false;
			mydata->run_audio= false;	
			mydata->exit_media = true;
			mydata->danavideoconn = NULL;
			dbg("danavideoconn_aborted APPNAME: %s\n", mydata->appname);
			pthread_join(mydata->thread_media,NULL);
			mydata = NULL;
		}
		
		dbg(" danavideoconn_aborted succeed\n");	
	return ;
}

void danavideoconn_command_handler(void *arg, dana_video_cmd_t cmd, uint32_t trans_id, void *cmd_arg, uint32_t cmd_arg_len){
	printf("danavideoconn_command_handler start %d\n",cmd);
	pdana_video_conn_t *danavideoconn = (pdana_video_conn_t *)arg;
	MYDATA *mydata;
	if (0 != lib_danavideo_get_userdata(danavideoconn, (void**)&mydata)) {       
		printf(" danavideoconn_command_handler lib_danavideo_get_userdata failed\n");        
		return;    
	}

	 uint32_t error_code;    
	 char *code_msg;
	
	 switch(cmd){
		case DANAVIDEOCMD_DEVDEF:                            //恢复出厂设置
		{
			error_code = 0;                
			code_msg = (char *)"DEFAULT";               
			NETSDK_conf_system_operation(eNSDK_SYSTEM_OPERATION_DEFAULT);
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {                    
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_DEVDEF send response succeed\n");                
			} else {                    
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_DEVDEF send response failed\n");                

			}

		}
			break;
		case DANAVIDEOCMD_DEVREBOOT:                         //通知设备端设备重启
		{
			error_code = 0;
			code_msg = (char *)"system rebooting...";
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) 
			{                   
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT send response succeed\n");
			} else {                    
			dbg(" danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT send response failed\n");
			}
			NETSDK_conf_system_operation(eNSDK_SYSTEM_OPERATION_REBOOT);
		}
			break;
		case DANAVIDEOCMD_GETSCREEN:						 //获取抓取图片
		{
			dbg(" danavideoconn_command_handler DANAVIDEOCMD_GETSCREEN\n");                
			DANAVIDEOCMD_GETSCREEN_ARG *getscreen_arg = (DANAVIDEOCMD_GETSCREEN_ARG *)cmd_arg;
			error_code = -1;                
			code_msg = (char *)"";
			error_code = dana_pic_media(getscreen_arg->ch_no,arg);					
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {                    
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT send response succeed\n");                
			} else {                    
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT send response failed\n");                
			} 
		}
		break;
		case DANAVIDEOCMD_GETALARM:
		{                
			dbg("danavideoconn_command_handler DANAVIDEOCMD_GETALARM\n");
			DANAVIDEOCMD_GETALARM_ARG *getalarm_arg = (DANAVIDEOCMD_GETALARM_ARG *)cmd_arg;
			dbg("getalarm_arg\n");
			dbg("ch_no: %u\n", getalarm_arg->ch_no); 
			dbg("\n");                
			error_code = 0;
			code_msg = (char *)""; 
			uint32_t motion_detection = 0;
			uint32_t opensound_detection = 0;
			uint32_t openi2o_detection = 0;
			uint32_t smoke_detection = 0;
			uint32_t shadow_detection = 0; 
			uint32_t other_detection = 0; 
			motion_detection = dana_get_alarm_status(getalarm_arg->ch_no);
			alarm_status = motion_detection;
			if (lib_danavideo_cmd_getalarm_response(danavideoconn, trans_id, error_code, code_msg, motion_detection, opensound_detection, openi2o_detection, smoke_detection, shadow_detection, other_detection)) {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETALARM send response succeed\n");
			} else {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETALARM send response failed\n");
			}            
		}            
		break; 
    	case DANAVIDEOCMD_GETBASEINFO:
		{
			error_code = 0; 
			code_msg = (char *)"";
			char *dana_id = NULL;
			char *api_ver = NULL;
			char *sn      = NULL;
			char *device_name = NULL;
			char *rom_ver = NULL;
			uint32_t device_type = DANAVIDEO_DEVICE_IPC;
			uint32_t ch_num = 0;
			uint64_t sdc_size = 0;
			uint64_t sdc_free = 0;
			ST_NSDK_SYSTEM_DEVICE_INFO device_info;
			NETSDK_conf_system_get_device_info(&device_info);
			sn = device_info.serialNumber;
			device_name = device_info.deviceName;
			rom_ver  = device_info.firmwareVersion;
			dana_id = device_info.serialNumber;
			api_ver = device_info.firmwareVersion;
			
			if (lib_danavideo_cmd_getbaseinfo_response(danavideoconn, trans_id, error_code, code_msg, dana_id, api_ver, sn, device_name, rom_ver, device_type, ch_num, sdc_size, sdc_free)) 
			{
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETBASEINFO send response succeed\n");
			} else {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETBASEINFO send response failed\n");
			}
		}break;
    	case DANAVIDEOCMD_GETCOLOR:
		{
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCOLOR\n");   
			DANAVIDEOCMD_GETCOLOR_ARG *getcolor_arg = (DANAVIDEOCMD_GETCOLOR_ARG *)cmd_arg;
			error_code = 0;
			code_msg = (char *)"";
			uint32_t brightness = 1;
			uint32_t contrast = 1;                
			uint32_t saturation = 1;
			uint32_t hue = 1; 
			ST_NSDK_VIN_CH vin_ch;
			NETSDK_conf_vin_ch_get(getcolor_arg->ch_no, &vin_ch);
			brightness = vin_ch.brightnessLevel;
			contrast = vin_ch.contrastLevel;
			saturation = vin_ch.saturationLevel;
			hue = vin_ch.hueLevel;
			if (lib_danavideo_cmd_getcolor_response(danavideoconn, trans_id, error_code, code_msg, brightness, contrast, saturation, hue))
			{ 
				dbg("danavideoconn_command_handler DANAVIDEOCMD_GETCOLOR send response succeed\n");
			} else {  
				dbg("danavideoconn_command_handler DANAVIDEOCMD_GETCOLOR send response failed\n");
			}            
		}            
		break; 
    	case DANAVIDEOCMD_GETFLIP:
		{
			dbg(" danavideoconn_command_handler DANAVIDEOCMD_GETFLIP\n");
			DANAVIDEOCMD_GETFLIP_ARG *getflip_arg = (DANAVIDEOCMD_GETFLIP_ARG *)cmd_arg;
			dbg("getflip_arg_arg\n");                
			dbg("ch_no: %u\n", getflip_arg->ch_no);
			dbg("\n");                
			error_code = 0;
			code_msg = (char *)""; 
			uint32_t flip_type = 0; 
			ST_NSDK_VIN_CH vin_ch;
			NETSDK_conf_vin_ch_get(getflip_arg->ch_no, &vin_ch);
			if((vin_ch.flip == false) && (vin_ch.mirror == false) ){
				flip_type =0;
			}else
			if((vin_ch.flip == true) && (vin_ch.mirror == false) ){
				flip_type =1;
			}else if ((vin_ch.flip == false) && (vin_ch.mirror == true) ){
				flip_type =2;
			}else if((vin_ch.flip == true) && (vin_ch.mirror == true) ){
				flip_type =3;
			}
			dbg("DANAVIDEOCMD_GETFLIP %d \n",flip_type);
			if (lib_danavideo_cmd_getflip_response(danavideoconn, trans_id, error_code, code_msg, flip_type)) 
			{  
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_GETFLIP send response succeed\n"); 
			} else {
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_GETFLIP send response failed\n");  
			}            
		}           
		break; 
    	case DANAVIDEOCMD_GETFUNLIST:                               
		{    
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFUNLIST\n");
			DANAVIDEOCMD_GETFUNLIST_ARG *getfunlist_arg = (DANAVIDEOCMD_GETFUNLIST_ARG *)cmd_arg;            
			error_code = -1;  
			code_msg = (char *)"";    
			uint32_t methodes_count = 22;   
			char *methods[] = { (char *)"DANAVIDEOCMD_DEVDEF",  
								(char *)"DANAVIDEOCMD_DEVREBOOT",  
								(char *)"DANAVIDEOCMD_GETSCREEN",    
								(char *)"DANAVIDEOCMD_GETALARM",      
								(char *)"DANAVIDEOCMD_GETBASEINFO",        
								(char *)"DANAVIDEOCMD_GETCOLOR",                 
								(char *)"DANAVIDEOCMD_GETFLIP",                  
								(char *)"DANAVIDEOCMD_GETFUNLIST",                   
								(char *)"DANAVIDEOCMD_GETNETINFO",                 
								(char *)"DANAVIDEOCMD_GETPOWERFREQ",             
								(char *)"DANAVIDEOCMD_GETTIME",                                              
								(char *)"DANAVIDEOCMD_SETALARM",                                     
								(char *)"DANAVIDEOCMD_SETCOLOR",       
								(char *)"DANAVIDEOCMD_SETFLIP",         
								(char *)"DANAVIDEOCMD_SETNETINFO",          
								(char *)"DANAVIDEOCMD_SETPOWERFREQ",          
								(char *)"DANAVIDEOCMD_SETTIME",                  
								(char *)"DANAVIDEOCMD_SETVIDEO",                                  
								(char *)"DANAVIDEOCMD_STARTAUDIO",                     
								(char *)"DANAVIDEOCMD_STARTVIDEO",           
								(char *)"DANAVIDEOCMD_STOPAUDIO",                
								(char *)"DANAVIDEOCMD_STOPVIDEO"             
								 };
			lib_danavideo_cmd_getfunlist_response(danavideoconn, trans_id, error_code, code_msg, methodes_count,(const char**)methods);
		}            
		break; 
    	case DANAVIDEOCMD_GETNETINFO:
		{                
			dbg(" danavideoconn_command_handler DANAVIDEOCMD_GETNETINFO\n");
			DANAVIDEOCMD_GETNETINFO_ARG *getnetinfo_arg = (DANAVIDEOCMD_GETNETINFO_ARG *)cmd_arg;               
			error_code = 0;
			code_msg = (char *)"";
			uint32_t ip_type = 0; // 0 "fixed"; 1 "dhcp" 
			char ipaddr[24];
			char netmask[24];
			char gateway[24];
			uint32_t dns_type = 0;               
			char dns_name1[24]; 
			char dns_name2[24];
			uint32_t http_port = 0;
			memset(ipaddr,0,sizeof(ipaddr));
			memset(netmask,0,sizeof(netmask));
			memset(gateway,0,sizeof(gateway));
			memset(dns_name1,0,sizeof(dns_name1));
			memset(dns_name2,0,sizeof(dns_name2));			
			ST_NSDK_NETWORK_INTERFACE net_info;
			NETSDK_conf_interface_get(getnetinfo_arg->ch_no, &net_info);
			strcpy(ipaddr,net_info.lan.staticIP);
			strcpy(netmask,net_info.lan.staticNetmask);
			strcpy(gateway,net_info.lan.staticGateway);
			ST_NSDK_NETWORK_PORT port;
			NETSDK_conf_port_get(getnetinfo_arg->ch_no,&port);
			http_port = port.value;
			if(strlen(net_info.dns.staticAlternateDns)>4){
				strcpy(dns_name1,net_info.dns.staticAlternateDns);
			}
			if(strlen(net_info.dns.staticPreferredDns)>4){
				strcpy(dns_name2,net_info.dns.staticPreferredDns);
			}
			if (lib_danavideo_cmd_getnetinfo_response(danavideoconn, trans_id, error_code, code_msg, ip_type, ipaddr, netmask, gateway, dns_type, dns_name1, dns_name2, http_port)) {
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_GETNETINFO send response succeed\n"); 
			} else {  
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_GETNETINFO send response failed\n"); 
			}           
		}            
		break;	
    	case DANAVIDEOCMD_GETPOWERFREQ:
		{                
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPOWERFREQ\n");
			DANAVIDEOCMD_GETPOWERFREQ_ARG *getpowerfreq_arg = (DANAVIDEOCMD_GETPOWERFREQ_ARG *)cmd_arg;
              
			error_code = 0;
			code_msg = (char *)"";
			uint32_t freq = DANAVIDEO_POWERFREQ_50HZ;
			ST_NSDK_VIN_CH vin_ch;
			NETSDK_conf_vin_ch_get(getpowerfreq_arg->ch_no, &vin_ch);
			if(50 == vin_ch.powerLineFrequencyMode){
				freq = DANAVIDEO_POWERFREQ_50HZ;
			}else{
				freq = DANAVIDEO_POWERFREQ_60HZ;
			}
			if (lib_danavideo_cmd_getpowerfreq_response(danavideoconn, trans_id, error_code, code_msg, freq)) {
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_GETPOWERFREQ send response succeed\n");
			} else {                    
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_GETPOWERFREQ send response failed\n");
			}
		}
		break; 
    	case DANAVIDEOCMD_GETTIME:							 
		{
			error_code = 0;
			code_msg = (char *)"success";			
			int64_t now_time =0;
			time(&now_time); 
			char text[64]={""};			
			char *time_zone = NULL;
			char *ntp_server_1 = NULL;
			char *ntp_server_2 = NULL; 
			printf("%s\n",ctime(&now_time));
			ST_NSDK_SYSTEM_TIME sys_time;
			NETSDK_conf_system_get_time(&sys_time);
			GMT_STRING(sys_time.greenwichMeanTime, text, sizeof(text));
			time_zone = text;
			if (lib_danavideo_cmd_gettime_response(danavideoconn, trans_id, error_code,code_msg, now_time, time_zone, ntp_server_1, ntp_server_2)) 
			{
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_GETTIME send response succeed\n");
			}else{
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_GETTIME send response failed\n");
			}
		}
			break;
   		case DANAVIDEOCMD_GETWIFIAP: 															//没实现
		{                
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFIAP\n");
			DANAVIDEOCMD_GETWIFIAP_ARG *getwifiap_arg = (DANAVIDEOCMD_GETWIFIAP_ARG *)cmd_arg;               
			error_code = -1;
			code_msg = (char *)"";    
			uint32_t wifi_device = 0;  
			uint32_t wifi_list_count = 0;     
//			libdanavideo_wifiinfo_t wifi_list[] = {{"danasz", 1, 5}, {"danasz5G", 1, 6}};
			if (lib_danavideo_cmd_getwifiap_response(danavideoconn, trans_id, error_code, code_msg, wifi_device, wifi_list_count, NULL)) {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFIAP send response succeeded\n"); 
			} else {                   
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFIAP send response failed\n"); 
			}        
		}
			break; 	
    	case DANAVIDEOCMD_GETWIFI:																//没实现
		{  
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFI\n");
			DANAVIDEOCMD_GETWIFI_ARG *getwifi_arg = (DANAVIDEOCMD_GETWIFI_ARG *)cmd_arg; 
			dbg("getwifi_arg\n");     
			dbg("ch_no: %"PRIu32"\n", getwifi_arg->ch_no);     
			dbg("\n");               
			error_code = -1;      
			code_msg = (char *)"";       
			char *essid = (char *)"danasz";      
//			char *auth_key = (char *)"wps";       
			uint32_t enc_type = 0;     
			if (lib_danavideo_cmd_getwifi_response(danavideoconn, trans_id, error_code, code_msg, NULL, NULL, enc_type)) {   
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFI send response succeeded\n");  
			} else {             
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFI send response failed\n");     
			}    
		}  
			break;
    	case DANAVIDEOCMD_PTZCTRL:																//没有实现
    	{  
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL\n"); 
			DANAVIDEOCMD_PTZCTRL_ARG *ptzctrl_arg = (DANAVIDEOCMD_PTZCTRL_ARG *)cmd_arg;		
			error_code = -1; 
			code_msg = (char *)"";    
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) { 
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL send response succeed\n");               
			} else {                    
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL send response failed\n"); 
			}            
		}            
		break; 
    	case DANAVIDEOCMD_SDCFORMAT:															//没有实现
		{
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SDCFORMAT\n");   
			DANAVIDEOCMD_SDCFORMAT_ARG *sdcformat_arg = (DANAVIDEOCMD_SDCFORMAT_ARG *)cmd_arg;   
			dbg("sdcformat_arg\n");            
			dbg("ch_no: %"PRIu32"\n", sdcformat_arg->ch_no);  
			dbg("\n");              
			error_code = -1;              
			code_msg = (char *)"";          
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {   
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SDCFORMAT send response succeeded\n");               
			} else {                  
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SDCFORMAT send response failed\n");           
			}       
		}
			break;
    	case DANAVIDEOCMD_SETALARM:
		{    
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM\n");     
			DANAVIDEOCMD_SETALARM_ARG *setalarm_arg = (DANAVIDEOCMD_SETALARM_ARG *)cmd_arg; 
			dbg("setalarm_arg\n");              
			dbg("ch_no: %u\n", setalarm_arg->ch_no);    
			dbg("motion_detection: %u\n", setalarm_arg->motion_detection);       
			dbg("opensound_detection: %u\n", setalarm_arg->opensound_detection);         
			dbg("openi2o_detection: %u\n", setalarm_arg->openi2o_detection);           
			dbg("smoke_detection: %u\n", setalarm_arg->smoke_detection);          
			dbg("shadow_detection: %u\n", setalarm_arg->shadow_detection);      
			dbg("other_detection: %u\n", setalarm_arg->other_detection);          
			dbg("\n");  

			ST_NSDK_MD_CH md_ch;			
			NETSDK_conf_md_ch_get(setalarm_arg->ch_no, &md_ch);
			alarm_status = setalarm_arg->motion_detection;
			if(0 == setalarm_arg->motion_detection){
				md_ch.enabled = false;				
			}else if( 0 < setalarm_arg->motion_detection)	{
				md_ch.enabled = true;	
				if(setalarm_arg->motion_detection < 4) 
					setalarm_arg->motion_detection = 33 * alarm_status ;
				else
					setalarm_arg->motion_detection = 100 ;
				
				if(kNSDK_MD_TYPE_GRID == md_ch.detectionType){
					md_ch.detectionGrid.sensitivityLevel = setalarm_arg->motion_detection;
				}else if(kNSDK_MD_TYPE_REGION == md_ch.detectionType){
					md_ch.detectionRegion.ch->sensitivityLevel = setalarm_arg->motion_detection;
				}
			}
			NETSDK_conf_md_ch_set(setalarm_arg->ch_no, &md_ch);
			
			error_code = 0;             
			code_msg = (char *)"";         
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) { 
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM send response succeed\n");     
			} else {             
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM send response failed\n");    
			}          
		}        
		break;	
    	case DANAVIDEOCMD_SETCHAN:  																//没有实现
		{    
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN\n");  
			DANAVIDEOCMD_SETCHAN_ARG *setchan_arg = (DANAVIDEOCMD_SETCHAN_ARG *)cmd_arg;  
			dbg("setchan_arg\n");            
			dbg("ch_no: %u\n", setchan_arg->ch_no);        
			dbg("chans_count: %lu\n", setchan_arg->chans_count);  
			int i=0;
			for (i=0; i<setchan_arg->chans_count; i++) {           
				dbg("chans[%u]: %u\n", i, setchan_arg->chans[i]);     
			}              
			dbg("\n");   
			error_code = -1;
			code_msg = (char *)"";     
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {      
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN send response succeed\n");           
			} else {  
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN send response failed\n");    
			} 
		}      
		break;
    	case DANAVIDEOCMD_SETCOLOR:							                                //手机端没有实现，无法验证										
		{                
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR\n"); 
			DANAVIDEOCMD_SETCOLOR_ARG *setcolor_arg = (DANAVIDEOCMD_SETCOLOR_ARG *)cmd_arg;
			dbg("setcolor_arg\n");                
			dbg("ch_no: %u\n", setcolor_arg->ch_no);
			dbg("video_rate: %u\n", setcolor_arg->video_rate);         //不设置，影响NVR
			dbg("brightness: %u\n", setcolor_arg->brightness); 
			dbg("contrast: %u\n", setcolor_arg->contrast); 
			dbg("saturation: %u\n", setcolor_arg->saturation); 
			dbg("hue: %u\n", setcolor_arg->hue);                
			dbg("\n");               
			error_code = 0;
			code_msg = (char *)"";
			ST_NSDK_VIN_CH vin_ch;
			NETSDK_conf_vin_ch_get(setcolor_arg->ch_no, &vin_ch);	
			vin_ch.brightnessLevel = setcolor_arg->brightness;
			vin_ch.contrastLevel = setcolor_arg->contrast;
			vin_ch.saturationLevel = setcolor_arg->saturation;
			NETSDK_conf_vin_ch_set(setcolor_arg->ch_no, &vin_ch);	
//			netsdk_vin_ch_set(setcolor_arg->ch_no, &vin_ch);
			NETSDK_conf_video_save2();
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) 
			{
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR send response succeed\n");
			} else { 
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR send response failed\n");
			}
		}            
		break;
    	case DANAVIDEOCMD_SETFLIP:
		{ 
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETFLIP\n");
			DANAVIDEOCMD_SETFLIP_ARG *setflip_arg = (DANAVIDEOCMD_SETFLIP_ARG *)cmd_arg;

			error_code = 0; 
			code_msg = (char *)"";
			ST_NSDK_VIN_CH vin_ch;
			NETSDK_conf_vin_ch_get(setflip_arg->ch_no, &vin_ch);
			if(0 == setflip_arg->flip_type){
				vin_ch.flip = false;
				vin_ch.mirror = false;
			}else if(1 == setflip_arg->flip_type){
				vin_ch.flip = true;
				vin_ch.mirror = false;
			}else if(2 == setflip_arg->flip_type){
				vin_ch.flip = false;
				vin_ch.mirror = true;
			}else if(3 == setflip_arg->flip_type){
				vin_ch.flip = true;
				vin_ch.mirror = true;
			}
			NETSDK_conf_vin_ch_set(setflip_arg->ch_no, &vin_ch);
			netsdk_vin_ch_set(setflip_arg->ch_no, &vin_ch);
			NETSDK_conf_video_save2();
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg))
			{ 
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_SETFLIP send response succeed\n");
			} else {
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_SETFLIP send response failed\n");
			}            
		}            
		break; 
    	case DANAVIDEOCMD_SETNETINFO:						//
		{ 
			dbg(" danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO\n");
			DANAVIDEOCMD_SETNETINFO_ARG *setnetinfo_arg = (DANAVIDEOCMD_SETNETINFO_ARG *)cmd_arg;

			ST_NSDK_NETWORK_INTERFACE net_info;
			ST_NSDK_NETWORK_PORT port;
			error_code = 0;
			code_msg = (char *)"";  
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO send response succeed\n");
			} else {     
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO send response failed\n");  
			} 
			
			NETSDK_conf_interface_get(setnetinfo_arg->ch_no, &net_info);
			NETSDK_conf_port_get(setnetinfo_arg->ch_no,&port);
			strcpy(net_info.lan.staticIP , setnetinfo_arg->ipaddr);
			strcpy(net_info.lan.staticNetmask,setnetinfo_arg->netmask);
			strcpy(net_info.lan.staticGateway,setnetinfo_arg->gateway);
			strcpy(net_info.dns.staticAlternateDns,setnetinfo_arg->dns_name1);
			strcpy(net_info.dns.staticPreferredDns,setnetinfo_arg->dns_name2);
			port.value = setnetinfo_arg->http_port;
			NETSDK_conf_interface_set(setnetinfo_arg->ch_no,&net_info,eNSDK_CONF_SAVE_RESTART);
			NETSDK_conf_port_set(setnetinfo_arg->ch_no,&port,eNSDK_CONF_SAVE_REBOOT);			

		}           
		break; 		
    	case DANAVIDEOCMD_SETPOWERFREQ:
		{ 
			dbg(" danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ\n");
			DANAVIDEOCMD_SETPOWERFREQ_ARG *setpowerfreq_arg = (DANAVIDEOCMD_SETPOWERFREQ_ARG *)cmd_arg;  
			ST_NSDK_VIN_CH vin_ch;
			NETSDK_conf_vin_ch_get(setpowerfreq_arg->ch_no, &vin_ch);
			if (DANAVIDEO_POWERFREQ_50HZ == setpowerfreq_arg->freq) { 
				vin_ch.powerLineFrequencyMode = 50;                
			} else if (DANAVIDEO_POWERFREQ_60HZ == setpowerfreq_arg->freq) {
				vin_ch.powerLineFrequencyMode = 60;             
			} else { 
				vin_ch.powerLineFrequencyMode = 50;            
			}               
			NETSDK_conf_vin_ch_set(setpowerfreq_arg->ch_no, &vin_ch);
			netsdk_vin_ch_set(setpowerfreq_arg->ch_no, &vin_ch);
			NETSDK_conf_video_save2();
			error_code = 0; 
			code_msg = (char *)"";
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ send response succeed\n");
			} else { 
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ send response failed\n");    
			}
		}
		break; 	
    	case DANAVIDEOCMD_SETTIME:							
		{ 
			dbg(" danavideoconn_command_handler DANAVIDEOCMD_SETTIME \n");                 
			DANAVIDEOCMD_SETTIME_ARG *settime_arg = (DANAVIDEOCMD_SETTIME_ARG *)cmd_arg; 
			dbg("settime_arg\n");             
			dbg("ch_no: %"PRIu32"\n", settime_arg->ch_no); 
			dbg("now_time: %"PRId64"\n", settime_arg->now_time); 
			dbg("time_zone: %s\n", settime_arg->time_zone);
			error_code = 0;
			code_msg = (char *)"";
			Dana_Settime(settime_arg->now_time, settime_arg->time_zone);
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id,error_code, code_msg)) 
			{
 				dbg(" danavideoconn_command_handler DANAVIDEOCMD_SETTIME send response succeed\n");
			} else {
 				dbg(" danavideoconn_command_handler DANAVIDEOCMD_SETTIME send response failed\n");
			}
		}
			break;			
    	case DANAVIDEOCMD_SETVIDEO:																// 设置帧数，不处理
 		{        
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO\n"); 
			DANAVIDEOCMD_SETVIDEO_ARG *setvideo_arg = (DANAVIDEOCMD_SETVIDEO_ARG *)cmd_arg;  
			dbg("setvideo_arg\n");             
			dbg("ch_no: %u\n", setvideo_arg->ch_no);      
			dbg("video_quality: %u\n", setvideo_arg->video_quality);     
			dbg("\n");               
			error_code = 0;       
			code_msg = (char *)"";      
			uint32_t set_video_fps = 30;    
			dana_SetVideoResolution(setvideo_arg->ch_no,setvideo_arg->video_quality, mydata);
			if (lib_danavideo_cmd_setvideo_response(danavideoconn, trans_id, error_code, code_msg, set_video_fps)) { 
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO send response succeed\n");               
			} else {                 
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO send response failed\n");    
			}           
		}           
		break; 
			
    	case DANAVIDEOCMD_SETWIFIAP:
		{    
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFIAP\n");      
			DANAVIDEOCMD_SETWIFIAP_ARG *setwifiap_arg = (DANAVIDEOCMD_SETWIFIAP_ARG *)cmd_arg;  
			dbg("setwifiap_arg\n");            
			dbg("ch_no: %"PRIu32"\n", setwifiap_arg->ch_no);          
			dbg("ip_type: %"PRIu32"\n", setwifiap_arg->ip_type);          
			dbg("ipaddr: %s\n", setwifiap_arg->ipaddr);           
			dbg("netmask: %s\n", setwifiap_arg->netmask);         
			dbg("gateway: %s\n", setwifiap_arg->gateway);           
			dbg("dns_name1: %s\n", setwifiap_arg->dns_name1);            
			dbg("dns_name2: %s\n", setwifiap_arg->dns_name2);            
			dbg("essid: %s\n", setwifiap_arg->essid);           
			dbg("auth_key: %s\n", setwifiap_arg->auth_key);     
			dbg("enc_type: %"PRIu32"\n", setwifiap_arg->enc_type);      
			dbg("\n");             
			error_code = -1;        
			code_msg = (char *)"";    
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFIAP send response succeeded\n");      
			} else {                  
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFIAP send response failed\n");          
			}    
		}       
		break; 
    	
    	case DANAVIDEOCMD_SETWIFI:
		{              
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFI\n"); 
			DANAVIDEOCMD_SETWIFI_ARG *setwifi_arg = (DANAVIDEOCMD_SETWIFI_ARG *)cmd_arg;   
			dbg("setwifi_arg\n");           
			dbg("ch_no: %"PRIu32"\n", setwifi_arg->ch_no);    
			dbg("essid: %s\n", setwifi_arg->essid);    
			dbg("auth_key: %s\n", setwifi_arg->auth_key); 
			dbg("enc_type: %"PRIu32"\n", setwifi_arg->enc_type);    
			dbg("\n");              
			error_code = -1;        
			code_msg = (char *)"";         
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {  
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFI send response succeeded\n");   
			} else {                   
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFI send response failed\n");  
			}         
		}         
		break; 	
    	case DANAVIDEOCMD_STARTAUDIO:			
		{               
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTAUDIO\n");  
			DANAVIDEOCMD_STARTAUDIO_ARG *startaudio_arg = (DANAVIDEOCMD_STARTAUDIO_ARG *)cmd_arg; 
			dbg("startaudio_arg\n");                dbg("ch_no: %u\n", startaudio_arg->ch_no);   
			dbg("\n");                
			error_code = 0;                
			code_msg = (char *)"";  	
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {  
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTAUDIO send response succeed\n");   
			} else {                 
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTAUDIO send response failed\n"); 
			}          
			mydata->run_audio = true;  
			ST_NSDK_AENC_CH aenc_ch;
			NETSDK_conf_aenc_ch_get(startaudio_arg->ch_no, &aenc_ch);
			aenc_ch.enabled = true;
			NETSDK_conf_aenc_ch_set(startaudio_arg->ch_no, &aenc_ch);

		}            
		break;
    	case DANAVIDEOCMD_STARTTALKBACK:
			{
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK\n");
				DANAVIDEOCMD_STARTTALKBACK_ARG *starttalkback_arg = (DANAVIDEOCMD_STARTTALKBACK_ARG *)cmd_arg;
				dbg("starttalkback_arg\n");                
				dbg("ch_no: %u\n", starttalkback_arg->ch_no);
				dbg("\n");                
				error_code = -1;
				code_msg = (char *)""; 
				uint32_t audio_codec = G711A; 
				if (lib_danavideo_cmd_starttalkback_response(danavideoconn, trans_id, error_code, code_msg, audio_codec)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK send response succeed\n");                
				} else {                   
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK send response failed\n");
				}
			}
		break;
    	case DANAVIDEOCMD_STARTVIDEO:										  //打开视频
			{									
			dbg(" danavideoconn_command_handler DANAVIDEOCMDSTARTVIDEO\n");
			 DANAVIDEOCMD_STARTVIDEO_ARG *startvideo_arg = (DANAVIDEOCMD_STARTVIDEO_ARG *)cmd_arg;
			 error_code = 0;
			 code_msg = (char *)"start";
			 uint32_t start_video_fps = 25;
			 if(lib_danavideo_cmd_startvideo_response(danavideoconn, trans_id, error_code, code_msg, start_video_fps)){
			 	dbg("IPC danavideoconn_command_handler DANAVIDEOCMDSTARTVIDEO send response succeed\n");
			 }else{
			 	dbg("IPC danavideoconn_command_handler DANAVIDEOCMDSTARTVIDEO send response failed\n");
			 }
			mydata->run_media = true;                
			mydata->exit_media = false; 
			mydata->change_media = false;
			mydata->chan_no = startvideo_arg->ch_no;
			mydata->video_quality = startvideo_arg->video_quality;
			if (0 != pthread_create(&(mydata->thread_media), NULL, &th_media, mydata)) {                    
				dbg("IPC danavideoconn_command_handler pthread_create th_media failed\n");                    // 发送无法开启视频线程到对端                
			} else {                   
				pthread_detach(mydata->thread_media);                    
				dbg("IPC danavideoconn_command_handler th_media is started, enjoy!\n");                
			}			
    	}
			break;
    	case DANAVIDEOCMD_STOPAUDIO: 
		{                
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPAUDIO\n");
			DANAVIDEOCMD_STOPAUDIO_ARG *stopaudio_arg = (DANAVIDEOCMD_STOPAUDIO_ARG *)cmd_arg; 
			dbg("stopaudio_arg\n");                
			dbg("ch_no: %u\n", stopaudio_arg->ch_no);  
			dbg("\n");    
			error_code = 0;      
			code_msg = (char *)""; 
			mydata->run_audio = false;
			ST_NSDK_AENC_CH aenc_ch;										//设置底层停止音频输入
			NETSDK_conf_aenc_ch_get(stopaudio_arg->ch_no, &aenc_ch);
			aenc_ch.enabled = false;
			NETSDK_conf_aenc_ch_set(stopaudio_arg->ch_no, &aenc_ch);
   
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) { 
				dbg(" danavideoconn_command_handler DANAVIDEOCMD_STOPAUDIO send response succeed\n"); 
			} else {   
				dbg("danavideoconn_command_handler DANAVIDEOCMD_STOPAUDIO send response failed\n");
			}
			pthread_join(mydata->thread_media, NULL);
			mydata->thread_media = NULL;
		}           
		break;	
    	case DANAVIDEOCMD_STOPTALKBACK:													//不实现
		{           
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPTALKBACK\n");   
			DANAVIDEOCMD_STOPTALKBACK_ARG *stoptalkback_arg = (DANAVIDEOCMD_STOPTALKBACK_ARG *)cmd_arg;   
			dbg("stoptalkback_arg\n");           
			dbg("ch_no: %"PRIu32"\n", stoptalkback_arg->ch_no);       
			dbg("\n");             
			// 关闭音频读取线程  
			dbg("TEST danavideoconn_command_handler stop th_talkback\n");     
			error_code = -1;  
			code_msg = (char *)"";      
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {   
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPTALKBACK send response succeeded\n");   
			} else {                  
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPTALKBACK send response failed\n");    
			}     
		}         
		break;	
    	case DANAVIDEOCMD_STOPVIDEO:													//关闭视频
		{
			dbg("danavideoconn_command_handler DANAVIDEOCMDSTOPVIDEO\n");
			DANAVIDEOCMD_STOPVIDEO_ARG *stopvideo_arg = (DANAVIDEOCMD_STOPVIDEO_ARG *)cmd_arg;
			mydata->run_media = false;                
			error_code = 0;                
			code_msg = (char *)"";
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMDSTOPVIDEO send response succeed\n");
			} else {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMDSTOPVIDEO send response failed\n");
			} 
		}
			break;
    	case DANAVIDEOCMD_RECLIST:
		{     
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST\n");  
			DANAVIDEOCMD_RECLIST_ARG *reclist_arg = (DANAVIDEOCMD_RECLIST_ARG *)cmd_arg;   
			dbg("reclist_arg\n");         
			dbg("ch_no: %"PRIu32"\n", reclist_arg->ch_no);      
			if (DANAVIDEO_REC_TYPE_ALARM == reclist_arg->get_type) {   
				dbg("get_type: DANAVIDEO_REC_TYPE_ALARM\n");          
			} else if (DANAVIDEO_REC_TYPE_NORMAL == reclist_arg->get_type) {   
				dbg("get_type: DANAVIDEO_REC_TYPE_NORMAL\n");              
			} else {                   
				dbg("Unknown get_type: %"PRIu32"\n", reclist_arg->get_type);  
			}              
			dbg("get_num: %"PRIu32"\n", reclist_arg->get_num);     
			dbg("last_time: %"PRId64"\n", reclist_arg->last_time);   
			dbg("\n");             
			error_code = -1;         
			code_msg = "";          
			uint32_t rec_lists_count = 0;  
			libdanavideo_reclist_recordinfo_t rec_lists[] = {{123, 123, DANAVIDEO_REC_TYPE_NORMAL}, {456, 456, DANAVIDEO_REC_TYPE_ALARM}}; 
			if (lib_danavideo_cmd_reclist_response(danavideoconn, trans_id, error_code, code_msg, 0, NULL)) {      
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST send response succeeded\n");      
			} else {           
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST send response failed\n");       
			}           
		}      
		break;
    	case DANAVIDEOCMD_RECPLAY:
		{         
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLAY\n");   
			DANAVIDEOCMD_RECPLAY_ARG *recplay_arg = (DANAVIDEOCMD_RECPLAY_ARG *)cmd_arg; 
			dbg("recplay_arg\n");    
			dbg("ch_no: %"PRIu32"\n", recplay_arg->ch_no);
			dbg("time_stamp: %"PRId64"\n", recplay_arg->time_stamp);   
			dbg("\n");            
			error_code = 0; 
			code_msg = "";         
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) { 
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLAY send response succeeded\n"); 
			} else {    
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLAY send response failed\n");   
			}        
		}        
		break;
    	case DANAVIDEOCMD_RECSTOP:
		{        
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP\n");   
			DANAVIDEOCMD_RECSTOP_ARG *recstop_arg = (DANAVIDEOCMD_RECSTOP_ARG *)cmd_arg;
			dbg("recstop_arg\n");             
			dbg("ch_no: %"PRIu32"\n", recstop_arg->ch_no); 
			dbg("\n");               
			error_code = 0; 
			code_msg = "";        
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {  
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP send response succeeded\n");   
			} else {               
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP send response failed\n"); 
			}         
		}         
		break;
    	case DANAVIDEOCMD_RECACTION:
		{       
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION\n");  
			DANAVIDEOCMD_RECACTION_ARG *recaction_arg = (DANAVIDEOCMD_RECACTION_ARG *)cmd_arg;   
			dbg("recaction_arg\n");            
			dbg("ch_no: %"PRIu32"\n", recaction_arg->ch_no);  
			if (DANAVIDEO_REC_ACTION_PAUSE == recaction_arg->action) {    
				dbg("action: DANAVIDEO_REC_ACTION_PAUSE\n");            
			} else if (DANAVIDEO_REC_ACTION_PLAY == recaction_arg->action) {  
			dbg("action: DANAVIDEO_REC_ACTION_PLAY\n");     
			} else {          
			dbg("Unknown action: %"PRIu32"\n", recaction_arg->action);      
			}         
			dbg("\n");   
			error_code = 0;    
			code_msg = "";       
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {      
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION send response succeeded\n");    
			} else {       
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION send response failed\n");    
			}        
		}        
		break;	
    	case DANAVIDEOCMD_RECSETRATE:
		{            
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE\n");    
			DANAVIDEOCMD_RECSETRATE_ARG *recsetrate_arg = (DANAVIDEOCMD_RECSETRATE_ARG *)cmd_arg; 
			dbg("recsetrate_arg\n");     
			dbg("ch_no: %"PRIu32"\n", recsetrate_arg->ch_no); 
			if (DANAVIDEO_REC_RATE_HALF == recsetrate_arg->rec_rate) {   
				dbg("rec_rate: DANAVIDEO_REC_RATE_HALF\n");            
			} else if (DANAVIDEO_REC_RATE_NORMAL == recsetrate_arg->rec_rate) {   
				dbg("rec_rate: DANAVIDEO_REC_RATE_NORMAL\n");        
			} else if (DANAVIDEO_REC_RATE_DOUBLE == recsetrate_arg->rec_rate) {  
				dbg("rec_rate: DANAVIDEO_REC_RATE_DOUBLE\n");    
			} else if (DANAVIDEO_REC_RATE_FOUR == recsetrate_arg->rec_rate) {  
				dbg("rec_rate: DANAVIDEO_REC_RATE_FOUR\n");           
			} else {           
				dbg("Unknown rec_rate: %"PRIu32"\n", recsetrate_arg->rec_rate);     
			}            
			dbg("\n"); 
			error_code = 0; 
			code_msg = "";   
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {    
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE send response succeeded\n");  
			} else {            
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE send response failed\n");      
			}        
		}        
		break;
    	case DANAVIDEOCMD_RECPLANGET:
		{          
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET\n");    
			DANAVIDEOCMD_RECPLANGET_ARG *recplanget_arg = (DANAVIDEOCMD_RECPLANGET_ARG *)cmd_arg;   
			dbg("recplanget_arg\n");             
			dbg("ch_no: %"PRIu32"\n", recplanget_arg->ch_no);  
			dbg("\n");             
			error_code = 0;              
			code_msg = "";          
			uint32_t rec_plans_count = 2;     
			libdanavideo_recplanget_recplan_t rec_plans[] = {{0, 2, {DANAVIDEO_REC_WEEK_MON, DANAVIDEO_REC_WEEK_SAT}, "12:23:34", "15:56:01", DANAVIDEO_REC_PLAN_OPEN}, {1, 3, {DANAVIDEO_REC_WEEK_MON, DANAVIDEO_REC_WEEK_SAT, DANAVIDEO_REC_WEEK_SUN}, "22:23:24", "23:24:25", DANAVIDEO_REC_PLAN_CLOSE}}; 
			if (lib_danavideo_cmd_recplanget_response(danavideoconn, trans_id, error_code, code_msg, rec_plans_count, rec_plans)) {  
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET send response succeeded\n");   
			} else {          
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET send response failed\n");      
			}     
		}        
		break;
    	case DANAVIDEOCMD_RECPLANSET:
		{             
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET\n"); 
			DANAVIDEOCMD_RECPLANSET_ARG *recplanset_arg = (DANAVIDEOCMD_RECPLANSET_ARG *)cmd_arg;   
			dbg("recplanset_arg\n");            
			dbg("ch_no: %"PRIu32"\n", recplanset_arg->ch_no);
			dbg("record_no: %"PRIu32"\n", recplanset_arg->record_no);  
			size_t i;        
			for (i=0; i<recplanset_arg->week_count; i++) {    
				if (DANAVIDEO_REC_WEEK_MON == recplanset_arg->week[i]) { 
					dbg("week: DANAVIDEO_REC_WEEK_MON\n"); 
				} else if (DANAVIDEO_REC_WEEK_TUE == recplanset_arg->week[i]) { 
					dbg("week: DANAVIDEO_REC_WEEK_TUE\n");        
				} else if (DANAVIDEO_REC_WEEK_WED == recplanset_arg->week[i]) {  
					dbg("week: DANAVIDEO_REC_WEEK_WED\n");               
				} else if (DANAVIDEO_REC_WEEK_THU == recplanset_arg->week[i]) {     
					dbg("week: DANAVIDEO_REC_WEEK_THU\n");   
				} else if (DANAVIDEO_REC_WEEK_FRI == recplanset_arg->week[i]) {     
					dbg("week: DANAVIDEO_REC_WEEK_FRI\n");      
				} else if (DANAVIDEO_REC_WEEK_SAT == recplanset_arg->week[i]) {
					dbg("week: DANAVIDEO_REC_WEEK_SAT\n");    
				} else if (DANAVIDEO_REC_WEEK_SUN == recplanset_arg->week[i]) {    
					dbg("week: DANAVIDEO_REC_WEEK_SUN\n");        
				} else {  
					dbg("Unknown week: %"PRIu32"\n", recplanset_arg->week[i]);  
				}         
			}           
			dbg("start_time: %s\n", recplanset_arg->start_time);   
			dbg("end_time: %s\n", recplanset_arg->end_time); 
			dbg("status: %"PRIu32"\n", recplanset_arg->status);  
			dbg("\n");                
			error_code = 0;      
			code_msg = "";   
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) { 
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET send response succeeded\n");   
			} else {          
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET send response failed\n");    
			}        
		}    
		break;	
    	case DANAVIDEOCMD_EXTENDMETHOD:
		{      
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_EXTENDMETHOD\n");  
			DANAVIDEOCMD_EXTENDMETHOD_ARG *extendmethod_arg = (DANAVIDEOCMD_EXTENDMETHOD_ARG *)cmd_arg;  
			dbg("extendmethod_arg\n");             
			dbg("extend_no: %"PRIu32"\n", extendmethod_arg->ch_no);
			dbg("extend_data_size: %zd\n", extendmethod_arg->extend_data.size);   
			// extend_data_bytes access via extendmethod_arg->extend_data.bytes   
			dbg("\n");   
			error_code = 0;     
			code_msg = "";   
			// 发送数据 TODO   
		}     
			break;
		case DANAVIDEOCMD_RESOLVECMDFAILED:          
		{ 
			dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED\n");   
			// 根据不同的method,调用lib_danavideo_cmd_response       
			error_code = 20145;      
			code_msg = (char *)"danavideocmd_resolvecmdfailed";  
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)cmd_arg, trans_id, error_code, code_msg)) { 
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED send response succeeded\n");      
			} else {        
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED send response failed\n");    
			}            
		}      
		break;
		
		default:
			{  
				dbg("TEST danavideoconn_command_handler UnKnown CMD: %"PRId32"\n", cmd);   
			}
			break;
	 }
	 
}


void th_media(void *arg){
	printf("ipcam.c dana th_media start\n");
#define AUDIO_FRAME_SIZE	320
#define BUF_SIZE	(1024*1024)
	MYDATA *mydata = (MYDATA *)arg;    
	int media_id;
	int is_first_i_frame = 1, i_frame_cnt = 0;
	unsigned int base_ts=0xffffffff,ts=0;
	int out_success=false;
	int send_ret = -1;
	lpMEDIABUF_USER media_user=NULL;
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME, "th_media");
	if(NULL ==mydata ){
		pthread_exit(NULL);
	}
	
	out_success = false;
	while((true == mydata->run_media) && (false == mydata->exit_media) &&(NULL != mydata->danavideoconn)){
		if(media_user == NULL){
			char * stream=  getenv("DANA_STREAM");
			//(stream ==  NULL)? "720p.264" : stream;
			printf("dana start video quality:%d\n", mydata->video_quality);
			if(mydata->video_quality > 50){
				stream = "720p.264";
			}else{
				stream = "360p.264";
			}
			media_id = MEDIABUF_lookup_byname(stream);
			if(media_id >= 0){
				media_user = MEDIABUF_attach(media_id);
				if(media_user == NULL){
					printf("media attach falied!\n");
				}
				MEDIABUF_sync(media_user);
				is_first_i_frame = 1;
				i_frame_cnt = 0;
			}else{
				printf("lookup name failed falied!\n");
			}
		}else if(true == mydata->change_media){
			//change stream with ungelivable network
			mydata->change_media = false;
			MEDIABUF_detach(media_user);
			media_user = NULL;
			char * stream=  getenv("DANA_STREAM");
			(stream ==  NULL)? "720p.264" : stream;
			media_id = MEDIABUF_lookup_byname(stream);
			if(media_id >= 0){
				media_user = MEDIABUF_attach(media_id);
				if(media_user == NULL){
					printf("media attach falied!\n");
				}
				MEDIABUF_sync(media_user);
				is_first_i_frame = 1;
				i_frame_cnt = 0;
			}else{
				printf("lookup name failed falied!\n");
			}
		}
		if(0 == MEDIABUF_out_lock(media_user)){
			const lpSDK_ENC_BUF_ATTR attr = NULL;
			size_t out_size = 0;
						
			if(0 == MEDIABUF_out(media_user, (void **)&attr, NULL, &out_size)){
				const void* const raw_ptr = (void*)(attr + 1);
				ssize_t const raw_size = attr->data_sz;
					if(is_first_i_frame && attr->h264.keyframe){
					if(++i_frame_cnt >= 2){
						is_first_i_frame = 0;
						base_ts = (uint32_t)(attr->timestamp_us/1000);
					}
				}
				if(i_frame_cnt < 2){
					MEDIABUF_out_unlock(media_user);
							continue;
				}
				if(((uint32_t)(attr->timestamp_us/1000) - base_ts -ts) > 50){
					dbg("ts dev:%d ts:%u->%u %u\n",(uint32_t)(attr->timestamp_us/1000) - base_ts -ts,ts,
								(uint32_t)(attr->timestamp_us/1000) - base_ts,base_ts);
				}
				ts = (uint32_t)(attr->timestamp_us/1000) - base_ts;
//				printf("ts_media timestamp %d\n",ts);
				if(kSDK_ENC_BUF_DATA_H264 == attr->type){
						send_ret= lib_danavideoconn_send(mydata->danavideoconn, video_stream, H264, mydata->chan_no, attr->h264.keyframe?1:0, ts,(const char*)(raw_ptr), raw_size,30*1000);
					if(true == send_ret){
					    dbg("th_media send H264  [%u] succeeded\n", raw_size); 
					} else {      
						dbg("th_media send H264 [%u] failed\n", raw_size);        
					}       
				}
				else {
					if(true == mydata->run_audio ){
						if(kSDK_ENC_BUF_DATA_PCM == attr->type){
							send_ret= lib_danavideoconn_send(mydata->danavideoconn, audio_stream, PCM, mydata->chan_no, 0, ts, (const char*)(raw_ptr), raw_size,30*1000);
							if(true == send_ret){
							    dbg("th_media send PCM [%u] succeeded\n", raw_size); 
							} else {      
								dbg("th_media send PCM [%u] failed\n", raw_size);      
							}  
						}else if(kSDK_ENC_BUF_DATA_G711A== attr->type){
							send_ret= lib_danavideoconn_send(mydata->danavideoconn, audio_stream, G711A, mydata->chan_no, 0, ts,  (const char*)(raw_ptr), raw_size,30*1000);
							if(true == send_ret){
							    dbg("th_media send G711A [%u] succeeded\n", raw_size); 
							} else {      
								dbg("th_media send G711A [%u] failed\n", raw_size);     
							}             					
						} 				
					}

//					dbg("ipcam dana got type=%d impossible\n", attr->type);
				}
				MEDIABUF_out_unlock(media_user);
			}else{
					MEDIABUF_out_unlock(media_user);
					usleep(2000);
			}
//			MEDIABUF_out_unlock(media_user);
		}
	}
	MEDIABUF_detach(media_user);
	dbg("th_media exit\n");    
	mydata->exit_media = true;
	mydata->run_audio = false;
	pthread_exit(NULL);
}

int dana_pic_media(uint32_t ch_no,void *arg){
	printf("ipcam.c dana dana_pic_media start\n");
	int ret =0;
	int resolutionWidth =  0;
	int resolutionHeight = 0;
	uint32_t id = ch_no;
	char filePath[256] = {""};
	MYDATA *mydata = (MYDATA *)arg;   
	if(!resolutionWidth || !resolutionHeight){
		ST_NSDK_VENC_CH venc_ch;
		if(NETSDK_conf_venc_ch_get(id, &venc_ch)){
			if(!venc_ch.freeResolution){
				resolutionWidth = (venc_ch.resolution >> 16) & 0xffff;
				resolutionHeight = (venc_ch.resolution >> 0) & 0xffff;
			}else{
				resolutionWidth = venc_ch.resolutionWidth;
				resolutionHeight = venc_ch.resolutionHeight;
			}
		}
	}
	dbg("dana_pic_media resolutionWidth %d  ,  resolutionHeight  %d\n",resolutionWidth,resolutionHeight);    
	NETSDK_venc_snapshot(id, kNSDK_SNAPSHOT_IMAGE_JPEG, resolutionWidth,resolutionHeight, filePath);
	
	dbg("dana_pic_media picture filePath %s \n",filePath);   

	FILE *raw_ptr = fopen(filePath,"r+");
	if(raw_ptr ==NULL )
	{
		return -1;
	}

	
	bool pmsg = lib_danavideoconn_send(mydata->danavideoconn, pic_stream, JPG, mydata->chan_no, 1, 0, (const char*)(raw_ptr), sizeof(raw_ptr),30*1000);
	if (true == pmsg) {                    
	    dbg("th_media send picture  succeeded\n" ); 
	} else {     
		dbg("th_media send picture  failed\n" );                         
	}
	fclose(raw_ptr);
	NETSDK_venc_free_snapshot(filePath);
	return ret;
}

void Dana_notify_event(){
	if(false ==lib_danavideo_started)
		return 0;
	alarm_status = dana_get_alarm_status(1);
	dbg("Dana_back_handle  liry 1111111111111111111111111111111111 %d\n",alarm_status);
	if (alarm_status){
		dbg("Dana_back_handle  liry 222222222222222222222222222222222222222");\
		uint32_t ch_no = 1;
		uint32_t msg_type = 1;   
		char     *msg_title = " IPC dana motion alarm"; 
		char     *msg_body  = "IPC motion alarm!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";  
		int64_t  cur_time = 0;


		uint32_t att_flag = 0;    uint32_t record_flag = 0;   
		if (lib_danavideo_util_pushmsg(ch_no,msg_type, msg_title, msg_body, cur_time, att_flag, NULL, NULL, record_flag, 0, 0, 0, NULL)) {
		dbg("\x1b[32m danavideo  lib_danavideo_util_pushmsg success\x1b[0m\n");  
		} else {      
		dbg("\x1b[34m danavideo  lib_danavideo_util_pushmsg failed\x1b[0m\n");  
		}
	}
}

static int dana_get_alarm_status(int ch_no){	
	int ret = 0;
	ST_NSDK_MD_CH md_ch;			
	NETSDK_conf_md_ch_get(ch_no, &md_ch);
	if(false == md_ch.enabled){
		ret =0;
	}else{
		if(kNSDK_MD_TYPE_GRID == md_ch.detectionType ){
				ret  = md_ch.detectionGrid.sensitivityLevel/33+(md_ch.detectionGrid.sensitivityLevel%33?1:0);
				if(ret > 3)
					ret = 3;
		}else if(kNSDK_MD_TYPE_REGION == md_ch.detectionType){
			ret = md_ch.detectionRegion.ch->sensitivityLevel/33+(md_ch.detectionGrid.sensitivityLevel%33?1:0);
				if(ret > 3)
					ret = 3;
		}
	}
	return ret;
}

 
void danavideo_autoset(const uint32_t power_freq, const int64_t now_time, const char *time_zone, const char *ntp_server1, const char *ntp_server2)
{ 
//	dbg("AUTOSET\n\tpower_freq: %"PRIu32"\n\tnow_time: %"PRId64"\n\ttime_zone: %s\n\tntp_server1: %s\n\tntp_server2: %s\n", power_freq, now_time, time_zone, ntp_server1, ntp_server2);
	Dana_Settime(now_time,time_zone);	
}

static boolean Dana_Settime(const int64_t now_time, const char *time_zone)
{
	char local_time_zone[64];
	char *tmp_zone = time_zone;
	memset(local_time_zone,0,sizeof(local_time_zone));
	if(strcmp(tmp_zone,"CST (China)") ==0){
		tmp_zone = "GMT+08:00";
	}
	strcpy(local_time_zone,tmp_zone);
	struct timeval tv;
	struct timezone tz;
	ST_NSDK_SYSTEM_TIME sys_time;
	NETSDK_conf_system_get_time(&sys_time);
	sys_time.greenwichMeanTime = sys_time.greenwichMeanTime = GMT_PARSE(local_time_zone,sys_time.greenwichMeanTime);
	NETSDK_conf_system_set_time(&sys_time);
	tv.tv_sec = now_time;
	tv.tv_usec = 0;
	tz.tz_minuteswest = 0;
	tz.tz_dsttime =0;
	settimeofday(&tv,NULL);
	return true;
}

static int dana_SetVideoResolution(int id,uint32_t video_quality, MYDATA *mydata)
{
	ST_NSDK_VENC_CH venc_ch;	
	static int last_video_quality = (55/25);
	int quality_tmp = (video_quality - 1)/25;

	if(quality_tmp + last_video_quality >= 2 && quality_tmp != last_video_quality){
		//must change the stream
		mydata->change_media = true;
	}else{
		NETSDK_conf_venc_ch_get(102, &venc_ch);
		venc_ch.constantBitRate = (512*video_quality)/50;
		if(venc_ch.constantBitRate < 64){
			venc_ch.constantBitRate = 64;
		}
		
		NETSDK_conf_venc_ch_set(102, &venc_ch);
		netsdk_venc_ch_changed(102, &venc_ch);
	}
	
	if(quality_tmp != last_video_quality){
		if(quality_tmp >= 2){
			//HD
			setenv("DANA_STREAM", "720p.264", true);
		}else if(quality_tmp >= 1){
			//SD
			setenv("DANA_STREAM", "360p.264", true);
		}else{
			//fluent
			setenv("DANA_STREAM", "360p.264", true);
		}
		last_video_quality = quality_tmp;
	}
	return 1;

}

char* Dana_get_device_id()
{
	return lib_danavideo_deviceid();
}
