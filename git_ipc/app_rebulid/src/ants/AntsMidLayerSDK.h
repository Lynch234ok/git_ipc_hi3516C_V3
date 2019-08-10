/*
*	IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
*
*	By downloading, copying, installing or using the software you agree to this license.
*	If you do not agree to this license, do not download, install,
*	Copy or use the software.
*
*	Copyright (C) 2012, ANTS, Inc, All Rights Reserved.
*
*	Project Name:ServerCoreV2
*	File Name:AntsMidLayerSDK.h
*
*	Writed by ItmanLee at 2012 - 11 - 05 Ants,WuHan,HuBei,China
*/
#ifndef __ANTSMIDLAYERSDK_H__
#define __ANTSMIDLAYERSDK_H__
#ifdef WIN32
#include <Windows.h>
#else
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned short USHORT;
typedef int LONG;
#ifndef BOOL
#define BOOL int
#else
typedef int BOOL;
#endif
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef unsigned int* LPDWORD; 
typedef void*	LPVOID;

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef NULL
#define NULL 	0
#endif
#endif

/** @���*/
#ifndef IN
#define IN
#endif

/** @����*/
#ifndef OUT
#define OUT
#endif

//!�궨��
#define ANTS_MID_MAX_NAMELEN			    16		//!DVR���ص�½��
#define ANTS_MID_MAX_RIGHT			    	32		//!�豸֧�ֵ�Ȩ�ޣ�1-12��ʾ����Ȩ�ޣ�13-32��ʾԶ��Ȩ�ޣ�
#define ANTS_MID_NAME_LEN			    		32    //!�û�������
#define ANTS_MID_PASSWD_LEN			    	16    //!���볤��
#define ANTS_MID_SERIALNO_LEN		    	48    //!���кų���
#define ANTS_MID_MACADDR_LEN			    6     //!MAC��ַ����
#define ANTS_MID_MAX_ETHERNET		    	2     //!�豸������̫����
#define ANTS_MID_MAX_NETWORK_CARD    	4     //!�豸�������������Ŀ
#define ANTS_MID_PATHNAME_LEN		    	128   //!·������

#define ANTS_MID_MAX_TIMESEGMENT		  8	   	//!�豸���ʱ�����

#define ANTS_MID_MAX_SHELTERNUM				4     //!�豸����ڵ�������
#define ANTS_MID_MAX_DAYS							7     //!ÿ������
#define ANTS_MID_PHONENUMBER_LEN			32    //!PPPOE���ź�����󳤶�

#define ANTS_MID_MAX_DISKNUM					8			//!�豸���Ӳ����
#define ANTS_MID_MAX_DISKNUM_V2					24

#define ANTS_MID_MAX_WINDOW						32    //�豸������ʾ��󲥷Ŵ�����
#define ANTS_MID_MAX_VGA							4     //�豸���ɽ�VGA��

#define ANTS_MID_MAX_USERNUM					8     //�豸����û���
#define ANTS_MID_MAX_EXCEPTIONNUM			32    //!�豸����쳣������
#define ANTS_MID_MAX_LINK							6     //!�豸��ͨ�������Ƶ��������

#define ANTS_MID_MAX_STRINGNUM				1			//!�豸���OSD�ַ�����
#define ANTS_MID_MAX_HD_GROUP					8	    //!�豸���Ӳ������

#define ANTS_MID_MAX_WIFI_ESSID_SIZE	    				32      //WIFI��SSID�ų���
#define ANTS_MID_MAX_WIFI_ENCODING_TOKEN					32      //WIFI��������ֽ���
#define ANTS_MID_MAX_WIFI_WEP_KEY_COUNT						4
#define ANTS_MID_MAX_WIFI_WEP_KEY_LENGTH					33
#define ANTS_MID_MAX_WIFI_WPA_PSK_KEY_LENGTH			63
#define ANTS_MID_MIN_WIFI_WPA_PSK_KEY_LENGTH			8
#define ANTS_MID_MAX_WIFI_AP_COUNT								20
#define ANTS_MID_WIFI_MACADDR_LEN									6

#define ANTS_MID_MAX_3G_DEVICE_DESC_LEN						32
#define ANTS_MID_MAX_3G_DEVICE_NUM								200
#define ANTS_MID_MAX_MANAGERHOST_NUM							2
#define ANTS_MID_DDNS_SERVICE_DESC_LEN							32
#define ANTS_MID_DDNS_SERVICE_NUM								200

#define ANTS_MID_MAX_SERIAL_NUM										64	    //���֧�ֵ�͸��ͨ��·��
#define ANTS_MID_MAX_DDNS_NUMS	        					10      //�豸������ddns��
#define ANTS_MID_MAX_DOMAIN_NAME		    					64			//�����������
#define ANTS_MID_MAX_EMAIL_ADDR_LEN	    					48      //���email��ַ����
#define ANTS_MID_MAX_EMAIL_PWD_LEN								32      //���email���볤��

#define ANTS_MID_MAX_NFS_DISK									8
#define ANTS_MID_MAX_NET_DISK 								16			//!�������Ӳ�̸���

#define ANTS_MID_MAXPROGRESS		        			100     //!�ط�ʱ�����ٷ���
#define ANTS_MID_MAX_SERIALNUM	        			2       //!�豸֧�ֵĴ����� 1-232�� 2-485

#define ANTS_MID_MAX_PRESET										256			//!�豸֧�ֵ���̨Ԥ�õ���
#define ANTS_MID_MAX_TRACK										256			//!�豸֧�ֵ���̨�켣��
#define ANTS_MID_MAX_CRUISE										256			//!�豸֧�ֵ���̨Ѳ����

#define ANTS_MID_MAX_CRUISE_PRESET_NUMS				32 	    //!һ��Ѳ������Ѳ����

#define ANTS_MID_MAX_SERIAL_PORT							8       //!�豸֧��232������
#define ANTS_MID_MAX_PREVIEW_MODE							8       //!�豸֧�����Ԥ��ģʽ��Ŀ 1����,4����,9����,16����....
#define ANTS_MID_LOG_INFO_LEN									11840   //!��־������Ϣ
#define ANTS_MID_DESC_LEN											32      //!��̨�����ַ�������
#define ANTS_MID_DESC_LEN_64									64
#define ANTS_MID_PTZ_PROTOCOL_NUM							200     //!���֧�ֵ���̨Э����

#define ANTS_MID_MAX_AUDIO										2       //!�����Խ�ͨ����
#define ANTS_MID_MAX_CHANNUM									16      //!�豸���ͨ����
#define ANTS_MID_MAX_CHANNUM_V2								64      //!�豸���ͨ����
#define ANTS_MID_MAX_ALARMIN									16      //!�豸��󱨾�������
#define ANTS_MID_MAX_ALARMIN_V2								64      //!�豸��󱨾�������
#define ANTS_MID_MAX_ALARMOUT									4       //!�豸��󱨾������

#define ANTS_MID_MAX_RECORD_FILE_NUM					20      //!ÿ��ɾ�����߿�¼������ļ���

#define ANTS_MID_MAX_FORTIFY_NUM							10   		//!��󲼷�����
#define ANTS_MID_MAX_INTERVAL_NUM							4    		//!���ʱ��������

#define	ANTS_MID_MAX_IPCNUM										32
#define	ANTS_MID_MAX_IPCNUM_V2								36

#define ANTS_MID_MAX_NODE_NUM				         	256  //!�ڵ����
#define ANTS_MID_MAX_ABILITYTYPE_NUM			  	12   //!���������
#define ANTS_MID_IPC_PROTOCOL_DESC_LEN  			16
#define ANTS_MID_IPC_PROTOCOL_NUM  				128

#define ANTS_MID_FILE_SUCCESS						1000	//!����ļ���Ϣ
#define ANTS_MID_FILE_NOFIND						1001	//!û���ļ�
#define ANTS_MID_ISFINDING						1002	//!���ڲ����ļ�
#define ANTS_MID_NOMOREFILE						1003	//!�����ļ�ʱû�и�����ļ�
#define ANTS_MID_FILE_EXCEPTION					1004	//!�����ļ�ʱ�쳣

//!¼������
#define ANTS_MID_RECORDTYPE_TIMER				1
#define ANTS_MID_RECORDTYPE_MOTION				2
#define ANTS_MID_RECORDTYPE_ALARM				3
#define ANTS_MID_RECORDTYPE_MOTIONORALARM		4
#define ANTS_MID_RECORDTYPE_MOTIONANDALARM		5
#define ANTS_MID_RECORDTYPE_COMMAND				6
#define ANTS_MID_RECORDTYPE_MANUAL				7

/**********************��̨�������� begin*************************/	
#define ANTS_MID_LIGHT_PWRON					2		/* ��ͨ�ƹ��Դ */
#define ANTS_MID_WIPER_PWRON				3		/* ��ͨ��ˢ���� */
#define ANTS_MID_FAN_PWRON					4		/* ��ͨ���ȿ��� */
#define ANTS_MID_HEATER_PWRON				5		/* ��ͨ���������� */
#define ANTS_MID_AUX_PWRON1					6		/* ��ͨ�����豸���� */
#define ANTS_MID_AUX_PWRON2					7		/* ��ͨ�����豸���� */
#define ANTS_MID_SET_PRESET					8		/* ����Ԥ�õ� */
#define ANTS_MID_CLE_PRESET					9		/* ���Ԥ�õ� */

#define ANTS_MID_ZOOM_IN						11	/* �������ٶ�SS���(���ʱ��) */
#define ANTS_MID_ZOOM_OUT						12	/* �������ٶ�SS��С(���ʱ�С) */
#define ANTS_MID_FOCUS_NEAR      		13  /* �������ٶ�SSǰ�� */
#define ANTS_MID_FOCUS_FAR      	 	14  /* �������ٶ�SS��� */
#define ANTS_MID_IRIS_OPEN      		15  /* ��Ȧ���ٶ�SS���� */
#define ANTS_MID_IRIS_CLOSE      		16  /* ��Ȧ���ٶ�SS��С */

#define ANTS_MID_TILT_UP						21	/* ��̨��SS���ٶ����� */
#define ANTS_MID_TILT_DOWN					22	/* ��̨��SS���ٶ��¸� */
#define ANTS_MID_PAN_LEFT						23	/* ��̨��SS���ٶ���ת */
#define ANTS_MID_PAN_RIGHT					24	/* ��̨��SS���ٶ���ת */
#define ANTS_MID_UP_LEFT						25	/* ��̨��SS���ٶ���������ת */
#define ANTS_MID_UP_RIGHT						26	/* ��̨��SS���ٶ���������ת */
#define ANTS_MID_DOWN_LEFT					27	/* ��̨��SS���ٶ��¸�����ת */
#define ANTS_MID_DOWN_RIGHT					28	/* ��̨��SS���ٶ��¸�����ת */
#define ANTS_MID_PAN_AUTO						29	/* ��̨��SS���ٶ������Զ�ɨ�� */

#define ANTS_MID_FILL_PRE_SEQ				30	/* ��Ԥ�õ����Ѳ������ */
#define ANTS_MID_SET_SEQ_DWELL			31	/* ����Ѳ����ͣ��ʱ�� */
#define ANTS_MID_SET_SEQ_SPEED			32	/* ����Ѳ���ٶ� */
#define ANTS_MID_CLE_PRE_SEQ				33	/* ��Ԥ�õ��Ѳ��������ɾ�� */
#define ANTS_MID_STA_MEM_CRUISE			34	/* ��ʼ��¼�켣 */
#define ANTS_MID_STO_MEM_CRUISE			35	/* ֹͣ��¼�켣 */
#define ANTS_MID_RUN_CRUISE					36	/* ��ʼ�켣 */
#define ANTS_MID_RUN_SEQ						37	/* ��ʼѲ�� */
#define ANTS_MID_STOP_SEQ						38	/* ֹͣѲ�� */
#define ANTS_MID_GOTO_PRESET				39	/* ����ת��Ԥ�õ� */
#define ANTS_MID_FILL_SEQ_CRUISE		40	/* ��Ѳ���������õ���̨�� */
/**********************��̨�������� end*************************/	

/*************************************************
�ط�ʱ���ſ�������궨�� ����֧�ֲ鿴����˵���ʹ��� Begin
**************************************************/	
#define ANTS_MID_PLAYSTART					1//��ʼ����
#define ANTS_MID_PLAYSTOP						2//ֹͣ����
#define ANTS_MID_PLAYPAUSE					3//��ͣ����
#define ANTS_MID_PLAYRESTART				4//�ָ�����
#define ANTS_MID_PLAYFAST						5//���
#define ANTS_MID_PLAYSLOW						6//����
#define ANTS_MID_PLAYNORMAL					7//�����ٶ�
#define ANTS_MID_PLAYFRAME					8//��֡��
#define ANTS_MID_PLAYSTARTAUDIO			9//������
#define ANTS_MID_PLAYSTOPAUDIO			10//�ر�����
#define ANTS_MID_PLAYAUDIOVOLUME		11//��������
#define ANTS_MID_PLAYSETPOS					12//�ı��ļ��طŵĽ���
#define ANTS_MID_PLAYGETPOS					13//��ȡ�ļ��طŵĽ���
#define ANTS_MID_PLAYGETTIME				14//��ȡ��ǰ�Ѿ����ŵ�ʱ��(���ļ��طŵ�ʱ����Ч)
#define ANTS_MID_PLAYGETFRAME				15//��ȡ��ǰ�Ѿ����ŵ�֡��(���ļ��طŵ�ʱ����Ч)
#define ANTS_MID_GETTOTALFRAMES			16//��ȡ��ǰ�����ļ��ܵ�֡��(���ļ��طŵ�ʱ����Ч)
#define ANTS_MID_GETTOTALTIME				17//��ȡ��ǰ�����ļ��ܵ�ʱ��(���ļ��طŵ�ʱ����Ч)
#define ANTS_MID_THROWBFRAME				20//��B֡
#define ANTS_MID_SETSPEED						24//���������ٶ�
#define ANTS_MID_KEEPALIVE					25//�������豸������(����ص�����������2�뷢��һ��)
#define ANTS_MID_PLAYSETTIME				26//������ʱ�䶨λ
#define ANTS_MID_PLAYGETTOTALLEN		27//��ȡ��ʱ��طŶ�Ӧʱ����ڵ������ļ����ܳ���
#define ANTS_MID_PLAYSETDISPLAYZOOM	30//���ûط�����ֲ��Ŵ�
/*************************************************
�ط�ʱ���ſ�������궨�� ����֧�ֲ鿴����˵���ʹ��� End
**************************************************/	

/*************************�豸���������� begin*******************************/
#define COMPRESSIONCFG_ABILITY  0x400    //��ȡѹ������������ȡ
typedef enum{
	ANTS_MID_COMPRESSION_STREAM_ABILITY = 0,
	ANTS_MID_MAIN_RESOLUTION_ABILITY,
	ANTS_MID_SUB_RESOLUTION_ABILITY,
	ANTS_MID_EVENT_RESOLUTION_ABILITY,
	ANTS_MID_FRAME_ABILITY,
	ANTS_MID_BITRATE_TYPE_ABILITY,
	ANTS_MID_BITRATE_ABILITY
}ANTS_MID_COMPRESSION_ABILITY_TYPE;
/*************************�豸���������� end*******************************/

