/******************************************************************************

  Copyright (C), 2014-, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : rtp.c
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2014/3/19
  Last Modified : 2014/3/19
  Description   : arp  utils , reference to rfc826
 				LINUX:SOCK_PACKET
  History       :
  1.Date        : 2014/3/19
    	Author      : kaga
 	Modification: Created file
******************************************************************************/
#define HAVE_ARP
#ifdef HAVE_ARP

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<stdint.h>
#include<errno.h>
#include<ctype.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>

#include <features.h> /* ��Ҫ����� glibc �汾�� */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h> /* ��·�㣨L2��Э�� */
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h> /* ��·��Э�� */
#endif
#include <net/if_arp.h>

//#include "generic.h" //crc32
#include "arp.h"
#include <sys/prctl.h>

#define ETH_HW_ADDR_LEN 	6
#define IPV4_ADDR_LEN 			4
#define ARP_FRAME_TYPE 		0x0806
#define ETHER_HW_TYPE 		1
#define IP_PROTO_TYPE 		0x0800
#define BROADCAST_HW_ADDR 	"FF:FF:FF:FF:FF:FF"
#define EMPTY_HW_ADDR		"00:00:00:00:00:00"
#define EMPTY_IP_ADDR		"0.0.0.0"

#define ARP_CHECK_RET(ret) \
	if ((ret) == (-1)) return -1
#define ARP_CHECK_RET_GOTO(ret) \
			if ((ret) == (-1)) goto _FAILED_EXIT

enum {
    OP_ARP_REQUEST = 1,
    OP_ARP_REPLY,
    OP_RARP_REQUEST,
    OP_RARP_REPLY,
    OP_DRARP_REQUEST,
    OP_DRARP_REPLY,
    OP_DRARP_ERROR,
    OP_INARP_REQUEST,
    OP_INARP_REPLY,
};

struct arp_packet {
    u_char targ_hw_addr[ETH_HW_ADDR_LEN];
    u_char src_hw_addr[ETH_HW_ADDR_LEN];
    u_short frame_type;
    u_short hw_type;
    u_short prot_type;
    u_char hw_addr_size;
    u_char prot_addr_size;
    u_short op;
    u_char sndr_hw_addr[ETH_HW_ADDR_LEN];
    u_char sndr_ip_addr[IPV4_ADDR_LEN];
    u_char rcpt_hw_addr[ETH_HW_ADDR_LEN];
    u_char rcpt_ip_addr[IPV4_ADDR_LEN];
    u_char padding[18];
    //u_int		crc32;
};

static char sArpDefEther[16] = {"eth0" };
#define DEFAULT_DEVICE sArpDefEther

static bool gArpTrigger = false;
static pthread_t gArpPid = 0;
static fAddOnePeer arp_hook = NULL;

int get_ip_addr(struct in_addr* in_addr, char* str)
{
    struct hostent* hostp;
    in_addr->s_addr = inet_addr(str);
    if (in_addr->s_addr == -1) {
        if ((hostp = gethostbyname(str))) {
            bcopy(hostp->h_addr, in_addr, hostp->h_length);
        } else {
            fprintf(stderr, "send_arp: unknown host %s\n", str);
            return -1;
        }
    }
    return 0;
}
int get_hw_addr(char* buf, char* str)
{
    int i;
    char c, val;
    for (i = 0; i < ETH_HW_ADDR_LEN; i++) {
        if (!(c = tolower(*str++))) {
            printf("Invalid hardware address!\n");
            return -1;
        }
        if (isdigit(c)) {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            printf("Invalid hardware address!\n");
            return -1;
        }
        *buf = val << 4;
        if (!(c = tolower(*str++))) {
            printf("Invalid hardware address!\n");
            return -1;
        }
        if (isdigit(c)) {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            printf("Invalid hardware address!\n");
            return -1;
        }
        *buf++ |= val;
        if (*str == ':') {
            str++;
        } else {
            if (i < 5) {
                printf("Invalid hardware address 5 , %c!\n", *str);
                return -1;
            }
        }
    }
    if (i < 6) {
        printf("Invalid hardware address 6!\n");
        return -1;
    }

    return 0;
}


