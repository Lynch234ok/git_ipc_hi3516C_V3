

#include "hichip_discover.h"
#include "http_util.h"
#include "sysconf.h"
#include "ifconf.h"
#include "esee_client.h"
#include "app_debug.h"
#include "netsdk.h"
#include "http_auth/authentication.h"
#include "usrm.h"
#include "ticker.h"
#include "base/ja_process.h"
#include "global_runtime.h"
#include "app_wifi.h"
#include "version.h"
#include "sock.h"

static const ST_HICHIP_MAP_STR_DEC hichip_wirelessMode_map[] = {
	{"none", NSDK_NETWORK_WIRELESS_MODE_NONE},
	{"accessPoint", NSDK_NETWORK_WIRELESS_MODE_ACCESSPOINT},
	{"stationMode", NSDK_NETWORK_WIRELESS_MODE_STATIONMODE},
	{"repeater", NSDK_NETWORK_WIRELESS_MODE_REPEATER},
};


static void hichip_delete_route(char* gateway)
{
	char cmd_str[128];

	sprintf(cmd_str, "route del default gw %s", gateway); 
	NK_SYSTEM(cmd_str);
}

static void hichip_make_stream_declaration(lpHICHIP_CONF_FUNC conf_func, char *buf)
{
	int i, ii, vin_count = 1 ,video_count = conf_func->get_video_count();
	char cam_buf[256], stream_buf[128], tmp_buf[128], tmp_out_buf[512];
	int vin_num[16], stream_num[16], stream_count[16];

	memset(vin_num, 0, sizeof(vin_num));
	memset(stream_num, 0, sizeof(stream_num));
	memset(stream_count, 0, sizeof(stream_count));
	memset(cam_buf, 0, sizeof(cam_buf));
	memset(stream_buf, 0, sizeof(stream_buf));
	memset(tmp_buf, 0, sizeof(tmp_buf));
	memset(tmp_out_buf, 0, sizeof(tmp_out_buf));

	for(i = 0; i < video_count; i++){
		conf_func->get_stream_name_by_index(i, tmp_buf);
		
		if(2 == sscanf(tmp_buf, "ch%d_%d.264", &vin_num[i], &stream_num[i])){
			if(0 == i){
				vin_count = 1;
			}else if(vin_num[i] == vin_num[i-1]){
			}else{
				vin_count++;
			}
			stream_count[vin_count-1]++;
		}
	}

	for(i = 0; i < vin_count; i++){
		snprintf(cam_buf, sizeof(cam_buf),
			"[cam%d]" kCRLF
			"id=%d" kCRLF
			"stream-count=%d" kCRLF,
			i+1, i+1, stream_count[i]);
		for(ii = 0; ii < stream_count[i];ii++){
			snprintf(tmp_buf, sizeof(tmp_buf), 
					"[cam%d-stream%d]" kCRLF
					"id=%d%d" kCRLF, 
					i+1, ii+1, i+1, ii+1);
			strncat(stream_buf, tmp_buf, sizeof(stream_buf));
		}
		strncat(cam_buf, stream_buf, sizeof(cam_buf));
		strncat(tmp_out_buf, cam_buf, sizeof(tmp_out_buf));
		memset(stream_buf, 0, sizeof(stream_buf));
		memset(cam_buf, 0, sizeof(cam_buf));
	}

	sprintf(buf,
			"[dev-media-info]" kCRLF
			"cam-count=%d" kCRLF
			"%s",
			vin_count, tmp_out_buf);
	//printf("%s\n", buf);
}

static int hichip_make_stream_declaration_json(lpHICHIP_CONF_FUNC conf_func, struct json_object *json)
{
	struct json_object *stream_array = NULL;
	struct json_object *channel_array = NULL;
	struct json_object *tmpStreamJson = NULL;
	struct json_object *tmpChannelJson = NULL;
	int i = 0, ii = 0, vin_count = 1 ,video_count = conf_func->get_video_count();
	char cam_buf[256], stream_buf[128], tmp_buf[128];
	int vin_num[16], stream_num[16], stream_count[16];
	ST_NSDK_VENC_CH venc_ch;

	if(!json){
		return -1;
	}

	memset(cam_buf, 0, sizeof(cam_buf));
	memset(stream_buf, 0, sizeof(stream_buf));
	memset(tmp_buf, 0, sizeof(tmp_buf));
	memset(vin_num, 0, sizeof(vin_num));
	memset(stream_num, 0, sizeof(stream_num));
	memset(stream_count, 0, sizeof(stream_count));

	for(i = 0; i < video_count; i++){
		conf_func->get_stream_name_by_index(i, tmp_buf);
		if(2 == sscanf(tmp_buf, "ch%d_%d.264", &vin_num[i], &stream_num[i])){
			if(0 == i){
				vin_count = 1;
			}else if(vin_num[i] == vin_num[i-1]){
			}else{
				vin_count++;
			}
			stream_count[vin_count-1]++;
		}
	}

	channel_array = json_object_new_array();
	for(i = 0; i < vin_count; i++){
		stream_array = json_object_new_array();
		tmpChannelJson = json_object_new_object();
		json_object_object_add(tmpChannelJson, "id", json_object_new_int(i + 1));
		json_object_object_add(tmpChannelJson, "Stream-Cnt", json_object_new_int(stream_count[i]));
		for(ii = 0; ii < stream_count[i]; ii++){
			NETSDK_conf_venc_ch_get((i+1)*100+ii+1, &venc_ch);
			tmpStreamJson = json_object_new_object();
			snprintf(tmp_buf, sizeof(tmp_buf), "%d%d", i + 1, ii + 1);
			json_object_object_add(tmpStreamJson, "id", json_object_new_int(atoi(tmp_buf)));
			snprintf(tmp_buf, sizeof(tmp_buf), "%dx%d", venc_ch.resolutionWidth, venc_ch.resolutionHeight);
			json_object_object_add(tmpStreamJson, "Resolution", json_object_new_string(tmp_buf));
			json_object_object_add(tmpStreamJson, "Bitrate", json_object_new_int(venc_ch.constantBitRate));
			json_object_object_add(tmpStreamJson, "Framerate", json_object_new_int(venc_ch.frameRate));
			json_object_array_add(stream_array, tmpStreamJson);
		}
		json_object_object_add(tmpChannelJson, "Stream-Info", stream_array);
		json_object_array_add(channel_array, tmpChannelJson);
	}

	json_object_object_add(json, "Channel-Cnt", json_object_new_int(vin_count));
	json_object_object_add(json, "Channel-Info", channel_array);

	return 0;
}

static int hichip_make_capabilities_declaration_json(lpHICHIP_CONF_FUNC conf_func, struct json_object *json)
{
	struct json_object *Capabilities = NULL;
	ST_NSDK_NETWORK_PORT port;
	if(!json){
		return -1;
	}
	NETSDK_conf_port_get(1, &port);

	Capabilities = json_object_new_object();
	if(Capabilities){
		json_object_object_add(Capabilities, "Http-Port", json_object_new_int(port.value));
		json_object_object_add(Capabilities, "MaxHardDiskDrivers", json_object_new_int(0));
		json_object_object_add(Capabilities, "MaxTFCards", json_object_new_int(0));
		json_object_object_add(json, "Capabilities", Capabilities);
	}

	return 0;
}

