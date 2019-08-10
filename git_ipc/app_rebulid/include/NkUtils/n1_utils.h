/*
 *  N1 Э�����������㡣
 *
 */

#include <NkUtils/n1_def.h>

#ifndef NK_UTILS_N1_UTILS_H_
#define NK_UTILS_N1_UTILS_H_
NK_CPP_EXTERN_BEGIN

/**
 * ��ӡ N1 Э��ͼ�ꡣ
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
 * N1 �����豸Э�鶨����鲥��ַ��
 */
#define NK_N1_DISC_MULT_ADDR "239.255.255.250"


/**
 * N1 �����豸Э�鶨����鲥�˿ڡ�
 */
#define NK_N1_DISC_MULT_PORT (8002)

/**
 * @ref NK_DeviceInfo ���ݽṹ���漰�����ַ�����󳤶ȡ�
 */
#define NK_N1_PARAM_TEXT_SZ (128)

/**
 * �������ݰ�������ͨ����
 */
typedef enum Nk_N1DiscoveryTunnel {

	NK_N1_DISC_TUNN_UNDEF = (-1),

	NK_N1_DISC_TUNN_WIRED,

	NK_N1_DISC_TUNN_WIRED_NVR,

	NK_N1_DISC_TUNN_WIFI,

	NK_N1_DISC_TUNN_WIFI_NVR,

} NK_N1DiscoveryTunnel;

/**
 * �豸��Ϣ���ݽṹ��
 */
typedef struct Nk_N1DeviceInfo
{
	/**
	 * �豸�� ID �š�
	 */
	NK_Char id[NK_N1_PARAM_TEXT_SZ];

	/**
	 * �豸���� ID �š�
	 */
	NK_Char cloud_id[NK_N1_PARAM_TEXT_SZ];

	/**
	 * �豸�ͺŶ��塣
	 */
	NK_Char model[NK_N1_PARAM_TEXT_SZ];

	/**
	 * �豸���ƶ��塣
	 */
	NK_Char name[NK_N1_PARAM_TEXT_SZ];

	/**
	 * �豸�汾�š�
	 */
	NK_Char version[NK_N1_PARAM_TEXT_SZ];


	/**
	 * �豸ֱ��ͨ�������壬��СΪ 1�����Ϊ 128��
	 */
	NK_Size live_channels;

	/**
	 * ÿ��ֱ��ͨ�������ԣ���Ч������ @ref live_channels ���Ӧ��
	 */
	struct {
		/**
		 * ÿ��ֱ��ͨ��������������СΪ 1�����Ϊ 8��
		 */
		NK_Size streams;
	} LiveChannels[128];

	/**
	 * ��̫��������Ϣ���ϡ�
	 */
	struct {

		NK_Char mac_address[NK_N1_PARAM_TEXT_SZ]; ///< ���������ַ��
		NK_Boolean dhcp_enabled;
		NK_Char ip_address[NK_N1_PARAM_TEXT_SZ];
		NK_Char netmask[NK_N1_PARAM_TEXT_SZ];
		NK_Char gateway[NK_N1_PARAM_TEXT_SZ];
		NK_Char preferred_dns[NK_N1_PARAM_TEXT_SZ];
		NK_Char alternative_dns[NK_N1_PARAM_TEXT_SZ];

		/**
		 * �� NK_N1DeviceInfo::Ethernet::is_wifi Ϊ True ʱ���ݽṹ����Ч��\n
		 * �������� Wi-Fi ���ӵ������Ϣ��
		 */
		struct {

			NK_Char essid[64];
			NK_Char psk[32];

		} WiFi;

	} Ethernet;

	/**
	 * HTTP ������Ϣ���ϡ�
	 */
	struct {
		NK_UInt16 listen_port;
	} HTTP;

} NK_N1DeviceInfo;


/**
 * N1 ����������������ݽṹ��
 */
