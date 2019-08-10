
#include "piecake.h"
#include "sdk/sdk_api.h"
#include "media_buf.h"
#include "generic.h"
#include "http_common.h"
#include "sensor.h"
#include "sysconf.h"
#include "timertask.h"
#include "sdk/sdk_vin.h"
#include "app_motion_detect.h"
#include "sdk/sdk_isp_def.h"
#include "cgi_hash.h"
#include "app_overlay.h"
#include "hichip_debug.h"
#include "base64.h"
#include "usrm.h"
#include "ja_ini.h"
#include "app_debug.h"
#include "socket_tcp.h"
#include "frank_crypt.h"
#include "frank_trace.h"
#include "ptz.h"
#include "mutex_f.h"
#include "flvlib.h"

#define TEST_USERNAME "admin"
#define TEST_PASSWORD ""

static int _pieCakeSignature = 0;

static int piecake_check_session(const char *session)
{
	char decryptBuf[128] = {""};
	if(frank_decrypt(session, strlen(session), decryptBuf, sizeof(decryptBuf)) > 0){
		APP_TRACE("Session: %s", decryptBuf);
		return 0;
	}
	return -1;
}

static ssize_t piecake_video_header(int ts_ms, int data_len, const char *codec, const char *h264_profile,
	int width, int height, int fps, bool key_frame,
	char *result, int result_max)
{
	char piecakeVideoHeadFmt[] = 
		"<PieCake version=\"1.0\">"\
			"<encryption>%s</encryption>"\
			"<video>"\
				"<timeStamp>%d</timeStamp>"\
				"<dataLength>%d</dataLength>"\
				"<codec>%s</codec>"\
				"<profile>%s</profile>"\
				"<resolutionWidth>%d</resolutionWidth>"\
				"<resolutionHeight>%d</resolutionHeight>"\
				"<frameRate>%d</frameRate>"\
				"<keyFrame>%s</keyFrame>"\
			"</video>"\
		"</PieCake>";
	char *preHeader = result;
	char *xmlBlock = result + 4;
	int const xmlBlock_max = result_max - 4;
	int xmlBlockLen = 0;

	xmlBlockLen = snprintf(xmlBlock, xmlBlock_max, piecakeVideoHeadFmt,
		"true", // encryption
		ts_ms, data_len,
		"H.264", "baseline",
		width, height, fps,
		key_frame ? "true" : "false");

	preHeader[0] = 'P';
	preHeader[1] = 'C';
	preHeader[2] = xmlBlockLen / 0x100;
	preHeader[3] = xmlBlockLen % 0x100;

	//APP_TRACE("Video XML Block: %d %d", preHeader[2], preHeader[3]);

	return xmlBlockLen + 4;
}

static ssize_t piecake_audio_header(int ts_ms, int data_len, const char *codec, int sample_rate, int sample_bitwidth,
	char *result, int result_max)
{

	char piecakeAudioHeadFmt[] = 
		"<PieCake version=\"1.0\">"\
			"<encryption>%s</encryption>"\
			"<audio>"\
				"<timeStamp>%d</timeStamp>"\
				"<dataLength>%d</dataLength>"\
				"<codec>%s</codec>"\
				"<sampleRate>%d</sampleRate>"\
				"<sampleBitWidth>%d</sampleBitWidth>"\
			"</audio>"\
		"</PieCake>";
	char *preHeader = result;
	char *xmlBlock = result + 4;
	int const xmlBlock_max = result_max - 4;
	int xmlBlockLen = 0;

	xmlBlockLen = snprintf(xmlBlock, xmlBlock_max, piecakeAudioHeadFmt,
		"false", // encryption
		ts_ms, data_len,
		codec, sample_rate, sample_bitwidth);

	preHeader[0] = 'P';
	preHeader[1] = 'C';
	preHeader[2] = xmlBlockLen / 0x100;
	preHeader[3] = xmlBlockLen % 0x100;
	
	//APP_TRACE("Audio XML Block: %d %d", preHeader[2], preHeader[3]);
	
	return xmlBlockLen + 4;
}

