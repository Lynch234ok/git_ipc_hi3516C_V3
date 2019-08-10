
#include "netsdk.h"
#include "netsdk_util.h"
#include "netsdk_private.h"
#include "generic.h"
#include "sdk/sdk_api.h"
#include "app_overlay.h"
#include "ticker.h"
#include "generic.h"
#include "socket_tcp.h"
#include "mutex_f.h"
#include "app_debug.h"

#if defined(SDCARD)
#include "sdcard/sdcard.h"
#include "frank_trace.h"

int NETSDK_sdcard_media_search(LP_HTTP_CONTEXT context)
{

	int i = 0;
	int endUTC = -1, beginUTC = -1;
	
	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	LP_HTTP_QUERY_PARA_LIST httpQuery = HTTP_UTIL_parse_query_as_para(httpHeadField->query);
	
	char *queryBeginUTC = httpQuery->read(httpQuery, "beginUTC");
	char *queryEndUTC = httpQuery->read(httpQuery, "endUTC");
	
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	
	char httpHeadBuf[2 * 1024];
	int httpHeadLength = 0;
	char httpContentBuf[32 * 1024];
	char *httpContentOffset = httpContentBuf;
	int httpContentMax = sizeof(httpContentBuf);
	int httpContentLength = 0;
	int httpStatusCode = kHTTP_STAT_OK;
	ST_SDCARD_MEDIA_SESSION sessionSearch[1024];
	int sessionCnt = 0;

	// get the UTC paramenter
	if(NULL != queryBeginUTC){
		beginUTC = atoi(queryBeginUTC);
	}
	if(NULL != queryEndUTC){
		endUTC = atoi(queryEndUTC);
	}

	sessionCnt = SDCARD_db_media_search_by_time(1, beginUTC, endUTC, sessionSearch, sizeof(sessionSearch));
	_TRACE("Session Count = %d", sessionCnt);
		
	if(sessionCnt > 0){
		char jsonHead[] =
			"[";
		char jsonFormat[] =
			"{"
				"\"sessionID\" : %d,"
				"\"channelID\": %d,"
				"\"beginUTC\": %d,"
				"\"endUTC\": %d,"
				"\"type\" : \"%s\""
			"},";
		char jsonTail[] = "]";

		strcpy(httpContentBuf, jsonHead);
		httpContentLength = strlen(httpContentBuf);
		// init the content offset and content max size
		httpContentOffset = httpContentBuf + httpContentLength;
		httpContentMax -= httpContentLength;
		
		for(i = 0; i < sessionCnt; ++i){
			LP_SDCARD_MEDIA_SESSION const session = &sessionSearch[i];
			char oneSessionBuf[1024];
			int const oneSessionLength = snprintf(httpContentOffset, httpContentMax, jsonFormat,
				session->sessionID,
				session->channelID,
				session->beginUTC, session->endUTC,
				session->type);
			
			httpContentOffset += oneSessionLength;
			httpContentMax -= oneSessionLength;

			// check the buffer full
			if(httpContentMax + sizeof(jsonTail) >= sizeof(httpContentBuf)){
				_TRACE("HTTP Buffer Size too Short!");
				break;
			}
		}
		httpContentBuf[strlen(httpContentBuf) - 1] = '\0'; // cut the last comma
		strcat(httpContentBuf, jsonTail);
		httpContentLength = strlen(httpContentBuf);

		json_object *sessionJSON = json_tokener_parse(httpContentBuf);

		strcpy(httpContentBuf, json_object_to_json_string(sessionJSON));
		json_object_put(sessionJSON);

		httpContentLength = strlen(httpContentBuf);

		_TRACE("%s", httpContentBuf);	

		//httpHeadField = HTTP_UTIL_new_response_head_200(kH_TAG_DATE | kH_TAG_SERVER, 0, "text/xml", httpContentLength);
		httpStatusCode = kHTTP_STAT_OK;
	}else{
		//httpHeadField = HTTP_UTIL_new_response_head_200(kH_TAG_DATE | kH_TAG_SERVER, 0, NULL, 0);
		httpStatusCode = 503;
	}
		
	if(kHTTP_STAT_OK == httpStatusCode){
		httpHeadField = HTTP_UTIL_new_response_head_200(kH_TAG_DATE | kH_TAG_SERVER, 0, "application/json", httpContentLength);
	}else{
		httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", httpStatusCode, NULL);
		httpHeadField->add_tag_server(httpHeadField, "IE/10.0");
		httpHeadField->add_tag_date(httpHeadField, 0);
		httpHeadField->add_tag_int(httpHeadField, "Content-Length", 0, true);
	}
	
	httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
	httpHeadField->free(httpHeadField);
	httpHeadField = NULL;

	// send out http head
	if(httpHeadLength == httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0)){
		// send out http content
		if(httpContentLength > 0){
			if(httpContentLength == httpTCP->send2(httpTCP, httpContentBuf, httpContentLength, 0)){
				return 0;
			}
		}
	}

	return 0;
}

