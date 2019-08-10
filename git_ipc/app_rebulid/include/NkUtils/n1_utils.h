/*
 *  N1 协议数处理抽象层。
 *
 */

#include <NkUtils/n1_def.h>

#ifndef NK_UTILS_N1_UTILS_H_
#define NK_UTILS_N1_UTILS_H_
NK_CPP_EXTERN_BEGIN

/**
 * 打印 N1 协议图标。
 *
 *           nnnn
 *          nnnn                    nn     nn nnnn    11
 *         nnnn  nnnnnnn          nnn      nnn   nn  111
 *        nnnn nnnnnnnnnnn      nnnn       nn    nn   11
 *     nnnnnnnn       nnnn    nnnnn        nn    nn   11
 *   nnnnnnn         nnnn      nnn         nn    nn  1111
 *     nnnn         nnnn      nnn
 *    nnnn         nnnn      nnn
 *   nnnn         nnnn      nnn
 *  nnnn         nnnnnnnnnnnnn
 *
 */
#define NK_N1_LOGO_PATTERN(__ver_maj, __ver_min, __ver_rev, __rel_yyyy, __rel_mm, __rel_dd) \
	do{\
		printf("\r\n");\
		printf("\033[40;30m");\
		printf("                                                                               \r\n");\
		printf("            \033[41;31mnnnn\033[40;30mn                                                           \r\n");\
		printf("           \033[41;31mnnnn\033[40;30mn                  \033[41;31mnn\033[40;30mn    \033[47;37mnn\033[40;30mn\033[47;37mnnnn\033[40;30mn   \033[47;37m11\033[40;30m1                      \r\n");\
		printf("          \033[41;31mnnnn\033[40;30mn \033[41;31mnnnnnn\033[40;30mn         \033[41;31mnnn\033[40;30mn     \033[47;37mnnn\033[40;30mn  \033[47;37mnn\033[40;30mn \033[47;37m111\033[40;30m1                      \r\n");\
		printf("         \033[41;31mnnnn\033[40;30mn\033[41;31mnnnnnnnnnn\033[40;30mn     \033[41;31mnnnn\033[40;30mn      \033[47;37mnn\033[40;30mn   \033[47;37mnn\033[40;30mn  \033[47;37m11\033[40;30m1                      \r\n");\
		printf("      \033[41;31mnnnnnnnn\033[40;30mn     \033[41;31mnnnn\033[40;30mn   \033[41;31mnnnnn\033[40;30mn       \033[47;37mnn\033[40;30mn   \033[47;37mnn\033[40;30mn  \033[47;37m11\033[40;30m1                      \r\n");\
		printf("    \033[41;31mnnnnnnn\033[40;30mn       \033[41;31mnnnn\033[40;30mn     \033[41;31mnnn\033[40;30mn        \033[47;37mnn\033[40;30mn   \033[47;37mnn\033[40;30mn \033[47;37m1111\033[40;30m1                     \r\n");\
		printf("      \033[41;31mnnnn\033[40;30mn       \033[41;31mnnnn\033[40;30mn     \033[41;31mnnn\033[40;30mn                                            \r\n");\
		printf("     \033[41;31mnnnn\033[40;30mn       \033[41;31mnnnn\033[40;30mn     \033[41;31mnnn\033[40;30mn          \033[40;37mVersion \033[4m%d.%d.%d\033[24m                    \r\n", __ver_maj, __ver_min, __ver_rev);\
		printf("    \033[41;31mnnnn\033[40;30mn       \033[41;31mnnnn\033[40;30mn     \033[41;31mnnn\033[40;30mn           \033[40;37mDate \033[4m%04u/%02u/%02u\033[24m                    \r\n", __rel_yyyy, __rel_mm, __rel_dd);\
		printf("   \033[41;31mnnnn\033[40;30mn       \033[41;31mnnnnnnnnnnnnn\033[40;30mn                                               \r\n");\
		printf("                                                                                           \r\n");\
		printf("\033[0m");\
		printf("\r\n");\
	} while(0)

/**
 * N1 发现设备协议定义的组播地址。
 */
#define NK_N1_DISC_MULT_ADDR "239.255.255.250"


/**
 * N1 发现设备协议定义的组播端口。
 */
#define NK_N1_DISC_MULT_PORT (8002)

/**
 * @ref NK_DeviceInfo 数据结构中涉及到的字符串最大长度。
 */
#define NK_N1_PARAM_TEXT_SZ (128)

/**
 * 搜索数据包的所在通道。
 */
