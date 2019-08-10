/**
 * NK Utils 库的版本定义。
 */

#include <NkUtils/types.h>

#if !defined(NK_UTILS_VERSION_H_)
# define NK_UTILS_VERSION_H_
NK_CPP_EXTERN_BEGIN

/**
 * 主版本号。
 */
#define NK_UTILS_VER_MAJ	(1U)
/**
 * 次版本号。
 */
#define NK_UTILS_VER_MIN	(4U)
/**
 * 修订版本号。
 */
#define NK_UTILS_VER_REV	(6U)


/**
 * 获取 NK Utils 版本号。\n
 * 此接口用于判断库内容与头文件是否一致。\n
 * 如果出现接口获取与头文件版本定义不一致的时候请匹配正确的头文件与库文件。
 *
 * @param[out]			major			对应 @ref NK_N1_VER_MAJ。
 * @param[out]			minor			对应 @ref NK_N1_VER_MIN。
 * @param[out]			revision		对应 @ref NK_N1_VER_REV。
 *
 */
extern NK_Void
NK_Utils_LibraryVersion(NK_Int *major, NK_Int *minor, NK_Int *revision);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_VERSION_H_ */




