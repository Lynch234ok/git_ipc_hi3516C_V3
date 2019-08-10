
#ifndef __NETWORK_INTERFACE_H__
#define __NETWORK_INTERFACE_H__

#include "ifconf.h"

#ifdef __cplusplus
extern "C" {
#endif
/* ����:true���ߣ�false���� */
extern bool network_check_interface();
extern int network_ifconf_set_interface(const char if_name[IFNAMSIZ], ifconf_interface_t* ifr);
#ifdef __cplusplus
}
#endif

#endif //__NETWORK_INTERFACE_H__

