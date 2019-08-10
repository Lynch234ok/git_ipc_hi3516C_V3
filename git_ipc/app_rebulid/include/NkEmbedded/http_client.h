/**
 * HTTP 客户端模块。
 */

#include <NkUtils/types.h>
#include <NkUtils/object.h>
#include <NkUtils/http_utils.h>
#include <NkEmbedded/socket.h>

#if !defined(NK_EMB_HTTP_CLIENT_H_)
#define NK_EMB_HTTP_CLIENT_H_
NK_CPP_EXTERN_BEGIN


typedef NK_Void (*NK_HTTPClientOnReceive)(NK_PVoid ctx, NK_SockTCP *ConnTCP, NK_HTTPHeadField *HeadField);

NK_API NK_Int
NK_HTTPClient_Request(NK_PChar method,
		NK_Size timeout,
		NK_PChar hostIP, NK_UInt16 port,
		NK_PChar abs_path, NK_PChar query_string,
		NK_PChar content_type, NK_PByte content, NK_Size content_len,
		NK_PChar username, NK_PChar passphrase,
		NK_HTTPClientOnReceive onReceive,
		NK_PVoid ctx);


NK_CPP_EXTERN_END
#endif /* NK_EMB_HTTP_SERVER_H_ */


