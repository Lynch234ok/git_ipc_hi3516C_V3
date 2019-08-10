/**
 * 日志信息控制单体模块 V2。
 */

#include <NkUtils/log.h>

#if !defined(NK_UTILS_LOG_V2_H_)
#define NK_UTILS_LOG_V2_H_
NK_CPP_EXTERN_BEGIN


/**
 * Log 单体模块句柄。
 */
struct Nk_LogV2
{
	/**
	 * 继承 V1 版本。
	 */
	struct Nk_Log Log;

	/**
	 * 销毁日志模块。
	 */
	NK_Void
	(*destroy)();

};

/**
 * @brief
 *  创建/获取单体句柄。
 * @details
 *  此方法调用会更新 @ref NK_Log 模块的接口信息，\n
 *  一旦 NK_LogV2 单体句柄创建以后，@ref NK_Log::alertV2 等方法可以正常使用。\n
 *  句柄通过 @ref struct Nk_LogV2::destroy() 方法进行销毁。
 *
 * @see NK_Log
 */
NK_API struct Nk_Log *
NK_LogV2();



NK_CPP_EXTERN_END
#endif /* NK_UTILS_LOG_V2_H_ */