typedef enum Nk_N1DiscoveryTunnel {

	NK_N1_DISC_TUNN_UNDEF = (-1),

	NK_N1_DISC_TUNN_WIRED,

	NK_N1_DISC_TUNN_WIRED_NVR,

	NK_N1_DISC_TUNN_WIFI,

	NK_N1_DISC_TUNN_WIFI_NVR,

} NK_N1DiscoveryTunnel;

/**
 * 设备信息数据结构。
 */
typedef struct Nk_N1DeviceInfo
{
	/**
	 * 设备的 ID 号。
	 */
	NK_Char id[NK_N1_PARAM_TEXT_SZ];

	/**
	 * 设备的云 ID 号。
	 */
	NK_Char cloud_id[NK_N1_PARAM_TEXT_SZ];

	/**
	 * 设备型号定义。
	 */
	NK_Char model[NK_N1_PARAM_TEXT_SZ];

	/**
	 * 设备名称定义。
	 */
	NK_Char name[NK_N1_PARAM_TEXT_SZ];

	/**
	 * 设备版本号。
	 */
	NK_Char version[NK_N1_PARAM_TEXT_SZ];


	/**
	 * 设备直播通道数定义，最小为 1，最大为 128。
	 */
	NK_Size live_channels;

	/**
	 * 每个直播通道的属性，有效数量与 @ref live_channels 相对应。
	 */
	struct {
		/**
		 * 每个直播通道的码流数，最小为 1，最大为 8。
		 */
		NK_Size streams;
	} LiveChannels[128];

	/**
	 * 以太网属性信息集合。
	 */
	struct {

		NK_Char mac_address[NK_N1_PARAM_TEXT_SZ]; ///< 网卡物理地址。
		NK_Boolean dhcp_enabled;
		NK_Char ip_address[NK_N1_PARAM_TEXT_SZ];
		NK_Char netmask[NK_N1_PARAM_TEXT_SZ];
		NK_Char gateway[NK_N1_PARAM_TEXT_SZ];
		NK_Char preferred_dns[NK_N1_PARAM_TEXT_SZ];
		NK_Char alternative_dns[NK_N1_PARAM_TEXT_SZ];

		/**
		 * 当 NK_N1DeviceInfo::Ethernet::is_wifi 为 True 时数据结构体有效。\n
		 * 用于描述 Wi-Fi 连接的相关信息。
		 */
		struct {

			NK_Char essid[64];
			NK_Char psk[32];

		} WiFi;

	} Ethernet;

	/**
	 * HTTP 配置信息集合。
	 */
	struct {
		NK_UInt16 listen_port;
	} HTTP;

} NK_N1DeviceInfo;


/**
 * N1 局域网设置相关数据结构。
 */
typedef struct Nk_N1DiscoverySetup
{
	/**
	 * 命令响应的设备 ID 号。
	 */
	NK_Char device_id[32];

	struct {

		NK_Boolean dhcp_enabled;
		NK_Char ip_address[NK_N1_PARAM_TEXT_SZ];
		NK_Char netmask[NK_N1_PARAM_TEXT_SZ];
		NK_Char gateway[NK_N1_PARAM_TEXT_SZ];
		NK_Char mac_address[NK_N1_PARAM_TEXT_SZ];

		/**
		 * 当使用 WiFi 时结构体指针 WiFi 不为 Nil。
		 */
		struct {

			enum {

				NK_N1_SETUP_WIFI_SECURITY_NONE = (0),
				NK_N1_SETUP_WIFI_SECURITY_WEP,
				NK_N1_SETUP_WIFI_SECURITY_WPA,
				NK_N1_SETUP_WIFI_SECURITY_WPA2,

			} security;

			NK_Char essid[64];
			NK_Char psk[32];

		} _WiFi, *WiFi;

	} _NetConf, *NetConf;

	struct {

		NK_UInt16 listen_port;

	} _HTTP, *HTTP;

} NK_N1DiscoverySetup;

/**
 * 终端打印 NK_N1DiscoverySetup 数据结构。
 *
 */