/*************************֡���Ͷ���begin*******************************/
typedef enum {
	AntsMidMainStream=0x00,
	AntsMidSubStream=0x01,
	AntsMidThirdStream=0x02
}eANTS_MID_STREAMTYPE;

typedef enum {
	//!������֡����
	AntsMidPktError=0x00,
	AntsMidPktIFrames=0x01,
	AntsMidPktAudioFrames=0x08,
	AntsMidPktPFrames=0x09,
	AntsMidPktBBPFrames=0x0a,
	AntsMidPktMotionDetection=0x0b,
	AntsMidPktDspStatus=0x0c,
	AntsMidPktOrigImage=0x0d,
	AntsMidPktSysHeader=0x0e,
	AntsMidPktBPFrames=0x0f,
	AntsMidPktSFrames=0x10,
	//!������֡����
	AntsMidPktSubSysHeader=0x11,
	AntsMidPktSubIFrames=0x12,
	AntsMidPktSubPFrames=0x13,
	AntsMidPktSubBBPFrames=0x14,
	//!���ܷ�����Ϣ֡����
	AntsMidPktVacEventZones=0x15,
	AntsMidPktVacObjects=0x16,
	//!��������֡����
	AntsMidPktThirdSysHeader=0x17,
	AntsMidPktThirdIFrames=0x18,
	AntsMidPktThirdPFrames=0x19,
	AntsMidPktThirdBBPFrames=0x1a
}eANTS_MID_FRAME_TYPE;
/*************************֡���Ͷ���end*******************************/

/*************************������������ begin*******************************/
//����SetParameter��GetParameter,ע�����Ӧ�����ýṹ
typedef enum{
	ANTS_MID_CFG=0x200,
	ANTS_MID_GET_DEVICECFG,						//!��ȡ�豸���� +
	ANTS_MID_SET_DEVICECFG,						//!�����豸����
	
	ANTS_MID_GET_NETCFG,							//!��ȡ������� +
	ANTS_MID_SET_NETCFG,							//!�����������
	
	ANTS_MID_GET_PICCFG,							//!��ȡͼ����� +
	ANTS_MID_SET_PICCFG,							//!����ͼ�����
	
	ANTS_MID_GET_PICCFG_V2,						//!��ȡͼ�����(64·��չ) +
	ANTS_MID_SET_PICCFG_V2,						//!����ͼ�����(64·��չ)
	
	ANTS_MID_GET_COMPRESSCFG,					//!��ȡѹ������ +
	ANTS_MID_SET_COMPRESSCFG,					//!����ѹ������
	
	ANTS_MID_GET_RECORDCFG,						//!��ȡ¼��ʱ����� +
	ANTS_MID_SET_RECORDCFG,						//!����¼��ʱ�����
	
	ANTS_MID_GET_DECODERCFG,					//!��ȡ���������� +
	ANTS_MID_SET_DECODERCFG,					//!���ý���������
	
	ANTS_MID_GET_RS232CFG,						//!��ȡ232���ڲ��� +
	ANTS_MID_SET_RS232CFG,						//!����232���ڲ���
	
	ANTS_MID_GET_ALARMINCFG,					//!��ȡ����������� +
	ANTS_MID_SET_ALARMINCFG,					//!���ñ����������
	
	ANTS_MID_GET_ALARMINCFG_V2,				//!��ȡ�����������(64·��չ) +
	ANTS_MID_SET_ALARMINCFG_V2,				//!���ñ����������(64·��չ)
	
	ANTS_MID_GET_ALARMOUTCFG,					//!��ȡ����������� +
	ANTS_MID_SET_ALARMOUTCFG,					//!���ñ����������
	
	ANTS_MID_GET_TIMECFG,							//!��ȡDVRʱ�� +
	ANTS_MID_SET_TIMECFG,							//!����DVRʱ��
	
	ANTS_MID_GET_USERCFG,							//!��ȡ�û����� +
	ANTS_MID_SET_USERCFG,							//!�����û�����
	
	ANTS_MID_GET_USERCFG_V2,					//!��ȡ�û�����(64·��չ) +
	ANTS_MID_SET_USERCFG_V2,					//!�����û�����(64·��չ)
	
	ANTS_MID_GET_EXCEPTIONCFG,				//!��ȡ�쳣���� +
	ANTS_MID_SET_EXCEPTIONCFG,				//!�����쳣����
	
	ANTS_MID_GET_ZONEANDDST,					//!��ȡʱ������ʱ�Ʋ��� +
	ANTS_MID_SET_ZONEANDDST,					//!����ʱ������ʱ�Ʋ���
	
	ANTS_MID_GET_SHOWSTRING,					//!��ȡ�����ַ����� +
	ANTS_MID_SET_SHOWSTRING,					//!���õ����ַ�����
	
	ANTS_MID_GET_EVENTCOMPCFG,				//!��ȡ�¼�����¼����� +
	ANTS_MID_SET_EVENTCOMPCFG,				//!�����¼�����¼�����
	
	ANTS_MID_GET_AUTOREBOOT,					//!��ȡ�Զ�ά������ +
	ANTS_MID_SET_AUTOREBOOT,					//!�����Զ�ά������
	
	ANTS_MID_GET_NETAPPCFG,						//!��ȡ����Ӧ�ò��� NTP/DDNS/EMAIL +
	ANTS_MID_SET_NETAPPCFG,						//!��������Ӧ�ò��� NTP/DDNS/EMAIL
	
	ANTS_MID_GET_NTPCFG,							//!��ȡ����Ӧ�ò��� NTP +
	ANTS_MID_SET_NTPCFG,							//!��������Ӧ�ò��� NTP
	
	ANTS_MID_GET_DDNSCFG,							//!��ȡ����Ӧ�ò��� DDNS +
	ANTS_MID_SET_DDNSCFG,							//!��������Ӧ�ò��� DDNS
	
	ANTS_MID_GET_EMAILCFG,						//!��ȡ����Ӧ�ò��� EMAIL +
	ANTS_MID_SET_EMAILCFG,						//!��������Ӧ�ò��� EMAIL
	
	ANTS_MID_GET_HDCFG,								//!��ȡӲ�̹������ò��� +
	ANTS_MID_SET_HDCFG,								//!����Ӳ�̹������ò���
	
	ANTS_MID_GET_HDGROUP_CFG,					//!��ȡ����������ò��� +
	ANTS_MID_SET_HDGROUP_CFG,					//!��������������ò���
	
	ANTS_MID_GET_HDGROUP_CFG_V2,			//!��ȡ����������ò���(64·��չ)+
	ANTS_MID_SET_HDGROUP_CFG_V2,			//!��������������ò���(64·��չ)
	
	ANTS_MID_GET_COMPRESSCFG_AUD,			//!��ȡ�豸�����Խ�������� +
	ANTS_MID_SET_COMPRESSCFG_AUD,			//!�����豸�����Խ��������
	
	ANTS_MID_GET_SNMPCFG,							//!��ȡ�豸SNMP���ò��� +
	ANTS_MID_SET_SNMPCFG,							//!�����豸SNMP���ò���
	
	ANTS_MID_GET_NETCFG_MULTI,				//!��ȡ�豸���������ò��� +
	ANTS_MID_SET_NETCFG_MULTI,				//!�����豸���������ò���
	
	ANTS_MID_GET_NFSCFG,							//!��ȡ�豸NFS���ò��� +
	ANTS_MID_SET_NFSCFG,							//!�����豸NFS���ò���
	
	ANTS_MID_GET_NET_DISKCFG,					//!��ȡ�豸����Ӳ�����ò��� +
	ANTS_MID_SET_NET_DISKCFG,					//!�����豸����Ӳ����������
	
	ANTS_MID_GET_IPCCFG,							//��ȡIPC���ò��� +
	ANTS_MID_SET_IPCCFG,							//����IPC���ò��� 
	
	ANTS_MID_GET_IPCCFG_V2,						//����IPC���ò���(64·��չ)  
	ANTS_MID_SET_IPCCFG_V2,						//��ȡIPC���ò���(64·��չ)+

	ANTS_MID_GET_WIFI_CFG,						//!��ȡIP����豸���߲���
	ANTS_MID_SET_WIFI_CFG,						//!����IP����豸���߲��� +
	
	ANTS_MID_GET_WIFI_WORKMODE,				//!��ȡIP����豸���ڹ���ģʽ����
	ANTS_MID_SET_WIFI_WORKMODE,				//!����IP����豸���ڹ���ģʽ���� +
	
	ANTS_MID_GET_3G_CFG,							//!��ȡ3G���ò���
	ANTS_MID_SET_3G_CFG,							//!����3G���ò��� +
	
	ANTS_MID_GET_MANAGERHOST_CFG, 		//!��ȡ����ע������������ò��� +
	ANTS_MID_SET_MANAGERHOST_CFG,			//!��������ע������������ò���
	
	ANTS_MID_GET_RTSPCFG,							//!��ȡRTSP���ò��� +
	ANTS_MID_SET_RTSPCFG,							//!����RTSP���ò���
	
	ANTS_MID_GET_VIDEOEFFECT,
	ANTS_MID_SET_VIDEOEFFECT,
	
	ANTS_MID_GET_MOTIONCFG,
	ANTS_MID_SET_MOTIONCFG,
	
	ANTS_MID_GET_MOTIONCFG_V2,
	ANTS_MID_SET_MOTIONCFG_V2,
	
	ANTS_MID_GET_SHELTERCFG,
	ANTS_MID_SET_SHELTERCFG,
	
	ANTS_MID_GET_HIDEALARMCFG,
	ANTS_MID_SET_HIDEALARMCFG,
	
	ANTS_MID_GET_VIDEOLOSTCFG,
	ANTS_MID_SET_VIDEOLOSTCFG,
	
	ANTS_MID_GET_OSDCFG,
	ANTS_MID_SET_OSDCFG,
	
	ANTS_MID_GET_VIDEOFORMAT,
	ANTS_MID_SET_VIDEOFORMAT,

	ANTS_MID_GET_NVRWORKMODE,
	ANTS_MID_SET_NVRWORKMODE,
	
	ANTS_MID_GET_NETDEVCONNETCTCFG,
	ANTS_MID_SET_NETDEVCONNETCTCFG,
	
	ANTS_MID_GET_DEVCHANNAME_CFG,
	ANTS_MID_SET_DEVCHANNAME_CFG,

	ANTS_MID_GET_DEVCHANNAME_CFG_V2,
	ANTS_MID_SET_DEVCHANNAME_CFG_V2,

	ANTS_MID_GET_HDCFG_V2,
	ANTS_MID_SET_HDCFG_V2,

	ANTS_MID_GET_SENSORCFG,
	ANTS_MID_SET_SENSORCFG,

	ANTS_MID_GET_3GDEVICE_CFG=0x600,	//!��ȡ�豸֧��3G�����豸����	
	ANTS_MID_GET_AP_INFO_LIST,				//!��ȡ����������Դ����
	ANTS_MID_GET_USERINFO,						//!��ȡ��ǰ�û���Ϣ
	ANTS_MID_GET_USERINFO_V2,
	ANTS_MID_GET_PTZCFG,							//!��ȡ�豸֧��PTZЭ�鼯��
	ANTS_MID_GET_WORKSTATUS,					//!��ȡ�豸��ǰ����״̬
	ANTS_MID_GET_WORKSTATUS_V2,
	ANTS_MID_GET_COMPRESS_ABILITY,		//!��ȡ�豸ѹ����������
	ANTS_MID_GET_WORKSTATUS_V3,

	ANTS_MID_GET_VIDEOEFFECTV2,
	ANTS_MID_SET_VIDEOEFFECTV2,

	ANTS_MID_GET_DDNS_ABILITY,			//!DDNS֧����������
}ANTS_MID_COMMAND;
/*************************������������ end*******************************/

/*******************������Ϣ�ṹ begin*********************/
typedef struct {
	DWORD dwAlarmType;															/*0-�ź�������,1-Ӳ����,2-�źŶ�ʧ,3���ƶ����,4��Ӳ��δ��ʽ��,5-��дӲ�̳���,6-�ڵ�����,7-��ʽ��ƥ��, 8-�Ƿ�����, 9-��Ƶ�ź��쳣��10-¼���쳣*/
	DWORD dwAlarmInputNumber;												/*��������˿�*/
	BYTE byAlarmOutputNumber[ANTS_MID_MAX_ALARMOUT];/*����������˿ڣ���һλΪ1��ʾ��Ӧ��һ�����*/
	BYTE byAlarmRelateChannel[ANTS_MID_MAX_CHANNUM];/*������¼��ͨ������һλΪ1��ʾ��Ӧ��һ·¼��, dwAlarmRelateChannel[0]��Ӧ��1��ͨ��*/
	BYTE byChannel[ANTS_MID_MAX_CHANNUM];						/*dwAlarmTypeΪ2��3,6,9,10ʱ����ʾ�ĸ�ͨ����dwChannel[0]λ��Ӧ��1��ͨ��*/
	BYTE byDiskNumber[ANTS_MID_MAX_DISKNUM];				/*dwAlarmTypeΪ1,4,5ʱ,��ʾ�ĸ�Ӳ��, dwDiskNumber[0]λ��Ӧ��1��Ӳ��*/
}ANTS_MID_ALARMINFO,*LPANTS_MID_ALARMINFO;

typedef struct {
	DWORD dwAlarmType;																	/*0-�ź�������,1-Ӳ����,2-�źŶ�ʧ,3���ƶ����,4��Ӳ��δ��ʽ��,5-��дӲ�̳���,6-�ڵ�����,7-��ʽ��ƥ��, 8-�Ƿ�����, 9-��Ƶ�ź��쳣��10-¼���쳣*/
	DWORD dwAlarmInputNumber;														/*��������˿�*/
	BYTE byAlarmOutputNumber[ANTS_MID_MAX_ALARMOUT];		/*����������˿ڣ���һλΪ1��ʾ��Ӧ��һ�����*/
	BYTE byAlarmRelateChannel[ANTS_MID_MAX_CHANNUM_V2];	/*������¼��ͨ������һλΪ1��ʾ��Ӧ��һ·¼��, dwAlarmRelateChannel[0]��Ӧ��1��ͨ��*/
	BYTE byChannel[ANTS_MID_MAX_CHANNUM_V2];						/*dwAlarmTypeΪ2��3,6,9,10ʱ����ʾ�ĸ�ͨ����dwChannel[0]λ��Ӧ��1��ͨ��*/
	BYTE byDiskNumber[ANTS_MID_MAX_DISKNUM];						/*dwAlarmTypeΪ1,4,5ʱ,��ʾ�ĸ�Ӳ��, dwDiskNumber[0]λ��Ӧ��1��Ӳ��*/
}ANTS_MID_ALARMINFO_V2,*LPANTS_MID_ALARMINFO_V2;
/*******************������Ϣ�ṹ end*********************/

