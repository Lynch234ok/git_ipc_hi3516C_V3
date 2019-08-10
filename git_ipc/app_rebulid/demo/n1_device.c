
/**
 * N1 �豸ʵ��������\n
 * ������ͨ������ N1 �豸ģ�飨�¼��ģ�飩��ؽӿڣ�\n
 * ʵ�־������� N1 �豸�� N1 �ͻ��ˣ��¼�ƿͻ��ˣ������Ĺ��̡�\n
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <n1_protocol/NkUtils/log.h>
#include <n1_protocol/NkUtils/macro.h>
#include <stdarg.h>

#include "n1_protocol/n1_device.h"
#include "n1_protocol/x264_cache.h"
#include "sdk/sdk_enc.h"
#include "media_buf.h"
/**
 * ����ģ�鹫�о����
 */

NK_N1Device public_N1Device;

typedef struct Session_USER {
	lpMEDIABUF_USER usersession;
	NK_PVoid sessionbuf;
	NK_UInt32 stream_id;	
}stSession_USER, *lpSession_USER;

/**
 * �����豸����¼���\n
 * �����������пͻ��˳��Է����豸ʱ��ģ��ᴥ�����¼���\n
 * �û�ͨ��ʵ�ִ��¼���֪�ÿͻ������������ֱ���� IP ��Ϣ���Լ������豸��Ϣ�ȡ�\n
 * ʵ����ͨ����ʵ�ִ˽ӿڸ��ͻ����ṩ�豸��Ϣ�������豸����ͨѶ��\n
 *
 */
static NK_N1Ret
n1_onDiscoveryGetInfo(NK_N1DeviceInfo *DeviceInfo, NK_PVoid ctx)
{
	int i = 0;

	/// �����豸������Ϣ��
	SET_TEXT(DeviceInfo->device_code,		"N1TEST");
	SET_TEXT(DeviceInfo->model,			"Developer 3518e1");
	SET_TEXT(DeviceInfo->name,				"Developer 3518e1");
	SET_TEXT(DeviceInfo->eseeID,			"N10123456666");

	/// ������̫����Ϣ��
	DeviceInfo->Ethernet.dhcp_enabled = NK_False;
	SET_TEXT(DeviceInfo->Ethernet.ip_address,		"192.168.30.169");
	SET_TEXT(DeviceInfo->Ethernet.netmask,			"255.255.0.0");
	SET_TEXT(DeviceInfo->Ethernet.mac_address,		"00:9A:16:65:96:AF");
	SET_TEXT(DeviceInfo->Ethernet.gateway,			"192.168.1.1");
	SET_TEXT(DeviceInfo->Ethernet.preferred_dns,	"192.168.1.1");
	SET_TEXT(DeviceInfo->Ethernet.alternative_dns,	"192.168.1.1");

	/// ���� HTTP ��Ϣ��
	DeviceInfo->HTTP.listen_port = LISTEN_PORT;

	/// ���� DDNS ��Ϣ��
	DeviceInfo->DDNS.enabled = NK_True;
	SET_TEXT(DeviceInfo->DDNS.user, "DDNS-User");
	SET_TEXT(DeviceInfo->DDNS.password, "DDNS-Password");
	SET_TEXT(DeviceInfo->DDNS.host, "www.ddns.com");
	DeviceInfo->DDNS.port = 53;
	/// ����ֱ��ͨ����Ϣ��
	DeviceInfo->live_channels = 1;
	for (i = 0; i < DeviceInfo->live_channels; ++i) {
		DeviceInfo->LiveChannels[i].streams = 2;
	}

	NK_Log()->info("n1_onDiscoveryGetInfo: DeviceInfo->device_code:%s, DeviceInfo->Ethernet.ip_address:%s\n", DeviceInfo->device_code, DeviceInfo->Ethernet.ip_address);
	return NK_N1_ERR_NONE;
}


