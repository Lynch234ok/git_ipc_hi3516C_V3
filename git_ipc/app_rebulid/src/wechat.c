
#include "wechat.h"
#include "ezxml.h"
#include "socket_tcp.h"
#include "app_debug.h"

#define kWECHAT_TOKEN "CplusplusRonaldoApplication"

typedef struct WECHAT_REQUEST_CONTEXT {	
/*
<xml>
	<ToUserName><![CDATA[gh_49018701783f]]></ToUserName>
	<FromUserName><![CDATA[oPfSPjqUOkF-kJ5QUVNZGEaIu7V8]]></FromUserName>
	<CreateTime>1379312431</CreateTime>
	<MsgType><![CDATA[text]]></MsgType>
	<Content><![CDATA[yygdd]]></Content>
	<MsgId>5924101782111256639</MsgId>
</xml>
*/
	char toUserName[64];
	char fromUserName[64];
	time_t createTime;
	char msgType[32];

	union {
		// text
		struct {
			char content[128];
		}text;

		// picture
		struct {
			char picURL[128];
		}image;

		// geolocation
		struct {
			float x, y;
			int scale;
			char label[128];
		}location;

		// link
		struct {
			char title[128];
			char description[256];
			char url[256];
		}link;

		// event
		struct {
			char event[32];
			char eventKey[32];
		}event;
	};

	char msgID[128];	
}ST_WECHAT_REQUEST_CONTEXT, *LP_WECHAT_REQUEST_CONTEXT;

static int wechat_message_response(int sock, const char *contentType, void *content, int contentLen)
{
	int httpBufLen = 0;
	int httpBufMax = 2048 + contentLen;
	char * const httpBuf = alloca(httpBufMax);
	ST_SOCKET_TCP sockTCP;
	LP_SOCKET_TCP tcp = socket_tcp2_r(sock, &sockTCP);
	
	int httpHeaderLen = 0;
	LP_HTTP_HEAD_FIELD httpHeader = NULL;
	
	httpHeader = HTTP_UTIL_new_response_header(NULL, "1.1", 200, NULL);
	httpHeader->add_tag_server(httpHeader, "IE/10.0");
	//httpHeader->add_tag_date(httpHeader, 0);
	httpHeader->add_tag_text(httpHeader, "Content-Type", contentType, true);
	//httpHeader->add_tag_text(httpHeader, "Connection", "keep-alive", true);     
	httpHeader->add_tag_text(httpHeader, "Connection", "close", true);      
	httpHeader->add_tag_int(httpHeader, "Content-Length", contentLen, true);
	httpHeaderLen = httpHeader->to_text(httpHeader, httpBuf, httpBufMax);
	httpHeader->free(httpHeader);
	httpHeader = NULL;

	// add the content
	httpBufLen = httpHeaderLen + contentLen;
	// offset to content
	memcpy(httpBuf + httpHeaderLen, content, contentLen);
	httpBuf[httpBufLen] = '\0';

	//APP_TRACE("Length = %d\r\n\"%s\"", httpBufLen, httpBuf);

	// send out the packet
	return tcp->send2(tcp, httpBuf, httpBufLen, 0);
}

