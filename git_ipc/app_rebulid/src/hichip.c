
#include "hichip_debug.h"
#include "hichip.h"
#include "hichip_discover.h"
#include "hichip_http_cgi.h"
#include "ifconf.h"
#include "sysconf.h"
#include "esee_client.h"
#include "md5sum.h"
#include "generic.h"

#include "app_debug.h"
#include "http_util.h"
#include "base/ja_process.h"
#include "sock.h"
#include <sys/prctl.h>

typedef struct HICHIP_SERVER {
	stHICHIP_CONF_FUNC conf_func;
	pthread_t discover_tid;
	bool discover_trigger;
	struct timeval discover_onvif_time;
	pthread_mutex_t lock;
}stHICHIP_SERVER, *lpHICHIP_SERVER;

static stHICHIP_SERVER _hichip = {
	.discover_tid = (pthread_t)NULL,
	.discover_trigger = false,
};
static lpHICHIP_SERVER _p_hichip = NULL;

static void hichip_add_multicast()
{
	NK_SYSTEM("route add -net 224.0.0.0 netmask 224.0.0.0 " "eth0");
}

static void hichip_delete_multicast()
{
	NK_SYSTEM("route del -net 224.0.0.0 netmask 224.0.0.0 " "eth0");
	NK_SYSTEM("route del -net 224.0.0.0 netmask 224.0.0.0 " "wlan0");
	NK_SYSTEM("route del -net 224.0.0.0 netmask 224.0.0.0 " "br0");
}