static inline int
SYSTEM(const char *fmt, ...)
{
	char command[4 * 1024];
	int ret = 0;
	va_list var;

	va_start(var, fmt);
	vsnprintf(command, sizeof(command), fmt, var);
	va_end(var);

	return system(command);
}

/**
 * �����豸����¼���\n
 * ���ͻ��˷��ֵ��豸ʱ�����������������ñ�����ڲ�����޷�������\n
 * �ͻ��˻�ͨ�������豸��ʽʹ�����ھ����������㽻��������\n
 * �ͻ�������ʱ�ᴥ�������¼���\n
 * ʵ����ͨ����ʵ�ִ˽ӿ��������绷��ʹ�ͻ��˺��豸�ھ����������㽻��������\n
 *
 */
static NK_N1Ret
n1_onDiscoverySetup(NK_N1DiscoverySetup *Setup, NK_PVoid ctx)
{
	/**
	 * ������ӡ���һ�� NK_N1DiscoverySetup ���ݽṹ��
	 */
	NK_N1_DICOVERY_SETUP_DUMP(Setup);
	
	/**
	*����IP��ַ
	*/
	//Todo
	printf("Setup->_NetConf->ip_address = %s\n", Setup->_NetConf.ip_address);
	SYSTEM("ifconfig eth0:1 %s netmask %s;", Setup->_NetConf.ip_address, Setup->_NetConf.netmask);
	SYSTEM("ifconfig eth0:1 hw ether %s\n", Setup->_NetConf.mac_address);

	//SYSTEM("ifconfig eth0:1 %s", Setup->_NetConf->ip_address);

	if (NK_False) {
		/**
		 * ������������д��ڴ����������������ֵ��
		 */
		return NK_N1_ERR_INVALID_PARAM;
	}

	/**
	 * ���������������óɹ���
	 */
	return NK_N1_ERR_NONE;
}

/**
 * ģ��ץͼ����Դ�ļ�·����
 */
#define LIVE_IMG_FILE_PATH "./test.jpg"


/**
 * �ֳ�ץͼ�¼���
 *
 */
static NK_N1Ret
n1_onLiveSnapshot(NK_Int channel_id, NK_Size width, NK_Size height, NK_PByte pic, NK_Size *size)
{
	FILE *fID = NK_Nil;
	NK_SSize readn = 0;

	NK_Log()->info("Test: Snapshot (Channel = %d, Size = %dx%d, Stack = %d).", channel_id, width, height, *size);

	fID = fopen(LIVE_IMG_FILE_PATH, "rb");
	if (!fID) {
		NK_Log()->info("Test: Open File (Path = %s) Failed.", LIVE_IMG_FILE_PATH);
		return NK_N1_ERR_DEVICE_BUSY;
	}

	readn = fread(pic, 1, *size, fID);
	fclose(fID);
	fID = NK_Nil;

	if (readn < 0) {
		return NK_N1_ERR_DEVICE_BUSY;
	}

	*size = readn;

	NK_Log()->info("Test: Snapshot Completed (Size = %d).", *size);
	return NK_N1_ERR_NONE;
}

/**
 * ��ý��ֱ������¼���\n
 * ���ͻ��˷��ֵ��豸ʱ�����������������ñ�����ڲ�����޷�������\n
 * �ͻ��˻�ͨ�������豸��ʽʹ�����ھ����������㽻��������\n
 * �ͻ�������ʱ�ᴥ�������¼���\n
 */
