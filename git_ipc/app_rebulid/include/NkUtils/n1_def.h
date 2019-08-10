/*
 * N1 Э�����ݽṹ���塣
 *
 */

#include <stdio.h>
#include <NkUtils/types.h>
#include <NkUtils/log.h>
#include <NkUtils/n1_property.h>
#include <NkUtils/n1_enum.h>
#include <NkUtils/assert.h>


#ifndef NK_UTILS_N1_DEF_H_
#define NK_UTILS_N1_DEF_H_
NK_CPP_EXTERN_BEGIN



typedef enum Nk_N1Ret
{
	/**
	 * δ����Ĵ���
	 */
	NK_N1_ERR_UNDEF					= (-1),

	/**
	 * û�д��󣬲����ɹ���
	 */
	NK_N1_ERR_NONE					= (0),

	/**
	 * ����������ء�
	 */
	NK_N1_ERR_INVALID_PARAM			= (101),	///< ��������
	NK_N1_ERR_INVALID_CHANNEL_STREAM_ID,		///< ͨ��/������ ID ����

	/**
	 * �豸�ڲ�������ء�
	 */
	NK_N1_ERR_DEVICE_BUSY			= (201),	///<�豸æµ���п��ܸù������ڱ�ռ�á�
	NK_N1_ERR_DEVICE_NOT_SUPPORT,				///< �豸��֧�֡�

	/**
	 * �����豸������ء�
	 */
	NK_N1_ERR_INVALID_OPERATE		= (301),	///< ��Ч���豸������
	NK_N1_ERR_OPERATE_TIMEOUT,					///< �豸������ʱ��

	/**
	 * �������ݰ�����
	 */
	NK_N1_ERR_INVALID_DATAGRAM		= (401),	///< ��Ч�Ĵ��䱨�ġ�


	/**
	 * �û�У��ʧ�ܡ�
	 */
	NK_N1_ERR_NOT_AUTHORIZATED		= (1001),

} NK_N1Error;

/**
 * ���ݰ汾 < 1.4.0
 */
#define NK_N1Ret NK_N1Error

/**
 * N1 ֱ��������������͡�
 */
typedef enum Nk_N1DataPayload
{
	NK_N1_DATA_PT_UNDEF				= (-1),
	/**
	 * G.711 A-Law ��Ƶ�������ݡ�
	 */
	NK_N1_DATA_PT_G711A				= (8),
	/**
	 * H.264 Nal-Unit ��Ƶ�������ݡ�
	 */
	NK_N1_DATA_PT_H264_NALUS		= (96),
	/**
	 * H.265 Nal-Unit ��Ƶ�������ݡ�
	 */
	NK_N1_DATA_PT_H265_NALUS		= (97),

	NK_N1_DATA_PT_CUSTOM			= (100),

} NK_N1DataPayload;


/**
 * ֱ���Ự���������ݽṹ��\n
 * �ڵ��� @ref NkN1Utils_InitLiveSession ����ʱ��Դ����ݽṹ���г�ʼ����\n
 * �ڳ�ʼ���Ự��ʱ���Դ������Ľ��г�ʼ����\n
 *
 */
typedef struct Nk_N1LiveSession
{
	/**
	 * ʵʱý��ͨ�� ID���� 0 ��ʼ��
	 */
	NK_UInt32 channel_id;

	/**
	 * ʵʱý��ͨ�� ID ������ ID���� 0 ��ʼ��
	 */
	NK_UInt32 stream_id;

	/**
	 * �ỰΨһ ID����ģ���ڲ�������
	 */
	NK_UInt32 session_id;

	/**
	 * ����֡���м�������
	 * ����ݽӿڵ��ò��ϵ�����
	 */
	NK_UInt32 sequence;

	struct
	{
		NK_N1DataPayload payload_type; ///< ý�����ͣ��� @ref NK_N1_LIVE_SESS_PT_*��
		NK_Size width, heigth; ///< ��Ƶ��ߡ�

	} Video;

	struct
	{
		NK_N1DataPayload payload_type; ///< ý�����ͣ��� @ref NK_N1_LIVE_SESS_PT_*��
		NK_UInt32 sample_rate; ///< �����ʣ�ÿ����Ƶ����������������
		NK_UInt32 sample_bitwidth; ///< ����λ��ֵ������8��16��24��32��
		NK_Boolean stereo; ///< ������/��������ʶ��Ϊ False ������������֮Ϊ��������

	} Audio;

	/**
	 * �û��Ự�����\n
	 * �����߿���ͨ���˾��������ǰ�Ự����������Ϣ��\n
	 *
	 */
	NK_PVoid user_session;

	/**
	 * ģ���ڱ�����������
	 */
	NK_Byte reserved[1024 * 2];

} NK_N1LiveSession;