int PIECAKE_live_picture(LP_HTTP_CONTEXT context)
{
	int ret = -1;
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	LP_HTTP_QUERY_PARA_LIST query = NULL;
	char fileName[128];
	int channelID = 0;
	int resolutionWidth = 320, resolutionHeight = 240;
	FILE* fid = NULL;
	int pictureSize= 0;

	char httpHeadBuf[1024];
	int httpHeadLength = 0;
	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	const char *session = httpHeadField->read_tag(httpHeadField, "Session");
	bool sessionPass = false;

	if(NULL != session && 0 == piecake_check_session(session)){
		if(HTTP_IS_GET(context)){
			
			// snapshot a picture from sdk
			snprintf(fileName, sizeof(fileName), "/tmp/PieCake%08x%08x%08x.jpg", (unsigned int)time(NULL), (unsigned int)getpid(), (unsigned int)rand());
			fid = fopen(fileName, "w+b");
			if(NULL != fid){
				if(0 == sdk_enc->snapshot(channelID, kSDK_ENC_SNAPSHOT_QUALITY_HIGH, resolutionWidth, resolutionHeight, fid)){
					GET_FILE_SIZE(fileName, pictureSize);
				}
				fclose(fid);
			}

			if(pictureSize > 0){
				httpHeadField = HTTP_UTIL_new_response_head_200(kH_TAG_SERVER|kH_TAG_DATE, false, "image/jpeg", pictureSize);
				httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
				httpHeadField->free(httpHeadField);
				httpHeadField = NULL;

				APP_TRACE("PieCake snapShot:\r\n%s", httpHeadBuf);
				ret = httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0);
				if(ret < 0){

				}

				sessionPass = true;
				fid = fopen(fileName, "r+b");
				if(NULL != fid){
					char fileBuf[1024];
					int readn = 0;
					while((readn = fread(fileBuf, 1, sizeof(fileBuf), fid)) > 0){
						ret = httpTCP->send2(httpTCP, fileBuf, readn, 0);
						if(ret < 0){
							// do something
						}
					}
				}
				fclose(fid);
				fid = NULL;
			}

			REMOVE_FILE(fileName);
		}
	}

	if(!sessionPass){
		httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", 403, NULL);
		httpHeadField->add_tag_server(httpHeadField, "JAWS/1.0");
		httpHeadField->add_tag_date(httpHeadField, 0);
		httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
		httpHeadField->free(httpHeadField);
		httpHeadField = NULL;

		ret = httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0);
		if(ret < 0){

		}
	}
	return 0;
}


int PIECAKE_live_stream(LP_HTTP_CONTEXT context)
{
	int i = 0;
	int ret = 0;
	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	int httpHeadLength = 0;
	char httpHeadBuf[1024] = {""};
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	const char *session = httpHeadField->read_tag(httpHeadField, "Session");
	bool sessionPass = false;

	if(NULL != session && 0 == piecake_check_session(session)){

		uint32_t static mediaBufSpeed = 0;
		int mediaBufID = 2; // FIXME:
		lpMEDIABUF_USER mediaBufUser = MEDIABUF_attach(mediaBufID);
		
		if(NULL != mediaBufUser){
			mediaBufSpeed = MEDIABUF_in_speed(mediaBufID);
			// response http header
			httpHeadField = HTTP_UTIL_new_response_head_200(kH_TAG_SERVER|kH_TAG_DATE, false, "application/piecake-stream", -1);
			httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
			httpHeadField->free(httpHeadField);
			httpHeadField = NULL;

			// send out the header
			ret = httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0);
			if(ret < 0){
				APP_TRACE("send error %s", strerror(errno));
				return -1;
			}

			sessionPass = true;
			while(*context->trigger){
				bool mediaBufOut = false;
				if(0 == MEDIABUF_out_lock(mediaBufUser)){
					lpSDK_ENC_BUF_ATTR attr = NULL;
					int outDataLen = 0;
					
					if(0 == MEDIABUF_out(mediaBufUser, &attr, NULL, &outDataLen)){
						const void *rawData = (void *)(attr + 1);
						int const rawDataLen = attr->data_sz;
						int pieCakeHeaderLen = 0;
						
						if(kSDK_ENC_BUF_DATA_H264 == attr->type){
							pieCakeHeaderLen = piecake_video_header(attr->timestamp_us / 1000, attr->data_sz,
								"H.264", "baseline",
								attr->h264.width, attr->h264.height, attr->h264.fps, attr->h264.keyframe,
								httpHeadBuf, sizeof(httpHeadBuf));

							// send out the header
							ret = httpTCP->send2(httpTCP, httpHeadBuf, pieCakeHeaderLen, 0);
							if(ret < 0){
								break;
							}

							// send out the data
							ret = httpTCP->send2(httpTCP, rawData, rawDataLen, 0);
							if(ret < 0){
								break;
							}
					
						}else if(kSDK_ENC_BUF_DATA_G711A == attr->type){
							pieCakeHeaderLen = piecake_audio_header(attr->timestamp_us / 1000, attr->data_sz,
								"G.711alaw", 8000, 16,
								httpHeadBuf, sizeof(httpHeadBuf));

							// send out the header
							ret = httpTCP->send2(httpTCP, httpHeadBuf, pieCakeHeaderLen, 0);
							if(ret < 0){
								break;
							}

							// send out the data
							ret = httpTCP->send2(httpTCP, rawData, rawDataLen, 0);
							if(ret < 0){
								break;
							}
						}

						mediaBufOut = true;
					}
					MEDIABUF_out_unlock(mediaBufUser);
				}

				if(!mediaBufOut){
					usleep(mediaBufSpeed / 2);
				}
			}

			MEDIABUF_detach(mediaBufUser);
			mediaBufUser = NULL;
		}
	}

	if(!sessionPass){
		httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", 403, NULL);
		httpHeadField->add_tag_server(httpHeadField, "JAWS/1.0");
		httpHeadField->add_tag_date(httpHeadField, 0);
		httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
		httpHeadField->free(httpHeadField);
		httpHeadField = NULL;

		ret = httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0);
		if(ret < 0){

		}

	}

	APP_TRACE("Exit PIE-CAKE");
	return 0;
}

