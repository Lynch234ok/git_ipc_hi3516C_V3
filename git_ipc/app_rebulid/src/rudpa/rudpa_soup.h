/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:	rudpa_soup.h
 * Describle: the api&& some struct expose
 * History: 
 * Last modified: 2013-03-28 17:42
=============================================================*/
#ifndef RUDPA_SOUP_H
#define RUDPA_SOUP_H

#include "rudp_session.h"
#include "sdk/sdk_api.h"

typedef enum _SoupCmd
{
	SoupCmdStart = __LINE__,
	SoupCmdPtz = 0,
	SoupCmdSeekStream,
	SoupCmdReqStream,
	SoupCmdAuth,
	SoupCmdDevinfo,
	SoupCmdVcon,
	SoupCmdEnd = __LINE__,
}SoupCmd;

typedef enum _ErrorSoup
{
	ES_SUCCESS,	
	ES_UNSUPPORT,
	ES_PASSWD,
	ES_NOPERMISSION,
	ES_HEADERROR,
	ES_UNDEF = 1024,	
}ErrorSoup;

#define MAX_SOUP_CMD (SoupCmdEnd - SoupCmdStart - 1)


typedef struct _tagSoupFrameHead{
	uint32_t magic;			// magic number �̶�Ϊ 0x534f55ff , "SOU����
	uint32_t version;		// �汾��Ϣ����ǰ�汾Ϊ1.0.0.0���̶�Ϊ0x01000000
	uint32_t frametype;	// ����֡���ͣ���ǰ�汾֧���������ͣ�0x00--��Ƶ 0x01--��ƵI֡ 0x02--��ƵP֡
	uint32_t framesize;	// ����֡�������ݳ���
	uint64_t pts;				// ֡ʱ�����64λ���ݣ����ȵ�΢��
	uint32_t externsize;	// ��չ���ݴ�С����ǰ�汾Ϊ0
	union{
		struct _tagVideoParam{
			uint32_t width;	// ��Ƶ��
			uint32_t height;	// ��Ƶ��
			uint32_t enc;	// ��Ƶ���룬�ĸ�ASIIC�ַ���ʾ����ǰ֧�ֵ���"H264"
		}v;
		struct _tagAudioParam{
			uint32_t samplerate;	// ��Ƶ������
			uint32_t samplewidth;	// ��Ƶ����λ��
			uint32_t enc;	// ��Ƶ���룬�ĸ�ASIIC�ַ���ʾ����ǰ֧�ֵ���"G711"
		}a;
	}_U;
}SoupFrameHead;



typedef struct _SoupData
{
	char *soup_version;
	uint32_t soup_cmd;
	char *ptz_chl;
	char *ptz_act;
	char *ptz_param1;
	char *ptz_param[2];	
	char *settings_method;
	char * settings_vin;
	uint32_t settings_stream_No;
	uint32_t streamreq_ch;
	uint32_t streamreq_stream_No;
	char *streamreq_opt;
	char *auth_usr;
	char *auth_psw;
	char *soup_ticket;
	uint32_t soup_error;
	uint32_t sd_camcnt;
	char *vcon_cmd;
	uint32_t vcon_id;
	char *vcon_app;
	char *vcon_load;
	uint32_t vcon_loadlen;
	char *vcon_ename;
}SoupData;

typedef struct _Soup
{

	int (*PackHead)(void *,const lpSDK_ENC_BUF_ATTR);
	int (*DataProc)(char *,int ,SoupData*);
	char* (*PackPkt)(SoupData *);
}Soup;


extern Soup * CreateNewSoup(void*);
extern char *GetStreamName(SoupData*);
extern char * PackSoupPkt(SoupData *thiz);


#endif /*end of rudpa_soup.h*/