/**
 * N1 ����֡���ݽṹ�ṹ��\n
 * ���ǵ��ڻ���֡���ݵ�ʱ��
 *
 *
 */
typedef struct Nk_N1DataFrame
{
	NK_Size n_vectors; ///< ��Ч Vectors �ĸ�����

	struct
	{
		NK_PVoid raw; ///<
		NK_Size len;

	} Vectors[1024];

} NK_N1DataFrame;


typedef enum Nk_N1LanSetupClassify
{

	NK_N1_LAN_SETUP_UNDEF = (-1),

	/**
	 * �豸��Ϣ��
	 */
	NK_N1_LAN_SETUP_INFO,

	/**
	 * ������ʱ�����ã���Ӧ NK_N1Lansetup::Time��
	 */
	NK_N1_LAN_SETUP_TIME,

	/**
	 * �����������˹�Ƭ���ã���Ӧ NK_N1Lansetup::IRCutFilter��
	 */
	NK_N1_LAN_SETUP_IRCUT,

	/**
	 * ��������Ƶͼ�����ã���Ӧ NK_N1Lansetup::VideoImage��
	 */
	NK_N1_LAN_SETUP_VIMG,

	/**
	 * ��������Ƶ�������ã���Ӧ NK_N1Lansetup::VideoEncoder��
	 */
	NK_N1_LAN_SETUP_VENC,

	/**
	 * ��������̨�������ã���Ӧ NK_N1Lansetup::PanTiltZoom��
	 */
	NK_N1_LAN_SETUP_PTZ,


	NK_N1_LAN_SETUP_NET_WIRED,
	NK_N1_LAN_SETUP_NET_WIRED_NVR,
	NK_N1_LAN_SETUP_NET_WIFI,

	/**
	 * ���� NVR ���ã���Ӧ NK_N1Lansetup::WiFiNVR��
	 */
	NK_N1_LAN_SETUP_NET_WIFI_NVR,

	/**
	 * DNS ���á�
	 */
	NK_N1_LAN_SETUP_DNS,

} NK_N1LanSetupClassify;


/**
 * N1 Э�����������������ݽṹ��
 */