static int hichip_make_network_declaration_json(lpHICHIP_CONF_FUNC conf_func, struct json_object *json)
{
	const char *eth_name = NULL;
	ifconf_interface_t ifconf_irf;
	ST_NSDK_NETWORK_INTERFACE nsdk_interface;
	struct json_object *wired_array = NULL;
	struct json_object *wireless_array = NULL;
	struct json_object *wiredJson = NULL;
	struct json_object *wirelessJson = NULL;
	struct json_object *wirelessSTAJson = NULL;
	struct json_object *wirelessAPJson = NULL;

	if(!json){
		return -1;
	}

	wired_array = json_object_new_array();
	wireless_array = json_object_new_array();
	wiredJson = json_object_new_object();
	wirelessJson = json_object_new_object();
	wirelessSTAJson = json_object_new_object();
	wirelessAPJson = json_object_new_object();

#if defined(REPEATER)
	eth_name = NSDK_NETWORK_WIRELESS_MODE_REPEATER_ETH;
#else
	eth_name = NSDK_NETWORK_WIRED_ETH;
#endif
	ifconf_get_interface(eth_name, &ifconf_irf);
	NETSDK_conf_interface_get(1, &nsdk_interface);
	json_object_object_add(wiredJson, "DHCP", json_object_new_boolean(nsdk_interface.lan.addressingType == kNSDK_NETWORK_LAN_ADDRESSINGTYPE_DYNAMIC));
	json_object_object_add(wiredJson, "Connected", json_object_new_boolean(check_nic(eth_name) == 0));
	json_object_object_add(wiredJson, "IP", json_object_new_string(ifconf_ipv4_ntoa(ifconf_irf.ipaddr)));
	json_object_object_add(wiredJson, "Netmask", json_object_new_string(ifconf_ipv4_ntoa(ifconf_irf.netmask)));
	json_object_object_add(wiredJson, "Gateway", json_object_new_string(nsdk_interface.lan.staticGateway));
	json_object_object_add(wiredJson, "MAC", json_object_new_string(ifconf_hw_ntoa(ifconf_irf.hwaddr)));

	json_object_array_add(wired_array, wiredJson);

	json_object_object_add(json, "Wired", wired_array);

	if(APP_WIFI_model_exist()){
		NETSDK_conf_interface_get(4, &nsdk_interface);
#if defined(REPEATER)
		eth_name = NSDK_NETWORK_WIRELESS_MODE_REPEATER_ETH;
#else
		eth_name = NSDK_NETWORK_WIRELESS_MODE_STA_ETH;
		if(NSDK_NETWORK_WIRELESS_MODE_REPEATER == nsdk_interface.wireless.wirelessMode && APP_WIFI_If_Exist(NSDK_NETWORK_WIRELESS_MODE_REPEATER_AP_ETH)){
			eth_name = NSDK_NETWORK_WIRELESS_MODE_REPEATER_ETH;
		}
#endif
		ifconf_get_interface(eth_name, &ifconf_irf);
		json_object_object_add(wirelessJson, "DHCP", json_object_new_boolean(nsdk_interface.wireless.dhcpServer.dhcpAutoSettingEnabled));
		json_object_object_add(wirelessJson, "Connected", json_object_new_boolean(APP_WIFI_is_sta_connected()));
		json_object_object_add(wirelessJson, "IP", json_object_new_string(ifconf_ipv4_ntoa(ifconf_irf.ipaddr)));
		json_object_object_add(wirelessJson, "Netmask", json_object_new_string(ifconf_ipv4_ntoa(ifconf_irf.netmask)));
		json_object_object_add(wirelessJson, "Gateway", json_object_new_string(ifconf_ipv4_ntoa(ifconf_irf.gateway)));
		json_object_object_add(wirelessJson, "MAC", json_object_new_string(ifconf_hw_ntoa(ifconf_irf.hwaddr)));
		json_object_object_add(wirelessJson, "Mode", json_object_new_string(hichip_map_dec2str(hichip_wirelessMode_map, sizeof(hichip_wirelessMode_map)/sizeof(hichip_wirelessMode_map[0]), 
		nsdk_interface.wireless.wirelessMode, "stationMode")));

		json_object_object_add(wirelessSTAJson, "AP-Essid", json_object_new_string(nsdk_interface.wireless.wirelessStaMode.wirelessApEssId));
		json_object_object_add(wirelessSTAJson, "AP-Psk",  json_object_new_string(nsdk_interface.wireless.wirelessStaMode.wirelessApPsk));

		json_object_object_add(wirelessAPJson, "Channel", json_object_new_int(nsdk_interface.wireless.wirelessApMode.wirelessApMode80211nChannel));
		json_object_object_add(wirelessAPJson, "Essid",  json_object_new_string(nsdk_interface.wireless.wirelessApMode.wirelessEssId));
		json_object_object_add(wirelessAPJson, "Psk",  json_object_new_string(nsdk_interface.wireless.wirelessApMode.wirelessPsk));

		json_object_object_add(wirelessJson, "StaMode", wirelessSTAJson);
		json_object_object_add(wirelessJson, "ApMode", wirelessAPJson);

		json_object_array_add(wireless_array, wirelessJson);

		json_object_object_add(json, "Wireless", wireless_array);

		json_object_object_add(json, "Repeater-ID", json_object_new_string(nsdk_interface.wireless.repeaterDevId));
        //json_object_object_add(json, "Repeater-ID", json_object_new_string(""));
    }
	return 0;
}

int HICHIP_DISCOVER_sock_broadcast_create(char *bindip, int port, int bReuseAddr,int rwTimeoutMS)
{
	int ret;
    int on = 1;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in my_addr;
	struct timeval r_timeo = { rwTimeoutMS/1000, (rwTimeoutMS%1000)*1000 };
	struct timeval w_timeo = { rwTimeoutMS/1000, (rwTimeoutMS%1000)*1000 };

    if (sock <=0 ) {
        return -1;
    }

    // set addr reuse
    if (bReuseAddr != 0) {
	    on = 1;
	    ret=setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	    if(ret < 0) {
	        printf("SOCK-ERROR: set port reuse failed");
	        close(sock);
	        return -1;
	    }
    }

    on = 1;
    ret=setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&on, sizeof(on));
    if(ret < 0) {
        printf("SOCK-ERROR: set broadcast failed");
        close(sock);
        return -1;
    }

	if (rwTimeoutMS > 0) {
		ret=setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&w_timeo, sizeof(w_timeo));
		if(ret < 0) {
			printf("SOCK-ERROR: set send timeout failed.");
			close(sock);
			return -1;
		}

		ret=setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&r_timeo,sizeof(r_timeo));
		if(ret < 0) {
			printf("SOCK-ERROR: set receive timeout failed.");
			close(sock);
			return -1;
		}
	}

    //
    memset(&my_addr,0,sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons((unsigned short)port);
    my_addr.sin_addr.s_addr = bindip ? inet_addr(bindip) : INADDR_ANY;
    //bind
    ret = bind(sock, &my_addr, sizeof(my_addr));
    if(ret < 0) {
        printf("SOCK-ERROR: udp bind failed");
       	close(sock);
        return -1;
    }
    //printf("SOCK-DEBUG:create udp port:%d(sock:%d) ok.",port,sock);

    return sock;
}

