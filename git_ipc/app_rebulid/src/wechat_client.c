#include "securedat.h"
#include "wechat_client.h"
#include "socket_tcp.h"
#include "sdk/sdk_api.h"
#include "generic.h"
#include "app_debug.h"

#define WECHAT_SERVER_IP "115.28.14.182"
//#define WECHAT_SERVER_IP "210.21.39.197"
#define WECHAT_SERVER_PORT (80)
uint32_t wechat_devid = 0;

typedef struct WECHAT_CLIENT {
	char serverIP[128];
	int serverPort;

	bool clientIsON;
	pthread_t clientTID;

	int helloInterval;
	bool helloResponse;
	time_t helloTimeStamp;
	
}ST_WECHAT_CLIENT, *LP_WECHAT_CLIENT;
static ST_WECHAT_CLIENT _weChatClient;
static LP_WECHAT_CLIENT _lpWeChatClient = NULL;


static int wechat_say_hello_to_server()
{
	int ret = -1;
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp_r(&sockTCP);
	bool successToSayHello = false;
	char httpBuf[2048];
	int sendn = 0, recvn = 0;
	int httpHeadLength = 0;
	char query_buf[1024] = {0};
	snprintf(query_buf, sizeof(query_buf),
		"deviceID=%d&encrypt=iLoveFrankSoMuch!!&deviceUserTable=@@{\"admin\":\"\",}",
		wechat_devid);
	LP_HTTP_HEAD_FIELD httpHeadField = HTTP_UTIL_new_request_header(NULL, "1.1", "GET",
		"/WeChat/Device/Hello",query_buf);
	if(NULL != httpHeadField){
		httpHeadLength = httpHeadField->to_text(httpHeadField, httpBuf, sizeof(httpBuf));
		httpHeadField->free(httpHeadField);
		httpHeadField = NULL;
		
		if(0 == httpTCP->connect(httpTCP, WECHAT_SERVER_IP, WECHAT_SERVER_PORT)){
			//APP_TRACE("Say hello:\r\n%s", httpBuf);
			if(httpTCP->send2(httpTCP, httpBuf, httpHeadLength, 0) == httpHeadLength){
				// get response
				int flag = kT_SELECT_READ;
				ret = httpTCP->select(httpTCP, &flag, 5, 0);
				if(ret > 0){
					LP_HTTP_HEAD_FIELD responseHeadField = HTTP_UTIL_recv_response_header(httpTCP->sock);
					if(NULL != responseHeadField){
						responseHeadField->dump(responseHeadField);
						// receive content
						ret = httpTCP->recv(httpTCP, httpBuf, sizeof(httpBuf), 0);
						if(ret > 0){
							httpBuf[ret] = '\0';
							//APP_TRACE("Hello Response \"%s\"", httpBuf);
							if(STR_THE_SAME(httpBuf, "Hello")){
								successToSayHello = true;
							}
						}
						responseHeadField->free(responseHeadField);
						responseHeadField = NULL;
					}
				}
			}
		}
	}
	usleep(10000);
	httpTCP->close(httpTCP);
	httpTCP = NULL;
	return successToSayHello ? 0 : -1;
}

static int wechat_upload_file_to_server(const char *filePath, const char *mime)
{
	int ret = -1;
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp_r(&sockTCP);
	bool successUploadFile = false;
	int fileSize = 0;
	GET_FILE_SIZE(filePath, fileSize);
	
	if(fileSize > 0){
		char httpBuf[2048];
		int httpHeadLength = 0;
		char query_buf[1024] = {0};
	snprintf(query_buf, sizeof(query_buf),
		"deviceID=%d&encrypt=iLoveFrankSoMuch!!",
		wechat_devid);
		LP_HTTP_HEAD_FIELD httpHeadField = HTTP_UTIL_new_request_header(NULL, "1.1", "PUT",
			"/WeChat/Device/Upload", query_buf);

		if(NULL != httpHeadField){
			// make http head
			httpHeadField->add_tag_server(httpHeadField, "IE/10.0");
			httpHeadField->add_tag_date(httpHeadField, 0);
			httpHeadField->add_tag_text(httpHeadField, "Content-Type", mime, true);
			httpHeadField->add_tag_int(httpHeadField, "Content-Length", fileSize, true);
			httpHeadField->add_tag_text(httpHeadField, "Session", "iLoveFrankSoMuch!!", true);
			httpHeadLength = httpHeadField->to_text(httpHeadField, httpBuf, sizeof(httpBuf));
			httpHeadField->free(httpHeadField);
			httpHeadField = NULL;

			APP_TRACE("PUT image:\r\n%s", httpBuf);

			// connect and send out
			if(0 == httpTCP->connect(httpTCP, WECHAT_SERVER_IP, WECHAT_SERVER_PORT)){
				if(httpTCP->send2(httpTCP, httpBuf, httpHeadLength, 0) == httpHeadLength){
					// sent out file
					int sentN = 0, recvN = 0;
					FILE *fid = NULL;
					bool contentSent = true;
					fid = fopen(filePath, "r+b");
					if(NULL != fid){
						char fileBuf[1024];
						int readn = 0;
						while((recvN = fread(fileBuf, 1, sizeof(fileBuf), fid)) > 0){
							sentN = httpTCP->send2(httpTCP, fileBuf, recvN, 0);
							if(sentN != recvN){
								// do something
								contentSent = false;
							}
						}
						fclose(fid);
					}
					
					// get response
					if(contentSent){
						int flag = kT_SELECT_READ;
						ret = httpTCP->select(httpTCP, &flag, 5, 0);
						if(ret > 0){
							LP_HTTP_HEAD_FIELD responseHeadField = HTTP_UTIL_recv_response_header(httpTCP->sock);
							if(NULL != responseHeadField){
								responseHeadField->dump(responseHeadField);
								// receive content
								ret = httpTCP->recv(httpTCP, httpBuf, sizeof(httpBuf), 0);
								if(ret > 0){
									httpBuf[ret] = '\0';
									APP_TRACE("Upload File Response \"%s\"", httpBuf);
									if(STR_THE_SAME(httpBuf, "Good!")){
										successUploadFile = true;
									}
								}
								responseHeadField->free(responseHeadField);
								responseHeadField = NULL;
							}
						}
					}
				}
			}
		}
	}
	
	usleep(10000);
	httpTCP->close(httpTCP);
	httpTCP = NULL;
	return successUploadFile ? 0 : -1;
}