typedef struct Nk_N1LanSetup
{
	/**
	 * ���÷��ࡣ
	 */
	NK_N1LanSetupClassify classify;

	/**
	 * ͨ���ţ����漰����ͨ������ʱ�漰����
	 */
	NK_Int channel_id;

	/**
	 * �����ţ����漰��ĳ��ͨ���¶���������ʱ�漰����
	 */
	NK_Int stream_id;

	union {

		/**
		 * �豸��Ϣ��
		 * �� classify ���� NK_N1_LAN_SETUP_INFO ʱ��Ч��
		 */
		struct {

			/**
			 * �豸���� ID �š�
			 */
			NK_Char cloud_id[32];

			/**
			 * �豸�ͺŶ��塣
			 */
			NK_Char model[32];

			/**
			 * �豸�汾�š�
			 */
			NK_Char version[32];

			/**
			 * �豸ֱ��ͨ�������壬��СΪ 1�����Ϊ 256��
			 */
			NK_Size live_channels;

			/**
			 * ÿ��ֱ��ͨ�������ԣ���Ч������ @ref live_channels ���Ӧ��
			 */
			struct {
				/**
				 * ÿ��ֱ��ͨ��������������СΪ 1�����Ϊ 8��
				 */
				NK_Size stream_channels;

			} LiveChannels[128];

		} Info;

		/**
		 * �豸ʱ������������ݽṹ��
		 * �� classify ���� NK_N1_LAN_SETUP_TIME ʱ��Ч��
		 */
		struct 	{

			/**
			 * UTC ʱ�䣨����������Թ�Ԫ1970��1��1��00ʱ00��00��ʱ�������
			 */
			NK_UTC1970 utc;

			/**
			 * ʱ������ʱ����
			 */
			NK_TimeZone gmt;

			/**
			 * ����ʱʹ�ñ�ʶ��
			 */
			NK_Boolean dst;

		} Time;

		/**
		 * �����˹�Ƭ������ء�
		 */
		struct {

			/**
			 * ��� @ref NK_N1IRCutFilterMode��
			 */
			NK_N1PropEnum Mode;

			/**
			 * ����ģʽ����ҹģʽ����ת���ĳ����ж�ʱ�䣨��λ���룩��
			 */
			NK_N1PropInteger DayToNightFilterTime, NightToDayFilterTime;

		} IRCutFilter;

		/**
		 * ��Ƶͼ������������ݽṹ��
		 * �� classify ���� NK_N1_LAN_SETUP_VIMG ʱ��Ч��
		 */
		struct 	{

			/**
			 * ��Ƶͼ������Ƶ�ʡ�
			 */
			NK_N1PropInteger PowerLineFrequenceMode;

			/**
			 * ��Ƶͼ������ֱ��ʡ�
			 */
			NK_N1PropEnum CaptureResolution;

			/**
			 * ��Ƶͼ������֡�ʡ�
			 */
			NK_N1PropInteger CaptureFrameRate;

			/**
			 * ����ͼ��ɫ�ʵ��ڡ�
			 * ���豸���������� ISP ��������ء�
			 */
			NK_N1PropInteger BrightnessLevel, ContrastLevel, SharpnessLevel, SaturationLevel, HueLevel;

			/**
			 * ˮƽ����ֱ��ת���ñ�ʶ��
			 */
			NK_N1PropBoolean Flip, Mirror;

			/**
			 * ��Ƶͼ����⡣
			 */
			struct {

				/**
				 * ������ʾ��ʶ��
				 */
				NK_N1PropBoolean Show;

				/**
				 * �����ı����롣
				 */
				NK_N1PropEnum TextEncoding;

				/**
				 * �����ı���
				 */
				NK_N1PropString Text;

			} Title;

			/**
			 * ��Ƶͼ���˶���⡣
			 */
			struct {
				/**
				 * �����˶�����ʶ��
				 */
				NK_N1PropBoolean Enabled;

				/**
				 * ��������ȡ�
				 */
				NK_N1PropInteger SensitivityLevel;

				/**
				 * ����������롣
				 * ������Ƶ�˶��������������������⻹��Ҫ���ü�����룬
				 * ����������ȫ��Ϊ True ��ʱ���ʾ������Ƶ�˶���⣬
				 * ������Ϊ���������������⣬�������������Ϊ 32x24������豸�˶��������ʵ�ֵĿ������ڴ�ֵ����Ҫ�������䡣
				 *
				 */
				struct {
					/**
					 * С�ڵ��� 32x24��
					 */
					NK_Size width, height;
					NK_Byte matrix[24][32];
				} Mask;

			} MotionDetection;

		} VideoImage;

		/**
		 * ��Ƶ��������������ݽṹ��
		 * �� classify ���� NK_N1_LAN_SETUP_VENC ʱ��Ч��
		 * ���� channel_id �� stream_id ����Ч��
		 */
		struct {

			/**
			 * ���������ƣ����ݴ�ֵ���־������ݽṹ��
			 */
			NK_N1PropEnum Codec;

			struct {

				/**
				 * ����ֱ��ʡ�
				 */
				NK_N1PropEnum Resolution;

				/**
				 * ���ʿ���ģʽ��
				 */
				NK_N1PropEnum BitRateCtrlMode;

				/**
				 * �������ʣ���λ��kbps��ǧλÿ�룩��
				 */
				NK_N1PropInteger BitRate;

				/**
				 * ����֡�ʣ���λ��fps��֡ÿ�룩��
				 */
				NK_N1PropInteger FrameRate;

				/**
				 * �ؼ�֡�������λ��frames��֡����
				 */
				NK_N1PropInteger KeyFrameInterval;

			} H264;

		} VideoEncoder;

		/**
		 * ��̨��������������ݽṹ��
		 * �� classify ���� NK_N1_LAN_SETUP_PTZ ʱ��Ч��
		 */
		struct {

			NK_N1PTZCommand command;

			/**
			 * ����ִ�б�ʶ������ NK_N1_LAN_SETUP_PTZ_CMD_TILT_* �� NK_N1_LAN_SETUP_PTZ_CMD_PAN_* ���\n
			 * ����ִ�к��豸ִ�е���ִ�к��Զ�ֹͣ��\n
			 * �ǵ���ִ��ʱ����Ҫ�ͻ����ٴη����� NK_N1_LAN_SETUP_PTZ_CMD_STOP ����ֹͣ��
			 *
			 */
			NK_Boolean step;

			/**
			 * Ԥ��λ�ţ��� command Ϊ NK_N1_LAN_SETUP_PTZ_CMD_*_PRESET ʱָʾ�����Ķ�ӦԤ��λ��
			 */
			NK_Integer preset_position;

			/**
			 * ��̨�˶��ٶȡ�
			 */
			NK_N1PropInteger Speed;

		} PanTiltZoom;


		/**
		 * �������� NVR ���á�\n
		 * �������� NVR �������������豸�������Ӷ�Ӧ������������
		 */
		struct {

			/**
			 *
			 * +-------------+------------+------------+------------+------------+------------+
			 * |             | ESSID      | PSK        | EnableDHCP | HwAddr     | { IPv4 }   |
			 * +-------------+------------+------------+------------+------------+------------+
			 * | NetWired    | n          | n          | y          | y          | y          |
			 * +-------------+------------+------------+------------+------------+------------+
			 * | NetWiredNVR | n          | n          | y          | y          | y          |
			 * +-------------+------------+------------+------------+------------+------------+
			 * | NetWiFi     | y          | y          | y          | y          | y          |
			 * +-------------+------------+------------+------------+------------+------------+
			 * | NetWiFiNVR  | y          | y          | False      | y          | y          |
			 * +-------------+------------+------------+------------+------------+------------+
			 *
			 */

			/**
			 * ���������ȵ� / NVR ����Ӧ�� ESSID��
			 */
			NK_N1PropString ESSID;

			/**
			 * �� classify Ϊ NK_N1_LAN_SETUP_WIFIAP ʱ��\n
			 * ��ʾ�ȵ�Ľ������룻\n
			 * �� classify Ϊ NK_N1_LAN_SETUP_WIFISTA ���� NK_N1_LAN_SETUP_WIFINVR ʱ��\n
			 * ��ʾ���������ȵ� / NVR ����Ӧ�� ESSID �Ľ������롣
			 */
			NK_N1PropString PSK;

			/**
			 * �� classify Ϊ NK_N1_LAN_SETUP_WIFIAP ʱ��\n
			 * ��ʾ�����Ƿ��� DHCP ����\n
			 * �� classify Ϊ NK_N1_LAN_SETUP_WIFISTA ʱ��\n
			 * ��ʾ�Ƿ�ʹ�������ȵ�� DHCP �����ȡ��ַ��\n
			 * �� classify Ϊ NK_N1_LAN_SETUP_WIFINVR ʱ��\n
			 * ��ֵһֱΪ False��
			 */
			NK_N1PropBoolean EnableDHCP;

			/**
			 * ����������ַ��
			 */
			NK_N1PropHwAddr HwAddr;

			/**
			 * IP ��ַ���á�
			 */
			NK_N1PropIPv4 IPAddress, Netmask, Gateway, DomainNameServer;

		} NetWired, NetWiredNVR, NetWiFi, NetWiFiNVR;

		/**
		 * DNS ��ַ���á�
		 */
		struct {

			/**
			 * ��ѡ�� DNS ��ַ��
			 */
			NK_N1PropIPv4 Preferred;

			/**
			 * ���õ� DNS ��ַ��
			 */
			NK_N1PropIPv4 Alternative;

		} DNS;

	};

} NK_N1LanSetup;


