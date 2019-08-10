/*
 * N1 协议数据结构定义。
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
	 * 未定义的错误。
	 */
	NK_N1_ERR_UNDEF					= (-1),

	/**
	 * 没有错误，操作成功。
	 */
	NK_N1_ERR_NONE					= (0),

	/**
	 * 参数错误相关。
	 */
	NK_N1_ERR_INVALID_PARAM			= (101),	///< 参数错误。
	NK_N1_ERR_INVALID_CHANNEL_STREAM_ID,		///< 通道/码流号 ID 错误。

	/**
	 * 设备内部错误相关。
	 */
	NK_N1_ERR_DEVICE_BUSY			= (201),	///<设备忙碌，有可能该功能正在被占用。
	NK_N1_ERR_DEVICE_NOT_SUPPORT,				///< 设备不支持。

	/**
	 * 操作设备错误相关。
	 */
	NK_N1_ERR_INVALID_OPERATE		= (301),	///< 无效的设备操作。
	NK_N1_ERR_OPERATE_TIMEOUT,					///< 设备操作超时。

	/**
	 * 传输数据包错误。
	 */
	NK_N1_ERR_INVALID_DATAGRAM		= (401),	///< 无效的传输报文。


	/**
	 * 用户校验失败。
	 */
	NK_N1_ERR_NOT_AUTHORIZATED		= (1001),

} NK_N1Error;

/**
 * 兼容版本 < 1.4.0
 */
#define NK_N1Ret NK_N1Error

/**
 * N1 直播数据载体的类型。
 */
typedef enum Nk_N1DataPayload
{
	NK_N1_DATA_PT_UNDEF				= (-1),
	/**
	 * G.711 A-Law 音频静荷数据。
	 */
	NK_N1_DATA_PT_G711A				= (8),
	/**
	 * H.264 Nal-Unit 视频静荷数据。
	 */
	NK_N1_DATA_PT_H264_NALUS		= (96),
	/**
	 * H.265 Nal-Unit 视频静荷数据。
	 */
	NK_N1_DATA_PT_H265_NALUS		= (97),

	NK_N1_DATA_PT_CUSTOM			= (100),

} NK_N1DataPayload;


/**
 * 直播会话上下文数据结构，\n
 * 在调用 @ref NkN1Utils_InitLiveSession 方法时会对此数据结构进行初始化，\n
 * 在初始化会话的时候会对此上下文进行初始化。\n
 *
 */
typedef struct Nk_N1LiveSession
{
	/**
	 * 实时媒体通道 ID，从 0 开始。
	 */
	NK_UInt32 channel_id;

	/**
	 * 实时媒体通道 ID 下码流 ID，从 0 开始。
	 */
	NK_UInt32 stream_id;

	/**
	 * 会话唯一 ID，由模块内部产生。
	 */
	NK_UInt32 session_id;

	/**
	 * 数据帧序列计数器。
	 * 会根据接口调用不断递增。
	 */
	NK_UInt32 sequence;

	struct
	{
		NK_N1DataPayload payload_type; ///< 媒体类型，见 @ref NK_N1_LIVE_SESS_PT_*。
		NK_Size width, heigth; ///< 视频宽高。

	} Video;

	struct
	{
		NK_N1DataPayload payload_type; ///< 媒体类型，见 @ref NK_N1_LIVE_SESS_PT_*。
		NK_UInt32 sample_rate; ///< 采样率，每秒音频采样的脉冲数量。
		NK_UInt32 sample_bitwidth; ///< 采样位宽，值可以是8、16、24、32。
		NK_Boolean stereo; ///< 单声道/立体生标识，为 False 代表单声道，反之为立体声。

	} Audio;

	/**
	 * 用户会话句柄。\n
	 * 调用者可以通过此句柄保留当前会话的上下文信息。\n
	 *
	 */
	NK_PVoid user_session;

	/**
	 * 模块内保留数据区。
	 */
	NK_Byte reserved[1024 * 2];

} NK_N1LiveSession;



