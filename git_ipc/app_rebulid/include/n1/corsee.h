


#include <NkUtils/types.h>

#if !defined(NK_EMBD_CORSEE_H_)
#define NK_EMBD_CORSEE_H_
NK_CPP_EXTERN_BEGIN

/**
 * 版本号定义。
 */
#define NK_CORSEE_VERSION "0.4"

/**
 * 设置数据结构。
 */
typedef struct Nk_CorseeSetup {

	struct {
		NK_Char access_token[128];
		NK_Char stream_name[64];
	} _BaiduMediaServer, *BaiduMediaServer;

	struct {

		struct {
			NK_Char ssid[128];
			NK_Char psk[128];
		} _WiFi, *WiFi;

		/**
		 * DHCP 使能。
		 */
		NK_Boolean dhcp;

		/**
		 * 当 dhcp 为 NK_False 下列数据结构有效。
		 */
		struct {
			NK_Char ipaddr[32], netmask[32], gateway[32], dns[32];
		} _IPAddr, *IPAddr;

	} _Ether, *Ether;

} NK_CorseeSetup;

/**
 * 热点定义。
 */
typedef struct Nk_CorseeWiFiAP {

	/// 热点名称。
	NK_Char ssid[64];

	/// -100 - 0
	NK_Int rssi;

	///bssid
	NK_Char bssid[20];

	///加密方式
	NK_Char encrytype[48];

} NK_CorseeWiFiAP;


/**
 * CORSEE 发现设备协议协议集。
 */
typedef struct Nk_CorseeDiscoveryEventSet {

	/**
	 * 获取设备 ID 事件。
	 */
	NK_Int
	(*onGetDeviceID)(NK_PVoid ctx, NK_PChar device_id, NK_Size *len);

	/**
	 * 获取设备 UID 事件。
	 */
	NK_Int
	(*onGetDeviceUID)(NK_PVoid ctx, NK_PChar uid, NK_Size *len);

	/**
	 * 获取设备 devkey 事件。
	 */
	NK_Int
	(*onGetDeviceKey)(NK_PVoid ctx, NK_PChar devkey, NK_Size *len);

	/**
	 * 设置独占模式接口2
	 */
	NK_Int
	(*onSetMonopolyMode2)(NK_PVoid ctx, NK_PChar monopolyinfo, NK_Size *monopolylen);

	/**
	 * 检测设备 是否为独占模式设备 事件。
	 */
	NK_Boolean
	(*onCheckMonopoly)(NK_PVoid ctx);

	/**
	 * 获取设备描述信息接口。
	 */
	NK_Int
	(*onGetDeviceDescribeInfo)(NK_PVoid ctx, NK_PChar devinfo, NK_Size *len);

	/**
	 * 设置独占模式接口(Binding)。
	 * monopolyinfo : json格式字符串内容 独占模式设置报文内容 [in]
	 * monopolylen : json格式字符串内容	独占模式设置报文长度 [in]
	 * contentinfo : json格式字符串内容 返回报文内容 [in out]
	 * contentinfolen : json格式字符串内容 返回报文长度 [in out]
	 * 返回0 成功 ; 返回-1 失败
	 */
	NK_Int
	(*onSetMonopolyMode)(NK_PVoid ctx, NK_PChar monopolyinfo, NK_Size *monopolylen, NK_PChar contentinfo, NK_Size *contentinfolen);

	/**
	 * 接收数据包事件。
	 */
	NK_Int
	(*onRecvPacket)(NK_PVoid ctx, NK_PChar fromIP, NK_UInt16 fromPort, NK_PChar packet);

	/**
	 * 设置事件。
	 */
	NK_Int
	(*onSetup)(NK_PVoid ctx, NK_CorseeSetup *Setup);

	/**
	 * @brief
	 *  获取设备附近无线 AP 列表。
	 *
	 * @param ctx [in,out]
	 *  事件上下文。
	 * @param APs [out]
	 *  AP 列表信息。
	 * @param nAPs [in,out]
	 *  AP 列表个数，传入栈最大值，返回周边 AP 的个数，如果没有这里设为 0。
	 *
	 */
	NK_Void
	(*onGetNearbyAPs)(NK_PVoid ctx, NK_CorseeWiFiAP *APs, NK_Size *nAPs);


} NK_CorseeDiscoveryEventSet;

/**
 * 启动/关闭发现设备监听器。
 *
 */
extern NK_Int
NK_Corsee_DiscoveryListener(NK_Boolean startup, NK_CorseeDiscoveryEventSet *EventSet, NK_PVoid event_ctx);

/**
 * 更新标签。
 *
 */
extern NK_Int
NK_Corsee_SetToken(NK_PChar token);

/**
 * 设置设备类型。
 *
 */
extern NK_Int
NK_Corsee_SetDeviceType(NK_PChar type);

NK_CPP_EXTERN_END
#endif /* NK_EMBD_CORSEE_H_ */











