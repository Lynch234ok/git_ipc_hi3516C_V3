
#include "netsdk.h"
#include "netsdk_private.h"
#include "netsdk_util.h"
#include "generic.h"
#include "sdk/sdk_api.h"
#include "socket_tcp.h"
#include "app_overlay.h"
#include "app_debug.h"

int NETSDK_stream_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t sub_uri, char *content, int content_max)
{
	int i = 0, id = 0;
	int ret = kNSDK_INS_RET_INVALID_OPERATION;
	HTTP_CSTR_t prefix = NULL;
	LP_JSON_OBJECT jsonRef = json_object_get(netsdk->stream_conf);
	stSOCKET_TCP sock;
	lpSOCKET_TCP tcp = socket_tcp2_r(context->sock, &sock);
	struct sockaddr_in sock_addr;
	socklen_t sock_addrlen = sizeof(sock_addr);

	if(0 != tcp->getsockname(tcp, &sock_addr, &sock_addrlen)){
		ret = kNSDK_INS_RET_DEVICE_ERROR;
	}else{
		LP_JSON_OBJECT channels = NETSDK_json_get_child(jsonRef, "streamChannel");		
		int const n_channels = json_object_array_length(channels);
		int const port = ntohs(sock_addr.sin_port);

		// update the port number
		for(i = 0; i < n_channels; ++i){
			LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
			LP_JSON_OBJECT transportRTSP = NETSDK_json_get_child(channel, "transportRTSP");
			LP_JSON_OBJECT transportRTSPoverHTTP = NETSDK_json_get_child(channel, "transportRTSPoverHTTP");
			LP_JSON_OBJECT transportFLVoverHTTP = NETSDK_json_get_child(channel, "transportFLVoverHTTP");
			LP_JSON_OBJECT transportRTMP = NETSDK_json_get_child(channel, "transportRTMP");
			
			NETSDK_json_set_int2(transportRTSP, "port", port);
			NETSDK_json_set_int2(transportRTSPoverHTTP, "port", port);
			NETSDK_json_set_int2(transportFLVoverHTTP, "port", port);
			NETSDK_json_set_int2(transportRTMP, "port", port);
		}

		if(H_IS_GET(context->request_header->method)){
			if(prefix = "/CHANNELS", 0 == strncmp(prefix, sub_uri, strlen(prefix))){
				sub_uri += strlen(prefix);
				snprintf(content, content_max, "%s", json_object_to_json_string(channels));
				ret = kNSDK_INS_RET_CONTENT_READY;
			}else if(prefix = "/CHANNEL/%d", 1 == sscanf(sub_uri, prefix, &id)){
				sub_uri += sprintf(content, prefix, id);
				if(id > 0){
					
					for(i = 0; i < n_channels; ++i){
						LP_JSON_OBJECT channel = json_object_array_get_idx(channels, i);
						int const channelID = NETSDK_json_get_int(channel, "id");
						if(channelID == id){
							snprintf(content, content_max, "%s", json_object_to_json_string(channel));
							ret = kNSDK_INS_RET_CONTENT_READY;
							break;
						}
					}
				}
			}
		}
	}

	json_object_put(jsonRef);
	jsonRef = NULL;
	return ret;
}