/*******************�����������ṹ begin*********************/
typedef struct{
	int iValue;
	BYTE byDescribe[ANTS_MID_DESC_LEN];
	DWORD dwFreeSpace;
	BYTE byRes[12];
}ANTS_MID_DESC_NODE,*LPANTS_MID_DESC_NODE;

typedef struct{
	DWORD dwAbilityType;
	BYTE byRes[32];
	DWORD dwNodeNum;
	ANTS_MID_DESC_NODE struDescNode[ANTS_MID_MAX_NODE_NUM];
}ANTS_MID_ABILITY_LIST,*LPANTS_MID_ABILITY_LIST;

typedef struct{
	DWORD dwSize;
	DWORD dwAbilityNum;
	ANTS_MID_ABILITY_LIST struAbilityNode[ANTS_MID_MAX_ABILITYTYPE_NUM];
}ANTS_MID_COMPRESSIONCFG_ABILITY,*LPANTS_MID_COMPRESSIONCFG_ABILITY;
/*******************�����������ṹ end*********************/

/*******************�������ýṹ������ begin*********************/
//!Уʱ�ṹ����
typedef struct{
	DWORD dwYear;			//��
	DWORD dwMonth;		//��
	DWORD dwDay;			//��
	DWORD dwHour;			//ʱ
	DWORD dwMinute;		//��
	DWORD dwSecond;		//��
}ANTS_MID_TIME, *LPANTS_MID_TIME;

//!ʱ���(�ӽṹ)
typedef struct{
	//!��ʼʱ��
  BYTE byStartHour;
	BYTE byStartMin;
	//!����ʱ��
	BYTE byStopHour;
	BYTE byStopMin;
}ANTS_MID_SCHEDTIME,*LPANTS_MID_SCHEDTIME;

typedef struct{
	BYTE byBrightness;  	/*����,0-255*/
	BYTE byContrast;    		/*�Աȶ�,0-255*/	
	BYTE bySaturation;  	/*���Ͷ�,0-255*/
	BYTE byHue;    		/*ɫ��,0-255*/
}ANTS_MID_COLOR,*LPANTS_MID_COLOR;

typedef struct{
	LONG lBrightness;  	/*����,0-255,-1��֧��*/
	LONG lContrast;    	/*�Աȶ�,0-255,-1��֧��*/	
	LONG lSaturation;  	/*���Ͷ�,0-255, -1��֧��*/
	LONG lHue;    		/*ɫ��,0-255,-1��֧��*/
}ANTS_MID_COLOR_V2, *LPANTS_MID_COLOR_V2;

typedef struct{
	LONG lDayNightMode;// -1--��֧��;0--�ⲿ�������;1--�Զ�ģʽ;2--ǿ�ư���;3--ǿ�ƺ�ҹ
	LONG lDelay ;// �Զ�ת���ӳ٣��Զ�ģʽ��Ч��0-30
	LONG lNighttoDayThreshold ;// �Զ�ת����ҹ���������ֵ0-255��Ĭ��0xEE
	LONG lDaytoNightThreshold ;// �Զ�ת�����쵽��ҹ����ֵ0-255��Ĭ��0x57
}ANTS_MID_SENSOR_DAYNIGHTMODE,*LPANTS_MID_SENSOR_DAYNIGHTMODE;

typedef struct{
	DWORD dwSize;
	DWORD dwValidMask;//!��Ӧλ0-��Ч��1-��Ч; 
	// bit0 - DayNightMode,bit1-lMinorMode,bit2-lGainMode,bit3-lAntiflickerMode,
	// bit4-lPicQualityMode,bit5-lWBMode   ,bit6-lBacklightMode,bit7-lShutterMode
	// bit8-lIrisMode,bit9-lSharpnessMode,bit10-l3DNRMode,bit11-3DNRTfode,
	// bit12-WDMode,bit13-GammaMode,bit14-AntiflickerFreqMode
	ANTS_MID_SENSOR_DAYNIGHTMODE DayNightMode;
	LONG lMinorMode ;//���� -1--��֧��;0--����;1--ˮƽ��ת;2--��ֱ��ת;3--180�㷭ת
	LONG lGainMode ;//���� -1--��֧��;0--��;1--��;2--��
	LONG lAntiflickerMode ;//���� -1--��֧��;0--��;1--��
	LONG lPicQualityMode ;//ͼ��Ч�� -1--��֧��;0--����;1--����;2--��Ȼ
	LONG lWBMode ;//��ƽ�� -1--��֧��;0--�Զ���ƽ��;1--����ģʽ;2--����ģʽ
	LONG lBacklightMode;// ���ⲹ��ģʽ-1--��֧��; 0--�ر�;1--BLC;2--HBLC
	LONG lShutterMode;// ����ģʽ -1--��֧��; 0--�Զ�����;
	/*����ģʽ:
	0x01:1/30(1/25), 
	0x02:1/60(1/50), 
	0x03:Flicker, 
	0x04:1/250, 
	0x05:1/500, 
	0x06:1/1000, 
	0x07:1/2000, 
	0x08:1/5000, 
	0x09:1/10000, 
	0x0A:1/50000, 
	0x0B:x2, 
	0x0C:x4, 
	0x0D:x6, 
	0x0E:x8, 
	0x0F:x10, 
	0x10:x15, 
	0x11:x20,
	0x12:x25,
	0x13:x30 */
	LONG lIrisMode;// ��ͷ��Ȧģʽ:-1--��֧��; 0--�Զ���Ȧ;1--�ֶ���̶���Ȧ
	LONG lSharpnessMode;// ���ģʽ: -1--��֧��;0--�ر�;1--��
	LONG lSharpnessLevel;// 0-100
	LONG l3DNRMode;// 3D����ģʽ : -1--��֧��;0--�ر�;1--��
	LONG l3DNRLevel;// 0-100
	LONG l3DNRTfode; // 3D����ʱ��-1--��֧�� 0--�ر�;1--��;2--��;3--�ϸ�;4--��
	LONG lWDMode; // ��̬ģʽ-1--��֧��0--�ر�;1--��;2--��;3--��
	LONG lGammaMode;// Gammaģʽ-1--��֧�� 0--Curve_1_6;1--Curve_1_6;2--Curve_2_0;3--Curve_2_2
	LONG lAntiflickerFreqMode; // ����ģʽ	-1--��֧��;0--�Զ�;1--50HZ;2--60HZ
	BYTE byMDICameraType;// MDI���������, ����ȡ,0-NONE,1-LMOP72A34,2-LMOV72063,3-LMOV72063IR,-1-��֧��
	BYTE byMDICameraAutoConfigEnable; // �Ƿ���������Զ�����,-1��֧��
	BYTE byAuxSupported; //  bit0:�Ƿ�֧�ֻ�����(0-��֧��,1-֧��);bit1:֧��У����Ȧ;bit2:֧��Զ������
	BYTE byRes;
}ANTS_MID_SENSOR_CFG,*LPANTS_MID_SENSOR_CFG;

//!�������쳣����ṹ(�ӽṹ)(�ദʹ��)
typedef struct{
	DWORD dwHandleType;		/*����ʽ,����ʽ��"��"���*/
												/*0x00: ����Ӧ*/
												/*0x01: �������Ͼ���*/
												/*0x02: ��������*/
												/*0x04: �ϴ�����*/
												/*0x08: �����������*/
												/*0x10: Jpegץͼ���ϴ�EMail*/
	BYTE byRelAlarmOut[ANTS_MID_MAX_ALARMOUT];	//�������������ͨ��,�������������,Ϊ1��ʾ���������
}ANTS_MID_HANDLEEXCEPTION,*LPANTS_MID_HANDLEEXCEPTION;

//!�źŶ�ʧ����(�ӽṹ)
typedef struct {
	BYTE byEnableHandleVILost;	/* �Ƿ����źŶ�ʧ���� */
	BYTE byRes[3];
	ANTS_MID_HANDLEEXCEPTION strVILostHandleType;	/* ����ʽ */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];//����ʱ��
}ANTS_MID_VILOST, *LPANTS_MID_VILOST;

//�ƶ����(�ӽṹ)
typedef struct {
	BYTE byMotionScope[64][96];/* �������,0-16λ,��ʾ12��,����16*12��С���,Ϊ1��ʾ���ƶ��������,0-��ʾ���� */
	BYTE byMotionSensitive;/* �ƶ����������, 0 - 5,Խ��Խ����,oxff�ر� */
	BYTE byEnableHandleMotion;/* �Ƿ����ƶ���� 0���� 1����*/ 
	BYTE byPrecision;/* �ƶ�����㷨�Ľ���: 0--16*16, 1--32*32, 2--64*64 ... (��ʱ�̶�Ϊ0)*/
	char reservedData;	
	ANTS_MID_HANDLEEXCEPTION struMotionHandleType;/* ����ʽ */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];/* ����ʱ�� */
	BYTE byRelRecordChan[ANTS_MID_MAX_CHANNUM];/* ����������¼��ͨ��*/
}ANTS_MID_MOTION, *LPANTS_MID_MOTION;

//!�ƶ����(�ӽṹ)
typedef struct {
	BYTE byMotionScope[64][96];/* �������,0-16λ,��ʾ12��,����16*12��С���,Ϊ1��ʾ���ƶ��������,0-��ʾ���� */
	BYTE byMotionSensitive;/* �ƶ����������, 0 - 5,Խ��Խ����,oxff�ر� */
	BYTE byEnableHandleMotion;/* �Ƿ����ƶ���� 0���� 1����*/ 
	BYTE byPrecision;/* �ƶ�����㷨�Ľ���: 0--16*16, 1--32*32, 2--64*64 ... (��ʱ�̶�Ϊ0)*/
	char reservedData;	
	ANTS_MID_HANDLEEXCEPTION struMotionHandleType;/* ����ʽ */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];/* ����ʱ�� */
	BYTE byRelRecordChan[ANTS_MID_MAX_CHANNUM_V2];/* ����������¼��ͨ��*/
}ANTS_MID_MOTION_V2, *LPANTS_MID_MOTION_V2;

//!�ڵ�����(�ӽṹ)  �����С704*576
typedef struct {
	DWORD dwEnableHideAlarm;/* �Ƿ������ڵ����� ,0-��,1-�������� 2-�������� 3-��������*/
	WORD wHideAlarmAreaTopLeftX;/* �ڵ������x���� */
	WORD wHideAlarmAreaTopLeftY;/* �ڵ������y���� */
	WORD wHideAlarmAreaWidth;/* �ڵ�����Ŀ� */
	WORD wHideAlarmAreaHeight;/* �ڵ�����ĸ�*/
	ANTS_MID_HANDLEEXCEPTION strHideAlarmHandleType;/* ����ʽ */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];//����ʱ��
}ANTS_MID_HIDEALARM,*LPANTS_MID_HIDEALARM;

//!�ڵ�����(�ӽṹ)
typedef struct {
	WORD wHideAreaTopLeftX;/* �ڵ������x���� */
	WORD wHideAreaTopLeftY;/* �ڵ������y���� */
	WORD wHideAreaWidth;/* �ڵ�����Ŀ� */
	WORD wHideAreaHeight;/*�ڵ�����ĸ�*/
}ANTS_MID_SHELTER,*LPANTS_MID_SHELTER;

typedef struct {
	//�ڵ������С704*576
	DWORD dwEnableHide;/* �Ƿ������ڵ� ,0-��,1-��*/
	ANTS_MID_SHELTER struShelter[ANTS_MID_MAX_SHELTERNUM];	
}ANTS_MID_SHELTERCFG,*LPANTS_MID_SHELTERCFG;

typedef struct {
	DWORD dwSize;
	BYTE sChanName[ANTS_MID_NAME_LEN];
	//!��ʾͨ����
	DWORD dwShowChanName;//Ԥ����ͼ�����Ƿ���ʾͨ������,0-����ʾ,1-��ʾ �����С704*576
	WORD wShowNameTopLeftX;/* ͨ��������ʾλ�õ�x���� */
	WORD wShowNameTopLeftY;/* ͨ��������ʾλ�õ�y���� */
	//!OSD
	DWORD dwShowOsd;// Ԥ����ͼ�����Ƿ���ʾOSD,0-����ʾ,1-��ʾ �����С704*576
	WORD wOSDTopLeftX;/* OSD��x���� */
	WORD wOSDTopLeftY;/* OSD��y���� */
	BYTE byOSDType;/* OSD����(��Ҫ�������ո�ʽ) */
									/* 0: XXXX-XX-XX ������ */
									/* 1: XX-XX-XXXX ������ */
									/* 2: XXXX��XX��XX�� */
									/* 3: XX��XX��XXXX�� */
									/* 4: XX-XX-XXXX ������*/
									/* 5: XX��XX��XXXX�� */
	BYTE byDispWeek;/*�Ƿ���ʾ���� */
	BYTE byOSDAttrib;/*OSD����:͸������˸ (����)*/
	BYTE byHourOSDType;/*OSDСʱ��:0-24Сʱ��,1-12Сʱ�� */
}ANTS_MID_OSDCFG,*LPANTS_MID_OSDCFG;

//!ͨ��ͼ��ṹ
typedef struct{
	DWORD dwSize;
	BYTE sChanName[ANTS_MID_NAME_LEN];
	DWORD dwVideoFormat;/*��Ƶ��ʽ 1-NTSC 2-PAL*/
	ANTS_MID_COLOR struColor;//ͼ�����
	char reservedData [60];/*����*/
	//!��ʾͨ����
	DWORD dwShowChanName;// Ԥ����ͼ�����Ƿ���ʾͨ������,0-����ʾ,1-��ʾ �����С704*576
	WORD wShowNameTopLeftX;/* ͨ��������ʾλ�õ�x���� */
	WORD wShowNameTopLeftY;/* ͨ��������ʾλ�õ�y���� */
	//!��Ƶ�źŶ�ʧ����
	ANTS_MID_VILOST struVILost;
	ANTS_MID_VILOST struRes;/*����*/
	//!�ƶ����
	ANTS_MID_MOTION	struMotion;
	//!�ڵ�����
	ANTS_MID_HIDEALARM struHideAlarm;
	//!�ڵ�  �����С704*576
	DWORD dwEnableHide;/* �Ƿ������ڵ� ,0-��,1-��*/
	ANTS_MID_SHELTER struShelter[ANTS_MID_MAX_SHELTERNUM];
	//!OSD
	DWORD dwShowOsd;//!Ԥ����ͼ�����Ƿ���ʾOSD,0-����ʾ,1-��ʾ �����С704*576
	WORD wOSDTopLeftX;/* OSD��x���� */
	WORD wOSDTopLeftY;/* OSD��y���� */
	BYTE byOSDType;		/* OSD����(��Ҫ�������ո�ʽ) */
										/* 0: XXXX-XX-XX ������ */
										/* 1: XX-XX-XXXX ������ */
										/* 2: XXXX��XX��XX�� */
										/* 3: XX��XX��XXXX�� */
										/* 4: XX-XX-XXXX ������*/
										/* 5: XX��XX��XXXX�� */
	BYTE byDispWeek;/* �Ƿ���ʾ���� */
	BYTE byOSDAttrib;/* OSD����:͸������˸ (����)*/
  BYTE byHourOSDType;/* OSDСʱ��:0-24Сʱ��,1-12Сʱ�� */
	BYTE byRes[64];
}ANTS_MID_PICCFG, *LPANTS_MID_PICCFG;

