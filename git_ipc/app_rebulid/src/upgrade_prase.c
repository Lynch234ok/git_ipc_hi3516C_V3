#include "remote_upgrade.h"
#include "remote_upgrade_debug.h"
#include <unistd.h>


int upgrade_prase_detail_url(char *detail_path, char *domain, int *ret_port, char *DevModel, char *SWVersion, char *ODMNum, char *ret_filepath, char *deviceSN)
{
    int release = 1;
    if(access(REMOTE_UPGRADE_TEST_FLAG_FILE, F_OK) != -1) {
        release = 0;
    }

	sprintf(ret_filepath, "/XVR/common/checkCommonUpdate.php?DevMod"
		"el=%s&SWVersion=%s&DeviceSN=%s&ODMNum=%s&FirmwareMagic=%s&Release=%d", 
		DevModel, SWVersion, deviceSN, ODMNum,
		"SlVBTiBJUENBTSBGSVJNV0FSRSBERVNJR05FRCBCWSBMQVc=", release);
	*ret_port = (int)8088;
	return 0;
}

/*static ezxml_t ezxml_child_ex(ezxml_t xml, const char *name, char *attr_name, char *attr_value)
{
	ezxml_t const find_node = ezxml_child(xml, name);
	ezxml_t find_idx_node = NULL;
	int i = 0;
	if(NULL == find_node){
		return NULL;
	}
	while(NULL != (find_idx_node = ezxml_idx(find_node, i++))){
		const char *ezattr = ezxml_attr(find_idx_node, attr_name);
		if(NULL != ezattr && NULL != attr_value && !strcmp(ezattr, attr_value)){
			return find_idx_node;
		}
	}
	return NULL;
}*/

int upgrade_prase_url(char *url, char *ret_domain, int *ret_port, char *ret_filepath)
{
	char *domain_tmp = NULL, *port_tmp, *filepath_tmp = NULL, *url_tmp = NULL;

	if(url){
		url_tmp = strstr(url, "http://");
		if(url_tmp){
			url_tmp += strlen("http://");
			domain_tmp = url_tmp;
			*(strstr(domain_tmp, ":")) = 0;
			//get rom file domain
			if(ret_domain){
				strncpy(ret_domain, domain_tmp, 64);//fix me
			}
			url_tmp += (strlen(domain_tmp) + strlen(":"));
			//get rom file port
			port_tmp = url_tmp;
			*(strstr(port_tmp, "/")) = 0;
			if(ret_port){
				*ret_port = atoi(port_tmp);
			}
			//get rom filepath
			filepath_tmp = url_tmp + strlen(port_tmp) + strlen("/");
			if(ret_filepath){
				snprintf(ret_filepath, 128, "/%s", filepath_tmp);
			}

			return 0;
		}
	}
	return -1;
}

int upgrade_prase_rom_detail(char *detail_file,  char ret_domain[512], int *ret_port, char ret_filepath[512], int *errnum)
{
	int ret = 0, domain_len = 0;
	char *str = NULL, *tmp = NULL, *ptr = NULL;
	str = strstr(detail_file, "New Firmware=");
	if(str){
		str += strlen("New Firmware=");
		if(str){
			ret = atoi(str);
		}
	}
	if(0 == ret){
		RU_TRACE("Firmware is new!\n%s", detail_file);
		*errnum = REMOTE_UPGRADE_ERROR_NEEDNT_TO_UPDATE;
		return -1;
	}

	tmp = strstr(detail_file, "http://");
	if(tmp){
		tmp += strlen("http://");
		str = strstr(tmp, ":");
		ptr = strstr(tmp, "/");
		if(ret_port){
			if(str){
				domain_len = str - tmp;
				str += 1;
				*ret_port = atoi(str);
			}else{
				//use default port 80
				*ret_port = 80;
				domain_len = ptr - tmp;
			}
		}

		if(ret_domain){
			if(domain_len < 512){
				strncpy(ret_domain, tmp, domain_len);
			}
			else{
				RU_TRACE("domain name too loog %d !!!", domain_len);
				return -1;
			}
		}

		if(ptr){
			if(ret_filepath && strlen(ptr) < 512){
				strcpy(ret_filepath, ptr);
				if(NULL != (ptr = strstr(ret_filepath, "rom"))){
					ptr += strlen("rom");
					*ptr = 0;
				}
			}
			else{
				RU_TRACE("ret_filepath too loog %d !!!", domain_len);
				return -1;
			}

			if(ret_domain && ret_port && ret_filepath){
				RU_TRACE("http://%s:%d/%s", ret_domain, *ret_port, ret_filepath);
			}
			return 0;
		}
	}
	
	return -1;
}

int upgrade_prase_resource_detail(char *detail_file,  char *ret_domain, int *ret_port, char *ret_filepath, int *errno, char *resource_version, char *vendor)
{
	/*ezxml_t const xml_node = ezxml_parse_file(detail_file);
	int ret = 0;
	char ini_flash_size[32] = {""};
	if(NULL != xml_node){
		ezxml_t const product_node = ezxml_child(xml_node, "product");
		char *pVersion = ezxml_attr(product_node, "version");
		//check resource version
		if(version_cmp(resource_version, pVersion) >= 0){
			RU_TRACE("Needn't to upgrade resource!%s/%s", resource_version, pVersion);
			*errno = REMOTE_UPGRADE_ERROR_VERSION_LIMIT;
			ezxml_free(xml_node);// Don't forget this!!
			return -1;
		}
		
		lpINI_PARSER inf = NULL;
		inf = OpenIniFile(getenv("FLASHMAP"));
		if(inf){
			inf->read_text(inf, "FIRMWARE", "flash_size", "16M", ini_flash_size, sizeof(ini_flash_size));
			CloseIniFile(inf);
			inf = NULL;
		}
		RU_TRACE("flash_size = %s", ini_flash_size);
			
		ezxml_t const file_node = ezxml_child_ex(product_node, "file", "flash_size", ini_flash_size);

		if(NULL != file_node){
			//get file node info			
			ezxml_t const url_node = ezxml_child(file_node, "url");
			if(url_node){
				char *pUrl = ezxml_txt(url_node);
				char *pVendor = ezxml_attr(url_node, "vendor");
				if(pVendor && !strcmp(vendor, pVendor)){
					ret = upgrade_prase_url(pUrl, ret_domain, ret_port, ret_filepath);
				}else{
					*errno = REMOTE_UPGRADE_ERROR_OEM_RESOURCE_LIMIT;
				}
			}else{
				*errno = REMOTE_UPGRADE_ERROR_XML_PRASE_ERROR;
			}
		}else{
			*errno = REMOTE_UPGRADE_ERROR_XML_PRASE_ERROR;
		}
		
		ezxml_free(xml_node); // Don't forget this!!
		if(0 < ret){
			*errno = REMOTE_UPGRADE_ERROR_XML_PRASE_ERROR;
			return -1;
		}
		return 0;
	}*/
	return -1;
}

