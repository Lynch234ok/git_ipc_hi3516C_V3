/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:	rudpa_vcon.h
 * Describle:
 * History: 
 * Last modified: 2014-01-15 10:28
=============================================================*/

#ifndef __RUDPA_VCON_H__
#define __RUDPA_VCON_H__
#include <stdint.h>


//vcon Status ,between rudp and app
#define VCON_CREAT 1
#define VCON_OPEN	VCON_CREAT
#define VCON_CLOSE	2
#define VCON_DESTROY 3
#define VCON_DESTROY_FIN 4 //recv already exit


#define VCON_EVENT_CLOSE 1
#define VCON_EVENT_DISCONN 2



#define letmesee(...) do{\
	printf("\033[33m %s(%d) || ",__FILE__,__LINE__);\
	printf(__VA_ARGS__);\
	printf("\033[0m");\
}while(0);

/***************implemention of the vcon queue*******************/
typedef struct ja_vcon_node{
	uint32_t vcon_sock;
	uint32_t port;
	uint32_t vcon_status;
	//use as the node's key  the same time ;
	union{
		uint32_t vcon_id;
		uint32_t node_key;
	};
	char vcon_app[64];
	void *vcon_rudpsession; //rudp's sesssion , use for asidesend 's handle
	struct ja_vcon_node* next;
}VCON_t,VCON_NODE_t;


uint32_t vcon_create(uint32_t vcon_id, char *vcon_app);

uint32_t vcon_destroy(uint32_t vcon_id);

uint32_t vcon_send(void *rudp_sesssion, uint32_t vcon_id, char *data, uint32_t size);

#endif // end of rudpa_vcon.h