//!ͨ��ͼ��ṹ
typedef struct{
	DWORD dwSize;
	BYTE sChanName[ANTS_MID_NAME_LEN];
	DWORD dwVideoFormat;/*��Ƶ��ʽ 1-NTSC 2-PAL*/
	ANTS_MID_COLOR struColor;//ͼ�����
	char reservedData[60];/*����*/
	//!��ʾͨ����
	DWORD dwShowChanName;// Ԥ����ͼ�����Ƿ���ʾͨ������,0-����ʾ,1-��ʾ �����С704*576
	WORD wShowNameTopLeftX;/* ͨ��������ʾλ�õ�x���� */
	WORD wShowNameTopLeftY;/* ͨ��������ʾλ�õ�y���� */

	//!��Ƶ�źŶ�ʧ����
	ANTS_MID_VILOST struVILost;
	ANTS_MID_VILOST struRes;/*����*/

	//!�ƶ����
	ANTS_MID_MOTION_V2 struMotion;

	//!�ڵ�����
	ANTS_MID_HIDEALARM struHideAlarm;

	//!�ڵ������С704*576
	DWORD dwEnableHide;/* �Ƿ������ڵ� ,0-��,1-��*/
	ANTS_MID_SHELTER struShelter[ANTS_MID_MAX_SHELTERNUM];
	//!OSD
	DWORD dwShowOsd;// Ԥ����ͼ�����Ƿ���ʾOSD,0-����ʾ,1-��ʾ �����С704*576
	WORD wOSDTopLeftX;/* OSD��x���� */
	WORD wOSDTopLeftY;/* OSD��y���� */
	BYTE byOSDType;/* OSD����(��Ҫ�������ո�ʽ) */
								/* 0: XXXX-XX-XX ������ */
								/* 1: XX-XX-XXXX ������ */
								/* 2: XXXX��XX��XX�� */
								/* 3: XX��XX��XXXX�� */
								/* 4: XX-XX-XXXX ������*/
								/* 5: XX��XX��XXXX�� */
	BYTE byDispWeek;/* �Ƿ���ʾ���� */
	BYTE byOSDAttrib;/* OSD����:͸������˸ (����)*/
  BYTE byHourOSDType;/* OSDСʱ��:0-24Сʱ��,1-12Сʱ�� */
	BYTE byRes[64];
}ANTS_MID_PICCFG_V2,*LPANTS_MID_PICCFG_V2;

//!����ѹ������(�ӽṹ)
typedef struct {
	BYTE byStreamType;//�������� 0-��Ƶ��, 1-������, ��ʾ�¼�ѹ������ʱ���λ��ʾ�Ƿ�����ѹ������
	BYTE byResolution;//�ֱ���0-DCIF 1-CIF, 2-QCIF, 3-4CIF, 4-2CIF 5��������,
	                  //16-VGA��640*480��, 17-UXGA��1600*1200��, 18-SVGA ��800*600��,
	                  //19-HD720p��1280*720��,20-XVGA,  21-HD900p, 27-HD1080i, 
	                  //28-2560*1920, 29-1600*304, 30-2048*1536, 31-2448*2048	
	BYTE byBitrateType;//�������� 0:������, 1:������
	BYTE byPicQuality;//ͼ������ 0-��� 1-�κ� 2-�Ϻ� 3-һ�� 4-�ϲ� 5-��
	DWORD dwVideoBitrate;//��Ƶ���� 0-���� 1-16K 2-32K 3-48k 4-64K 5-80K 6-96K 7-128K 8-160k 9-192K 10-224K 11-256K 12-320K
											//13-384K 14-448K 15-512K 16-640K 17-768K 18-896K 19-1024K 20-1280K 21-1536K 22-1792K 23-2048K
											//���λ(31λ)�ó�1��ʾ���Զ�������, 0-30λ��ʾ����ֵ��
	DWORD dwVideoFrameRate;//֡�� 0-ȫ��; 1-1/16; 2-1/8; 3-1/4; 4-1/2; 5-1; 6-2; 7-4; 8-6; 9-8; 10-10; 11-12; 12-16; 13-20; 14-15; 15-18; 16-22,17-25;
	WORD wIntervalFrameI;//I֡���
	BYTE byIntervalBPFrame;//0-BBP֡; 1-BP֡; 2-��P֡ (����)
 	BYTE byres1;//����
 	BYTE byVideoEncType;//��Ƶ�������� 0 ˽��h264;1��׼h264; 2��׼mpeg4; 3-M-JPEG
 	BYTE byAudioEncType;//��Ƶ�������� 0-OggVorbis;1-G711_U;2-G711_A
 	BYTE byres[10];//���ﱣ����Ƶ��ѹ������
}ANTS_MID_COMPRESSION_INFO, *LPANTS_MID_COMPRESSION_INFO;

//!ͨ��ѹ������
typedef struct {
	DWORD dwSize;
	ANTS_MID_COMPRESSION_INFO	struNormHighRecordPara;//¼��
	ANTS_MID_COMPRESSION_INFO	struRes;//���� char reserveData[28];
  	ANTS_MID_COMPRESSION_INFO	struEventRecordPara;//�¼�����ѹ������
	ANTS_MID_COMPRESSION_INFO	struNetPara;//����(������)
}ANTS_MID_COMPRESSIONCFG, *LPANTS_MID_COMPRESSIONCFG;

//!ʱ���¼���������(�ӽṹ)
typedef struct {
	ANTS_MID_SCHEDTIME struRecordTime;
	BYTE byRecordType;//0:��ʱ¼��1:�ƶ���⣬2:����¼��3:����|������4:����&����, 5:�����, 6:�ֶ�¼��
	char reservedData[3];
}ANTS_MID_RECORDSCHED, *LPANTS_MID_RECORDSCHED;

//!ȫ��¼���������(�ӽṹ)
typedef struct {
	WORD wAllDayRecord;/* �Ƿ�ȫ��¼�� 0-�� 1-��*/
	BYTE byRecordType;/* ¼������ 0:��ʱ¼��1:�ƶ���⣬2:����¼��3:����|������4:����&���� 5:�����*/
	char reservedData;
}ANTS_MID_RECORDDAY, *LPANTS_MID_RECORDDAY;

//!ͨ��¼���������
typedef struct {
	DWORD dwSize;
	DWORD dwRecord;/*�Ƿ�¼�� 0-�� 1-��*/
	ANTS_MID_RECORDDAY struRecAllDay[ANTS_MID_MAX_DAYS];
	ANTS_MID_RECORDSCHED struRecordSched[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];
	DWORD dwRecordTime;/* ¼����ʱ���� 0-5�룬 1-20�룬 2-30�룬 3-1���ӣ� 4-2���ӣ� 5-5���ӣ� 6-10����*/
	DWORD dwPreRecordTime;/* Ԥ¼ʱ�� 0-��Ԥ¼ 1-5�룬2-10�룬3-15�룬4-20�룬5-25�룬6-30�� 7-0xffffffff(������Ԥ¼) */
	DWORD dwRecorderDuration;/* ¼�񱣴���ʱ�� */
	BYTE byRedundancyRec;/*�Ƿ�����¼��,��Ҫ����˫���ݣ�0/1*/
	BYTE byAudioRec;/*¼��ʱ����������ʱ�Ƿ��¼��Ƶ���ݣ������д˷���*/
	BYTE byReserve[10];	
}ANTS_MID_RECORD,*LPANTS_MID_RECORD;

//!ͨ��������(��̨)��������
typedef struct {
	DWORD dwSize;
	DWORD dwBaudRate;//������(bps)��0��50��1��75��2��110��3��150��4��300��5��600��6��1200��7��2400��8��4800��9��9600��10��19200�� 11��38400��12��57600��13��76800��14��115.2k;
	BYTE byDataBit;//�����м�λ 0��5λ��1��6λ��2��7λ��3��8λ;
	BYTE byStopBit;//ֹͣλ 0��1λ��1��2λ;
	BYTE byParity;//У�� 0����У�飬1����У�飬2��żУ��;
	BYTE byFlowcontrol;//0���ޣ�1��������,2-Ӳ����
	WORD wDecoderType;//����������  ANTS_MID_PTZCFG�еõ�
	WORD wDecoderAddress;/*��������ַ:0 - 255*/
	BYTE bySetPreset[ANTS_MID_MAX_PRESET];/* Ԥ�õ��Ƿ�����,0-û������,1-����*/
	BYTE bySetCruise[ANTS_MID_MAX_CRUISE];/* Ѳ���Ƿ�����: 0-û������,1-���� */
	BYTE bySetTrack[ANTS_MID_MAX_TRACK];/* �켣�Ƿ�����,0-û������,1-����*/
}ANTS_MID_DECODERCFG,*LPANTS_MID_DECODERCFG;

//!DVR�豸����
typedef struct{
	DWORD dwSize;
	BYTE sDVRName[ANTS_MID_NAME_LEN];//DVR����
	DWORD dwDVRID;//DVR ID,����ң����
	DWORD dwRecycleRecord;//�Ƿ�ѭ��¼��,0:����; 1:��

	//!���²�������
	BYTE sSerialNumber[ANTS_MID_SERIALNO_LEN];//���к�
	DWORD dwSoftwareVersion;//����汾��,��16λ�����汾,��16λ�Ǵΰ汾
	DWORD dwSoftwareBuildDate;//�����������,0xYYYYMMDD
	DWORD dwDSPSoftwareVersion;//DSP����汾,��16λ�����汾,��16λ�Ǵΰ汾
	DWORD dwDSPSoftwareBuildDate;// DSP�����������,0xYYYYMMDD
	DWORD dwPanelVersion;//ǰ���汾,��16λ�����汾,��16λ�Ǵΰ汾
	DWORD dwHardwareVersion;//Ӳ���汾,��16λ�����汾,��16λ�Ǵΰ汾
	BYTE byAlarmInPortNum;//DVR�����������
	BYTE byAlarmOutPortNum;//DVR�����������
	BYTE byRS232Num;//DVR 232���ڸ���
	BYTE byRS485Num;//DVR 485���ڸ���
	BYTE byNetworkPortNum;//����ڸ���
	BYTE byDiskCtrlNum;//DVR Ӳ�̿���������
	BYTE byDiskNum;//DVR Ӳ�̸���
	BYTE byDVRType;//DVR����, 1:DVR 2:Result 3:DVS ......
	BYTE byChanNum;//DVR ͨ������
	BYTE byStartChan;//��ʼͨ����,����DVS-1,DVR - 1
	BYTE byDecodeChans;//DVR ����·��
	BYTE byVGANum;//VGA�ڵĸ���
	BYTE byUSBNum;//USB�ڵĸ���
	BYTE byAuxoutNum;//���ڵĸ���
	BYTE byAudioNum;//�����ڵĸ���
	BYTE byIPChanNum;//�������ͨ����
}ANTS_MID_DEVICECFG,*LPANTS_MID_DEVICECFG;

//!IP��ַ
typedef struct{		
	char sIpV4[16];/* IPv4��ַ */
	BYTE byIPv6[128];/* ���� */
}ANTS_MID_IPADDR,*LPANTS_MID_IPADDR;

//!PPP��������(�ӽṹ)
typedef struct {
	ANTS_MID_IPADDR struRemoteIP;//Զ��IP��ַ
	ANTS_MID_IPADDR struLocalIP;//����IP��ַ
	char sLocalIPMask[16];//����IP��ַ����
	BYTE sUsername[ANTS_MID_NAME_LEN];/* �û��� */
	BYTE sPassword[ANTS_MID_PASSWD_LEN];/* ���� */
	BYTE byPPPMode;//PPPģʽ, 0��������1������
	BYTE byRedial;//�Ƿ�ز� ��0-��,1-��
	BYTE byRedialMode;//�ز�ģʽ,0-�ɲ�����ָ��,1-Ԥ�ûز�����
	BYTE byDataEncrypt;//���ݼ���,0-��,1-��
	DWORD dwMTU;//MTU
	char sTelephoneNumber[ANTS_MID_PHONENUMBER_LEN];//�绰����
}ANTS_MID_PPPCFG,*LPANTS_MID_PPPCFG;

//!RS232���ڲ�������
typedef struct{
    DWORD dwBaudRate;/*������(bps)��0��50��1��75��2��110��3��150��4��300��5��600��6��1200��7��2400��8��4800��9��9600��10��19200�� 11��38400��12��57600��13��76800��14��115.2k;*/
    BYTE byDataBit;/* �����м�λ 0��5λ��1��6λ��2��7λ��3��8λ */
    BYTE byStopBit;/* ֹͣλ 0��1λ��1��2λ */
    BYTE byParity;/* У�� 0����У�飬1����У�飬2��żУ�� */
    BYTE byFlowcontrol;/* 0���ޣ�1��������,2-Ӳ���� */
    DWORD	dwWorkMode;/* ����ģʽ��0��232��������PPP���ţ�1��232�������ڲ������ƣ�2��͸��ͨ�� */
}ANTS_MID_SINGLE_RS232;

//!RS232���ڲ�������
typedef struct {
	DWORD dwSize;
  ANTS_MID_SINGLE_RS232 struRs232;
	BYTE byRes[84]; 
	ANTS_MID_PPPCFG struPPPConfig;
}ANTS_MID_RS232CFG,*LPANTS_MID_RS232CFG;

//!���������������
typedef struct {
	DWORD dwSize;
	BYTE sAlarmInName[ANTS_MID_NAME_LEN];/* ���� */
	BYTE byAlarmType;//����������,0������,1������
	BYTE byAlarmInHandle;/* �Ƿ��� 0-������ 1-����*/
  BYTE byRes1[2];
	ANTS_MID_HANDLEEXCEPTION struAlarmHandleType;/* ����ʽ */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];//����ʱ��
	BYTE byRelRecordChan[ANTS_MID_MAX_CHANNUM];//����������¼��ͨ��,Ϊ1��ʾ������ͨ��
	BYTE byEnablePreset[ANTS_MID_MAX_CHANNUM];/* �Ƿ����Ԥ�õ� 0-��,1-��*/
	BYTE byPresetNo[ANTS_MID_MAX_CHANNUM];/* ���õ���̨Ԥ�õ����,һ������������Ե��ö��ͨ������̨Ԥ�õ�, 0xff��ʾ������Ԥ�õ㡣*/
	BYTE byRes2[192];/* ���� */
	BYTE byEnableCruise[ANTS_MID_MAX_CHANNUM];/* �Ƿ����Ѳ�� 0-��,1-��*/
	BYTE byCruiseNo[ANTS_MID_MAX_CHANNUM];/* Ѳ�� */
	BYTE byEnablePtzTrack[ANTS_MID_MAX_CHANNUM];/* �Ƿ���ù켣 0-��,1-��*/
	BYTE byPTZTrack[ANTS_MID_MAX_CHANNUM];/* ���õ���̨�Ĺ켣��� */
  BYTE byRes3[16];
}ANTS_MID_ALARMINCFG,*LPANTS_MID_ALARMINCFG;

