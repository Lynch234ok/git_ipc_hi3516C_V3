#include <stdio.h>
#include <pthread.h>
#include "generic.h"
#include "socket_tcp.h"
#include "http_util.h"
#include "app_recover.h"
#include "securedat.h"
#include "base/ja_process.h"


void app_recover_proc(const char* ip)
{
	int ret;
	//const char *ip = (const char*)arg;
	char text[2048];
	LP_HTTP_HEAD_FIELD snd_header = NULL;
	LP_HTTP_HEAD_FIELD rcv_header = NULL;
	char *operation = NULL;
	stSOCKET_TCP sockTCP;
	lpSOCKET_TCP tcp = socket_tcp_r(&sockTCP);
	//set socket time out
	tcp->set_send_timeout(tcp, 2, 0);
	tcp->set_recv_timeout(tcp, 2, 0);

	char sn_str[32] = {0};
	char esee_id[32] = {0};
	if(0 == UC_SNumberGet(sn_str)) {
		if(strlen(sn_str)> 10){
			if(sn_str[strlen(sn_str) - 10] == '0'){
				memcpy(esee_id, &sn_str[strlen(sn_str) - 10+1], 9);
			}else{
				memcpy(esee_id, &sn_str[strlen(sn_str) - 10], 10);
			}
		}
	}
	printf("esee id:%s\n", esee_id);
	char base64_id[64] = {0};
	base64_encode(esee_id, base64_id, strlen(esee_id));

	snd_header = HTTP_UTIL_new_request_header("http", "1.1", "GET", "recover.cgi", NULL);
	//snd_header->add_tag_text(snd_header, "Authorization", "Basic admin:", true);
	snd_header->add_tag_text(snd_header, "User-Agent", "JUAN SREACHER", true);
	snd_header->add_tag_text(snd_header, "Host", ip, true);
	snd_header->add_tag_text(snd_header, "Connection", "Keep-Alive", true);
	snd_header->add_tag_text(snd_header, "Accept", "*/*", true);
	snd_header->add_tag_text(snd_header, "Device-ID", base64_id, true);
	snd_header->to_text(snd_header, text, sizeof(text));	


	//printf("REQUEST size %d:\n%s\r\n", strlen(text), text);

	ret = tcp->connect(tcp, ip, 80);
	if(ret < 0){
		printf("connect %s failed!\r\n", ip);
		goto ERR_EXIT1;
	}

	ret = tcp->send2(tcp, text, strlen(text), 0);
	if(ret < 0){
		printf("send recover.cgi failed!\r\n");
		goto ERR_EXIT1;
	}

	
	if(NULL == (rcv_header = HTTP_UTIL_recv_response_header(tcp->sock))){
		printf("receive recover.cgi failed!\r\n");
		goto ERR_EXIT1;
	}

	operation = rcv_header->read_tag(rcv_header, "Operation");
	printf("receive operation:%s/%d\r\n", operation, rcv_header->status_code);
	if(NULL != operation && !strcmp(operation, "Recover") && 200 == rcv_header->status_code){//recover config
		NK_SYSTEM("rm -rf /media/conf/netsdk");
		printf("JUAN IPC RECOVER!\r\n");
		//NK_SYSTEM("reboot");
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
	return;
		
ERR_EXIT1:
	if(snd_header) snd_header->free(snd_header);
	if(rcv_header) rcv_header->free(rcv_header);
	if(tcp->sock > 0) tcp->close(tcp);
	
	return;	


}

static unsigned int recover_do_crc32(void* data, ssize_t data_sz)
{
	int i = 0;
	uint32_t* data_32 = data;
	uint32_t data_sz_32 = data_sz / sizeof(uint32_t);
	uint32_t const crc32_origin = 0xfefeef11;
	uint32_t crc32_result = crc32_origin;
	for(i = 0; i < data_sz_32; ++i){
		crc32_result ^= data_32[i];
	}
	return crc32_result;
}


int check_file(const char *srcpath, const char *dstpath, const char *filename)// -1:different 0:same
{
	int ret = -1;
	unsigned int src_crc, dst_crc;
	char srcfile[128], dstfile[128];
	FILE *src_fd = NULL, *dst_fd = NULL;
	if(srcpath && dstpath && filename){
		snprintf(srcfile, sizeof(srcfile), "%s/%s", srcpath, filename);
		snprintf(dstfile, sizeof(dstfile), "%s/%s", dstpath, filename);
		src_fd = fopen(srcfile, "rb");
		dst_fd = fopen(dstfile, "rb");
		if(src_fd && dst_fd){
			//get file stat
			struct stat srcfile_stat, dstfile_stat;
			fstat(fileno(src_fd), &srcfile_stat);
			fstat(fileno(dst_fd), &dstfile_stat);
			if(srcfile_stat.st_size == dstfile_stat.st_size){
				char *srcbuf = calloc(srcfile_stat.st_size + 16, 1);
				char *dstbuf = calloc(dstfile_stat.st_size + 16, 1);
				if(srcbuf && dstbuf){
					fread(srcbuf, srcfile_stat.st_size, 1, src_fd);
					fread(dstbuf, dstfile_stat.st_size, 1, dst_fd);
					src_crc = recover_do_crc32(srcbuf, srcfile_stat.st_size);
					dst_crc = recover_do_crc32(dstbuf, dstfile_stat.st_size);
					printf("file:%s CRC:%x/%x\r\n", filename, src_crc, dst_crc);
					if(src_crc == dst_crc){
						//same file
						ret = 0;
					}
					free(srcbuf);
					free(dstbuf);
				}
			}
		}
	}
	return ret;
}

int config_file_check(const char *srcpath, const char *dstpath)
{
	char srcfile[128], dstfile[128];
 	if(check_file(srcpath, dstpath, "audio.json")||
		check_file(srcpath, dstpath, "image.json") ||
		check_file(srcpath, dstpath, "network.json") ||
		check_file(srcpath, dstpath, "stream.json") ||
		check_file(srcpath, dstpath, "video.json") ||
		check_file(srcpath, dstpath, "io.json") ||
		check_file(srcpath, dstpath, "ptz.json") ||
		check_file(srcpath, dstpath, "system.json")){
		//the files are different
		return -1;
	}
	//the files are the same
	return 0;
}

int APP_RECOVER_init(const char *ip, const char *srcpath, const char *dstpath)
{
	if(!check_ipv4_addr(ip) && config_file_check(srcpath, dstpath)){
		app_recover_proc(ip);
	}
}