static NK_N1Ret
n1_onLiveConnected(NK_N1LiveSession *Session, NK_PVoid ctx)
{
	/**
	 *
	 * �������ֱ��ʲ�������� 1920x1080 ���أ�֡�ʲ�������� 30 ֡\n
	 * �������ֱ��ʲ�������� 704x480 ���أ�֡�ʲ�������� 30 ֡��\n
	 * ����ʹ�ͻ��˴�������Դ���Ĺ���Ӱ�����顣
	 *
	 */
	stSession_USER *Session_user = malloc(sizeof(stSession_USER));
	Session_user->usersession = NULL;
	Session_user->sessionbuf = NULL;

	Session->Video.payload_type = NK_N1_DATA_PT_H264_NALUS;
	if (0 == Session->stream_id) {
		Session->Video.width = 1280;
		Session->Video.heigth = 720;
	} else {
		Session->Video.width = 640;
		Session->Video.heigth = 480;
	}
	Session->Audio.payload_type = NK_N1_DATA_PT_G711A;
	Session->Audio.sample_rate = 8000;
	Session->Audio.sample_bitwidth = 16;
	Session->Audio.stereo = NK_False;
	Session->user_session = NK_Nil;
		if(!Session->user_session)
		{
			int mediabuf_ch;
			switch(Session->stream_id)
			{
			case 0:		
				mediabuf_ch = MEDIABUF_lookup_byname("720p.264"); 
				Session->Video.width = 1280;
				Session->Video.heigth = 720;
				Session_user->stream_id = 2;				
				break;
			case 1:		
				mediabuf_ch = MEDIABUF_lookup_byname("360p.264"); 
				Session->Video.width = 640;
				Session->Video.heigth = 360;
				Session_user->stream_id = 1;								
			break;
			case 2:		mediabuf_ch = MEDIABUF_lookup_byname("qvga.264"); break;
			default:	mediabuf_ch = MEDIABUF_lookup_byname("720p.264"); break;
			}

			Session_user->sessionbuf = malloc(Session->Video.heigth * Session->Video.heigth);
			if(mediabuf_ch >= 0)
			{
					Session_user->usersession = MEDIABUF_attach(mediabuf_ch);					
					Session->user_session = (typeof(Session->user_session))(Session_user);
				if(NULL == Session_user->usersession)
				{
					NK_Log()->info("Media pool is full!");
					sleep(3);
					if(NULL != Session_user->sessionbuf)
					{
						free(Session_user->sessionbuf);
						Session_user->sessionbuf = NULL;
					}
					if(NULL != Session_user)
					{
						free(Session_user);
						Session_user = NULL;
					}
					return NK_N1_ERR_DEVICE_BUSY;
				}
				else
				{
					MEDIABUF_sync(Session_user->usersession);
					return NK_N1_ERR_NONE;
				}
			}
		}
		
	return NK_N1_ERR_NONE;
}


static NK_N1Ret
n1_onLiveDisconnected(NK_N1LiveSession *Session, NK_PVoid ctx)
{
        lpSession_USER mysession = (lpSession_USER)(Session->user_session);

        if(NULL != mysession->usersession)
        {
                MEDIABUF_detach(mysession->usersession);
                mysession->usersession = NK_Nil;
        }

        if(NULL != mysession->sessionbuf)
        {
                free(mysession->sessionbuf);
        }

        NK_Log()->info("Test: Live on Channel: %d Stream: %d Disconnected.",
                        Session->channel_id, Session->stream_id, Session->session_id);

        return NK_N1_ERR_NONE;
}

