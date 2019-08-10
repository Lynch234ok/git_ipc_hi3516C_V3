/**
 *
 * CRC 运算类。
 *
 */

#include <NkUtils/md5.h>

#if !defined(NK_UTILS_CRC_H_)
# define NK_UTILS_CRC_H_
NK_CPP_EXTERN_BEGIN

/**
 * @brief
 *  创建 CRC 模块句柄。
 *
 * @param Alloctr [in]
 *  内存分配器。
 * @param bits [in]
 *  CRC 校验的位数，目前只支持 32
 * @return
 */
NK_API NK_CheckSum *
NK_CRC_Create(NK_Allocator *Alloctr, NK_Size bits);

/**
 * 销毁 CRC 模块句柄。
 */
NK_API NK_Void
NK_CRC_Free(NK_CheckSum **CRC_r);

/**
 * @brief
 *  CRC 运算。
 */
NK_API NK_UInt32
NK_CRC_Compute(NK_Size bits, const NK_PVoid data, NK_Size len);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_CRC_H_ */