#include "flvlib.h"
#include "semaphore_f.h"

static ssize_t flv_write(int fd, const void* buf, size_t count)
{
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(fd, &sockTCP);

	return httpTCP->send2(httpTCP, buf, count, 0);
}


static int playback_over_flv(LP_HTTP_CONTEXT context, int *fIDs, int nFIDs, int sessionID)
{
	int i = 0;
	int err = 0;
	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	LP_HTTP_QUERY_PARA_LIST httpQuery = HTTP_UTIL_parse_query_as_para(httpHeadField->query);

	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	FLV_t *flv = FLV_dup_file(context->sock);

	bool flvKeyFrameRequest = false;
	
	int sendN = 0, readN = 0;

	char httpHeadBuf[2 * 1024];
	int httpHeadLength = 0;
	
	char payload[512 * 1024];
	ST_SDCARD_FRAMER framer;

	flv->set_write(flv, flv_write);

	// make an HTTP head and send the packet out
	httpHeadField = HTTP_UTIL_new_response_head_200(kH_TAG_DATE | kH_TAG_SERVER, false, "flv-application/octet-stream", -1);
	httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
	httpHeadField->free(httpHeadField);
	httpHeadField = NULL;

	// send out http header
	sendN = httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0);
	if(sendN == httpHeadLength){
		// send out content
		for(i = 0; i < nFIDs && *context->trigger && 0 == err; ++i){
			while((readN = SDCARD_media_read(fIDs[i], &framer, payload, sizeof(payload))) > 0 && *context->trigger && 0 == err){
				
				if(kSDF_TYPE_VIDEO == framer.type){
					LP_SDCARD_FRAMER_VIDEO const framerVideo = &framer.video;
					if(sessionID != framerVideo->sessionID){
						continue;
					}
					if(0 == framerVideo->channelSubID){
						if(framerVideo->keyFrame && !flvKeyFrameRequest){
							flv->set_video(flv, framerVideo->resolutionWidth, framerVideo->resolutionHeight, framerVideo->frameRate, 4000);
							//flv->set_audio(flv);
							flvKeyFrameRequest = true;
						}

						if(flvKeyFrameRequest){
							sendN = flv->write_h264(flv, payload, framer.dataLength, framerVideo->keyFrame, framer.timestampMS);
							if(sendN < 0){
								err = errno;
							}
						}
					}
				}else if(kSDF_TYPE_AUDIO == framer.type){
					LP_SDCARD_FRAMER_AUDIO const framerAudio = &framer.audio;
					if(0){//if(sessionID != framerAudio->sessionID){
						continue;
					}

					_TRACE("Ignore Audio Frames");
				}else if(kSDF_TYPE_PIC == framer.type){
					LP_SDCARD_FRAMER_PIC const framerPicture = &framer.picture;
					if(0){//if(sessionID != framerPicture->sessionID){
						continue;
					}
				
					_TRACE("Ignore Picture Frames");
				}else if(kSDF_TYPE_EOF == framer.type){
					// exit this file
					break;
				}
			}
		}
	}else{
		err = errno;
	}

	FLV_close(flv);
	flv = NULL;
	
	httpQuery->free(httpQuery);
	httpQuery = NULL;
	return err;
}


