
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

//#include "hichip/hichip.h"
//#include "slog.h"
#ifdef __NVR
#include "_md5.h"
#else
#include "http_auth/_md5.h"
#endif
#include "nk_stream_rw.h"
#include "hichip_alarm.h"

#define HICHIP_ALARM_PARSE_MAX_NUM	(3)

#define HICHIP_alarm_return_val_if_fail(expr, val)	do{if(!(expr)) {return val;}}while(0)
#define HICHIP_alarm_free_set_null(ptr)	do{if(ptr != NULL) {free(ptr);ptr = NULL;}}while(0)

#ifndef SLOG_ERROR
#define SLOG_ERROR(module, fmt, arg...)	printf("\033[32;1m[%s::%s-%d]\033[0m "fmt"\r\n", module, basename(strdupa(__FILE__)), __LINE__, ##arg)
#endif
#ifndef SLOG_DEBUG
#define SLOG_DEBUG(module, fmt, arg...)
#endif

char *nk_msg_str[DETECTION_MSG_CNT] = 
{
		"md",
		"bi",
		"lv",
		"vo",
		"vm",
		"nd",
		"de",
		"db",
		"re",
		"ds",
		"sm",
		"rc",
		"dk",
		"other",
};

static void _HICHIP_ALARM_HEADER_ntohl(lpNK_HICHIP_ALARM_HEADER ptr)
{
	ptr->magic = ntohl(ptr->magic);
	ptr->length = ntohl(ptr->length);
	ptr->ver = ntohl(ptr->ver);
	ptr->type = ntohl(ptr->type);
}

static void _HICHIP_ALARM_HEADER_htonl(lpNK_HICHIP_ALARM_HEADER ptr)
{
	ptr->magic = htonl(ptr->magic);
	ptr->length = htonl(ptr->length);
	ptr->ver = htonl(ptr->ver);
	ptr->type = htonl(ptr->type);
}

static void _HICHIP_ALARM_BOUNDARY_ntohl(lpNK_HICHIP_ALARM_BOUNDARY ptr)
{
	ptr->type = ntohl(ptr->type);
}

static void _HICHIP_ALARM_BOUNDARY_htonl(lpNK_HICHIP_ALARM_BOUNDARY ptr)
{
	ptr->type = htonl(ptr->type);
}

static void _HICHIP_MD_DATA_ntohl(lpNK_HICHIP_MD_DATA ptr)
{
	ptr->chn = ntohl(ptr->chn);
}

static void _HICHIP_MD_DATA_htonl(lpNK_HICHIP_MD_DATA ptr)
{
	ptr->chn = htonl(ptr->chn);
}

static void _HICHIP_IO_DATA_ntohl(lpNK_HICHIP_IO_DATA ptr)
{
	ptr->chn = ntohl(ptr->chn);
	ptr->type = ntohl(ptr->type);
}

static void _HICHIP_IO_DATA_htonl(lpNK_HICHIP_IO_DATA ptr)
{
	ptr->chn = htonl(ptr->chn);
	ptr->type = htonl(ptr->type);
}

static void _HICHIP_HEARTBEAT_DATA_ntohl(lpNK_HICHIP_HEARTBEAT_DATA ptr)
{
	ptr->status = ntohl(ptr->status);
}

static void _HICHIP_HEARTBEAT_DATA_htonl(lpNK_HICHIP_HEARTBEAT_DATA ptr)
{
	ptr->status = htonl(ptr->status);
}

static void _HICHIP_OPERATION_DATA_ntohl(lpNK_HICHIP_OPERATION_DATA ptr)
{
	ptr->status = ntohl(ptr->status);
}

static void _HICHIP_OPERATION_DATA_htonl(lpNK_HICHIP_OPERATION_DATA ptr)
{
	ptr->status = htonl(ptr->status);
}