static int _get_if_addr(char *eth, char * const ip, char * const mac)
{
    int sock;
    //
    struct sockaddr_in sin;
    struct ifreq ifr;

    if(ip) 			ip[0] = 0;
    if(mac) 		mac[0]= 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    strncpy(ifr.ifr_name, eth, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    if(ip) {
        if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
            perror("ioctl");
            close(sock);
            return -1;
        }
        memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
        strcpy(ip, inet_ntoa(sin.sin_addr));
    }

    if(mac) {
        if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
            perror("ioctl");
            close(sock);
            return -1;
        }
        sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", ifr.ifr_hwaddr.sa_data[0] & 0xff,ifr.ifr_hwaddr.sa_data[1] & 0xff,ifr.ifr_hwaddr.sa_data[2] & 0xff,
                ifr.ifr_hwaddr.sa_data[3] & 0xff,ifr.ifr_hwaddr.sa_data[4] & 0xff,ifr.ifr_hwaddr.sa_data[5] & 0xff);
    }

    close(sock);
    return 0;
}


static void arp_recv_sig_alarm(int signo)
{
    gArpTrigger = false;
    printf("sig alarm!\n");
}

int ARP_sock_new()
{
    int ret, sock;
    struct timeval timeo = { ARP_RECV_TIMEOUT, 0};
    int protocal = ETH_P_ARP;

    /*
    if (type == OP_ARP_REQUEST || type == OP_ARP_REPLY) {
    	protocal = ETH_P_ARP;
    }else if (type == OP_RARP_REQUEST || type == OP_RARP_REPLY) {
    	protocal = ETH_P_RARP;
    } */

    sock = socket(AF_INET, SOCK_PACKET, htons(protocal));
    if (sock < 0) {
        perror("socket");
        return -1;
    }
    ret=setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,(void *) &timeo, sizeof(timeo));
    if(ret < 0) {
        printf("ARP-ERROR: set send timeout failed,sock:%d",sock);
        close(sock);
        return -1;
    }
    //set receive timeout
    ret=setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(void *) &timeo,sizeof(timeo));
    if(ret < 0) {
        printf("ARP-ERROR: set receive timeout failed.");
        close(sock);
        return -1;
    }

    return sock;
}

