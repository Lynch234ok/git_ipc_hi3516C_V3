
#include "firmware.h"
#include <sys/mount.h>
#include "md5sum.h"
#include "version.h"
#include "generic.h"
#include "base/ja_process.h"


#define FIRMWARE_IMPORT_BUFFER_SZ (256 * 1024)

#define FIRMWARE_IMPORT_HTTP_FILE "/latest/FW_HI3507_JAN72.rom"

#define FIRMWARE_IMPORT_FROM_FOLDER_FLAG (1<<0)
#define FIRMWARE_IMPORT_FROM_FTP_FLAG (1<<1)
#define FIRMWARE_IMPORT_FROM_HTTP_FLAG (1<<2)

struct FwImportFromHttp
{
	ssize_t file_size;
	ssize_t recv_size;
};

typedef struct FwImport
{
	uint32_t flag;
	struct FwImportFromHttp from_http;
}FwImport_t;
static FwImport_t* _fw_import = NULL;

void FIRMWARE_import_clear()
{
	unlink(FIRMWARE_IMPORT_FILE);
	remove(FIRMWARE_IMPORT_FILE);
}

uint8_t _firmware_check_version_limit_ex(FwVersion_t firmware_version, FwVersion_t cur_version)
{
	int ret = 0;

	if(firmware_version.major > cur_version.major){
		ret = 1;
	}

	if(firmware_version.major == cur_version.major && firmware_version.minor >= cur_version.minor){
		ret = 1;
	}

	printf("firmware ver is %s, firmware :%d.%d.%d  cur:%d.%d.%d\r\n", ret ? "new" : "old",
		firmware_version.major, firmware_version.minor, firmware_version.revision,
		cur_version.major, cur_version.minor, cur_version.revision);
	return ret;
}

uint8_t _firmware_version_compare(FwVersion_t higher, FwVersion_t lower)
{
	if(higher.major > lower.major){
		return 1;
	}

	if(higher.major == lower.major && higher.minor > lower.minor){
		return 1;
	}

	if(higher.major == lower.major && higher.minor == lower.minor && higher.revision >= lower.revision){
		return 1;
	}

	return 0;
}

uint8_t _firmware_check_version_limit(FwVersionLimit_t limit_version, FwVersion_t cur_version)
{
	int ret = 0;
	if(_firmware_version_compare(cur_version, limit_version.low_version) 
		&& _firmware_version_compare(limit_version.high_version, cur_version)){
		ret = 1;
	}

	printf("%s version low:%d.%d.%d  high:%d.%d.%d  cur:%d.%d.%d\r\n", ret ? "access" : "limit",
			limit_version.low_version.major, limit_version.low_version.minor, limit_version.low_version.revision,
			limit_version.high_version.major, limit_version.high_version.minor, limit_version.high_version.revision,
			cur_version.major, cur_version.minor, cur_version.revision);

	return ret;
}

int FIRMWARE_import_get_rate()
{
	if(_fw_import->flag & FIRMWARE_IMPORT_FROM_FOLDER_FLAG){
		;
	}else if(_fw_import->flag & FIRMWARE_IMPORT_FROM_FTP_FLAG){
		;
	}else if(_fw_import->flag & FIRMWARE_IMPORT_FROM_HTTP_FLAG){
		if(0 == _fw_import->from_http.recv_size){
			return 0;
		}
		return _fw_import->from_http.recv_size * 100 / _fw_import->from_http.file_size;
	}
	return 0;
}

int FIRMWARE_import_from_folder(const char* folder, FIRMWARE_IMPORT_MATCH match)
{
	// FIXME:
	return -1;
}

int FIRMWARE_import_from_ftp(const char* ftp_addr, const char* user, const char* password)
{
	// FIXME:
	return -1;
}

