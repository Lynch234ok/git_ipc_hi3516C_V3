#ifdef REBOOT_ONTIME
#ifndef _NK_REBOOT_ONTIME_H
#define _NK_REBOOT_ONTIME_H
#ifdef __cplusplus
extern "C" {
#endif

#include <NkUtils/types.h>
	/*
	 * function : 启动设备定时重启功能 \n
	 * attention ; 想成功调用此接口之前须要先设置配置文件中定时重启的开关enable为true。
	 *
	 * @param[in]		hourNum		设备重启时间点,要求为整点,单位为小时,范围为0 ~ 23,若超出范围,将默认设置为 2
	 *
	 * @return		写入成功，返回写入数据的长度，写入失败返回 -1。
	 */
typedef void *(*fDestroySystem)(void);
extern NK_Int NK_REBOOT_ONTIME_init(NK_Int hourNum, fDestroySystem OnDestroy);
extern NK_Int NK_REBOOT_ONTIME_destroy();
extern NK_Boolean NK_REBOOT_ONTIME_is_flag_exist();


#ifdef __cplusplus
};
#endif
#endif /*NK_TFER_H*/
#endif
