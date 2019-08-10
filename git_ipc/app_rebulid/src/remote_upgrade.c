#include "remote_upgrade.h"
#include "remote_upgrade_debug.h"
#include "http_util.h"
#include "socket_tcp.h"
#include <sys/prctl.h>
#include "global_runtime.h"
#include "base/ja_process.h"

extern int upgrade_prase_detail_url(char *detail_path, char *domain, int *ret_port, char *DevModel, char *SWVersion, char *ODMNum, char *ret_filepath, char *deviceSN);
extern int upgrade_prase_rom_detail(char *detail_file,  char ret_domain[512], int *ret_port, char ret_filepath[512], int *errnum);
extern int upgrade_prase_url(char *url, char *ret_domain, int *ret_port, char *ret_filepath);

#define REMOTE_UPGRADE_XML_DIR "/media/custom/web/oem.xml" 
#define REMOTE_UPGRADE_DETAIL_XML_DIR "/tmp/update.xml" 

enum{
	REMOTE_UPGRADE_STATUS_IDLE = 0,
	REMOTE_UPGRADE_STATUS_PREPARE, 
	REMOTE_UPGRADE_STATUS_CHECK_NETWORK,
	REMOTE_UPGRADE_STATUS_CHECK_UPDATE_DETAIL,
	REMOTE_UPGRADE_STATUS_DOWNLOAD_ROOTFS,
	REMOTE_UPGRADE_STATUS_DOWNLOAD_RESOURCE,
	REMOTE_UPGRADE_STATUS_SUCCESS,
	REMOTE_UPGRADE_STATUS_FAILED,
};

typedef struct remote_upgrade
{
	pthread_t thread_tid;
	//int remote_sock;
	int upgrade_status;
	int upgrade_errno;
	char resource_version[16];
	char detail_domain[512];
	char detail_ipaddr[32];
	int detail_port;
	char detail_filepath[512];
	char file_domain[512];
	char file_ipaddr[32];
	int file_port;
	char rom_filepath[512];
	bool inited;
	bool trigger;
	bool need_reboot;
	int upgrade_rate;
	ST_REMOTE_UPGRADE_DEVINFO info;
}ST_REMOTE_UPGRADE,*LP_REMOTE_UPGRADE;

static ST_REMOTE_UPGRADE upgrade_attr = {0};


static int remote_upgrade_gethostbyname(char *domain, char *ret_ip)
{
	if(domain && ret_ip){
		struct hostent* host_ent = NULL;
		host_ent = gethostbyname2(domain, AF_INET);
		if(!host_ent){
			// no host name
			printf("OTA-domain %s not found!\r\n", domain);
			return -1;
		}
		// reset the server ip
		memcpy(ret_ip, inet_ntoa(*(struct in_addr*)(host_ent->h_addr)), strlen(inet_ntoa(*(struct in_addr*)(host_ent->h_addr))));
		printf("host: %s -- ip: %s\r\n", domain, ret_ip);
	}else{
		strcpy(ret_ip, "42.96.185.60");
	}
	return 0;
}