int FIRMWARE_import_from_http(const char* addr, uint16_t port)
{
	printf("%s start\n", __FUNCTION__);
	int ret = 0;
	int reuse_on = 1;
	int sock = -1;
	char buf[FIRMWARE_IMPORT_BUFFER_SZ];
	char* buf_ptr = NULL;
	struct sockaddr_in server_addr;
	ssize_t buf_sz = 0;
	FILE* fid = NULL;
	const char* http_get =
		"GET " FIRMWARE_IMPORT_HTTP_FILE " HTTP/1.0\r\n"
		"Connection: close\r\n"
		"\r\n";
//
// 1. setup sock -> connect to sever -> send 'GET' to request file
// 2. recevie http response -> analyse the header and the the filesize
// 3. loop to receive the whole file
// 4. success to import a file from http server
//

	// setup flag
	_fw_import->flag |= FIRMWARE_IMPORT_FROM_HTTP_FLAG;
	memset(&_fw_import->from_http, 0, sizeof(_fw_import->from_http));

///////////////////////////////////////////////////////////////////////////////
// step 1
///////////////////////////////////////////////////////////////////////////////
	// create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		goto FIRMWARE_import_from_http_err1;
	}
	// port reuse active
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_on, sizeof(int));
	assert(0 == ret);

	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(port);
	ret = connect(sock,(struct sockaddr*)&server_addr, sizeof(server_addr));
	if(ret < 0){
		// connect to server failed
		goto FIRMWARE_import_from_http_err2;
	}

	ret = send(sock, http_get, strlen(http_get), 0);
	assert(ret > 0);

///////////////////////////////////////////////////////////////////////////////
// step 2
///////////////////////////////////////////////////////////////////////////////
	// receive file
	memset(buf, 0, sizeof(buf));
	ret = recv(sock, buf, sizeof(buf), 0);
	buf[ret] = 0;
	// file not found or other problems
	buf_ptr = strstr(buf, "200 OK\r\n");
	if(!buf_ptr){
		goto FIRMWARE_import_from_http_err2;
	}

	buf_ptr = strstr(buf, "Content-Length:");
	if(!buf_ptr){
		goto FIRMWARE_import_from_http_err2;
	}

//	sscanf(buf, "Content-Length: %d\r\n", &fw_sz);
	_fw_import->from_http.file_size = atoi(buf_ptr + strlen("Content-Length:"));
	if(_fw_import->from_http.file_size > FIRMWARE_max_rom_size()){
		// file is too large
		goto FIRMWARE_import_from_http_err2;
	}

	// seek the end of http header
	buf_ptr = strstr(buf, "\r\n\r\n");
	if(!buf_ptr){
		goto FIRMWARE_import_from_http_err2;
	}

///////////////////////////////////////////////////////////////////////////////
// step 3
///////////////////////////////////////////////////////////////////////////////
	// clear the recv size stats
	_fw_import->from_http.recv_size = 0;

	fid = fopen(FIRMWARE_IMPORT_FILE, "wb");
	assert(fid);
	buf_sz = ret - (buf_ptr + strlen("\r\n\r\n") - buf); // receive data size - header
	ret = fwrite(buf_ptr + strlen("\r\n\r\n"), 1, buf_sz, fid);
	assert(ret == buf_sz);
	_fw_import->from_http.recv_size += buf_sz;

	while(1){
		ret = recv(sock, buf, sizeof(buf), 0);
		if(ret < 0){
			goto FIRMWARE_import_from_http_err3;
		}
		if(0 == ret){
			break;
		}
		buf_sz = ret;
		ret = fwrite(buf, 1, buf_sz, fid);
		assert(ret == buf_sz);
		_fw_import->from_http.recv_size += buf_sz;

		printf("download: %d%% from %s\r\n", FIRMWARE_import_get_rate(), addr);
	}
	fclose(fid);
	fid = NULL;

	close(sock);
	sock = -1;

//	NK_SYSTEM("ls -l " FIRMWARE_IMPORT_FILE);
	// success
	_fw_import->flag &= ~FIRMWARE_IMPORT_FROM_HTTP_FLAG;
	printf("%s end\n", __FUNCTION__);
	return 0;

FIRMWARE_import_from_http_err3:
	printf("FIRMWARE_import_from_http_err3\n");
	fclose(fid);
	fid = NULL;
	FIRMWARE_import_clear();
FIRMWARE_import_from_http_err2:
	printf("FIRMWARE_import_from_http_err2\n");
	close(sock);
	sock = -1;