#define NK_N1_DICOVERY_SETUP_DUMP(__LanSetup) \
	do{\
		NK_TermTable Table;\
		NK_TermTbl_BeginDraw(&Table, "N1 Discovery Setup Package", 64, 4);\
		if ((__LanSetup)->NetConf) {\
			NK_TermTbl_PutText(&Table, NK_True, "Net Configuration");\
			NK_TermTbl_PutKeyValue(&Table, NK_False, "DHCP", (__LanSetup)->NetConf->dhcp_enabled ? "Enable" : "Disable");\
			NK_TermTbl_PutKeyValue(&Table, NK_False, "IP Address", (__LanSetup)->NetConf->ip_address);\
			NK_TermTbl_PutKeyValue(&Table, NK_False, "Netmask", (__LanSetup)->NetConf->netmask);\
			NK_TermTbl_PutKeyValue(&Table, NK_True, "Gateway", (__LanSetup)->NetConf->gateway);\
			if ((__LanSetup)->NetConf->WiFi) {\
				NK_TermTbl_PutText(&Table, NK_True, "Wi-Fi");\
				NK_TermTbl_PutKeyValue(&Table, NK_True, "ESSID", (__LanSetup)->NetConf->WiFi->essid);\
				NK_TermTbl_PutKeyValue(&Table, NK_True, "PSK", (__LanSetup)->NetConf->WiFi->psk);\
			}\
		}\
		if ((__LanSetup)->HTTP) {\
			NK_TermTbl_PutText(&Table, NK_True, "HTTP Configuration");\
			NK_TermTbl_PutKeyValue(&Table, NK_True, "Listen Port", "%d", (NK_Int)((__LanSetup)->HTTP->listen_port));\
		}\
		NK_TermTbl_EndDraw(&Table);\
	} while(0)



/**
 * 生成 SDP 帧。\n
 * 在 Live 会话中必须先发送此帧，客户端才能有效解析媒体数据。\n
 *
 * @param Session
 * @return
 */
extern int
NK_N1Utils_MakeSDPFrame(NK_N1LiveSession *Session,
		NK_PByte sdp, NK_Size stack_len);

/**
 *
 *
 * @param Session
 * @param ts_ms
 * @param[in,out]		DataFrame		数据
 * @return
 */
extern int
NK_N1Utils_MakeDataFrame(NK_N1LiveSession *Session, NK_N1DataPayload payload_type, NK_UInt32 ts_ms, NK_N1DataFrame *DataFrame);


/**
 * 发现设备会话。
 *
 */
typedef struct Nk_HisnetDiscovery
{
	/**
	 * 设备的 ID 号。\n
	 * 在发现协议中用于标识设备的唯一性。
	 *
	 */
	NK_Char device_id[64];

	/**
	 * 触发事件集。
	 */
	struct {

		/**
		 * 用户上下文。\n
		 */
		NK_PVoid user_ctx;

		/**
		 * 操作校验事件。\n
		 * 当传入数据包中含有需要操作校验信息时会触发此接口进行校验，\n
		 * 若用户没有实现此接口，则默认会通过校验。\n
		 *
		 * @param[in]		user			操作用户名。
		 * @param[in]		password		操作用户密码。
		 * @param[in,out]	ctx				用户上下文，在 @ref NK_N1DiscoverySession 数据结构中传入。
		 *
		 * @return		校验成功返回 True，失败返回 False。
		 */
		NK_Boolean
		(*onAuthorize)(NK_PChar user, NK_PChar password, NK_PVoid ctx);

		/**
		 * 局域网获取设备信息事件。\n
		 *
		 * @param[in,out]	ctx				用户上下文，在 @ref NK_N1DiscoverySession 数据结构中传入。
		 * @param[in]		tunnel			搜索类型。
		 * @param[out]		DeviceInfo		设备信息数据结构。
		 *
		 * @return		用户实现返回 0 时模块才会读取 @ref DeviceInfo 信息生成报文。
		 */
		NK_N1Error
		(*onGetInfo)(NK_PVoid ctx, NK_N1DiscoveryTunnel tunnel, NK_N1DeviceInfo *DeviceInfo);

		/**
		 * 局域网配置事件。\n
		 * 调用 @ref NK_N1Utils_HandleDiscovery 方法时，当传入的是局域网设置数据包时会触发此事件。\n
		 * 用户配置成功后返回 0，否则返回错误码。
		 *
		 * @param[in,out]	ctx				用户上下文，在 @ref NK_N1DiscoverySession 数据结构中传入。
		 * @param[in]		tunnel			配置类型。
		 * @param[in]		Setup			局域网配置命令数据结构，用户根据数据结构内容配置设备。
		 *
		 * @return		用户实现返回对应的错误码，模块会根据返回码内容生成报文。
		 */
		NK_N1Error
		(*onSetup)(NK_PVoid ctx, NK_N1DiscoveryTunnel tunnel, NK_N1DiscoverySetup *Setup);

	} EventSet;

} NK_HisnetDiscovery;