//!���������������
typedef struct {
	DWORD dwSize;
	BYTE sAlarmInName[ANTS_MID_NAME_LEN];/* ���� */
	BYTE byAlarmType;//����������,0������,1������
	BYTE byAlarmInHandle;/* �Ƿ��� 0-������ 1-����*/
  BYTE byRes1[2];
	ANTS_MID_HANDLEEXCEPTION struAlarmHandleType;/* ����ʽ */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];//����ʱ��
	BYTE byRelRecordChan[ANTS_MID_MAX_CHANNUM_V2];//����������¼��ͨ��,Ϊ1��ʾ������ͨ��
	BYTE byEnablePreset[ANTS_MID_MAX_CHANNUM_V2];/* �Ƿ����Ԥ�õ� 0-��,1-��*/
	BYTE byPresetNo[ANTS_MID_MAX_CHANNUM_V2];/* ���õ���̨Ԥ�õ����,һ������������Ե��ö��ͨ������̨Ԥ�õ�, 0xff��ʾ������Ԥ�õ㡣*/
	BYTE byRes2[192];/* ���� */
	BYTE byEnableCruise[ANTS_MID_MAX_CHANNUM_V2];/* �Ƿ����Ѳ�� 0-��,1-��*/
	BYTE byCruiseNo[ANTS_MID_MAX_CHANNUM_V2];/* Ѳ�� */
	BYTE byEnablePtzTrack[ANTS_MID_MAX_CHANNUM_V2];/* �Ƿ���ù켣 0-��,1-��*/
	BYTE byPTZTrack[ANTS_MID_MAX_CHANNUM_V2];/* ���õ���̨�Ĺ켣��� */
  BYTE byRes3[16];
}ANTS_MID_ALARMINCFG_V2,*LPANTS_MID_ALARMINCFG_V2;

//!DVR�������
typedef struct {
	DWORD dwSize;
	BYTE sAlarmOutName[ANTS_MID_NAME_LEN];/* ���� */
	DWORD dwAlarmOutDelay;/* �������ʱ��(-1Ϊ���ޣ��ֶ��ر�) */
												//0-5��,1-10��,2-30��,3-1����,4-2����,5-5����,6-10����,7-�ֶ�
	ANTS_MID_SCHEDTIME struAlarmOutTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];/* �����������ʱ��� */
  BYTE byRes[16];
}ANTS_MID_ALARMOUTCFG,*LPANTS_MID_ALARMOUTCFG;

//!���û�����(�ӽṹ)
typedef struct{
	BYTE sUserName[ANTS_MID_NAME_LEN];/* �û��� */
	BYTE sPassword[ANTS_MID_PASSWD_LEN];/* ���� */
	BYTE byLocalRight[ANTS_MID_MAX_RIGHT];/* ����Ȩ�� */
														/*����0: ���ؿ�����̨*/
														/*����1: �����ֶ�¼��*/
														/*����2: ���ػط�*/
														/*����3: �������ò���*/
														/*����4: ���ز鿴״̬����־*/
														/*����5: ���ظ߼�����(��������ʽ�����������ػ�)*/
													  /*����6: ���ز鿴���� */
													  /*����7: ���ع���ģ���IP camera */
													  /*����8: ���ر��� */
													  /*����9: ���عػ�/���� */
	BYTE byRemoteRight[ANTS_MID_MAX_RIGHT];/* Զ��Ȩ�� */	
														/*����0: Զ�̿�����̨*/
														/*����1: Զ���ֶ�¼��*/
														/*����2: Զ�̻ط� */
														/*����3: Զ�����ò���*/
														/*����4: Զ�̲鿴״̬����־*/
														/*����5: Զ�̸߼�����(��������ʽ�����������ػ�)*/
														/*����6: Զ�̷��������Խ�*/
														/*����7: Զ��Ԥ��*/
														/*����8: Զ�����󱨾��ϴ����������*/
														/*����9: Զ�̿��ƣ��������*/
														/*����10: Զ�̿��ƴ���*/	
												    /*����11: Զ�̲鿴���� */
												    /*����12: Զ�̹���ģ���IP camera */
												    /*����13: Զ�̹ػ�/���� */
	BYTE byLocalPreviewRight[ANTS_MID_MAX_CHANNUM];/* ���ؿ���Ԥ����ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byNetPreviewRight[ANTS_MID_MAX_CHANNUM];/* Զ�̿���Ԥ����ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byLocalPlaybackRight[ANTS_MID_MAX_CHANNUM];/* ���ؿ��Իطŵ�ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byNetPlaybackRight[ANTS_MID_MAX_CHANNUM];/* Զ�̿��Իطŵ�ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byLocalRecordRight[ANTS_MID_MAX_CHANNUM];/* ���ؿ���¼���ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byNetRecordRight[ANTS_MID_MAX_CHANNUM];/* Զ�̿���¼���ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byLocalPTZRight[ANTS_MID_MAX_CHANNUM];/* ���ؿ���PTZ��ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byNetPTZRight[ANTS_MID_MAX_CHANNUM];/* Զ�̿���PTZ��ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byLocalBackupRight[ANTS_MID_MAX_CHANNUM];/* ���ر���Ȩ��ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	ANTS_MID_IPADDR struUserIP;/* �û�IP��ַ(Ϊ0ʱ��ʾ�����κε�ַ) */
	BYTE byMACAddr[ANTS_MID_MACADDR_LEN];/* �����ַ */
	BYTE byPriority;/* ���ȼ���0xff-�ޣ�0--�ͣ�1--�У�2--�� */
                  /*
                  �ޡ�����ʾ��֧�����ȼ�������
                  �͡���Ĭ��Ȩ��:�������غ�Զ�̻ط�,���غ�Զ�̲鿴��־��״̬,���غ�Զ�̹ػ�/����
                  �С����������غ�Զ�̿�����̨,���غ�Զ���ֶ�¼��,���غ�Զ�̻ط�,�����Խ���Զ��Ԥ��
                        ���ر���,����/Զ�̹ػ�/����
                  �ߡ�������Ա
                  */
	BYTE byRes[1];	
}ANTS_MID_USER_INFO,*LPANTS_MID_USER_INFO;

//���û�����(�ӽṹ)
typedef struct{
	BYTE sUserName[ANTS_MID_NAME_LEN];/* �û��� */
	BYTE sPassword[ANTS_MID_PASSWD_LEN];/* ���� */
	BYTE byLocalRight[ANTS_MID_MAX_RIGHT];/* ����Ȩ�� */
															/*����0: ���ؿ�����̨*/
															/*����1: �����ֶ�¼��*/
															/*����2: ���ػط�*/
															/*����3: �������ò���*/
															/*����4: ���ز鿴״̬����־*/
															/*����5: ���ظ߼�����(��������ʽ�����������ػ�)*/
													    /*����6: ���ز鿴���� */
													    /*����7: ���ع���ģ���IP camera */
													    /*����8: ���ر��� */
													    /*����9: ���عػ�/���� */
	BYTE byRemoteRight[ANTS_MID_MAX_RIGHT];/* Զ��Ȩ�� */	
															/*����0: Զ�̿�����̨*/
															/*����1: Զ���ֶ�¼��*/
															/*����2: Զ�̻ط� */
															/*����3: Զ�����ò���*/
															/*����4: Զ�̲鿴״̬����־*/
															/*����5: Զ�̸߼�����(��������ʽ�����������ػ�)*/
															/*����6: Զ�̷��������Խ�*/
															/*����7: Զ��Ԥ��*/
															/*����8: Զ�����󱨾��ϴ����������*/
															/*����9: Զ�̿��ƣ��������*/
															/*����10: Զ�̿��ƴ���*/	
													    /*����11: Զ�̲鿴���� */
													    /*����12: Զ�̹���ģ���IP camera */
													    /*����13: Զ�̹ػ�/���� */
	BYTE byLocalPreviewRight[ANTS_MID_MAX_CHANNUM_V2];/* ���ؿ���Ԥ����ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byNetPreviewRight[ANTS_MID_MAX_CHANNUM_V2];/* Զ�̿���Ԥ����ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byLocalPlaybackRight[ANTS_MID_MAX_CHANNUM_V2];/* ���ؿ��Իطŵ�ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byNetPlaybackRight[ANTS_MID_MAX_CHANNUM_V2];/* Զ�̿��Իطŵ�ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byLocalRecordRight[ANTS_MID_MAX_CHANNUM_V2];/* ���ؿ���¼���ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byNetRecordRight[ANTS_MID_MAX_CHANNUM_V2];/* Զ�̿���¼���ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byLocalPTZRight[ANTS_MID_MAX_CHANNUM_V2];/* ���ؿ���PTZ��ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byNetPTZRight[ANTS_MID_MAX_CHANNUM_V2];/* Զ�̿���PTZ��ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	BYTE byLocalBackupRight[ANTS_MID_MAX_CHANNUM_V2];/* ���ر���Ȩ��ͨ�� 1-��Ȩ�ޣ�0-��Ȩ��*/
	ANTS_MID_IPADDR struUserIP;/* �û�IP��ַ(Ϊ0ʱ��ʾ�����κε�ַ) */
	BYTE byMACAddr[ANTS_MID_MACADDR_LEN];/* �����ַ */
	BYTE byPriority;/* ���ȼ���0xff-�ޣ�0--�ͣ�1--�У�2--�� */
                  /*
                  �ޡ�����ʾ��֧�����ȼ�������
                  �͡���Ĭ��Ȩ��:�������غ�Զ�̻ط�,���غ�Զ�̲鿴��־��״̬,���غ�Զ�̹ػ�/����
                  �С����������غ�Զ�̿�����̨,���غ�Զ���ֶ�¼��,���غ�Զ�̻ط�,�����Խ���Զ��Ԥ��
                        ���ر���,����/Զ�̹ػ�/����
                  �ߡ�������Ա
                  */
	BYTE byRes[1];	
}ANTS_MID_USER_INFO_V2,*LPANTS_MID_USER_INFO_V2;

//!DVR�û�����
typedef struct{
	DWORD dwSize;
	ANTS_MID_USER_INFO struUser[ANTS_MID_MAX_USERNUM];
}ANTS_MID_USER,*LPANTS_MID_USER;

//!DVR�û�����
typedef struct{
	DWORD dwSize;
	ANTS_MID_USER_INFO_V2 struUser[ANTS_MID_MAX_USERNUM];
}ANTS_MID_USER_V2,*LPANTS_MID_USER_V2;

typedef struct{
	char sUserName[ANTS_MID_NAME_LEN];
	DWORD LoginTime;
	BOOL bLocal;
	ANTS_MID_IPADDR LoginIP;
}ANTS_MID_LOGINUSERINFO,*LPANTS_MID_LOGINUSERINFO;

//!DVR�쳣����
typedef struct {
	DWORD dwSize;
	ANTS_MID_HANDLEEXCEPTION struExceptionHandleType[ANTS_MID_MAX_EXCEPTIONNUM];
	/*����0-����,1- Ӳ�̳���,2-���߶�,3-��������IP ��ַ��ͻ, 4-�Ƿ�����, 5-����/�����Ƶ��ʽ��ƥ��, 6-��Ƶ�ź��쳣, 7-¼���쳣*/
}ANTS_MID_EXCEPTION,*LPANTS_MID_EXCEPTION;

//!ʱ���(�ӽṹ)
typedef struct {
	DWORD dwMonth;//��0-11��ʾ1-12����
	DWORD dwWeekNo;//�ڼ���0����1�� 1����2�� 2����3�� 3����4�� 4�����һ��
	DWORD dwWeekDate;//���ڼ�0�������� 1������һ 2�����ڶ� 3�������� 4�������� 5�������� 6��������
	DWORD dwHour;//Сʱ	��ʼʱ��0��23 ����ʱ��1��23
	DWORD dwMin;//��0��59
}ANTS_MID_TIMEPOINT,*LPANTS_MID_TIMEPOINT;

//!����ʱ����
typedef struct {
	DWORD dwSize;
	BYTE byRes1[16];//����
	DWORD dwEnableDST;//�Ƿ�������ʱ�� 0�������� 1������
	BYTE byDSTBias;//����ʱƫ��ֵ��30min, 60min, 90min, 120min, �Է��Ӽƣ�����ԭʼ��ֵ
	BYTE byRes2[3];
	ANTS_MID_TIMEPOINT struBeginPoint;//��ʱ�ƿ�ʼʱ��
	ANTS_MID_TIMEPOINT struEndPoint;//��ʱ��ֹͣʱ��
}ANTS_MID_ZONEANDDST,*LPANTS_MID_ZONEANDDST;

//!���ַ�����(�ӽṹ)
typedef struct {
	WORD wShowString;// Ԥ����ͼ�����Ƿ���ʾ�ַ�,0-����ʾ,1-��ʾ �����С704*576,�����ַ��Ĵ�СΪ32*32
	WORD wStringSize;/* �����ַ��ĳ��ȣ����ܴ���44���ַ� */
	WORD wShowStringTopLeftX;/* �ַ���ʾλ�õ�x���� */
	WORD wShowStringTopLeftY;/* �ַ�������ʾλ�õ�y���� */
	char sString[44];/* Ҫ��ʾ���ַ����� */
}ANTS_MID_SHOWSTRINGINFO,*LPANTS_MID_SHOWSTRINGINFO;

//!�����ַ�
typedef struct {
	DWORD dwSize;
	ANTS_MID_SHOWSTRINGINFO struStringInfo[ANTS_MID_MAX_STRINGNUM];/* Ҫ��ʾ���ַ����� */
}ANTS_MID_SHOWSTRING,*LPANTS_MID_SHOWSTRING;

//!����Ӳ����Ϣ����(�ӽṹ)
typedef struct{
    DWORD dwHDNo;/*Ӳ�̺�, ȡֵ0~ANTS_MID_MAX_DISKNUM-1*/
    DWORD dwCapacity;/*Ӳ������(��������)*/
    DWORD dwFreeSpace;/*Ӳ��ʣ��ռ�(��������)*/
    DWORD dwHdStatus;/*Ӳ��״̬(��������) HD_STAT*/
    BYTE byHDAttr;/*0-Ĭ��, 1-����; 2-ֻ��*/
		BYTE byHDType;/*0-����Ӳ��,1-ESATAӲ��,2-NASӲ��,3-iSCSIӲ�� 4-Array�������*/
		BYTE byRes1[2];
    DWORD dwHdGroup;/*�����ĸ����� 1-ANTS_MID_MAX_HD_GROUP*/
    BYTE byRes2[120];
}ANTS_MID_SINGLE_HD,*LPANTS_MID_SINGLE_HD;

typedef struct{
    DWORD dwSize;
    DWORD dwHDCount;/*Ӳ����(��������)*/
    ANTS_MID_SINGLE_HD struHDInfo[ANTS_MID_MAX_DISKNUM];//Ӳ����ز�������Ҫ����������Ч��
}ANTS_MID_HDCFG,*LPANTS_MID_HDCFG;

