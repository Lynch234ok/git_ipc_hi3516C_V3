
#include <NkUtils/object.h>
#include <NkEmbedded/thread.h>
#include "http_server.h"

#if !defined(N1_DEVICE_2WAYTALK_H_)
#define N1_DEVICE_2WAYTALK_H_
NK_CPP_EXTERN_BEGIN


/**
 *
 *
 *  404 Not Found -- 设备不支持。
 *  401 Unauthorized -- 校验。
 *  202 Accept -- 占线。
 *  200 Ok -- 拨号成功。
 *
 * @param context
 * @return
 */
extern int
HTTP_CGI_Chat(LP_HTTP_CONTEXT context);


NK_Int
NK_N1Device_TwoWayTalk_init();


NK_CPP_EXTERN_END
#endif /* N1_DEVICE_2WAYTALK_H_ */

