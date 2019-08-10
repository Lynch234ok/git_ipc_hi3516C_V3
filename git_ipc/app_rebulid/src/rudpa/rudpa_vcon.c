/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:	rudpa_vcon.c
 * Describle:   vitual connect over rudpa,like a proxy tunnel
 * History: 
 * Last modified: 2014-01-07 16:59
 =============================================================*/


/*
left:
queue need a mutex, safe,put it in vcon_queue's interface function
event not use!!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "rudpa_vcon.h"
#include "send.h"

#include "../netsdk_def.h"
#include <sys/prctl.h>



static VCON_NODE_t g_vcon_queue_hdr = {0};

void vcon_qpush(uint32_t node_key)
{
	VCON_NODE_t *vcon_queue = &g_vcon_queue_hdr;
	while(vcon_queue->next){
		vcon_queue = vcon_queue->next;
	}

	VCON_NODE_t* node = (VCON_NODE_t*)calloc(sizeof(VCON_t), 1);
	node->node_key = node_key;
	vcon_queue->next = node;
}

void vcon_qpop(uint32_t node_key)
{
	VCON_NODE_t *pre_node = &g_vcon_queue_hdr;
	VCON_NODE_t *cur_node = g_vcon_queue_hdr.next;
	while(cur_node){
		if(cur_node->node_key == node_key) {
			break;
		}
		pre_node = cur_node;
		cur_node = cur_node->next;
	}
	if(cur_node){
		pre_node->next = cur_node->next;
		free(cur_node);
	}
}

VCON_t* vcon_qfind(uint32_t node_key)
{
	VCON_NODE_t *cur_node = g_vcon_queue_hdr.next;
	while(cur_node)
	{
		if(cur_node->node_key == node_key){
			return cur_node;
		}
		cur_node = cur_node->next;
	}
	return NULL; //else end of queue,no node_key equal
}


void vcon_dump(void)
{

	VCON_NODE_t *cur_node = &g_vcon_queue_hdr;
	while(cur_node->next)
	{
		cur_node = cur_node->next;
		printf("node(%p) with key %d\n",cur_node, cur_node->node_key);
	}
}

/******get the real spook communication port**************/
#define VCON_APP_SEARCH_PORT 9013
#define IPC
static uint32_t vcon_scan_ip_port(char *app_ip, uint32_t *app_port)
{

#ifndef IPC
	int32_t ret = 0;
	struct timeval tm;
	tm.tv_sec = 2;
	tm.tv_usec = 0;
	uint32_t sock_search = socket(AF_INET, SOCK_DGRAM, 0); 
	if(sock_search <= 0)
	{
		perror("app scan:");
		return 0;
	}

	uint32_t flag = 1;
	if( //0 != setsockopt(sock_search, SOL_SOCKET, SO_BROADCAST, (char*)&flag, sizeof(flag)) 
			0 != setsockopt(sock_search, SOL_SOCKET, SO_RCVTIMEO, (char*)&tm, sizeof(tm))
			|| 0 != setsockopt(sock_search, SOL_SOCKET, SO_SNDTIMEO, &tm, sizeof(tm)))
	{
		letmesee("set sockopt error: %s\n",strerror(errno)); 
		return 0; 
	}

	struct sockaddr_in search_addr;
	search_addr.sin_family = AF_INET;
	search_addr.sin_port = htons(VCON_APP_SEARCH_PORT);	
	search_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	socklen_t socklen = sizeof(struct sockaddr_in);


	char *sendmsg =  "SEARCHJA&";
	char recvbuf[1024] = {0};
	ret = sendto(sock_search, sendmsg, strlen(sendmsg), 0, (struct sockaddr*)&search_addr, socklen);
	if(-1 ==  ret){
		letmesee("send SEARCHJA& error:%s\n",strerror(errno));
		return 0;//
	}
	struct sockaddr_in any;
	ret = recvfrom(sock_search, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&any, &socklen);
	if(-1 ==  ret){
		letmesee("recv SEARCHJA& error:%s\n",strerror(errno));
		return 0;//
	}	
	char ip_buf[32];	
	sscanf(recvbuf, "JAIP%[^&]&ID%*[^&]&PORT%d", ip_buf, app_port);
	if(app_ip){
		memcpy(app_ip, ip_buf, strlen(ip_buf));
	}	
	return *app_port;
#else
	ST_NSDK_NETWORK_PORT port;
	NETSDK_conf_port_get(1, &port);
	*app_port = port.value;
	printf("spook's port = %d\n", *app_port);
#endif
}