/**
 * �K�˴�ӡ NK_N1LanSetup ���ݽṹ�����ڵ��ԡ�
 *
 */
static inline NK_Void
NK_N1_LAN_SETUP_DUMP(NK_N1LanSetup *__LanSetup)
{
	NK_Int i = 0, ii = 0;
	NK_TermTable Table;
	NK_CHECK_POINT();
	NK_TermTbl_BeginDraw(&Table, "Lan Setup Data Field", 80, 4);
	if (NK_N1_LAN_SETUP_INFO == (__LanSetup)->classify) {
		NK_TermTbl_PutText(&Table, NK_True, "%-48s", "Device Infomation");
	} else if (NK_N1_LAN_SETUP_TIME == (__LanSetup)->classify) {
		NK_TermTbl_PutText(&Table, NK_True, "%-48s", "Time");
		NK_TermTbl_PutKeyValue(&Table, NK_False, "UTC", "%u", (__LanSetup)->Time.utc);
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Time Zone", "%d", (__LanSetup)->Time.gmt);
		NK_TermTbl_PutKeyValue(&Table, NK_True, "DST", "%s", (__LanSetup)->Time.dst ? "Enabled" : "Disabled");
	} else if (NK_N1_LAN_SETUP_IRCUT == (__LanSetup)->classify) {
		NK_TermTbl_PutText(&Table, NK_True, "%-48s", "IRCut Filter");
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Mode", "%s",
				NK_ENUM_MAP(N1IRCutFilterMode, (__LanSetup)->IRCutFilter.Mode.val));
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Day to Night Time", "%d [%d, %d]",
				(__LanSetup)->IRCutFilter.DayToNightFilterTime.val,
				(__LanSetup)->IRCutFilter.DayToNightFilterTime.min,
				(__LanSetup)->IRCutFilter.DayToNightFilterTime.max);
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Night to Day Time", "%d [%d, %d]",
				(__LanSetup)->IRCutFilter.NightToDayFilterTime.val,
				(__LanSetup)->IRCutFilter.NightToDayFilterTime.min,
				(__LanSetup)->IRCutFilter.NightToDayFilterTime.max);
	} else if (NK_N1_LAN_SETUP_VIMG == (__LanSetup)->classify) {
		NK_TermTbl_PutText(&Table, NK_True, "%-48s", "Video Image");
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Channel", "%d", (__LanSetup)->channel_id);
		NK_TermTbl_PutText(&Table, NK_True, "%-48s", "Title");
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Show", "%s", (__LanSetup)->VideoImage.Title.Show.val ? "Yes" : "No");
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Text", "%s", (__LanSetup)->VideoImage.Title.Text.val);
		NK_TermTbl_PutText(&Table, NK_True, "%-48s", "Color");
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Hue Level", "%d [%d, %d]",
			(__LanSetup)->VideoImage.HueLevel.val, (__LanSetup)->VideoImage.HueLevel.min, (__LanSetup)->VideoImage.HueLevel.max);
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Sharpness Level", "%d [%d, %d]",
			(__LanSetup)->VideoImage.SharpnessLevel.val, (__LanSetup)->VideoImage.SharpnessLevel.min, (__LanSetup)->VideoImage.SharpnessLevel.max);
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Contrast Level", "%d [%d, %d]",
			(__LanSetup)->VideoImage.ContrastLevel.val, (__LanSetup)->VideoImage.ContrastLevel.min, (__LanSetup)->VideoImage.ContrastLevel.max);
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Brightness Level", "%d [%d, %d]",
			(__LanSetup)->VideoImage.BrightnessLevel.val, (__LanSetup)->VideoImage.BrightnessLevel.min, (__LanSetup)->VideoImage.BrightnessLevel.max);
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Saturation Level", "%d [%d, %d]",
			(__LanSetup)->VideoImage.SaturationLevel.val, (__LanSetup)->VideoImage.SaturationLevel.min, (__LanSetup)->VideoImage.SaturationLevel.max);
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Flip / Mirror", "%s / %s",
			(__LanSetup)->VideoImage.Flip.val ? "Yes" : "No", (__LanSetup)->VideoImage.Mirror.val ? "Yes" : "No");
		NK_TermTbl_PutText(&Table, NK_True, "%-48s", "Motion Detection");
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Enabled", "%s", (__LanSetup)->VideoImage.MotionDetection.Enabled.val ? "Yes" : "No");
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Sensitivity Level", "%d [%d, %d]",
			(__LanSetup)->VideoImage.MotionDetection.SensitivityLevel.val,
			(__LanSetup)->VideoImage.MotionDetection.SensitivityLevel.min,
			(__LanSetup)->VideoImage.MotionDetection.SensitivityLevel.max);
		NK_TermTbl_PutText(&Table, NK_False, "%s (%u x %u)", "Mask",
				(__LanSetup)->VideoImage.MotionDetection.Mask.width, (__LanSetup)->VideoImage.MotionDetection.Mask.height);
		for (i = 0; i < (__LanSetup)->VideoImage.MotionDetection.Mask.height; ++i) {
			NK_Char mask[80];
			NK_BZERO(mask, sizeof(mask));
			for (ii = 0; ii < (__LanSetup)->VideoImage.MotionDetection.Mask.width; ++ii) {
				mask[ii * 2] = mask[ii * 2 + 1] = (__LanSetup)->VideoImage.MotionDetection.Mask.matrix[i][ii] ? 'O' : '.';
			}
			NK_TermTbl_PutText(&Table, NK_False, "%s", mask);
		}

	} else if (NK_N1_LAN_SETUP_VENC == (__LanSetup)->classify) {
		NK_TermTbl_PutText(&Table, NK_True, "%-48s", "Video Encoder");
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Channel", "%d", (__LanSetup)->channel_id);
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Stream", "%d", (__LanSetup)->stream_id);
		if (NK_N1_VENC_CODEC_H264 == (__LanSetup)->VideoEncoder.Codec.val) {
			NK_TermTbl_PutKeyValue(&Table, NK_False, "Resolution", "%d", (__LanSetup)->VideoEncoder.H264.Resolution.val);
			NK_TermTbl_PutKeyValue(&Table, NK_False, "Bit Rate", "%d", (__LanSetup)->VideoEncoder.H264.BitRate.val);
			NK_TermTbl_PutKeyValue(&Table, NK_False, "Frame Rate", "%d", (__LanSetup)->VideoEncoder.H264.FrameRate.val);
			NK_TermTbl_PutKeyValue(&Table, NK_False, "Key Frame Interval", "%d", (__LanSetup)->VideoEncoder.H264.KeyFrameInterval.val);
			NK_TermTbl_PutKeyValue(&Table, NK_False, "Bit Rate Control Mode", "%s", NK_ENUM_MAP(N1BitRateCtrlMode, (__LanSetup)->VideoEncoder.H264.BitRateCtrlMode.val));
		}
	} else if (NK_N1_LAN_SETUP_PTZ == (__LanSetup)->classify) {
		NK_TermTbl_PutText(&Table, NK_True, "%-48s", "Pan Tilt Zoom");
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Channel", "%d", (__LanSetup)->channel_id);
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Command", NK_ENUM_MAP(N1PTZCommand, (__LanSetup)->PanTiltZoom.command));
		if (NK_N1_PTZ_CMD_SET_PRESET == (__LanSetup)->PanTiltZoom.command
				|| NK_N1_PTZ_CMD_GOTO_PRESET == (__LanSetup)->PanTiltZoom.command
				|| NK_N1_PTZ_CMD_CLEAR_PRESET == (__LanSetup)->PanTiltZoom.command) {
			NK_TermTbl_PutKeyValue(&Table, NK_False, "Preset Position", "%d", (__LanSetup)->PanTiltZoom.preset_position);
		}
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Speed", "%d [%d, %d]", (__LanSetup)->PanTiltZoom.Speed.val,
				(__LanSetup)->PanTiltZoom.Speed.min, (__LanSetup)->PanTiltZoom.Speed.max);
	} else if (NK_N1_LAN_SETUP_NET_WIRED == (__LanSetup)->classify
			|| NK_N1_LAN_SETUP_NET_WIRED_NVR == (__LanSetup)->classify
			|| NK_N1_LAN_SETUP_NET_WIFI == (__LanSetup)->classify
			|| NK_N1_LAN_SETUP_NET_WIFI_NVR == (__LanSetup)->classify) {
		NK_Char text[64];
		if (NK_N1_LAN_SETUP_NET_WIFI == (__LanSetup)->classify
				|| NK_N1_LAN_SETUP_NET_WIFI_NVR == (__LanSetup)->classify) {
			NK_TermTbl_PutText(&Table, NK_True, "%-48s", "WiFi Net");
			NK_TermTbl_PutKeyValue(&Table, NK_False, "ESSID", "%s", (__LanSetup)->NetWiFi.ESSID.val);
			NK_TermTbl_PutKeyValue(&Table, NK_False, "PSK", "%s", (__LanSetup)->NetWiFi.PSK.val);
		} else {
			NK_TermTbl_PutText(&Table, NK_True, "%-48s", "Wired Net");
		}
		NK_N1_PROP_HWADDR_STR(&(__LanSetup)->NetWiFi.HwAddr, text, sizeof(text));
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Hardware Address", "%s", text);
		NK_TermTbl_PutKeyValue(&Table, NK_False, "DHCP", "%s", (__LanSetup)->NetWiFi.EnableDHCP.val ? "Enabled" : "Disabled");
		NK_N1_PROP_IPV4_NTOA(&(__LanSetup)->NetWiFi.IPAddress, text, sizeof(text));
		NK_TermTbl_PutKeyValue(&Table, NK_False, "IP Address", "%s", text);
		NK_N1_PROP_IPV4_NTOA(&(__LanSetup)->NetWiFi.Netmask, text, sizeof(text));
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Netmask", "%s", text);
		NK_N1_PROP_IPV4_NTOA(&(__LanSetup)->NetWiFi.Gateway, text, sizeof(text));
		NK_TermTbl_PutKeyValue(&Table, NK_False, "Gateway", "%s", text);
		NK_N1_PROP_IPV4_NTOA(&(__LanSetup)->NetWiFi.DomainNameServer, text, sizeof(text));
		NK_TermTbl_PutKeyValue(&Table, NK_False, "DNS", "%s", text);
	}
	NK_TermTbl_EndDraw(&Table);
}