static int remote_upgrade_detail_request(void)
{
	int ret;
	char text[1024*5];
	LP_HTTP_HEAD_FIELD snd_header = NULL;
	LP_HTTP_HEAD_FIELD rcv_header = NULL;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp_r(&sockTCP);
	//set socket time out
	tcp->set_send_timeout(tcp, 30, 0);
	tcp->set_recv_timeout(tcp, 30, 0);

	snd_header = HTTP_UTIL_new_request_header("http", "1.1", "GET", upgrade_attr.detail_filepath, NULL);
	//snd_header->add_tag_text(snd_header, "Authorization", "Basic admin:", true);
	snd_header->add_tag_text(snd_header, "User-Agent", "RemoteUpgradeClient", true);
	snd_header->add_tag_text(snd_header, "Host", upgrade_attr.detail_ipaddr, true);
	snd_header->add_tag_text(snd_header, "Connection", "Keep-Alive", true);
	snd_header->add_tag_text(snd_header, "Accept", "*/*", true);
	snd_header->to_text(snd_header, text, sizeof(text));	


	RU_TRACE("REQUEST size %d:\n%s", strlen(text), text);

	ret = tcp->connect(tcp, upgrade_attr.detail_ipaddr, upgrade_attr.detail_port);
	if(ret < 0){
		RU_TRACE("connect failed");
		goto ERR_EXIT1;
	}

	ret = tcp->send2(tcp, text, strlen(text), 0);
	if(ret < 0){
		RU_TRACE("send data failed");
		goto ERR_EXIT1;
	}

	ret = tcp->recv(tcp, text, sizeof(text)-1, 0);
	if(ret <= 0){
		RU_TRACE("receive data data failed");
		goto ERR_EXIT1;
	}

	RU_TRACE("remote_upgrade_detail_request:RESPONSE size %d:\n%s", ret, text);

	if(NULL == (rcv_header = HTTP_UTIL_parse_response_header(text, strlen(text)))){
		RU_TRACE("parse response header failed");
		goto ERR_EXIT1;
	}

	if( 200 != rcv_header->status_code){
		RU_TRACE("ERROR response %d (%s)", rcv_header->status_code, rcv_header->reason_phrase);
		goto ERR_EXIT1;
	}

	snd_header->free(snd_header);
	rcv_header->free(rcv_header);
	snd_header = NULL;
	rcv_header = NULL;
	if(tcp && tcp->sock > 0){
		tcp->close(tcp);
		tcp = NULL;
	}

	char *pXml = strstr(text, "\r\n\r\n");
	if(pXml){
		//RU_TRACE("%s", pXml);	
		return  upgrade_prase_rom_detail(pXml, upgrade_attr.file_domain, &upgrade_attr.file_port, upgrade_attr.rom_filepath, &upgrade_attr.upgrade_errno);
	}else{
		goto ERR_EXIT2;
	}

ERR_EXIT1:
	RU_TRACE("%s failed!", __FUNCTION__);
	upgrade_attr.upgrade_errno = REMOTE_UPGRADE_ERROR_NO_SERVER_RESPONSE;
	if(snd_header) snd_header->free(snd_header);
	if(rcv_header) rcv_header->free(rcv_header);
	if(tcp && tcp->sock > 0){
		tcp->close(tcp);
		tcp = NULL;
	}

ERR_EXIT2:
	return -1;	
}

static int remote_upgrade_write_rom_process(void *pUpdataMem, int dataSize)
{
	int ret = 0;

	if(NULL == pUpdataMem || dataSize <= 0)
	{
        return -1;
	}
	// start upgrade
	//FIRMWARE_upgrade_start(0, false);
	if(upgrade_attr.info.function.flash_sync){
		upgrade_attr.info.function.flash_sync(upgrade_attr.info.firmware_path, pUpdataMem, dataSize);
	}
	//FIRMWARE_upgrade_parse_rom_from_file("/tmp/firmware.rom", REMOTE_UPGRADE_FILE_DIR);
	
	//waiting for the flash writing
	do{
		if(upgrade_attr.info.function.get_rate){
			upgrade_attr.upgrade_rate = upgrade_attr.info.function.get_rate();
		}
		//remote_upgrade_update_rate();
		RU_TRACE("upgrade_rate = %d", upgrade_attr.upgrade_rate);
		sleep(1);
	}while(100 > upgrade_attr.upgrade_rate);
	
	return ret;
}

