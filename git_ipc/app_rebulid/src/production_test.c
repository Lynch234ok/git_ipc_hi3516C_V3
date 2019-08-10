#include "production_test.h"
#include "inifile.h"
#include "app_debug.h"

#define PRODUCT_TEST_INFO_FILE_PATH "/media/tf/production_test.ini"

static LP_PRODUCT_TEST_INFO product_info = NULL;

static void product_info_dump(LP_PRODUCT_TEST_INFO info)
{
	APP_TRACE("enter to SDcard info getting");
	APP_TRACE("ESSID:%s", info->staEssid);
	APP_TRACE("PASSWORD:%s", info->staPassword);
	APP_TRACE("IP:%s", info->staStaticIp);
	APP_TRACE("netmask:%s", info->staNetmask);
	APP_TRACE("gateway:%s", info->staGateway);
    APP_TRACE("dns:%s", info->staDns);
    APP_TRACE("model:%s", info->model);
    APP_TRACE("audioInput:%d", info->audioInput);
    APP_TRACE("audioOutput:%d", info->audioOutput);
    APP_TRACE("lightControl:%d", info->lightControl);
    APP_TRACE("bulbControl:%d", info->bulbControl);
    APP_TRACE("ptz:%d", info->ptz);
    APP_TRACE("sdCard:%d", info->sdCard);
    APP_TRACE("fisheye:%d", info->fisheye);
    APP_TRACE("pir:%d", info->pir);
    APP_TRACE("pirTrigger:%d", info->pirTrigger);
}

LP_PRODUCT_TEST_INFO PRODUCT_TEST_get_info(LP_PRODUCT_TEST_INFO info)
{
	if(product_info){
		memcpy((void *)info, (void *)product_info, sizeof(ST_PRODUCT_TEST_INFO));
		return product_info;
	}else{
		return NULL;
	}
}

int PRODUCT_TEST_init()
{
	lpINI_PARSER inf = NULL;
	product_info = (LP_PRODUCT_TEST_INFO)calloc(sizeof(ST_PRODUCT_TEST_INFO), 1);
	inf = OpenIniFile(PRODUCT_TEST_INFO_FILE_PATH);
	if(inf){
		if(product_info){
            if(NULL == inf->find_section(inf, "wifi"))
            {
                product_info->isWifi = 0;
            }
            else
            {
                product_info->isWifi = 1;
                inf->read_text(inf, "wifi", "ssid", "", product_info->staEssid, sizeof(product_info->staEssid));
                inf->read_text(inf, "wifi", "psk", "", product_info->staPassword, sizeof(product_info->staPassword));
                inf->read_text(inf, "wifi", "ip", "", product_info->staStaticIp, sizeof(product_info->staStaticIp));

                // mask 可选 8,16,25，分别代表掩码 255.0.0.0，255.255.0.0，255.255.255.0，缺省为 8
                inf->read_text(inf, "wifi", "mask", "", product_info->staMask, sizeof(product_info->staMask));
                if(!strncmp(product_info->staMask, "8", 1))
                {
                    sprintf(product_info->staNetmask, "255.0.0.0");
                }
                else if(!strncmp(product_info->staMask, "16", 2))
                {
                    sprintf(product_info->staNetmask, "255.255.0.0");
                }
                else if(!strncmp(product_info->staMask, "25", 2))
                {
                    sprintf(product_info->staNetmask, "255.255.255.0");
                }
                else
                {
                    sprintf(product_info->staNetmask, "255.255.0.0");
                }
                inf->read_text(inf, "wifi", "gw", "", product_info->staGateway, sizeof(product_info->staGateway));
                inf->read_text(inf, "wifi", "dns", "", product_info->staDns, sizeof(product_info->staDns));
            }

            if(NULL == inf->find_section(inf, "CapabilitySet"))
            {
                product_info->isCapability = 0;
            }
            else
            {
                product_info->isCapability = 1;
                inf->read_text(inf, "CapabilitySet", "model", "", product_info->model, sizeof(product_info->model));
                product_info->audioInput = inf->read_int(inf, "CapabilitySet", "audioInput", -1);
                product_info->audioOutput = inf->read_int(inf, "CapabilitySet", "audioOutput", -1);
                product_info->lightControl = inf->read_int(inf, "CapabilitySet", "lightControl", -1);
                product_info->bulbControl = inf->read_int(inf, "CapabilitySet", "bulbControl", -1);
                product_info->ptz = inf->read_int(inf, "CapabilitySet", "ptz", -1);
                product_info->sdCard = inf->read_int(inf, "CapabilitySet", "sdCard", -1);
                product_info->fisheye = inf->read_int(inf, "CapabilitySet", "fisheye", -1);
            }

			if(NULL == inf->find_section(inf, "4gTest"))
			{
				product_info->is4gTest = 0;
			}
			else
			{
				product_info->is4gTest = 1;
				product_info->enable = inf->read_int(inf, "4gTest", "enable", -1);
				inf->read_text(inf, "4gTest", "pingNetwork", "", product_info->pingNetwork, sizeof(product_info->pingNetwork));
				inf->read_text(inf, "4gTest", "startVoiceFile", "", product_info->startVoiceFile, sizeof(product_info->startVoiceFile));
				inf->read_text(inf, "4gTest", "succeedVoiceFile", "", product_info->succeedVoiceFile, sizeof(product_info->succeedVoiceFile));
				inf->read_text(inf, "4gTest", "failedVoiceFile", "", product_info->failedVoiceFile, sizeof(product_info->failedVoiceFile));
			}

            if(NULL == inf->find_section(inf, "custom"))
            {
                product_info->isCustom = 0;
            }
            else
            {
                product_info->isCustom = 1;
                product_info->pir = inf->read_int(inf, "custom", "pir", -1);
                product_info->pirTrigger = inf->read_int(inf, "custom", "pirTrigger", -1);
            }

			product_info_dump(product_info);
		}
		CloseIniFile(inf);
		inf = NULL;
		return 0;
	}else{
		if(product_info){
			free(product_info);
			product_info = NULL;
		}
		return -1;
	}
}