//!����������Ϣ����
typedef struct{
    DWORD dwHDGroupNo;/*�����(��������) 1-ANTS_MID_MAX_HD_GROUP*/        
    BYTE byHDGroupChans[ANTS_MID_MAX_CHANNUM];/*�����Ӧ��¼��ͨ��, 0-��ʾ��ͨ����¼�󵽸����飬1-��ʾ¼�󵽸�����*/
    BYTE byRes[8];
}ANTS_MID_SINGLE_HDGROUP,*LPANTS_MID_SINGLE_HDGROUP;

//!����������Ϣ����
typedef struct{
    DWORD dwHDGroupNo;/*�����(��������) 1-ANTS_MID_MAX_HD_GROUP*/        
    BYTE byHDGroupChans[ANTS_MID_MAX_CHANNUM_V2];/*�����Ӧ��¼��ͨ��, 0-��ʾ��ͨ����¼�󵽸����飬1-��ʾ¼�󵽸�����*/
    BYTE byRes[8];
}ANTS_MID_SINGLE_HDGROUP_V2,*LPANTS_MID_SINGLE_HDGROUP_V2;

typedef struct{
    DWORD dwSize;
    DWORD dwHDGroupCount;/*��������(��������)*/
    ANTS_MID_SINGLE_HDGROUP struHDGroupAttr[ANTS_MID_MAX_HD_GROUP];//Ӳ����ز�������Ҫ����������Ч��
}ANTS_MID_HDGROUP_CFG,*LPANTS_MID_HDGROUP_CFG;

typedef struct{
    DWORD dwSize;
    DWORD dwHDGroupCount;/*��������(��������)*/
    ANTS_MID_SINGLE_HDGROUP_V2 struHDGroupAttr[ANTS_MID_MAX_HD_GROUP];//Ӳ����ز�������Ҫ����������Ч��
}ANTS_MID_HDGROUP_CFG_V2,*LPANTS_MID_HDGROUP_CFG_V2;

//!�����Խ�����
typedef struct{
	BYTE byAudioEncType;//��Ƶ�������� 0-OggVorbis;1-G711_U;2-G711_A;3-G726(Ĭ��)
	BYTE byres[7];//���ﱣ����Ƶ��ѹ������ 
}ANTS_MID_COMPRESSION_AUDIO,*LPANTS_MID_COMPRESSION_AUDIO;

//!�Զ�ά������
typedef struct{
	BYTE byAutoRebootMode;//�Զ�ά��ģʽ:0--��ά����1--ÿ�춨ʱά����2--ÿ�ܶ�ʱά����3--����ά��
	DWORD dwSingleTime;//����ά��ʱ��:time_t����
	DWORD dwEveryDayTime;//ÿ��ά��ʱ��:0-7λ�Ƿ��ӣ�8-15λ��Сʱ
	BOOL bWeeklyDay[ANTS_MID_MAX_DAYS];//ÿ��7���Ƿ�����ά��:0--�����죬1--����һ����������
	DWORD dwWeeklyTime[ANTS_MID_MAX_DAYS];//ÿ��ά��ʱ��:0-7λ�Ƿ��ӣ�8-15λ��Сʱ
}ANTS_MID_AUTOREBOOT,*LPANTS_MID_AUTOREBOOT;

//!PPPOE�ṹ
typedef struct {
	DWORD dwPPPOE;//0-������,1-����
	BYTE sPPPoEUser[ANTS_MID_NAME_LEN];//PPPoE�û���
	char sPPPoEPassword[ANTS_MID_PASSWD_LEN];//PPPoE����
	ANTS_MID_IPADDR	struPPPoEIP;//PPPoE IP��ַ
}ANTS_MID_PPPOECFG,*LPANTS_MID_PPPOECFG;

/*�������ݽṹ(�ӽṹ)*/
typedef struct {
	ANTS_MID_IPADDR struDVRIP;//DVR IP��ַ
	ANTS_MID_IPADDR struDVRIPMask;//DVR IP��ַ����
	DWORD dwNetInterface;//����ӿ�1-10MBase-T 2-10MBase-Tȫ˫�� 3-100MBase-TX 4-100Mȫ˫�� 5-10M/100M����Ӧ 6-100M/1000M����Ӧ
	WORD wDVRPort;//�˿ں�
	WORD wMTU;//����MTU����,Ĭ��1500
	BYTE byMACAddr[ANTS_MID_MACADDR_LEN];//�����ַ
	BYTE byRes[2];//��������
}ANTS_MID_ETHERNET,*LPANTS_MID_ETHERNET;

//!�������ýṹ
typedef struct{
	DWORD dwSize;
	ANTS_MID_ETHERNET struEtherNet[ANTS_MID_MAX_ETHERNET];//��̫����
	ANTS_MID_IPADDR struRes1[2];/*����*/
	ANTS_MID_IPADDR struAlarmHostIpAddr;/* ��������IP��ַ */
	WORD wHttpsPort;/*HTTPS�˿�*/	
	WORD wRes2[1];/* ���� */
	WORD wAlarmHostIpPort;/* ���������˿ں� */
	BYTE byUseDhcp;/* �Ƿ�����DHCP 0xff-��Ч 0-������ 1-����*/
	BYTE byRes3;
	ANTS_MID_IPADDR struDnsServer1IpAddr;/* ����������1��IP��ַ */
	ANTS_MID_IPADDR	struDnsServer2IpAddr;/* ����������2��IP��ַ */
	BYTE byIpResolver[ANTS_MID_MAX_DOMAIN_NAME];	/* IP����������������IP��ַ */
	WORD wIpResolverPort;/* IP�����������˿ں� */
	WORD wHttpPortNo;/* HTTP�˿ں� */
	ANTS_MID_IPADDR struMulticastIpAddr;/* �ಥ���ַ */
	ANTS_MID_IPADDR struGatewayIpAddr;/* ���ص�ַ */
	ANTS_MID_PPPOECFG struPPPoE;	
	char szManagerHostIpV4[32];/*����ע�������IP��ַ0-������1-����*/
	WORD wManagerHostPort;/*����ע��������˿�*/
	BYTE byUseManagerHost;/*�Ƿ���������ע�����0-������1-����*/
	BYTE byRes[29];
}ANTS_MID_NETCFG,*LPANTS_MID_NETCFG;

//!NTP
typedef struct {
	BYTE sNTPServer[64];/* Domain Name or IP addr of NTP server */
	WORD wInterval;/* adjust time interval(hours) */
	BYTE byEnableNTP;/* enable NPT client 0-no��1-yes*/
	signed char cTimeDifferenceH;/* ����ʱ�׼ʱ��� Сʱƫ��-12 ... +13 */
	signed char cTimeDifferenceM;/* ����ʱ�׼ʱ��� ����ƫ��0, 30, 45*/
	BYTE res1;
	WORD wNtpPort;/* ntp server port �豸Ĭ��Ϊ123*/
	BYTE res2[8];
}ANTS_MID_NTPPARA,*LPANTS_MID_NTPPARA;

//!DDNS
typedef struct {
	BYTE byEnableDDNS;
	BYTE byHostIndex;/* 0-˽��DDNS 1��Dyndns 2��PeanutHull(������) 3- NO-IP 4-qdns*/
	BYTE byRes1[2];
  struct{    
		BYTE sUsername[ANTS_MID_NAME_LEN];/* DDNS�˺��û���*/
		BYTE sPassword[ANTS_MID_PASSWD_LEN];/* ���� */
		BYTE sDomainName[ANTS_MID_MAX_DOMAIN_NAME];/* �豸�䱸��������ַ */
		BYTE sServerName[ANTS_MID_MAX_DOMAIN_NAME];/* DDNSЭ���Ӧ�ķ�������ַ��������IP��ַ������ */
		WORD wDDNSPort;/* �˿ں� */
		WORD wCheckIPIntervalTime;                 		/*IP�����ʱ��,��λ��*/
		WORD wUpdateIPIntervalTime;                 		/*IP���¼��ʱ��,��λ��*/
		BYTE byRes[6];		
  }struDDNS[ANTS_MID_MAX_DDNS_NUMS];
	BYTE byRes2[16];
}ANTS_MID_DDNSPARA,*LPANTS_MID_DDNSPARA;

//!�����������
typedef struct {
	DWORD dwSize;
	char sDNSIp[16];/* DNS��������ַ */
	ANTS_MID_NTPPARA struNtpClientParam;/* NTP���� */
	ANTS_MID_DDNSPARA struDDNSClientParam;/* DDNS���� */
	BYTE res[464];/* ���� */
}ANTS_MID_NETAPPCFG,*LPANTS_MID_NETAPPCFG;

/*EMAIL�����ṹ*/
typedef struct{		
	DWORD dwSize;
	BYTE sAccount[ANTS_MID_NAME_LEN];/* �˺�*/ 
	BYTE sPassword[ANTS_MID_MAX_EMAIL_PWD_LEN];/*���� */
	struct{
		BYTE sName[ANTS_MID_NAME_LEN];/* ���������� */
		BYTE sAddress[ANTS_MID_MAX_EMAIL_ADDR_LEN];/* �����˵�ַ */
	}struSender;

	BYTE sSmtpServer[ANTS_MID_MAX_EMAIL_ADDR_LEN];/* smtp������ */
	BYTE sPop3Server[ANTS_MID_MAX_EMAIL_ADDR_LEN];/* pop3������ */

	struct{
		BYTE sName[ANTS_MID_NAME_LEN];/* �ռ������� */
		BYTE sAddress[ANTS_MID_MAX_EMAIL_ADDR_LEN];/* �ռ��˵�ַ */
	}struReceiver[3];/* ����������3���ռ��� */

	BYTE byAttachment;/* �Ƿ������ */
	BYTE bySmtpServerVerify;/* ���ͷ�����Ҫ�������֤ */
  BYTE byMailInterval;/* mail interval */
	BYTE byEnableSSL;//ssl�Ƿ�����
	WORD wSmtpPort;//gmail��465����ͨ��Ϊ25     
	BYTE byRes[74];//����
}ANTS_MID_EMAILCFG,*LPANTS_MID_EMAILCFG;

typedef struct{
	DWORD dwSize;//!�ṹ����
	BYTE byEnable;//!0-����SNMP��1-��ʾ����SNMP
	BYTE byRes1[3];//!����
	WORD wVersion;//!snmp �汾  v1 = 1, v2 =2, v3 =3���豸Ŀǰ��֧�� v3
	WORD wServerPort;//!snmp��Ϣ���ն˿ڣ�Ĭ�� 161
	BYTE byReadCommunity[ANTS_MID_NAME_LEN];//!����ͬ�壬���31,Ĭ��"public"
	BYTE byWriteCommunity[ANTS_MID_NAME_LEN];//!д��ͬ��,���31 �ֽ�,Ĭ�� "private"
	BYTE byTrapHostIP[ANTS_MID_DESC_LEN_64];//!��������ip��ַ������֧��IPV4 IPV6����������    
	WORD wTrapHostPort;//!trap�����˿�
	BYTE byRes2[102];//!����
}ANTS_MID_SNMPCFG,*LPANTS_MID_SNMPCFG;

typedef struct{
	ANTS_MID_IPADDR struDVRIP; //!DVR IP��ַ
	ANTS_MID_IPADDR struDVRIPMask; //!DVR IP��ַ����
	DWORD dwNetInterface; //!����ӿ�1-10MBase-T 2-10MBase-Tȫ˫�� 3-100MBase-TX 4-100Mȫ˫�� 5-10M/100M����Ӧ
	BYTE byRes1[2];
	WORD wMTU; //!����MTU���ã�Ĭ��1500��
	BYTE byMACAddr[ANTS_MID_MACADDR_LEN];//!�����ַ��ֻ������ʾ
	BYTE byRes2[2];//!����
	BYTE byUseDhcp;//!�Ƿ�����DHCP 
	BYTE byRes3[3];
	ANTS_MID_IPADDR struGatewayIpAddr;//!���ص�ַ 
	ANTS_MID_IPADDR struDnsServer1IpAddr;//!����������1��IP��ַ 
	ANTS_MID_IPADDR struDnsServer2IpAddr;//!����������2��IP��ַ 
}ANTS_MID_ETHERNET_MULTI,*LPANTS_MID_ETHERNET_MULTI;

typedef struct{
	DWORD dwSize;
	BYTE byDefaultRoute;//!Ĭ��·�ɣ�0��ʾstruEtherNet[0]��1��ʾstruEtherNet[1]
	BYTE byNetworkCardNum;//!�豸ʵ�ʿ����õ�������Ŀ
	BYTE byUPNP;//! 1-����UPNP��0-�ر�UPNP
	BYTE byRes[1];				
	ANTS_MID_ETHERNET_MULTI struEtherNet[ANTS_MID_MAX_NETWORK_CARD];	//!��̫����
	ANTS_MID_IPADDR struManageHost1IpAddr;			//!����������IP��ַ 
	ANTS_MID_IPADDR struManageHost2IpAddr;			//!����������IP��ַ 
	ANTS_MID_IPADDR struAlarmHostIpAddr;			//!��������IP��ַ 
	WORD wManageHost1Port;				//!�����������˿ں� 
	WORD wManageHost2Port;				//!�����������˿ں� 
	WORD wAlarmHostIpPort;				//!���������˿ں� 
	BYTE byUseManagerHost1;				/*�Ƿ���������ע�����0-������1-����*/
	BYTE byUseManagerHost2;				/*�Ƿ���������ע�����0-������1-����*/
	BYTE byIpResolver[ANTS_MID_MAX_DOMAIN_NAME];	//!IP����������������IP��ַ 
	WORD wIpResolverPort;				//!IP�����������˿ں� 
	WORD wDvrPort;						//!ͨѶ�˿� Ĭ��8000 
	WORD wHttpPortNo;					//!HTTP�˿ں� 
	BYTE byRes2[6];
	ANTS_MID_IPADDR struMulticastIpAddr;			//!�ಥ���ַ
	ANTS_MID_PPPOECFG struPPPoE;
	BYTE byRes3[24];
}ANTS_MID_NETCFG_MULTI, *LPANTS_MID_NETCFG_MULTI;

typedef struct{
	DWORD dwSize;//!����
	WORD wPort;//!RTSPrtsp�����������˿�
	BYTE byReserve[54];//!Ԥ��
}ANTS_MID_RTSPCFG,*LPANTS_MID_RTSPCFG;

typedef struct{
	char sNfsHostIPAddr[16];
	BYTE sNfsDirectory[ANTS_MID_PATHNAME_LEN];	// ANTS_PATHNAME_LEN = 128
}ANTS_MID_SINGLE_NFS, *LPANTS_MID_SINGLE_NFS;

typedef struct{
	DWORD dwSize;
	ANTS_MID_SINGLE_NFS struNfsDiskParam[ANTS_MID_MAX_NFS_DISK];
}ANTS_MID_NFSCFG,*LPANTS_MID_NFSCFG;