int NETSDK_sdcard_media_playback_flv(LP_HTTP_CONTEXT context)
{
	static LP_MUTEX mutex = NULL;
	int i = 0;
	int endUTC = -1, beginUTC = -1;

	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	LP_HTTP_QUERY_PARA_LIST httpQuery = HTTP_UTIL_parse_query_as_para(httpHeadField->query);
	
	char *querySessionID = httpQuery->read(httpQuery, "sessionID");
	char *queryBeginUTC = httpQuery->read(httpQuery, "beginUTC");
	char *queryEndUTC = httpQuery->read(httpQuery, "endUTC");
	
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	int sendN = 0, readN = 0;

	//querySessionID = "70";
	
	char httpHeadBuf[2 * 1024];
	int httpHeadLength = 0;
	char httpContentBuf[16 * 1024];
	int httpContentMax = sizeof(httpContentBuf);
	int httpContentLength = 0;
	int httpStatusCode = kHTTP_STAT_OK;

	int err = 0;

	// prevent the http keep alive
	context->keep_alive = 0;

	if(!mutex){
		mutex = MUTEX_create();
	}

	if(0 == mutex->tryLock(mutex)){
		if(!querySessionID){
			// query session ID is necessary
			_TRACE("SessionID is Neccessary");
			httpStatusCode = kHTTP_STAT_BAD_REQUEST;
			httpContentLength = snprintf(httpContentBuf, sizeof(httpContentBuf), "SessionID is Neccessary");
		}else{
			int const sessionID = atoi(querySessionID);
			ST_SDCARD_MEDIA_SESSION mediaSession;

			_TRACE("Session ID = %d", sessionID);

			readN = SDCARD_db_media_search_by_session(sessionID, &mediaSession);
			if(readN > 0){
				int const beginFileID = mediaSession.beginFileID;
				int const beginFileOffset = mediaSession.beginFileOffset;
				int const endFileID = mediaSession.endFileID;
				int const endFileOffset = mediaSession.endFileOffset;
				int const fIDsMax = endFileID - beginFileID + 1;
				int *fIDs = alloca(fIDsMax * sizeof(int));
				int fIDsN = 0;

				fIDsN = SDCARD_media_open_range(beginFileID, endFileID, fIDs, fIDsMax);
				if(fIDsN > 0){
					char payload[512 * 1024];
					ST_SDCARD_FRAMER framer;

					_TRACE("Begin to Playback [%d, %d]", beginFileID, endFileID);
					//err = playback_over_piecake(context, fIDs, fIDsN, sessionID);
					// begin file offset
					lseek(fIDs[0], beginFileOffset, SEEK_SET);
					err = playback_over_flv(context, fIDs, fIDsN, sessionID);
					_TRACE("End to Playback [%d, %d]", beginFileID, endFileID);
					httpStatusCode = kHTTP_STAT_OK;
					
					// close all the file
					SDCARD_media_close_range(fIDs, fIDsN);
				}else{
					httpStatusCode = kHTTP_STAT_CONFLICT;
					httpContentLength = snprintf(httpContentBuf, sizeof(httpContentBuf), "SD Card is Busy");
				}
			}else if(0 == readN){
				httpStatusCode = kHTTP_STAT_NOT_FOUND;
				httpContentLength = snprintf(httpContentBuf, sizeof(httpContentBuf), "Session ID Not Found");
			}else{
				httpStatusCode = kHTTP_STAT_CONFLICT;
				httpContentLength = snprintf(httpContentBuf, sizeof(httpContentBuf), "SD Card is Busy");
			}
		}
		// post sem
		mutex->unlock(mutex);
	}else{
		_TRACE("Someone is Playing Back on SD Card");
		httpStatusCode = kHTTP_STAT_CONFLICT;
		httpContentLength = snprintf(httpContentBuf, sizeof(httpContentBuf), "SD Card is Busy");
	}
	
	_TRACE("Playback HTTP Status %d: %s", httpStatusCode, httpContentBuf);
	if(0 == err && kHTTP_STAT_OK != httpStatusCode){
		
		
		httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", httpStatusCode, NULL);
		httpHeadField->add_tag_server(httpHeadField, "IE/10.0");
		httpHeadField->add_tag_date(httpHeadField, 0);
		httpHeadField->add_tag_text(httpHeadField, "Content-Type", "text/plain", true);
		httpHeadField->add_tag_int(httpHeadField, "Content-Length", httpHeadLength, true);

		httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
		httpHeadField->free(httpHeadField);
		httpHeadField = NULL;

		// send out http head when situation not 200 ok
		if(httpHeadLength == httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0)){
			// send out http content
			if(httpContentLength > 0){
				if(httpContentLength == httpTCP->send2(httpTCP, httpContentBuf, httpContentLength, 0)){
					// send
				}
			}
			return 0;
		}
	}

	httpQuery->free(httpQuery);
	return 0;
}

