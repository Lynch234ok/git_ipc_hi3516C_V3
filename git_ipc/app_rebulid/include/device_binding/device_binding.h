#ifndef GIT_IPC_DEVICE_BINDING_H
#define GIT_IPC_DEVICE_BINDING_H


/**
 * ÓÃÓÚÉè±¸°ó¶¨Ê±ÊÜµ½Êı¾İºóµÄ»Øµ÷º¯Êı
 *
 * @param ssid      wifiÃû³Æ
 * @param ssid      wifiÃÜÂë
 * @param ssid      wifi°²È«Ä£Ê½
 * @param ssid      access toekn
 *
 * @return 0, OK; -1 fail
 */
typedef int (*DEV_BIND_CB_on_recved_data)(char *ssid,
                                          char *pass,
                                          char *wifi_mode,
                                          char *token);

typedef struct devNearAP {

    /// çƒ­ç‚¹åç§°ã€‚
    char ssid[64];

    /// -100 - 0
    int rssi;

    /// bssid
    char bssid[20];

    /// åŠ å¯†æ–¹å¼
    char encrytype[48];

} stDEV_NEAR_AP, *lpDEV_NEAR_AP;

typedef void (*DEV_BIND_get_near_ap)(lpDEV_NEAR_AP lpAPs, unsigned int *nAPs);

/**
 * ¿ªÆôÉè±¸°ó¶¨¹¦ÄÜ¡£ on_recved_data »Øµ÷º¯Êı±ØĞëÒª°´½Ó¿ÚÊµÏÖ¡£
 *
 * @return 0, OK; -1 fail
 */
int DEV_BIND_start(DEV_BIND_CB_on_recved_data on_recved_data, DEV_BIND_get_near_ap get_near_ap);

/**
 * ÔÚĞèÒªµÄÊ±ºò£¬ÌáÇ°ÖĞ¶Ï°ó¶¨¹ı³Ì
 *
 * @return 0, OK; -1 fail
 */
int DEV_BIND_interrupt();


#endif //GIT_IPC_DEVICE_BINDING_H