/**
 *
 * @verbatim
 *
 * Data Flow:
 *
 *                   +-------------+
 *                   |             |
 *                   |    Client   |
 *                   |             |
 *                   +-------------+
 *     <Client IP>:8002 ||      /\ <Client IP>:8002
 *                      ||     /||\
 *            Discovery ||      ||
 *              Request ||      || Discovery
 *                     \||/     || Response
 *                      \/      ||
 *   +-------------------------------------------------+
 *   +                                                 +
 *   +                     LAN                         +
 *   +                                                 +
 *   +-------------------------------------------------+
 *   +                                                 +
 *   +        UDP (Broadcasting / Multicasting)        +
 *   +                                                 +
 *   +-------------------------------------------------+
 *       || 239.255.255.250:8002                /\ 239.255.255.250:8002
 *       ||                                    /||\
 *       || Receive                             || Send
 *       || Plain Packet                        || Plain Packet
 *       ||                                     ||
 *   +---||-------------------------------------||----+
 *   |   ||                                     ||    |
 *   |   ||       NK_N1Utils_HandleDiscovery    ||    |
 *   |  \||/                                    ||    |
 *   |   \/                                     ||    |
 *   |  +---------+     +----------+     +---------+  |
 *   |  |  Parse  |====>|  Active  |====>|  Make   |  |
 *   |  |  Plain  |     |  Event   |     |  Plain  |  |
 *   |  +---------+     +----------+     +---------+  |
 *   |                       /\                       |
 *   |                      /||\                      |
 *   +-----------------------||-----------------------+
 *                          \||/
 *                           \/ <Device IP>:8002
 *                     +-------------+
 *                     |   Device    |
 *                     |   Control   |
 *                     +-------------+
 *
 * @endverbatim
 *
 * 处理 N1 发现设备协议的数据包，\n
 * 初始化协议会话句柄后，不断接收套接字数据，然后传入此接口做相关数据操作，\n
 * 接口内部会根据传入数据内容触发各个会话事件进行各样设备相关操作。\n
 * 操作成功后，接口会在 @ref stack 缓冲填充操作结果的数据和返回数据长度，\n
 * 上层用户获取到数据包后再通过套接字发送出去。
 *
 * @param[in]		Session						协议会话句柄。
 * @param[in]		packet						协议数据包。
 * @param[out]		response_packet				回复报文数据缓冲。
 * @param[in,out]	response_len				传入回复报文数据栈长度，返回回复报文数据长度。
 *
 * @verbatim
 * 用例:
 *
 * discovery_example() {
 *
 *   /// 1 - 创建通讯套接字。
 *   socket_init();
 *
 *   /// 2 - 循环接收请求报文，处理及回复报文。
 *   for(;;) {
 *     /// 2.1 - 接收报文。
 *     socket_recv(NK_N1_DISC_MULT_ADDR, NK_N1_DISC_MULT_PORT);
 *     /// 2.2 - 解析报文并触发相关设备事件。
 *     NK_N1Utils_HandleDiscoveryPacket();
 *     /// 2.3 - 回复报文。
 *     socket_send(NK_N1_DISC_MULT_ADDR, NK_N1_DISC_MULT_PORT);
 *   }
 *
 *   /// 3 - 销毁通讯套接字。
 *   socket_destroy();
 * }
 *
 * @endverbatim
 *
 * @return		操作成功返回 0，并从 @ref response_len 中获取回复报文大小，操作错误返回 -1。
 */
extern NK_Int
NK_Hisnet_HandleDiscoveryRequest(NK_HisnetDiscovery *Session, NK_PChar packet,
		NK_PChar response_packet, NK_Size *response_len);



/**
 * 通过 ESSID 检测对应热点是否为 NVR。
 */
extern NK_Boolean
NK_N1Utils_IsHotSpotOfNVR(NK_PChar essid);



/**
 * 通过 NVR 无线热点 ESSID 获取热点密码。\n
 * 内部会调用 @ref NK_N1Utils_IsHotSpotOfNVR() 方法。
 *
 * @param[in]		essid						NVR 无线热点 ESSID 名称。
 * @param[out]		psk							NVR 无线热点密码。
 * @param[in,out]	psk_size					传入密码栈区大小，传出无线热点密码长度。
 *
 * @return		获取热点密码成功返回 0，失败返回 -1。
 */
extern NK_Int
NK_N1Utils_CrackWiFiNVRPSK(NK_PChar essid, NK_PChar psk, NK_Size *psk_size);

/**
 * 更正 @ref LanSetup 数据结构内容。
 *
 */
extern NK_Int
NK_N1Utils_CorrectLanSetup(NK_N1LanSetup *LanSetup);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_N1_UTILS_H_ */