// for get method
static int wechat_message_joinup(LP_HTTP_CONTEXT context, LP_HTTP_QUERY_PARA_LIST queryList)
{
	int i = 0, ii = 0;
	if(HTTP_IS_GET(context)){
		const char *token = kWECHAT_TOKEN;
		const char *signature = queryList->read(queryList, "signature");
		const char *timestamp = queryList->read(queryList, "timestamp");
		const char *nonce = queryList->read(queryList, "nonce");
		const char *echostr = queryList->read(queryList, "echostr");

		if(NULL != token && NULL != signature && NULL != timestamp && NULL != nonce && NULL != echostr){
			/*
			const char *group[3] = {
				[0] = token,
				[1] = timestamp,
				[2] = nonce,
			};
			int const combinationMax = 1 + strlen(token) + strlen(timestamp) + strlen(nonce);
			char *combination = alloca(combinationMax);
			
			for(i = 0; i < sizeof(group) / sizeof(group[0]) - 1; ++i){
				for(ii = 0; ii < sizeof(group) / sizeof(group[0]) - 1; ++ii){
					const char *tmp = NULL;
					if(strcasecmp(group[ii], group[ii + 1]) >= 0){
						tmp = group[ii];
						group[ii] = group[ii + 1];
						group[ii + 1] = tmp;
					}
				}
			}
			for(i = 0; i < sizeof(group) / sizeof(group[0]); ++i){
				strncat(combination, group[i], combinationMax - strlen(combination));
			}

			APP_TRACE("%s -- %s -- %s", group[0], group[1], group[2]);
			APP_TRACE("Signature == %s", combination);
			APP_TRACE("Signature -- %s", signature);

			FILE *fid = fopen("/tmp/signature.bin", "w+b");
			if(NULL != fid){
				fwrite(combination, 1, strlen(combination), fid);

				fclose(fid);
				fid = NULL;
			}

			system("sha1sum /tmp/signature.bin");
			system("cat /tmp/signature.bin");

			APP_TRACE("WeChart Authorization!!");
			*/
			// FIXME:
			wechat_message_response(context->sock, "application/octet-stream", echostr, strlen(echostr));

			return 0;
		}
	}
	return -1;
}


// for post method
static int wechat_message_response_text(LP_HTTP_CONTEXT context, LP_WECHAT_REQUEST_CONTEXT weChat, const char *text)
{
	if(HTTP_IS_POST(context)){
		char xmlBuf[1024] = {""};
		int const xmlLen = snprintf(xmlBuf, sizeof(xmlBuf),
			"<xml>" kCRLF
			"<ToUserName><![CDATA[%s]]></ToUserName>" kCRLF
			"<FromUserName><![CDATA[%s]]></FromUserName>" kCRLF
			"<CreateTime>%d</CreateTime>" kCRLF
			"<MsgType><![CDATA[text]]></MsgType>" kCRLF
			"<Content><![CDATA[%s]]></Content>" kCRLF
			"</xml>",
			weChat->fromUserName,
			weChat->toUserName,
			weChat->createTime,
			text);
		//APP_TRACE("buf = %d/%d", strlen(xmlBuf), xmlLen);
		//char command[4096];
		//snprintf(command, sizeof(command), "echo \"%s\" > wechat.xml", xmlBuf);
		//system(command);
		return wechat_message_response(context->sock, "application/xml", xmlBuf, xmlLen);
	}
	return -1;
}

static int wechat_message_response_news(LP_HTTP_CONTEXT context, LP_WECHAT_REQUEST_CONTEXT weChat,
	const char *newsTitle, const char *newsDescription, const char *newPicUrl, const char *newsUrl)
{
	if(HTTP_IS_POST(context)){
		char xmlBuf[1024] = {""};
		int const xmlLen = snprintf(xmlBuf, sizeof(xmlBuf),
			"<xml>" kCRLF
				"<ToUserName><![CDATA[%s]]></ToUserName>" kCRLF
				"<FromUserName><![CDATA[%s]]></FromUserName>" kCRLF
				"<CreateTime>%d</CreateTime>" kCRLF
				"<MsgType><![CDATA[news]]></MsgType>" kCRLF
				"<ArticleCount>1</ArticleCount>" kCRLF
				"<Articles>" kCRLF
					"<item>" kCRLF
						"<Title><![CDATA[%s]]></Title>" kCRLF
						"<Description><![CDATA[%s]]></Description>" kCRLF
						"<PicUrl><![CDATA[%s]]></PicUrl>" kCRLF
						"<Url><![CDATA[%s]]></Url>" kCRLF
					"</item>" kCRLF
				"</Articles>" kCRLF
			"</xml>",
			weChat->fromUserName,
			weChat->toUserName,
			weChat->createTime,
			newsTitle, newsDescription, newPicUrl, newsUrl);
		//char command[4096];
		//snprintf(command, sizeof(command), "echo \"%s\" > wechat.xml", xmlBuf);
		//system(command);
		//APP_TRACE("News Length %d:\r\n%s", xmlLen, xmlBuf);
		
		return wechat_message_response(context->sock, "application/xml", xmlBuf, xmlLen);
	}
	return -1;
}

