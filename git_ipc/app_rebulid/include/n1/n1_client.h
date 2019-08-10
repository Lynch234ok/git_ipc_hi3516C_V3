
/**
 * N1 客户端 API 接口。
 */

#include "n1_def.h"

#ifndef NK_N1_CLIENT_H_
#define NK_N1_CLIENT_H_
NK_CPP_EXTERN_BEGIN



#define NK_N1_OPT_PROTO_HINET		(1<<0)
#define NK_N1_OPT_PROTO_JUAN		(1<<1)
#define NK_N1_OPT_PROTO_CORSEE		(1<<2)
#define NK_N1_OPT_PROTO_ISAPI		(1<<21)


#define NK_N1_OPT_PROTO_ALL			(NK_N1_OPT_PROTO_HINET | NK_N1_OPT_PROTO_JUAN | NK_N1_OPT_PROTO_CORSEE | NK_N1_OPT_PROTO_ISAPI)


/**
*	NK_N1Client_Login		用户登录客户端接口
*   [in]addrIP				注册的ip
*   [in]port				注册的端口号
*	[in]username			注册的用户名
*	[in]passphrase			注册的密码
*	[in]opt					暂时无效
*	[out]userID				注册成功返回的用户ID号
*  	return					返回值:注册成功返回NK_N1_OK，错误为其他值。
*/
extern NK_N1Result
NK_N1Client_Login(NK_PChar addrIP, NK_UInt16 port, NK_PChar username, NK_PChar passphrase, NK_UInt32 opt, NK_UInt32* userID);

/*
*	NK_N1Client_PTZControl		云台控制接?
*	[in]userID					NK_N1Client_Login 返回的ID值
*  	[in]cmd						云台命令参数 NK_N1PTZCommand 枚举类型
*  	[in]para  					cmd 为NK_N1_PTZ_CMD_GOTO_PRESET，NK_N1_PTZ_CMD_SET_PRESET 时，para为预置点号。
*									其他cmd值时，para为NK_true 为开始，NK_False为停止。
*	return						成功返回	NK_N1_OK, 错误为其他值。
*/
extern NK_N1Result
NK_N1Client_PTZControl(NK_UInt32 userID, NK_N1PTZCommand cmd, NK_Int para);

typedef struct RecvFrameData{
	NK_PVoid  user_live_data;
	NK_UInt32 LiveID;
	NK_PVoid  frame;
	NK_Int frame_len;	
	NK_UInt32 timestamp;
	NK_UInt16 data_type;
}RecvFrameData_t;


typedef void (* onRecvFrame)(RecvFrameData_t* framedata);

/**
*	NK_N1Client_OpenLive		视频直播连接
*	[in]userID					NK_N1Client_Login 返回的ID值
*	[in]nChannelId				频道号，从零起
*	[in]nStreamId				码流号，0,主码流;1,子码流
*	[in]recvframe					接收帧回调接口
*	[in]user_live_data			用户数据，上下文。
*	[out]LiveID					连接设备成功返回的直播号码
*	return						成功返回	NK_N1_OK, 错误为其他值。
*/
extern NK_N1Result
NK_N1Client_OpenLive(NK_UInt32 userID, NK_Int nChannelId,  NK_Int nStreamId, onRecvFrame recvframe,NK_PVOID user_live_data ,NK_UInt32* LiveID);

/**
*	NK_N1Client_CloseLive		视频直播连接关闭
*	[in]userID					NK_N1Client_Login 返回的ID值
*	[in]LiveID					NK_N1Client_OpenLive 获取的ID号
*	return						成功返回	NK_N1_OK, 错误为其他值。
*/
extern NK_N1Result
NK_N1Client_CloseLive(NK_UInt32 userID, NK_UInt32 LiveID);

 
/*
*	NK_N1Client_Logout			登出命令
*	[in]userID					NK_N1Client_Login 返回的ID值
*	return						成功返回	NK_N1_OK, 错误为其他值。
*/
extern NK_N1Result
NK_N1Client_Logout(NK_UInt32 userID);

