#include <stdint.h>
#include "HTTPClient.h"
#include "cJSON.h"
#include <string.h>
#include "NkUtils/md5.h"
//#include "md5.h"
#include "netsdk.h"

#define VERIFY_MIX_STR ("Japass^2>.j")


// 对返回的数据进行解读，包括分析状态码等动作
static int deal_result(HTTP_SESSION_HANDLE http,int nhasJson,cJSON **jsonObj)
{

	int nRetCode;
	HTTP_CLIENT info;
	HTTPClientGetInfo(http,&info);
	if ( 200 == info.HTTPStatusCode ||
		500 == info.HTTPStatusCode)
	{
		// 需要解析json对象的返回
		if (nhasJson)
		{
			if (NULL == jsonObj)
			{
				return -1;
			}

			// 读取数据
			// 解析json对象
			char readBuffer[1024]; // 一般情况下1K已经足够放nonce数据了，如果超出1K的情况，肯定是服务器内部出错了
			UINT32 nReadSize;
			// 默认使用同步读取的方式，交互已经在RecvResponse完成了，直接在这边读数据即可,超时设置为0
			nRetCode = HTTPClientReadData(http,readBuffer,sizeof(readBuffer),0,&nReadSize);
			if (HTTP_CLIENT_EOS != nRetCode)
			{
				return -1;
			}

			*jsonObj = cJSON_Parse(readBuffer);

			// 如果返回不是json，则表明请求失败
			if (NULL == *jsonObj)
			{
				return -1;
			}
		}

		return 0;
	}
	else
	{
		return -1;
	}
}

// 获取服务器权限码
static int get_nonce(cJSON **jsonObj)
{
	HTTP_SESSION_HANDLE http;
	UINT32 nRetCode;
	int nRet;
	char url[256] = "http://pm.dvr163.com/message/nonce?method=get";
	http = HTTPClientOpenRequest(0);
	if (0 == http)
	{
		return -1;
	}

	nRetCode = HTTPClientSendRequest(http,url,NULL,0,FALSE,0,0);
	if (HTTP_CLIENT_SUCCESS != nRetCode)
	{
		HTTPClientCloseRequest(&http);
		return -1;
	}

	// 3秒超时
	nRetCode = HTTPClientRecvResponse(http,3);
	if (HTTP_CLIENT_SUCCESS != nRetCode)
	{
		HTTPClientCloseRequest(&http);
		return -1;
	}

	nRet = deal_result(http,1,jsonObj);
	if ( 0 != nRet )
	{
		HTTPClientCloseRequest(&http);
		return -1;
	}

	HTTPClientCloseRequest(&http);

	return 0;
}

static int message_post(char * eseeid, int channel,
						char * verify, char * request_id,
						char * type, char * message, int64_t ts_s)
{
	char url[1024]={0};
	char sPost[2048] = {0};
	HTTP_SESSION_HANDLE http;
	UINT32 nRet;
	char sMessage[1024] = {0};

	sprintf(sMessage,"<message><alert>%s</alert><type>%s</type><time>%lld</time><id>%s</id><channel>%d</channel></message>",
		message,
		type,
		ts_s,
		eseeid,
		channel);
	sprintf(url,"http://pm.dvr163.com/message/message?method=post&eseeid=%s&verify=%s&request_id=%s",eseeid,verify,request_id);
	http = HTTPClientOpenRequest(0);
	if (0 == http)
	{
		return -1;
	}

	nRet = HTTPClientSetVerb(http,VerbPost);
	if (HTTP_CLIENT_SUCCESS != nRet)
	{
		HTTPClientCloseRequest(&http);
		return -1;
	}
	nRet = HTTPClientAddRequestHeaders(http,"Content-Type","application/x-www-form-urlencoded",TRUE);
	if (HTTP_CLIENT_SUCCESS != nRet)
	{
		HTTPClientCloseRequest(&http);
		return -1;
	}

	sprintf(sPost,"message=%s",sMessage);
	nRet = HTTPClientSendRequest(http,url,sPost,strlen(sPost),TRUE,0,0);
	if (HTTP_CLIENT_SUCCESS != nRet)
	{
		HTTPClientCloseRequest(&http);
		return -1;
	}

	// 3秒超时
	nRet = HTTPClientRecvResponse(http,3);
	if (HTTP_CLIENT_SUCCESS != nRet)
	{
		HTTPClientCloseRequest(&http);
		return -1;
	}

	nRet = deal_result(http,0,NULL);
	if ( 0 != nRet )
	{
		HTTPClientCloseRequest(&http);
		return -1;
	}

	HTTPClientCloseRequest(&http);

	return 0;
}

// 在同级中找第一个name的节点
cJSON * json_node(cJSON * node,char *name)
{
	cJSON * pRet = node;
	while (NULL != pRet)
	{
		if (!strcmp(pRet->string,name))
		{
			break;
		}
		pRet = pRet->next;
	}
	return pRet;
}

static char * strupr(char *sOrigin)
{
	// 未做边界检测
	char *pRead = sOrigin;
	while (*pRead)
	{
		if (*pRead>=0x61 && *pRead <= 0x7A)
		{
			*pRead -= 0x20;
		}
		pRead ++;
	}
}

int Esee_msg_send(char *eseeId, char * msg, char *type, int64_t ts_s)
{
	cJSON * nonceObj;
	char sMd5[64];
	memset(sMd5,0,sizeof(sMd5));

	if(NULL != eseeId && NULL != msg && NULL != type && strlen(eseeId) > 0){
		int nRet = get_nonce(&nonceObj);		
		if ( 0 == nRet)
		{
			cJSON * jsonRequest_id = json_node(nonceObj->child,"request_id");
			cJSON * jsonNonce = json_node(nonceObj->child,"nonce");
			char sVerifyPlain[128];
			sprintf(sVerifyPlain,"%s%s%s",jsonNonce->valuestring,eseeId,jsonRequest_id->valuestring);
			strupr(sVerifyPlain);
			sprintf(sVerifyPlain,"%s%s",sVerifyPlain,VERIFY_MIX_STR);
			//md5_encode(sVerifyPlain,strlen(sVerifyPlain),sMd5,sizeof(sMd5));
			//NK_MD5_CALC(sVerifyPlain, sMd5);
			NK_MD5_HASH_STR(sVerifyPlain, NK_True, sMd5);
			nRet = message_post(eseeId,0,sMd5,jsonRequest_id->valuestring,type,msg,ts_s);
			printf("post result:%d",nRet);
		}
	}
	return 0;
}