static int wechat_message_text_listener(LP_HTTP_CONTEXT context, LP_WECHAT_REQUEST_CONTEXT weChat)
{
	int ret = 0;
	char picURL[64] = {""}, linkURL[64] = {""};
	snprintf(picURL, sizeof(picURL), "http://210.21.39.197/snapshot?size=395x220&nonce=%d", time(NULL));
	snprintf(linkURL, sizeof(linkURL), "http://210.21.39.197/snapshot?size=320x240&nonce=%d", time(NULL));
	//APP_TRACE("%s", picURL);

	//ret = wechat_message_response_text(context, &weChat, "yyo");
	ret = wechat_message_response_news(context, weChat, "Alarm", "There is an alarm occurs!",
		picURL, linkURL);
	return 0;
}

static int wechat_message_image_listener(LP_HTTP_CONTEXT context, LP_WECHAT_REQUEST_CONTEXT weChat)
{
	int ret = 0;
	APP_TRACE("Get a Picture URL: %s", weChat->image.picURL);
	ret = wechat_message_response_text(context, weChat, "Thank you for your picture so much /::)!");
	return 0;
}

static int wechat_message_location_listener(LP_HTTP_CONTEXT context, LP_WECHAT_REQUEST_CONTEXT weChat)
{
	int ret = 0;
	//APP_TRACE("Request Event: \"%s\"", weChat->event.eventKey);
	char text[128] = {""};
	snprintf(text, sizeof(text), "I found you /::> You are so shine! (%.1f%c %.1f%c) %s",
		weChat->location.x, weChat->location.x > 0 ? 'N' : 'S', weChat->location.y, weChat->location.y > 0 ? 'E' : 'W',
		weChat->location.label);
	ret = wechat_message_response_text(context, weChat, text);
	return 0;
}

static int wechat_message_link_listener(LP_HTTP_CONTEXT context, LP_WECHAT_REQUEST_CONTEXT weChat)
{
	int ret = 0;
	//APP_TRACE("Request Event: \"%s\"", weChat.event.eventKey);
	ret = wechat_message_response_text(context, weChat, "Thank you for your message!");
	return 0;
}


static int wechat_message_voice_listener(LP_HTTP_CONTEXT context, LP_WECHAT_REQUEST_CONTEXT weChat)
{
	int ret = 0;
	//APP_TRACE("Request Event: \"%s\"", weChat.event.eventKey);
	ret = wechat_message_response_text(context, weChat, "Nice voice, I enjoy it so much! /::>");
	return 0;
}

static int wechat_message_event_listener(LP_HTTP_CONTEXT context, LP_WECHAT_REQUEST_CONTEXT weChat)
{
	int ret = 0;
	APP_TRACE("Request Event: \"%s\"", weChat->event.eventKey);
	ret = wechat_message_response_text(context, weChat, "Hello! I'm C++Ronaldo, Nice to meet you!");
	return 0;
}