void *vcon_recv_thread(void *ctx)
{
	VCON_t *thiz = (VCON_t*)ctx;
	int32_t ret = 0;
	int32_t recv_sz = 0;
	char app_buf[1024*10] ;
	letmesee("vcon_recv_start!!id:%u\n", thiz->vcon_id);
	prctl(PR_SET_NAME, "vcon_recv_thread");
	for(;;)
	{
		usleep(50000);
		struct timeval tm = {.tv_sec =0,.tv_usec = 50000,};
		fd_set rfd_set;
		FD_ZERO(&rfd_set);
		FD_SET(thiz->vcon_sock, &rfd_set);

		ret = select(thiz->vcon_sock+1, &rfd_set, NULL, NULL, &tm);
		if(ret < 0){
			//letmesee("select error %s\n", strerror(errno));
			close(thiz->vcon_sock);
			thiz->vcon_sock = 0;
			//thiz->vcon_sock = socket(AF_INET, SOCK_STREAM, 0);	//joke me?
			thiz->vcon_status = VCON_CLOSE;			
		}else if(0 == ret){
			//timeout			
		}else{
			if(FD_ISSET(thiz->vcon_sock, &rfd_set)){
				char buf[1024*3] = {0};
				recv_sz = recv(thiz->vcon_sock, buf, sizeof(buf), 0);
				if(recv_sz > 0){
					//do blah blah
					
					memset(app_buf, 0, sizeof(app_buf));
					sprintf(app_buf,"<SOUP version=\"1.1\">"
								   "<vcon cmd=\"data\" id=\"%d\" length=\"%d\"/>"
								   "</SOUP>",
								   thiz->vcon_id, recv_sz);
					uint32_t snd_datalen = strlen(app_buf) + recv_sz;
					memcpy(app_buf+strlen(app_buf), buf, recv_sz);					
					if(thiz->vcon_rudpsession){
						char *snd_data = (char*)calloc(snd_datalen, 1);
						memcpy(snd_data, app_buf, snd_datalen); 
						int ret = send_data(thiz->vcon_rudpsession,snd_data, snd_datalen);
						//printf("vcon_recv ret:%d,recv_sz:%d\n", ret, recv_sz);
						free(snd_data);
					}
					
				}else if(0 == recv_sz || -1 == recv_sz){
					close(thiz->vcon_sock);//peer already shutdown
					thiz->vcon_sock = 0;
					thiz->vcon_status = VCON_CLOSE;
					
				}
			}
		}

		//when destroy the vcon,exit thiz thread
		if(VCON_DESTROY == thiz->vcon_status){
			thiz->vcon_status = VCON_DESTROY_FIN;
			break;
		}
	}
	letmesee("vcon_recv_end!!id:%d\n", thiz->vcon_id);
}

uint32_t vcon_create(uint32_t vcon_id, char *vcon_app)
{
	vcon_qpush(vcon_id);
	letmesee("vcon_create:%u,app:%s\n", vcon_id, vcon_app);

	VCON_t *vcon = vcon_qfind(vcon_id);	
	vcon->vcon_sock = socket(AF_INET, SOCK_STREAM, 0);
	memcpy(vcon->vcon_app, vcon_app, sizeof(vcon->vcon_app));
	vcon->vcon_status = VCON_CLOSE;	
	vcon_scan_ip_port(NULL,&vcon->port);	
	//init the recv thread 
	pthread_t tid_vcon;
	pthread_create(&tid_vcon, NULL, vcon_recv_thread, vcon);
	
}


uint32_t vcon_destroy(uint32_t vcon_id)
{
	VCON_t *thiz = vcon_qfind(vcon_id);
	if(thiz){
		thiz->vcon_status = VCON_DESTROY;
		while(VCON_DESTROY_FIN != thiz->vcon_status){
			usleep(100);
		}
		letmesee("vcon_destroy:%u,app:%s\n", vcon_id,thiz->vcon_app);
		vcon_qpop(vcon_id);
	}	
	return 0;
}