static int remote_upgrade_get_rom()
{
	int ret, tmp_rate;
	char text[1024*4];
	char temp[256];
	LP_HTTP_HEAD_FIELD snd_header = NULL;
	LP_HTTP_HEAD_FIELD rcv_header = NULL;
	char *pRev_length = NULL;
	int receive_size = 0;
	FILE *rom_fd = NULL;
	int receive_file_size = 0, rom_file_size = 0;
	//int sock = -1;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp_r(&sockTCP);
	//set socket time out
	tcp->set_send_timeout(tcp, 60, 0);
	tcp->set_recv_timeout(tcp, 60, 0);
	
	
	snd_header = HTTP_UTIL_new_request_header("http", "1.1", "GET", upgrade_attr.rom_filepath, NULL);
	//snd_header->add_tag_text(snd_header, "Authorization", "Basic admin:", true);
	snd_header->add_tag_text(snd_header, "User-Agent", "RemoteUpgradeClient", true);
	sprintf(temp, "%s:%d", upgrade_attr.detail_ipaddr, upgrade_attr.file_port);
	snd_header->add_tag_text(snd_header, "Host", upgrade_attr.file_domain, true);
	snd_header->add_tag_text(snd_header, "Connection", "Keep-Alive", true);
	snd_header->add_tag_text(snd_header, "Accept", "*/*", true);
	//snd_header->add_tag_text(snd_header, "Accept-Encoding", "gzip, deflate", true);
	snd_header->to_text(snd_header, text, sizeof(text));	

	if(remote_upgrade_gethostbyname(upgrade_attr.file_domain, upgrade_attr.file_ipaddr) != 0){
		RU_TRACE("gethostbyname failed, %s", upgrade_attr.file_domain);
		goto ERR_EXIT1;
	}

	RU_TRACE("REQUEST size %d:\n%s", strlen(text), text);
	//RU_TRACE("FILE_PORT:%d", upgrade_attr.file_port);
	ret = tcp->connect(tcp, upgrade_attr.file_ipaddr, upgrade_attr.file_port);
	if(ret < 0){
		RU_TRACE("connect failed");
		goto ERR_EXIT1;
	}

	ret = tcp->send2(tcp, text, strlen(text), 0);
	if(ret < 0){
		RU_TRACE("send data failed");
		goto ERR_EXIT1;
	}

	/*sock = SOCK_tcp_connect2(upgrade_attr.detail_ipaddr, upgrade_attr.file_port, 40000);
	if(sock < 0){
	goto ERR_EXIT1;
	}

	ret = SOCK_send(sock, text, strlen(text));
	if(ret < 0){
		goto ERR_EXIT1;
	}

	ret = SOCK_recv(sock, text, sizeof(text) - 1, 0);
	RU_TRACE("recv:%s" , text);
	if( ret <= 0){
		goto ERR_EXIT1;
	}*/

	
	if(NULL == (rcv_header = HTTP_UTIL_recv_response_header(tcp->sock))){
		goto ERR_EXIT1;
	}

	pRev_length = (char *)rcv_header->read_tag(rcv_header, "Content-Length");
	if(NULL != pRev_length){
		// receive the content of http request
		rom_file_size = atoi(pRev_length);
	}

#if 0
	rom_fd = fopen(upgrade_attr.info.firmware_path, "w+b");
	upgrade_attr.upgrade_rate = 0;
	if(rom_fd){
		tmp_rate = upgrade_attr.upgrade_rate;
		while(rom_file_size > receive_file_size){
			//receive_size = tcp->recv2(tcp, text, sizeof(text), 0);
			receive_size = tcp->recv(tcp, text, sizeof(text), 0);
			if(receive_size <= 0){
				RU_TRACE("receive rom error!");
				break;
			}
			receive_file_size += receive_size;
			ret = fwrite(text, 1, receive_size, rom_fd);
			upgrade_attr.upgrade_rate = (receive_file_size * 50)/rom_file_size;
			if(upgrade_attr.upgrade_rate > tmp_rate){
				RU_TRACE("upgrade_rate:%d %d/%d", upgrade_attr.upgrade_rate, receive_file_size, rom_file_size);
				tmp_rate = upgrade_attr.upgrade_rate;
			}
		}
		fclose(rom_fd);
		RU_TRACE("receive_size:%d/%d", receive_file_size, rom_file_size);
		if(receive_size <= 0){
			goto ERR_EXIT1;
		}
	}else{
		goto ERR_EXIT1;
	}
	
	//tcp->close(tcp);
	snd_header->free(snd_header);
	rcv_header->free(rcv_header);
	snd_header = NULL;
	rcv_header = NULL;
	//return -1;

    GLOBAL_IOTdaemonExit();

	return remote_upgrade_write_rom_process();
#else
    void *upfile_mem = NULL;
    void* file_buf = NULL;
    printf("%s ota rev rom size %d\n", __FUNCTION__, rom_file_size);
	upfile_mem = FIRMWARE_upgrade_env_prepare(&rom_file_size);
    if(NULL == upfile_mem)
    {
        printf("%s() error line:%d\n", __FUNCTION__, __LINE__);
        goto ERR_EXIT1;
    }
    memset(upfile_mem, 0, rom_file_size);
    file_buf = upfile_mem;
	upgrade_attr.upgrade_rate = 0;
	tmp_rate = upgrade_attr.upgrade_rate;
	while(rom_file_size > receive_file_size){
		receive_size = tcp->recv(tcp, file_buf, sizeof(text), 0);
		if(receive_size <= 0)
		{
			RU_TRACE("receive rom error!");
			break;
		}
		receive_file_size += receive_size;
		file_buf = file_buf + receive_size;
		upgrade_attr.upgrade_rate = (receive_file_size * 50)/rom_file_size;
		if(upgrade_attr.upgrade_rate > tmp_rate)
		{
			RU_TRACE("upgrade_rate:%d %d/%d", upgrade_attr.upgrade_rate, receive_file_size, rom_file_size);
			tmp_rate = upgrade_attr.upgrade_rate;
		}
	}
	RU_TRACE("receive_size:%d/%d", receive_file_size, rom_file_size);
	if(receive_size <= 0){
		goto ERR_EXIT1;
	}

	//tcp->close(tcp);
	snd_header->free(snd_header);
	rcv_header->free(rcv_header);
	snd_header = NULL;
	rcv_header = NULL;
	//return -1;


	return remote_upgrade_write_rom_process(upfile_mem, rom_file_size);
#endif

ERR_EXIT1:
	RU_TRACE("%s failed!", __FUNCTION__);
	upgrade_attr.upgrade_errno = REMOTE_UPGRADE_ERROR_NO_SERVER_RESPONSE;
	if(snd_header) snd_header->free(snd_header);
	if(rcv_header) rcv_header->free(rcv_header);
	if(tcp->sock > 0) tcp->close(tcp);

	return -1;	
}