FIRMWARE_import_from_http_err1:
	printf("FIRMWARE_import_from_http_err1\n");
	_fw_import->flag &= ~FIRMWARE_IMPORT_FROM_HTTP_FLAG;
	return -1;
}

#define FIRMWARE_MD5_CHECK_TAG "MD5 CHECK PASSED QC LAW!"
uint32_t FIRMWARE_import_check(char *soc, int *ret_errno, bool downgrade, const char *firmware_file)
{
	printf("%s start\n", __FUNCTION__);
	int ret = 0;
	FwHeader_t *header = NULL;
	char md5sum[64] = {""};
	char* p_md5 = NULL;
	FILE* fid = NULL;
	//return true;
	fid = fopen(firmware_file, "r+");
	if(fid){

        header = calloc(sizeof(FwHeader_t), 1);
        if(NULL == header)
        {
            fclose(fid);
            return false;
        }

		// get the md5 from firmware file
		ret = fread(header, 1, sizeof(FwHeader_t), fid);
		assert(sizeof(FwHeader_t) == ret);
		if(!strcmp(soc, header->soc) || (!strcmp(soc, "N16C")&& !strcmp(header->soc, "N18A"))){
			strcpy(md5sum, &header->md5_sum[32]);
			if(0 == strcmp(md5sum, FIRMWARE_MD5_CHECK_TAG) && strlen(md5sum) == strlen(FIRMWARE_MD5_CHECK_TAG)){
				// has been check
				fclose(fid);
				FwVersion_t cur_version;
				cur_version.major = SWVER_MAJ;
				cur_version.minor = SWVER_MIN;
				cur_version.revision = SWVER_REV;
				sprintf(cur_version.extend, SWVER_EXT);
				if(downgrade || (_firmware_check_version_limit(header->version_limit,cur_version)
					&& _firmware_check_version_limit_ex(header->version, cur_version))){
					*ret_errno = FIRMWARE_UPGRADE_ERROR_NONE;

                    if(NULL != header)
                    {
                        free(header);
                        header = NULL;
                    }
					return true;
				}else{
					goto FALSE_RETURN;
				}
			}else{
				// first time to check
				memset(header->md5_sum, 0, sizeof(header->md5_sum));
				ret = fseek(fid, 0, SEEK_SET);
				assert(0 == ret);
				ret = fwrite(header, 1, sizeof(FwHeader_t), fid);
				assert(sizeof(FwHeader_t) == ret);
				p_md5 = md5sum_file(firmware_file);
				if(0 == strcmp(md5sum, p_md5) && strlen(md5sum) == strlen(p_md5)){
					// passed
					strcpy(&header->md5_sum[32], FIRMWARE_MD5_CHECK_TAG);
					ret = fseek(fid, 0, SEEK_SET);
					assert(0 == ret);
					ret = fwrite(header, 1, sizeof(FwHeader_t), fid);
					assert(sizeof(FwHeader_t) == ret);
					fclose(fid);

                    if(NULL != header)
                    {
                        free(header);
                        header = NULL;
                    }
					return FIRMWARE_import_check(soc, ret_errno, downgrade, firmware_file);
				}else{
					fclose(fid);
					printf("md5 mismatch %s/%s\r\n", p_md5, md5sum);
					*ret_errno = FIRMWARE_UPGRADE_ERROR_MISMATCH_MD5;
					goto FALSE_RETURN;
				}
			}
		}
		else{
			fclose(fid);
			printf("soc mismatch %s/%s\r\n", soc, header->soc);
			*ret_errno = FIRMWARE_UPGRADE_ERROR_MISMATCH_SOC;
			goto FALSE_RETURN;
		}
	}

FALSE_RETURN:

    if(NULL != header)
    {
        free(header);
        header = NULL;
    }

	return false;
}