int NETSDK_sdcard_format(LP_HTTP_CONTEXT context)
{
	_TRACE("SD Card Request Format");
	SDCARD_request_format();
	return 0;
}

int NETSDK_sdcard_status(LP_HTTP_CONTEXT context)
{
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	
	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	char httpHeadBuf[512];
	int httpHeadLength = 0;
	char httpContentBuf[512];
	int httpContentLength = 0;
	int sdcardStat = 0;

	// content
#if defined(SDCARD)
	sdcardStat = SDCARD_status();
#else 
	sdcardStat = kSD_STAT_NOT_INIT;
#endif
	switch(sdcardStat){
		case kSD_STAT_ON_WORK:
			strcpy(httpContentBuf, "\"work\"");
			break;
		case kSD_STAT_EJECTED:
			strcpy(httpContentBuf, "\"ejected\"");
			break;
		case kSD_STAT_FS_ERROR:
			strcpy(httpContentBuf, "\"fserror\"");
			break;
		case kSD_STAT_FORMATTING:
			strcpy(httpContentBuf, "\"formatting\"");
			break;
		case kSD_STAT_NOT_INIT:
		default:
			strcpy(httpContentBuf, "\"zombie\"");
	}
	httpContentLength = strlen(httpContentBuf);

	httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", 200, NULL);
	httpHeadField->add_tag_server(httpHeadField, "IE/10.0");
	httpHeadField->add_tag_date(httpHeadField, 0);
	httpHeadField->add_tag_text(httpHeadField, "Content-Type", "application/json", true);
	httpHeadField->add_tag_int(httpHeadField, "Content-Length", httpContentLength, true);

	httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
	httpHeadField->free(httpHeadField);
	httpHeadField = NULL;
	
	// send out http head when situation not 200 ok
	if(httpHeadLength == httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0)){
		// send out http content
		if(httpContentLength > 0){
			if(httpContentLength == httpTCP->send2(httpTCP, httpContentBuf, httpContentLength, 0)){
				// send
			}
		}
		return 0;
	}

	return 0;
}

int NETSDK_sdcard_checkRW(LP_HTTP_CONTEXT context)
{
	return 0;
}

int NETSDK_sdcard_file(LP_HTTP_CONTEXT context)
{
	return 0;
}



#endif//define (SDCARD)

#if defined(TFCARD)
#include "tfcard.h"

#if defined(TS_RECORD)
#include "tfcard/include/NK_Tfcard.h"
#endif

int NETSDK_sdcard_media_search(LP_HTTP_CONTEXT context)
{
	return 0;
}


int NETSDK_sdcard_media_playback_flv(LP_HTTP_CONTEXT context)
{
	return 0;
}

int NETSDK_sdcard_format(LP_HTTP_CONTEXT context)
{
#if defined(TS_RECORD)
    NK_TFCARD_Format();
#else
	NK_TFCARD_format();
#endif
	return 0;
}