static int remote_upgrade_get_resource()
{
#if 0
	int ret;
	char text[1024*5];
	LP_HTTP_HEAD_FIELD snd_header = NULL;
	LP_HTTP_HEAD_FIELD rcv_header = NULL;
	char *pRev_length = NULL;
	int receive_size = 0, receive_size_cnt = 0, receive_file_size = 0;
	FILE *rom_fd = NULL;
	stSOCKET_TCP sockTCP;

	/*if(0 != upgrade_prase_resource_detail(REMOTE_UPGRADE_DETAIL_XML_DIR, NULL, &upgrade_attr.file_port, 
		upgrade_attr.rom_filepath, &upgrade_attr.upgrade_errno, 
		upgrade_attr.resource_version, upgrade_attr.vendor)){
		return -1;
	}*/

	lpSOCKET_TCP tcp = socket_tcp_r(&sockTCP);
	//set socket time out
	tcp->set_send_timeout(tcp, 60, 0);
	tcp->set_recv_timeout(tcp, 60, 0);

	snd_header = HTTP_UTIL_new_request_header("http", "1.1", "GET", upgrade_attr.rom_filepath, NULL);
	//snd_header->add_tag_text(snd_header, "Authorization", "Basic admin:", true);
	snd_header->add_tag_text(snd_header, "User-Agent", "RemoteUpgradeClient", true);
	snd_header->add_tag_text(snd_header, "Host", upgrade_attr.detail_ipaddr, true);
	snd_header->add_tag_text(snd_header, "Connection", "Keep-Alive", true);
	snd_header->add_tag_text(snd_header, "Accept", "*/*", true);
	snd_header->to_text(snd_header, text, sizeof(text));	


	RU_TRACE("REQUEST size %d:\n%s", strlen(text), text);

	ret = tcp->connect(tcp, upgrade_attr.detail_ipaddr, upgrade_attr.file_port);
	if(ret < 0){
		goto ERR_EXIT1;
	}

	ret = tcp->send2(tcp, text, strlen(text), 0);
	if(ret < 0){
		goto ERR_EXIT1;
	}

	
	if(NULL == (rcv_header = HTTP_UTIL_recv_response_header(tcp->sock))){
		goto ERR_EXIT1;
	}

	pRev_length = (char *)rcv_header->read_tag(rcv_header, "Content-Length");
	if(NULL != pRev_length){
		// receive the content of http request
		receive_file_size = atoi(pRev_length);
	}
	
	rom_fd = fopen(upgrade_attr.info.firmware_path, "w+b");
	if(rom_fd){
		while(receive_file_size > receive_size_cnt){
			//receive_size = tcp->recv2(tcp, text, sizeof(text), 0);
			receive_size = tcp->recv(tcp, text, sizeof(text), 0);
			if(receive_size < 0){
				RU_TRACE("receive rom error!");
				break;
			}
			receive_size_cnt += receive_size;
			ret = fwrite(text, 1, receive_size, rom_fd);
			RU_TRACE("receive_size:%d/%d", receive_size_cnt, receive_file_size);
		}
		fclose(rom_fd);
		if(receive_size < 0){
			goto ERR_EXIT1;
		}
	}else{
		goto ERR_EXIT1;
	}
	

	tcp->close(tcp);
	snd_header->free(snd_header);
	rcv_header->free(rcv_header);
	snd_header = NULL;
	rcv_header = NULL;

	return remote_upgrade_write_rom_process();
	
ERR_EXIT1:
	RU_TRACE("%s failed!", __FUNCTION__);
	upgrade_attr.upgrade_errno = REMOTE_UPGRADE_ERROR_NO_SERVER_RESPONSE;
	if(snd_header) snd_header->free(snd_header);
	if(rcv_header) rcv_header->free(rcv_header);
	if(tcp->sock > 0) tcp->close(tcp);
	
	return -1;	
#else
    return 0;
#endif
}

