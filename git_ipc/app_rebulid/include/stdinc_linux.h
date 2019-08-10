#ifndef __STDINC_LINUX_HEAD_FILE__
#define __STDINC_LINUX_HEAD_FILE__

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/vfs.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdarg.h>
#ifndef EZXML_NOMMAP
#include <sys/mman.h>
#endif // EZXML_NOMMAP

#include <netdb.h>
#include <signal.h>
#include <netinet/ip.h>
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

#endif