int HICHIP_DISCOVER_sock_send_to(int sock,char *ip,int port,void *buf,unsigned int size, int flags)
{
	int ret;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    ret=sendto(sock,buf,size, flags, &addr,sizeof(struct sockaddr_in));
    if(ret != size) {
        printf("SOCK-ERROR: udp send to %s:%d failed,size:%d sock:%d buf:%p SOCK_ERR:%d\n",
               ip ? ip : "broadcast",port,size,sock,buf,errno);
        return -1;
    }
    //printf("SOCK-DEBUG: udp send to%s:%d uccess,size:%d\n",ip,port,size);
    return ret;
}

struct sockaddr_in HICHIP_DISCOVER_multicast_addr()
{
	int ret = 0;
	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(HICHIP_MULTICAST_PORT);
	ret = inet_aton(HICHIP_MULTICAST_IPADDR, &sock_addr.sin_addr);
	//APP_ASSERT(ret > 0, "HICHIP get multicast address error!");
	return sock_addr;
}

int HICHIP_DISCOVER_sock_create(const char *local_ip)
{
	int sock = 0;
	int ret = 0, flag = 0;
	
	struct sockaddr_in my_addr, to_addr;
	struct ip_mreq mreq;

	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	memset(&to_addr, 0, sizeof(struct sockaddr_in));
	memset(&mreq, 0, sizeof(struct ip_mreq));

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(HICHIP_MULTICAST_PORT);
	inet_aton(HICHIP_MULTICAST_IPADDR, &my_addr.sin_addr);

	// create udp socket
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock <= 0){
		APP_TRACE("HICHIP create udp socket failed!, errno %d", errno);
	}

	ret = bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr));
	if(0 != ret){
		APP_TRACE("HICHIP udp socket bind failed!, errno %d", errno);
	}

	inet_aton(HICHIP_MULTICAST_IPADDR, &mreq.imr_multiaddr);
	APP_TRACE("bind ip:%s", local_ip);
	mreq.imr_interface.s_addr = inet_addr(local_ip);//htonl(INADDR_ANY);
	//mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	ret = setsockopt(sock, SOL_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	if(0 != ret){
		APP_TRACE("HICHP add member failed!, errno %d", errno);
	}

	flag = 0; // not backrush 
	ret = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, sizeof(flag));
	if(0 != ret){
		APP_TRACE("HICHP set IP_MULTICAST_LOOP failed!, errno %d", errno);
	}

	ret = fcntl(sock, F_SETFL,O_NONBLOCK);
	//assert(0 == ret);

	return sock;
}

void HICHIP_DISCOVER_sock_release(int sock)
{
	close(sock);
}

int HICHIP_DISCOVER_sock_send(int sock, const void *buf, size_t buf_size)
{
	struct sockaddr_in peer_addr = HICHIP_DISCOVER_multicast_addr();
	return sendto(sock, buf, buf_size, 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr)); 
}

int HICHIP_DISCOVER_send(lpHICHIP_DISCOVER_RESOPNSE response, char *buf, int buf_len)
{
	int ret = -1;
	if(!response || !buf || buf_len <= 0){
		return ret;
	}

	if(response && response->sock > 0){
		switch(response->type){
			case HICHIP_DISCOVER_TYPE_MULTICAST:
				ret = HICHIP_DISCOVER_sock_send(response->sock, buf, buf_len);
				break;
			case HICHIP_DISCOVER_TYPE_BROADCAST:
				ret = HICHIP_DISCOVER_sock_send_to(response->sock, response->peer_ip, response->peer_port, buf, buf_len, 0);
				break;
			default:
				APP_TRACE("Error : Unknow response, buf :\n%s", buf);
				break;
		}
	}
	return ret;
}

static int hichip_discovery_send_401_digest(lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const char *nonce)
{
	LP_HTTP_HEAD_FIELD response_header = NULL;
	char response_buf[1024] = {""};
	char nonce_str[128] = {""};
	size_t response_len = 0;
	int ret;
	
	response_header = HTTP_UTIL_new_response_header(request_header->protocol,
		request_header->version, 401, NULL);
	response_header->add_tag_server(response_header, "IE/10.0");		
	response_header->add_tag_date(response_header, 0);
	response_header->add_tag_text(response_header, "Client-ID", request_header->read_tag(request_header, "Client-ID"), true);
	response_header->add_tag_text(response_header, "Device-ID", request_header->read_tag(request_header, "Device-ID"), true);
	response_header->add_tag_int(response_header, "Content-Length", 0, true);
	response_header->add_tag_text(response_header, "Connection", "close", true);
	sprintf(nonce_str, "Digest realm=\"IPCAM\" nonce=\"%s\"", nonce);
	response_header->add_tag_text(response_header, "WWW-Authenticate", nonce_str, true);
	response_len = response_header->to_text(response_header, response_buf, sizeof(response_buf));
	response_header->dump(response_header);
	response_header->free(response_header);
	response_header = NULL;

	ret = HICHIP_DISCOVER_send(response, response_buf, strlen(response_buf));
	if(ret < 0){
		// do something!
		printf("send 401 failed %d-%s!\r\n", errno, strerror(errno));
		usleep(5000);
	}
	return 0;
}


static bool hichip_discovery_check_auth(lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const char* nonce)
{
	bool ret = false;
	Authentication_t auth;
	USRM_I_KNOW_U_t usrm;
	HTTP_CSTR_t authorization = request_header->read_tag(request_header, "Authorization");
	auth.type = HTTP_AUTH_DIGEST;
	//fix default user
	if(USRM_check_user("admin", "") == USRM_GREAT){
		APP_TRACE("default user!");
		return true;
	}
	if(HTTP_AUTH_get_auth(&auth, authorization)){
		strcpy(auth.url, "www.dvr163.com");
		strcpy(auth.method, "SET");
		strcpy(auth.nonce, nonce);
		if(USRM_get_user(auth.user, &usrm) != NULL 
			&& (usrm.permit_flag&USRM_PERMIT_SETTING != 0)){//usr need setting permit flag
			strcpy(auth.pwd, usrm.password);
			if(HTTP_AUTH_validate2(&auth, authorization)){
				ret = true;
			}else{
				APP_TRACE("HTTP_AUTH_validate2 FAILED!");
			}
		}else{
			ret = false;
			APP_TRACE("USRM_get_user FAILED!:%s", auth.user);
		}
	}else{
		ret = false;
		APP_TRACE("HTTP_AUTH_get_auth FAILED!:%s", authorization);
	}
	if(ret == false){//failed to auth
		hichip_discovery_send_401_digest(response, request_header, nonce);
	}
	return ret;
}

static int hichip_process_cmd_ret_200(lpHICHIP_DISCOVER_RESOPNSE response, char *client_id, char *device_id, LP_HTTP_HEAD_FIELD request_header)
{
	int ret = 0;
	LP_HTTP_HEAD_FIELD response_header = HTTP_UTIL_new_response_header("MCTP", "1.0", 200, NULL);
	if(NULL != response_header){
		char response_buf[1024] = {""};
		const char *response_content = "Segment-Num:1" kCRLF
			"Segment-Seq:1" kCRLF
			"Data-Length:35" kCRLF
			kCRLF
			"[Success] set net information OK!" kCRLF;
		
		response_header->add_tag_int(response_header, "CSeq", atoi(request_header->read_tag(request_header, "CSeq")), true);
		response_header->add_tag_text(response_header, "Client-ID", client_id, true);
		response_header->add_tag_text(response_header, "Device-ID", device_id, true);
		response_header->add_tag_text(response_header, "Content-Type", "text/HDP", true);
		response_header->add_tag_int(response_header, "Content-Length", strlen(response_content), true);
		response_header->to_text(response_header, response_buf, sizeof(response_buf));
		//response_header->dump(response_header);
		response_header->free(response_header);
		response_header = NULL;
		// add content
		strcat(response_buf, response_content);
		//APP_TRACE("%s", response_buf);

		// response
		ret = HICHIP_DISCOVER_send(response, response_buf, strlen(response_buf));
		if(ret < 0){
			//return -1;
		}
	}
	return 0;
}

