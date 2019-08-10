
/**
 * @brief
 *  N1 对接设备双向语音相关 API 接口定义。
 * @details
 *
 */

#include "n1_device.h"

#ifndef NK_N1_DEVICE_TWOWAYTALK_H_
#define NK_N1_DEVICE_TWOWAYTALK_H_
NK_CPP_EXTERN_BEGIN


/**
 * @brief
 *  N1 对接设备双向语音相关事件定义。
 */
typedef struct Nk_N1DeviceEventTwoWayTalk {

	/**
	 * @brief
	 *  对讲接入拨号事件。
	 * @details
	 *
	 * @param ctx [in,out]
	 *  用户事件上下文，在方法 @ref NK_N1Device_Init() 调用时传入。
	 * @param chid [in]
	 *  对讲连接对应多媒体的通道号，按逻辑顺序从 0 开始递增。
	 *
	 * @return
	 *  如果当前对讲功能被占线返回 False，允许对讲返回 True。
	 *
	 */
	NK_Boolean
	(*onCalled)(NK_PVoid ctx, NK_Int chid);

	/**
	 * @brief
	 *  对讲接收数据事件。
	 * @details
	 *
	 * @param ctx [in,out]
	 *  用户事件上下文，在方法 @ref NK_N1Device_Init() 调用时传入。
	 * @param chid [in]
	 *  对讲连接对应多媒体的通道号，按逻辑顺序从 0 开始递增。
	 * @param Package
	 *  数据包属性。
	 * @param data [in]
	 *  接收到的数据块起始地址。
	 * @param len [in]
	 *  接收到的数据长度。
	 *
	 */
	NK_Void
	(*onRecv)(NK_PVoid ctx, NK_Int chid, NK_N1MediaPackage *Package, NK_PByte data, NK_Size len);

	/**
	 * @brief
	 *  对讲挂起事件。
	 *
	 * @param ctx [in,out]
	 *  用户事件上下文，在方法 @ref NK_N1Device_Init() 调用时传入。
	 * @param chid [in]
	 *  对讲连接对应多媒体的通道号，按逻辑顺序从 0 开始递增。
	 *
	 */
	NK_Void
	(*onHungUp)(NK_PVoid ctx, NK_Int chid);


} NK_N1DeviceEventTwoWayTalk;

/**
 * @brief
 *  配置 N1 设备生产相关事件。
 * @details
 *
 * @param Event [in]
 *  用户事件定义。
 *
 * @return
 *  配置成功返回 0，否则返回 -1。
 */
NK_API NK_Int
NK_N1Device_TwoWayTalk(NK_N1DeviceEventTwoWayTalk *Event);



NK_CPP_EXTERN_END
#endif /* NK_N1_DEVICE_TWOWAYTALK_H_ */