/**
 * N1 数据帧数据结构结构。\n
 * 考虑到在缓存帧数据的时候。
 *
 *
 */
typedef struct Nk_N1DataFrame
{
	NK_Size n_vectors; ///< 有效 Vectors 的个数。

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
	 * 设备信息。
	 */
	NK_N1_LAN_SETUP_INFO,

	/**
	 * 局域网时间配置，对应 NK_N1Lansetup::Time。
	 */
	NK_N1_LAN_SETUP_TIME,

	/**
	 * 局域网红外滤光片配置，对应 NK_N1Lansetup::IRCutFilter。
	 */
	NK_N1_LAN_SETUP_IRCUT,

	/**
	 * 局域网视频图像配置，对应 NK_N1Lansetup::VideoImage。
	 */
	NK_N1_LAN_SETUP_VIMG,

	/**
	 * 局域网视频编码配置，对应 NK_N1Lansetup::VideoEncoder。
	 */
	NK_N1_LAN_SETUP_VENC,

	/**
	 * 局域网云台控制配置，对应 NK_N1Lansetup::PanTiltZoom。
	 */
	NK_N1_LAN_SETUP_PTZ,


	NK_N1_LAN_SETUP_NET_WIRED,
	NK_N1_LAN_SETUP_NET_WIRED_NVR,
	NK_N1_LAN_SETUP_NET_WIFI,

	/**
	 * 无线 NVR 配置，对应 NK_N1Lansetup::WiFiNVR。
	 */
	NK_N1_LAN_SETUP_NET_WIFI_NVR,

	/**
	 * DNS 配置。
	 */
	NK_N1_LAN_SETUP_DNS,

} NK_N1LanSetupClassify;


/**
 * N1 协议局域网配置相关数据结构。
 */
