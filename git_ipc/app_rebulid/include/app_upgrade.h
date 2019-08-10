#ifndef _APP_UPGRADE_H_
#define _APP_UPGRADE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
* @brief  固件升级处理, 目前处理逻辑是假如开始了升级进程，不管最终是否升级成功，都会重启
* @param firmware_file  升级固件绝对路径文件名
* @param downgrade      强制降级标志
* @return 成功  0
*         失败    -1
*/
extern int APP_UPGRADE_start(const char *firmware_file, bool downgrade);

#ifdef __cplusplus
}
#endif

#endif // _APP_UPGRADE_H_
