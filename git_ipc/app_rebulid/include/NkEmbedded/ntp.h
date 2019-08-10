/**
 * NTP 相关接口方法，内部使用 NK_Socket 模块。
 */

#include <NkUtils/object.h>
#include <NkUtils/allocator.h>

#if !defined(NK_EMB_NTP_H_)
#define NK_EMB_NTP_H_
NK_CPP_EXTERN_BEGIN


/**
 * @brief
 *  通过 NTP 服务器获取当前 UTC 时间。
 *
 */
NK_API NK_UTC1970
NK_NTP_GetUTC(NK_Size timeo, const NK_PChar ntpServerIP, NK_UInt16 ntpServerPort);



NK_CPP_EXTERN_END
#endif /* NK_EMB_NTP_H_ */

