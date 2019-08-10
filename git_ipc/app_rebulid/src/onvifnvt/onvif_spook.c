#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generic.h"
#include "http_common.h"
#include "app_debug.h"

#include "onvif.h"
#include "spook/onvif_spook.h"

SPOOK_SESSION_PROBE_t ONVIF_nvt_probe(const void* msg, ssize_t msg_sz)
{
	if((msg != NULL) && (msg_sz > 0)){
		if(0 == strncasecmp(msg, "POST", 4)){
			HTTP_REQUEST_LINE_t request_line;
			char* const request_msg = alloca(msg_sz + 1);
			char* http_uri_suffix = NULL;
			char* ptr = NULL;
			char func_name[260];
			AVal saction;

			memcpy(request_msg, msg, msg_sz);
			request_msg[msg_sz] = '\0';
			http_parse_request_line(request_msg, &request_line);

			// get suffix
			http_uri_suffix = AVAL_STRDUPA(request_line.uri_suffix);
			//APP_TRACE("Check suffix: \"%s\"", http_uri_suffix);
			
			if (http_read_header(request_msg, "SOAPAction", &saction) == 0) {
				saction.av_val[saction.av_len] = 0;
				if (strstr(saction.av_val,"Renew") == NULL && strstr(saction.av_val,"PullMessages") == NULL)
					APP_TRACE("Check suffix:%s SOAPAction: %s", http_uri_suffix, saction.av_val);
			} else {
				if ((ptr = strstr(request_msg, "Body")) != NULL) {
					char *pbody = ptr;
					if ((ptr = strstr(pbody, ">")) != NULL) {
						strncpy(func_name, ptr  + 2, 256);
						func_name[256] = '\0';
						if ((ptr = strstr(func_name, ">")) != NULL) {
							*(ptr  + 1)= '\0';
						}
						if ((ptr = strstr(func_name, " ")) != NULL) {
							*ptr= '\0';
						}
					}
					if (strstr(func_name,"Renew") == NULL && strstr(func_name,"PullMessages") == NULL)
						APP_TRACE("Check suffix:%s function: %s", http_uri_suffix, func_name);
				}
			}
			if(ONVIF_check_uri(http_uri_suffix, strlen(http_uri_suffix)) == 0){
				return SPOOK_PROBE_MATCH;
			}
		}
	}
	return SPOOK_PROBE_MISMATCH;
}

SPOOK_SESSION_LOOP_t ONVIF_nvt_loop(bool* trigger, int sock, time_t* read_pts)
{
	int ret;
	
	signal(SIGPIPE, SIG_IGN);
	ret = ONVIF_SERVER_daemon(sock);
	*trigger = false;
	return ret;
}