static void* wechat_upload_image_to_server(void *args)
{
	int ret = 0;
	FILE *fid = NULL;
	char fileName[128];
	pthread_detach(pthread_self());
	snprintf(fileName, sizeof(fileName), "/tmp/WeChat%08x%08x%08x.jpg", time(NULL), getpid(), rand());
	fid = fopen(fileName, "w+b");
	if(NULL != fid){
		if(0 == sdk_enc->snapshot(0, kSDK_ENC_SNAPSHOT_QUALITY_HIGH, 320, 240, fid)){
			//GET_FILE_SIZE(fileName, fileSize);
		}
		fclose(fid);
		fid = NULL;
	}

	ret = wechat_upload_file_to_server(fileName, "image/jpeg");
	REMOVE_FILE(fileName);
	APP_TRACE("upload %s@(%08x, %08x)",(ret == 0) ? "success" : "failed", getpid(), pthread_self());
	pthread_exit(NULL);
	return NULL;
}


static int wechat_upload_video_to_server()
{
	int ret = 0;
	FILE *fid = NULL;
	char fileName[128];

	snprintf(fileName, sizeof(fileName), "/tmp/WeChat%08x%08x%08x.avi", time(NULL), getpid(), rand());
	fid = fopen(fileName, "w+b");
	if(NULL != fid){
		if(1 /*capture a video*/){
		
		}
		fclose(fid);
		fid = NULL;
	}

	ret = wechat_upload_file_to_server(fileName, "image/jpeg");
	REMOVE_FILE(fileName);

	return ret;
}


static bool _weChatUploadImage = false;

static void *wechat_task_listener(void *arg)
{
	
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP httpTCP = socket_tcp_r(&sockTCP);
	APP_TRACE("wechat client start running! (%p && %p)\n", _lpWeChatClient, _lpWeChatClient->clientIsON);
	APP_TRACE("interval = %d\n",_lpWeChatClient->helloInterval);
	while(_lpWeChatClient && _lpWeChatClient->clientIsON){
		time_t curTimeStamp = time(NULL);
		if(curTimeStamp - _lpWeChatClient->helloTimeStamp >= _lpWeChatClient->helloInterval){
			// say hello to server
			if(0 == wechat_say_hello_to_server()){
				APP_TRACE("Success to Say Hello to WeChat Server!");
				_lpWeChatClient->helloInterval = 100;//300; // FIXME:
			}else{
				APP_TRACE("Failed to Say Hello to WeChat Server!");
				_lpWeChatClient->helloInterval = 10; // try in a short while
			}
			_lpWeChatClient->helloTimeStamp = curTimeStamp;
			APP_TRACE("wechat client is running!\n");
		}
		// the other task
		if(curTimeStamp == _lpWeChatClient->helloTimeStamp){
			//wechat_upload_image_to_server();
		}
		sleep(1);
	}
	// close socket
	httpTCP->close(httpTCP);
	httpTCP = NULL;
	pthread_exit(NULL);
}

//
int WECHAT_CLIENT_put_image(void)
{
	if(_lpWeChatClient->clientIsON){
		pthread_t tid_image;
		pthread_create(&tid_image, NULL, wechat_upload_image_to_server, NULL);
	}	
	return 0;
}

int WECHAT_CLIENT_init(char serverIP[128], int serverPort)
{
	int ret = 0;
	char devid_buf[32] = {0};
	if(0 == UC_SNumberGet(devid_buf)) {
	}
	wechat_devid = atoi(&devid_buf[strlen(devid_buf) - 10]);
	APP_TRACE("wechat client get id:%d\n", wechat_devid);

	if(!_lpWeChatClient){
		_lpWeChatClient = &_weChatClient;
		// init element
		_lpWeChatClient->helloInterval = 10; // default second
		_lpWeChatClient->helloResponse = false;
		_lpWeChatClient->helloTimeStamp = 0;

	
		// start task listener
		_lpWeChatClient->clientIsON = true;
		ret = pthread_create(&_lpWeChatClient->clientTID, NULL, wechat_task_listener, NULL);
		usleep(50000);
		return 0;
	}
	return -1;
}

void WECHAT_CLIENT_destroy()
{
	if(_lpWeChatClient){
		// stop task listner
		_lpWeChatClient->clientIsON = false;
		pthread_join(_lpWeChatClient->clientTID, NULL);
		_lpWeChatClient->clientTID = (pthread_t)NULL;

		// cleanup
		_lpWeChatClient = NULL;
	}
}