int HICHIP_DISCOVER_process_cmd(lpHICHIP_CONF_FUNC conf_func, lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const void *request_content)
{
	int ret = 0;

	// some tags of header
	HTTP_CSTR_t cseq = request_header->read_tag(request_header, "CSeq");
	HTTP_CSTR_t client_id = request_header->read_tag(request_header, "Client-ID");
	HTTP_CSTR_t device_id = request_header->read_tag(request_header, "Device-ID");
	HTTP_CSTR_t authorization = request_header->read_tag(request_header, "Authorization");
	HTTP_CSTR_t modify_type = request_header->read_tag(request_header, "X-Modify-Type");
	HTTP_CSTR_t virtualIP = request_header->read_tag(request_header, "Virtual-IP");
	char network_para[1024] = {""};
	int modifyType;

	bool lan_flag = false, dhcp_flag = true, dns_flag = false, port_flag = false;
	bool dns_stat_flag = false;

	const char *nonce = "";

	if(conf_func->nonce){
		nonce = conf_func->nonce();
	}

	if(hichip_discovery_check_auth(response, request_header, nonce) == false){
		return 0;
	}

	const char *set_item = NULL;
	const char *item_end = NULL;
	int str_len;

	if(NULL == modify_type){
		//NVR
		modifyType = 0;
	}else if(0 == strcmp(modify_type, "soft")){
		//DNVR
		modifyType = 1;
	}else if(0 == strcmp(modify_type, "WIFI")){
		//WIFI
		modifyType = 2;
	}else{
		//default for NVR
		modifyType = 0;
	}

	ST_NSDK_NETWORK_INTERFACE nsdk_interface;
	ST_NSDK_NETWORK_PORT port;
	switch(modifyType){
		default:
		case 0:{
			NETSDK_conf_interface_get(1, &nsdk_interface);
		}break;
		case 1:{
			NETSDK_conf_interface_get(2, &nsdk_interface);
		}break;
		case 2:{
			NETSDK_conf_interface_get(4, &nsdk_interface);
		}break;
	}

	//add a virtual ip for test
	ifconf_interface_t intrface;
	memset(&intrface, 0, sizeof(ifconf_interface_t));

	if(virtualIP){
		hichip_process_cmd_ret_200(response, client_id, device_id, request_header);
		if(modifyType == 0){
			NETSDK_tmp_interface_get(1, &nsdk_interface);
			snprintf(nsdk_interface.lan.staticIP,sizeof(nsdk_interface.lan.staticIP), virtualIP);
			NETSDK_tmp_interface_set(1, &nsdk_interface, eNSDK_CONF_SAVE_RESTART);
		}
		else if(modifyType == 2){
			NETSDK_tmp_interface_get(4, &nsdk_interface);
			snprintf(nsdk_interface.lan.staticIP,sizeof(nsdk_interface.lan.staticIP), virtualIP);
			NETSDK_tmp_interface_set(4, &nsdk_interface, eNSDK_CONF_SAVE_RESTART);
		}
		return 0;
	}
	
	if(1 == sscanf(request_content, "%[^" kCRLF "]", network_para) && strlen(network_para) > 0){
		
		APP_TRACE("HICHIP X-Modify-Type: %s", modify_type);
		
		// part 1.
		//netconf set -ipaddr 192.168.1.7 -netmask 255.255.255.0 -gateway 192.168.1.1 -dhcp off -fdnsip 192.168.1.2 -sdnsip 211.23.12.13 -dnsstat 0 -hwaddr 00:01:89:11:11:07.
		// part 2.
		//httpport set -httpport 8000.
		
		if(0 == strncasecmp(network_para, "netconf set", strlen("netconf set"))){
			APP_TRACE("HiChip NetConf set: %s", network_para);
			ifconf_interface_t ifc;
			ifconf_dns_t dns;
			
			memset(&ifc, 0, sizeof(ifc));
			memset(&dns, 0, sizeof(dns));

			// check ip address
			set_item = strstr(network_para, "-ipaddr");
			
			if(NULL != set_item){
				set_item += strlen("-ipaddr") + 1; // offset to value
				item_end = strstr(set_item, " ");
				if(item_end != NULL){
					str_len = item_end - set_item;
				}else{
					str_len = strlen(set_item);
				}
				memset(nsdk_interface.lan.staticIP, 0, sizeof(nsdk_interface.lan.staticIP));
				strncpy(nsdk_interface.lan.staticIP, set_item, str_len);
				APP_TRACE("setup ipaddr:%s", nsdk_interface.lan.staticIP);
				lan_flag = true;
			}
			
			// check net mask address
			set_item = strstr(network_para, "-netmask");
			if(NULL != set_item){
				set_item += strlen("-netmask") + 1; // offset to value
				item_end = strstr(set_item, " ");
				if(item_end != NULL){
					str_len = item_end - set_item;
				}else{
					str_len = strlen(set_item);
				}
				memset(nsdk_interface.lan.staticNetmask, 0, sizeof(nsdk_interface.lan.staticNetmask));
				strncpy(nsdk_interface.lan.staticNetmask, set_item, str_len);
				APP_TRACE("setup netmask:%s", nsdk_interface.lan.staticNetmask);
				lan_flag = true;
			}
			// check gateway
			set_item = strstr(network_para, "-gateway");
			if(NULL != set_item){
				set_item += strlen("-gateway") + 1;
				item_end = strstr(set_item, " ");
				if(item_end != NULL){
					str_len = item_end - set_item;
				}else{
					str_len = strlen(set_item);
				}
				if(0 == modifyType){//for nvr
					hichip_delete_route(nsdk_interface.lan.staticGateway);	
				}
				memset(nsdk_interface.lan.staticGateway, 0, sizeof(nsdk_interface.lan.staticGateway));
				strncpy(nsdk_interface.lan.staticGateway, set_item, str_len);
				APP_TRACE("setup gateway:%s", nsdk_interface.lan.staticGateway);
				lan_flag = true;
			}
			// check dhcp
			set_item = strstr(network_para, "-dhcp");
			if(NULL != set_item){
				set_item += strlen("-dhcp") + 1;
				if(0 == strcasecmp("off", set_item)){
					dhcp_flag = false;
                    nsdk_interface.lan.addressingType = 0;
				}else{
					dhcp_flag = true;
                    nsdk_interface.lan.addressingType = 1;
				}
				// FIXME: ignore this dhchp temprory
			}
			// check preferred dns address
			set_item = strstr(network_para, "-fdnsip");
			if(NULL != set_item){
				set_item += strlen("-fdnsip") + 1;
				dns.preferred = ifconf_ipv4_aton(set_item);
				APP_TRACE("setup fdnsip %s", ifconf_ipv4_ntoa(dns.preferred));
				dns_flag = true;
			}
			// check dns
			set_item = strstr(network_para, "-sdnsip");
			if(NULL != set_item){
				set_item += strlen("-sdnsip") + 1;
				dns.alternate = ifconf_ipv4_aton(set_item);
				APP_TRACE("setup sdnsip %s", ifconf_ipv4_ntoa(dns.alternate));
				dns_flag = true;
			}
			set_item = strstr(network_para, "-dnsstat");
			if(NULL != set_item){
				set_item += strlen("-dnsstat") + 1;
				if(0 == strcasecmp("off", set_item)){
					dns_stat_flag = false;
				}else{
					dns_stat_flag = true;
				}
			}
			// check MAC address
			set_item = strstr(network_para, "-hwaddr");
			if(NULL != set_item){
				set_item += strlen("-hwaddr") + 1;
			}
		}
		set_item = NULL;
		set_item = strstr(request_content, "-httpport");
		if(NULL != set_item && modifyType == 0){
			int modifyPort = 80;
			set_item += strlen("-httpport") + 1;
			modifyPort = atoi(set_item);
			NETSDK_conf_port_get(1, &port);
			if(port.value != modifyPort){
				port.value = modifyPort;
				APP_TRACE("setup httpport = %d", port.value);
				port_flag = true;
			}
			else{
				APP_TRACE("setup httpport = %d, but same as before", modifyPort);
			}
		}
		//check wireless mode
		set_item = strstr(network_para, "-wireless");
		if(NULL != set_item){
			set_item += strlen("-wireless") + 1; // offset to value
			item_end = strstr(set_item, " ");
			if(item_end != NULL){
				str_len = item_end - set_item;
			}else{
				str_len = strlen(set_item);
			}
			char wirelessmode_str[32];
			memset(wirelessmode_str, 0, sizeof(wirelessmode_str));
			strncpy(wirelessmode_str, set_item, str_len);
			nsdk_interface.wireless.wirelessMode = hichip_map_str2dec(hichip_wirelessMode_map, sizeof(hichip_wirelessMode_map)/sizeof(hichip_wirelessMode_map[0]), 
				wirelessmode_str, "stationMode");
			APP_TRACE("setup wirelessMode:%s", wirelessmode_str);
			lan_flag = true;
		}
		//check wirelessApEssId
		set_item = strstr(network_para, "-wirelessApEssId");
		if(NULL != set_item){
			set_item += strlen("-wirelessApEssId") + 1; // offset to value
			item_end = strstr(set_item, " ");
			if(item_end != NULL){
				str_len = item_end - set_item;
			}else{
				str_len = strlen(set_item);
			}
			memset(nsdk_interface.wireless.wirelessStaMode.wirelessApEssId, 0, sizeof(nsdk_interface.wireless.wirelessStaMode.wirelessApEssId));
			strncpy(nsdk_interface.wireless.wirelessStaMode.wirelessApEssId, set_item, str_len);
			APP_TRACE("setup wirelessApEssId:%s", nsdk_interface.wireless.wirelessStaMode.wirelessApEssId);
			lan_flag = true;
		}
		//check wirelessApPsk
		set_item = strstr(network_para, "-wirelessApPsk");
		if(NULL != set_item){
			set_item += strlen("-wirelessApPsk") + 1; // offset to value
			item_end = strstr(set_item, " ");
			if(item_end != NULL){
				str_len = item_end - set_item;
			}else{
				str_len = strlen(set_item);
			}
			memset(nsdk_interface.wireless.wirelessStaMode.wirelessApPsk, 0, sizeof(nsdk_interface.wireless.wirelessStaMode.wirelessApPsk));
			strncpy(nsdk_interface.wireless.wirelessStaMode.wirelessApPsk, set_item, str_len);
			APP_TRACE("setup wirelessApPsk:%s", nsdk_interface.wireless.wirelessStaMode.wirelessApPsk);
			lan_flag = true;
		}
		//check wirelessStationDhcp
		set_item = strstr(network_para, "-wirelessStationDhcp");
		if(NULL != set_item){
			set_item += strlen("-wirelessStationDhcp") + 1; // offset to value
			if(!strncmp(set_item, "true", strlen("true"))){
				//open dhcp
				nsdk_interface.wireless.dhcpServer.dhcpAutoSettingEnabled = 1;
			}else{
				nsdk_interface.wireless.dhcpServer.dhcpAutoSettingEnabled = 0;
			}
			//nsdk_interface.wireless.dhcpServer.dhcpAutoSettingEnabled = 0;
			APP_TRACE("setup wirelessStationDhcp:%d-%s", nsdk_interface.wireless.dhcpServer.dhcpAutoSettingEnabled, set_item);
			lan_flag = true;
		}
		//check repeaterDevId
		set_item = strstr(network_para, "-repeaterDevId");
		if(NULL != set_item){
			set_item += strlen("-repeaterDevId") + 1; // offset to value
			item_end = strstr(set_item, " ");
			if(item_end != NULL){
				str_len = item_end - set_item;
			}else{
				str_len = strlen(set_item);
			}
			memset(nsdk_interface.wireless.repeaterDevId, 0, sizeof(nsdk_interface.wireless.repeaterDevId));
			strncpy(nsdk_interface.wireless.repeaterDevId, set_item, str_len);
			if(strcmp(nsdk_interface.wireless.repeaterDevId,"none") == 0){
				memset(nsdk_interface.wireless.repeaterDevId, 0, sizeof(nsdk_interface.wireless.repeaterDevId));
				nsdk_interface.lan.addressingType = kNSDK_NETWORK_LAN_ADDRESSINGTYPE_STATIC;
				if(conf_func->turn_off_onvif){
					conf_func->turn_off_onvif();
				}
			}else if (strlen(nsdk_interface.wireless.repeaterDevId) > 0){
				nsdk_interface.lan.addressingType = kNSDK_NETWORK_LAN_ADDRESSINGTYPE_DYNAMIC;
				if(conf_func->turn_on_onvif){
					conf_func->turn_on_onvif();
				}
				if(conf_func->ip_adapt_pause){
					conf_func->ip_adapt_pause(0);
				}
			}

			APP_TRACE("setup repeaterDevId:%s", nsdk_interface.wireless.repeaterDevId);
			lan_flag = true;
		}
		else if(modifyType == 2 && strncmp(request_header->version, "1.0", strlen("1.0")) == 0){
			memset(nsdk_interface.wireless.repeaterDevId, 0, sizeof(nsdk_interface.wireless.repeaterDevId));
		}
	}

	if(lan_flag){
			hichip_process_cmd_ret_200(response, client_id, device_id, request_header);

			// save this parameter
			APP_TRACE("SET PARAM:%s", modify_type);
			ST_NSDK_IMAGE image;
			switch(modifyType){
				default:
				case 0:{
					NETSDK_conf_interface_set(1, &nsdk_interface, eNSDK_CONF_SAVE_RESTART);
					if(port_flag){
						NETSDK_conf_port_set(1, &port, eNSDK_CONF_SAVE_REBOOT);
					}
				}break;
				case 1:{
					NETSDK_conf_interface_set(2, &nsdk_interface, eNSDK_CONF_SAVE_RESTART);
				}break;
				case 2:{	
					
					if(0x0 == g_encryp_chip_odm1 && 0x0 == g_encryp_chip_odm2){
						//中性
						NETSDK_conf_interface_set(4, &nsdk_interface, eNSDK_CONF_SAVE_RESTART);
						
#if 0
						network_conf_save(5);//in order to reboot

#endif

					}else{
						//fix me:ODM贴片客户不允许无线模块进行有线对码
					}
				}break;
			}
			//sleep(4);
			//exit(0);
	}
	return 0;
}