static NK_SSize
n1_onLiveReadFrame(NK_N1LiveSession *Session, NK_PVoid ctx,
		NK_N1DataPayload *payload_type, NK_UInt32 *ts_ms, NK_PByte *data)
{
	/**
	 *
	 * �����������ݰ��ܴ�С��������� 1M �ֽڣ�\n
	 * �����������ݰ��ܴ�С��������� 300k �ֽڣ�\n
	 * ����ͻ��˿�������˲ʱ�ڴ治����ɽ����쳣��
	 *
	 */
	int Length = 0;
	lpSession_USER mysession = (lpSession_USER)(Session->user_session);
	FILE *fID = NULL;
	FILE *fvo = NULL;
	char fileName[128]={0};
	char fileName2[128]={0};
	bool out_frame = false;
	int send_ret = 0;
	static unsigned char is_first_i_frame = 1, i_frame_cnt = 0;

	if((NULL != mysession->usersession))
	{
		if(0 == MEDIABUF_out_lock(mysession->usersession))
		{
			void* send_ptr = 0;
			ssize_t send_sz = 0;
			
			// out a frame from media buf
			if(0 == MEDIABUF_out(mysession->usersession, &send_ptr, NULL, &send_sz))
			{
				const lpSDK_ENC_BUF_ATTR const attr = (lpSDK_ENC_BUF_ATTR)send_ptr;
				if(attr->type == kSDK_ENC_BUF_DATA_H264 || attr->type == kSDK_ENC_BUF_DATA_H265)
				{//video
						memcpy((NK_Byte *)mysession->sessionbuf, (NK_Byte *)(send_ptr + sizeof(stSDK_ENC_BUF_ATTR)), send_sz - sizeof(stSDK_ENC_BUF_ATTR));
						Length = send_sz - sizeof(stSDK_ENC_BUF_ATTR);
						if(attr->type == kSDK_ENC_BUF_DATA_H264)
						{
							*payload_type = NK_N1_DATA_PT_H264_NALUS;
						}
						if(attr->type == kSDK_ENC_BUF_DATA_H265)
						{
							*payload_type = NK_N1_DATA_PT_H265;
						}
						*data = (NK_PByte)(mysession->sessionbuf);
						*ts_ms = attr->timestamp_us;
				}
			}
			// out unlock
			MEDIABUF_out_unlock(mysession->usersession);
		}
		
		if(NULL != fID)
		{
			fclose(fID);
			fID = NULL;
		}
		if(NULL != fvo)
		{
			fclose(fvo);
			fvo = NULL;
		}
	}
	return Length;
}

/**
 * �豸��������������¼���\n
 *
 */