typedef struct Nk_N1LanSetup
{
	/**
	 * 配置分类。
	 */
	NK_N1LanSetupClassify classify;

	/**
	 * 通道号，在涉及到多通道配置时涉及到。
	 */
	NK_Int channel_id;

	/**
	 * 码流号，在涉及到某个通道下多码流配置时涉及到。
	 */
	NK_Int stream_id;

	union {

		/**
		 * 设备信息。
		 * 当 classify 等于 NK_N1_LAN_SETUP_INFO 时有效。
		 */
		struct {

			/**
			 * 设备的云 ID 号。
			 */
			NK_Char cloud_id[32];

			/**
			 * 设备型号定义。
			 */
			NK_Char model[32];

			/**
			 * 设备版本号。
			 */
			NK_Char version[32];

			/**
			 * 设备直播通道数定义，最小为 1，最大为 256。
			 */
			NK_Size live_channels;

			/**
			 * 每个直播通道的属性，有效数量与 @ref live_channels 相对应。
			 */
			struct {
				/**
				 * 每个直播通道的码流数，最小为 1，最大为 8。
				 */
				NK_Size stream_channels;

			} LiveChannels[128];

		} Info;

		/**
		 * 设备时间配置相关数据结构。
		 * 当 classify 等于 NK_N1_LAN_SETUP_TIME 时有效。
		 */
		struct 	{

			/**
			 * UTC 时间（格林尼治相对公元1970年1月1日00时00分00秒时间戳）。
			 */
			NK_UTC1970 utc;

			/**
			 * 时间所在时区。
			 */
			NK_TimeZone gmt;

			/**
			 * 夏令时使用标识。
			 */
			NK_Boolean dst;

		} Time;

		/**
		 * 红外滤光片配置相关。
		 */
		struct {

			/**
			 * 详见 @ref NK_N1IRCutFilterMode。
			 */
			NK_N1PropEnum Mode;

			/**
			 * 白天模式到黑夜模式互相转换的持续判断时间（单位：秒）。
			 */
			NK_N1PropInteger DayToNightFilterTime, NightToDayFilterTime;

		} IRCutFilter;

		/**
		 * 视频图像配置相关数据结构。
		 * 当 classify 等于 NK_N1_LAN_SETUP_VIMG 时有效。
		 */
		struct 	{

			/**
			 * 视频图像输入频率。
			 */
			NK_N1PropInteger PowerLineFrequenceMode;

			/**
			 * 视频图像输入分辨率。
			 */
			NK_N1PropEnum CaptureResolution;

			/**
			 * 视频图像输入帧率。
			 */
			NK_N1PropInteger CaptureFrameRate;

			/**
			 * 输入图像色彩调节。
			 * 与设备传感器或者 ISP 处理器相关。
			 */
			NK_N1PropInteger BrightnessLevel, ContrastLevel, SharpnessLevel, SaturationLevel, HueLevel;

			/**
			 * 水平、垂直反转设置标识。
			 */
			NK_N1PropBoolean Flip, Mirror;

			/**
			 * 视频图像标题。
			 */
			struct {

				/**
				 * 标题显示标识。
				 */
				NK_N1PropBoolean Show;

				/**
				 * 标题文本编码。
				 */
				NK_N1PropEnum TextEncoding;

				/**
				 * 标题文本。
				 */
				NK_N1PropString Text;

			} Title;

			/**
			 * 视频图像运动侦测。
			 */
			struct {
				/**
				 * 启动运动侦测标识。
				 */
				NK_N1PropBoolean Enabled;

				/**
				 * 检测灵敏度。
				 */
				NK_N1PropInteger SensitivityLevel;

				/**
				 * 检测区域掩码。
				 * 配置视频运动侦测除了配置灵敏度以外还须要配置检测掩码，
				 * 当掩码活动区域全部为 True 的时候表示整个视频运动检测，
				 * 否则则为掩码区域活动的区域检测，检测掩码最大颗粒为 32x24，如果设备运动这测区域实现的颗粒大于此值则须要进行适配。
				 *
				 */
				struct {
					/**
					 * 小于等于 32x24。
					 */
					NK_Size width, height;
					NK_Byte matrix[24][32];
				} Mask;

			} MotionDetection;

		} VideoImage;

		/**
		 * 视频编码配置相关数据结构。
		 * 当 classify 等于 NK_N1_LAN_SETUP_VENC 时有效。
		 * 属性 channel_id 和 stream_id 均有效。
		 */
		struct {

			/**
			 * 编码器名称，根据此值区分具体数据结构。
			 */
			NK_N1PropEnum Codec;

			struct {

				/**
				 * 编码分辨率。
				 */
				NK_N1PropEnum Resolution;

				/**
				 * 码率控制模式。
				 */
				NK_N1PropEnum BitRateCtrlMode;

				/**
				 * 编码码率（单位：kbps，千位每秒）。
				 */
				NK_N1PropInteger BitRate;

				/**
				 * 编码帧率（单位：fps，帧每秒）。
				 */
				NK_N1PropInteger FrameRate;

				/**
				 * 关键帧间隔（单位：frames，帧）。
				 */
				NK_N1PropInteger KeyFrameInterval;

			} H264;

		} VideoEncoder;

		/**
		 * 云台控制配置相关数据结构。
		 * 当 classify 等于 NK_N1_LAN_SETUP_PTZ 时有效。
		 */
		struct {

			NK_N1PTZCommand command;

			/**
			 * 单步执行标识，对于 NK_N1_LAN_SETUP_PTZ_CMD_TILT_* 或 NK_N1_LAN_SETUP_PTZ_CMD_PAN_* 命令，\n
			 * 单步执行后，设备执行单步执行后自动停止，\n
			 * 非单步执行时，须要客户端再次发命令 NK_N1_LAN_SETUP_PTZ_CMD_STOP 才能停止。
			 *
			 */
			NK_Boolean step;

			/**
			 * 预置位号，当 command 为 NK_N1_LAN_SETUP_PTZ_CMD_*_PRESET 时指示操作的对应预置位。
			 */
			NK_Integer preset_position;

			/**
			 * 云台运动速度。
			 */
			NK_N1PropInteger Speed;

		} PanTiltZoom;


		/**
		 * 连接无线 NVR 配置。\n
		 * 根据无线 NVR 发起请求配置设备无线连接对应的无线网卡。
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
			 * 连接无线热点 / NVR 所对应的 ESSID。
			 */
			NK_N1PropString ESSID;

			/**
			 * 当 classify 为 NK_N1_LAN_SETUP_WIFIAP 时，\n
			 * 表示热点的接入密码；\n
			 * 当 classify 为 NK_N1_LAN_SETUP_WIFISTA 或者 NK_N1_LAN_SETUP_WIFINVR 时，\n
			 * 表示连接无线热点 / NVR 所对应的 ESSID 的接入密码。
			 */
			NK_N1PropString PSK;

			/**
			 * 当 classify 为 NK_N1_LAN_SETUP_WIFIAP 时，\n
			 * 表示本地是否开启 DHCP 服务；\n
			 * 当 classify 为 NK_N1_LAN_SETUP_WIFISTA 时，\n
			 * 表示是否使用无线热点的 DHCP 服务获取地址；\n
			 * 当 classify 为 NK_N1_LAN_SETUP_WIFINVR 时，\n
			 * 此值一直为 False。
			 */
			NK_N1PropBoolean EnableDHCP;

			/**
			 * 物理网卡地址。
			 */
			NK_N1PropHwAddr HwAddr;

			/**
			 * IP 地址配置。
			 */
			NK_N1PropIPv4 IPAddress, Netmask, Gateway, DomainNameServer;

		} NetWired, NetWiredNVR, NetWiFi, NetWiFiNVR;

		/**
		 * DNS 地址配置。
		 */
		struct {

			/**
			 * 首选的 DNS 地址。
			 */
			NK_N1PropIPv4 Preferred;

			/**
			 * 备用的 DNS 地址。
			 */
			NK_N1PropIPv4 Alternative;

		} DNS;

	};

} NK_N1LanSetup;