uint32_t FIRMWARE_import_check1(char *soc, char *ret_errno, char *filename)
{
	printf("%s start\n", __FUNCTION__);
	int ret = 0;
	FwHeader_t *header = NULL;
	char md5sum[64] = {""};
	char* p_md5 = NULL;
	FILE* fid = NULL;
	//return true;
	fid = fopen(filename, "r+");
	if(fid){

        header = calloc(sizeof(FwHeader_t), 1);
        if(NULL == header)
        {
            fclose(fid);
            return false;
        }

		// get the md5 from firmware file
		ret = fread(header, 1, sizeof(FwHeader_t), fid);
		assert(sizeof(FwHeader_t) == ret);
		if(!strcmp(soc, header->soc) || (!strcmp(soc, "N16C")&& !strcmp(header->soc, "N18A"))){
			strcpy(md5sum, &header->md5_sum[32]);
			if(0 == strcmp(md5sum, FIRMWARE_MD5_CHECK_TAG) && strlen(md5sum) == strlen(FIRMWARE_MD5_CHECK_TAG)){
				// has been check
				fclose(fid);
				FwVersion_t cur_version;
				cur_version.major = SWVER_MAJ;
				cur_version.minor = SWVER_MIN;
				cur_version.revision = SWVER_REV;
				sprintf(cur_version.extend, SWVER_EXT);
				if(_firmware_check_version_limit(header->version_limit,cur_version)
					&& _firmware_check_version_limit_ex(header->version, cur_version)){
					*ret_errno = FIRMWARE_UPGRADE_ERROR_NONE;

                    if(NULL != header)
                    {
                        free(header);
                        header = NULL;
                    }
					return true;
				}else{
					goto FALSE_RETURN;
				}
			}else{
				// first time to check
				memset(header->md5_sum, 0, sizeof(header->md5_sum));
				ret = fseek(fid, 0, SEEK_SET);
				assert(0 == ret);
				ret = fwrite(header, 1, sizeof(FwHeader_t), fid);
				assert(sizeof(FwHeader_t) == ret);
				p_md5 = md5sum_file(filename);
				if(0 == strcmp(md5sum, p_md5) && strlen(md5sum) == strlen(p_md5)){
					// passed
					strcpy(&header->md5_sum[32], FIRMWARE_MD5_CHECK_TAG);
					ret = fseek(fid, 0, SEEK_SET);
					assert(0 == ret);
					ret = fwrite(header, 1, sizeof(FwHeader_t), fid);
					assert(sizeof(FwHeader_t) == ret);
					fclose(fid);

                    if(NULL != header)
                    {
                        free(header);
                        header = NULL;
                    }
					return FIRMWARE_import_check1(soc, ret_errno, filename);
				}else{
					fclose(fid);
					printf("md5 mismatch %s/%s\r\n", p_md5, md5sum);
					*ret_errno = FIRMWARE_UPGRADE_ERROR_MISMATCH_MD5;
					goto FALSE_RETURN;
				}
			}
		}
		else{
			fclose(fid);
			printf("soc mismatch %s/%s\r\n", soc, header->soc);
			*ret_errno = FIRMWARE_UPGRADE_ERROR_MISMATCH_SOC;
			goto FALSE_RETURN;
		}
	}

FALSE_RETURN:

    if(NULL != header)
    {
        free(header);
        header = NULL;
    }

	return false;
}