uint32_t vcon_send(void *rudp_session, uint32_t vcon_id, char *data, uint32_t size)
{
	VCON_t *vcon = vcon_qfind(vcon_id);
	if(NULL == vcon){
		return -1;
	}

	socklen_t socklen = sizeof(struct sockaddr_in);
	struct sockaddr_in app_addr;
	int32_t ret = 0;

	vcon->vcon_rudpsession = rudp_session;

	if(vcon->vcon_status != VCON_OPEN)
	{
		if(0 == vcon->vcon_sock){
			vcon->vcon_sock = socket(AF_INET, SOCK_STREAM, 0);
      		 }		
		app_addr.sin_family = AF_INET;
		app_addr.sin_port = htons(vcon->port);
		//app_addr.sin_addr.s_addr = inet_addr("192.168.2.9");
		app_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		ret = connect(vcon->vcon_sock, (struct sockaddr*)&app_addr, socklen);
		if(0 == ret){
			vcon->vcon_status = VCON_OPEN;			
		}else if(ret < 0){
			letmesee("connect error %d %d:%s \n",ret, errno, strerror(errno));
		}	
	}

	//now send the data!!!!
	if(vcon->vcon_status == VCON_OPEN)
	{
		letmesee("vcon send:%s\n", data);
		ret = send(vcon->vcon_sock, data, size, 0);
		if(ret < 0){
			letmesee("senderror %d %d:%s\n",ret, errno, strerror(errno));
		}
		
	}
}

//#define  TEST_RUDPA_VCON
#if defined(TEST_RUDPA_VCON)

uint32_t main(uint32_t argc, char **argv)
{
	vcon_create(1314, "spook");
	char buf[1024];
	PackHead bubbleMsg = {.cHeadChar = 0xaa, .uiLength = htonl(0x35),.cPackType = PT_MSGPACK};
	MsgPackData packMsg = {.uiLength = htonl(0x2c), .cMsgType = MSGT_USERVRF};
	UserVrf uservrf = {.sUserName = "admin"};
	memcpy(buf,&bubbleMsg, sizeof(PackHead));
	memcpy(buf + sizeof(PackHead), &packMsg, sizeof(MsgPackData));
	memcpy(buf + sizeof(PackHead) + sizeof(MsgPackData), &uservrf, sizeof(UserVrf));
	uint32_t msglen = sizeof(PackHead) + sizeof(MsgPackData) + sizeof(UserVrf);
	letmesee("msglen:%d\n",msglen);
	//vcon_send(NULL, 1314, buf, msglen);
	//vcon_send(NULL, 1314, "whatthehell", 11);

	char *post_msg = 
		"POST /cgi-bin/gw.cgi HTTP/1.1\r\n"
		"Connection: keep-alive\r\n"
		"Content-Length: 128\r\n"
		"Host:210.21.39.197:8088\r\n\r\n"
		"<juan ver=\"1\" squ=\"abcdefg\" dir=\"0\"><rpermission usr=\"admin\" pwd=\"\" ><config base=\"\" /><playback base=\"\" /></rpermission></juan>";

	letmesee("vcon's msg:%s\n", post_msg);

	vcon_send(NULL, 1314, post_msg, strlen(post_msg));
	

	//	CheckBubble(, msglen);

	int32_t cnt = 0;
	for(;;){
		sleep(3);
		vcon_send(NULL, 1314, post_msg, strlen(post_msg));
		if( 3 == ++cnt){
			break;
		}	
	}
	vcon_destroy(1314);
	
	return 0;
}

#endif

/*
 *#define STRUCT_MEMBER_POS(t,m)  ((unsigned long)(&(((t *)(0))->m)))
 *
 *uint32_t CheckBubble(void *msg , uint32_t msg_sz)
 *{
 *    PackHead *pack = (PackHead *)msg;
 *
 *    letmesee("here i am!!\n");
 *    if (msg_sz < STRUCT_MEMBER_POS(PackHead,uiLength) + sizeof(pack->uiLength))
 *    {
 *        letmesee("undtermin\n");
 *        return -1;
 *    }
 *
 *    
 *    letmesee("%d   :    %d\n", 0xaa != pack->cHeadChar, 0xab != pack->cHeadChar);
 *    //if (0xaa != pack->cHeadChar || 0xab != pack->cHeadChar)
 *    if (0xaa != pack->cHeadChar) 
 *    {
 *        letmesee("mismatch\n");
 *        return -2;
 *    }
 *
 *    uint32_t uiPackLength = ntohl(pack->uiLength);
 *    if (msg_sz < uiPackLength + STRUCT_MEMBER_POS(PackHead,uiLength) + sizeof(pack->uiLength))
 *    {
 *        letmesee("undetermin\n");
 *        return -3;
 *    }
 *
 *    letmesee("nice msg\n");
 *    return 0;
 *}
 */