static int hichip_parse_pack_resopnse(lpHICHIP_DISCOVER_RESOPNSE response, char *buf, int len)
{

	if(0 == strncmp("HDS", buf,3)){
		// broadcast response from others, ignore
//		APP_TRACE("ignore %s\r\n", buf);	
	}else if(0 == strncmp("MCTP", buf, 4)){
		// modify ip response from others, ignore
//		APP_TRACE("ignore %s\r\n", buf);
	}else if(0 == strncmp("HTTP", buf, 4)){
		// netsdk response from others, ignore
//		APP_TRACE("ignore %s\r\n", buf);
	}else{
		buf[len] = '\0';
		size_t const header_size = HTTP_UTIL_check_header(buf, len);
		LP_HTTP_HEAD_FIELD http_request = HTTP_UTIL_parse_request_header(buf, header_size);
		const char *http_content = (char*)buf + header_size;
		// endsym
		if(NULL != http_request){
			//http_request->dump(http_request);
			
			if(0 == strcmp("SEARCH", http_request->method)){
				//APP_TRACE("HICHP search by ");
				HICHIP_DISCOVER_process_search(&_p_hichip->conf_func, response, http_request, http_content);
			}else if(0 == strcmp("CMD", http_request->method)){
				const char *device_id = http_request->read_tag(http_request, "Device-ID");
                APP_TRACE("device_id = %s", device_id);
				if(NULL != device_id && NULL != _p_hichip->conf_func.device_id){
					// check the match device id
					if(0 == strcmp(_p_hichip->conf_func.device_id(), device_id)
						&& strlen(_p_hichip->conf_func.device_id()) == strlen(device_id)){
						// only operate only the device id match one
						HICHIP_DISCOVER_process_cmd(&_p_hichip->conf_func, response, http_request, http_content);
					}
				}
			}else if(0 == strcmp("GBCMD", http_request->method)){
				printf("%s\r\n", buf);
				const char *device_id = http_request->read_tag(http_request, "Device-ID");
				if(NULL != device_id && NULL != _p_hichip->conf_func.device_id){
					// check the match device id
					if(0 == strcmp(_p_hichip->conf_func.device_id(), device_id)
						&& strlen(_p_hichip->conf_func.device_id()) == strlen(device_id)){
						// only operate only the device id match one
						HICHIP_DISCOVER_process_gb28181(&_p_hichip->conf_func, response, http_request, http_content);
					}
				}
				//						
			}else if(0 == strcmp("PING", http_request->method)){
				if(HICHIP_DISCOVER_Pong(&_p_hichip->conf_func, response, http_request, http_content) == 0){
					if(_p_hichip->conf_func.turn_on_onvif){						
						gettimeofday(&_p_hichip->discover_onvif_time,NULL); 						
						_p_hichip->conf_func.turn_on_onvif();
					}
				}
			}else{
				// other
				APP_TRACE("other %s\r\n", buf);
			}

			http_request->free(http_request);
			http_request = NULL;
		}
	}
}
static void *hichip_discover_listener()
{
	int ret = 0, broadcast_sock = -1, curent_sock = -1;
	char buf[2048];
//	ssize_t recv_sz = 0;
	struct sockaddr_in from_addr, peer_addr;
//	struct ip_mreq mreq;
	ifconf_interface_t irf;
    prctl(PR_SET_NAME, "hichip_discover_listener");
	memset(&irf, 0, sizeof(ifconf_interface_t));
	stHICHIP_DISCOVER_RESOPNSE response_args;
	
	struct timeval discover_onvif_time_end;
	gettimeofday(&_p_hichip->discover_onvif_time,NULL); 						

	if(!strcmp(_p_hichip->conf_func.ether_lan(), "eth0")){
		irf.ipaddr.s_addr = 0xA8A8A8C0;//192.168.168.168
	}else{
		//wait for dhcp
		do{
			ifconf_get_interface(_p_hichip->conf_func.ether_lan(), &irf);
			usleep(300000);//300ms
		}while(0 == irf.ipaddr.s_addr && _p_hichip->discover_trigger);

	}
	//ifconf_get_interface(_p_hichip->conf_func.ether_lan(), &irf);
	if(0 == irf.ipaddr.s_addr){
		goto hichip_get_ip_failed;
	}
	//hichip_add_multicast();
	APP_TRACE("%s", _p_hichip->conf_func.ether_lan());
	
	socklen_t addr_len = sizeof(from_addr);
	int sock = HICHIP_DISCOVER_sock_create(ifconf_ipv4_ntoa(irf.ipaddr));

	if(strcmp(_p_hichip->conf_func.ether_lan(), "eth0") != 0){
		broadcast_sock = HICHIP_DISCOVER_sock_broadcast_create(NULL, HICHIP_BROADCAST_PORT, 0, 2000);
	}

	if(sock < 0 && broadcast_sock < 0){
		APP_TRACE("Error: hichip_discover_listener init failed");
		return;
	}

	char route_cmd[256] = {""};

	// add route
	snprintf(route_cmd, sizeof(route_cmd),
		"route add -net "HICHIP_MULTICAST_NET_SEGMENT" netmask "HICHIP_MULTICAST_NET_SEGMENT" %s", _p_hichip->conf_func.ether_lan());
	NK_SYSTEM(route_cmd);

	// peer address
	peer_addr = HICHIP_DISCOVER_multicast_addr();

	//bind eth interface
	if(sock > 0){
		ret = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &irf.ipaddr, sizeof(irf.ipaddr));
		if(ret < 0){
			perror("setsockopt:IP_MULTICAST_LOOP");
			APP_TRACE("setsockopt failed");
		}
	}

	curent_sock = (broadcast_sock > sock) ? broadcast_sock : sock;
	while(_p_hichip->discover_trigger){
		fd_set read_fds;
		struct timeval poll_wait;

#if defined(REPEATER)
		if(ping_count % 120 == 0){
			HICHIP_DISCOVER_Ping(&_p_hichip->conf_func);
		}
		ping_count++;
#endif


#if defined(ONVIFNVT) && defined(WIFI)
		gettimeofday(&discover_onvif_time_end, NULL);
		if(discover_onvif_time_end.tv_sec > _p_hichip->discover_onvif_time.tv_sec){
			if((discover_onvif_time_end.tv_sec - _p_hichip->discover_onvif_time.tv_sec) >= 300){//300			
				if(_p_hichip->conf_func.turn_off_onvif){				
					_p_hichip->discover_onvif_time = discover_onvif_time_end;
					_p_hichip->conf_func.turn_off_onvif();
				}
			}
		}
#endif			

		FD_ZERO(&read_fds);
		if(sock > 0){
			FD_SET(sock, &read_fds);
		}
		if(broadcast_sock > 0){
			FD_SET(broadcast_sock, &read_fds);
		}
		poll_wait.tv_sec = 0;
		poll_wait.tv_usec = 500000;
		ret = select(curent_sock + 1, &read_fds, NULL, NULL, &poll_wait);
		if(ret < 0){
			perror("select");
			break;
		}else if(0 == ret){
			continue; // to next loop
		}else{
			if(sock > 0 && FD_ISSET(sock, &read_fds)) {
				memset(buf, 0, sizeof(buf));
				ret = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from_addr, &addr_len);
#ifndef MAKE_IMAGE
				if(0 != strncmp(inet_ntoa(from_addr.sin_addr), "192.168.2.36", sizeof("192.168.2.36"))){
					continue;
				}
#endif
				if(ret > 0){
					//snprintf(response_args.peer_ip, sizeof(response_args.peer_ip), "%s", inet_ntoa(from_addr.sin_addr));
					response_args.sock = sock;
					response_args.type = HICHIP_DISCOVER_TYPE_MULTICAST;
					hichip_parse_pack_resopnse(&response_args, buf, ret);
				}
			}

			if(broadcast_sock > 0 && FD_ISSET(broadcast_sock, &read_fds)){
				memset(buf, 0, sizeof(buf));
				ret = recvfrom(broadcast_sock, buf, sizeof(buf), 0, (struct sockaddr *)&from_addr, &addr_len);
				if(ret > 0){
					//snprintf(response_args.peer_ip, sizeof(response_args.peer_ip), "%s", inet_ntoa(from_addr.sin_addr));
					response_args.peer_port = ntohs(from_addr.sin_port);
					response_args.sock = broadcast_sock;
					response_args.type = HICHIP_DISCOVER_TYPE_BROADCAST;
					hichip_parse_pack_resopnse(&response_args, buf, ret);
				}
			}

			if(ret < 0){
				if(errno == EAGAIN){
					// try next time
					continue;
				}
				perror("recvfrom");
				break;
			}
		}
	}

	// remove route
	snprintf(route_cmd, sizeof(route_cmd),
		"route del -net "HICHIP_MULTICAST_NET_SEGMENT" netmask "HICHIP_MULTICAST_NET_SEGMENT" %s", _p_hichip->conf_func.ether_lan());
	NK_SYSTEM(route_cmd);

	// close multicast socket
	if(sock > 0){
		HICHIP_DISCOVER_sock_release(sock);
		sock = -1;
	}
	if(broadcast_sock > 0){
		HICHIP_DISCOVER_sock_release(broadcast_sock);
		broadcast_sock = -1;
	}
	//hichip_delete_multicast();

