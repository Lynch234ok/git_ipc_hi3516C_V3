
/**
 * @brief
 *  N1 对接 VR 摄像机相关事件及接口定义。
 * @details
 *  用户通过此模块实现对固件升级的相关事件响应，实现对应功能，\n
 *  模块通过接口 @ref NK_N1Device_Upgrade() 加载事件，\n
 *  该接口必须在 @ref NK_N1Device_Init() 调用成功后才能调用，否则失败。
 *
 */

#include "n1_device.h"

#ifndef NK_N1_DEVICE_VRCAM_H_
#define NK_N1_DEVICE_VRCAM_H_
NK_CPP_EXTERN_BEGIN


/**
 * @brief
 *  设备鱼眼校正。
 */
typedef struct Nk_N1FishEyeCalibration {

	/// 圆心校准。
	struct {
		NK_Float x; ///< 与画幅宽度相对。
		NK_Float y; ///< 与画幅高度相对。
	} Origin;

	/// 多目摄像头安装的角度。
	struct {
		NK_Int x, y, z;
	} Angle;

	/// 圆半径，与画幅宽度相对。
	NK_Float radius;

} NK_N1FishEyeCalibration;


/**
 * @brief
 *  摄像头安装方式定义。
 */
typedef enum Nk_N1CameraInstalling {

	NK_N1_CAM_INST_NA = (0),
	NK_N1_CAM_INST_CEIL,
	NK_N1_CAM_INST_WALL,
	NK_N1_CAM_INST_DESKTOP,

} NK_N1CameraInstalling;


/**
 * @brief
 *  相关响应事件定义。
 */
typedef struct Nk_N1DeviceEventVRCam {

	/**
	 * @brief
	 *  获取鱼眼矫正参数。
	 * @details
	 *
	 * @param chid [in]
	 *  媒体通道序号，从 0 开始。
	 * @param lensid [in]
	 *  镜头序号，同一个媒体序号可能对应多个镜头，从 0 开始。
	 * @param Calibration [out]
	 *  矫正参数。
	 *
	 * @return
	 *  获取成功返回 True，该镜头不存在返回 False。
	 */
	NK_Boolean
	(*onGetFishEyeCalibration)(NK_PVoid ctx, NK_Int chid, NK_Int lensid, NK_N1FishEyeCalibration *Calibration);

	/**
	 * @brief
	 *  更新鱼眼矫正参数。
	 * @details
	 *  详见 @ref onGetFishEyeCalibration。
	 *
	 */
	NK_Void
	(*onSetFishEyeCalibration)(NK_PVoid ctx, NK_Int chid, NK_Int lensid, NK_N1FishEyeCalibration *Calibration);

	/**
	 * @brief
	 *  获取摄像头安装方式。
	 *
	 * @param ctx [in,out]
	 *  用户上下文，在调用接口 @ref NK_N1Device_Init 时传入。
	 * @param chid [in]
	 *  媒体序号，从 0 开始。
	 *
	 * @return
	 *  返回对应媒体通道的摄像头的安装方式。
	 */
	NK_N1CameraInstalling
	(*onGetCameraInstalling)(NK_PVoid ctx, NK_Int chid);

	/**
	 * @brief
	 *  设置摄像头安装方式。
	 * @details
	 *  详见 @ref onGetCameraInstalling。
	 *
	 */
	NK_Void
	(*onSetCameraInstalling)(NK_PVoid ctx, NK_Int chid, NK_N1CameraInstalling inst);


} NK_N1DeviceEventVRCam;

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
NK_N1Device_VRCam(NK_N1DeviceEventVRCam *Event);



NK_CPP_EXTERN_END
#endif /* NK_N1_DEVICE_VRCAM_H_ */
