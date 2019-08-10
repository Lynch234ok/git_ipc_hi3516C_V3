
#include "netsdk.h"
#include "netsdk_util.h"
#include "netsdk_private.h"
#include "generic.h"
#include "sdk/sdk_api.h"
#include "app_overlay.h"
#include "ticker.h"
#include "generic.h"
#include "app_debug.h"
#include "base64.h"

#include <arpa/inet.h>
#include "base/ja_process.h"
#include "ifconf.h"
#include "inifile.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// NetSDK general opertaions
/////////////////////////////////////////////////////////////////////////////////////////////////
/*static int network_sync_read_lock()
{
	return NETSDK_private_read_lock(&netsdk->network_sync);
}

static int network_sync_try_read_lock()
{
	return NETSDK_private_try_read_lock(&netsdk->network_sync);
}

static int network_sync_write_lock()
{
	return NETSDK_private_write_lock(&netsdk->network_sync);
}

static int network_sync_try_write_lock()
{
	return NETSDK_private_try_write_lock(&netsdk->network_sync);
}

static int network_sync_unlock()
{
	return NETSDK_private_unlock(&netsdk->network_sync);
}*/

//network
const ST_NSDK_MAP_STR_DEC ipVersion_map[] = {
	{"v4", kNSDK_NETWORK_LAN_IP_VERSION_V4},
	{"v6", kNSDK_NETWORK_LAN_IP_VERSION_V6},
};

const ST_NSDK_MAP_STR_DEC addressType_map[] = {
	{"static", kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC},
	{"dynamic", kNSDK_NETWORK_LAN_ADDRESSINGTYPE_DYNAMIC},
};

const ST_NSDK_MAP_STR_DEC ddnsProvider_map[] = {
	{"DYNDDNS", kNSDK_NETWORK_DDNS_PROVIDER_DYNDDNS},
	{"NOIP", kNSDK_NETWORK_DDNS_PROVIDER_NOIP},
	{"3322", kNSDK_NETWORK_DDNS_PROVIDER_3322},
	{"CHANGEIP", kNSDK_NETWORK_DDNS_PROVIDER_CHANGEIP},
	{"POPDVR", kNSDK_NETWORK_DDNS_PROVIDER_POPDVR},
	{"SKYBEST", kNSDK_NETWORK_DDNS_PROVIDER_SKYBEST},
	{"DVRTOP", kNSDK_NETWORK_DDNS_PROVIDER_DVRTOP},
};

const ST_NSDK_MAP_STR_DEC wireLessMode_map[] = {
	{"none", NSDK_NETWORK_WIRELESS_MODE_NONE},
	{"accessPoint", NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT},
	{"stationMode", NSDK_NETWORK_WIRELESS_MODE_STATIONMODE},
	{"repeater", NSDK_NETWORK_WIRELESS_MODE_REPEATER},
};	

const ST_NSDK_MAP_STR_DEC wireLessApMode_map[] = {
	{"802.11b", NSDK_NETWORK_WIRELESS_APMODE_80211B},
	{"802.11g", NSDK_NETWORK_WIRELESS_APMODE_80211G},
	{"802.11n", NSDK_NETWORK_WIRELESS_APMODE_80211N},
	{"802.11bg", NSDK_NETWORK_WIRELESS_APMODE_80211BG},
	{"802.11bgn", NSDK_NETWORK_WIRELESS_APMODE_80211BGN},
};

const ST_NSDK_MAP_STR_DEC wireLessApMode80211nChannel_map[] = {
	{"Auto", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_AUTO},
	{"1", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_1},
	{"2", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_2},
	{"3", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_3},
	{"4", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_4},
	{"5", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_5},
	{"6", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_6},
	{"7", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_7},
	{"8", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_8},
	{"9", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_9},
	{"10", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_10},
	{"11", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_11},
	{"12", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_12},
	{"13", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_13},
	{"14", NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_14},
};

const ST_NSDK_MAP_STR_DEC wirelessWpaMode_map[] = {
	{"WPA_PSK", NSDK_NETWORK_WIRELESS_WPAMODE_WPA_PSK},
	{"WPA2_PSK", NSDK_NETWORK_WIRELESS_WPAMODE_WPA2_PSK},
};

const ST_NSDK_MAP_STR_DEC repeaterWiredBandWidth_map[] = {
	{"auto", NSDK_NETWORK_REPEAER_WIRED_BANDWIDTH_AUTO},
	{"10M", NSDK_NETWORK_REPEAER_WIRED_BANDWIDTH_10M},
	{"100M", NSDK_NETWORK_REPEAER_WIRED_BANDWIDTH_100M},
	{"1000M", NSDK_NETWORK_REPEAER_WIRED_BANDWIDTH_1000M},
};


const ST_NSDK_MAP_STR_DEC repeaterMode_map[] = {
	{"auto", NSDK_NETWORK_REPEAER_MODE_AUTO},
	{"wired", NSDK_NETWORK_REPEAER_MODE_WIRED},
	{"wireless", NSDK_NETWORK_REPEAER_MODE_WIRELESS},
};

const ST_NSDK_MAP_STR_DEC repeaterWorkMode_map[] = {
	{"auto", NSDK_NETWORK_REPEAER_WORK_MODE_AUTO},
	{"bridge", NSDK_NETWORK_REPEAER_WORK_MODE_BRIDGE},
	{"bond", NSDK_NETWORK_REPEAER_WORK_MODE_BOND},
};

static inline int NETWORK_ENTER_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }

	return pthread_rwlock_wrlock(&netsdk->network_sync);
}

static int NETWORK_LEAVE_CRITICAL()
{
    if(0 == netsdk->lock_sync_enabled)
    {
        return -1;
    }

	return pthread_rwlock_unlock(&netsdk->network_sync);
}

static LP_JSON_OBJECT network_find_interface(LP_JSON_OBJECT interfaces, int id)
{
	int i = 0;
	int const n_interfaces = json_object_array_length(interfaces);
	// one channel
	for(i = 0; i < n_interfaces; ++i){
		LP_JSON_OBJECT j_interface = json_object_array_get_idx(interfaces, i);
		if(json_object_get_int(json_object_object_get(j_interface, "id")) == id){
			return j_interface;
		}
	}
	return NULL;
}

static void netsdk_conf_interface_conflict(void)
{
	char lan[64] = {0}, vlan[64] = {0};
	unsigned int eth0_ip = 0, eth0_1_ip = 0;
	unsigned char netclass = 0;
	struct in_addr addr;

	LP_JSON_OBJECT network = json_object_get(netsdk->network_conf);
	LP_JSON_OBJECT interfaces = NETSDK_json_get_child(network, "networkInterface.networkInterfaceList");

	if(NULL != interfaces)
	{
		LP_JSON_OBJECT if1 = network_find_interface(interfaces, 1);
		LP_JSON_OBJECT if2 = network_find_interface(interfaces, 2);
		if(NULL != if1 && NULL != if2)
		{
			LP_JSON_OBJECT lan1 = NETSDK_json_get_child(if1, "lan");
			LP_JSON_OBJECT lan2 = NETSDK_json_get_child(if2, "lan");
			if(NULL != lan1 && NULL != lan2)
			{
				NETSDK_json_get_string(lan1, "staticIP", lan, sizeof(lan));
				NETSDK_json_get_string(lan2, "staticIP", vlan, sizeof(vlan));
				

				eth0_ip = inet_addr(lan);
				eth0_1_ip = inet_addr(vlan);
				if((eth0_ip & 0xFFFFFF) == (eth0_1_ip & 0xFFFFFF))
				{
					netclass = eth0_ip & 0x0FF;
					if(0 <= netclass && 127 > netclass) {
						netclass = 172;
					}else if(127 <= netclass && 192 > netclass) {
						netclass = 192;
					}else if (192 <= netclass) {
						netclass = 10;
					}else{
						netclass = 193;
					}

					eth0_1_ip = (eth0_1_ip & 0xFFFFFF00) | (netclass & 0xFF);
					memcpy(&addr, &eth0_1_ip, sizeof(unsigned int));		
					memset(vlan, 0, sizeof(vlan));
					sprintf(vlan, "%s", inet_ntoa(addr));
					APP_TRACE("Netsdk Change vlan IP to %s\n", vlan);
					NETSDK_json_set_string2(lan2, "staticIP", vlan);
				}
			}
		}
	}
	json_object_put(network);
}

void NETSDK_conf_network_save()
{
    if(0 == NETWORK_ENTER_CRITICAL())
    {
        APP_TRACE("NetSDK Network Conf Save!!");
        netsdk_conf_interface_conflict();
        NETSDK_conf_save(netsdk->network_conf, "network");
        NETWORK_LEAVE_CRITICAL();
    }
}