int arp_send_packet(int type, char *dst_mac, char *src_mac, char *ip, char *mac, char *query_ip,
                    char (*query_mac)[18], int out_size)
{

    struct in_addr src_in_addr, targ_in_addr;
    struct arp_packet pkt, *arp = NULL;
    struct sockaddr sa, da;
    char buf[10*1000];
    socklen_t addrlen;
    int ret, sock = 0;
    int query_index = 0;
    fd_set rset;
    struct timeval timeo;

    if ((sock = ARP_sock_new())  < 0) {
        return -1;
    }
    //printf("mac local:%s  query mac:%s ip:%s\n", src_mac, query_mac[0], query_ip);
    pkt.frame_type = htons(ARP_FRAME_TYPE);
    pkt.hw_type = htons(ETHER_HW_TYPE);
    pkt.prot_type = htons(IP_PROTO_TYPE);
    pkt.hw_addr_size = ETH_HW_ADDR_LEN;
    pkt.prot_addr_size = IPV4_ADDR_LEN;
    pkt.op = htons(type);
    ARP_CHECK_RET_GOTO(get_hw_addr(pkt.targ_hw_addr, dst_mac));
    ARP_CHECK_RET_GOTO(get_hw_addr(pkt.rcpt_hw_addr, query_mac[0]));
    ARP_CHECK_RET_GOTO(get_hw_addr(pkt.src_hw_addr, src_mac));
    ARP_CHECK_RET_GOTO(get_hw_addr(pkt.sndr_hw_addr, mac));
    ARP_CHECK_RET_GOTO(get_ip_addr(&src_in_addr, ip));
    ARP_CHECK_RET_GOTO(get_ip_addr(&targ_in_addr, query_ip));
    memcpy(pkt.sndr_ip_addr, &src_in_addr, IPV4_ADDR_LEN);
    memcpy(pkt.rcpt_ip_addr, &targ_in_addr, IPV4_ADDR_LEN);
    bzero(pkt.padding, 18);
    //pkt.crc32 = calculate_crc32(0, &pkt, sizeof(struct arp_packet) - 4);
    bzero(&sa, sizeof(sa));
    strcpy(sa.sa_data, DEFAULT_DEVICE);

    if (sendto(sock, &pkt, sizeof(pkt), 0, &sa, sizeof(sa)) < 0) {
        perror("sendto");
        close(sock);
        return -1;
    }

    gArpTrigger = true;

    timeo.tv_sec =ARP_RECV_TIMEOUT;
    timeo.tv_usec = 0;
    do {
        FD_ZERO(&rset);
        FD_SET(sock, &rset);

        ret = select(sock + 1, &rset, NULL, NULL, &timeo);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            printf("arp recv failed, errno:%d!\n", errno);
            close(sock);
            return -1;
        } else if (ret == 0) {
            //printf("arp recv timeout!\n");
            break;
        } else {
            char peer_ip[20], peer_mac[48];

            bzero(&da, sizeof(da));
            addrlen = sizeof(da);
            if (recvfrom(sock, buf, sizeof(buf), 0, &da, &addrlen) < 0) {
                printf("arp recv failed, errno:%d!\n", errno);
                close(sock);
                return -1;
            } else {
                arp = (struct arp_packet *)buf;
                if (arp->frame_type == ntohs(ARP_FRAME_TYPE) && arp->prot_type ==  ntohs(IP_PROTO_TYPE)) {
                    sprintf(peer_mac, "%02x:%02x:%02x:%02x:%02x:%02x", arp->sndr_hw_addr[0], arp->sndr_hw_addr[1],	 arp->sndr_hw_addr[2],
                            arp->sndr_hw_addr[3],	arp->sndr_hw_addr[4],  arp->sndr_hw_addr[5]);
                    sprintf(peer_ip,"%d.%d.%d.%d", arp->sndr_ip_addr[0], arp->sndr_ip_addr[1], arp->sndr_ip_addr[2], arp->sndr_ip_addr[3] );
                    //printf("got arp packet , frametype:0x%x,  type:%d !!!!!!\n",ntohs(arp->frame_type), ntohs(arp->op));
                    //printf("\tdst mac:%s, dst ip:%s\n", peer_mac, peer_ip);
                    if (type == OP_ARP_REQUEST && arp->op == htons(OP_ARP_REPLY)) {
                        if (memcmp(pkt.rcpt_ip_addr, arp->sndr_ip_addr, IPV4_ADDR_LEN) == 0) {
                            printf("got arp reply %d %s -> %s: !\n", query_index, query_ip, peer_mac);
                            strcpy(query_mac[query_index++], peer_mac);
                            if (query_index >= out_size)
                                break; //kaga
                        }
                    } else if (type == OP_RARP_REQUEST /* && arp->op == htons(OP_ARP_REPLY) */
                               /* && arp->op == htons(OP_RARP_REPLY)*/) {
                        if (memcmp(pkt.rcpt_hw_addr, arp->sndr_hw_addr, ETH_HW_ADDR_LEN) == 0) {
                            //printf("got rarp reply: !\n");
                            strcpy(query_ip, peer_ip);
                            query_index++;
                            break;
                        }
                    }
                }
            }
        }
    } while(gArpTrigger);

_FAILED_EXIT:
    if (sock > 0)
        close(sock);
    return query_index;
}