int HICHIP_DISCOVER_search_v1_0(lpHICHIP_CONF_FUNC conf_func, lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const void *request_content, char *response_buf, int bufferlen)
{
	int ret = 0;
	char str_seg[2048] = {0};
	char str_seg1_data[768] = {0};
	char str_seg1[1024] = {0};
	char str_seg2_data[768] = {0};
	char str_seg2[1024] = {0};
    char sw_version[64] = {0};

	// some parameters
	ESEE_CLIENT_INFO_t ret_esee_info;
	SYSCONF_t* const sysconf = SYSCONF_dup();

	ifconf_interface_t ifconf_irf;
	const char *eth_name = NULL;

	const char *cseq = NULL, *client_id = NULL, *search_type = NULL, *search_disable_time = NULL;
	bool search_from_dnvr = false;
	int searchType; //0:NVR 1:DNVR 2:WIFI
	int searchDisableTime = 10;
	LP_HTTP_HEAD_FIELD response_header = NULL;

	const char *device_id = "", *device_model = "", *device_name = "", *nonce = "";
    snprintf(sw_version, sizeof(sw_version), "%s", sysconf->ipcam.info.software_version);
	
	if(conf_func->device_id){
		device_id = conf_func->device_id();
	}
	if(conf_func->device_model){
		device_model = conf_func->device_model();
	}
	if(conf_func->device_name){
		device_name = conf_func->device_name();
	}
	if(conf_func->nonce){
		nonce = conf_func->nonce();
	}
	
	// some tags of header
	cseq = request_header->read_tag(request_header, "CSeq");
	client_id = request_header->read_tag(request_header, "Client-ID");
	search_type = request_header->read_tag(request_header, "X-Search-Type");
	search_disable_time = request_header->read_tag(request_header, "X-Search-Disable-Time");
	//APP_TRACE("X-Search-Type: %s", search_type);

	//search_from_dnvr = (NULL != search_type && 0 == strcmp(search_type, "DNVR")) ? true : false;
	if(NULL == search_type){
		//NVR
		searchType = 0;
	}else if(0 == strcmp(search_type, "DNVR")){
		//DNVR
		searchType = 1;
	}else if(0 == strcmp(search_type, "WIFI")){
		//WIFI
		searchType = 2;
	}else{
		//default for NVR
		searchType = 0;
	}

	if(NULL != search_disable_time){
		searchDisableTime = atoi(search_disable_time);
    }
    if(NULL != conf_func->ip_adapt_pause){
		conf_func->ip_adapt_pause(5 > searchDisableTime ? 5 : 120 < searchDisableTime ? 120 : searchDisableTime);
	}
	
	ST_NSDK_NETWORK_INTERFACE nsdk_interface;
	memset(&nsdk_interface, 0, sizeof(nsdk_interface));
	switch(searchType){
		default:
		case 0:{
			eth_name = conf_func->ether_lan();
			NETSDK_conf_interface_get(1, &nsdk_interface);
		}break;
		case 1:{
			eth_name = conf_func->ether_vlan();
			NETSDK_conf_interface_get(2, &nsdk_interface);
		}break;
		case 2:{
			eth_name = NSDK_NETWORK_WIRELESS_MODE_STA_ETH;//conf_func->ether_lan();
			NETSDK_conf_interface_get(4, &nsdk_interface);
        	if(NSDK_NETWORK_WIRELESS_MODE_REPEATER == nsdk_interface.wireless.wirelessMode && APP_WIFI_If_Exist(NSDK_NETWORK_WIRELESS_MODE_REPEATER_AP_ETH)){
				eth_name = NSDK_NETWORK_WIRELESS_MODE_REPEATER_ETH;
			}
		}break;
	}
	ret = ifconf_get_interface(eth_name, &ifconf_irf);

	ST_NSDK_NETWORK_PORT port;
	NETSDK_conf_port_get(1, &port);

	char netmask[16];
	sprintf(netmask, "%s", ifconf_ipv4_ntoa(ifconf_irf.netmask));
	
	sprintf(str_seg1_data,
		"Device-ID=%s" kCRLF // device_id
		"Device-Model=%s" kCRLF
		"Device-Name=%s" kCRLF
		"Esee-ID=%s" kCRLF
		"Channel-Cnt=1" kCRLF
		"IP=%s" kCRLF // ipaddr
		"MASK=%s" kCRLF //netmask
		"MAC=%02X:%02X:%02X:%02X:%02X:%02X" kCRLF // hwaddr
		"Gateway=%s" kCRLF // gateway
		"Software-Version=%s" kCRLF
		"Http-Port=%d" kCRLF // port
		"Dhcp=%d" kCRLF
		"Ddns=0" kCRLF
		"Fdns=%s" kCRLF // preferred_dns
		"Sdns=%s" kCRLF // alternate_dns
		"DDNS-Enable=0" kCRLF
		"DDNS-User=" kCRLF
		"DDNS-Passwd=" kCRLF
		"DDNS-Host=" kCRLF
		"DDNS-Port=" kCRLF
		"Interface=%s" kCRLF
		"wireless=%s" kCRLF
		"wirelessApEssId=%s" kCRLF
		"wirelessApPsk=%s" kCRLF
		"wirelessStationDhcp=%s" kCRLF
		"nonce=%s" kCRLF,
		device_id,
		device_model,
		device_name,
		g_esee_id,
		ifconf_ipv4_ntoa(ifconf_irf.ipaddr),//nsdk_interface.lan.staticIP,
		netmask,//nsdk_interface.lan.staticNetmask
		ifconf_irf.hwaddr.s_b[0], ifconf_irf.hwaddr.s_b[1], ifconf_irf.hwaddr.s_b[2],
		ifconf_irf.hwaddr.s_b[3], ifconf_irf.hwaddr.s_b[4], ifconf_irf.hwaddr.s_b[5],
		nsdk_interface.lan.staticGateway,
		sw_version,
		port.value,
		nsdk_interface.lan.addressingType,
		nsdk_interface.dns.staticPreferredDns,
		nsdk_interface.dns.staticAlternateDns,
		conf_func->ether_lan(),
		hichip_map_dec2str(hichip_wirelessMode_map, sizeof(hichip_wirelessMode_map)/sizeof(hichip_wirelessMode_map[0]), 
			nsdk_interface.wireless.wirelessMode, "stationMode"),
		nsdk_interface.wireless.wirelessStaMode.wirelessApEssId,
		nsdk_interface.wireless.wirelessStaMode.wirelessApPsk,
		nsdk_interface.wireless.dhcpServer.dhcpAutoSettingEnabled ? "true":"false",
		nonce);

	sprintf(str_seg1,
		"Segment-Seq:1" kCRLF
		"Data-Length:%d" kCRLF
		"" kCRLF
		"%s",
		strlen(str_seg1_data),
		str_seg1_data);

	hichip_make_stream_declaration(conf_func, str_seg2_data);

	sprintf(str_seg2,
		"Segment-Seq:2" kCRLF
		"Data-Length:%d" kCRLF
		"" kCRLF
		"%s",
		strlen(str_seg2_data),
		str_seg2_data);

	sprintf(str_seg,
		"Segment-Num:2" kCRLF
		"%s"
		"%s",
		str_seg1,
		str_seg2);

	response_header = HTTP_UTIL_new_response_header("HDS", "1.0", 200, NULL);
	if(NULL != response_header){
		response_header->add_tag_int(response_header, "CSeq", atoi(cseq), true);
		response_header->add_tag_text(response_header, "Client-ID", (char*)client_id, true);
		response_header->add_tag_text(response_header, "Content-Type", "text/HDP", true);
		response_header->add_tag_int(response_header, "Content-Length", strlen(str_seg), true);
		response_header->to_text(response_header, response_buf, bufferlen);
		//response_header->dump(response_header);
		response_header->free(response_header);
		response_header = NULL;

		// catch the content
		strcat(response_buf, str_seg);
		//APP_TRACE("%s", response_buf);
		return 0;
	}
	
	return -1;
}

