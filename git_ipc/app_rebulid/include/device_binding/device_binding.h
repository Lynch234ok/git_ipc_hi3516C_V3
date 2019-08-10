#ifndef GIT_IPC_DEVICE_BINDING_H
#define GIT_IPC_DEVICE_BINDING_H


/**
 * �����豸��ʱ�ܵ����ݺ�Ļص�����
 *
 * @param ssid      wifi����
 * @param ssid      wifi����
 * @param ssid      wifi��ȫģʽ
 * @param ssid      access toekn
 *
 * @return 0, OK; -1 fail
 */
typedef int (*DEV_BIND_CB_on_recved_data)(char *ssid,
                                          char *pass,
                                          char *wifi_mode,
                                          char *token);

typedef struct devNearAP {

    /// 热点名称。
    char ssid[64];

    /// -100 - 0
    int rssi;

    /// bssid
    char bssid[20];

    /// 加密方式
    char encrytype[48];

} stDEV_NEAR_AP, *lpDEV_NEAR_AP;

typedef void (*DEV_BIND_get_near_ap)(lpDEV_NEAR_AP lpAPs, unsigned int *nAPs);

/**
 * �����豸�󶨹��ܡ� on_recved_data �ص���������Ҫ���ӿ�ʵ�֡�
 *
 * @return 0, OK; -1 fail
 */
int DEV_BIND_start(DEV_BIND_CB_on_recved_data on_recved_data, DEV_BIND_get_near_ap get_near_ap);

/**
 * ����Ҫ��ʱ����ǰ�жϰ󶨹���
 *
 * @return 0, OK; -1 fail
 */
int DEV_BIND_interrupt();


#endif //GIT_IPC_DEVICE_BINDING_H
