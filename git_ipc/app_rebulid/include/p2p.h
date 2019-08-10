/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:	soup.h
 * Describle:
 * History: 
 * Last modified: 2014-07-29 11:13
=============================================================*/

#ifndef JAP2P_H
#define JAP2P_H

#include <sys/types.h>
#include <sys/socket.h>

//put the PLAT definetion in makefile
//#define PLAT_X64     1     //build x64's prog 
#define ESEE_AUTH_TTL 15   //login esee timeout
#define ESEE_HEARTBEAT_TTL 30 //esee hearbeat timeout
#define ESEE_DEBUG_ADDR "113.105.223.77"
#define ESEE_DEBUG_SN     "JA00002570E52B38611010"
#define HOLETO_TTL 10 
#define TURN_KEEP_CHN_TTL 30  //turn chn alive time when no conn on this channel


//every udx connect snd buf
#ifndef P2P_UDX_BUF_SIZE 
#define P2P_UDX_BUF_SIZE (1024*1024)
#endif

//snd thread's buf size
//main, sub1, sub2
#ifndef P2P_MAINSRTEAM_BUF_SIZE 
#define P2P_MAINSRTEAM_BUF_SIZE (1024*1024*1.2)
#endif

#ifndef P2P_SUBSTREAM1_BUF_SIZE 
#define P2P_SUBSTREAM1_BUF_SIZE (1024*1024*1.2/4)
#endif  

#ifndef P2P_SUBSTREAM2_BUF_SIZE 
#define P2P_SUBSTREAM2_BUF_SIZE (256*1024*1.2/4)   //for ipc use
#endif  



//every udx connect snd buf
#define P2P_VCON_RCV_BUF_SIZE 1024*20 

//type of transmission that comunicate with local service
#define SOUP_VCON_TCP 1       
#define SOUP_VCON_UDP 2
#define SOUP_VCON_UNIX_STREAM 3  //unix domain socket stream
#define SOUP_VCON_UNIX_DGRAM 4  //unix domain socket DGRAM



//cauze some diff between SN(crypto),medibuf,user manage
//you must define  one, only one of these macro 
//#define P2P_FOR_DVR
//#define P2P_FOR_NVR
#define P2P_FOR_IPC

#if \
	(defined(P2P_FOR_DVR) && defined(P2P_FOR_NVR))\
	|| (defined(P2P_FOR_DVR) && defined(P2P_FOR_IPC))\
	|| (defined(P2P_FOR_NVR) && defined(P2P_FOR_IPC))\
	|| (!defined(P2P_FOR_DVR) && !defined(P2P_FOR_NVR) && !defined(P2P_FOR_IPC))
#error "You must define one and only one of P2P_FOR_DVR/NVR/IPC macros"
#endif

#ifdef __cplusplus
extern "C"{
#endif


int P2PStart(const char *sn, const char *version, const char *vend, const char *hwcode);

int P2PAddVconService(const char *service,
					  int type, 
					  struct sockaddr* serviceaddr, 
					  socklen_t addrlen);

#ifdef __cplusplus
}
#endif

#endif //end of p2p.h
