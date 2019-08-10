

/**
 * 标准库接口定义。
 */

#include <NkUtils/types.h>

#if !defined(NK_EMB_STDLIB_H_)
# define NK_EMB_STDLIB_H_
NK_CPP_EXTERN_BEGIN



/**
 * @brief
 *  获取进系统时间戳（单位：纳秒）。
 *
 * @return
 * 	返回系统时间戳（单位：纳秒）。
 *
 */
NK_API NK_Size64
NK_ClockNano();

/**
 * @brief
 *  获取进系统时间戳（单位：微秒）。
 */
NK_API NK_Size64
NK_ClockMacro();

/**
 * @brief
 *  获取进系统时间戳（单位：毫秒）。
 */
NK_API NK_Size64
NK_ClockMilli();

/**
 * @brief
 *  获取进系统时间戳（单位：秒）。
 */
NK_API NK_Size64
NK_Clock();

 /**
 * @brief
 *  判断文件是否存在。
 *
 * @param path [in]
 *  文件所在路径。
 * 
 * @return
 *  文件存在返回 NK_True，否则返回 NK_False。
 *
 */
NK_API NK_Boolean
NK_HasFile(NK_PChar path);

/**
 * @brief
 *  输出去掉目录后的路径结尾。
 *
 * @details
 *  如：/usr/bin/sort -> "sort"\n
 *
 */
NK_API NK_PChar
NK_Basename(NK_PChar path);

/**
 * @brief
 *  返回 Epoch, 1970-01-01 00:00:00 +0000 的秒数。
 */
NK_API NK_UTC1970
NK_UTC();




NK_API NK_Void
NK_sleep(NK_Size sec);

NK_API NK_Void
NK_usleep(NK_Size usec);

NK_API struct tm *
NK_gmtime(NK_UTC1970 utc, struct tm *TM);

NK_API NK_Int64
NK_atoi64(NK_PChar str);

NK_CPP_EXTERN_END
#endif /* NK_EMB_STDLIB_H_ */