NK_N1Ret
n1_onLanSetup(NK_PVoid ctx, NK_Boolean set_or_get, NK_N1LanSetup *LanSetup)
{
	switch (LanSetup->classify) {

		case NK_N1_LAN_SETUP_TIME:
		{
			/**
			 * �����豸ʱ�䡣
			 */
			struct tm setup_tm;
			if (set_or_get) {
				/**
				 * �����豸ʱ�������
				 */

				/// ת���ɿ��Ķ���ʱ���ʽ��
				gmtime_r((time_t *)(&LanSetup->Time.utc), &setup_tm);

//				NK_Log()->info("Test: Set Time %04d:%02d:%02d %02d:%02d:%02d GMT",
//						setup_tm.tm_year + 1900, setup_tm.tm_mon + 1, setup_tm.tm_mday,
//						setup_tm.tm_hour, setup_tm.tm_min, setup_tm.tm_sec);

			} else {
				/**
				 * ��ȡ�豸ʱ��������á�
				 */
				/// TODO
			}
		}
		break;

		/**
		 * @name ������Ƶ�������������
		 */
		case NK_N1_LAN_SETUP_VENC:
		{
			typeof(LanSetup->VideoEncoder) *VideoEncoder = &LanSetup->VideoEncoder;

			/**
			 * ������Ƶ���������
			 */
			if (set_or_get) {

				/**
				 * ������Ƶ���������
				 */
				NK_Log()->info("Test: Set Video Encoder Attributes.");
				NK_N1_LAN_SETUP_DUMP(LanSetup);

			} else {

				/**
				 * ��ȡ��Ƶ����������á�
				 */

				typeof(VideoEncoder->H264) *H264 = &VideoEncoder->H264;

				NK_BZERO(H264, sizeof(H264[0]));

				VideoEncoder->Codec.val = NK_N1_VENC_CODEC_H264;
				VideoEncoder->Codec.def = NK_N1_VENC_CODEC_H264;
				NK_N1_PROP_ADD_ENUM(&VideoEncoder->Codec, N1VideoEncCodec, NK_N1_VENC_CODEC_H264);

				H264->FrameRate.val = 30;
				H264->FrameRate.def = 30;
				H264->FrameRate.min = 5;
				H264->FrameRate.max = 30;

				H264->KeyFrameInterval.val = 60;
				H264->KeyFrameInterval.def = 60;
				H264->KeyFrameInterval.min = 30;
				H264->KeyFrameInterval.max = 60;

				H264->BitRateCtrlMode.val = NK_N1_BR_CTRL_MODE_CBR;
				H264->BitRateCtrlMode.def = NK_N1_BR_CTRL_MODE_CBR;
				NK_N1_PROP_ADD_ENUM(&H264->BitRateCtrlMode, N1BitRateCtrlMode, NK_N1_BR_CTRL_MODE_CBR);
				NK_N1_PROP_ADD_ENUM(&H264->BitRateCtrlMode, N1BitRateCtrlMode, NK_N1_BR_CTRL_MODE_VBR);

				if (0 == LanSetup->stream_id) {
					H264->Resolution.val = NK_N1_IMG_SZ_1280X720;
					H264->Resolution.def = NK_N1_IMG_SZ_1280X960;
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_1920X1080);
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_1280X960);
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_1280X720);
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_960X480);
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_704X480);
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_640X480);
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_640X360);
					H264->BitRate.val = 2000;
					H264->BitRate.val = 1600;
					H264->BitRate.min = 320;
					H264->BitRate.max = 2000;
				} else {
					H264->Resolution.val = NK_N1_IMG_SZ_704X480;
					H264->Resolution.def = NK_N1_IMG_SZ_640X480;
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_704X480);
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_640X480);
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_640X360);
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_352X240);
					NK_N1_PROP_ADD_ENUM(&H264->Resolution, N1ImageSize, NK_N1_IMG_SZ_160X90);
					H264->BitRate.val = 500;
					H264->BitRate.val = 320;
					H264->BitRate.min = 160;
					H264->BitRate.max = 1000;
				}

			}
		}
		break;

		/**
		 * @name ������Ƶͼ�����������
		 */
		case NK_N1_LAN_SETUP_VIMG:
		{
			typeof(LanSetup->VideoImage) *VideoImage = &LanSetup->VideoImage;

			if (set_or_get) {

				/**
				 * ����ͼ�������
				 */

				NK_Log()->info("Test: Set Video Image Attributes.");
				NK_N1_LAN_SETUP_DUMP(LanSetup);

			} else {

				/**
				 * ��ȡͼ��������á�
				 */
				NK_Log()->info("Test: Get Video Image Attributes.");

				VideoImage->PowerLineFrequenceMode.val = 60;
				VideoImage->PowerLineFrequenceMode.def = 60;
				NK_N1_PROP_ADD_OPT(&VideoImage->PowerLineFrequenceMode, 50);
				NK_N1_PROP_ADD_OPT(&VideoImage->PowerLineFrequenceMode, 60);
				NK_N1_PROP_ADD_OPT(&VideoImage->PowerLineFrequenceMode, 100);
				NK_N1_PROP_ADD_OPT(&VideoImage->PowerLineFrequenceMode, 120);

				VideoImage->CaptureResolution.val = NK_N1_IMG_SZ_1280X960;
				VideoImage->CaptureResolution.def = NK_N1_IMG_SZ_1280X960;
				NK_N1_PROP_ADD_OPT(&VideoImage->CaptureResolution, NK_N1_IMG_SZ_1280X960);

				VideoImage->CaptureFrameRate.val = 30;
				VideoImage->CaptureFrameRate.def = 30;
				NK_N1_PROP_ADD_OPT(&VideoImage->CaptureFrameRate, 25);
				NK_N1_PROP_ADD_OPT(&VideoImage->CaptureFrameRate, 30);

				VideoImage->HueLevel.min = VideoImage->BrightnessLevel.min \
						= VideoImage->SharpnessLevel.min \
						= VideoImage->ContrastLevel.min \
						= VideoImage->SaturationLevel.min = 0;
				VideoImage->HueLevel.max = VideoImage->BrightnessLevel.max \
						= VideoImage->SharpnessLevel.max \
						= VideoImage->ContrastLevel.max \
						= VideoImage->SaturationLevel.max = 100;

				VideoImage->HueLevel.val = VideoImage->HueLevel.def = 50;
				VideoImage->BrightnessLevel.val = VideoImage->BrightnessLevel.def = 50;
				VideoImage->ContrastLevel.val = VideoImage->ContrastLevel.def = 50;
				VideoImage->SaturationLevel.val = VideoImage->SaturationLevel.def = 50;
				VideoImage->SharpnessLevel.val = VideoImage->SharpnessLevel.def = 50;

			}
		}
		break;

		/**
		 * @name ��̨�����������������
		 */
		case NK_N1_LAN_SETUP_PTZ:
		{
			NK_N1_LAN_SETUP_DUMP(LanSetup);
		}
		break;

		default:
		break;
	}

	if (NK_False) {
		/**
		 * ������������д��ڴ����������������ֵ��
		 */
		return NK_N1_ERR_INVALID_PARAM;
	}

	/**
	 * ���óɹ���
	 */
	return NK_N1_ERR_NONE;
}