int PIECAKE_ptz(LP_HTTP_CONTEXT context)
{
	int ret = -1;
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);

	char httpHeadBuf[1024];
	int httpHeadLength = 0;
	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	const char *session = httpHeadField->read_tag(httpHeadField, "Session");
	bool sessionPass = false;
	LP_HTTP_QUERY_PARA_LIST query = HTTP_UTIL_parse_query_as_para(httpHeadField->query);

	const char *av_step, *av_act, *av_speed;
	int n_step = -1, n_speed = -1;
	char *act = NULL;	

	if(NULL != session && 0 == piecake_check_session(session)){
		if(HTTP_IS_GET(context)){
			av_step = query->read(query, "step");
			if(av_step){
				n_step = atoi(av_step);
			}
			av_act = query->read(query, "act");
			if(av_act){
				act = av_act;
			}
			av_speed = query->read(query, "speed");
			if(av_speed){
				n_speed = atoi(av_speed);
				if(n_speed > 63){
					n_speed == 63;
				}
			}
			APP_TRACE("PieCake PTZ control step=%d act=%s speed=%d",n_step, act, n_speed);
			PTZ_Send(0, PTZ_Cmd_str2int(act), n_speed*5/63);
			
			sessionPass = true;

		}
	}

	httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", sessionPass ? 200 : 403, NULL);
	if(NULL != httpHeadField){
		httpHeadField->add_tag_server(httpHeadField, "JAWS/1.0");
		httpHeadField->add_tag_date(httpHeadField, 0);
		httpHeadField->add_tag_int(httpHeadField, "Content-Length", 0, true);
		httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
		httpHeadField->free(httpHeadField);
		httpHeadField = NULL;

		ret = httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0);
		if(ret < 0){

		}
	}
	
	return 0;
}

int PIECAKE_login(LP_HTTP_CONTEXT context)
{
	int ret = -1;
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	char httpBuf[1024];
	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	int httpHeadLength = 0;
	LP_HTTP_QUERY_PARA_LIST query = HTTP_UTIL_parse_query_as_para(httpHeadField->query);
	bool loginSuccess = false;
	char pieCakeSession[128] = {""};

	if(0 == _pieCakeSignature){
		_pieCakeSignature = abs(rand());
	}
	
	if(NULL != query){
		const char *authorization = query->read(query, "authorization");
		if(NULL != authorization){
			char decryptBuf[128];
			if(frank_decrypt(authorization, strlen(authorization), decryptBuf, sizeof(decryptBuf)) > 0){
				APP_TRACE("decrypt = %s", decryptBuf);
				// FIXME:
				if(STR_THE_SAME(decryptBuf, "PieCake:" TEST_USERNAME ":" TEST_PASSWORD)){
					// pass
					// generate the session
					char encryptBuf[128];
					snprintf(pieCakeSession, sizeof(pieCakeSession), "%x:%s:%s", _pieCakeSignature, TEST_USERNAME, TEST_PASSWORD);
					if(frank_encrypt(pieCakeSession, strlen(pieCakeSession), encryptBuf, sizeof(encryptBuf)) > 0){
						loginSuccess = true;
						strcpy(pieCakeSession, encryptBuf);
						APP_TRACE("Login Session: \"%s\"", pieCakeSession);
					}
				}
			}
		}
		query->free(query);
		query = NULL;
	}

	if(loginSuccess){
		APP_TRACE("PieCake Login Success");
	}else{
		APP_TRACE("PieCake Login Failed");
	}

	httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", loginSuccess ? 200 : 403, NULL);
	if(NULL != httpHeadField){
		httpHeadField->add_tag_server(httpHeadField, "JAWS/1.0");
		httpHeadField->add_tag_date(httpHeadField, 0);
		httpHeadField->add_tag_int(httpHeadField, "Content-Length", 0, true);
		if(loginSuccess){
			httpHeadField->add_tag_text(httpHeadField, "Session", pieCakeSession, true);
		}
		httpHeadLength = httpHeadField->to_text(httpHeadField, httpBuf, sizeof(httpBuf));
		httpHeadField->free(httpHeadField);
		httpHeadField = NULL;

		ret = httpTCP->send2(httpTCP, httpBuf, httpHeadLength, 0);
		if(ret < 0){

		}
	}
	return 0;
}