hichip_get_ip_failed:

	pthread_exit(NULL);
}

static void hichip_discover_start()
{
	APP_TRACE("hichip start");
	if(!_p_hichip->discover_tid){
		int ret = 0;
		_p_hichip->discover_trigger = true;
		ret = pthread_create(&_p_hichip->discover_tid, NULL, hichip_discover_listener, NULL);
		//assert(0 == ret);
	}
}

static void hichip_discover_stop()
{
	if(_p_hichip->discover_tid){
		_p_hichip->discover_trigger = false;
		pthread_join(_p_hichip->discover_tid, NULL);
		_p_hichip->discover_tid = (pthread_t)NULL;
		APP_TRACE("hichip stop");
	}
}

int hichip_map_str2dec(const ST_HICHIP_MAP_STR_DEC map[], int map_items, const char *str_key, int def_val)
{
	int i = 0;
	for(i = 0; i < map_items; ++i){
		LP_HICHIP_MAP_STR_DEC map_item = map + i;
		if(strlen(map_item->str) == strlen(str_key)
			&& 0 == strcasecmp(map_item->str, str_key)){
			return map_item->dec;
		}
	}
	return def_val;
}

const char *hichip_map_dec2str(const ST_HICHIP_MAP_STR_DEC map[], int map_items, int dec_key, const char *def_val)
{
	int i = 0;
	for(i = 0; i < map_items; ++i){
		LP_HICHIP_MAP_STR_DEC map_item = map + i;
		if(map_item->dec == dec_key){
			return map_item->str;
		}
	}
	return def_val;
}


void HICHIP_update_interface(const char *eth)
{
	char route_cmd[256];
	if(eth){
		setenv("DEF_ETHER", eth, true);
		hichip_delete_multicast();
		snprintf(route_cmd, sizeof(route_cmd),
			"route add -net "HICHIP_MULTICAST_NET_SEGMENT" netmask "HICHIP_MULTICAST_NET_SEGMENT" %s", _p_hichip->conf_func.ether_lan());
		NK_SYSTEM(route_cmd);
		NK_SYSTEM("route");
		printf("bind %s\n", eth);
	}
}

int HICHIP_init(stHICHIP_CONF_FUNC conf_func)
{
	HICHIP_Lock();
	if(!_p_hichip){
		HICHIP_http_init();
		_p_hichip = &_hichip;
		// init elements
		_p_hichip->conf_func = conf_func;
		_p_hichip->discover_trigger = false;
		_p_hichip->discover_tid = (pthread_t)NULL;
		//
		hichip_discover_start();
		HICHIP_Unlock();
		return 0;
	}
	HICHIP_Unlock();
	return -1;
}

void HICHIP_destroy()
{
	HICHIP_Lock();
	if(_p_hichip){
		HICHIP_http_destroy();
		hichip_discover_stop();
		_p_hichip = NULL;
	}
	HICHIP_Unlock();
}

int HICHIP_Lock_init()
{
	return pthread_mutex_init(&_hichip.lock, NULL);
}

int HICHIP_Lock_destroy()
{
	return pthread_mutex_destroy(&_hichip.lock);
}

int HICHIP_Lock()
{
	return pthread_mutex_lock(&_hichip.lock);
}

int HICHIP_Unlock()
{
	return pthread_mutex_unlock(&_hichip.lock);
}