int PRODUCT_TEST_destroy()
{
	if(product_info){
		free(product_info);
		product_info = NULL;
		return 0;
	}
	return -1;
}

int PRODUCT_TEST_getWifi(LP_PRODUCT_TEST_INFO info)
{
    if((NULL == product_info) || (NULL == info))
    {
        return -1;
    }

    if(0 == product_info->isWifi)
    {
        return -1;
    }

    snprintf(info->staEssid, sizeof(info->staEssid), "%s", product_info->staEssid);
    snprintf(info->staPassword, sizeof(info->staPassword), "%s", product_info->staPassword);
    snprintf(info->staStaticIp, sizeof(info->staStaticIp), "%s", product_info->staStaticIp);
    snprintf(info->staMask, sizeof(info->staMask), "%s", product_info->staMask);
    snprintf(info->staNetmask, sizeof(info->staNetmask), "%s", product_info->staNetmask);
    snprintf(info->staGateway, sizeof(info->staGateway), "%s", product_info->staGateway);
    snprintf(info->staDns, sizeof(info->staDns), "%s", product_info->staDns);

    return 0;

}

int PRODUCT_TEST_getCapability(LP_PRODUCT_TEST_INFO info)
{
    if((NULL == product_info) || (NULL == info))
    {
        return -1;
    }

    if(0 == product_info->isCapability)
    {
        return -1;
    }

    snprintf(info->model, sizeof(info->model), "%s", product_info->model);
    info->audioInput = product_info->audioInput;
    info->audioOutput = product_info->audioOutput;
    info->lightControl = product_info->lightControl;
    info->bulbControl = product_info->bulbControl;
    info->ptz = product_info->ptz;
    info->sdCard = product_info->sdCard;
    info->fisheye = product_info->fisheye;

    return 0;

}

int PRODUCT_TEST_get4G(LP_PRODUCT_TEST_INFO info)
{
	if((NULL == product_info) || (NULL == info))
	{
		return -1;
	}

	if(0 == product_info->is4gTest)
	{
		return -1;
	}

	info->enable = product_info->enable;
	snprintf(info->pingNetwork, sizeof(info->pingNetwork), "%s", product_info->pingNetwork);
	snprintf(info->startVoiceFile, sizeof(info->startVoiceFile), "%s", product_info->startVoiceFile);
	snprintf(info->succeedVoiceFile, sizeof(info->succeedVoiceFile), "%s", product_info->succeedVoiceFile);
	snprintf(info->failedVoiceFile, sizeof(info->failedVoiceFile), "%s", product_info->failedVoiceFile);

	return 0;
}

int PRODUCT_TEST_getCustom(LP_PRODUCT_TEST_INFO info)
{
    if((NULL == product_info) || (NULL == info))
    {
        return -1;
    }

    if(0 == product_info->isCustom)
    {
        return -1;
    }

    info->pir = product_info->pir;
    info->pirTrigger = product_info->pirTrigger;

    return 0;

}