static void HICHIP_alarm_calculate_md5(unsigned char digest[16])
{
	struct MD5Context MD5_context;
	char *MD5_buf = "_+N1ALARM+CSUM+_";
	static unsigned char MD5_digest[16] = {0};

	memset(digest, 0, sizeof(MD5_digest));
	if(memcmp(digest, MD5_digest, sizeof(MD5_digest)) == 0){
		memset(&MD5_context, 0, sizeof(MD5_context));
		MD5Init(&MD5_context);
		MD5Update(&MD5_context, (unsigned char *)MD5_buf, strlen(MD5_buf));
		MD5Final(MD5_digest, &MD5_context);
	}
	memcpy(digest, MD5_digest, sizeof(MD5_digest));
}

static int HICHIP_alarm_parse_v10(char *data, int max_len, lpNK_HICHIP_ALARM_DATA alarm_data, int max_num)
{
	int count = 0, data_size;
	char *ptr = NULL;
	void *p_data = NULL;
    lpNK_HICHIP_ALARM_BOUNDARY p_boundary = NULL;

	ptr = data;
	ptr += sizeof(stNK_HICHIP_ALARM_HEADER);
    p_boundary = (lpNK_HICHIP_ALARM_BOUNDARY)ptr;
	_HICHIP_ALARM_BOUNDARY_ntohl(p_boundary);

    while(p_boundary->type != NK_HICHIP_ALARM_END && ptr - data < max_len){
		data_size = 0;
		ptr += sizeof(stNK_HICHIP_ALARM_BOUNDARY);
        if(p_boundary->type == NK_HICHIP_ALARM_MD){
            data_size = sizeof(stNK_HICHIP_MD_DATA);
			p_data = (lpNK_HICHIP_MD_DATA)ptr;
			_HICHIP_MD_DATA_ntohl(p_data);
			memcpy(&alarm_data[count].md, p_data, data_size);
        }
        else if(p_boundary->type == NK_HICHIP_ALARM_IO){
            data_size = sizeof(stNK_HICHIP_IO_DATA);
            p_data = (lpNK_HICHIP_IO_DATA)ptr;
			_HICHIP_IO_DATA_ntohl(p_data);
			memcpy(&alarm_data[count].io, p_data, data_size);
        }
        else if(p_boundary->type == NK_HICHIP_ALARM_HEARDBEAT){
            data_size = sizeof(stNK_HICHIP_HEARTBEAT_DATA);
            p_data = (lpNK_HICHIP_HEARTBEAT_DATA)ptr;
			_HICHIP_HEARTBEAT_DATA_ntohl(p_data);
			memcpy(&alarm_data[count].io, p_data, data_size);
        }
        else if(p_boundary->type == NK_HICHIP_ALARM_OPERATION){
            data_size = sizeof(stNK_HICHIP_OPERATION_DATA);
            p_data = (lpNK_HICHIP_OPERATION_DATA)ptr;
			_HICHIP_OPERATION_DATA_ntohl(p_data);
			memcpy(&alarm_data[count].io, p_data, data_size);
        }
        else{
            //unknow type
            break;
        }
		alarm_data[count].type = p_boundary->type;
		count++;
		if(count >= max_num){
			break;
		}
		ptr += data_size;
		p_boundary = (lpNK_HICHIP_ALARM_BOUNDARY)ptr;
		_HICHIP_ALARM_BOUNDARY_ntohl(p_boundary);
    }
	
    return count;
}

int HICHIP_alarm_parse2_init(lpNK_HICHIP_ALARM_ARG alarmArg)
{
	if(alarmArg->alarmType == NULL){
		alarmArg->alarmType = calloc(HICHIP_ALARM_PARSE_MAX_NUM, sizeof(stNK_HICHIP_ALARM_TYPE));
	}
	if(alarmArg->ioData == NULL){
		alarmArg->ioData = calloc(HICHIP_ALARM_PARSE_MAX_NUM, sizeof(stNK_HICHIP_IO_DATA));
	}
	if(alarmArg->heartBeatData == NULL){
		alarmArg->heartBeatData = calloc(HICHIP_ALARM_PARSE_MAX_NUM, sizeof(stNK_HICHIP_HEARTBEAT_DATA));
	}
	if(alarmArg->operationData == NULL){
		alarmArg->operationData = calloc(HICHIP_ALARM_PARSE_MAX_NUM, sizeof(stNK_HICHIP_OPERATION_DATA));
	}

	return 0;
}

