

#include <time.h>

#ifndef _SDK_INTERFACE_H_
#define _SDK_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif


#if defined(_WIN32)
# include <Windows.h>
#else
# define BYTE unsigned char
# define DWORD unsigned int
# define BOOL  unsigned int
# define UINT64 unsigned long long
#endif


typedef struct sNkEther
{

	BOOL isDHCP;
	BYTE bIPAddr[32];
	BYTE bNetmask[32];
	BYTE bGateway[32];
	BYTE bHwAddr[32];

	/**
	 * When WiFi Mode Actived.
	 */

	BYTE bSSID[128];
	BYTE bPSK[128];

}sNkEther;

enum {

	NK_SDK_PTZ_CTRL_STOP			= (0x1000),
	NK_SDK_PTZ_CTRL_HOME			= (0x2000),
	NK_SDK_PTZ_CTRL_PAN_LEFT		= (0x3000),
	NK_SDK_PTZ_CTRL_PAN_RIGHT,
	NK_SDK_PTZ_CTRL_PAN_AUTO,
	NK_SDK_PTZ_CTRL_TILT_UP			= (0x4000),
	NK_SDK_PTZ_CTRL_TILT_DOWN,
	NK_SDK_PTZ_CTRL_ZOOM_FAR		= (0x5000),
	NK_SDK_PTZ_CTRL_ZOOM_NEAR,
	NK_SDK_PTZ_CTRL_FOCUS_IN		= (0x6000),
	NK_SDK_PTZ_CTRL_FOCUS_OUT,
	NK_SDK_PTZ_CTRL_IRIS_OPEN		= (0x7000),
	NK_SDK_PTZ_CTRL_IRIS_CLOSE,
	NK_SDK_PTZ_CTRL_SET_PREPOS		= (0x8000),
	NK_SDK_PTZ_CTRL_GOTO_PREPOS,
	NK_SDK_PTZ_CTRL_CLR_PREPOS,

};

typedef struct sNkSysInitType
{

	BYTE bCloudID[32];

	struct {

		BYTE bVerion[16]; ///< 版本号字段，在发现设备时显示。
		BYTE bDevName[16]; ///< 设备名称，将会在 NVR 搜索列表上名称中显示，缺省为 HD IPCAM。
		DWORD dwHwCode; ///< 硬件码，默认取值从 100000 - 999999，默认 645110，用于不同设备升级时 NVR 匹配。
		DWORD dwVideoWidth[2], dwVideoHeight[2];

	} sSpec;

	void (*vCapturePic)(DWORD dwWidth, DWORD dwHeight, BYTE *pPicBuffer, DWORD *pdwDataSize);
	void (*vStartStreamChannel)(DWORD dwChannelID);
	void (*vStopStreamChannel)(DWORD dwChannelID);
	void (*vNVRSyncTime)(time_t timeNow);
	void (*vNVRPTZCtrl)(DWORD dwDir, DWORD dwPrePos);
	void (*vGetEther)(BOOL isWiFi, sNkEther *Ether);
	void (*vSetEther)(BOOL isWiFi, sNkEther *Ether);
	void (*vUpgrade)(BYTE *data, DWORD len, DWORD *rate);
	void (*vNVRReboot)();
	
	/**
	 * @brief
	 *  视频旋转事件。
	 *
	 * @details
	 *  当 NVR 需要设备进行视频旋转的时候会触发此事件，根据传入 @ref degree 角度要求作相应的视频旋转。
	 *
	 * @param degree [in]
	 *  旋转角度，目前只有 180。
	 *
	 */
	void (*vRotate)(DWORD degree);
	
	/**
	 * @brief
	 *  查询设备有线是否连接。
	 *
	 * @details
	 *  当设备存在多个路由系统的时候，模块需要了解设备是否连接着有线网络，触发此事件查询。
	 *
	 * @return
	 *  有线网络接上返回 TRUE，否则返回 FALSE。
	 *
	 */
	BOOL (*fgIsEthWiredConnected)();

} sNkSysInitType;


/**
 * @brief
 *  初始化模块。
 *
 * @details
 *  内部调用 @ref fgNKSysInitEx(psInitPrm, dwMaxMemSize, FALSE) 方法。
 */
BOOL fgNKSysInit(sNkSysInitType *psInitPrm, DWORD dwMaxMemSize);

/**
 * @brief
 *  初始化模块。
 *
 * @param sNkSysInitType [in]
 *  初始化参数。
 *
 * @param dwMaxMemSize [in]
 *  内部可分配内存大小。
 *
 * @param bPublic [in]
 *  公开标识，为 TRUE 时表示公开版本，其他型号的 NVR 可以连接该设备，为 FALSE 表示内部版本，只有 GWell NVR 才能连接该设备。
 */
BOOL fgNKSysInitEx(sNkSysInitType *psInitPrm, DWORD dwMaxMemSize, BOOL bPublic);

BOOL fgSendVideoFrame(DWORD dwChannelID, BYTE *pDataBuffer, DWORD dwDataSize, UINT64 u64PTS);
BOOL fgSendAudioFrame(DWORD dwChannelID, BYTE *pDataBuffer, DWORD dwDataSize, UINT64 u64PTS);
BOOL fgNotifyMotionDetection(DWORD dwChannelID);

void vGetApPasswordFromSSID(char *pSSID, char *pPassword);
void vNKSysExit(void);
void vNKSysDebug(BOOL isEnable);

#ifdef __cplusplus
};
#endif
#endif