typedef enum NK_N1Client_Event{
	NK_N1Client_Event_NULL =0,
	NK_N1Client_Event_MD =1,
	NK_N1Client_MD_TYPE_SENSOR_IN = 2,
	NK_N1Client_Event_CONNECT_OK,
	NK_N1Client_Event_DISCONNECT_OK,
}NK_N1Client_Event_Type;

/**
*事件回调接口
*eventType   				事件类型
*lParam					根据事件类型，具体的参数信息。
*rParam					liveID
*customCtx				事件上下文
*/
typedef void (*NK_N1Client_EVENT_HOOK)(NK_N1Client_Event_Type eventType, int lParam, int rParam, void *customCtx);


/**
*设置事件回调接口
*userID					NK_N1Client_Login 返回的ID值
*hook					回调接口,设置为NULL 时，为取消回调。
*user_event_data			用户数据，上下文。
*return					成功返回	NK_N1_OK, 错误为其他值。
*/
extern NK_N1Result NK_N1Client_SetEventHook(NK_UInt32 userID,  NK_N1Client_EVENT_HOOK hook,NK_PVOID user_event_data);

/**
 * @brief
 *  双向语音呼叫。
 * @details
 *  使用双向语音功能时须先调用此方法进行呼叫，然后获取一个通话 @ref talkID 句柄，\n
 *  对句柄进行操作数据通讯操作。
 *
 * @param userID [in]
 *  用户句柄，调用方法 @ref NK_N1Client_Login() 返回。
 * @param chID [in]
 *  对讲连接对应多媒体的通道号，按逻辑顺序从 0 开始递增。
 * @param wait_us [in]
 *  等待超时时间，单位：微秒，\n
 *  为 0 表示不等待超时，使用同 @ref NK_HTTPWS_PredictFrame() 方法，\n
 *  为 -1 表示一直等待，直到错误返回。
 * @param talkID [out]
 *  通话句柄，通过此方法呼叫成功后返回。
 *
 * @retval NK_N1_OK
 *  呼叫成功，从 @ref talkID 获取通话句柄。
 * @retval NK_N1_ERR_DEVICE_NOT_SUPPORT
 *  呼叫失败，设备不支持。
 * @retval NK_N1_ERR_2WAYTALK_BUSY
 *  呼叫失败，设备语音占线。
 *
 */
extern NK_N1Result
NK_N1Client_TwoWayTalk_Call(NK_UInt32 userID, NK_Int chID, NK_Int waitus, NK_UInt32 *talkID);

/**
 * @brief
 *  断开呼叫。
 * @details
 *  断开双向语音通讯，次接口调用成功后，
 *
 * @param talkID [in]
 *  通话句柄，调用 @ref NK_N1Client_TwoWayTalk_Call() 获取。
 *
 * @retval NK_N1_OK
 *  断开成功，@ref talkID 不再有效。
 * @retval NK_N1_ERR_2WAYTALK_INVALID_ID
 *  无效的通话句柄。
 * @retval NK_N1_ERR_OPERATE_TIMEOUT
 *  对讲呼叫超时。
 *
 */
extern NK_N1Result
NK_N1Client_TwoWayTalk_HangUp(NK_UInt32 talkID);

/**
 * @brief
 *  发送通讯数据。
 * @details
 *  目前只支持 G.711a。
 *
 * @param talkID [in]
 *  通话句柄，调用 @ref NK_N1Client_TwoWayTalk_Call() 获取。
 * @param Package [in]
 *  保留属性，传入 Nil 表示 G.711a 采样率 8k16b 净荷数据，\n
 *  由内部产生相对时间戳。
 * @param data [in]
 *  数据起始地址。
 * @param len [in]
 *  数据长度。
 *
 * @retval NK_N1_OK
 *  断开成功，@ref talkID 不再有效。
 * @retval NK_N1_ERR_2WAYTALK_INVALID_ID
 *  无效的通话句柄。
 * @retval NK_N1_ERR_2WAYTALK_SEND_FAILED
 *  发送数据失败，可能链接已经断开。
 */
extern NK_N1Result
NK_N1Client_TwoWayTalk_Send(NK_UInt32 talkID, NK_N1MediaPackage *Package, NK_PByte data, NK_Size len);



NK_CPP_EXTERN_END
#endif /* NK_N1_DEVICE_H_ */