/*
	checksum is : md5("++session++session_id++content++content++")
*/
int HICHIP_DISCOVER_Cal_CheckSum(char *ret_checksum, int ret_size,char *session_id, char *content, int content_len)
{
	char sumbuffer[8 * 1024];
	int buffer_len = 0;
	if(!session_id || !ret_checksum){
		return -1;
	}
	memset(sumbuffer, 0, sizeof(sumbuffer));
	snprintf(sumbuffer, sizeof(sumbuffer), "++session++%s++content++", session_id);
	buffer_len = strlen(sumbuffer);
	if(content_len && content_len > 0){
		if(content_len + buffer_len  + strlen("++") <= sizeof(sumbuffer)){
			memcpy(sumbuffer + buffer_len, content, content_len);
			buffer_len += content_len;
		}
		else{
			printf("HICHIP_DISCOVER_Cal_CheckSum buffer not enough !\n");
			return -1;
		}
	}
	memcpy(sumbuffer + buffer_len, "++", strlen("++"));
	buffer_len += strlen("++");

	snprintf(ret_checksum, ret_size, "%s", md5sum_buffer(sumbuffer, buffer_len));
	return 0;
}

int HICHIP_DISCOVER_search_v1_1(lpHICHIP_CONF_FUNC conf_func, lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const void *request_content, char *response_buf, int bufferlen)
{
	int ret = 0;
	SYSCONF_t* const sysconf = SYSCONF_dup();
	const char *client_id = NULL, *cseq = NULL, *device_id = NULL, *checksum = NULL, *session_id = NULL, *content_len = NULL,  *search_disable_time = NULL;
	LP_HTTP_HEAD_FIELD response_header = NULL;
	char repsone_header[512];
	char repsone_content[4096];
	char cal_checksum[128];
	char *_request_content = (char *)request_content;
	int searchDisableTime = 10;

	struct json_object *json = NULL;
	
	char sw_version[64] = {0};
	snprintf(sw_version, sizeof(sw_version), "%s", sysconf->ipcam.info.software_version);

	// some tags of header
	cseq = request_header->read_tag(request_header, "CSeq");
	client_id = request_header->read_tag(request_header, "Client-ID");
	device_id = request_header->read_tag(request_header, "Device-ID");
	checksum = request_header->read_tag(request_header, "X-Content-Checksum");
	session_id = request_header->read_tag(request_header, "X-Session-Id");
	content_len = request_header->read_tag(request_header, "Content-Length");
	search_disable_time = request_header->read_tag(request_header, "X-Search-Disable-Time");
	
	ret = content_len ? atoi(content_len) : 0;

	//match device id, for single search
	if(NULL != device_id && conf_func && conf_func->device_id){
		if(0 != strcmp(conf_func->device_id(), device_id)){
			printf("single search pack not match, drop. \n",device_id);
			return 0;
		}
		printf("single search pack match. \n");
	}

	if(!checksum || !session_id){
		printf("search without checksum, drop. \n");
		return -1;
	}

	//Check request checksum
	ret = HICHIP_DISCOVER_Cal_CheckSum(cal_checksum, sizeof(cal_checksum), session_id, _request_content, ret);
	if(ret < 0 || strcmp(checksum, cal_checksum) != 0){
		printf("search checksum not match %s (%s), drop. \n", checksum, cal_checksum);
		return -1;
	}

	if(NULL != search_disable_time){
		searchDisableTime = atoi(search_disable_time);
	}

	if(NULL != conf_func->ip_adapt_pause){
		conf_func->ip_adapt_pause(5 > searchDisableTime ? 5 : 120 < searchDisableTime ? 120 : searchDisableTime);
	}

	json = json_object_new_object();

	json_object_object_add(json, "Ver", json_object_new_string(request_header->version));
	json_object_object_add(json, "Nonce", json_object_new_string(conf_func->nonce()));
	json_object_object_add(json, "Device-ID", json_object_new_string(conf_func->device_id()));
	json_object_object_add(json, "Device-Model", json_object_new_string(conf_func->device_model()));
#if defined(REPEATER)	
	json_object_object_add(json, "Device-Type", json_object_new_string("REPEATER"));
#else
	json_object_object_add(json, "Device-Type", json_object_new_string("IPCAM"));
#endif
	json_object_object_add(json, "Esee-ID", json_object_new_string(g_esee_id));
	json_object_object_add(json, "Software-Version", json_object_new_string(sw_version));	
	hichip_make_network_declaration_json(conf_func, json);

#if !defined(REPEATER)
	hichip_make_stream_declaration_json(conf_func, json);
#endif
	hichip_make_capabilities_declaration_json(conf_func, json);

	snprintf(repsone_content, sizeof(repsone_content), "%s", json_object_to_json_string(json));
	json_object_put(json);
	json = NULL;

	//cal respone checksum
	ret = HICHIP_DISCOVER_Cal_CheckSum(cal_checksum, sizeof(cal_checksum), session_id, repsone_content, strlen(repsone_content));

	response_header = HTTP_UTIL_new_response_header("HDS", request_header->version, 200, NULL);

	if(NULL != response_header){
		response_header->add_tag_int(response_header, "CSeq", atoi(cseq), true);
		response_header->add_tag_text(response_header, "Client-ID", (char*)client_id, true);
		response_header->add_tag_text(response_header, "Content-Type", "application/json", true);
		response_header->add_tag_int(response_header, "Content-Length", strlen(repsone_content), true);
		response_header->add_tag_text(response_header, "X-Session-Id", session_id, true);
		response_header->add_tag_text(response_header, "X-Content-Checksum", cal_checksum, true);
		response_header->to_text(response_header, repsone_header, sizeof(repsone_header));
		response_header->free(response_header);
		response_header = NULL;

		snprintf(response_buf, bufferlen, "%s%s", repsone_header, repsone_content);

		return 0;
	}
	return -1;
}