static void *remote_upgrade_process(void * arg)
{
	//do init
	upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_IDLE;
	prctl(PR_SET_NAME, "remote_upgrade_process");
	pthread_detach(pthread_self());
	
	while(upgrade_attr.trigger){
		RU_TRACE("upgrade_attr.upgrade_status:%d", upgrade_attr.upgrade_status);
		switch(upgrade_attr.upgrade_status){
			case REMOTE_UPGRADE_STATUS_IDLE:
				//fix me: to do the schedule
				upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_PREPARE;
			break;
			case REMOTE_UPGRADE_STATUS_PREPARE:
			{
				upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_CHECK_NETWORK;
				if(0 == upgrade_prase_detail_url(upgrade_attr.info.detail_path,  
					upgrade_attr.detail_domain, &upgrade_attr.detail_port, 
					upgrade_attr.info.model, upgrade_attr.info.FWversion,
					upgrade_attr.info.OEMnum, upgrade_attr.detail_filepath,
					upgrade_attr.info.deviceSN)){
					//prase url xml success					
				}else{
					upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_FAILED;
				}
			}
			break;
			case REMOTE_UPGRADE_STATUS_CHECK_NETWORK:
			{
				if(0 == remote_upgrade_gethostbyname(upgrade_attr.detail_domain, upgrade_attr.detail_ipaddr)){
					//prase url success	
					upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_CHECK_UPDATE_DETAIL;
				}else{
					upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_FAILED;
				}
			}
			break;
			case REMOTE_UPGRADE_STATUS_CHECK_UPDATE_DETAIL:
			{
				if(0 == remote_upgrade_detail_request()){
					upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_DOWNLOAD_ROOTFS;
				}/*else if(REMOTE_UPGRADE_ERROR_VERSION_LIMIT == upgrade_attr.upgrade_errno){
					//the app version is new
					upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_DOWNLOAD_RESOURCE;
				}*/else{
					upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_FAILED;
				}
				//upgrade_prase_rom_detail(REMOTE_UPGRADE_DETAIL_XML_DIR, 1, NULL, NULL, NULL);
			}
			break;
			case REMOTE_UPGRADE_STATUS_DOWNLOAD_ROOTFS:
			{			
				//remote_upgrade_check_reboot();
				#if 0
				if(upgrade_attr.info.function.env_prepare){
					upgrade_attr.info.function.env_prepare(upgrade_attr.need_reboot);
				}
				#endif
				if(0 == remote_upgrade_get_rom()){
					//fix me 
					//upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_DOWNLOAD_RESOURCE;
					upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_SUCCESS;
				}else{
					upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_FAILED;
				}				
			}
			break;
			case REMOTE_UPGRADE_STATUS_DOWNLOAD_RESOURCE:
			{
				//end of remote upgrade
				if(0 == remote_upgrade_get_resource()){
					upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_SUCCESS;
				}else{
					upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_FAILED;
				}
			}
			break;
			case REMOTE_UPGRADE_STATUS_FAILED:
			{
				//out of remote upgrade with error
				sleep(3);
				RU_TRACE("upgrade failed!");
				//SearchFileAndPlay(SOUND_Upgrade_failed, NK_True);
				if(upgrade_attr.info.function.ota_status_trap){
					upgrade_attr.info.function.ota_status_trap(OTA_STATUS_FAILED);
				}
				upgrade_attr.trigger = false;
				upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_IDLE;
			}
			break;
			case REMOTE_UPGRADE_STATUS_SUCCESS:
			{
				//remote upgrade success
				sleep(4);
				RU_TRACE("upgrade success!");
				if(upgrade_attr.info.function.ota_status_trap){
					upgrade_attr.info.function.ota_status_trap(OTA_STATUS_SUCCESS);
				}
				//SearchFileAndPlay(SOUND_Upgrade_completed, NK_True);
			}
			break;
			default:
				break;
		}
	}
	RU_TRACE("remote upgrade thread exit!-errno:%d", upgrade_attr.upgrade_errno);
	NK_SYSTEM("rm -rf /tmp/upgrade/dev");
	upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_IDLE;
	pthread_exit(NULL);
}