/**
 * N1 ֪ͨ���͡�
 *
 */
typedef enum Nk_N1NotificationType
{
	NK_N1_NOTF_UNDEF,

	/**
	 * �˶����֪ͨ��
	 */
	NK_N1_NOTF_MOTION_DETECTION,

	/**
	 * ����������֪ͨ��
	 */
	NK_N1_NOTF_PIR_DETECTION,


} NK_N1NotificationType;

/**
 * N1 ֪ͨ���ݽṹ��
 */
typedef struct Nk_N1Notification
{
	NK_N1NotificationType type;

	/**
	 * ��Ӧͨ���š�
	 */
	NK_Int channel_id;

	union {

		/**
		 * �˶���⡣\n
		 * �� @ref NK_N1Notification::type Ϊ NK_N1_NOTF_MOTION_DETECTION ��Ч��
		 */
		struct {

		} MotionDetection;

		/**
		 * ����������⡣\n
		 * �� @ref NK_N1Notification::type Ϊ NK_N1_NOTF_PIR_DETECTION ��Ч��
		 */
		struct {

		} PIR;

	};

} NK_N1Notification;


/**
 * �����ȵ����ݽṹ���塣
 */
typedef struct Nk_WiFiHotSpot
{
	/**
	 * �����ȵ�� BSSID��
	 */
	NK_Char bssid[32];
	/**
	 * �����ȵ�ͨ���ŵ���0 ��ʾ�Զ���
	 */
	NK_Int channel;
	/**
	 * �����ȵ��ź�ǿ�ȡ�
	 */
	NK_Int dBm, sdBm;
	/**
	 * �����ȵ������ڡ�
	 */
	NK_Int age;

	/**
	 * �����ȵ�� ESSID��
	 */
	NK_Char essid[128];

	/**
	 * �����ȵ�� PSK��
	 */
	NK_Char psk[32];

} NK_WiFiHotSpot;

/**
 * ��ӡ NK_WiFiHotSpot ���ݽṹ��
 */
#define NK_N1_WIFI_HOTSPOT_DUMP(__HotSpot) \
	do{\
		NK_TermTable Table;\
		NK_TermTbl_BeginDraw(&Table, "N1 Hot Spot", 64, 4);\
		NK_TermTbl_PutKeyValue(&Table, NK_True, "BSSID", "%s", (__HotSpot)->bssid);\
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Channel", "%d", (__HotSpot)->channel);\
		NK_TermTbl_PutKeyValue(&Table, NK_True, "dBm / SdBm", "%d / %d", (__HotSpot)->dBm, (__HotSpot)->sdBm);\
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Age", "%d", (__HotSpot)->age);\
		NK_TermTbl_PutKeyValue(&Table, NK_True, "ESSID", "%s", (__HotSpot)->essid);\
		NK_TermTbl_PutKeyValue(&Table, NK_True, "PSK", "%s", (__HotSpot)->psk);\
		NK_TermTbl_EndDraw(&Table);\
	} while(0)


NK_CPP_EXTERN_END
#endif /* NK_UTILS_N1_DEF_H_ */