static NK_Void
wait_for_quit()
{
	printf("Please Click 'q' to Exit Test Application.\r\n");
	while (sleep(1), 'Q' != toupper(getchar()));
	printf("Waiting for Exit.\r\n");
}

/**
 * ʵ��������
 * return :��ʼ���ɹ����ص�ַ -1��ʼ������
 *
 */
NK_N1Device  *n1_Device_main(unsigned char *device_id)
{
	NK_N1Device *const N1Device = &public_N1Device;
	/**
	 * ������־�ն�����ȼ���
	 */
#if defined(_DEBUG)
	NK_Log()->setConsoleLevel(NK_LOG_ALL);
#endif

	/**
	 * ������ݽṹ��������Ч��������ڴ����
	 */
	NK_BZERO(N1Device, sizeof(NK_N1Device));
	/**
	 * �����豸���豸�ţ����豸�������ھ���������Ϊ�豸��Ψһ��ʶ��\n
	 * ���ÿһ���豸���벻һ����
	 */
	snprintf(N1Device->device_id, sizeof(N1Device->device_id),
			device_id);
	/**
	 * Э��ʹ�ö˿ڡ�
	 * ģ��ͨ��ʹ�ô˶˿ڽ��� TCP/IP �������������
	 * ����������Ӧ�ñ���˶˿���ģ������� TCP/IP �˿ڳ�ͻ��
	 */
	N1Device->port = LISTEN_PORT;
	N1Device->user_ctx = NK_Nil;
	N1Device->EventSet.onDiscoveryGetInfo = n1_onDiscoveryGetInfo;
	N1Device->EventSet.onDiscoverySetup = n1_onDiscoverySetup;
	N1Device->EventSet.onLiveSnapshot = n1_onLiveSnapshot;
	N1Device->EventSet.onLiveConnected = n1_onLiveConnected;
	N1Device->EventSet.onLiveDisconnected = n1_onLiveDisconnected;
	N1Device->EventSet.onLiveReadFrame = n1_onLiveReadFrame;
	N1Device->EventSet.onLanSetup  = n1_onLanSetup;



	/**
	 * ��ʼ�� N1 �豸������
	 */
	if (0 != NK_N1Device_Init(N1Device)) {
		return NULL;
		//exit(EXIT_FAILURE);
	}
	/**
	 * �ȴ��û���ֹӦ�á�
	 */
	wait_for_quit();
	return N1Device;

	/**
	 * ���� N1 �豸������
	 */
//	NK_N1Device_Destroy(&N1Device);

//	exit(EXIT_SUCCESS);
}