int HICHIP_DISCOVER_process_search(lpHICHIP_CONF_FUNC conf_func, lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const void *request_content)
{
	int ret = 0;
	char response_buf[2048] = {""};
	if(strncmp(request_header->version, "1.0", strlen("1.0")) == 0){
#if defined(REPEATER)
		return 0;
#else
		ret = HICHIP_DISCOVER_search_v1_0(conf_func, response, request_header, request_content, response_buf, sizeof(response_buf));
#endif
	}
	else if(strncmp(request_header->version, "1.1", strlen("1.1")) == 0){
		ret = HICHIP_DISCOVER_search_v1_1(conf_func, response, request_header, request_content, response_buf, sizeof(response_buf));
	}
	else{
		return 0;
	}

	// response the search request
	ret = HICHIP_DISCOVER_send(response, response_buf, strlen(response_buf));
	if(ret < 0){
		return -1;
	}

	return 0;
}

int HICHIP_DISCOVER_process_gb28181(lpHICHIP_CONF_FUNC conf_func, lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const void *request_content)
{
	int ret = 0;
	//SYSCONF_t* const sysconf = SYSCONF_dup();

	const char *cseq = NULL, *client_id = NULL;
	char response_buf[2048] = {""};
	LP_HTTP_HEAD_FIELD response_header = NULL;

	//printf("recv_conf_content:%s\n",(char *)request_content);
	if(conf_func->gb28181_conf){
		conf_func->gb28181_conf(request_content);
	}
	
	// some tags of header
	cseq = request_header->read_tag(request_header, "CSeq");
	client_id = request_header->read_tag(request_header, "Client-ID");

	response_header = HTTP_UTIL_new_response_header("DMCBG", "1.0", 200, NULL);
	if(NULL != response_header){
		response_header->add_tag_int(response_header, "CSeq", atoi(cseq), true);
		response_header->add_tag_text(response_header, "Client-ID", (char*)client_id, true);
		response_header->to_text(response_header, response_buf, sizeof(response_buf));
		//response_header->dump(response_header);
		response_header->free(response_header);
		response_header = NULL;
		printf("response:%s.\n",response_buf);
		// response the search request
		ret = HICHIP_DISCOVER_send(response, response_buf, strlen(response_buf));
		if(ret < 0){
			return -1;
		}

		return 0;
	}
	
	return -1;

}


