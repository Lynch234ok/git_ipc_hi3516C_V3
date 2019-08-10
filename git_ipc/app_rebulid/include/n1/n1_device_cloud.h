
/**
 * @brief
 *  N1 对接空中配对相关事件及接口定义。
 * @details
 *  用户通过此模块实现对空中配对的相关事件响应，实现对应功能，\n
 *  模块通过接口 @ref NK_N1Device_AirPair() 加载事件，\n
 *  该接口必须在 @ref NK_N1Device_Init() 调用成功后才能调用，否则失败。
 *
 */

#include "n1_device.h"

#ifndef NK_N1_DEVICE_CLOUD_H_
#define NK_N1_DEVICE_CLOUD_H_
NK_CPP_EXTERN_BEGIN


/**
 * @brief
 *  相关响应事件定义。
 */
typedef struct Nk_N1DeviceEventCloud {

	NK_Int reserved;

} NK_N1DeviceEventCloud;

/**
 * @brief
 *  配置相关扩展事件。
 * @details
 *
 * @param Event [in]
 *  用户事件定义。
 *
 * @return
 *  配置成功返回 0，否则返回 -1。
 */
NK_API NK_Int
NK_N1Device_Cloud(NK_N1DeviceEventCloud *Event);

/**
 * @brief
 *  云录像总开关。
 *
 * @param enabled [in]
 *  开启、关闭标识符。
 *
 */
NK_API NK_Void
NK_N1Device_CloudEnableRecord(NK_Boolean enabled);

/**
 * @brief
 *  开始云录制。
 * @details
 *
 *
 *
 * @param enabled [in]
 *  开启 / 关闭 Wi-Fi NVR 配对标识。
 *
 * @return
 *  成功返回 0，失败返回 -1。
 */
NK_API NK_Int
NK_N1Device_CloudStartRecord(NK_Int chid, NK_Int streamid, const NK_PChar tag);

/**
 * @brief
 *  停止云录制。
 *
 * @param chid [in] 通道 ID，从 0 开始
 *
 * @param streamid [in] 码流 ID，从 0 开始
 *
 *
 */
NK_API NK_Void
NK_N1Device_CloudStopRecord(NK_Int chid, NK_Int streamid);


NK_CPP_EXTERN_END
#endif /* NK_N1_DEVICE_CLOUD_H_ */