int REMOTE_UPGRADE_start(void)
{
	int ret = 0;
	RU_TRACE("%d-%d", upgrade_attr.trigger, upgrade_attr.upgrade_status);
	if(false == upgrade_attr.trigger && REMOTE_UPGRADE_STATUS_IDLE  == upgrade_attr.upgrade_status){
		//memset(&upgrade_attr, 0, sizeof(ST_REMOTE_UPGRADE));
		upgrade_attr.upgrade_errno = REMOTE_UPGRADE_ERROR_NONE;
		upgrade_attr.thread_tid = (pthread_t)NULL;
		if(upgrade_attr.info.function.ota_status_trap){
			upgrade_attr.info.function.ota_status_trap(OTA_STATUS_START);
		}
		//SearchFileAndPlay(SOUND_Upgrade, NK_True);
		sprintf(upgrade_attr.detail_domain, "update.e-seenet.com");
		RU_TRACE("start");
		GLOBAL_before_upgrade_destroy();
		upgrade_attr.trigger = true;
		ret = pthread_create(&upgrade_attr.thread_tid,NULL,(void *)remote_upgrade_process,NULL);
	}else{
		RU_TRACE("start failed");
		return -1;
	}
	return 0;
}

int REMOTE_UPGRADE_stop()
{
	if((pthread_t)NULL != upgrade_attr.thread_tid){
		//pthread_join(upgrade_attr.thread_tid, NULL);
		upgrade_attr.trigger = false;
		//upgrade_attr.thread_tid = NULL;
		//upgrade_attr.upgrade_status = REMOTE_UPGRADE_STATUS_IDLE;
	}
	return 0;
}

int REMOTE_UPGRADE_get_rate()
{
	return upgrade_attr.upgrade_rate;
}

int REMOTE_UPGRADE_get_status()
{
	return upgrade_attr.upgrade_status;
}

int REMOTE_UPGRADE_get_errno()
{
	return upgrade_attr.upgrade_errno;
}

int REMOTE_UPGRADE_init(bool need_reboot, LP_REMOTE_UPGRADE_DEVINFO attr)
{
	if(false == upgrade_attr.inited && NULL != attr){
		upgrade_attr.inited = true;
		upgrade_attr.need_reboot = need_reboot;
		memcpy(&upgrade_attr.info, attr, sizeof(ST_REMOTE_UPGRADE_DEVINFO));
		return 0;
	}	
	return -1;
}
