/******************************************************************************

  Copyright (C), 2014-, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : nvc_dhcp.h
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2014/8/6
  Last Modified : 2014/8/6
  Description   : setup network interface of IPCs automatically
 	
  History       : 
  1.Date        : 2014/8/6
    	Author      : kaga
 	Modification: Created file	
  2, Date 	: 2014/8/13
	Author		: kaga
	Modification: Done, only support onvif now
******************************************************************************/

#ifndef __NET_DHCP_H__
#define __NET_DHCP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <pthread.h>

/*
 other net-utils
*/
/*
*	note: found available ip
*	return:
*		failed return -1, else return number of ip found
*	parameter:
*		-- sz_l_ip: ip address
*		-- sz_l_netmask: netmask
*		-- ip_start: ip address to search start(null to use default: network + 15)
*		-- ip_end : ip address to search end (0xffffffff)
*		-- need_n: the number of available IPs needed
*		-- multi_tasks : the number of pallel tasks to use when searching available IPs
*		-- timeout_s : timeout in second
*		-- hook : search found hook
*/
typedef void (*f_ip_found)(char *origin_ip, char *origin_netmask, char *szip, char *szmac);
extern int NET_find_avai_ip(char *sz_l_ip, char *sz_l_netmask, char *ip_start, char *ip_end, 
	int need_n, int multi_tasks, int timeout_s, f_ip_found hook);
extern int NET_check_ip_conflict(char *ip);

/**
	 *开启DHCP 客户端，成功后udhcpc自动退出
	
	 * @param[in]		网卡名称

	 * @return	void
	 */
extern void NET_openUdhcpc(char *interfaceName);

#ifdef __cplusplus
}
#endif

#endif