typedef struct{
	BYTE byNetDiskType;//!����Ӳ������, 0-NFS,1-iSCSI
	BYTE byRes1[3];//!����
	ANTS_MID_IPADDR struNetDiskAddr;//!����Ӳ�̵�ַ
	BYTE sDirectory[ANTS_MID_PATHNAME_LEN];//!ANTS_PATHNAME_LEN = 128
	WORD wPort;//!iscsi�ж˿ڣ�����ΪĬ��
	BYTE byRes2[66];//!����
}ANTS_MID_SINGLE_NET_DISK_INFO,*LPANTS_MID_SINGLE_NET_DISK_INFO;

typedef struct{
	DWORD dwSize;
	ANTS_MID_SINGLE_NET_DISK_INFO struNetDiskParam[ANTS_MID_MAX_NET_DISK];
}ANTS_MID_NET_DISKCFG, *LPANTS_MID_NET_DISKCFG;

typedef struct{
	char sFileName[100];//!ͼƬ��
	ANTS_MID_TIME struTime;//!ͼƬ��ʱ��
	DWORD dwFileSize;//!ͼƬ�Ĵ�С
	char sCardNum[32];//!����
}ANTS_MID_FIND_PICTURE,*LPANTS_MID_FIND_PICTURE;

typedef struct{
	ANTS_MID_TIME strLogTime;
	DWORD dwMajorType;//!������
	DWORD dwMinorType;//!������
	BYTE sPanelUser[ANTS_MID_MAX_NAMELEN];//!���������û���
	BYTE sNetUser[ANTS_MID_MAX_NAMELEN];//!����������û���
	ANTS_MID_IPADDR struRemoteHostAddr;//!Զ��������ַ
	DWORD dwParaType;//!��������,9000�豸MINOR_START_VT/MINOR_STOP_VTʱ����ʾ�����Խ��Ķ��Ӻ�
	DWORD dwChannel;//!ͨ����
	DWORD dwDiskNumber;//!Ӳ�̺�
	DWORD dwAlarmInPort;//!��������˿�
	DWORD dwAlarmOutPort;//!��������˿�
	DWORD dwInfoLen;
	char sInfo[ANTS_MID_LOG_INFO_LEN];
}ANTS_MID_LOG,*LPANTS_MID_LOG;

typedef struct{
	ANTS_MID_TIME strLogTime;
	DWORD dwMajorType;//!������
	DWORD dwMinorType;//!������
	BYTE sPanelUser[ANTS_MID_MAX_NAMELEN];//!���������û���
	BYTE sNetUser[ANTS_MID_MAX_NAMELEN];//!����������û���
	ANTS_MID_IPADDR	struRemoteHostAddr;//!Զ��������ַ
	DWORD wParaType;//!��������,9000�豸MINOR_START_VT/MINOR_STOP_VTʱ����ʾ�����Խ��Ķ��Ӻ�
	DWORD dwChannel;//!ͨ����
	DWORD dwDiskNumber;//!Ӳ�̺�
	DWORD dwAlarmInPort;//!��������˿�
	DWORD dwAlarmOutPort;//!��������˿�
	DWORD dwInfoLen;
	char sInfo[4];
}ANTS_MID_LOG_V2,*LPANTS_MID_LOG_V2;

typedef struct{
	DWORD dwVolume;
	DWORD dwFreeSpace;
	DWORD dwHardDiskStatic;
}ANTS_MID_DISKSTATE,*LPANTS_MID_DISKSTATE;

typedef struct{
	BYTE byRecordStatic;//!ͨ���Ƿ���¼��,0-��¼��,1-¼��
	BYTE bySignalStatic;//!���ӵ��ź�״̬,0-����,1-�źŶ�ʧ
	BYTE byHardwareStatic;//!ͨ��Ӳ��״̬,0-����,1-�쳣,����DSP����
	BYTE byRes1;
	DWORD dwBitRate;//!ʵ������
	DWORD dwLinkNum;//!�ͻ������ӵĸ���
	ANTS_MID_IPADDR struClientIP[ANTS_MID_MAX_LINK];//!�ͻ��˵�IP��ַ
	DWORD dwIPLinkNum;//!�����ͨ��ΪIP���룬��ô��ʾIP���뵱ǰ��������
	BYTE byRes[12];
}ANTS_MID_CHANNELSTATE,*LPANTS_MID_CHANNELSTATE;

typedef struct{
	DWORD dwDeviceStatic;//!�豸��״̬,0-����,1-CPUռ����̫��,����85%,2-Ӳ������,���紮������
	ANTS_MID_DISKSTATE struHardDiskStatic[ANTS_MID_MAX_DISKNUM];
	ANTS_MID_CHANNELSTATE struChanStatic[ANTS_MID_MAX_CHANNUM];	//!ͨ����״̬
	BYTE byAlarmInStatic[ANTS_MID_MAX_ALARMIN];//!�����˿ڵ�״̬,0-û�б���,1-�б���
	BYTE byAlarmOutStatic[ANTS_MID_MAX_ALARMOUT];//!��������˿ڵ�״̬,0-û�����,1-�б������
	DWORD dwLocalDisplay;//!������ʾ״̬,0-����,1-������
	BYTE byAudioChanStatus[ANTS_MID_MAX_AUDIO];//!��ʾ����ͨ����״̬ 0-δʹ�ã�1-ʹ����, 0xff��Ч
	BYTE byRes[10];
}ANTS_MID_WORKSTATE,*LPANTS_MID_WORKSTATE;

typedef struct{
	DWORD dwDeviceStatic;//!�豸��״̬,0-����,1-CPUռ����̫��,����85%,2-Ӳ������,���紮������
	ANTS_MID_DISKSTATE struHardDiskStatic[ANTS_MID_MAX_DISKNUM];
	ANTS_MID_CHANNELSTATE struChanStatic[ANTS_MID_MAX_CHANNUM_V2];	//!ͨ����״̬
	BYTE byAlarmInStatic[ANTS_MID_MAX_ALARMIN_V2];//!�����˿ڵ�״̬,0-û�б���,1-�б���
	BYTE byAlarmOutStatic[ANTS_MID_MAX_ALARMOUT];//!��������˿ڵ�״̬,0-û�����,1-�б������
	DWORD dwLocalDisplay;//!������ʾ״̬,0-����,1-������
	BYTE byAudioChanStatus[ANTS_MID_MAX_AUDIO];//!��ʾ����ͨ����״̬ 0-δʹ�ã�1-ʹ����, 0xff��Ч
	BYTE byRes[10];
}ANTS_MID_WORKSTATE_V2,*LPANTS_MID_WORKSTATE_V2;

typedef struct{
	DWORD dwDeviceStatic;//!�豸��״̬,0-����,1-CPUռ����̫��,����85%,2-Ӳ������,���紮������
	ANTS_MID_DISKSTATE struHardDiskStatic[ANTS_MID_MAX_DISKNUM_V2];
	ANTS_MID_CHANNELSTATE struChanStatic[ANTS_MID_MAX_CHANNUM_V2];	//!ͨ����״̬
	BYTE byAlarmInStatic[ANTS_MID_MAX_ALARMIN_V2];//!�����˿ڵ�״̬,0-û�б���,1-�б���
	BYTE byAlarmOutStatic[ANTS_MID_MAX_ALARMOUT];//!��������˿ڵ�״̬,0-û�����,1-�б������
	DWORD dwLocalDisplay;//!������ʾ״̬,0-����,1-������
	BYTE byAudioChanStatus[ANTS_MID_MAX_AUDIO];//!��ʾ����ͨ����״̬ 0-δʹ�ã�1-ʹ����, 0xff��Ч
	BYTE byRes[10];
}ANTS_MID_WORKSTATE_V3,*LPANTS_MID_WORKSTATE_V3;

typedef struct{
	LONG lChannel;
	DWORD dwFileType;
	DWORD dwIsLocked;
	DWORD dwUseCardNo;
	BYTE sCardNumber[32];
	ANTS_MID_TIME struStartTime;
	ANTS_MID_TIME struStopTime;
}ANTS_MID_FILECOND,*LPANTS_MID_FILECOND;

typedef struct{
  char sFileName[100];
  ANTS_MID_TIME struStartTime;
  ANTS_MID_TIME struStopTime;
  DWORD dwFileSize;
  char sCardNum[32];
  BYTE byLocked;
  BYTE byFileType;
  BYTE byRes[2];
}ANTS_MID_FINDDATA,*LPANTS_MID_FINDDATA;

typedef struct{
	DWORD dwType;
	BYTE byDescribe[ANTS_MID_IPC_PROTOCOL_DESC_LEN];
	BYTE byEnable; // ��ǰ�Ƿ�����
	BYTE byRes[15];
}ANTS_MID_PROTO_TYPE,*LANTS_MID_PROTO_TYPE;

typedef struct{
	DWORD dwSize;
	DWORD dwProtoNum;
	ANTS_MID_PROTO_TYPE struProto[ANTS_MID_IPC_PROTOCOL_NUM];
	BYTE byRes[8];
}ANTS_MID_IPC_PROTO_LIST,*LPANTS_MID_IPC_PROTO_LIST;

#define ANTS_MID_MAX_SUPPORTDEVICEPROTOL      (64)

typedef struct {
	BYTE byMac[8];// �޸�ʱ���������
	int bDhcpEnable;
	char szIpAddr[16];
	char szNetMask[16];
	char szGateway[16];
	char szDns1[16];
	char szDns2[16];
	BYTE byRes[128];
}ANTS_MID_IPV4,*LPANTS_MID_IPV4;

typedef struct {
	DWORD dwSize;
	DWORD dwSupportProtocol;//֧�ֵ�Э��
	DWORD dwVideoPort[ANTS_MID_MAX_SUPPORTDEVICEPROTOL];//!˽��Э��˿�
	char	szDeviceName[64];
	char szUserName[40];
	char szPassword[40];
	ANTS_MID_IPV4 struIpv4Info;
	BYTE byDeviceType;//0-δ����� (��Ĭ��ΪIPC),  1-dvr,2-nvr,3-ipc,4-dec
	BYTE byRes[127];
}ANTS_MID_REARCH_DEVICEINFO,*LPANTS_MID_REARCH_DEVICEINFO;

typedef struct{
	LONG lChannel;
	ANTS_MID_TIME struStartTime;
	ANTS_MID_TIME struStopTime;
	BYTE byDiskDes[ANTS_MID_PATHNAME_LEN];
	BYTE byWithPlayer;
	BYTE byRes[35];
}ANTS_MID_BACKUP_TIME_PARAM,*LPANTS_MID_BACKUP_TIME_PARAM;

typedef struct{
	WORD wPicSize;//!ͼƬ�ߴ磺0-CIF��1-QCIF��2-D1��3-UXGA(1600x1200)��
								//!4-SVGA(800x600)��5-HD720p(1280x720)��6-VGA��7-XVGA��8-HD900p��
								//!9-HD1080��10-2560*1920��11-1600*304��12-2048*1536��13-2448*2048��
								//!14-2448*1200��15-2448*800��16-XGA(1024*768)��17-SXGA(1280*1024)��18-WD1(960*576/960*480),19-1080i 
	WORD wPicQuality;//!ͼƬ����ϵ����0-��ã�1-�Ϻã�2-һ�� 
}ANTS_MID_JPEGPARA,*LPANTS_MID_JPEGPARA;

typedef struct{
	BYTE Output[ANTS_MID_MAX_ALARMOUT];
}ANTS_MID_ALARMOUTSTATUS,*LPANTS_MID_ALARMOUTSTATUS;

typedef struct{
	DWORD dwType;
	BYTE byDescribe[ANTS_MID_DESC_LEN];
}ANTS_MID_PTZ_PROTOCOL,*LPANTS_MID_PTZ_PROTOCOL;

typedef struct{
	DWORD dwSize;
	ANTS_MID_PTZ_PROTOCOL struPtz[ANTS_MID_PTZ_PROTOCOL_NUM];
	DWORD dwPtzNum;
	BYTE byRes[8];
}ANTS_MID_PTZCFG,*LPANTS_MID_PTZCFG;

typedef struct{
	BYTE PresetNum;
	BYTE Dwell;
	BYTE Speed;
	BYTE Reserve;
}ANTS_MID_CRUISE_POINT,*LPANTS_MID_CRUISE_POINT;

typedef struct{
	ANTS_MID_CRUISE_POINT struCruisePoint[32];
}ANTS_MID_CRUISE_RET,*LPANTS_MID_CRUISE_RET;

//����˽ṹ��
typedef struct{
	char sDVRIP[16];
	DWORD dwDVRPort;
	DWORD dwChannel;
	DWORD dwTransProtocol;
	DWORD dwTransMode;
	DWORD dwLinkProtocol;//!0-TCP;1-UDP
	DWORD dwEXMode;//!�Ƿ�������ǿ����ģʽ
	char sUserName[ANTS_MID_NAME_LEN];
	char sPassword[ANTS_MID_PASSWD_LEN];
	char sRtspMain[ANTS_MID_PATHNAME_LEN];
	char sRtspAux[ANTS_MID_PATHNAME_LEN];
	char byRes[16];//!�Ƿ�������ǿ����ģʽ
}ANTS_MID_IPCINFO,*LPANTS_MID_IPCINFO;

typedef struct{
	DWORD	dwSize;
	ANTS_MID_IPCINFO struIPCInfos[ANTS_MID_MAX_IPCNUM];
}ANTS_MID_IPCCFG,*LPANTS_MID_IPCCFG;

typedef struct{
	DWORD	dwSize;
	ANTS_MID_IPCINFO struIPCInfos[ANTS_MID_MAX_IPCNUM_V2];
}ANTS_MID_IPCCFG_V2,*LPANTS_MID_IPCCFG_V2;

typedef struct {
	char  sSsid[ANTS_MID_MAX_WIFI_ESSID_SIZE];
	DWORD dwMode;/* 0 mange ģʽ;1 ad-hocģʽ���μ�NICMODE */
	DWORD dwSecurity;/*0 �����ܣ�1 wep���ܣ�2 wpa-psk;3 wpa-Enterprise���μ�WIFISECURITY*/
	DWORD dwChannel;/*1-11��ʾ11��ͨ��*/
	DWORD dwSignalStrength;/*0-100�ź���������Ϊ��ǿ*/
	DWORD dwSpeed;/*����,��λ��0.01mbps*/
}ANTS_MID_AP_INFO,*LPANTS_MID_AP_INFO;

typedef struct {
	DWORD dwSize;
	DWORD dwCount;/*����AP������������20*/
	ANTS_MID_AP_INFO struApInfo[ANTS_MID_MAX_WIFI_AP_COUNT];
}ANTS_MID_AP_INFO_LIST,*LPANTS_MID_AP_INFO_LIST;

typedef struct {	
	char sIpAddress[16];/*IP��ַ*/
	char sIpMask[16];/*����*/	
	BYTE byMACAddr[ANTS_MID_WIFI_MACADDR_LEN];/*�����ַ��ֻ������ʾ*/
	BYTE bRes[2];
	DWORD dwEnableDhcp;/*�Ƿ�����dhcp  0������ 1����*/
	DWORD dwAutoDns;/*�������dhcp�Ƿ��Զ���ȡdns,0���Զ���ȡ 1�Զ���ȡ�����������������dhcpĿǰ�Զ���ȡdns*/	
	char sFirstDns[16];/*��һ��dns����*/
	char sSecondDns[16];/*�ڶ���dns����*/
	char sGatewayIpAddr[16];/* ���ص�ַ*/
	BYTE bRes2[8];
}ANTS_MID_WIFIETHERNET,*LPANTS_MID_WIFIETHERNET;

