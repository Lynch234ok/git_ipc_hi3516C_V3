#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include "socket_tcp.h"
#include "frank_trie.h"
#include <sys/stat.h>
#include "ftp_debug.h"
#ifndef FTP_SERVER_H_
#define FTP_SERVER_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef struct FTP_SERVER_COMMAND_CONTEXT;
//typedef struct FTP_COMMAND_SERVER_PTHREAD;
typedef struct FTP_SERVER_DATA_SEND;
typedef int(*ftp_context_tpye_stru_mode_server)(struct FTP_SERVER_DATA_SEND *const context,char * dir,char *flag);
typedef int(*ftp_context_tpye_send_buf_server)(struct FTP_SERVER_DATA_SEND *const context,char * buf,int size);




typedef struct FTP_SERVER_USER_CONTEXT {
	bool actived;
	char username[64], password[64];
	LP_FRANK_TRIE command_history;
}ST_FTP_SERVER_USER_CONTEXT, *LP_FTP_SERVER_USER_CONTEXT;

typedef struct FTP_SERVER_ATTR {
	int sock;
	in_port_t listen_port;
	char folder[1024];

	LP_FRANK_TRIE command_sets;
	LP_FRANK_TRIE user_sets;
	
	bool listener_trigger;
	pthread_t listener_tid;

	ST_FTP_SERVER_USER_CONTEXT user_context[1];
	
}ST_FTP_SERVER_ATTR, *LP_FTP_SERVER_ATTR;
typedef struct FTP_SERVER_COMMAND_CONTEXT {
	LP_FTP_SERVER_ATTR serv_attr;
	int sock; // data session listen port
	lpSOCKET_TCP client_sock;
	int client_listen_sock;
	int data_listener_trigger;
	bool data_ready_trigger;
	
	LP_FRANK_TRIE  command_default_sets;
	LP_FRANK_TRIE context_type_stru_mode_sets;
	LP_FRANK_TRIE comtext_type_send_buf_sets;
	
	in_port_t data_port;
	
	int (*ftp_create_data_sock_listener)(struct FTP_SERVER_COMMAND_CONTEXT *const context);
	int (*ftp_stop_data_sock_listener)(struct FTP_SERVER_COMMAND_CONTEXT *const context);
	int(*ftp_context_type_send_buf_add)(struct FTP_SERVER_COMMAND_CONTEXT*const context,char *type,ftp_context_tpye_send_buf_server service);
	int(*ftp_context_type_stru_mode_add)(struct FTP_SERVER_COMMAND_CONTEXT*const context,char *type_stru_mode,ftp_context_tpye_stru_mode_server service);
}ST_FTP_SERVER_COMMAND_CONTEXT, *LP_FTP_SERVER_COMMAND_CONTEXT;
typedef struct FTP_SERVER_DATA_SEND{
	LP_FTP_SERVER_COMMAND_CONTEXT  context;
	lpSOCKET_TCP client_sock;
	int client_listen_sock;
}ST_FTP_SERVER_DATA_SEND,*LP_FTP_SERVER_DATA_SEND;

typedef struct FTP_SERVER_DATA_CONTEXT {
	LP_FTP_SERVER_ATTR serv_attr;
	in_port_t data_port; // data session listen port
}ST_FTP_SERVER_DATA_CONTEXT, *LP_FTP_SERVER_DATA_CONTEXT;
typedef int(*ftp_command_server)(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * command_para,lpSOCKET_TCP tcp);
typedef struct FTP_SERVER {
	int forbidden_zero;
	int (*add_command)(struct FTP_SERVER *const ftp_server,const char * command, const char *permission, ftp_command_server service);
	int (*del_command)(struct FTP_SERVER *const ftp_server, const char * command);
	int (*add_user)(struct FTP_SERVER *const ftp_server,const char *username,const char *password,const char * permission);
	int(*del_user)(struct FTP_SERVER *const ftp_server,const char *username);
}ST_FTP_SERVER, *LP_FTP_SERVER;
typedef struct FTP_COMMAND_SERVER_PTHREAD{
LP_FTP_SERVER_COMMAND_CONTEXT command_context;
char *command_para;
lpSOCKET_TCP tcp;
lpSOCKET_TCP client_sock;
int client_listen_sock;
bool create_trigger;
int data_listener_trigger;
bool data_ready_trigger;
}ST_FTP_COMMAND_SERVER_PTHREAD,*LP_FTP_COMMAND_SERVER_PTHREAD;
extern LP_FTP_SERVER FTP_SERV_create(in_port_t listen_port, const char folder[1024]);
extern ftp_context_tpye_stru_mode_server ftp_contest_type_stru_mode_test(LP_FTP_SERVER_DATA_SEND const context,char *flag);
extern int ftp_context_databuf_send(LP_FTP_SERVER_DATA_SEND const context,char * buf,int size);
extern int ftp_context_file_send(LP_FTP_SERVER_DATA_SEND const context,char*dir);
extern void FTP_SERV_release(LP_FTP_SERVER ftp_serv);
extern void  FTP_SERV_COMMAND_CONTEXT_release(LP_FTP_SERVER_COMMAND_CONTEXT  command_context);

#ifdef __cplusplus
}
#endif
#endif //FTP_SERVER_H_