#define HICHIP_DISCOVER_PING_PACK "{\"Ver\":\"1.1\",\"IP\":\"%s\"}"
int HICHIP_DISCOVER_Ping(lpHICHIP_CONF_FUNC conf_func)
{
	static int session_id_i = 0;
	LP_HTTP_HEAD_FIELD snd_header = NULL;
	char *eth_name = NULL;
	char cal_checksum[128], session_id[64], buf[1024], content[512];
	ifconf_interface_t ifconf_irf;
	int broadcast_sock = -1;

	eth_name = conf_func->ether_lan();
	ifconf_get_interface(eth_name, &ifconf_irf);
	broadcast_sock = HICHIP_DISCOVER_sock_broadcast_create(ifconf_ipv4_ntoa(ifconf_irf.ipaddr), 0, 0, 2000);

	if(broadcast_sock > 0){
		memset(buf, 0, sizeof(buf));
		memset(content, 0, sizeof(content));

		snprintf(content, sizeof(content), HICHIP_DISCOVER_PING_PACK, ifconf_ipv4_ntoa(ifconf_irf.ipaddr));
		snprintf(session_id, sizeof(session_id), "%d", session_id_i);
		HICHIP_DISCOVER_Cal_CheckSum(cal_checksum, sizeof(cal_checksum), session_id, content, strlen(content));
		snd_header = HTTP_UTIL_new_request_header("HDS", "1.1", "PING", "*", NULL);
		if(snd_header){
			snd_header->add_tag_text(snd_header, "CSeq", session_id, true);
			snd_header->add_tag_text(snd_header, "Client-ID", conf_func->device_id(), true);
			snd_header->add_tag_text(snd_header, "X-Session-Id", session_id, true);
			snd_header->add_tag_text(snd_header, "X-Content-Checksum", cal_checksum, true);
			snd_header->add_tag_text(snd_header, "Content-Type", "application/json", true);
			snd_header->add_tag_int(snd_header, "Content-Length", strlen(content), true);
			snd_header->to_text(snd_header, buf, sizeof(buf));
			snd_header->free(snd_header);
			snd_header = NULL;
			strncpy(buf + strlen(buf), content, sizeof(buf) - strlen(buf) - 1);
			HICHIP_DISCOVER_sock_send_to(broadcast_sock, NULL, HICHIP_BROADCAST_PORT, buf, strlen(buf), 0);
		}
		SOCK_close(broadcast_sock);
	}
	session_id_i++;

	return 0;
}

#define HICHIP_DISCOVER_PONG_PACK "{\"Ver\":\"1.1\"}"
int HICHIP_DISCOVER_Pong(lpHICHIP_CONF_FUNC conf_func, lpHICHIP_DISCOVER_RESOPNSE response, LP_HTTP_HEAD_FIELD request_header, const void *request_content)
{
#if !defined(REPEATER)
	int ret = 0, len = 0, header_size = 0;
	char buf[1024];
	LP_HTTP_HEAD_FIELD snd_header;
	char *http_content = NULL, *checksum = NULL, *session_id = NULL, *content_len = NULL, *device_id = NULL;
	char cal_checksum[128];
	ST_NSDK_NETWORK_INTERFACE nsdk_interface;

	if(response->type == HICHIP_DISCOVER_TYPE_MULTICAST){
		return -1;
	}

	if(strncmp(request_header->version, "1.1", strlen("1.1")) != 0){
		return -1;
	}

	checksum = request_header->read_tag(request_header, "X-Content-Checksum");
	session_id = request_header->read_tag(request_header, "X-Session-Id");
	content_len = request_header->read_tag(request_header, "Content-Length");
	device_id = (char*)request_header->read_tag(request_header, "Client-ID");
	ret = content_len ? atoi(content_len) : 0;

	if(!checksum || !session_id || !device_id){
		printf("PING without checksum, drop. \n");
		return -1;
	}

	NETSDK_conf_interface_get(4, &nsdk_interface);
	if(strcmp(device_id, nsdk_interface.wireless.repeaterDevId) != 0){
		//printf("PING device_id not match , [%s]-->[%s], drop\n", device_id, nsdk_interface.wireless.repeaterDevId);
		return -1;
	}

	//Check request checksum
	ret = HICHIP_DISCOVER_Cal_CheckSum(cal_checksum, sizeof(cal_checksum), session_id, request_content, ret);
	if(ret < 0 || strcmp(checksum, cal_checksum) != 0){
		printf("PING checksum not match %s (%s), drop. \n", checksum, cal_checksum);
		return -1;
	}

	if(strstr(request_content, "\"Ver\":\"1.1\"") == NULL){
		return -1;
	}

	if(strstr(request_content, response->peer_ip) == NULL){
		return -1;
	}

	snd_header = HTTP_UTIL_new_response_header("HDS", "1.1", 100, "PONG");
	if(snd_header){
		HICHIP_DISCOVER_Cal_CheckSum(cal_checksum, sizeof(cal_checksum), session_id, HICHIP_DISCOVER_PONG_PACK, strlen(HICHIP_DISCOVER_PONG_PACK));
		snd_header->add_tag_text(snd_header, "Client-ID", device_id, true);
		snd_header->add_tag_text(snd_header, "Device-ID", conf_func->device_id(), true);
		snd_header->add_tag_text(snd_header, "CSeq", session_id, true);
		snd_header->add_tag_text(snd_header, "X-Session-Id", session_id, true);
		snd_header->add_tag_text(snd_header, "X-Content-Checksum", cal_checksum, true);
		snd_header->add_tag_text(snd_header, "Content-Type", "application/json", true);
		snd_header->add_tag_int(snd_header, "Content-Length", strlen(HICHIP_DISCOVER_PONG_PACK), true);
		snd_header->to_text(snd_header, buf, sizeof(buf));
		snd_header->free(snd_header);
		snd_header = NULL;
		strncpy(buf + strlen(buf), HICHIP_DISCOVER_PONG_PACK, sizeof(buf) - strlen(buf) - 1);

		HICHIP_DISCOVER_send(response, buf, strlen(buf));
	}
#endif
	return 0;
}