typedef struct Nk_N1DiscoverySetup
{
	/**
	 * ������Ӧ���豸 ID �š�
	 */
	NK_Char device_id[32];

	struct {

		NK_Boolean dhcp_enabled;
		NK_Char ip_address[NK_N1_PARAM_TEXT_SZ];
		NK_Char netmask[NK_N1_PARAM_TEXT_SZ];
		NK_Char gateway[NK_N1_PARAM_TEXT_SZ];
		NK_Char mac_address[NK_N1_PARAM_TEXT_SZ];

		/**
		 * ��ʹ�� WiFi ʱ�ṹ��ָ�� WiFi ��Ϊ Nil��
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
 * �ն˴�ӡ NK_N1DiscoverySetup ���ݽṹ��
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
 * ���� SDP ֡��\n
 * �� Live �Ự�б����ȷ��ʹ�֡���ͻ��˲�����Ч����ý�����ݡ�\n
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
 * @param[in,out]		DataFrame		����
 * @return
 */
extern int
NK_N1Utils_MakeDataFrame(NK_N1LiveSession *Session, NK_N1DataPayload payload_type, NK_UInt32 ts_ms, NK_N1DataFrame *DataFrame);


/**
 * �����豸�Ự��
 *
 */
typedef struct Nk_HisnetDiscovery
{
	/**
	 * �豸�� ID �š�\n
	 * �ڷ���Э�������ڱ�ʶ�豸��Ψһ�ԡ�
	 *
	 */
	NK_Char device_id[64];

	/**
	 * �����¼�����
	 */
	struct {

		/**
		 * �û������ġ�\n
		 */
		NK_PVoid user_ctx;

		/**
		 * ����У���¼���\n
		 * ���������ݰ��к�����Ҫ����У����Ϣʱ�ᴥ���˽ӿڽ���У�飬\n
		 * ���û�û��ʵ�ִ˽ӿڣ���Ĭ�ϻ�ͨ��У�顣\n
		 *
		 * @param[in]		user			�����û�����
		 * @param[in]		password		�����û����롣
		 * @param[in,out]	ctx				�û������ģ��� @ref NK_N1DiscoverySession ���ݽṹ�д��롣
		 *
		 * @return		У��ɹ����� True��ʧ�ܷ��� False��
		 */
		NK_Boolean
		(*onAuthorize)(NK_PChar user, NK_PChar password, NK_PVoid ctx);

		/**
		 * ��������ȡ�豸��Ϣ�¼���\n
		 *
		 * @param[in,out]	ctx				�û������ģ��� @ref NK_N1DiscoverySession ���ݽṹ�д��롣
		 * @param[in]		tunnel			�������͡�
		 * @param[out]		DeviceInfo		�豸��Ϣ���ݽṹ��
		 *
		 * @return		�û�ʵ�ַ��� 0 ʱģ��Ż��ȡ @ref DeviceInfo ��Ϣ���ɱ��ġ�
		 */
		NK_N1Error
		(*onGetInfo)(NK_PVoid ctx, NK_N1DiscoveryTunnel tunnel, NK_N1DeviceInfo *DeviceInfo);

		/**
		 * �����������¼���\n
		 * ���� @ref NK_N1Utils_HandleDiscovery ����ʱ����������Ǿ������������ݰ�ʱ�ᴥ�����¼���\n
		 * �û����óɹ��󷵻� 0�����򷵻ش����롣
		 *
		 * @param[in,out]	ctx				�û������ģ��� @ref NK_N1DiscoverySession ���ݽṹ�д��롣
		 * @param[in]		tunnel			�������͡�
		 * @param[in]		Setup			�����������������ݽṹ���û��������ݽṹ���������豸��
		 *
		 * @return		�û�ʵ�ַ��ض�Ӧ�Ĵ����룬ģ�����ݷ������������ɱ��ġ�
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
 * ���� N1 �����豸Э������ݰ���\n
 * ��ʼ��Э��Ự����󣬲��Ͻ����׽������ݣ�Ȼ����˽ӿ���������ݲ�����\n
 * �ӿ��ڲ�����ݴ����������ݴ��������Ự�¼����и����豸��ز�����\n
 * �����ɹ��󣬽ӿڻ��� @ref stack ������������������ݺͷ������ݳ��ȣ�\n
 * �ϲ��û���ȡ�����ݰ�����ͨ���׽��ַ��ͳ�ȥ��
 *
 * @param[in]		Session						Э��Ự�����
 * @param[in]		packet						Э�����ݰ���
 * @param[out]		response_packet				�ظ��������ݻ��塣
 * @param[in,out]	response_len				����ظ���������ջ���ȣ����ػظ��������ݳ��ȡ�
 *
 * @verbatim
 * ����:
 *
 * discovery_example() {
 *
 *   /// 1 - ����ͨѶ�׽��֡�
 *   socket_init();
 *
 *   /// 2 - ѭ�����������ģ������ظ����ġ�
 *   for(;;) {
 *     /// 2.1 - ���ձ��ġ�
 *     socket_recv(NK_N1_DISC_MULT_ADDR, NK_N1_DISC_MULT_PORT);
 *     /// 2.2 - �������Ĳ���������豸�¼���
 *     NK_N1Utils_HandleDiscoveryPacket();
 *     /// 2.3 - �ظ����ġ�
 *     socket_send(NK_N1_DISC_MULT_ADDR, NK_N1_DISC_MULT_PORT);
 *   }
 *
 *   /// 3 - ����ͨѶ�׽��֡�
 *   socket_destroy();
 * }
 *
 * @endverbatim
 *
 * @return		�����ɹ����� 0������ @ref response_len �л�ȡ�ظ����Ĵ�С���������󷵻� -1��
 */
extern NK_Int
NK_Hisnet_HandleDiscoveryRequest(NK_HisnetDiscovery *Session, NK_PChar packet,
		NK_PChar response_packet, NK_Size *response_len);



/**
 * ͨ�� ESSID ����Ӧ�ȵ��Ƿ�Ϊ NVR��
 */
extern NK_Boolean
NK_N1Utils_IsHotSpotOfNVR(NK_PChar essid);



/**
 * ͨ�� NVR �����ȵ� ESSID ��ȡ�ȵ����롣\n
 * �ڲ������ @ref NK_N1Utils_IsHotSpotOfNVR() ������
 *
 * @param[in]		essid						NVR �����ȵ� ESSID ���ơ�
 * @param[out]		psk							NVR �����ȵ����롣
 * @param[in,out]	psk_size					��������ջ����С�����������ȵ����볤�ȡ�
 *
 * @return		��ȡ�ȵ�����ɹ����� 0��ʧ�ܷ��� -1��
 */
extern NK_Int
NK_N1Utils_CrackWiFiNVRPSK(NK_PChar essid, NK_PChar psk, NK_Size *psk_size);

/**
 * ���� @ref LanSetup ���ݽṹ���ݡ�
 *
 */
extern NK_Int
NK_N1Utils_CorrectLanSetup(NK_N1LanSetup *LanSetup);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_N1_UTILS_H_ */
