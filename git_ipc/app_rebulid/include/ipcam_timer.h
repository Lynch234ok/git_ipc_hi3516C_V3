
#ifndef __IPCAM_TIMER_H__
#define __IPCAM_TIMER_H__

extern int IPCAM_timer_init();
extern void IPCAM_timer_destroy();

extern void IPCAM_timer_sdk_destroy();

extern void IPCAM_timer_network_destroy();
extern void IPCAM_timer_network_init();
extern void IPCAM_timer_open_wifi_modify_bps();
extern void IPCAM_timer_destory_wifi_modify_bps();

/*
	只有在以下情况下开启wifi sta 连接状态检测
	1.ipc初始化时，wifi连接方式是sta模式开启,ap模式不开启
	2.AP或声波配置完毕，改为sta模式后
*/
extern void IPCAM_timer_check_sta_status_start();

/*
	在以下情况下关闭wifi sta 连接状态检测，避免异步导致的提示音错报问题
	如，提示音报恢复出厂设置，然后又报无线连接失败
	1.切换到AP模式
	2.恢复出厂设置
*/
extern void IPCAM_timer_check_sta_status_stop();


#endif //__IPCAM_TIMER_H__