int arp_send_packet2(int type, char *dst_mac, char *src_mac, char *ip, char *mac, char *query_ip,
                     char (*query_mac)[18], int out_size, int timeout)
{
    struct in_addr src_in_addr, targ_in_addr;
    struct arp_packet pkt, *arp = NULL;
    char buf[10*1000];
    socklen_t addrlen;
    int query_index = 0;
    fd_set rset;
    struct timeval timeo;

    int ret, sock = 0;
    struct sockaddr_ll addr_ii, sa, da;
    struct ifreq ifr;
    int protocal = ETH_P_ARP;
    //int protocal = ETH_P_ALL;
    int eth_index = 0;

    if (type == OP_ARP_REQUEST) {
        protocal = ETH_P_ARP;
    } else if (type == OP_RARP_REQUEST) {
        protocal = ETH_P_RARP;
    } else {
        return -1;
    }

    sock = socket(PF_PACKET, SOCK_RAW, htons(protocal));
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, DEFAULT_DEVICE);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        close(sock);
        return -1;
    }
    eth_index = ifr.ifr_ifindex;
    /*
    	memset(&ifr, 0, sizeof(ifr));
    	strcpy(ifr.ifr_name, DEFAULT_DEVICE);
    	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
    		close(sock);
    		return -1;
    	}
    	memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
    */
    memset( &addr_ii, 0, sizeof(addr_ii) );
    addr_ii.sll_family = AF_PACKET;
    addr_ii.sll_ifindex = eth_index;
    addr_ii.sll_protocol = htons(protocal);
    addr_ii.sll_hatype = 0;//ARPHRD_ETHER;
    addr_ii.sll_pkttype = PACKET_HOST;
    ARP_CHECK_RET_GOTO(get_hw_addr(addr_ii.sll_addr, src_mac));
    //memcpy (addr_ii.sll_addr, src_mac, ETH_HW_ADDR_LEN * sizeof (uint8_t));
    addr_ii.sll_halen =  ETH_HW_ADDR_LEN;
    if(bind(sock, (struct sockaddr *) &addr_ii, (socklen_t)sizeof(addr_ii)) == -1 ) {
        perror("bind()");
        close(sock);
        return -1;
    }

    //printf("mac local:%s  query mac:%s ip:%s\n", src_mac, query_mac[0], query_ip);
    pkt.frame_type = htons(ARP_FRAME_TYPE);
    pkt.hw_type = htons(ETHER_HW_TYPE);
    pkt.prot_type = htons(IP_PROTO_TYPE);
    pkt.hw_addr_size = ETH_HW_ADDR_LEN;
    pkt.prot_addr_size = IPV4_ADDR_LEN;
    pkt.op = htons(type);
    ARP_CHECK_RET_GOTO(get_hw_addr(pkt.targ_hw_addr, dst_mac));
    ARP_CHECK_RET_GOTO(get_hw_addr(pkt.rcpt_hw_addr, query_mac[0]));
    ARP_CHECK_RET_GOTO(get_hw_addr(pkt.src_hw_addr, src_mac));
    ARP_CHECK_RET_GOTO(get_hw_addr(pkt.sndr_hw_addr, mac));
    ARP_CHECK_RET_GOTO(get_ip_addr(&src_in_addr, ip));
    ARP_CHECK_RET_GOTO(get_ip_addr(&targ_in_addr, query_ip));
    memcpy(pkt.sndr_ip_addr, &src_in_addr, IPV4_ADDR_LEN);
    memcpy(pkt.rcpt_ip_addr, &targ_in_addr, IPV4_ADDR_LEN);
    bzero(pkt.padding, 18);
    //pkt.crc32 = calculate_crc32(0, &pkt, sizeof(struct arp_packet) - 4);

    //printf("sock:%d index:%d , type:%x , srcmac:%s dst_mac: %s\n", sock, eth_index, type, src_mac, dst_mac);

    memset( &sa, 0, sizeof(sa) );
    sa.sll_family = AF_PACKET;
    sa.sll_ifindex = eth_index;
    sa.sll_protocol = htons(protocal);
    sa.sll_hatype = 0;//ARPHRD_ETHER;
    sa.sll_pkttype = PACKET_BROADCAST;
    ARP_CHECK_RET_GOTO(get_hw_addr(sa.sll_addr, dst_mac));
    sa.sll_halen =  ETH_HW_ADDR_LEN;
    if (sendto(sock, &pkt, sizeof(pkt), 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("sendto");
        close(sock);
        return -1;
    }

    gArpTrigger = true;

    timeo.tv_sec = ARP_RECV_TIMEOUT;
    timeo.tv_usec = 0;
    do {
        char peer_ip[20], peer_mac[48];

        FD_ZERO(&rset);
        FD_SET(sock, &rset);

        ret = select(sock + 1, &rset, NULL, NULL, &timeo);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            printf("arp recv failed, errno:%d!\n", errno);
            close(sock);
            return -1;
        } else if (ret == 0) {
            //printf("arp recv timeout!\n");
            break;
        } else {
            bzero(&da, sizeof(da));
            addrlen = sizeof(da);
            if (recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&da, &addrlen) < 0) {
                printf("arp recv failed, errno:%d!\n", errno);
                close(sock);
                return -1;
            } else {
                arp = (struct arp_packet *)buf;
                //printf("got packet type: %d(%x)\n", arp->frame_type, arp->frame_type);
                if (arp->frame_type == ntohs(ARP_FRAME_TYPE) && arp->prot_type ==  ntohs(IP_PROTO_TYPE)) {
                    sprintf(peer_mac, "%02x:%02x:%02x:%02x:%02x:%02x", arp->sndr_hw_addr[0], arp->sndr_hw_addr[1],	 arp->sndr_hw_addr[2],
                            arp->sndr_hw_addr[3],	arp->sndr_hw_addr[4],  arp->sndr_hw_addr[5]);
                    sprintf(peer_ip,"%d.%d.%d.%d", arp->sndr_ip_addr[0], arp->sndr_ip_addr[1], arp->sndr_ip_addr[2], arp->sndr_ip_addr[3] );
                    //printf("got arp packet , frametype:0x%x,  type:%d !!!!!!\n",ntohs(arp->frame_type), ntohs(arp->op));
                    //printf("\tdst mac:%s, dst ip:%s\n", peer_mac, peer_ip);
                    //if (type == OP_ARP_REQUEST && arp->op == htons(OP_ARP_REPLY)) {
                    if (type == OP_ARP_REQUEST) {//FIX ME
                        if (memcmp(pkt.rcpt_ip_addr, arp->sndr_ip_addr, IPV4_ADDR_LEN) == 0) {
                            printf("got arp reply %d %s -> %s: !\n", query_index, query_ip, peer_mac);
                            strcpy(query_mac[query_index++], peer_mac);
                            if (query_index >= out_size)
                                break;
                        }
                    } else if (type == OP_RARP_REQUEST /* && arp->op == htons(OP_ARP_REPLY) */
                               /* && arp->op == htons(OP_RARP_REPLY)*/) {
                        if (memcmp(pkt.rcpt_hw_addr, arp->sndr_hw_addr, ETH_HW_ADDR_LEN) == 0) {
                            //printf("got rarp reply: !\n");
                            strcpy(query_ip, peer_ip);
                            query_index++;
                            break;
                        }
                    }
                }
            }
        }

    } while(gArpTrigger);