int HICHIP_alarm_parse2_deinit(lpNK_HICHIP_ALARM_ARG alarmArg)
{
	HICHIP_alarm_free_set_null(alarmArg->alarmType);
	HICHIP_alarm_free_set_null(alarmArg->ioData);
	HICHIP_alarm_free_set_null(alarmArg->heartBeatData);
	HICHIP_alarm_free_set_null(alarmArg->operationData);

	return 0;
}

static int _HICHIP_alarm_parse(char *pbuf, int len, lpNK_HICHIP_ALARM_ARG alarm_arg, lpNK_HICHIP_ALARM_DATA retData, int maxNum)
{
	int i, ret = -1;
	char *ptr = NULL;
	lpNK_HICHIP_ALARM_HEADER p_alarm_header = NULL;
	lpNK_HICHIP_ALARM_DATA tmp_data = NULL;
	unsigned char digest[16];
	
	do{
		if((retData != NULL && maxNum < 1) || (retData == NULL && alarm_arg == NULL)){
			break;
		}
		if((ptr = pbuf) == NULL){
			SLOG_ERROR("HICHIP", "buf is NULL");
		    break;
		}
		if(len < sizeof(stNK_HICHIP_ALARM_HEADER)){
			SLOG_ERROR("HICHIP", "alarm buf(%d) is too small(%d)", len, sizeof(stNK_HICHIP_ALARM_HEADER));
		    break;
		}		
		p_alarm_header = (lpNK_HICHIP_ALARM_HEADER)ptr;
		_HICHIP_ALARM_HEADER_ntohl(p_alarm_header);
		if(p_alarm_header->magic != NK_HICHIP_ALARM_MAGIC){
			SLOG_ERROR("HICHIP", "alarm magic error");
			break;
		}
		else if(len - sizeof(p_alarm_header->magic) < p_alarm_header->length){
			SLOG_ERROR("HICHIP", "alarm length error");
			break;
		}
		else{
			HICHIP_alarm_calculate_md5(digest);
			if(memcmp(p_alarm_header->checksum, digest, sizeof(p_alarm_header->checksum)) != 0){
				SLOG_ERROR("HICHIP", "alarm md5 error");
				break;
			}
		}
		if(p_alarm_header->ver == NK_HICHIP_ALARM_VER_10){
			if(retData != NULL){
				return HICHIP_alarm_parse_v10(ptr, len, retData, maxNum);
			}
			else{
				tmp_data = alloca(sizeof(stNK_HICHIP_ALARM_DATA) * HICHIP_ALARM_PARSE_MAX_NUM);
				HICHIP_alarm_return_val_if_fail(tmp_data != NULL, -1);
				ret = HICHIP_alarm_parse_v10(ptr, len, tmp_data, HICHIP_ALARM_PARSE_MAX_NUM);
			}
			if(ret > 0){
				alarm_arg->alarmNum = 0;
				alarm_arg->ioDataNum = 0;
				alarm_arg->heartBeatDataNum = 0;
				alarm_arg->operationDataNum = 0;
				for(i = 0; i < ret; i++){
					switch(tmp_data[i].type){
						case NK_HICHIP_ALARM_MD:
							if(alarm_arg->alarmNum < HICHIP_ALARM_PARSE_MAX_NUM){
								alarm_arg->alarmType[alarm_arg->alarmNum].chn = tmp_data[i].md.chn;
								alarm_arg->alarmType[alarm_arg->alarmNum].type = DETECTION_MSG_MD;
								alarm_arg->alarmNum++;
							}
							break;
						case NK_HICHIP_ALARM_IO:
							switch(tmp_data[i].io.type){
								case NK_HICHIP_IO_PIR:
									if(alarm_arg->alarmNum < HICHIP_ALARM_PARSE_MAX_NUM){
										alarm_arg->alarmType[alarm_arg->alarmNum].chn = tmp_data[i].io.chn;
										alarm_arg->alarmType[alarm_arg->alarmNum].type = DETECTION_MSG_BI;
										alarm_arg->alarmNum++;
									}
									break;
								case NK_HICHIP_IO_TF:
									if(alarm_arg->ioDataNum < HICHIP_ALARM_PARSE_MAX_NUM){
										memcpy(&alarm_arg->ioData[alarm_arg->ioDataNum], &tmp_data[i].io, sizeof(stNK_HICHIP_IO_DATA));
										alarm_arg->ioDataNum++;
									}
									break;
								default:
									break;
							}
							break;
						case NK_HICHIP_ALARM_HEARDBEAT:
							if(alarm_arg->heartBeatDataNum < HICHIP_ALARM_PARSE_MAX_NUM){
								memcpy(&alarm_arg->heartBeatData[alarm_arg->heartBeatDataNum], &tmp_data[i].heartBeat, sizeof(stNK_HICHIP_HEARTBEAT_DATA));
								alarm_arg->heartBeatDataNum++;
							}
							break;
						case NK_HICHIP_ALARM_OPERATION:
							if(alarm_arg->operationDataNum < HICHIP_ALARM_PARSE_MAX_NUM){
								memcpy(&alarm_arg->operationData[alarm_arg->operationDataNum], &tmp_data[i].operation, sizeof(stNK_HICHIP_OPERATION_DATA));
								alarm_arg->operationDataNum++;
							}
							break;
						default:
							break;
					}
				}
				SLOG_DEBUG("HICHIP", "recv %d alarm", ret);
				for(ret = 0; ret < alarm_arg->alarmNum; ret++){
					SLOG_DEBUG("HICHIP", "recv[%d] CH%d-%d", ret, alarm_arg->alarmType[ret].chn, alarm_arg->alarmType[ret].type);
				}
				ret = 0;
			}
			else{
				ret = -1;
			}
		}
	}while(0);

	return ret;
}

