

#include <NkUtils/n1_def.h>
#include <NkUtils/http_utils.h>
#include <NkUtils/json.h>

#ifndef NK_UTILS_NETSDK_DEF_H_
#define NK_UTILS_NETSDK_DEF_H_
NK_CPP_EXTERN_BEGIN

/**
 * NetSDK �����롣
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
 * �����Ӧ����� NetSDK ���Ľ����
 *
 */
extern NK_Int
NK_NetSDK_Result(NK_HTTPHeadField *HeadField, NK_NetSDKError err, NK_PChar result, NK_Size *result_len);


/**
 * NetSDK �ӿڡ�
 */
typedef enum Nk_NetSDKInterface
{
	NK_NETSDK_INTF_UNDEF = (-1),

	NK_NETSDK_INTF_SYS,			///< ��Ӧ /NetSDK/System/ ǰ׺������ӿڡ�
	NK_NETSDK_INTF_NET,			///< ��Ӧ /NetSDK/Network/ ǰ׺������ӿڡ�
	NK_NETSDK_INTF_AENC,		///< ��Ӧ /NetSDK/Audio/encode/ ǰ׺������ӿڡ�
	NK_NETSDK_INTF_VIN,			///< ��Ӧ /NetSDK/Video/input/ ǰ׺������ӿڡ�
	NK_NETSDK_INTF_VMD,			///< ��Ӧ /NetSDK/Video/motionDetection/ ǰ׺������ӿڡ�
	NK_NETSDK_INTF_VENC,		///< ��Ӧ /NetSDK/Video/encode/ ǰ׺������ӿڡ�

} NK_NetSDKInterface;


/**
 * NetSDK �������ԡ�
 */
typedef struct Nk_PropertyNetSDK
{
	NK_Char interface[1024];
	NK_Boolean all_channels;
	NK_Int channel;
	NK_Boolean with_properties;

} NK_PropertyNetSDK;

/**
 * ��ӡ���ݽṹ @ref NK_PropertyNetSDK��
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
 * �� HTTP ����ͷ����ȡ NetSDK ���ԡ�
 *
 * @param[in]			HeadField				NetSDK ��ؽӿڡ�
 * @param[out]			PropertyNetSDK			NetSDK ���ԡ�
 *
 * @return 		��ȡ�ɹ����� 0��ʧ�ܷ��� -1��
 */
extern NK_Int
NK_NetSDK_ExtractProperty(NK_HTTPHeadField *HeadField, NK_PropertyNetSDK *PropertyNetSDK);


/**
 * ��ȡ���� JSON ���ݽṹ�Ŀ�ߡ�\n
 * ����ľ��� @ref Matrix ���ݽṹ����ʱһ�� MxN �Ķ�λ�������ݽṹ��
 *
 */
extern NK_Int
NK_NetSDK_GetMatrixSize(NK_JSON *Matrix, NK_Size *width_r, NK_Size *height_r);


/**
 * ��ȡ���� JSON ���ݽṹ��Ԫ�ء�\n
 * ����ľ��� @ref Matrix ���ݽṹ����ʱһ�� MxN �Ķ�λ�������ݽṹ��
 *
 */
extern NK_JSON *
NK_NetSDK_GetMatrixItem(NK_JSON *Matrix, NK_Size x, NK_Size y);


/**
 * ת�� @ref NK_N1LanSetup ���ݽṹ�� NetSDK ��� JSON ���ݽṹ��
 *
 * @param[in]			intf					NetSDK ��ؽӿڡ�
 * @param[in]			with_properties			���ص� JSON ���ݽṹ�Ƿ���� properties ������
 * @param[in]			LanSetup				����� @ref NK_N1LanSetup ���ݽṹ��
 * @param[out]			Json					���� JSON ���󣬷��� @ref LanSetup ���Ӧ�� JSON ���ݽṹ��
 *
 * @return		���ز������ @ref NK_NetSDKError��
 */
extern NK_NetSDKError
NK_NetSDK_LanSetupToJSON(NK_NetSDKInterface intf, NK_Boolean with_properties, NK_N1LanSetup *LanSetup, NK_JSON *Json);

/**
 * ת�� NetSDK ��� JSON ���ݽṹ�� @ref NK_N1LanSetup ���ݽṹ��
 *
 * @param[in]			intf					NetSDK ��ؽӿڡ�
 * @param[in]			Json
 * @param[out]			LanSetup
 *
 * @return
 */
extern NK_NetSDKError
NK_NetSDK_LanSetupFromJSON(NK_NetSDKInterface intf, NK_JSON *Json, NK_N1LanSetup *LanSetup);



NK_CPP_EXTERN_END
#endif /* NK_UTILS_NETSDK_DEF_H_ */