_FAILED_EXIT:
    if (sock > 0)
        close(sock);
    return query_index;
}


int ARP_send_request(char *query_ip, char (*query_mac)[18], int max_size, int timeout)
{
    char ip[20];
    char mac[48];

    if (timeout > 30) {
        timeout = 30;
    } else if (timeout < 1) {
        timeout = ARP_RECV_TIMEOUT;
    }
	char def_eth[128];
    if(NULL == getenv("DEF_ETHER")){
		snprintf(def_eth, sizeof(def_eth), "eth0");
	}else{
		snprintf(def_eth, sizeof(def_eth), "%s", getenv("DEF_ETHER"));
	}
    strncpy(sArpDefEther, def_eth, sizeof(sArpDefEther));

    if (_get_if_addr(DEFAULT_DEVICE, ip, mac) < 0) return -1;

	strcpy(ip, "0.0.0.0");
    strncpy(query_mac[0], EMPTY_HW_ADDR, 18);
    return arp_send_packet2(OP_ARP_REQUEST, BROADCAST_HW_ADDR, mac, ip, mac, query_ip, query_mac, max_size, timeout);
}

int ARP_query(char *query_ip, char *query_mac)
{
    int ret;
    char ip[20];
    char mac[48];
    char out_mac[1][18];

	char def_eth[128];
	if(NULL == getenv("DEF_ETHER")){
		snprintf(def_eth, sizeof(def_eth), "eth0");
	}else{
		snprintf(def_eth, sizeof(def_eth), "%s", getenv("DEF_ETHER"));
	}
    strncpy(sArpDefEther, def_eth, sizeof(sArpDefEther));

    if (_get_if_addr(DEFAULT_DEVICE, ip, mac) < 0) return -1;
    strcpy(out_mac[0], EMPTY_HW_ADDR);
    ret =  arp_send_packet2(OP_ARP_REQUEST, BROADCAST_HW_ADDR, mac, ip, mac, query_ip, out_mac, 1, ARP_RECV_TIMEOUT);
    strcpy(query_mac, out_mac[0]);
    return ret;
}

int ARP_send_request3(char *query_ip, char *query_mac)
{
    int sock;
    struct arpreq areq;
    struct sockaddr_in *pinaddr;
    unsigned char *hw_addr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

   char def_eth[128];
    if(NULL == getenv("DEF_ETHER")){
		snprintf(def_eth, sizeof(def_eth), "eth0");
	}else{
		snprintf(def_eth, sizeof(def_eth), "%s", getenv("DEF_ETHER"));
	}
    strncpy(sArpDefEther, def_eth, sizeof(sArpDefEther));

    memset(&areq, 0, sizeof(areq));
    pinaddr = (struct sockaddr_in *)&areq.arp_pa;
    pinaddr->sin_family =  AF_INET;
    pinaddr->sin_addr.s_addr = inet_addr(query_ip);
    strcpy(areq.arp_dev, DEFAULT_DEVICE);

    if (ioctl(sock, SIOCGARP, &areq) < 0) {
        perror("ioctl SIOCGARP");
        close(sock);
        return -1;
    }
    hw_addr = areq.arp_ha.sa_data;

    printf("HWAddr found : %02x:%02x:%02x:%02x:%02x:%02x\n",
           hw_addr[0], hw_addr[1], hw_addr[2], hw_addr[3], hw_addr[4],hw_addr[5]);

    close(sock);
    return 0;
}