int NETSDK_sdcard_status(LP_HTTP_CONTEXT context)
{
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	
	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	char httpHeadBuf[512];
	int httpHeadLength = 0;
	char httpContentBuf[512];
	int httpContentLength = 0;
	char tfcard_status[32] = {0};
	int status;
	// content

#if defined(TS_RECORD)
    status = NK_TFCARD_GetStatus();
    switch(status)
    {
        case EN_NK_TFCARD_STATUS_OK:
        case EN_NK_TFCARD_STATUS_FORMATED:
            strcpy(httpContentBuf, "\"work\"");
        break;
        case EN_NK_TFCARD_STATUS_NO_TFCARD:
            strcpy(httpContentBuf, "\"ejected\"");
        break;
        case EN_NK_TFCARD_STATUS_NO_FORMAT:
            strcpy(httpContentBuf, "\"fserror\"");
        break;
        case EN_NK_TFCARD_STATUS_FORMATTING:
            strcpy(httpContentBuf, "\"formatting\"");
        break;
        default:
            strcpy(httpContentBuf, "\"zombie\"");
    }

#else
	status = NK_TFCARD_get_status(tfcard_status);
	switch(status){
		case emTFCARD_STATUS_OK:
		case emTFCARD_STATUS_FORMATED:
			strcpy(httpContentBuf, "\"work\"");
			break;
		case emTFCARD_STATUS_NO_TFCARD:
			strcpy(httpContentBuf, "\"ejected\"");
			break;
		case emTFCARD_STATUS_NOT_FORMAT:
			strcpy(httpContentBuf, "\"fserror\"");
			break;
		case emTFCARD_STATUS_FORMATTING:
			strcpy(httpContentBuf, "\"formatting\"");
			break;
		default:
			strcpy(httpContentBuf, "\"zombie\"");
	}
#endif
	httpContentLength = strlen(httpContentBuf);

	httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", 200, NULL);
	httpHeadField->add_tag_server(httpHeadField, "IE/10.0");
	httpHeadField->add_tag_date(httpHeadField, 0);
	httpHeadField->add_tag_text(httpHeadField, "Content-Type", "application/json", true);
	httpHeadField->add_tag_int(httpHeadField, "Content-Length", httpContentLength, true);

	httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
	httpHeadField->free(httpHeadField);
	httpHeadField = NULL;
	
	// send out http head when situation not 200 ok
	if(httpHeadLength == httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0)){
		// send out http content
		if(httpContentLength > 0){
			if(httpContentLength == httpTCP->send2(httpTCP, httpContentBuf, httpContentLength, 0)){
				// send
			}
		}
		return 0;
	}
	return 0;
}

int NETSDK_sdcard_checkRW(LP_HTTP_CONTEXT context)
{
#define SDCARD_CHECK_FILE "/media/tf/checkRW"
#define SDCARD_CHECK_TEXT_LENGTH (32)
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	
	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	char httpHeadBuf[512];
	int httpHeadLength = 0;
	char httpContentBuf[512];
	int httpContentLength = 0, i = 0, ret = 0;
	char ram_num[SDCARD_CHECK_TEXT_LENGTH] = {0};
	char buf[SDCARD_CHECK_TEXT_LENGTH] = {0};
	bool read_status = false, write_status = false;
	FILE * fd = NULL;
	// content

	struct timespec timetic;
	clock_gettime(CLOCK_MONOTONIC, &timetic);
	srand((unsigned) timetic.tv_nsec);
	for(i = 0; i < SDCARD_CHECK_TEXT_LENGTH; ++i){
		ram_num[i] = rand() % 26 + ((0 == rand() % 2) ? 'A' : 'a');
	}

	//test write
	fd = fopen(SDCARD_CHECK_FILE, "wb");
	if(fd){
		ret = fwrite(ram_num, 1, sizeof(ram_num), fd);
		if(ret == sizeof(ram_num)){
			write_status = true;
		}
		fclose(fd);
	}

	//test read
	fd = fopen(SDCARD_CHECK_FILE, "rb");
	if(fd){
		ret = fread(buf, 1, sizeof(buf), fd);
		if(0 == strncmp(buf, ram_num, SDCARD_CHECK_TEXT_LENGTH)){
			read_status = true;
		}
		fclose(fd);
	}

	//remove test file
	REMOVE_FILE(SDCARD_CHECK_FILE);

	snprintf(httpContentBuf, sizeof(httpContentBuf), 
		"{\"ReadResult\":\"%s\",\"WriteResult\":\"%s\"}",
		read_status? "Success":"Fail", write_status? "Success":"Fail");
	httpContentLength = strlen(httpContentBuf);

	httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", 200, NULL);
	httpHeadField->add_tag_server(httpHeadField, "IE/10.0");
	httpHeadField->add_tag_date(httpHeadField, 0);
	httpHeadField->add_tag_text(httpHeadField, "Content-Type", "application/json", true);
	httpHeadField->add_tag_int(httpHeadField, "Content-Length", httpContentLength, true);

	httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
	httpHeadField->free(httpHeadField);
	httpHeadField = NULL;
	
	// send out http head when situation not 200 ok
	if(httpHeadLength == httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0)){
		// send out http content
		if(httpContentLength > 0){
			if(httpContentLength == httpTCP->send2(httpTCP, httpContentBuf, httpContentLength, 0)){
				// send
			}
		}
	}
	return 0;

}