/**
 * 終端打印 NK_N1LanSetup 数据结构，用于调试。
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
 * N1 通知类型。
 *
 */
typedef enum Nk_N1NotificationType
{
	NK_N1_NOTF_UNDEF,

	/**
	 * 运动侦测通知。
	 */
	NK_N1_NOTF_MOTION_DETECTION,

	/**
	 * 被动红外检测通知。
	 */
	NK_N1_NOTF_PIR_DETECTION,


} NK_N1NotificationType;

/**
 * N1 通知数据结构。
 */
typedef struct Nk_N1Notification
{
	NK_N1NotificationType type;

	/**
	 * 对应通道号。
	 */
	NK_Int channel_id;

	union {

		/**
		 * 运动侦测。\n
		 * 当 @ref NK_N1Notification::type 为 NK_N1_NOTF_MOTION_DETECTION 有效。
		 */
		struct {

		} MotionDetection;

		/**
		 * 被动红外侦测。\n
		 * 当 @ref NK_N1Notification::type 为 NK_N1_NOTF_PIR_DETECTION 有效。
		 */
		struct {

		} PIR;

	};

} NK_N1Notification;


/**
 * 无线热点数据结构定义。
 */
typedef struct Nk_WiFiHotSpot
{
	/**
	 * 无线热点的 BSSID。
	 */
	NK_Char bssid[32];
	/**
	 * 无线热点通信信道，0 表示自动。
	 */
	NK_Int channel;
	/**
	 * 无线热点信号强度。
	 */
	NK_Int dBm, sdBm;
	/**
	 * 无线热点生存期。
	 */
	NK_Int age;

	/**
	 * 无线热点的 ESSID。
	 */
	NK_Char essid[128];

	/**
	 * 无线热点的 PSK。
	 */
	NK_Char psk[32];

} NK_WiFiHotSpot;

/**
 * 打印 NK_WiFiHotSpot 数据结构。
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
