
/**
 * WebSocket 功能接口定义。
 * 详见 https://tools.ietf.org/html/rfc6455。
 *
 */

#include <NkEmbedded/http_server.h> ///< FIXME

#if !defined(NK_HTTP_WEB_SOCKET_H_)
#define NK_HTTP_WEB_SOCKET_H_
NK_CPP_EXTERN_BEGIN


/**
 * @macro
 *  WebSocket 帧数据类型定义。
 */
#define NK_HTTP_WS_OPTYPE_CONTINUATION (0x00) ///< Denotes a continuation frame.
#define NK_HTTP_WS_OPTYPE_TEXT         (0x01) ///< Denotes a text frame.
#define NK_HTTP_WS_OPTYPE_BINARY       (0x02) ///< Denotes a binary frame.
#define NK_HTTP_WS_OPTYPE_CONN_CLOSE   (0x08) ///< Denotes a connection close.
#define NK_HTTP_WS_OPTYPE_PING         (0x09) ///< Denotes a ping.
#define NK_HTTP_WS_OPTYPE_PONG         (0x0a) ///< Denotes a pong.

/**
 * @macro
 *  掩码键值长度，单位：字节。
 */
#define NK_HTTP_WS_MASKKEY_SZ          (4)

/**
 * @macro
 *  最小的帧头长度，单位：字节。
 */
#define NK_HTTP_WS_MIN_FRAME_HEAD_SZ   (2)

/**
 * @macro
 *  最大的帧头长度。
 */
#define NK_HTTP_WS_MAX_FRAME_HEAD_SZ   (NK_HTTP_WS_MIN_FRAME_HEAD_SZ + 8 + NK_HTTP_WS_MASKKEY_SZ)

/**
 * @brief
 *  WebSocket 帧头域数据结构，\n
 *  对应 WebSocket 版本 13。
 */
typedef struct Nk_HTTPWSFrameHeadField {

	NK_Size size;              ///< 头域长度，单位：字节。

	NK_Boolean fin;            ///< FIN 标致位。
	NK_Boolean mask;           ///< 掩码标识。
	NK_UInt32 optype;          ///< 数据类型。
	NK_Size extraLenBytes;     ///< 额外长度描述大小，可能是 0，2，8。
	NK_Size64 payloadlen;      ///< 净荷数据长度。

	/// 掩码键数组。
	NK_Byte maskingKey[NK_HTTP_WS_MASKKEY_SZ];

} NK_HTTPWSFrameHeadField;


NK_API NK_SockTCP *
NK_HTTPWS_Handshake(NK_Allocator *Alloctr, NK_PChar url, NK_PChar username, NK_PChar passphrase, NK_Int us);

/**
 * @brief
 *  WebSocket 握手回应。
 * @brief
 *  在 CGI 实现内调用，针对 WebSocket 版本 13 及相关版本，\n
 *  如果当前 CGI 会话是 WebSocket 会话，\n
 *  内部会调用 @ref Conn 套接字发送接口回复握手确认信息。
 *
 * @param Conn [in]
 *  CGI 连接 TCP 句柄。
 * @param Session [in]
 *  CGI 会话。
 *
 * @return
 *  握手回应成功返回 True，否则返回 False。
 */
NK_API NK_Boolean
NK_HTTPWS_HandshakeAns(NK_SockTCP *ConnTCP, NK_HTTPServerSession *Session);


/**
 * @brief
 *  预读数据帧，并获取帧参数。
 * @details
 *  在 CGI 实现内调用，调用此方法前必先调用 @ref NK_HTTPServer_WSHandshakeAns() 确认 WebSocket 握手成功，\n
 *  否则返回错误，内部会调用通过 @ref Conn 句柄调用 @ref NK_Socket::peekBuffer() 方法去读取 WebSocket 数据帧头，\n
 *  因此此接口调用完后，数据仍然存在于 Socket 缓冲，\n
 *  如果 Socket 当前没有接收到任何数据此方法会立即返回，不会阻塞（阻塞见 @ref NK_HTTPWS_WaitFrame()）。\n
 *  获取帧头域数据后通过 @ref HeadField 数据结构返回。
 *
 * @param Conn [in]
 *  CGI 连接 TCP 句柄。
 * @param HeadField [out]
 *  帧头域信息。
 *
 * @return
 *  预读成功返回 True，并从 @ref HeadField 获取帧参数，否则返回 False。
 *
 */
