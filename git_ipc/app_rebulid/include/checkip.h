#ifndef __CHECKIP_H
#define __CHECKIP_H


int is_ip_using(unsigned char ip[4]);
int is_same_subnet(unsigned char *netmask, unsigned char *ipv4_host,unsigned char *ipv4_dst);

#endif /* __CHECKIP_H */

