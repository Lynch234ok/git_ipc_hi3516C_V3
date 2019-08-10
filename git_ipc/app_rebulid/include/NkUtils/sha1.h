
/**
 *
 * SHA1 运算类。
 *
 */

#include <NkUtils/check_sum.h>

#if !defined(NK_UTILS_SHA1_H_)
# define NK_UTILS_SHA1_H_
NK_CPP_EXTERN_BEGIN


/**
 * 创建 SHA1 模块句柄。
 */
extern NK_CheckSum *
NK_SHA1_Create(NK_Allocator *Alloctr);

/**
 * 销毁 SHA1 模块句柄。
 */
extern NK_Void
NK_SHA1_Free(NK_CheckSum **SHA1_r);



NK_CPP_EXTERN_END
#endif /* NK_UTILS_SHA1_H_ */