NK_API NK_Boolean
NK_HTTPWS_PredictFrame(NK_SockTCP *ConnTCP, NK_HTTPWSFrameHeadField *HeadField);

/**
 * @brief
 *  预读数据帧，并获取帧参数。
 * @details
 *  与 @ref NK_HTTPWS_PredictFrame() 方法类似，\n
 *  区别在于内部通过调用 @ref NK_Socket::canRead() 方法等待一个超时时间，\n
 *  超时以后仍然未读取到数据则返回 -1。
 *
 * @param Conn [in]
 *  CGI 连接 TCP 句柄。
 * @param HeadField [out]
 *  帧头域信息。
 * @param wait_us [in]
 *  等待超时时间，单位：微秒，\n
 *  为 0 表示不等待超时，使用同 @ref NK_HTTPWS_PredictFrame() 方法，\n
 *  为 -1 表示一直等待，直到错误返回。
 *
 * @return
 *  见 @ref NK_HTTPWS_PredictFrame() 定义。
 */
NK_API NK_SSize
NK_HTTPWS_WaitFrame(NK_SockTCP *ConnTCP, NK_HTTPWSFrameHeadField *HeadField, NK_Int wait_us);

/**
 * @brief
 *  接收数据帧。
 * @details
 *  此方法用于接收帧的净荷数据，并保存到 @ref stack 内存地址上，\n
 *  调用此接口会立即返回不会阻塞，因此在调用之前必须先调用 @ref NK_HTTPWS_PredictFrame() 或者 @ref NK_HTTPWS_WaitFrame() 确保有数据可以接收，\n
 *  并根据 @ref NK_HTTPWS_PredictFrame() 或者 @ref NK_HTTPWS_WaitFrame() 的调用结果确定分配存放内存的空间，配置 @ref stacklen 大小。\n
 *  如果接收的帧净荷数据大小大于 @ref stacklen，此方法会返回 0，不进行读取。
 *
 * @param Conn [in]
 *  CGI 连接 TCP 句柄。
 * @param HeadField [out]
 *  帧头域信息。
 * @param stack [out]
 *  帧的净荷数据内存位置的起始地址。
 * @param stacklen [in]
 *  帧的经和数据内存大小，单位：字节。
 *
 * @return
 *  读取成功返回净荷数据的大小，有可能为 0，错误返回 -1。
 *
 */
NK_API NK_SSize
NK_HTTPWS_RecvFrame(NK_SockTCP *ConnTCP, NK_HTTPWSFrameHeadField *HeadField, NK_PByte stack, NK_Size64 stacklen);


/**
 * @brief
 *  接收数据帧。
 * @details
 *  与 @ref NK_HTTPWS_RecvFrame() 方法类似，\n
 *  额外增加超时参数 @ref wait_us，允许阻塞读取。\n
 *
 * @param Conn [in]
 *  CGI 连接 TCP 句柄。
 * @param HeadField [out]
 *  帧头域信息。
 * @param stack [out]
 *  帧的净荷数据内存位置的起始地址。
 * @param stacklen [in]
 *  帧的经和数据内存大小，单位：字节。
 * @param wait_us [in]
 *  等待超时时间，单位：微秒，\n
 *  为 0 表示不等待超时，使用同 @ref NK_HTTPWS_PredictFrame() 方法，\n
 *  为 -1 表示一直等待，直到错误返回。
 *
 * @return
 *  读取成功返回净荷数据的大小，有可能为 0，错误返回 -1。
 *
 */