int RARP_send_request(char *query_ip, char *query_mac)
{
    char ip[20];
    char mac[48];
    char __mac[1][18];

    char def_eth[128];
    if(NULL == getenv("DEF_ETHER")){
		snprintf(def_eth, sizeof(def_eth), "eth0");
	}else{
		snprintf(def_eth, sizeof(def_eth), "%s", getenv("DEF_ETHER"));
	}
    strncpy(sArpDefEther, def_eth, sizeof(sArpDefEther));

    if (_get_if_addr(DEFAULT_DEVICE, ip, mac) < 0) return -1;
    strcpy(query_ip, EMPTY_IP_ADDR);
    strcpy(__mac[0], query_mac);
    return arp_send_packet2(OP_RARP_REQUEST, BROADCAST_HW_ADDR, mac, ip, mac,query_ip , __mac, 1, ARP_RECV_TIMEOUT);
}


void *arp_proc(void *param)
{
    int sock = -1;
    char buf[10000];
    struct sockaddr  da;
    socklen_t addrlen;
    struct arp_packet *arp = NULL;
    char peer_ip[20], peer_mac[48];

    pthread_detach(pthread_self());
	prctl(PR_SET_NAME, "arp_proc");

    if ((sock = ARP_sock_new()) < 0) {
        goto ERR_EXIT;
    }
    do {
        strcpy(da.sa_data, DEFAULT_DEVICE);
        if (recvfrom(sock, buf, sizeof(buf), 0, &da, &addrlen) < 0) {
            printf( "arp recv failed or timeout, errno:%d!\n", errno);
            continue;
        } else {
            arp = (struct arp_packet *)buf;
            if (arp->frame_type == ntohs(ARP_FRAME_TYPE) && arp->prot_type ==  ntohs(IP_PROTO_TYPE)) {
                sprintf(peer_mac, "%02x:%02x:%02x:%02x:%02x:%02x", arp->sndr_hw_addr[0], arp->sndr_hw_addr[1],	arp->sndr_hw_addr[2],
                        arp->sndr_hw_addr[3],  arp->sndr_hw_addr[4],  arp->sndr_hw_addr[5]);
                sprintf(peer_ip,"%d.%d.%d.%d", arp->sndr_ip_addr[0], arp->sndr_ip_addr[1], arp->sndr_ip_addr[2], arp->sndr_ip_addr[3] );
                //printf("got arp packet , frametype:0x%x,  type:%d !!!!!!",ntohs(arp->frame_type), ntohs(arp->op));
                printf("\tdst mac:%s, dst ip:%s\n", peer_mac, peer_ip);
                if (arp_hook) {
                    arp_hook(0, peer_ip, peer_mac);
                }
            }
        }
    } while(gArpTrigger);

ERR_EXIT:
    if (sock > 0) close(sock);
    pthread_exit(NULL);
}


int ARP_listener_start(fAddOnePeer hook)
{
    gArpTrigger = true;
    arp_hook = hook;
    pthread_create(&gArpPid, NULL, arp_proc, NULL);
    return 0;
}

int ARP_listener_stop()
{
    gArpTrigger = false;
    return 0;
}


#ifdef ARP_TEST
int main(int argc, char *argv[])
{
    setenv("DEF_ETHER", "eth0", 1);

    if (argc != 3)
        return -1;
    if (strcmp(argv[1], "-a")  == 0) {
        char query_mac[128][18];
        ARP_send_request(argv[2], query_mac, 128, 3);
    } else if (strcmp(argv[1], "-a2")  == 0) {
        char _mac[18];
        ARP_query(argv[2], _mac);
    } else if (strcmp(argv[1], "-a3")  == 0) {
        ARP_send_request3(argv[2], NULL);
    } else {
        char szip[32];
        RARP_send_request(szip, argv[2]);
    }
    return 0;
}
#endif

#endif//HAVE_ARP