#if defined(SDCARD)
#include "sdcard/sdcard.h"
int PIECAKE_sdcard_media_search(LP_HTTP_CONTEXT context)
{
	// channelID
	// channelIDs
	// begin UTC
	// end UTC
	// type
	//
		
	int i = 0;
	int endUTC = -1, beginUTC = -1;
	
	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	LP_HTTP_QUERY_PARA_LIST httpQuery = HTTP_UTIL_parse_query_as_para(httpHeadField->query);
	
	const char *session = httpHeadField->read_tag(httpHeadField, "Session");
	char *queryBeginUTC = httpQuery->read(httpQuery, "beginUTC");
	char *queryEndUTC = httpQuery->read(httpQuery, "endUTC");
	
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	char httpHeadBuf[2 * 1024];
	int httpHeadLength = 0;
	char httpContentBuf[16 * 1024];
	char *httpContentOffset = httpContentBuf;
	int httpContentMax = sizeof(httpContentBuf);
	int httpContentLength = 0;
	int httpStatusCode = kHTTP_STAT_OK;

	// get the channel ID paramenter
	// FIXME:
	

	// get the UTC paramenter
	if(NULL != queryBeginUTC){
		beginUTC = atoi(queryBeginUTC);
	}
	if(NULL != queryEndUTC){
		endUTC = atoi(queryEndUTC);
	}
	/*
	if(-1 == beginUTC && -1 == endUTC){
		endUTC = time(NULL);
		beginUTC = endUTC - 60 * 60;
	}
	*/
	
	if(1){//if(NULL != session && 0 == piecake_check_session(session)){
		ST_SDCARD_MEDIA_SESSION sessionSearch[1024];
		int const sessionCnt = SDCARD_db_media_search_by_time(1, beginUTC, endUTC, sessionSearch, sizeof(sessionSearch));
		_TRACE("Session Count = %d", sessionCnt);
		
		if(sessionCnt >= 0){
			char xmlHead[] =
				"<PieCake version=\"1.0\">"
					"<record>";
			char xmlFormat[] =
				"<session>"
					"<sessionID>%d</sessionID>"
					"<channelID>%d</channelID>"
					"<beginUTC>%d</beginUTC>"
					"<endUTC>%d</endUTC>"
					"<type>%s</type>"
				"</session>";
			char xmlTail[] = 
					"</record>"
				"</PieCake>";
			int const xmlTailLength = strlen(xmlTail);

			strcpy(httpContentBuf, xmlHead);
			httpContentLength = strlen(httpContentBuf);
			// init the content offset and content max size
			httpContentOffset = httpContentBuf + httpContentLength;
			httpContentMax -= httpContentLength;
			
			for(i = 0; i < sessionCnt; ++i){
				LP_SDCARD_MEDIA_SESSION const session = &sessionSearch[i];
				char oneSessionBuf[1024];
				int const oneSessionLength = snprintf(httpContentOffset, httpContentMax, xmlFormat,
					session->sessionID,
					session->channelID,
					session->beginUTC, session->endUTC,
					session->type);
				
				httpContentOffset += oneSessionLength;
				httpContentMax -= oneSessionLength;

				// check the buffer full
				if(httpContentMax + xmlTailLength >= sizeof(httpContentBuf)){
					_TRACE("HTTP Buffer Size too Short!");
					break;
				}
			}
			strcat(httpContentBuf, xmlTail);
			httpContentLength = strlen(httpContentBuf);

			_TRACE("%s", httpContentBuf);	

			//httpHeadField = HTTP_UTIL_new_response_head_200(kH_TAG_DATE | kH_TAG_SERVER, 0, "text/xml", httpContentLength);
			httpStatusCode = kHTTP_STAT_OK;
		}else{
			//httpHeadField = HTTP_UTIL_new_response_head_200(kH_TAG_DATE | kH_TAG_SERVER, 0, NULL, 0);
			httpStatusCode = 503;
		}
			
	}else{
		httpStatusCode = 403;
	}

	if(kHTTP_STAT_OK == httpStatusCode){
		httpHeadField = HTTP_UTIL_new_response_head_200(kH_TAG_DATE | kH_TAG_SERVER, 0, "text/xml", httpContentLength);
	}else{
		httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", httpStatusCode, NULL);
		httpHeadField->add_tag_server(httpHeadField, "JAWS/1.0");
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

	return -1;
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

					}else if(1 == framerVideo->channelSubID){
						if(framerVideo->keyFrame && !flvKeyFrameRequest){
							flv->set_video(flv, framerVideo->resolutionWidth, framerVideo->resolutionHeight, framerVideo->frameRate, 4000);
							flv->set_audio(flv);
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


static int playback_over_piecake(LP_HTTP_CONTEXT context, int *fIDs, int nFIDs, int sessionID)
{
	int i = 0;
	int err = 0;
	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	LP_HTTP_QUERY_PARA_LIST httpQuery = HTTP_UTIL_parse_query_as_para(httpHeadField->query);

	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp2_r(context->sock, &sockTCP);
	
	int sendN = 0, readN = 0;

	char httpHeadBuf[2 * 1024];
	int httpHeadLength = 0;
	
	char payload[512 * 1024];
	ST_SDCARD_FRAMER framer;

	bool keyFrameRequest = false;

	// make an HTTP head and send the packet out
	httpHeadField = HTTP_UTIL_new_response_head_200(kH_TAG_DATE | kH_TAG_SERVER, false, "application/piecake-stream", -1);
	httpHeadLength = httpHeadField->to_text(httpHeadField, httpHeadBuf, sizeof(httpHeadBuf));
	httpHeadField->free(httpHeadField);
	httpHeadField = NULL;

	// send out http header
	sendN = httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0);
	if(sendN == httpHeadLength){
		// send out content
		for(i = 0; i < nFIDs && *context->trigger && 0 == err; ++i){
			while(0 == err
				&& *context->trigger
				&& (readN = SDCARD_media_read(fIDs[i], &framer, payload, sizeof(payload))) > 0){
				
				if(kSDF_TYPE_VIDEO == framer.type){
					LP_SDCARD_FRAMER_VIDEO const framerVideo = &framer.video;
					if(sessionID != framerVideo->sessionID){
						continue;
					}
					if(1 == framer.video.channelSubID){
						if(framerVideo->keyFrame && !keyFrameRequest){
							keyFrameRequest = true;
						}

						if(keyFrameRequest){
							httpHeadLength = piecake_video_header(framer.timestampMS, framer.dataLength,
								"H.264", "baseline", framerVideo->resolutionWidth, framerVideo->resolutionHeight, framerVideo->frameRate, framerVideo->keyFrame,
								httpHeadBuf, sizeof(httpHeadBuf));

							// send out the header
							sendN = httpTCP->send2(httpTCP, httpHeadBuf, httpHeadLength, 0);
							if(sendN < 0){
								_TRACE("Send PieCake Head Failed!");
								err = errno;
							}

							// send out the data
							sendN = httpTCP->send2(httpTCP, payload, readN, 0);
							if(sendN < 0){
								_TRACE("Send PieCake Payload Failed!");
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

	httpQuery->free(httpQuery);
	httpQuery = NULL;
	return err;
}

int PIECAKE_sdcard_media_playback(LP_HTTP_CONTEXT context)
{
	static LP_MUTEX mutex = NULL;
	int i = 0;
	int endUTC = -1, beginUTC = -1;

	LP_HTTP_HEAD_FIELD httpHeadField = context->request_header;
	LP_HTTP_QUERY_PARA_LIST httpQuery = HTTP_UTIL_parse_query_as_para(httpHeadField->query);
	
	const char *tagSession = httpHeadField->read_tag(httpHeadField, "Session");
	
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

	if(1){//if(NULL != tagSession && 0 == piecake_check_session(tagSession)){
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
						err = playback_over_piecake(context, fIDs, fIDsN, sessionID);
						//err = playback_over_flv(context, fIDs, fIDsN, sessionID);
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
	}else{
		_TRACE("PieCake Session Error!");
		httpStatusCode = kHTTP_STAT_FORBIDDEN;
	}
	
	_TRACE("Playback HTTP Status %d: %s", httpStatusCode, httpContentBuf);
	if(0 == err && kHTTP_STAT_OK != httpStatusCode){
		
		
		httpHeadField = HTTP_UTIL_new_response_header(NULL, "1.1", httpStatusCode, NULL);
		httpHeadField->add_tag_server(httpHeadField, "JAWS/1.0");
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

#endif //define (SDCARD)