NK_API NK_SSize
NK_HTTPWS_RecvFrameEx(NK_SockTCP *ConnTCP, NK_HTTPWSFrameHeadField *HeadField, NK_PByte stack, NK_Size64 stacklen, NK_Int wait_us);


/**
 * @macro
 *  净荷数据加掩码宏操作。
 */
#define NK_HTTP_WS_MASK_PAYLOAD(__mask, __payload, __payloadlen) \
	do {\
		NK_Int __i = 0;\
		for (__i = 0; __i < __payloadlen; ++__i) {\
			*((NK_PByte)(__payload) + __i) = *((NK_PByte)(__payload) + __i) ^ *((NK_PByte)(__mask) + __i % 4);\
		}\
	} while(0)

/**
 * @macro
 *  净荷数据去掩码宏操作。
 */
#define NK_HTTP_WS_UMASK_PAYLOAD(__mask, __payload, __payloadlen) NK_HTTP_WS_MASK_PAYLOAD(__mask, __payload, __payloadlen)

/**
 * @brief
 *  发送帧数据。
 * @details
 *  向对方发送一帧数据。
 *
 * @param Conn [in]
 *  CGI 连接 TCP 句柄。
 * @param optype [in]
 *  净荷数据内行。
 * @param mask [in]
 *  掩码标识，标识发送数据是否使用数据掩码。
 * @param data [in]
 *  净荷数据的内存起始地址。
 * @param len [in]
 *  净荷数据长度。
 *
 * @return
 *  发送成功返回 True，否则返回 False。
 */
NK_API NK_Boolean
NK_HTTPWS_SendFrame(NK_SockTCP *ConnTCP, NK_UInt32 optype, NK_Boolean mask, NK_PByte data, NK_Size64 len);

/**
 * @macro
 *  发送 Ping 控制数据操作宏。
 */
#define NK_HTTPWS_Ping(__Conn) \
	NK_HTTPWS_SendFrame(__Conn, NK_HTTP_WS_OPTYPE_PING, NK_False, NK_Nil, 0)

/**
 * @macro
 *  发送 Pong 控制数据操作宏。
 */
#define NK_HTTPWS_Pong(__Conn) \
	NK_HTTPWS_SendFrame(__Conn, NK_HTTP_WS_OPTYPE_PONG, NK_False, NK_Nil, 0)

/**
 * @macro
 *  发送 Close 控制数据操作宏。
 */
#define NK_HTTPWS_Close(__Conn) \
	NK_HTTPWS_SendFrame(__Conn, NK_HTTP_WS_OPTYPE_CONN_CLOSE, NK_False, NK_Nil, 0)

/**
 * @macro
 *  发送文本数据操作宏。
 */
#define NK_HTTPWS_SendTextToServer(__Conn, __text) \
	NK_HTTPWS_SendFrame(__Conn, NK_HTTP_WS_OPTYPE_TEXT, NK_True, (NK_PByte)(__text), strlen(__text))

/**
 * @macro
 *  发送文本数据操作宏。
 */
#define NK_HTTPWS_SendTextToClient(__Conn, __text) \
	NK_HTTPWS_SendFrame(__Conn, NK_HTTP_WS_OPTYPE_TEXT, NK_False, (NK_PByte)(__text), strlen(__text))


/**
 * @macro
 *  发送二进制数据操作宏。
 */
#define NK_HTTPWS_SendBinaryToServer(__Conn, __bin, __len) \
	NK_HTTPWS_SendFrame(__Conn, NK_HTTP_WS_OPTYPE_BINARY, NK_True, (NK_PByte)(__bin), (__len))

/**
 * @macro
 *  发送二进制数据操作宏。
 */
#define NK_HTTPWS_SendBinaryToClient(__Conn, __bin, __len) \
	NK_HTTPWS_SendFrame(__Conn, NK_HTTP_WS_OPTYPE_BINARY, NK_False, (NK_PByte)(__bin), (__len))


NK_CPP_EXTERN_END
#endif /* NK_HTTP_WEB_SOCKET_H_ */
