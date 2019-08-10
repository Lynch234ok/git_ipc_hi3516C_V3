
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
	ֻ������������¿���wifi sta ����״̬���
	1.ipc��ʼ��ʱ��wifi���ӷ�ʽ��staģʽ����,apģʽ������
	2.AP������������ϣ���Ϊstaģʽ��
*/
extern void IPCAM_timer_check_sta_status_start();

/*
	����������¹ر�wifi sta ����״̬��⣬�����첽���µ���ʾ��������
	�磬��ʾ�����ָ��������ã�Ȼ���ֱ���������ʧ��
	1.�л���APģʽ
	2.�ָ���������
*/
extern void IPCAM_timer_check_sta_status_stop();


#endif //__IPCAM_TIMER_H__