uint32_t FIRMWARE_import_check2(bool oem_force, bool downgrade, char *oemNumber, char *soc, char *ret_errno, void *up_memp, int file_len)
{
	printf("%s start\n", __FUNCTION__);
	int ret = -1;
	FwHeader_t *header = NULL;
	char md5sum[64] = {""};
	char* p_md5 = NULL;

	if((NULL != up_memp) && (NULL == header))
	{
        header = calloc(sizeof(FwHeader_t), 1);
        if(NULL == header)
        {
            return false;
        }

		// get the md5 from firmware file
		memcpy(header, up_memp, sizeof(FwHeader_t));

		if((!strcmp(soc, header->soc))){
			if(oem_force){
				//force to upgrade
				//OEM强制升级开启时，OEM number不相同也可互升
				ret = 0;
			}else{
				if(!strcmp(oemNumber, header->oemNumber)){
					//强制升级关闭时，只能中性升级到中性，OEM升级到OEM
					ret = 0;
				}else{
					ret = -1;
				}
			}
		}else{
			//soc mismatch
			ret = -1;
		}

		if(0 == ret){
			strcpy(md5sum, &header->md5_sum[32]);
			if(0 == strcmp(md5sum, FIRMWARE_MD5_CHECK_TAG) && strlen(md5sum) == strlen(FIRMWARE_MD5_CHECK_TAG)){
			    // has been check
			    FwVersion_t cur_version;
			    cur_version.major = SWVER_MAJ;
			    cur_version.minor = SWVER_MIN;
			    cur_version.revision = SWVER_REV;
			    sprintf(cur_version.extend, SWVER_EXT);
				//强制升级开启可以忽略版本号
			    if(downgrade || (_firmware_check_version_limit(header->version_limit,cur_version)
						&& _firmware_check_version_limit_ex(header->version, cur_version))){
						*ret_errno = FIRMWARE_UPGRADE_ERROR_NONE;

                    if(NULL != header)
                    {
                        free(header);
                        header = NULL;
                    }
			        return true;
			    }else{
					goto FALSE_RETURN;
				}
			}else{
				// first time to check
				memset(header->md5_sum, 0, sizeof(header->md5_sum));
				memcpy(up_memp, header, sizeof(FwHeader_t));
				p_md5 = md5sum_buffer(up_memp, file_len);

				if(0 == strcmp(md5sum, p_md5) && strlen(md5sum) == strlen(p_md5)){
					// passed
					strcpy(&header->md5_sum[32], FIRMWARE_MD5_CHECK_TAG);
					memcpy(up_memp, header, sizeof(FwHeader_t));

                    if(NULL != header)
                    {
                        free(header);
                        header = NULL;
                    }
					return FIRMWARE_import_check2(oem_force, downgrade, oemNumber, soc, ret_errno, up_memp, file_len);
				}else{
					printf("md5 mismatch %s/%s\r\n", p_md5, md5sum);
					*ret_errno = FIRMWARE_UPGRADE_ERROR_MISMATCH_MD5;
					goto FALSE_RETURN;
				}
			}
		}
		else{
		    printf("soc mismatch %s/%s ---%s/%s\r\n", soc, header->soc, oemNumber, header->oemNumber);
			*ret_errno = FIRMWARE_UPGRADE_ERROR_MISMATCH_SOC;
		    goto FALSE_RETURN;
		}
	}

FALSE_RETURN:

    if(NULL != header)
    {
        free(header);
        header = NULL;
    }

	return false;
}


int FIRMWARE_import_init()
{
	if(!_fw_import){
		_fw_import = calloc(sizeof(FwImport_t), 1);
		return 0;
	}
	return -1;
}

void FIRMWARE_import_destroy()
{
	if(_fw_import){
		free(_fw_import);
		_fw_import = NULL;
	}
}

int FIRMWARE_import_release_memery()
{
#if defined(HI3516E_V1)     // 16e浼樺寲鍐呭瓨鍚庯紝custom鏀逛负搴旂敤鍒嗗尯鎸傝浇
    return 0;
#endif
	if(IS_FILE_EXIST("/media/custom/custom.jffs2")){
		int ret = umount2("/media/custom", MNT_DETACH | MNT_FORCE);
		usleep(50*1000);
		unlink("/tmp/custom.fs");
		remove("/tmp/custom.fs");
		usleep(50*1000);
		printf("%s\r\n", __FUNCTION__);
	}

	return 0;

}

int FIRMWARE_import_recover_memery()
{
#if defined(HI3516E_V1)     // 16e浼樺寲鍐呭瓨鍚庯紝custom鏀逛负搴旂敤鍒嗗尯鎸傝浇
    return 0;
#endif
	if(!IS_FILE_EXIST("/media/custom/custom.jffs2")){
		// FIXME:
		char shell_cmd[128] = {""};
#if defined(HI3518E_V2)
		sprintf(shell_cmd, "cp -Rf /dev/mtdblock6 /tmp/custom.fs");
#if !defined(PC_TOOLS)
		NK_SYSTEM(shell_cmd);
#endif
		sleep(1);
		sprintf(shell_cmd, "mount -t squashfs /tmp/custom.fs /media/custom");
#if !defined(PC_TOOLS)
		NK_SYSTEM(shell_cmd);
#endif
#endif
		usleep(50*1000);
		printf("%s:%s\r\n", __FUNCTION__, shell_cmd);
	}

    return 0;

}