int NETSDK_sdcard_file(LP_HTTP_CONTEXT context)
{
	int ret = 0;

	LP_HTTP_QUERY_PARA_LIST pHead=NULL;
	const char *cmd = NULL;
	LP_HTTP_HEAD_FIELD httpHeadField;
	char httpHeadBuf[512];
	int httpHeadLength = 0;
	char httpContentBuf[512];
	int httpContentLength = 0;
	FILE* sdcard_fd = NULL;
	int code = -2;
	char *mesg = NULL;
	char file_path[128];
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	pHead= HTTP_UTIL_parse_query_as_para(context->request_header->query);

	if(NULL == pHead){
		goto http_head_parse_failed;
	}

	cmd = pHead->read(pHead, "path");
	if(NULL == cmd){
		goto http_param_parse_failed;
	}

	memset(httpContentBuf, 0, sizeof(httpContentBuf));
	snprintf(file_path, sizeof(file_path), "/media/tf/%s", cmd);

	if(HTTP_IS_GET(context)){
		APP_TRACE("sd card test:get");
		sdcard_fd = fopen(file_path, "rb");
		if(sdcard_fd){
			fread(httpContentBuf, sizeof(httpContentBuf), 1, sdcard_fd);
			fclose(sdcard_fd);
		}else{
			sprintf(httpContentBuf, "failed");
		}
	}else if(HTTP_IS_POST(context) || HTTP_IS_PUT(context)){
		APP_TRACE("sd card test:post");
		sdcard_fd = fopen(file_path, "wb+");
		if(sdcard_fd){
			ret = fwrite(context->request_content, 1, context->request_content_len, sdcard_fd);
			if(ret == context->request_content_len){
				code = 0;
			}
			fclose(sdcard_fd);
		}
	}else if(HTTP_IS_DELETE(context)){
		APP_TRACE("sd card test:delete");
		REMOVE_FILE(file_path);
		code = 0;
	}else{
		APP_TRACE("unknow method!");
	}

	switch(code){
			case 0:
				mesg = "OK"; break;
			case -2:
				mesg = "Device Error"; break;
			default:
				mesg = "Unknown Error"; break;
		}

	if(!HTTP_IS_GET(context)){
		snprintf(httpContentBuf, sizeof(httpContentBuf),
				"{"
				"\"requestURI\" : \"%s\","
				"\"statusCode\" : \"%d\","
				"\"statusString\" : \"%s\""
				"}",
				context->request_header->uri,
				code,
				mesg);
	}
	
	httpContentLength = strlen(httpContentBuf);

	httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", 200, NULL);
	httpHeadField->add_tag_server(httpHeadField, "IE/10.0");
	httpHeadField->add_tag_date(httpHeadField, 0);
	httpHeadField->add_tag_text(httpHeadField, "Content-Type", "application/json", true);
	httpHeadField->add_tag_int(httpHeadField, "Content-Length", httpContentLength, true);

	httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
	httpHeadField->free(httpHeadField);
	httpHeadField = NULL;
	
	// send out http head when situation not 200 ok
	if(httpHeadLength == httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0)){
		// send out http content
		if(httpContentLength > 0){
			if(httpContentLength == httpTCP->send2(httpTCP, httpContentBuf, httpContentLength, 0)){
				// send
			}
		}
	}
	
http_param_parse_failed:
	pHead->free(pHead);
	pHead = NULL;

http_head_parse_failed:
	return 0;
}

#endif//define (TFCARD)


int NETSDK_sdcard_instance(LP_HTTP_CONTEXT context, HTTP_CSTR_t subURI, char *content, int contentMax)
{


	return 0;
}