int HICHIP_alarm_parse(char *pbuf, int len, lpNK_HICHIP_ALARM_DATA retData, int maxNum)
{
	return _HICHIP_alarm_parse(pbuf, len, NULL, retData, maxNum);
}

int HICHIP_alarm_parse2(char *pbuf, int len, lpNK_HICHIP_ALARM_ARG alarmArg)
{
	return _HICHIP_alarm_parse(pbuf, len, alarmArg, NULL, 0);
}

static int HICHIP_alarm_construct_v10(char *data, int data_size, lpNK_HICHIP_ALARM_DATA alarm_data, int max_num)
{
	stNK_HICHIP_ALARM_HEADER alarm_header = {0};
	stNK_HICHIP_ALARM_BOUNDARY alarm_boundary = {0};
	lpNK_STREAM_RW rwops = NULL;
	int i, total_size;

	if(data_size <= sizeof(stNK_HICHIP_ALARM_HEADER) + sizeof(stNK_HICHIP_ALARM_BOUNDARY)){
		return -1;
	}
	rwops = alloca(sizeof(stNK_STREAM_RW));
	HICHIP_alarm_return_val_if_fail(rwops != NULL, -1);
	NK_STREAM_RWFromMem(rwops, data + sizeof(stNK_HICHIP_ALARM_HEADER), data_size - sizeof(stNK_HICHIP_ALARM_HEADER), 0);

	//alarm data
	for(i = 0; i < max_num; i++){
		switch(alarm_data[i].type){
			case NK_HICHIP_ALARM_MD:
				alarm_boundary.type = htonl(NK_HICHIP_ALARM_MD);
				HICHIP_alarm_return_val_if_fail(NK_STREAM_RWwrite(rwops, &alarm_boundary, sizeof(stNK_HICHIP_ALARM_BOUNDARY), 1) == 1, -1);
				_HICHIP_MD_DATA_htonl(&alarm_data[i].md);
				HICHIP_alarm_return_val_if_fail(NK_STREAM_RWwrite(rwops, &alarm_data[i].md, sizeof(stNK_HICHIP_MD_DATA), 1) == 1, -1);
				alarm_header.type = alarm_header.type | NK_HICHIP_ALARM_MD;
				break;
			case NK_HICHIP_ALARM_IO:
				alarm_boundary.type = htonl(NK_HICHIP_ALARM_IO);
				HICHIP_alarm_return_val_if_fail(NK_STREAM_RWwrite(rwops, &alarm_boundary, sizeof(stNK_HICHIP_ALARM_BOUNDARY), 1) == 1, -1);
				_HICHIP_IO_DATA_htonl(&alarm_data[i].io);
				HICHIP_alarm_return_val_if_fail(NK_STREAM_RWwrite(rwops, &alarm_data[i].io, sizeof(stNK_HICHIP_IO_DATA), 1) == 1, -1);
				alarm_header.type = alarm_header.type | NK_HICHIP_ALARM_IO;
				break;
			case NK_HICHIP_ALARM_HEARDBEAT:
				alarm_boundary.type = htonl(NK_HICHIP_ALARM_HEARDBEAT);
				HICHIP_alarm_return_val_if_fail(NK_STREAM_RWwrite(rwops, &alarm_boundary, sizeof(stNK_HICHIP_ALARM_BOUNDARY), 1) == 1, -1);
				_HICHIP_HEARTBEAT_DATA_htonl(&alarm_data[i].heartBeat);
				HICHIP_alarm_return_val_if_fail(NK_STREAM_RWwrite(rwops, &alarm_data[i].heartBeat, sizeof(stNK_HICHIP_HEARTBEAT_DATA), 1) == 1, -1);
				alarm_header.type = alarm_header.type | NK_HICHIP_ALARM_HEARDBEAT;
				break;
			case NK_HICHIP_ALARM_OPERATION:
				alarm_boundary.type = htonl(NK_HICHIP_ALARM_OPERATION);
				HICHIP_alarm_return_val_if_fail(NK_STREAM_RWwrite(rwops, &alarm_boundary, sizeof(stNK_HICHIP_ALARM_BOUNDARY), 1) == 1, -1);
				_HICHIP_OPERATION_DATA_htonl(&alarm_data[i].operation);
				HICHIP_alarm_return_val_if_fail(NK_STREAM_RWwrite(rwops, &alarm_data[i].operation, sizeof(stNK_HICHIP_OPERATION_DATA), 1) == 1, -1);
				alarm_header.type = alarm_header.type | NK_HICHIP_ALARM_OPERATION;
				break;
			case NK_HICHIP_ALARM_END:
			default:
				break;
		}
	}

	//alarm end
	alarm_boundary.type = htonl(NK_HICHIP_ALARM_END);
	NK_STREAM_RWwrite(rwops, &alarm_boundary, sizeof(stNK_HICHIP_ALARM_BOUNDARY), 1);
	HICHIP_alarm_return_val_if_fail(NK_STREAM_RWwrite(rwops, &alarm_boundary, sizeof(stNK_HICHIP_ALARM_BOUNDARY), 1) == 1, -1);

	//alarm header
	total_size = sizeof(stNK_HICHIP_ALARM_HEADER) + NK_STREAM_RWsize(rwops);
	alarm_header.magic = NK_HICHIP_ALARM_MAGIC;
	alarm_header.length = total_size - sizeof(alarm_header.magic);
	alarm_header.ver = NK_HICHIP_ALARM_VER_10;
	HICHIP_alarm_calculate_md5(alarm_header.checksum);
	_HICHIP_ALARM_HEADER_htonl(&alarm_header);
	memcpy(data, &alarm_header, sizeof(stNK_HICHIP_ALARM_HEADER));

	return total_size;
}

int HICHIP_alarm_construct(char *retBuf, int bufSize, lpNK_HICHIP_ALARM_DATA alarmData, int alarmNum)
{
	if(alarmData != NULL && alarmNum > 0 && retBuf != NULL && bufSize > 0){
		return HICHIP_alarm_construct_v10(retBuf, bufSize, alarmData, alarmNum);
	}

	return -1;
}

