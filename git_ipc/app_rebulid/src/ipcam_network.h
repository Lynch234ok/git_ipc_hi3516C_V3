
#ifndef __IPCAM_NETWORK_H__
#define __IPCAM_NETWORK_H__

extern int IPCAM_network_init();
extern void IPCAM_network_destroy();
extern void IPCAM_network_restart();
extern void IPCAM_network_md_handle(int vin, const char *rect_name);
extern void IPCAM_network_switch_to_ap();
extern void IPCAM_network_wireless_restart();

extern int IPCAM_network_init2();
extern int IPCAM_network_protocol_init();

#endif //__IPCAM_NETWORK_H__