int WECHAT_http_service(LP_HTTP_CONTEXT context)
{
	int ret = 0;
	LP_HTTP_QUERY_PARA_LIST queryList = NULL;
	queryList = HTTP_UTIL_parse_query_as_para(context->request_header->query);
	
	if(HTTP_IS_GET(context)){
		// WeChat platform join up
		ret = wechat_message_joinup(context, queryList);
	}else if(HTTP_IS_POST(context)){
		ezxml_t xml = ezxml_parse_str(context->request_content, strlen(context->request_content));
		if(NULL != xml){
			ezxml_t xmlToUserName = ezxml_child(xml, "ToUserName");
			ezxml_t xmlFromUserName = ezxml_child(xml, "FromUserName");
			ezxml_t xmlCreateTime = ezxml_child(xml, "CreateTime");
			ezxml_t xmlMsgType = ezxml_child(xml, "MsgType");
			ezxml_t xmlMsgID = ezxml_child(xml, "MsgId");
			ST_WECHAT_REQUEST_CONTEXT weChat;

			memset(&weChat, 0, sizeof(weChat));
			strncpy(weChat.toUserName, xmlToUserName ? ezxml_txt(xmlToUserName) : "", sizeof(weChat.toUserName));
			strncpy(weChat.fromUserName, xmlFromUserName ? ezxml_txt(xmlFromUserName) : "", sizeof(weChat.fromUserName));
			weChat.createTime = xmlCreateTime ? atoi(ezxml_txt(xmlCreateTime)) : 0;
			strncpy(weChat.msgType, xmlMsgType ? ezxml_txt(xmlMsgType) : "", sizeof(weChat.msgType));
			strncpy(weChat.msgID, xmlMsgID ? ezxml_txt(xmlMsgID) : "", sizeof(weChat.msgID));

			if(0 == strcasecmp("text", weChat.msgType)){
				ezxml_t xmlContent = ezxml_child(xml, "Content");
				strncpy(weChat.text.content, xmlContent ? ezxml_txt(xmlContent) : "", sizeof(weChat.text.content));
				ret = wechat_message_text_listener(context, &weChat);
			}else if(0 == strcasecmp("image", weChat.msgType)){
				ezxml_t xmlPicURL = ezxml_child(xml, "PicUrl");
				strncpy(weChat.image.picURL, xmlPicURL ? ezxml_txt(xmlPicURL) : "", sizeof(weChat.image.picURL));
				ret = wechat_message_image_listener(context, &weChat);
			}else if(0 == strcasecmp("location", weChat.msgType)){
				ezxml_t xmlLocationX = ezxml_child(xml, "Location_X");
				ezxml_t xmlLocationY = ezxml_child(xml, "Location_Y");
				ezxml_t xmlScale = ezxml_child(xml, "Scale");
				ezxml_t xmlLabel = ezxml_child(xml, "Label");

				weChat.location.x = xmlLocationX ? atof(ezxml_txt(xmlLocationX)) : 0.0;
				weChat.location.y = xmlLocationY ? atof(ezxml_txt(xmlLocationY)) : 0.0;
				weChat.location.scale = xmlLabel ? atoi(ezxml_txt(xmlLabel)) : 0;
				strncpy(weChat.location.label, xmlLabel ? ezxml_txt(xmlLabel) : "", sizeof(weChat.location.label));
			
				ret = wechat_message_location_listener(context, &weChat);
				
			}else if(0 == strcasecmp("link", weChat.msgType)){
				ezxml_t xmlTitle = ezxml_child(xml, "Title");
				ezxml_t xmlDescription = ezxml_child(xml, "Description");
				ezxml_t xmlURL = ezxml_child(xml, "Url");

				strncpy(weChat.link.title, xmlTitle ? ezxml_txt(xmlTitle) : "", sizeof(weChat.link.title));
				strncpy(weChat.link.description, xmlDescription ? ezxml_txt(xmlDescription) : "", sizeof(weChat.link.description));
				strncpy(weChat.link.url, xmlURL ? ezxml_txt(xmlURL) : "", sizeof(weChat.text.content));
				
				ret = wechat_message_link_listener(context, &weChat);
			}else if(0 == strcasecmp("voice", weChat.msgType)){
				ret = wechat_message_voice_listener(context, &weChat);
			}else if(0 == strcasecmp("event", weChat.msgType)){
				ret = wechat_message_event_listener(context, &weChat);
			}else{
				APP_TRACE("Message Type : %s", weChat.msgType);
			}

			// free the xml
			ezxml_free(xml);
			xml = NULL;

		}
	}

	// free the query string list
	queryList->free(queryList);
	queryList = NULL;
	
	return ret;
}


