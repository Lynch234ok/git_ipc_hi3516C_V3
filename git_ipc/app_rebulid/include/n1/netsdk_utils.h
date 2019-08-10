

#include <NkUtils/n1_def.h>
#include <NkUtils/http_utils.h>
#include <NkUtils/json.h>

#ifndef NK_UTILS_NETSDK_DEF_H_
#define NK_UTILS_NETSDK_DEF_H_
NK_CPP_EXTERN_BEGIN

/**
 * NetSDK 错误码。
 */
typedef enum Nk_NetSDKError
{
	NK_NETSDK_ERR_UNKNOWN = (-1),
	NK_NETSDK_OK = (0),

	NK_NETSDK_ERR_DEVICE_ERROR = (100),
	NK_NETSDK_ERR_DEVICE_BUSY,
	NK_NETSDK_ERR_DEVICE_OPERTAION,
	NK_NETSDK_ERR_DEVICE_NOT_SUPPORT,
	NK_NETSDK_ERR_DEVICE_NOT_IMPLEMENT,

	NK_NETSDK_ERR_INVALID_OPERATION = (200),
	NK_NETSDK_ERR_INVALID_AUTHORIZED,
	NK_NETSDK_ERR_INVALID_REQUEST,
	NK_NETSDK_ERR_INVALID_DOCUMENT,


} NK_NetSDKError;

/**
 * 输出对应结果的 NetSDK 报文结果。
 *
 */
extern NK_Int
NK_NetSDK_Result(NK_HTTPHeadField *HeadField, NK_NetSDKError err, NK_PChar result, NK_Size *result_len);


/**
 * NetSDK 接口。
 */
typedef enum Nk_NetSDKInterface
{
	NK_NETSDK_INTF_UNDEF = (-1),

	NK_NETSDK_INTF_SYS,			///< 对应 /NetSDK/System/ 前缀的请求接口。
	NK_NETSDK_INTF_NET,			///< 对应 /NetSDK/Network/ 前缀的请求接口。
	NK_NETSDK_INTF_AENC,		///< 对应 /NetSDK/Audio/encode/ 前缀的请求接口。
	NK_NETSDK_INTF_VIN,			///< 对应 /NetSDK/Video/input/ 前缀的请求接口。
	NK_NETSDK_INTF_VMD,			///< 对应 /NetSDK/Video/motionDetection/ 前缀的请求接口。
	NK_NETSDK_INTF_VENC,		///< 对应 /NetSDK/Video/encode/ 前缀的请求接口。

} NK_NetSDKInterface;


/**
 * NetSDK 请求属性。
 */
typedef struct Nk_PropertyNetSDK
{
	NK_Char intf[1024];
	NK_Boolean all_channels;
	NK_Int channel;
	NK_Boolean with_properties;

} NK_PropertyNetSDK;

/**
 * 打印数据结构 @ref NK_PropertyNetSDK。
 *
 */
#define NK_PROPERTY_NETSDK_DUMP(__PropertyNetSDK) \
	do{\
		NK_TermTable Table;\
		NK_CHECK_POINT();\
		NK_TermTbl_BeginDraw(&Table, "NetSDK Head Filed", 96, 4);\
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Path", "%s", (__PropertyNetSDK)->interface);\
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Properties", (__PropertyNetSDK)->with_properties ? "Yes" : "No");\
		if ((__PropertyNetSDK)->all_channels) {\
			NK_TermTbl_PutKeyValue(&Table, NK_True, "All Channels", "Yes");\
		} else {\
			NK_TermTbl_PutKeyValue(&Table, NK_True, "Channel", "%d", (__PropertyNetSDK)->channel);\
		}\
		NK_TermTbl_EndDraw(&Table);\
	} while(0)


/**
 * 从 HTTP 请求头域提取 NetSDK 属性。
 *
 * @param[in]			HeadField				NetSDK 相关接口。
 * @param[out]			PropertyNetSDK			NetSDK 属性。
 *
 * @return 		提取成功返回 0，失败返回 -1。
 */
extern NK_Int
NK_NetSDK_ExtractProperty(NK_HTTPHeadField *HeadField, NK_PropertyNetSDK *PropertyNetSDK);


/**
 * 获取矩阵 JSON 数据结构的宽高。\n
 * 传入的矩阵 @ref Matrix 数据结构必须时一个 MxN 的二位数组数据结构。
 *
 */
extern NK_Int
NK_NetSDK_GetMatrixSize(NK_JSON *Matrix, NK_Size *width_r, NK_Size *height_r);


/**
 * 获取矩阵 JSON 数据结构的元素。\n
 * 传入的矩阵 @ref Matrix 数据结构必须时一个 MxN 的二位数组数据结构。
 *
 */
extern NK_JSON *
NK_NetSDK_GetMatrixItem(NK_JSON *Matrix, NK_Size x, NK_Size y);


/**
 * 转换 @ref NK_N1LanSetup 数据结构成 NetSDK 相关 JSON 数据结构。
 *
 * @param[in]			intf					NetSDK 相关接口。
 * @param[in]			with_properties			返回的 JSON 数据结构是否带由 properties 参数。
 * @param[in]			LanSetup				传入的 @ref NK_N1LanSetup 数据结构。
 * @param[out]			Json					传入 JSON 对象，返回 @ref LanSetup 相对应的 JSON 数据结构。
 *
 * @return		返回操作结果 @ref NK_NetSDKError。
 */
extern NK_NetSDKError
NK_NetSDK_LanSetupToJSON(NK_NetSDKInterface intf, NK_Boolean with_properties, NK_N1LanSetup *LanSetup, NK_JSON *Json);

/**
 * 转换 NetSDK 相关 JSON 数据结构成 @ref NK_N1LanSetup 数据结构。
 *
 * @param[in]			intf					NetSDK 相关接口。
 * @param[in]			Json
 * @param[out]			LanSetup
 *
 * @return
 */
extern NK_NetSDKError
NK_NetSDK_LanSetupFromJSON(NK_NetSDKInterface intf, NK_JSON *Json, NK_N1LanSetup *LanSetup);



NK_CPP_EXTERN_END
#endif /* NK_UTILS_NETSDK_DEF_H_ */