void NETSDK_conf_network_save2(int opteration, int delay)
{
	if(netsdk->network_conf_save){
		netsdk->network_conf_save(opteration, delay);
	}else{
		NETSDK_conf_network_save();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/*
/NetSDK/interfaces/
/NetSDK/interfaces/properties
/NetSDK/interfaces/ID/
/NetSDK/interfaces/ID/properties
/NetSDK/interfaces/ID/lan
/NetSDK/interfaces/ID/lan/properties
/NetSDK/interfaces/ID/lan/port/ID
/NetSDK/interfaces/ID/lan/port/ID/properties
/NetSDK/interfaces/ID/pppoe
/NetSDK/interfaces/ID/pppoe/properties
/NetSDK/interfaces/ID/ddns
/NetSDK/interfaces/ID/ddns/properties
/NetSDK/interfaces/ID/wireless
/NetSDK/interfaces/ID/wireless/properties
*/

static void netsdk_remove_gateway(LP_JSON_OBJECT lanJSON)
{
	char gateway[32];
	char cmd_str[128];
	NETSDK_json_get_string(lanJSON, "staticGateway", gateway, sizeof(gateway));
		
	sprintf(cmd_str, "route del default gw %s dev eth0", gateway); 
	NK_SYSTEM(cmd_str);
}

static void network_interface_remove_properties(LP_JSON_OBJECT inf)
{	
	int i = 0, ii = 0;
	LP_JSON_OBJECT interfaces = NETSDK_json_get_child(inf, "networkInterfaceList");
	int const n_interfaces = json_object_array_length(interfaces);
	for(i = 0; i < n_interfaces; ++i){
		LP_JSON_OBJECT j_interface = json_object_array_get_idx(interfaces, i);
		NETSDK_json_remove_properties(j_interface);

		LP_JSON_OBJECT lan = NETSDK_json_get_child(j_interface, "lan");
		if(lan != NULL){
			NETSDK_json_remove_properties(lan);
		}

		LP_JSON_OBJECT upnp = NETSDK_json_get_child(j_interface, "upnp");
		if(upnp != NULL){
			NETSDK_json_remove_properties(upnp);
		}

		LP_JSON_OBJECT pppoe = NETSDK_json_get_child(j_interface, "pppoe");
		if(pppoe != NULL){
			NETSDK_json_remove_properties(pppoe);
		}
		
		LP_JSON_OBJECT ddns = NETSDK_json_get_child(j_interface, "ddns");
		if(ddns != NULL){
			NETSDK_json_remove_properties(ddns);
		}

		LP_JSON_OBJECT wireless = NETSDK_json_get_child(j_interface, "wireless");
		
		if(wireless != NULL){
			LP_JSON_OBJECT staMode = NETSDK_json_get_child(wireless, "stationMode");
			LP_JSON_OBJECT staModeBackup = NETSDK_json_get_child(wireless, "stationModeBackup");
			LP_JSON_OBJECT apMode = NETSDK_json_get_child(wireless, "accessPointMode");
			LP_JSON_OBJECT dhcpServer = NETSDK_json_get_child(wireless, "dhcpServer");
			LP_JSON_OBJECT repeater = NETSDK_json_get_child(wireless, "repeater");
			
			NETSDK_json_remove_properties(wireless);
			if(staMode != NULL){
				NETSDK_json_remove_properties(staMode);
			}
			if(staModeBackup != NULL){
				NETSDK_json_remove_properties(staModeBackup);
			}
			if(apMode != NULL){
				NETSDK_json_remove_properties(apMode);
			}
			if(dhcpServer){
				NETSDK_json_remove_properties(dhcpServer);
			}
			if(repeater){
				NETSDK_json_remove_properties(repeater);
			}
		}
	}
}


static int network_interfaces(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri,
	LP_JSON_OBJECT json_dup, char *content, int content_max)
{
	int i = 0, ret = 0;
	snprintf(content, content_max, "%s", json_object_to_json_string(json_dup));
	ret = kNSDK_INS_RET_CONTENT_READY;

	return ret;
}
/*
static int network_interface(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri,
	int id, bool properties, char *content, int content_max)
{
	int i = 0, ret = 0;
	char *json_text = NULL;
	LP_JSON_OBJECT network = json_object_get(netsdk->network_conf);
	LP_JSON_OBJECT interface_ref = NETSDK_json_get_child(network, "networkInterface");
	LP_JSON_OBJECT interface_dup = NETSDK_json_dup(interface_ref);
	LP_JSON_OBJECT interfaces = NETSDK_json_get_child(interface_dup, "networkInterfaceList");

	if(H_IS_GET(context->request_header->method)){
		if(!properties){
			network_remove_properties(interface_dup);
		}

		if(0 == id){
			snprintf(content, content_max, "%s", json_object_to_json_string(channels));
			ret = kNSDK_INS_RET_CONTENT_READY;
		}else if(id > 0){
			LP_JSON_OBJECT jInterface = network_find_interface(interfaces, id);
			// one channel
			snprintf(content, content_max, "%s", json_object_to_json_string(jInterface));
			ret = kNSDK_INS_RET_CONTENT_READY;
		}
	}else if(H_IS_PUT(context->request_header->method)){
		LP_JSON_OBJECT form = NETSDK_json_parse(context->request_content);
		APP_TRACE("Content : %s", context->request_content);
		if(!form){
			ret = kNSDK_INS_RET_INVALID_DOCUMENT;
		}else{
			LP_JSON_OBJECT interfaces = NETSDK_json_get_child(interface_ref, "networkInterfaceList");
			APP_TRACE(json_object_to_json_string(form));	
			if(0 == id){
				ret = kNSDK_INS_RET_INVALID_DOCUMENT;
			}else if(id > 0){
				LP_JSON_OBJECT jInterface = network_find_interface(interfaces, id);
				if(NULL != jInterface){
					//NETSDK_json_copy_child(form, channel, "id");
					NETSDK_json_copy_child(form, jInterface, "");
					NETSDK_json_copy_child(form, jInterface, "outputVolume");
					//FIX ME   do callback to setting network

					NETSDK_conf_network_save2(eNSDK_CONF_SAVE_REBOOT, 1);
					ret = kNSDK_INS_RET_OK;
					
				}
			}
				
			json_object_put(form);
			form = NULL;
		}

	}

	// release json
	json_object_put(network);
	network = NULL;

	return ret;
}
*/

int NETSDK_conf_limit_netmask(int id, int opteration);
static int network_interface(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri,
	LP_JSON_OBJECT json_ref, LP_JSON_OBJECT json_dup, int id, LP_JSON_OBJECT formJSON, char *content, int content_max)
{
	int i = 0, ii = 0, ret = kNSDK_INS_RET_INVALID_OPERATION;
	char text[128] = {""};
	if(H_IS_GET(context->request_header->method)){
		LP_JSON_OBJECT interfaces = NETSDK_json_get_child(json_dup, "networkInterfaceList");
		if(0 == id){
			snprintf(content, content_max, "%s", json_object_to_json_string(interfaces));
			ret = kNSDK_INS_RET_CONTENT_READY;
		}else if(id > 0){
			LP_JSON_OBJECT jInterface = network_find_interface(interfaces, id);
			if(NULL != jInterface){
				snprintf(content, content_max, "%s", json_object_to_json_string(jInterface));
				ret = kNSDK_INS_RET_CONTENT_READY;
			}
		}		
	}else if(H_IS_PUT(context->request_header->method)){
		if(!context->request_content || 0 == context->request_content_len){
			ret = kNSDK_INS_RET_INVALID_DOCUMENT;
		}else{
			if(!formJSON){
				ret = kNSDK_INS_RET_INVALID_DOCUMENT;
			}else{
				LP_JSON_OBJECT interfaces = NETSDK_json_get_child(json_ref, "networkInterfaceList");
				//APP_TRACE(json_object_to_json_string(formJSON));	
				if(0 == id){
					ret = kNSDK_INS_RET_INVALID_DOCUMENT;
				}else if(id > 0){
					LP_JSON_OBJECT jInterface = network_find_interface(interfaces, id);
					if(NULL != jInterface){
						LP_JSON_OBJECT lanJSON = NETSDK_json_get_child(jInterface, "lan");
						LP_JSON_OBJECT upnpJSON = NETSDK_json_get_child(jInterface, "upnp");
						LP_JSON_OBJECT pppoeJSON = NETSDK_json_get_child(jInterface, "pppoe");
						LP_JSON_OBJECT ddnsJSON = NETSDK_json_get_child(jInterface, "ddns");
						LP_JSON_OBJECT wirelessJSON = NETSDK_json_get_child(jInterface, "wireless");
						
						LP_JSON_OBJECT formLanJSON = NETSDK_json_get_child(formJSON, "lan");
						LP_JSON_OBJECT formUpnpJSON = NETSDK_json_get_child(formJSON, "upnp");
						LP_JSON_OBJECT formPppoeJSON = NETSDK_json_get_child(formJSON, "pppoe");
						LP_JSON_OBJECT formDdnsJSON = NETSDK_json_get_child(formJSON, "ddns");
						LP_JSON_OBJECT formWirelessJSON = NETSDK_json_get_child(formJSON, "wireless");

						
						NETSDK_json_copy_child(formJSON, jInterface, "id");
						NETSDK_json_copy_child(formJSON, jInterface, "interfaceName");

						if(NULL != formLanJSON&& NULL != lanJSON){
							char staticIP[32], staticNetmask[32], staticGateway[32];
							
							NETSDK_json_copy_child(formLanJSON, lanJSON, "ipVersion");
							NETSDK_json_copy_child(formLanJSON, lanJSON, "addressingType");
							NETSDK_json_get_string(formLanJSON, "staticIP", staticIP, sizeof(staticIP));
							NETSDK_json_get_string(formLanJSON, "staticNetmask", staticNetmask, sizeof(staticNetmask));
							NETSDK_json_get_string(formLanJSON, "staticGateway", staticGateway, sizeof(staticGateway));
							if(IS_VALID_IPADDR(staticIP)){
								NETSDK_json_copy_child(formLanJSON, lanJSON, "staticIP");
							}
							if(IS_VALID_NETMASK(staticNetmask)){
								NETSDK_json_copy_child(formLanJSON, lanJSON, "staticNetmask");
							}
							if(MATCH_GATEWAY(staticIP, staticNetmask, staticGateway)){
								//remove the old gateway
								if(id == 1){
									netsdk_remove_gateway(lanJSON);
								}
								NETSDK_json_set_string2(lanJSON, "staticGateway", staticGateway);
							}
						}

						if(NULL != formUpnpJSON && NULL != upnpJSON){
							NETSDK_json_copy_child(formUpnpJSON, upnpJSON, "enabled");
						}

						if(NULL != formPppoeJSON && NULL != pppoeJSON){
							NETSDK_json_copy_child(formPppoeJSON, pppoeJSON, "enabled");
							NETSDK_json_copy_child(formPppoeJSON, pppoeJSON, "pppoeUserName");
							NETSDK_json_copy_child(formPppoeJSON, pppoeJSON, "pppoePassword");
						}

						if(NULL != formDdnsJSON && NULL != ddnsJSON){
							NETSDK_json_copy_child(formDdnsJSON, ddnsJSON, "enabled");
							NETSDK_json_copy_child(formDdnsJSON, ddnsJSON, "ddnsProvider");
							NETSDK_json_copy_child(formDdnsJSON, ddnsJSON, "ddnsUrl");
							NETSDK_json_copy_child(formDdnsJSON, ddnsJSON, "ddnsUserName");
							NETSDK_json_copy_child(formDdnsJSON, ddnsJSON, "ddnsPassword");
						}

						if(NULL != formWirelessJSON && NULL != wirelessJSON){
							LP_JSON_OBJECT staModeJSON = NETSDK_json_get_child(wirelessJSON, "staMode");
							LP_JSON_OBJECT apModeJSON = NETSDK_json_get_child(wirelessJSON, "apMode");
							LP_JSON_OBJECT dhcpServerJSON = NETSDK_json_get_child(wirelessJSON, "dhcpServer");
							LP_JSON_OBJECT rpeaterJSON = NETSDK_json_get_child(wirelessJSON, "repeater");

							LP_JSON_OBJECT formStaModeJSON = NETSDK_json_get_child(formWirelessJSON, "staMode");
							LP_JSON_OBJECT formApModeJSON = NETSDK_json_get_child(formWirelessJSON, "apMode");
							LP_JSON_OBJECT formDhcpServerJSON = NETSDK_json_get_child(formWirelessJSON, "dhcpServer");
							LP_JSON_OBJECT formRepeaterJSON = NETSDK_json_get_child(formWirelessJSON, "repeater");
							
							
							NETSDK_json_copy_child(formWirelessJSON, wirelessJSON, "wirelessMode");
							NETSDK_json_copy_child(formWirelessJSON, wirelessJSON, "repeaterDevId");
							//APP_TRACE(json_object_to_json_string(formStaModeJSON));
							//APP_TRACE(json_object_to_json_string(staModeJSON));
							if(NULL != formStaModeJSON && NULL != staModeJSON){
								//APP_TRACE(json_object_to_json_string(formStaModeJSON));
								//APP_TRACE(json_object_to_json_string(staModeJSON));
								NETSDK_json_copy_child(formStaModeJSON, staModeJSON, "wirelessApBssId");
								NETSDK_json_copy_child(formStaModeJSON, staModeJSON, "wirelessApEssId");
								NETSDK_json_copy_child(formStaModeJSON, staModeJSON, "wirelessApPsk");
								NETSDK_json_copy_child(formStaModeJSON, staModeJSON, "wirelessFixedBpsModeEnabled");
							}

							if(NULL != formApModeJSON && NULL != apModeJSON){
								NETSDK_json_copy_child(formApModeJSON, apModeJSON, "wirelessBssId");
								NETSDK_json_copy_child(formApModeJSON, apModeJSON, "wirelessEssId");
								NETSDK_json_copy_child(formApModeJSON, apModeJSON, "wirelessPsk");
								NETSDK_json_copy_child(formApModeJSON, apModeJSON, "wirelessApMode");
								NETSDK_json_copy_child(formApModeJSON, apModeJSON, "wirelessEssIdBroadcastingEnabled");
								NETSDK_json_copy_child(formApModeJSON, apModeJSON, "wirelessApMode80211nChannel");
								NETSDK_json_copy_child(formApModeJSON, apModeJSON, "wirelessWpaMode");
							}

							if(NULL != formDhcpServerJSON && NULL != dhcpServerJSON){
								NETSDK_json_copy_child(formDhcpServerJSON, dhcpServerJSON, "enabled");
								NETSDK_json_copy_child(formDhcpServerJSON, dhcpServerJSON, "dhcpAutoSettingEnabled");
								NETSDK_json_copy_child(formDhcpServerJSON, dhcpServerJSON, "dhcpIpRange");
								NETSDK_json_copy_child(formDhcpServerJSON, dhcpServerJSON, "dhcpIpNumber");
								NETSDK_json_copy_child(formDhcpServerJSON, dhcpServerJSON, "dhcpIpDns");
								NETSDK_json_copy_child(formDhcpServerJSON, dhcpServerJSON, "dhcpIpGateway");
							}

							if(NULL != formRepeaterJSON && NULL != rpeaterJSON){
								NETSDK_json_copy_child(formRepeaterJSON, rpeaterJSON, "wiredBandWidth");
								NETSDK_json_copy_child(formRepeaterJSON, rpeaterJSON, "wiredMaxConn");
								NETSDK_json_copy_child(formRepeaterJSON, rpeaterJSON, "wirelessMaxConn");
								NETSDK_json_copy_child(formRepeaterJSON, rpeaterJSON, "repeaterMode");
								NETSDK_json_copy_child(formRepeaterJSON, rpeaterJSON, "repeaterWorkMode");
							}
						}
						
						//FIX ME   do callback to setting network
						NETSDK_conf_network_save2(eNSDK_CONF_SAVE_RESTART, 1);
						NETSDK_conf_limit_netmask(id, eNSDK_CONF_SAVE_RESTART);
						ret = kNSDK_INS_RET_OK;
						
					}
				}
			}
		}
	}

	APP_TRACE(content);	
	return ret;
}

static void network_port_remove_properties(LP_JSON_OBJECT inf)
{	
	int i = 0, ii = 0;
	LP_JSON_OBJECT ports = NETSDK_json_get_child(inf, "networkPortList");
	int const n_ports = json_object_array_length(ports);
	for(i = 0; i < n_ports; ++i){
		LP_JSON_OBJECT j_port = json_object_array_get_idx(ports, i);
		NETSDK_json_remove_properties(j_port);
	}
}


static int network_port(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	int id, char *content, int contentMax)
{
	int i = 0, ii = 0, ret = kNSDK_INS_RET_INVALID_OPERATION;
	//char *json_text = NULL;
	LP_JSON_OBJECT network = json_object_get(netsdk->network_conf);
	LP_JSON_OBJECT networkRefJSON = NETSDK_json_get_child(network, "networkPort");
	LP_JSON_OBJECT networkDupJSON = NULL;
	LP_JSON_OBJECT ports = NULL;

	if(HTTP_IS_GET(context)){
		networkDupJSON = NETSDK_json_dup(networkRefJSON);
		ports = NETSDK_json_get_child(networkDupJSON, "networkPortList");
		if(kH_METH_GET == context->request_method && !NSDK_PROPERTIES(subURI)){
		network_port_remove_properties(networkDupJSON);
		}
		if(0 == id){
			snprintf(content, contentMax, "%s", json_object_to_json_string(ports));
			ret = kNSDK_INS_RET_CONTENT_READY;
		}else if(id > 0){
			LP_JSON_OBJECT port = network_find_interface(ports,id);
			if(port != NULL){
				snprintf(content, contentMax, "%s", json_object_to_json_string(port));
				ret = kNSDK_INS_RET_CONTENT_READY;
			}
		}
		// release json
		json_object_put(networkDupJSON);
		networkDupJSON = NULL;
	}
	else if(kH_METH_POST == context->request_method || kH_METH_PUT == context->request_method){
		LP_JSON_OBJECT from = NETSDK_json_parse(context->request_content);
		ports = NETSDK_json_get_child(networkRefJSON, "networkPortList");

		if(NULL != from){
			APP_TRACE(json_object_to_json_string(from));	
			// FIXME: lock or enter critical here
			APP_TRACE("id:%d", id);
			if(0 == id){
				ret = kNSDK_INS_RET_INVALID_OPERATION;
			}else if(id > 0){
				LP_JSON_OBJECT port = NULL;
				port=network_find_interface(ports,id);
				if(port != NULL){
					ret = NETSDK_json_copy_child(from,port,"value");	
					APP_TRACE("set ret:%d", ret);
				}

				NETSDK_conf_network_save2(eNSDK_CONF_SAVE_REBOOT, 1);
				ret = kNSDK_INS_RET_OK;
					// setup input with new configure
					/*if(netsdk->videoInputChannelChanged){
						ST_NSDK_VIN_CH vin_ch;
						if(NETSDK_conf_vin_ch_get(id, &vin_ch)){
							if(0 == netsdk->videoInputChannelChanged(id, &vin_ch)){
								// save to file
								NETSDK_conf_video_save2();
								// response
								ret = kNSDK_INS_RET_OK;
							}else{
								ret = kNSDK_INS_RET_DEVICE_ERROR;
							}
						}
					}else{
						ret = kNSDK_INS_RET_DEVICE_NOT_IMPLEMENT;
					}*/

									
			}
			json_object_put(from);
			from = NULL;
		}else{
			ret = kNSDK_INS_RET_INVALID_DOCUMENT;
		}
	}
	
	json_object_put(network);
	return ret;
}


static int network_interfaceList_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri, char *content, int content_max)
{
	int id = 0, i, ii;
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;
	LP_JSON_OBJECT network = json_object_get(netsdk->network_conf);
	LP_JSON_OBJECT json_ref = NETSDK_json_get_child(network, "networkInterface");
	LP_JSON_OBJECT json_dup = NETSDK_json_dup(json_ref);
	LP_JSON_OBJECT formJSON = NULL;
	
	if(NULL != context->request_content && 0 != context->request_content_len){
		APP_TRACE("request :%p  -  %d",context->request_content ,context->request_content_len);
		formJSON = NETSDK_json_parse(context->request_content);
		APP_TRACE("Form JSON:\r\n%s", json_object_to_json_string(formJSON));
	}
	// check properties necessary when GET method
	
	if(kH_METH_GET == context->request_method && !NSDK_PROPERTIES(sub_uri)){
		network_interface_remove_properties(json_dup);
	}
	APP_TRACE("URI : %s", sub_uri);
	if(prefix = "/INTERFACES", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		// get all channels
		sub_uri += strlen(prefix);
		ret = network_interfaces(context, sub_uri, json_dup, content, content_max);

	}else if(prefix = "/INTERFACE/%d", 1 == sscanf(sub_uri, prefix, &id)){
		if(id > 0){
			ret = sprintf(content, prefix, id);
			sub_uri += ret;
			APP_TRACE("URI : %s", sub_uri);

			LP_JSON_OBJECT interfacesJSON = NULL;
			LP_JSON_OBJECT interfaceJSON = NULL;

			if(HTTP_IS_GET(context)){
				interfacesJSON = NETSDK_json_get_child(json_dup, "networkInterfaceList");
			}else{//else if(HTTP_IS_PUT(context)){
				interfacesJSON = NETSDK_json_get_child(json_ref, "networkInterfaceList");
			}
			interfaceJSON = network_find_interface(interfacesJSON, id);

			if(prefix = "/LAN", 0 == strncmp(sub_uri, prefix, strlen(prefix))){
				LP_JSON_OBJECT lanJSON = NETSDK_json_get_child(interfaceJSON, "lan");
				int port_id = 0;
				LP_JSON_OBJECT portJSON;
				sub_uri += strlen(prefix);
				if(HTTP_IS_GET(context)){					
					snprintf(content, content_max, "%s", json_object_to_json_string(lanJSON));
					ret = kNSDK_INS_RET_CONTENT_READY;
				}else if(HTTP_IS_PUT(context)){
					if(0 == NETWORK_ENTER_CRITICAL()){
						char staticIP[32], staticNetmask[32], staticGateway[32];
						NETSDK_json_copy_child(formJSON, lanJSON, "ipVersion");
						NETSDK_json_copy_child(formJSON, lanJSON, "addressingType");
						NETSDK_json_get_string(formJSON, "staticIP", staticIP, sizeof(staticIP));
						NETSDK_json_get_string(formJSON, "staticNetmask", staticNetmask, sizeof(staticNetmask));
						NETSDK_json_get_string(formJSON, "staticGateway", staticGateway, sizeof(staticGateway));
						if(IS_VALID_IPADDR(staticIP)){
							NETSDK_json_copy_child(formJSON, lanJSON, "staticIP");
						}
						if(IS_VALID_NETMASK(staticNetmask)){
							NETSDK_json_copy_child(formJSON, lanJSON, "staticNetmask");
						}
						if(MATCH_GATEWAY(staticIP, staticNetmask, staticGateway)){
							//remove the old gateway
							if(id == 1){
								netsdk_remove_gateway(lanJSON);
							}
							NETSDK_json_set_string2(lanJSON, "staticGateway", staticGateway);
						}								
						NETWORK_LEAVE_CRITICAL();						
					}
					ret = network_interface(context, sub_uri, json_ref, json_dup, id, lanJSON, content, content_max);
				}
			}else if(prefix = "/UPNP", 0 == strncmp(sub_uri, prefix, strlen(prefix))){
				if(HTTP_IS_GET(context)){	
					LP_JSON_OBJECT upnpJSON = NETSDK_json_get_child(interfaceJSON, "upnp");
					snprintf(content, content_max, "%s", json_object_to_json_string(upnpJSON));
					ret = kNSDK_INS_RET_CONTENT_READY;
				}else if(HTTP_IS_PUT(context)){
					LP_JSON_OBJECT upnpJSON = NETSDK_json_get_child(interfaceJSON, "upnp");
					if(0 == NETWORK_ENTER_CRITICAL()){
						NETSDK_json_copy_child(formJSON, upnpJSON, "enabled");
						NETWORK_LEAVE_CRITICAL();
						ret = network_interface(context, sub_uri, json_ref, json_dup, id, interfaceJSON, content, content_max);
					}
				}
			}else if(prefix = "/PPPOE", 0 == strncmp(sub_uri, prefix, strlen(prefix))){
				if(HTTP_IS_GET(context)){	
					LP_JSON_OBJECT pppoeJSON = NETSDK_json_get_child(interfaceJSON, "pppoe");
					snprintf(content, content_max, "%s", json_object_to_json_string(pppoeJSON));
					ret = kNSDK_INS_RET_CONTENT_READY;
				}else if(HTTP_IS_PUT(context)){
					LP_JSON_OBJECT pppoeJSON = NETSDK_json_get_child(interfaceJSON, "pppoe");
					if(0 == NETWORK_ENTER_CRITICAL()){
						NETSDK_json_copy_child(formJSON, pppoeJSON, "enabled");
						NETSDK_json_copy_child(formJSON, pppoeJSON, "pppoeUserName");
						NETSDK_json_copy_child(formJSON, pppoeJSON, "pppoePassword");
						NETWORK_LEAVE_CRITICAL();
						ret = network_interface(context, sub_uri, json_ref, json_dup, id, interfaceJSON, content, content_max);
					}
				}
			}else if(prefix = "/DDNS", 0 == strncmp(sub_uri, prefix, strlen(prefix))){
				if(HTTP_IS_GET(context)){	
					LP_JSON_OBJECT ddnsJSON = NETSDK_json_get_child(interfaceJSON, "ddns");
					snprintf(content, content_max, "%s", json_object_to_json_string(ddnsJSON));
					ret = kNSDK_INS_RET_CONTENT_READY;
				}else if(HTTP_IS_PUT(context)){
					LP_JSON_OBJECT ddnsJSON = NETSDK_json_get_child(interfaceJSON, "ddns");
					if(0 == NETWORK_ENTER_CRITICAL()){
						NETSDK_json_copy_child(formJSON, ddnsJSON, "enabled");
						NETSDK_json_copy_child(formJSON, ddnsJSON, "ddnsProvider");
						NETSDK_json_copy_child(formJSON, ddnsJSON, "ddnsUrl");
						NETSDK_json_copy_child(formJSON, ddnsJSON, "ddnsUserName");
						NETSDK_json_copy_child(formJSON, ddnsJSON, "ddnsPassword");
						NETWORK_LEAVE_CRITICAL();
						ret = network_interface(context, sub_uri, json_ref, json_dup, id, interfaceJSON, content, content_max);
					}
				}
			}else if((prefix = "/WIRELESS_B64EN", 0 == strncmp(sub_uri, prefix, strlen(prefix)))
				|| (prefix = "/WIRELESS", 0 == strncmp(sub_uri, prefix, strlen(prefix)))){
				if(HTTP_IS_GET(context)){	
					LP_JSON_OBJECT wirelessJSON = NETSDK_json_get_child(interfaceJSON, "wireless");
					if((NULL != wirelessJSON)
					&& (0 != strncmp(sub_uri, "/WIRELESS_B64EN", strlen("/WIRELESS_B64EN")))) { //Non-Base64 Translation
						LP_JSON_OBJECT staModeJSON = NETSDK_json_get_child(wirelessJSON, "stationMode");
						LP_JSON_OBJECT apModeJSON = NETSDK_json_get_child(wirelessJSON, "accessPointMode");
						LP_JSON_OBJECT staModeBackupJSON = NETSDK_json_get_child(wirelessJSON, "stationModeBackup");
						if(NULL != staModeJSON) {
							char STA_Base64_Buffer[256] = {0};
							char STA_Decode_Buffer[256] = {0};
							NETSDK_json_get_string(staModeJSON, "wirelessApEssId",
								STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
							base64_decode(STA_Base64_Buffer, STA_Decode_Buffer, strlen(STA_Base64_Buffer));
							NETSDK_json_set_string2(staModeJSON, "wirelessApEssId", STA_Decode_Buffer);

							NETSDK_json_get_string(staModeJSON, "wirelessApPsk",
								STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
							base64_decode(STA_Base64_Buffer, STA_Decode_Buffer, strlen(STA_Base64_Buffer));
							NETSDK_json_set_string2(staModeJSON, "wirelessApPsk", STA_Decode_Buffer);
						}
						if(NULL != staModeBackupJSON && NULL != NETSDK_json_get_child(staModeBackupJSON, "SettingB64En")) {
							char STA_Base64_Buffer[256] = {0};
							char STA_Decode_Buffer[256] = {0};
							NETSDK_json_get_string(staModeBackupJSON, "wirelessApEssId",
								STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
							base64_decode(STA_Base64_Buffer, STA_Decode_Buffer, strlen(STA_Base64_Buffer));
							NETSDK_json_set_string2(staModeBackupJSON, "wirelessApEssId", STA_Decode_Buffer);

							NETSDK_json_get_string(staModeBackupJSON, "wirelessApPsk",
								STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
							base64_decode(STA_Base64_Buffer, STA_Decode_Buffer, strlen(STA_Base64_Buffer));
							NETSDK_json_set_string2(staModeBackupJSON, "wirelessApPsk", STA_Decode_Buffer);
						}
						//ap mode
						if(NULL != apModeJSON) {
							char STA_Base64_Buffer[256] = {0};
							char STA_Decode_Buffer[256] = {0};
							NETSDK_json_get_string(apModeJSON, "wirelessEssId",
								STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
							base64_decode(STA_Base64_Buffer, STA_Decode_Buffer, strlen(STA_Base64_Buffer));
							NETSDK_json_set_string2(apModeJSON, "wirelessEssId", STA_Decode_Buffer);

							NETSDK_json_get_string(apModeJSON, "wirelessPsk",
								STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
							base64_decode(STA_Base64_Buffer, STA_Decode_Buffer, strlen(STA_Base64_Buffer));
							NETSDK_json_set_string2(apModeJSON, "wirelessPsk", STA_Decode_Buffer);
						}
					}
					snprintf(content, content_max, "%s", json_object_to_json_string(wirelessJSON));
					ret = kNSDK_INS_RET_CONTENT_READY;
				}else if(HTTP_IS_PUT(context)){
					LP_JSON_OBJECT wirelessJSON = NETSDK_json_get_child(interfaceJSON, "wireless");
					LP_JSON_OBJECT staModeJSON = NETSDK_json_get_child(wirelessJSON, "stationMode");
					LP_JSON_OBJECT staModeBackupJSON = NETSDK_json_get_child(wirelessJSON, "stationModeBackup");
					LP_JSON_OBJECT apModeJSON = NETSDK_json_get_child(wirelessJSON, "accessPointMode");
					LP_JSON_OBJECT dhcpServerJSON = NETSDK_json_get_child(wirelessJSON, "dhcpServer");
					LP_JSON_OBJECT rpeaterJSON = NETSDK_json_get_child(wirelessJSON, "repeater");

					LP_JSON_OBJECT formstaModeJSON = NETSDK_json_get_child(formJSON, "stationMode");
					LP_JSON_OBJECT formstaModeBackupJSON = NETSDK_json_get_child(formJSON, "stationModeBackup");
					LP_JSON_OBJECT formapModeJSON = NETSDK_json_get_child(formJSON, "accessPointMode");
					LP_JSON_OBJECT formdhcpServerJSON = NETSDK_json_get_child(formJSON, "dhcpServer");
					LP_JSON_OBJECT formRepeaterJSON = NETSDK_json_get_child(formJSON, "repeater");

					if(0 == NETWORK_ENTER_CRITICAL()){
						NETSDK_json_copy_child(formJSON, wirelessJSON, "repeaterDevId");
						NETSDK_json_copy_child(formJSON, wirelessJSON, "wirelessMode");
						//APP_TRACE(json_object_to_json_string(formJSON));	
						//APP_TRACE(json_object_to_json_string(staModeJSON));
						NETSDK_json_copy_child(formstaModeJSON, staModeJSON, "wirelessApBssId");
						NETSDK_json_copy_child(formstaModeJSON, staModeJSON, "wirelessApEssId");
						NETSDK_json_copy_child(formstaModeJSON, staModeJSON, "wirelessApPsk");
						NETSDK_json_copy_child(formstaModeBackupJSON, staModeBackupJSON, "wirelessApBssId");
						NETSDK_json_copy_child(formstaModeBackupJSON, staModeBackupJSON, "wirelessApEssId");
						NETSDK_json_copy_child(formstaModeBackupJSON, staModeBackupJSON, "wirelessApPsk");
						if((NULL != wirelessJSON)
						&& (0 != strncmp(sub_uri, "/WIRELESS_B64EN", strlen("/WIRELESS_B64EN")))) { //Non-Base64 Translation
							if(NULL != staModeJSON) {
								char STA_Base64_Buffer[256] = {0};
								char STA_Encode_Buffer[256] = {0};
								NETSDK_json_get_string(staModeJSON, "wirelessApEssId",
									STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
								base64_encode(STA_Base64_Buffer, STA_Encode_Buffer, strlen(STA_Base64_Buffer));
								NETSDK_json_set_string2(staModeJSON, "wirelessApEssId", STA_Encode_Buffer);

								NETSDK_json_get_string(staModeJSON, "wirelessApPsk",
									STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
								base64_encode(STA_Base64_Buffer, STA_Encode_Buffer, strlen(STA_Base64_Buffer));
								NETSDK_json_set_string2(staModeJSON, "wirelessApPsk", STA_Encode_Buffer);
							}
							if(NULL != staModeBackupJSON) {
								char STA_Base64_Buffer[256] = {0};
								char STA_Encode_Buffer[256] = {0};
								NETSDK_json_get_string(staModeBackupJSON, "wirelessApEssId",
									STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
								base64_encode(STA_Base64_Buffer, STA_Encode_Buffer, strlen(STA_Base64_Buffer));
								NETSDK_json_set_string2(staModeBackupJSON, "wirelessApEssId", STA_Encode_Buffer);

								NETSDK_json_get_string(staModeBackupJSON, "wirelessApPsk",
									STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
								base64_encode(STA_Base64_Buffer, STA_Encode_Buffer, strlen(STA_Base64_Buffer));
								NETSDK_json_set_string2(staModeBackupJSON, "wirelessApPsk", STA_Encode_Buffer);
							}
						}
						NETSDK_json_copy_child(formstaModeJSON, staModeJSON, "wirelessFixedBpsModeEnabled");
						NETSDK_json_copy_child(formstaModeBackupJSON, staModeBackupJSON, "wirelessFixedBpsModeEnabled");

						NETSDK_json_copy_child(formapModeJSON, apModeJSON, "wirelessBssId");
						NETSDK_json_copy_child(formapModeJSON, apModeJSON, "wirelessEssId");
						NETSDK_json_copy_child(formapModeJSON, apModeJSON, "wirelessPsk");
						if((NULL != wirelessJSON)
						&& (0 != strncmp(sub_uri, "/WIRELESS_B64EN", strlen("/WIRELESS_B64EN")))) { //Non-Base64 Translation
							if(NULL != apModeJSON) {
								char STA_Base64_Buffer[256] = {0};
								char STA_Encode_Buffer[256] = {0};
								NETSDK_json_get_string(apModeJSON, "wirelessEssId",
									STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
								base64_encode(STA_Base64_Buffer, STA_Encode_Buffer, strlen(STA_Base64_Buffer));
								NETSDK_json_set_string2(apModeJSON, "wirelessEssId", STA_Encode_Buffer);

								NETSDK_json_get_string(apModeJSON, "wirelessPsk",
									STA_Base64_Buffer, sizeof(STA_Base64_Buffer));
								base64_encode(STA_Base64_Buffer, STA_Encode_Buffer, strlen(STA_Base64_Buffer));
								NETSDK_json_set_string2(apModeJSON, "wirelessPsk", STA_Encode_Buffer);
							}
						}
						NETSDK_json_copy_child(formapModeJSON, apModeJSON, "wirelessApMode");
						NETSDK_json_copy_child(formapModeJSON, apModeJSON, "wirelessEssIdBroadcastingEnabled");
						NETSDK_json_copy_child(formapModeJSON, apModeJSON, "wirelessApMode80211nChannel");
						NETSDK_json_copy_child(formapModeJSON, apModeJSON, "wirelessWpaMode");
						NETSDK_json_copy_child(formdhcpServerJSON, dhcpServerJSON, "enabled");
						NETSDK_json_copy_child(formdhcpServerJSON, dhcpServerJSON, "dhcpAutoSettingEnabled");
						NETSDK_json_copy_child(formdhcpServerJSON, dhcpServerJSON, "dhcpIpRange");
						NETSDK_json_copy_child(formdhcpServerJSON, dhcpServerJSON, "dhcpIpNumber");
						NETSDK_json_copy_child(formdhcpServerJSON, dhcpServerJSON, "dhcpIpDns");
						NETSDK_json_copy_child(formdhcpServerJSON, dhcpServerJSON, "dhcpIpGateway");

                        if(NULL != formRepeaterJSON && NULL != rpeaterJSON)
                        {
                            NETSDK_json_copy_child(formRepeaterJSON, rpeaterJSON, "wiredBandWidth");
                            NETSDK_json_copy_child(formRepeaterJSON, rpeaterJSON, "wiredMaxConn");
                            NETSDK_json_copy_child(formRepeaterJSON, rpeaterJSON, "wirelessMaxConn");
                            NETSDK_json_copy_child(formRepeaterJSON, rpeaterJSON, "repeaterMode");
                            NETSDK_json_copy_child(formRepeaterJSON, rpeaterJSON, "repeaterWorkMode");
                        }

						NETWORK_LEAVE_CRITICAL();
						//APP_TRACE(json_object_to_json_string(formJSON));	
						//APP_TRACE(json_object_to_json_string(staModeJSON));
						//APP_TRACE(json_object_to_json_string(interfaceJSON));	
						ret = network_interface(context, sub_uri, json_ref, json_dup, id, interfaceJSON, content, content_max);
					}
				}
			}else{
				ret = network_interface(context, sub_uri, json_ref, json_dup, id, formJSON, content, content_max);
			}
		}
	}


	// put the both json
	if(NULL != formJSON){
		json_object_put(formJSON);
	}
	json_object_put(json_dup);
	json_object_put(network);
	return ret;
}

static int network_portList_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri, char *content, int content_max)
{
	int id = 0, i, ii;
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;

	APP_TRACE("URI : %s", sub_uri);
	if(prefix = "/PORTS", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		ret = network_port(context, sub_uri, 0, content, content_max);
	}else if(prefix = "/PORT/%d", 1 == sscanf(sub_uri, prefix, &id)){
		if(id > 0){
			ret = network_port(context, sub_uri, id, content, content_max);
		}
	}
	return ret;
}


static int network_dns(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	char *content, int contentMax)
{
	int i = 0, ii = 0, ret = kNSDK_INS_RET_INVALID_OPERATION;
	//char *json_text = NULL;
	LP_JSON_OBJECT network = json_object_get(netsdk->network_conf);
	LP_JSON_OBJECT dnsRefJSON = NETSDK_json_get_child(network, "networkDNS");
	LP_JSON_OBJECT dnsDupJSON = NULL;

	if(HTTP_IS_GET(context)){
		dnsDupJSON = NETSDK_json_dup(dnsRefJSON);
		/*if(kH_METH_GET == context->request_method && !NSDK_PROPERTIES(subURI)){
			network_remove_properties(dnsDupJSON);
		}*/
		snprintf(content, contentMax, "%s", json_object_to_json_string(dnsDupJSON));
		ret = kNSDK_INS_RET_CONTENT_READY;
		// release json
		json_object_put(dnsDupJSON);
		dnsDupJSON = NULL;
	}
	else if(kH_METH_POST == context->request_method || kH_METH_PUT == context->request_method){
		LP_JSON_OBJECT from = NETSDK_json_parse(context->request_content);
		if(NULL != from){
			APP_TRACE(json_object_to_json_string(from));	
			char preferredDns[32] = {0}, staticAlternateDns[32] = {0};
			NETSDK_json_get_string(from, "preferredDns", preferredDns, sizeof(preferredDns));
			NETSDK_json_get_string(from, "staticAlternateDns", staticAlternateDns, sizeof(staticAlternateDns));

			if(IS_VALID_IPADDR(preferredDns)){
				NETSDK_json_copy_child(from,dnsRefJSON,"preferredDns");
			}
			if(IS_VALID_IPADDR(staticAlternateDns)){
				NETSDK_json_copy_child(from,dnsRefJSON,"staticAlternateDns");
			}
			
			NETSDK_conf_network_save2(eNSDK_CONF_SAVE_REBOOT, 1);
			ret = kNSDK_INS_RET_OK;

			json_object_put(from);
			from = NULL;
		}else{
			ret = kNSDK_INS_RET_INVALID_DOCUMENT;
		}
	}
	
	json_object_put(network);
	return ret;
}



static int network_esee(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	char *content, int contentMax)
{
	int i = 0, ii = 0, ret = kNSDK_INS_RET_INVALID_OPERATION;
	//char *json_text = NULL;
	LP_JSON_OBJECT network = json_object_get(netsdk->network_conf);
	LP_JSON_OBJECT eseeRefJSON = NETSDK_json_get_child(network, "esee");
	LP_JSON_OBJECT eseeDupJSON = NULL;

	if(HTTP_IS_GET(context)){
		eseeDupJSON = NETSDK_json_dup(eseeRefJSON);
		/*if(kH_METH_GET == context->request_method && !NSDK_PROPERTIES(subURI)){
			network_remove_properties(eseeDupJSON);
		}*/
		snprintf(content, contentMax, "%s", json_object_to_json_string(eseeDupJSON));
		ret = kNSDK_INS_RET_CONTENT_READY;
		// release json
		json_object_put(eseeDupJSON);
		eseeDupJSON = NULL;
	}
	else if(kH_METH_POST == context->request_method || kH_METH_PUT == context->request_method){
		LP_JSON_OBJECT from = NETSDK_json_parse(context->request_content);
		if(NULL != from){
			APP_TRACE(json_object_to_json_string(from));	
	
			NETSDK_json_copy_child(from,eseeRefJSON,"enabled");
			
			NETSDK_conf_network_save2(eNSDK_CONF_SAVE_REBOOT, 1);
			ret = kNSDK_INS_RET_OK;

			json_object_put(from);
			from = NULL;
		}else{
			ret = kNSDK_INS_RET_INVALID_DOCUMENT;
		}
	}

	json_object_put(network);
	return ret;
}


static int network_wireless(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI,
	char *content, int contentMax)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	const char *prefix = NULL;
	APP_TRACE("%s", subURI);
	ST_NSDK_NETWORK_WIFI_STATUS_ATTR status_attr;
	if(prefix = "/STATUS", 0 == strncmp(subURI, prefix, strlen(prefix))){
		if(netsdk->networkWirelessStatus){
			if(HTTP_IS_GET(context)){
				netsdk->networkWirelessStatus(eNSDK_NETWORK_WIFI_STATUS_TYPE_MODEL_EXIST, &status_attr);
				if(eNSDK_NETWORK_WIFI_STATUS_TYPE_MODEL_EXIST == status_attr.statusType && status_attr.modelExist){
					snprintf(content,contentMax, "%s", "true");
				}else{
					snprintf(content,contentMax, "%s", "false");
				}
				ret = kNSDK_INS_RET_CONTENT_READY;				
			}else{
				ret = kNSDK_INS_RET_INVALID_OPERATION;
			}
		}else{
			ret = kNSDK_INS_RET_UNKNOWN_ERROR;
		}
	}else if(prefix = "/STATIONSIGNAL", 0 == strncmp(subURI, prefix, strlen(prefix))){
		if(netsdk->networkWirelessStatus){
			if(HTTP_IS_GET(context)){
				netsdk->networkWirelessStatus(eNSDK_NETWORK_WIFI_STATUS_TYPE_STATION_SIGNAL, &status_attr);			
				snprintf(content,contentMax, "-%d", status_attr.stationSignal);
				ret = kNSDK_INS_RET_CONTENT_READY;				
			}else{
				ret = kNSDK_INS_RET_INVALID_OPERATION;
			}
		}else{
			ret = kNSDK_INS_RET_UNKNOWN_ERROR;
		}
	}else{
	
	}
	return ret;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
/////////////////////////////////////////////////////////////////////////////////////////////////////

void NETSDK_conf_interface_dump(LP_NSDK_NETWORK_INTERFACE n_interface)
{
	printf("id:%d\r\n"
		"interfaceName:%s\r\n"
		"lan.ipversion:%d\r\n"
		"lan.addressingType:%d\r\n"
		"lan.staticIP:%s\r\n"
		"lan.staticNetmask:%s\r\n"
		"lan.staticGateway:%s\r\n\r\n", 
		n_interface->id,
		n_interface->interfaceName,
		n_interface->lan.ipVersion,
		n_interface->lan.addressingType,
		n_interface->lan.staticIP,
		n_interface->lan.staticNetmask,
		n_interface->lan.staticGateway
		);

	printf("upnp.enabled:%d\r\n"
		"pppoe.enabled:%d\r\n"
		"pppoe.username:%s\r\n"
		"pppoe.password:%s\r\n"
		"ddns.enabled:%d\r\n"
		"ddns.provider:%d\r\n"
		"ddns.url:%s\r\n"
		"ddns.usrname:%s\r\n"
		"ddns.password:%s\r\n\r\n",
		n_interface->upnp.enabled,
		n_interface->pppoe.enabled,
		n_interface->pppoe.pppoeUserName,
		n_interface->pppoe.pppoePassword,
		n_interface->ddns.enabled,
		n_interface->ddns.ddnsProvider,
		n_interface->ddns.ddnsUrl,
		n_interface->ddns.ddnsUserName,
		n_interface->ddns.ddnsPassword
		);

	printf("wirelessMode:%d\r\n"
		"staMode:\r\n"
		"wirelessStamode:%s\r\n"
		"wirelessApBssId:%s\r\n"
		"wirelessApEssId:%s\r\n"
		"wirelessApPsk:%s\r\n"
		"staModeBackup:\r\n"
		"wirelessStamode:%s\r\n"
		"wirelessApBssId:%s\r\n"
		"wirelessApEssId:%s\r\n"
		"wirelessApPsk:%s\r\n"
		"apMode:\r\n"
		"wirelessBssId:%s\r\n"
		"wirelessEssId:%s\r\n"
		"wirelessPsk:%s\r\n"
		"wirelessApMode:%d\r\n"
		"wirelessApMode80211nChannel:%d\r\n"
		"wirelessEssIdBroadcastingEnabled:%d\r\n"
		"wirelessWpaMode:%d\r\n"
		"dhcpServer.enabled:%d\r\n"
		"dhcpServer.dhcpAutoSettingEnabled:%d\r\n"
		"dhcpServer.dhcpIpRange:%s\r\n"
		"dhcpServer.dhcpIpNumber:%s\r\n"
		"dhcpServer.dhcpIpDns:%s\r\n"
		"dhcpServer.dhcpIpGateway:%s\r\n\r\n",
		n_interface->wireless.wirelessMode,
		n_interface->wireless.wirelessStaMode.wirelessStaMode,
		n_interface->wireless.wirelessStaMode.wirelessApBssId,
		n_interface->wireless.wirelessStaMode.wirelessApEssId,
		n_interface->wireless.wirelessStaMode.wirelessApPsk,
		n_interface->wireless.wirelessStaModeBackup.wirelessStaMode,
		n_interface->wireless.wirelessStaModeBackup.wirelessApBssId,
		n_interface->wireless.wirelessStaModeBackup.wirelessApEssId,
		n_interface->wireless.wirelessStaModeBackup.wirelessApPsk,
		n_interface->wireless.wirelessApMode.wirelessBssId,
		n_interface->wireless.wirelessApMode.wirelessEssId,
		n_interface->wireless.wirelessApMode.wirelessPsk,
		n_interface->wireless.wirelessApMode.wireLessApMode,
		n_interface->wireless.wirelessApMode.wirelessApMode80211nChannel,
		n_interface->wireless.wirelessApMode.wireLessEssIdBroadcastingEnabled,
		n_interface->wireless.wirelessApMode.wirelessWpaMode,
		n_interface->wireless.dhcpServer.enabled,
		n_interface->wireless.dhcpServer.dhcpAutoSettingEnabled,
		n_interface->wireless.dhcpServer.dhcpIpRange,
		n_interface->wireless.dhcpServer.dhcpIpNumber,
		n_interface->wireless.dhcpServer.dhcpIpDns,
		n_interface->wireless.dhcpServer.dhcpIpGateway
	);

	printf("networkDNS.perferred:%s\r\n"
		"networkDNS.alternate:%s\r\n\r\n"
		"esee.enabled:%d\r\n\r\n",
		n_interface->dns.staticPreferredDns, 
		n_interface->dns.staticAlternateDns,
		n_interface->esee.enabled
	);

	
}

static LP_NSDK_NETWORK_INTERFACE netsdk_conf_interface(bool set_flag, int id, LP_NSDK_NETWORK_INTERFACE n_interface, int opteration, int delay, int enableb64)
{
	int i = 0;
	char text[128] = {""};
	char *str = NULL;
	bool force_reboot = false;

	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		return NULL;
	}


	if(n_interface){
		if(0 == NETWORK_ENTER_CRITICAL()){
			LP_JSON_OBJECT network = json_object_get(netsdk->network_conf);
			LP_JSON_OBJECT interfaces = NETSDK_json_get_child(network, "networkInterface.networkInterfaceList");
			LP_JSON_OBJECT j_interface = network_find_interface(interfaces, id);
			if(set_flag){//set
				if(NULL != j_interface){
					LP_JSON_OBJECT lanJSON = NETSDK_json_get_child(j_interface, "lan");
					LP_JSON_OBJECT upnpJSON = NETSDK_json_get_child(j_interface, "upnp");
					LP_JSON_OBJECT pppoeJSON = NETSDK_json_get_child(j_interface, "pppoe");
					LP_JSON_OBJECT ddnsJSON = NETSDK_json_get_child(j_interface, "ddns");

					str = NETSDK_MAP_DEC2STR(ipVersion_map,n_interface->lan.ipVersion,"v4");
					NETSDK_json_set_string2(lanJSON, "ipVersion", str);
					str = NETSDK_MAP_DEC2STR(addressType_map,n_interface->lan.addressingType,"static");
					NETSDK_json_set_string2(lanJSON, "addressingType", str);
					if(IS_VALID_IPADDR(n_interface->lan.staticIP)){
						NETSDK_json_set_string2(lanJSON, "staticIP", n_interface->lan.staticIP);
					}
					if(IS_VALID_NETMASK(n_interface->lan.staticNetmask)){
						NETSDK_json_set_string2(lanJSON, "staticNetmask", n_interface->lan.staticNetmask);
					}
					if(MATCH_GATEWAY(n_interface->lan.staticIP, n_interface->lan.staticNetmask, n_interface->lan.staticGateway)){
						//remove the old gateway
						if(opteration != eNSDK_CONF_SAVE_JUST_SAVE){
							if(id == 1){
								netsdk_remove_gateway(lanJSON);
							}
						}
						NETSDK_json_set_string2(lanJSON, "staticGateway", n_interface->lan.staticGateway);
					}
					NETSDK_json_set_boolean2(upnpJSON, "enabled", n_interface->upnp.enabled);
					NETSDK_json_set_boolean2(pppoeJSON, "enabled", n_interface->pppoe.enabled);
					NETSDK_json_set_string2(pppoeJSON, "pppoeUserName", n_interface->pppoe.pppoeUserName);
					NETSDK_json_set_string2(pppoeJSON, "pppoePassword", n_interface->pppoe.pppoePassword);
					NETSDK_json_set_boolean2(ddnsJSON, "enabled", n_interface->ddns.enabled);
					str = NETSDK_MAP_DEC2STR(ddnsProvider_map, n_interface->ddns.ddnsProvider, "DYNDDNS");
					NETSDK_json_set_string2(ddnsJSON, "ddnsProvider", str);
					NETSDK_json_set_string2(ddnsJSON, "ddnsUrl", n_interface->ddns.ddnsUrl);
					NETSDK_json_set_string2(ddnsJSON, "ddnsUserName", n_interface->ddns.ddnsUserName);
					NETSDK_json_set_string2(ddnsJSON, "ddnsPassword", n_interface->ddns.ddnsPassword);
					if(id == 4){
						LP_JSON_OBJECT wirelessJSON = NETSDK_json_get_child(j_interface, "wireless");
						if(wirelessJSON != NULL){
							LP_JSON_OBJECT staModeJSON = NETSDK_json_get_child(wirelessJSON, "stationMode");
							LP_JSON_OBJECT staModeBackupJSON = NETSDK_json_get_child(wirelessJSON, "stationModeBackup");
							LP_JSON_OBJECT apModeJSON = NETSDK_json_get_child(wirelessJSON, "accessPointMode");
							LP_JSON_OBJECT dhcpServerJSON = NETSDK_json_get_child(wirelessJSON, "dhcpServer");
							LP_JSON_OBJECT repeaterJSON = NETSDK_json_get_child(wirelessJSON, "repeater");
							NETSDK_json_set_string2(wirelessJSON, "repeaterDevId", n_interface->wireless.repeaterDevId);

							str = NETSDK_MAP_DEC2STR(wireLessMode_map, n_interface->wireless.wirelessMode, "accessPoint");
							NETSDK_json_set_string2(wirelessJSON, "wirelessMode", str);
							NETSDK_json_set_string2(staModeJSON, "wirelessApBssId", n_interface->wireless.wirelessStaMode.wirelessApBssId);
							if(enableb64) { //Take Place By Base64 Encoded Result
								char ApEssIdBuf[256] = {0};
								char ApPskBuf[256] = {0};
								base64_encode(n_interface->wireless.wirelessStaMode.wirelessApEssId,
									ApEssIdBuf,
									strlen(n_interface->wireless.wirelessStaMode.wirelessApEssId));
								base64_encode(n_interface->wireless.wirelessStaMode.wirelessApPsk,
									ApPskBuf,
									strlen(n_interface->wireless.wirelessStaMode.wirelessApPsk));
								NETSDK_json_set_string2(staModeJSON, "wirelessApEssId", ApEssIdBuf);
								if(strlen(ApPskBuf)>0){
									NETSDK_json_set_string2(staModeJSON, "wirelessApPsk", ApPskBuf);
								}else{
									NETSDK_json_set_string(staModeJSON, "wirelessApPsk", ApPskBuf);
								}
							}
							else {
								NETSDK_json_set_string2(staModeJSON, "wirelessApEssId", n_interface->wireless.wirelessStaMode.wirelessApEssId);
								if(strlen(n_interface->wireless.wirelessStaMode.wirelessApPsk)> 0){
									NETSDK_json_set_string2(staModeJSON, "wirelessApPsk", n_interface->wireless.wirelessStaMode.wirelessApPsk);
								}else{
									NETSDK_json_set_string(staModeJSON, "wirelessApPsk", n_interface->wireless.wirelessStaMode.wirelessApPsk);
								}
							}
							NETSDK_json_set_boolean2(staModeJSON, "wirelessFixedBpsModeEnabled", n_interface->wireless.wirelessStaMode.wirelessFixedBpsModeEnabled);
							NETSDK_json_set_string2(staModeBackupJSON, "wirelessApBssId", n_interface->wireless.wirelessStaModeBackup.wirelessApBssId);

							if(enableb64) { //Take Place By Base64 Encoded Result
								char ApEssIdBuf[256] = {0};
								char ApPskBuf[256] = {0};
								base64_encode(n_interface->wireless.wirelessApMode.wirelessEssId,
									ApEssIdBuf,
									strlen(n_interface->wireless.wirelessApMode.wirelessEssId));
								base64_encode(n_interface->wireless.wirelessApMode.wirelessPsk,
									ApPskBuf,
									strlen(n_interface->wireless.wirelessApMode.wirelessPsk));
								NETSDK_json_set_string2(apModeJSON, "wirelessEssId", ApEssIdBuf);
								NETSDK_json_set_string2(apModeJSON, "wirelessPsk", ApPskBuf);

								memset(ApEssIdBuf, 0, sizeof(ApEssIdBuf));
								memset(ApPskBuf, 0, sizeof(ApPskBuf));
								base64_encode(n_interface->wireless.wirelessStaModeBackup.wirelessApEssId,
									ApEssIdBuf,
									strlen(n_interface->wireless.wirelessStaModeBackup.wirelessApEssId));
								base64_encode(n_interface->wireless.wirelessStaModeBackup.wirelessApPsk,
									ApPskBuf,
									strlen(n_interface->wireless.wirelessStaModeBackup.wirelessApPsk));
								NETSDK_json_set_string2(staModeBackupJSON, "wirelessApEssId", ApEssIdBuf);
								NETSDK_json_set_string2(staModeBackupJSON, "wirelessApPsk", ApPskBuf);
							}
							else {
								NETSDK_json_set_string2(staModeBackupJSON, "wirelessApEssId", n_interface->wireless.wirelessStaModeBackup.wirelessApEssId);
								NETSDK_json_set_string2(staModeBackupJSON, "wirelessApPsk", n_interface->wireless.wirelessStaModeBackup.wirelessApPsk);
								NETSDK_json_set_string2(apModeJSON, "wirelessEssId", n_interface->wireless.wirelessApMode.wirelessEssId);
								NETSDK_json_set_string2(apModeJSON, "wirelessPsk", n_interface->wireless.wirelessApMode.wirelessPsk);
							}
							NETSDK_json_set_boolean2(apModeJSON, "wirelessEssIdBroadcastingEnabled", n_interface->wireless.wirelessApMode.wireLessEssIdBroadcastingEnabled);
							str = NETSDK_MAP_DEC2STR(wirelessWpaMode_map, n_interface->wireless.wirelessApMode.wirelessWpaMode, "WPA_PSK");
							NETSDK_json_set_string2(apModeJSON, "wirelessWpaMode", str);
							str = NETSDK_MAP_DEC2STR(wireLessApMode80211nChannel_map, n_interface->wireless.wirelessApMode.wirelessApMode80211nChannel, "Auto");
							NETSDK_json_set_string2(apModeJSON, "wirelessApMode80211nChannel", str);
							NETSDK_json_set_boolean2(dhcpServerJSON, "enabled", n_interface->wireless.dhcpServer.enabled);
							NETSDK_json_set_boolean2(dhcpServerJSON, "dhcpAutoSettingEnabled", n_interface->wireless.dhcpServer.dhcpAutoSettingEnabled);
							NETSDK_json_set_string2(dhcpServerJSON, "dhcpIpRange", n_interface->wireless.dhcpServer.dhcpIpRange);
							NETSDK_json_set_string2(dhcpServerJSON, "dhcpIpNumber", n_interface->wireless.dhcpServer.dhcpIpNumber);
							NETSDK_json_set_string2(dhcpServerJSON, "dhcpIpDns", n_interface->wireless.dhcpServer.dhcpIpDns);
							NETSDK_json_set_string2(dhcpServerJSON, "dhcpIpGateway", n_interface->wireless.dhcpServer.dhcpIpGateway);

                            if(NULL != repeaterJSON)
                            {
                                str = NETSDK_MAP_DEC2STR(repeaterMode_map, n_interface->wireless.repeater.repeaterMode, "auto");
                                NETSDK_json_set_string2(repeaterJSON, "repeaterMode", str);
                                str = NETSDK_MAP_DEC2STR(repeaterWorkMode_map, n_interface->wireless.repeater.repeaterWorkMode, "auto");
                                NETSDK_json_set_string2(repeaterJSON, "repeaterWorkMode", str);
                                str = NETSDK_MAP_DEC2STR(repeaterWiredBandWidth_map, n_interface->wireless.repeater.wiredBandWidth, "100M");
                                NETSDK_json_set_string2(repeaterJSON, "wiredBandWidth", str);
                                NETSDK_json_set_int2(repeaterJSON, "wiredMaxConn", n_interface->wireless.repeater.wiredMaxConn);
                                NETSDK_json_set_int2(repeaterJSON, "wirelessMaxConn", n_interface->wireless.repeater.wirelessMaxConn);
                            }
						}
					}
				}
				LP_JSON_OBJECT dnsJSON = NETSDK_json_get_child(network, "networkDNS");
				if(dnsJSON != NULL){
					if(IS_VALID_IPADDR(n_interface->dns.staticPreferredDns)){
						NETSDK_json_set_string2(dnsJSON, "preferredDns", n_interface->dns.staticPreferredDns);
					}
					if(IS_VALID_IPADDR(n_interface->dns.staticAlternateDns)){
						NETSDK_json_set_string2(dnsJSON, "staticAlternateDns", n_interface->dns.staticAlternateDns);
					}
				}

				LP_JSON_OBJECT eseeJSON = NETSDK_json_get_child(network, "esee");
				if(eseeJSON != NULL){
					bool esee_enable = NETSDK_json_get_boolean(eseeJSON, "enabled");
					if(esee_enable != n_interface->esee.enabled){
						force_reboot = true;
					}
					NETSDK_json_set_boolean2(eseeJSON, "enabled", n_interface->esee.enabled);
				}
				
			
		}else{//get
				if(NULL != j_interface){
					LP_JSON_OBJECT lanJSON = NETSDK_json_get_child(j_interface, "lan");
					LP_JSON_OBJECT upnpJSON = NETSDK_json_get_child(j_interface, "upnp");
					LP_JSON_OBJECT pppoeJSON = NETSDK_json_get_child(j_interface, "pppoe");
					LP_JSON_OBJECT ddnsJSON = NETSDK_json_get_child(j_interface, "ddns");

					n_interface->id = id;
					NETSDK_json_get_string(j_interface, "interfaceName", n_interface->interfaceName, sizeof(n_interface->interfaceName));
					NETSDK_json_get_string(lanJSON, "ipVersion", text, sizeof(text));
					n_interface->lan.ipVersion = NETSDK_MAP_STR2DEC(ipVersion_map, text, kNSDK_NETWORK_LAN_IP_VERSION_V4);
					NETSDK_json_get_string(lanJSON, "addressingType", text, sizeof(text));
					n_interface->lan.addressingType = NETSDK_MAP_STR2DEC(addressType_map, text, kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC);
					NETSDK_json_get_string(lanJSON, "staticIP", n_interface->lan.staticIP, sizeof(n_interface->lan.staticIP));
					NETSDK_json_get_string(lanJSON, "staticNetmask", n_interface->lan.staticNetmask, sizeof(n_interface->lan.staticNetmask));
					NETSDK_json_get_string(lanJSON, "staticGateway", n_interface->lan.staticGateway, sizeof(n_interface->lan.staticGateway));
					n_interface->upnp.enabled = NETSDK_json_get_boolean(upnpJSON, "enabled");
					n_interface->pppoe.enabled = NETSDK_json_get_boolean(pppoeJSON, "enabled");
					NETSDK_json_get_string(pppoeJSON, "pppoeUserName", n_interface->pppoe.pppoeUserName, sizeof(n_interface->pppoe.pppoeUserName));
					NETSDK_json_get_string(pppoeJSON, "pppoePassword", n_interface->pppoe.pppoePassword, sizeof(n_interface->pppoe.pppoePassword));
					n_interface->ddns.enabled = NETSDK_json_get_boolean(ddnsJSON, "enabled");
					NETSDK_json_get_string(ddnsJSON, "ddnsProvider", text, sizeof(text));
					n_interface->ddns.ddnsProvider= NETSDK_MAP_STR2DEC(ddnsProvider_map, text, kNSDK_NETWORK_DDNS_PROVIDER_DYNDDNS);
					NETSDK_json_get_string(ddnsJSON, "ddnsUrl", n_interface->ddns.ddnsUrl, sizeof(n_interface->ddns.ddnsUrl));
					NETSDK_json_get_string(ddnsJSON, "ddnsUserName", n_interface->ddns.ddnsUserName, sizeof(n_interface->ddns.ddnsUserName));
					NETSDK_json_get_string(ddnsJSON, "ddnsPassword", n_interface->ddns.ddnsPassword, sizeof(n_interface->ddns.ddnsPassword));					

					if(id == 4){
						LP_JSON_OBJECT wirelessJSON = NETSDK_json_get_child(j_interface, "wireless");
						if(wirelessJSON != NULL){
							LP_JSON_OBJECT staModeJSON = NETSDK_json_get_child(wirelessJSON, "stationMode");
							LP_JSON_OBJECT staModeBackupJSON = NETSDK_json_get_child(wirelessJSON, "stationModeBackup");
							LP_JSON_OBJECT apModeJSON = NETSDK_json_get_child(wirelessJSON, "accessPointMode");
							LP_JSON_OBJECT dhcpServerJSON = NETSDK_json_get_child(wirelessJSON, "dhcpServer");
							LP_JSON_OBJECT repeaterJSON = NETSDK_json_get_child(wirelessJSON, "repeater");

							NETSDK_json_get_string(wirelessJSON, "repeaterDevId", n_interface->wireless.repeaterDevId, sizeof(n_interface->wireless.repeaterDevId));

							NETSDK_json_get_string(wirelessJSON, "wirelessMode", text, sizeof(text));
							n_interface->wireless.wirelessMode = NETSDK_MAP_STR2DEC(wireLessMode_map, text, NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT);
							NETSDK_json_get_string(staModeJSON, "wirelessStaMode", n_interface->wireless.wirelessStaMode.wirelessStaMode, sizeof(n_interface->wireless.wirelessStaMode.wirelessStaMode));
							NETSDK_json_get_string(staModeJSON, "wirelessApBssId", n_interface->wireless.wirelessStaMode.wirelessApBssId, sizeof(n_interface->wireless.wirelessStaMode.wirelessApBssId));
							if(enableb64) { //Take Place By Base64 Encoded Result
								char ApEssIdBuf[256] = {0};
								char ApPskBuf[256] = {0};
								NETSDK_json_get_string(staModeJSON, "wirelessApEssId", ApEssIdBuf, sizeof(ApEssIdBuf));
								NETSDK_json_get_string(staModeJSON, "wirelessApPsk", ApPskBuf, sizeof(ApPskBuf));
								base64_decode(ApEssIdBuf, n_interface->wireless.wirelessStaMode.wirelessApEssId, strlen(ApEssIdBuf));
								base64_decode(ApPskBuf, n_interface->wireless.wirelessStaMode.wirelessApPsk, strlen(ApPskBuf));
								memset(ApEssIdBuf, 0, sizeof(ApEssIdBuf));
								memset(ApPskBuf, 0, sizeof(ApPskBuf));
								NETSDK_json_get_string(staModeBackupJSON, "wirelessApEssId", ApEssIdBuf, sizeof(ApEssIdBuf));
								NETSDK_json_get_string(staModeBackupJSON, "wirelessApPsk", ApPskBuf, sizeof(ApPskBuf));
								base64_decode(ApEssIdBuf, n_interface->wireless.wirelessStaModeBackup.wirelessApEssId, strlen(ApEssIdBuf));
								base64_decode(ApPskBuf, n_interface->wireless.wirelessStaModeBackup.wirelessApPsk, strlen(ApPskBuf));
							}
							else {
								NETSDK_json_get_string(staModeJSON, "wirelessApEssId", n_interface->wireless.wirelessStaMode.wirelessApEssId, sizeof(n_interface->wireless.wirelessStaMode.wirelessApEssId));
								NETSDK_json_get_string(staModeJSON, "wirelessApPsk", n_interface->wireless.wirelessStaMode.wirelessApPsk, sizeof(n_interface->wireless.wirelessStaMode.wirelessApPsk));
								NETSDK_json_get_string(staModeBackupJSON, "wirelessApEssId", n_interface->wireless.wirelessStaModeBackup.wirelessApEssId, sizeof(n_interface->wireless.wirelessStaModeBackup.wirelessApEssId));
								NETSDK_json_get_string(staModeBackupJSON, "wirelessApPsk", n_interface->wireless.wirelessStaModeBackup.wirelessApPsk, sizeof(n_interface->wireless.wirelessStaModeBackup.wirelessApPsk));
							}
							n_interface->wireless.wirelessStaMode.wirelessFixedBpsModeEnabled = NETSDK_json_get_boolean(staModeJSON, "wirelessFixedBpsModeEnabled");

							NETSDK_json_get_string(staModeBackupJSON, "wirelessApBssId", n_interface->wireless.wirelessStaModeBackup.wirelessApBssId, sizeof(n_interface->wireless.wirelessStaModeBackup.wirelessApBssId));
							n_interface->wireless.wirelessStaModeBackup.wirelessFixedBpsModeEnabled = NETSDK_json_get_boolean(staModeBackupJSON, "wirelessFixedBpsModeEnabled");

							NETSDK_json_get_string(apModeJSON, "wirelessBssId", n_interface->wireless.wirelessApMode.wirelessBssId, sizeof(n_interface->wireless.wirelessApMode.wirelessBssId));
							if(enableb64) { //Take Place By Base64 Encoded Result
								char ApEssIdBuf[256] = {0};
								char ApPskBuf[256] = {0};
								NETSDK_json_get_string(apModeJSON, "wirelessEssId", ApEssIdBuf, sizeof(ApEssIdBuf));
								NETSDK_json_get_string(apModeJSON, "wirelessPsk", ApPskBuf, sizeof(ApPskBuf));
								base64_decode(ApEssIdBuf, n_interface->wireless.wirelessApMode.wirelessEssId, strlen(ApEssIdBuf));
								base64_decode(ApPskBuf, n_interface->wireless.wirelessApMode.wirelessPsk, strlen(ApPskBuf));
							}
							else {
								NETSDK_json_get_string(apModeJSON, "wirelessEssId", n_interface->wireless.wirelessApMode.wirelessEssId, sizeof(n_interface->wireless.wirelessApMode.wirelessEssId));
								NETSDK_json_get_string(apModeJSON, "wirelessPsk", n_interface->wireless.wirelessApMode.wirelessPsk, sizeof(n_interface->wireless.wirelessApMode.wirelessPsk));
							}
							NETSDK_json_get_string(apModeJSON, "wirelessApMode", text, sizeof(text));
							n_interface->wireless.wirelessApMode.wireLessApMode= NETSDK_MAP_STR2DEC(wireLessApMode_map, text, NSDK_NETWORK_WIRELESS_APMODE_80211N);
							
							n_interface->wireless.wirelessApMode.wireLessEssIdBroadcastingEnabled= NETSDK_json_get_boolean(apModeJSON, "wirelessEssIdBroadcastingEnabled");
							NETSDK_json_get_string(apModeJSON, "wirelessWpaMode", text, sizeof(text));
							n_interface->wireless.wirelessApMode.wirelessWpaMode= NETSDK_MAP_STR2DEC(wirelessWpaMode_map, text, NSDK_NETWORK_WIRELESS_WPAMODE_WPA_PSK);
							NETSDK_json_get_string(apModeJSON, "wirelessApMode80211nChannel", text, sizeof(text));
							n_interface->wireless.wirelessApMode.wirelessApMode80211nChannel= NETSDK_MAP_STR2DEC(wireLessApMode80211nChannel_map, text, NSDK_NETWORK_WIRELESS_APMODE_80211N_CHANNEL_AUTO);
							n_interface->wireless.dhcpServer.enabled = NETSDK_json_get_boolean(dhcpServerJSON, "enabled");
							n_interface->wireless.dhcpServer.dhcpAutoSettingEnabled = NETSDK_json_get_boolean(dhcpServerJSON, "dhcpAutoSettingEnabled");
							NETSDK_json_get_string(dhcpServerJSON, "dhcpIpRange", n_interface->wireless.dhcpServer.dhcpIpRange, sizeof(n_interface->wireless.dhcpServer.dhcpIpRange));
							NETSDK_json_get_string(dhcpServerJSON, "dhcpIpNumber", n_interface->wireless.dhcpServer.dhcpIpNumber, sizeof(n_interface->wireless.dhcpServer.dhcpIpNumber));
							NETSDK_json_get_string(dhcpServerJSON, "dhcpIpDns", n_interface->wireless.dhcpServer.dhcpIpDns, sizeof(n_interface->wireless.dhcpServer.dhcpIpDns));
							NETSDK_json_get_string(dhcpServerJSON, "dhcpIpGateway", n_interface->wireless.dhcpServer.dhcpIpGateway, sizeof(n_interface->wireless.dhcpServer.dhcpIpGateway));

                            if(NULL != repeaterJSON)
                            {
                                NETSDK_json_get_string(repeaterJSON, "repeaterMode", text, sizeof(text));
                                n_interface->wireless.repeater.repeaterMode = NETSDK_MAP_STR2DEC(repeaterMode_map, text, NSDK_NETWORK_REPEAER_MODE_AUTO);

                                NETSDK_json_get_string(repeaterJSON, "repeaterWorkMode", text, sizeof(text));
                                n_interface->wireless.repeater.repeaterWorkMode = NETSDK_MAP_STR2DEC(repeaterWorkMode_map, text, NSDK_NETWORK_REPEAER_WORK_MODE_AUTO);

                                NETSDK_json_get_string(repeaterJSON, "wiredBandWidth", text, sizeof(text));
                                n_interface->wireless.repeater.wiredBandWidth = NETSDK_MAP_STR2DEC(repeaterWiredBandWidth_map, text, NSDK_NETWORK_REPEAER_WIRED_BANDWIDTH_100M);

                                n_interface->wireless.repeater.wiredMaxConn = NETSDK_json_get_int(repeaterJSON, "wiredMaxConn");
                                n_interface->wireless.repeater.wirelessMaxConn = NETSDK_json_get_int(repeaterJSON, "wirelessMaxConn");
                            }
						}
					}					
				}
				LP_JSON_OBJECT dnsJSON = NETSDK_json_get_child(network, "networkDNS");
				if(dnsJSON != NULL){
					NETSDK_json_get_string(dnsJSON, "preferredDns", n_interface->dns.staticPreferredDns, sizeof(n_interface->dns.staticPreferredDns));
					NETSDK_json_get_string(dnsJSON, "staticAlternateDns", n_interface->dns.staticAlternateDns, sizeof(n_interface->dns.staticAlternateDns));
				}

				LP_JSON_OBJECT eseeJSON = NETSDK_json_get_child(network, "esee");
				if(eseeJSON != NULL){
					n_interface->esee.enabled = NETSDK_json_get_boolean(eseeJSON, "enabled");
				}
			}
			// release
			json_object_put(network);
			network = NULL;
			NETWORK_LEAVE_CRITICAL();
		}
		if(set_flag){
			// save to file
			if(force_reboot){
				NETSDK_conf_network_save2(eNSDK_CONF_SAVE_REBOOT, delay);
			}else{
				NETSDK_conf_network_save2(opteration, delay);
			}
			//NETSDK_conf_interface_dump(n_interface);
		}
		//NETSDK_conf_interface_dump(n_interface);
		return n_interface;
	}

	return NULL;
}

int NETSDK_conf_limit_netmask(int id, int opteration)
{
#if defined(WIFI)
	ST_NSDK_NETWORK_INTERFACE wired, wireless;
	ifconf_ipv4_addr_t wired_ip, wired_netmask, wireless_ip, wireless_netmask, limit_netmask;

	NETSDK_conf_interface_get(1, &wired);
	NETSDK_conf_interface_get(4, &wireless);

	wired_ip = ifconf_ipv4_aton(wired.lan.staticIP);
	wired_netmask = ifconf_ipv4_aton(wired.lan.staticNetmask);

	wireless_ip = ifconf_ipv4_aton(wireless.lan.staticIP);
	wireless_netmask = ifconf_ipv4_aton(wireless.lan.staticNetmask);

	APP_TRACE("NETSDK_conf_limit_netmask %x(%x) %x(%x)!", 
				wired_ip.s_addr, wireless_netmask.s_addr, wireless_ip.s_addr, wired_netmask.s_addr);

	if((0x00FFFFFF & wired_ip.s_addr) == (0x00FFFFFF & wireless_ip.s_addr)){
		//limit_netmask.s_addr = (wired_netmask.s_addr > wireless_netmask.s_addr ? wireless_netmask.s_addr : wired_netmask.s_addr);
		if(wireless_netmask.s_addr != wired_netmask.s_addr){
			if(id == 4){
				snprintf(wired.lan.staticNetmask, sizeof(wired.lan.staticNetmask), "%s", ifconf_ipv4_ntoa(wireless_netmask));
				APP_TRACE("reset wired netmask to %s!", wired.lan.staticNetmask);
				netsdk_conf_interface(true, 1, &wired, opteration, 1, 1);
			}
			else if(id == 1){
				snprintf(wireless.lan.staticNetmask, sizeof(wireless.lan.staticNetmask), "%s", ifconf_ipv4_ntoa(wired_netmask));
				APP_TRACE("reset wireless netmask to %s!", wired.lan.staticNetmask);
				netsdk_conf_interface(true, 4, &wireless, opteration, 1, 1);
			}
		}
	}
#endif
	return 0;
}

LP_NSDK_NETWORK_INTERFACE NETSDK_conf_interface_get(int id, LP_NSDK_NETWORK_INTERFACE n_interface)
{
	return netsdk_conf_interface(false, id, n_interface, 0, 0, 1);
}

LP_NSDK_NETWORK_INTERFACE NETSDK_conf_interface_set_by_delay(int id, LP_NSDK_NETWORK_INTERFACE n_interface, EM_NSDK_CONF_SAVE_OPERATION opteration, int delay)
{
	LP_NSDK_NETWORK_INTERFACE ret = netsdk_conf_interface(true, id, n_interface, opteration, delay, 1);
	NETSDK_conf_limit_netmask(id, opteration);
	return ret;
}

LP_NSDK_NETWORK_INTERFACE NETSDK_conf_interface_set(int id, LP_NSDK_NETWORK_INTERFACE n_interface, EM_NSDK_CONF_SAVE_OPERATION opteration)
{
	LP_NSDK_NETWORK_INTERFACE ret = netsdk_conf_interface(true, id, n_interface, opteration, 1, 1);
	NETSDK_conf_limit_netmask(id, opteration);
	return ret;
}

#define NETSDK_TMP_CONFIG_INI "/tmp/tmp_netsdk_conf.ini"
int NETSDK_tmp_interface_set(int id, LP_NSDK_NETWORK_INTERFACE n_interface, EM_NSDK_CONF_SAVE_OPERATION opteration)
{
	lpINI_PARSER config_ini = NULL;
	FILE* fid = NULL;
	//init file
	if(access(NETSDK_TMP_CONFIG_INI, F_OK) != 0){
		FILE* fid = fopen(NETSDK_TMP_CONFIG_INI, "w+b");
		if(fid){
			fclose(fid);
			fid = NULL;
		}
	}

	config_ini = OpenIniFile(NETSDK_TMP_CONFIG_INI);
	if(n_interface && config_ini){
		if(id == 4){
			config_ini->write_text(config_ini, "wirelessStaMode", "wirelessApEssId", n_interface->wireless.wirelessStaMode.wirelessApEssId);
			config_ini->write_text(config_ini, "wirelessStaMode", "wirelessApPsk", n_interface->wireless.wirelessStaMode.wirelessApPsk);
			config_ini->write_text(config_ini, "wirelessStaMode", "staticIP", n_interface->lan.staticIP);
			config_ini->write_int(config_ini, "wirelessStaMode", "dhcp", n_interface->wireless.dhcpServer.dhcpAutoSettingEnabled);
			config_ini->write_int(config_ini, "wirelessStaMode", "enable", 1);
			config_ini->write_int(config_ini, "wirelessStaMode", "wirelessMode", n_interface->wireless.wirelessMode);
		}
		else if(id == 1){
			config_ini->write_int(config_ini, "wired", "enable", 1);
			config_ini->write_text(config_ini, "wired", "staticIP", n_interface->lan.staticIP);
		}
		WriteIniFile(config_ini, NETSDK_TMP_CONFIG_INI);
		CloseIniFile(config_ini);
		config_ini = NULL;
		NETSDK_conf_network_save2(opteration, 1);
		return 0;
	}
	return 0;
}

int NETSDK_tmp_interface_get(int id, LP_NSDK_NETWORK_INTERFACE n_interface)
{
	lpINI_PARSER config_ini = NULL;
	int flag = 0;
	if(access(NETSDK_TMP_CONFIG_INI, F_OK) != 0){
		return 0;
	}

	config_ini = OpenIniFile(NETSDK_TMP_CONFIG_INI);
	if(n_interface && config_ini){
		printf("NETSDK_tmp_interface_get %d, enable %d\n", id, config_ini->read_int(config_ini, "wirelessStaMode", "enable", 0));
		if(id == 4){
			if(config_ini->read_int(config_ini, "wirelessStaMode", "enable", 0)){
				config_ini->read_text(config_ini, "wirelessStaMode", "wirelessApEssId", n_interface->wireless.wirelessStaMode.wirelessApEssId, n_interface->wireless.wirelessStaMode.wirelessApEssId, sizeof(n_interface->wireless.wirelessStaMode.wirelessApEssId));
				config_ini->read_text(config_ini, "wirelessStaMode", "wirelessApPsk", n_interface->wireless.wirelessStaMode.wirelessApPsk, n_interface->wireless.wirelessStaMode.wirelessApPsk, sizeof(n_interface->wireless.wirelessStaMode.wirelessApPsk));
				config_ini->read_text(config_ini, "wirelessStaMode", "staticIP", "172.20.14.114", n_interface->lan.staticIP, sizeof(n_interface->lan.staticIP));
				n_interface->wireless.dhcpServer.dhcpAutoSettingEnabled = config_ini->read_int(config_ini, "wirelessStaMode", "dhcp", 0);
				n_interface->wireless.wirelessMode = config_ini->read_int(config_ini, "wirelessStaMode", "wirelessMode", NSDK_NETWORK_WIRELESS_MODE_STATIONMODE);
				flag = 1;
			}
		}
		else if(id == 1){
			if(config_ini->read_int(config_ini, "wired", "enable", 0)){
				config_ini->read_text(config_ini, "wired", "staticIP", "192.168.1.168", n_interface->lan.staticIP, sizeof(n_interface->lan.staticIP));
				flag = 1;
			}
		}
		CloseIniFile(config_ini);
		config_ini = NULL;
		return flag;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
/////////////////////////////////////////////////////////////////////////////////////////////////////

void NETSDK_conf_port_dump(LP_NSDK_NETWORK_PORT port)
{
	printf("id:%d\r\n"
		"name:%s\r\n"
		"value:%d\r\n\r\n",
		port->id,
		port->portName,
		port->value
	);
}


static LP_NSDK_NETWORK_PORT netsdk_conf_port(bool set_flag, int id, LP_NSDK_NETWORK_PORT port, int opteration, int delay)
{
	int i = 0;
	char text[128] = {""};
	char *str = NULL;

	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		return NULL;
	}

	if(port){
		if(0 == NETWORK_ENTER_CRITICAL()){
			LP_JSON_OBJECT network = json_object_get(netsdk->network_conf);
			LP_JSON_OBJECT portsJSON = NETSDK_json_get_child(network, "networkPort.networkPortList");
			LP_JSON_OBJECT portJSON = network_find_interface(portsJSON, id);
			if(set_flag){//set
				if(portJSON != NULL){
					NETSDK_json_set_int2(portJSON, "value", port->value);
				}			
			}else{//get
				if(portJSON != NULL){
					port->id = id;
					port->value = NETSDK_json_get_int(portJSON, "value");
					NETSDK_json_get_string(portJSON, "portName", port->portName, sizeof(port->portName));
				}
			}
			// release
			json_object_put(network);
			network = NULL;
			NETWORK_LEAVE_CRITICAL();
		}
		if(set_flag){
			// save to file
			NETSDK_conf_network_save2(opteration, delay);
		}
		//NETSDK_conf_port_dump(port);
		return port;
	}

	return NULL;
}


LP_NSDK_NETWORK_PORT NETSDK_conf_port_get(int id, LP_NSDK_NETWORK_PORT port)
{
	return netsdk_conf_port(false, id, port, 0, 0);
}

LP_NSDK_NETWORK_PORT NETSDK_conf_port_set_by_delay(int id, LP_NSDK_NETWORK_PORT port, EM_NSDK_CONF_SAVE_OPERATION opteration, int delay)
{
	return netsdk_conf_port(true, id, port, opteration, delay);
}

LP_NSDK_NETWORK_PORT NETSDK_conf_port_set(int id, LP_NSDK_NETWORK_PORT port, EM_NSDK_CONF_SAVE_OPERATION opteration)
{
	return netsdk_conf_port(true, id, port, opteration, 1);
}

static LP_NSDK_NETWORK_OSSCLD netsdk_conf_osscloud(bool set_flag, LP_NSDK_NETWORK_OSSCLD osscld)
{
	int i = 0;
	int arrayLen;
	int chNum;  // 允许上传的通道总数。从第一个通道依次往后数
	int ch;
	int chMax;
	int stream;
	int streamMax;
	LP_NSDK_NETWORK_OSSCLD retPst = NULL;
	LP_JSON_OBJECT networkJson = NULL;
	LP_JSON_OBJECT ossCloudJson = NULL;
	LP_JSON_OBJECT isBoundJson = NULL;
	LP_JSON_OBJECT chNumJson = NULL;
	LP_JSON_OBJECT uploadJson = NULL;
	LP_JSON_OBJECT uploadEleJson = NULL;
	LP_JSON_OBJECT chJson = NULL;
	LP_JSON_OBJECT streamJson = NULL;
	LP_JSON_OBJECT enabledJson = NULL;
	LP_JSON_OBJECT typeJson = NULL;

	if(NULL == netsdk){
		APP_TRACE("netsdk not init!");
		retPst = NULL;
		goto FUNC_RETURN;
	}

	if(NULL == osscld){
		APP_TRACE("%s: osscld can't be NULL!", __FUNCTION__);
		retPst = NULL;
		goto FUNC_RETURN;
	}

	// network json
	networkJson = json_object_get(netsdk->network_conf);


	// 检查 network.json ossCloud 中的各个字段

	// ossCloud
	ossCloudJson = NETSDK_json_get_child(networkJson, "ossCloud");
	if(NULL == ossCloudJson){
		APP_TRACE("%s: Failed to get \"ossCloud\" from network json", __FUNCTION__);
		retPst = NULL;
		goto FUNC_RETURN;
	}

	// ossCloud.isBound
	isBoundJson = NETSDK_json_get_child(ossCloudJson, "isBound");
	if(NULL == isBoundJson){
		APP_TRACE("%s: Failed to get \"isBound\" from ossCloud json", __FUNCTION__);
		retPst = NULL;
		goto FUNC_RETURN;
	}

	// ossCloud.chNum
	chNumJson = NETSDK_json_get_child(ossCloudJson, "chNum");
	if(NULL == chNumJson){
		APP_TRACE("%s: Failed to get \"chNum\" from ossCloud json", __FUNCTION__);
		retPst = NULL;
		goto FUNC_RETURN;
	}

	// ossCloud.upload
	uploadJson = NETSDK_json_get_child(ossCloudJson, "upload");
	if(NULL == uploadJson){
		APP_TRACE("%s: Failed to get \"upload\" from ossCloud json", __FUNCTION__);
		retPst = NULL;
		goto FUNC_RETURN;
	}

	if (!json_object_is_type(uploadJson, json_type_array)) {
		APP_TRACE("%s: \"upload\" object is not array", __FUNCTION__);
		retPst = NULL;
		goto FUNC_RETURN;
	}

	chMax = sizeof(osscld->channel)/sizeof(osscld->channel[0]);
	streamMax = sizeof(osscld->channel[0].stream)/sizeof(osscld->channel[0].stream[0]);

	chNum = json_object_get_int(chNumJson);
	if (chNum < 0 || chNum > chMax) {
		APP_TRACE("%s: get chNum is out of range", __FUNCTION__);
		retPst = NULL;
		goto FUNC_RETURN;
	}

	arrayLen = json_object_array_length(uploadJson);
	if (arrayLen < 0) {
		APP_TRACE("%s: get arrayLen is out of range", __FUNCTION__);
		retPst = NULL;
		goto FUNC_RETURN;
	}

	if (set_flag) {
		// set
		int rec_type;

		NETSDK_json_set_boolean(ossCloudJson, "isBound", osscld->isBound);

		for (i = 0; i < arrayLen; i++) {
			// ossCloud.upload[]
			uploadEleJson = json_object_array_get_idx(uploadJson, i);
			if (NULL == uploadEleJson) {
				APP_TRACE("%s: Failed to get object from array", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}

			// ossCloud.upload[].ch
			chJson = NETSDK_json_get_child(uploadEleJson, "ch");
			if(NULL == chJson){
				APP_TRACE("%s: Failed to get \"ch\" from uploadEleJson", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}

			// ossCloud.upload[].stream
			streamJson = NETSDK_json_get_child(uploadEleJson, "stream");
			if(NULL == streamJson){
				APP_TRACE("%s: Failed to get \"stream\" from uploadEleJson", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}

//			// ossCloud.upload[].enabled
//			enabledJson = NETSDK_json_get_child(uploadEleJson, "enabled");
//			if(NULL == enabledJson){
//				APP_TRACE("%s: Failed to get \"enable\" from uploadEleJson", __FUNCTION__);
//				retPst = NULL;
//				goto FUNC_RETURN;
//
//			}

			ch = json_object_get_int(chJson);
			if (ch < 1 || ch > chNum) {
				APP_TRACE("%s: get channel number is out of range", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}
			stream = json_object_get_int(streamJson);
			if (stream < 1 || stream > streamMax) {
				APP_TRACE("%s: get stream number is out of range", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}

			rec_type = osscld->channel[ch-1].stream[stream-1].type;
			if (rec_type < 0 || rec_type > 1) {
				APP_TRACE("%s: record type number is out of range, rec_type: %d",
						  __FUNCTION__, rec_type);
				retPst = NULL;
				goto FUNC_RETURN;
			}

			NETSDK_json_set_boolean(uploadEleJson,
									"enabled",
									osscld->channel[ch-1].stream[stream-1].enable);

			NETSDK_json_set_int(uploadEleJson, "type", rec_type);
		}
		NETSDK_conf_network_save();
		retPst = osscld;

	} else {
		// get

		osscld->isBound = json_object_get_boolean(isBoundJson)?true:false;
		osscld->chNum = chNum;

		for (i = 0; i < arrayLen; i++) {
			// ossCloud.upload[]
			uploadEleJson = json_object_array_get_idx(uploadJson, i);
			if (NULL == uploadEleJson) {
				APP_TRACE("%s: Failed to get object from array", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}

			// ossCloud.upload[].ch
			chJson = NETSDK_json_get_child(uploadEleJson, "ch");
			if(NULL == chJson) {
				APP_TRACE("%s: Failed to get \"ch\" from uploadEleJson", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}

			// ossCloud.upload[].stream
			streamJson = NETSDK_json_get_child(uploadEleJson, "stream");
			if(NULL == streamJson) {
				APP_TRACE("%s: Failed to get \"stream\" from uploadEleJson", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}

			// ossCloud.upload[].enabled
			enabledJson = NETSDK_json_get_child(uploadEleJson, "enabled");
			if(NULL == enabledJson) {
				APP_TRACE("%s: Failed to get \"enable\" from uploadEleJson", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}

			// ossCloud.upload[].type
			typeJson = NETSDK_json_get_child(uploadEleJson, "type");
			if(NULL == typeJson) {
				APP_TRACE("%s: Failed to get \"type\" from uploadEleJson", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}

			ch = json_object_get_int(chJson);
			if (ch < 1 || ch > chNum) {
				APP_TRACE("%s: get channel number is out of range", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}
			stream = json_object_get_int(streamJson);
			if (stream < 1 || stream > streamMax) {
				APP_TRACE("%s: get stream number is out of range", __FUNCTION__);
				retPst = NULL;
				goto FUNC_RETURN;
			}

			osscld->channel[ch-1].stream[stream-1].enable = json_object_get_boolean(enabledJson)?true:false;
			osscld->channel[ch-1].stream[stream-1].type = json_object_get_int(typeJson);
		}
		retPst = osscld;
	}


FUNC_RETURN:
	if (NULL != networkJson) {
		json_object_put(networkJson);
	}

	return retPst;
}


LP_NSDK_NETWORK_OSSCLD NETSDK_conf_osscloud_get(LP_NSDK_NETWORK_OSSCLD osscld)
{
	return netsdk_conf_osscloud(false, osscld);
}

LP_NSDK_NETWORK_OSSCLD NETSDK_conf_osscloud_set(LP_NSDK_NETWORK_OSSCLD osscld)
{
	return netsdk_conf_osscloud(true, osscld);
}

int NETSDK_network_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri, char *content, int content_max)
{
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	HTTP_CSTR_t prefix = NULL;
	if(prefix = "/INTERFACE", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		// get all channels
		ret =network_interfaceList_instance(context, sub_uri, content, content_max);
	}else if(prefix = "/PORT", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		ret =network_portList_instance(context, sub_uri, content, content_max);
	}else if(prefix = "/DNS", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		ret =network_dns(context, sub_uri, content, content_max);
	}else if(prefix = "/ESEE", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		ret =network_esee(context, sub_uri, content, content_max);
	}else if(prefix = "/WIRELESS", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		sub_uri += strlen(prefix);
		ret =network_wireless(context, sub_uri, content, content_max);
	}else if(prefix = "/WIRELESS_B64EN", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
		sub_uri += strlen(prefix);
		ret =network_wireless(context, sub_uri, content, content_max);
	}else{
		
	}
	return ret;
}