typedef struct {
	/*wifi����*/
	ANTS_MID_WIFIETHERNET struEtherNet;
	/*SSID*/
	char sEssid[ANTS_MID_MAX_WIFI_ESSID_SIZE];
	/* 0 mange ģʽ;1 ad-hocģʽ���μ�*/
	DWORD dwMode;
	/*0 �����ܣ�1 wep���ܣ�2 wpa-psk; */
	DWORD dwSecurity;
	union{
		struct _tagkey{
			/*0 -����ʽ 1-����ʽ*/
			DWORD dwAuthentication;
			/* 0 -64λ��1- 128λ��2-152λ*/
			DWORD dwKeyLength;
			/*0 16����;1 ASCI */
			DWORD dwKeyType;
			/*0 ������0---3��ʾ����һ����Կ*/
			DWORD dwActive;
			char sKeyInfo[ANTS_MID_MAX_WIFI_WEP_KEY_COUNT][ANTS_MID_MAX_WIFI_WEP_KEY_LENGTH];
		}wep;
		struct {
			/*8-63��ASCII�ַ�*/
			DWORD dwKeyLength;
			char sKeyInfo[ANTS_MID_MAX_WIFI_WPA_PSK_KEY_LENGTH];
			char sRes;
		}wpa_psk;
	}key;
}ANTS_MID_WIFI_CFG_EX,*LPANTS_MID_WIFI_CFG_EX;

//!wifi���ýṹ
typedef struct {
	DWORD dwSize;
	ANTS_MID_WIFI_CFG_EX struWifiCfg;
}ANTS_MID_WIFI_CFG,*LPANTS_MID_WIFI_CFG;

//!wifi����ģʽ
typedef struct {
	DWORD dwSize;
	DWORD dwNetworkInterfaceMode; /*0 �Զ��л�ģʽ��1 ����ģʽ*/
}ANTS_MID_WIFI_WORKMODE,*LPANTS_MID_WIFI_WORKMODE;

//!3G
typedef struct {
	DWORD dwSize;
	BOOL bEnable;//!�Ƿ�����3G��������
	char szAPNAddr[32];//!APN��ַ
	char szTelePhone[32];//!�κź���
	char szIPAddr[32];//!3G IP��ַ
	DWORD dwWorkMode;/*!
				3G����ģʽ
				00-��ʾ��ADSL���粢�й���
				01-��ʾADSL����Ͽ�030�����
				02-��ʾADSL����Ͽ�035�����
				03-��ʾADSL����Ͽ�040�����
				04-��ʾADSL����Ͽ�045�����
				05-��ʾADSL����Ͽ�050�����
				06-��ʾADSL����Ͽ�055�����
				07-��ʾADSL����Ͽ�060�����
				08-��ʾADSL����Ͽ�065�����
				09-��ʾADSL����Ͽ�070�����
				10-��ʾADSL����Ͽ�075�����
				11-��ʾADSL����Ͽ�080�����
				12-��ʾADSL����Ͽ�085�����
				13-��ʾADSL����Ͽ�090�����
				14-��ʾADSL����Ͽ�095�����					
				*/
	DWORD dwDeviceType;//!�豸����
				/*
				0-ZTE MF100 WCDMA
				1-HUAWEI E156G WCDMA
				2-VITION E1916 CDMA2000
				*/
	BYTE byRes[128];	
}ANTS_MID_3G_CFG,*LPANTS_MID_3G_CFG;

//!��ý���������������
typedef struct {
	BOOL bEnableManagerHost;
	char szManagerHost[128];
	WORD wManagerHostPort;
	BYTE byRes[2];
}ANTS_MID_MANAGERHOST,*LPANTS_MID_MANAGERHOST;

typedef struct {
	DWORD dwSize;
	ANTS_MID_MANAGERHOST struManagerHosts[ANTS_MID_MAX_MANAGERHOST_NUM];
	BYTE byRes[128];
}ANTS_MID_MANAGERHOST_CFG,*LPANTS_MID_MANAGERHOST_CFG;

//!3G���������ͼ�����
typedef struct {
	DWORD dwType;//!3G����ֵ
	BYTE byDescribe[ANTS_MID_MAX_3G_DEVICE_DESC_LEN];
	BYTE byISPDescribe[16];
}ANTS_MID_3G_DEVICE,*LPANTS_MID_3G_DEVICE;

typedef struct {
	DWORD dwSize;
	DWORD dw3GDevNum;
	ANTS_MID_3G_DEVICE stru3Gs[ANTS_MID_MAX_3G_DEVICE_NUM];
	BYTE byRes[256];
}ANTS_MID_3GDEVICE_CFG,*LPANTS_MID_3GDEVICE_CFG;

//!DDNS ��������
typedef struct {
	DWORD dwIndex;//!DDNS����ֵ
	BYTE byDescribe[ANTS_MID_DDNS_SERVICE_DESC_LEN];
	BYTE byServerName[64];
	WORD wServerPort;
	WORD wRes;
}ANTS_MID_DDNS_SERVICE,*LPANTS_MID_DDNS_SERVICE;

typedef struct {
	DWORD dwSize;
	DWORD dwDdnsServiceNum;
	ANTS_MID_DDNS_SERVICE struDdnsServices[ANTS_MID_DDNS_SERVICE_NUM];
	BYTE byRes[256];
}ANTS_MID_DDNSSERVICE_ABILITY,*LPANTS_MID_DDNSSERVICE_ABILITY;

typedef struct{
	char szIP[16];
	char szMask[16];
	char szGateWay[16];
	char szDns1[16];
	char szDns2[16];
	BYTE byMacAddr[ANTS_MID_MACADDR_LEN];
	BYTE byRes[14];
}ANTS_MID_DISCOVERY_INFO,*LPANTS_MID_DISCOVERY_INFO;

typedef struct{
	ANTS_MID_DISCOVERY_INFO struDiscoveryInfos[3];//0-eth0 1-wifi 2-3g
	WORD wPorts[4];//0-Private 1-Http 2-Rtsp 3-����
	BYTE bySerialNo[ANTS_MID_SERIALNO_LEN];
	char szName[ANTS_MID_NAME_LEN];
	char szPwd[ANTS_MID_PASSWD_LEN];
	char szDeviceType[32];//!DVR-04 DVR-08 DVR-16 NVR-04 NVR-08 NVR-16  IPC
	DWORD dwSoftwareVersion;//����?t��?��?o?,??16??��??�¡�?��?,�̨�16??��?��?��?��?
	DWORD dwSoftwareBuildDate;//����?t����3����??��,0xYYYYMMDD
	BYTE byRes[24];
}ANTS_MID_DISCOVERYCFG,*LPANTS_MID_DISCOVERYCFG;

//!��ȡNVR/DVR/IPC��ͨ������
typedef struct{
	DWORD dwSize;
	char szChanName[ANTS_MID_NAME_LEN];
	BYTE byRes[4];
}ANTS_MID_DEVCHANNELNAME_CFG,*LPANTS_MID_DEVCHANNELNAME_CFG;

//!��ȡNVR/DVR/IPC��ͨ������
typedef struct{
	DWORD dwSize;
	DWORD dwChanNum;
	char szChanName[ANTS_MID_MAX_CHANNUM_V2][ANTS_MID_NAME_LEN];
}ANTS_MID_DEVCHANNELNAME_CFG_V2,*LPANTS_MID_DEVCHANNELNAME_CFG_V2;

typedef struct{
	DWORD dwSize;
	DWORD dwHDCount;/*Ӳ����(��������)*/
	ANTS_MID_SINGLE_HD struHDInfo[ANTS_MID_MAX_DISKNUM_V2];//Ӳ����ز�������Ҫ����������Ч��
}ANTS_MID_HDCFG_V2, *LPANTS_MID_HDCFG_V2;

typedef struct {
	unsigned int uiFrameNo;					//!֡��
	unsigned int uiFrameTime;				//!UTCʱ��
	unsigned int uiFrameTickCount;			//!����Ϊ��λ�ĺ���ʱ��
	unsigned int uiRelativeTime;				//!���ʱ�����
	unsigned short usWidth ;				//!��Ƶ���
	unsigned short usHeight ;				//!��Ƶ�߶�
	unsigned char ucCodecId ;				//!��Ƶcocdecid����Ƶcodecid
	unsigned char ucFrameType;				//!֡����
	unsigned char ucReserve[2] ;				//!����λ
}ANTS_MID_FRAMEHEADER,*LPANTS_MID_FRAMEEADER;

/*******************�������ýṹ������ end*********************/

/********************************SDK�ӿں�������*********************************/
#ifdef __cplusplus
extern "C"{
#endif

typedef void (*fVoiceStreamCallBack)(IN LONG lVoiceHandle,IN BYTE *lpBuffer,IN DWORD dwSize,IN LPVOID lpUser);

//!Added by ItmanLee at 2013-01-10
typedef BOOL (*fCheckPassword)(IN const char *lpUserName,IN const char *lpPassword);

//!������ȡ������
typedef BOOL (*fSetParameter)(IN DWORD dwCommand,IN DWORD dwChannel,IN LPVOID lpBuffer,IN DWORD dwSize);
typedef BOOL (*fGetParameter)(IN DWORD dwCommand,IN DWORD dwChannel,OUT LPVOID lpBuffer,IN DWORD dwInSize,OUT DWORD *lpOutSize);

//!PTZ����
typedef BOOL (*fPtzControl)(IN DWORD dwChannel,IN DWORD dwPtzCommand,IN DWORD dwStop,IN DWORD dwSpeed);
typedef BOOL (*fPtzPreset)(IN DWORD dwChannel,IN DWORD dwPresetCommand,IN DWORD dwPresetIndex);
typedef BOOL (*fPtzTrack)(IN DWORD dwChannel,IN DWORD dwTrackCommand);
typedef BOOL (*fPtzCruise)(IN DWORD dwChannel,IN DWORD dwCruiseCommand,IN BYTE byCruiseRoute,IN BYTE byCruisePoint,IN WORD wInput);
typedef BOOL (*fPtzTrans)(IN DWORD dwChannel,IN BYTE *lpBuffer,IN DWORD dwSize);
typedef BOOL (*fPtz3D)(IN DWORD dwChannel,IN DWORD dwXPoint,IN DWORD dwYPoint,IN DWORD dwScale);
typedef BOOL (*fGetPtzCruise)(IN DWORD dwChannel,IN DWORD dwCruiseRoute,OUT LPANTS_MID_CRUISE_RET lpCruiseRet);

//!Jpegץͼ����
typedef BOOL (*fCaptureJpeg)(IN DWORD dwChannel,IN LPANTS_MID_JPEGPARA lpJpegParam,OUT BYTE *lpBuffer,IN DWORD dwInSize,OUT DWORD *lpOutSize);

//!��ȡͨ��ʵʱ����������������Ϣ
typedef BOOL (*fGetChannelBitRate)(IN DWORD dwChannel,OUT DWORD *lpMainStreamBitRate,OUT DWORD *lpSubStreamBitRate);

//!�������״̬��ȡ������
typedef BOOL (*fGetAlarmOut)(OUT LPANTS_MID_ALARMOUTSTATUS lpAlarmOutStatus);
typedef BOOL (*fSetAlarmOut)(IN LONG lAlarmOutPort,IN LONG lAlarmOutStatic);

typedef BOOL (*fGetAlarmInfo)(OUT DWORD *lpCommand,OUT BYTE *lpBuffer,OUT DWORD *lpBufLen);

//!�Խ�����
typedef LONG (*fStartVoice)(IN DWORD dwVoiceChannel,IN fVoiceStreamCallBack fVoiceStream,IN void *lpUser);
typedef BOOL (*fStopVoice)(IN LONG lVoiceHandle);
typedef BOOL (*fSendVoiceData)(IN LONG lVoiceHandle,IN BYTE *lpBuffer,IN DWORD dwSize);

//!�豸����/�ػ�/�ָ�Ĭ��ֵ/�����������
typedef BOOL (*fReboot)( );

//!��ȡSDK�汾��������
typedef DWORD (*fGetSDKVersion)( );
typedef DWORD (*fGetLastError)( );

//!Added by ItmanLee at 2013-04-22 ǿ��I֡
typedef BOOL (*fMakeIFrame)(IN DWORD dwStreamType,IN DWORD dwChannel);

//!Added by ItmanLee at 2013-05-22 ֪ͨ���������������
typedef BOOL (*fInputData)(IN DWORD dwStreamType,IN DWORD dwChannel,IN BOOL bEnable,DWORD dwReserve);

//!Added by ItmanLee at 2013-10-26 ��ʷ���طſ���
typedef BOOL (*fHistoryStreamCallBack )(IN LONG lPlayHandle,IN LPANTS_MID_FRAMEEADER lpFrameHeader,IN BYTE *lpBuffer,IN DWORD dwSize,IN void* lpUser);
typedef LONG (*fStartHistoryStream)(IN DWORD dwChannel,IN LPANTS_MID_TIME lpStartTime,IN LPANTS_MID_TIME lpStopTime,fHistoryStreamCallBack fpHistoryStreamCallBack,void *lpUser);
typedef BOOL (*fControlHistoryStream)(IN LONG lPlayHandle,IN DWORD dwCommand,IN BYTE *lpInBuffer,DWORD dwInLen,BYTE *lpOutBuffer,DWORD *lpOutLen);
typedef BOOL (*fStopHistoryStream)(IN LONG lPlayHandle);

//!Added by ItmanLee at 2013-11-18 ��ʷ���ļ���ѯ
typedef LONG (*fStartFindFile)(IN LPANTS_MID_FILECOND lpFindFileCond);
typedef LONG (*fFindNextFile)(IN LONG lFileHandle,OUT LPANTS_MID_FINDDATA lpFindData);
typedef BOOL (*fStopFindFile)(IN LONG lFileHandle);

//!Added by ItmanLee at 2013-11-27 ��ѯĳ��ĳ����ǰ�������Ƿ���¼����
//!@dwStartYear:��ʾ��Ҫ��ѯ��ݴ�1900��ʼ
//!@dwStartMonth:��ʾ��Ҫ��ѯ���·ݴ�1�¿�ʼ
//!@dwDays:��ʾ��Ҫ��ѯ���·ݶ�����
//!@lpResultContent:��ʾ���·��Ƿ���¼����,��λ����
typedef BOOL (*fQueryFileMonthly)(IN DWORD dwStartYear,IN DWORD dwStartMonth,IN DWORD dwDays,OUT DWORD *lpResultContent);

typedef BOOL (*fChangeStream)(IN DWORD dwStreamType,IN DWORD dwChannel);

#ifdef __cplusplus
}
#endif
#endif

