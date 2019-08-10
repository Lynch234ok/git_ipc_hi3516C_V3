
#include "ftp_server.h"
#include <sys/prctl.h>
#define MAX 1024


typedef struct FTP_SERV_COMMAND_VECTOR{
	char command[20];
	char permission[20];
	ftp_command_server service;
}ST_FTP_SERV_COMMAND_VECTOR,*LP_FTP_SERV_COMMAND_VECTOR;
typedef struct FTP_CONTEXT_TPYE_STRU_MODE_VECTOR{
	char type_stru_mode[20];
	ftp_context_tpye_stru_mode_server service;
}ST_FTP_CONTEXT_TPYE_STRU_MODE_VECTOR,*LP_FTP_CONTEXT_TPYE_STRU_MODE_VECTOR;
typedef struct FTP_CONTEXT_TYPE_SEND_BUF_VECTOR{
	char type[20];
	ftp_context_tpye_send_buf_server service;
}ST_FTP_CONTEXT_TYPE_SEND_BUF_VECTOR,*LP_FTP_CONTEXT_TYPE_SEND_BUF_VECTOR;
typedef struct FTP_SERV_USER_VECTOR{
	char username[20];
	char password[20];
	char permission[20];
}ST_FTP_SERV_USER_VECTOR,*LP_FTP_SERV_USER_VECTOR;

typedef struct FTP_SERV_COMMAND_DEFAULT_VECTOR{
	char command[20];
	char command_val[50];
}ST_FTP_SERV_COMMAND_DEFAULT_VECTOR,*LP_FTP_SERV_COMMAND_DEFAULT_VECTOR;

//command operation
static int ftp_serv_add_command(struct FTP_SERVER *const ftp_server,const char * command,const char *permission, ftp_command_server service){
		LP_FTP_SERVER_ATTR attr = (LP_FTP_SERVER_ATTR)(ftp_server+1);
		ST_FTP_SERV_COMMAND_VECTOR command_vector;
		if(NULL==ftp_server||NULL==command||NULL==service){
			return -1;
		}
		if(NULL==attr->command_sets){
			attr->command_sets=FTRIE_create(sizeof(ST_FTP_SERV_COMMAND_VECTOR));
		}
		if(NULL==attr->command_sets){
			return -1;
		}
		snprintf(command_vector.command, sizeof(command_vector.command), "%s", command);
		snprintf(command_vector.permission, sizeof(command_vector.permission), "%s", permission);
		command_vector.service = service;
		return attr->command_sets->add(attr->command_sets,command_vector.command,&command_vector);
}
static int ftp_serv_del_command(struct FTP_SERVER * const ftp_server, const char * command){
		LP_FTP_SERVER_ATTR attr = (LP_FTP_SERVER_ATTR)(ftp_server+1);
		if(!ftp_server||!command){
			return -1;
		}
		
		return attr->command_sets->del(attr->command_sets,command);
}
static int ftp_serv_add_user(struct FTP_SERVER *const ftp_server,const char *username,const char *password,const char * permission){
		LP_FTP_SERVER_ATTR attr = (LP_FTP_SERVER_ATTR)(ftp_server+1);
		ST_FTP_SERV_USER_VECTOR user_vector;
		if(NULL!=username&&NULL!=password&&NULL!=permission){
			if(!attr->user_sets){
				attr->user_sets=FTRIE_create(sizeof(ST_FTP_SERV_USER_VECTOR));
			}
			if(NULL==attr->user_sets){
				return -1;
			}
			snprintf(user_vector.username,sizeof(user_vector.username),"%s",username);
			snprintf(user_vector.password,sizeof(user_vector.password),"%s",password);
			snprintf(user_vector.permission,sizeof(user_vector.permission),"%s",permission);
			return attr->user_sets->add(attr->user_sets,user_vector.username,&user_vector);
		}
		return -1;
}
static int ftp_serv_del_user(struct FTP_SERVER *const ftp_server,const char *username){
		LP_FTP_SERVER_ATTR attr = (LP_FTP_SERVER_ATTR)(ftp_server+1);
		if(!ftp_server||!username||!attr->user_sets){
			return -1;
		}
		return attr->user_sets->del(attr->user_sets,username);
}
//tpye-str-mode
static int ftp_serv_context_tpye_str_mode_add(struct FTP_SERVER_COMMAND_CONTEXT *const context,char *type_stru_mode,ftp_context_tpye_stru_mode_server service){
		LP_FTP_SERVER_COMMAND_CONTEXT pcontext=(LP_FTP_SERVER_COMMAND_CONTEXT)(context);
		ST_FTP_CONTEXT_TPYE_STRU_MODE_VECTOR tpye_stru_mode_vector;
		if(NULL!=pcontext&&NULL!=type_stru_mode&&NULL!=service){
			if(!pcontext->context_type_stru_mode_sets){
				pcontext->context_type_stru_mode_sets=FTRIE_create(sizeof(ST_FTP_CONTEXT_TPYE_STRU_MODE_VECTOR));
			}
			if(NULL==pcontext->context_type_stru_mode_sets){
				return -1;
			}
			tpye_stru_mode_vector.service = service;
			snprintf(tpye_stru_mode_vector.type_stru_mode,sizeof(tpye_stru_mode_vector.type_stru_mode),"%s",type_stru_mode);
			pcontext->context_type_stru_mode_sets->add(pcontext->context_type_stru_mode_sets,tpye_stru_mode_vector.type_stru_mode,&tpye_stru_mode_vector);
			return 0;
		}
		return -1;

}
static int ftp_serv_context_type_send_buf_add(struct FTP_SERVER_COMMAND_CONTEXT *const context,char *type,ftp_context_tpye_send_buf_server service){
		LP_FTP_SERVER_COMMAND_CONTEXT pcontext=(LP_FTP_SERVER_COMMAND_CONTEXT)(context);
		ST_FTP_CONTEXT_TYPE_SEND_BUF_VECTOR buf_verctor;
		if(NULL==pcontext||NULL==type||NULL==service){
			return -1;
		}
		if(NULL==pcontext->comtext_type_send_buf_sets){
			pcontext->comtext_type_send_buf_sets=FTRIE_create(sizeof(ST_FTP_CONTEXT_TYPE_SEND_BUF_VECTOR));
		}
		if(NULL==pcontext->comtext_type_send_buf_sets){
			return -1;
		}
		snprintf(buf_verctor.type,sizeof(buf_verctor.type),"%s",type);
		buf_verctor.service=service;
		pcontext->comtext_type_send_buf_sets->add(pcontext->comtext_type_send_buf_sets,buf_verctor.type,&buf_verctor);
		return 0;
}
static int ftp_stop_data_sock_listener(struct FTP_SERVER_COMMAND_CONTEXT *const context){
		printf("ftp_stop_data_sock_listener_%d\n",__LINE__);
		context->client_sock->close(context->client_sock);
		context->data_listener_trigger=1;
		context->data_ready_trigger=0;
		context->data_port=-1;
		close(context->client_listen_sock);
		return 0;
}
static void substitute(char *pinput,char*poutput,char*psrc,char *pdst){
	char *pi,*po,*p,*po_1;
	int nsrclen,ndstlen,nlen;
	pi=pinput;
	po=poutput;
	po_1=po;
	nsrclen=strlen(psrc);
	ndstlen=strlen(pdst);
	p=strstr(pi,psrc);
	if(p){
		while(p){
				nlen=(int)(p-pi);
				memcpy(po,pi,nlen);
				memcpy(po+nlen,pdst,ndstlen);	
				pi=p+nsrclen;
				po=po+nlen+ndstlen;
				p=strstr(pi,psrc);	
			}
			strcpy(po,pi);
		}
	else{strcpy(po,pi);}
}
static int ftp_context_I_F_S(struct FTP_SERVER_DATA_SEND *const context,char *dir,char *flag){
		printf("ftp_context_I_F_S_%d\n",__LINE__);
		char buf[MAX];
		memset(buf,0,sizeof(char)*MAX);
		FILE *fp=NULL;
		int r_num =0;
		int s_num=0;
		int if_zero=0;
		int buf_finist=0;
		int time_num=0;
		if(NULL!=dir&&NULL!=context->client_sock){
			fp=fopen(dir,"rb");
			if(NULL!=fp){
				r_num=fread(buf,sizeof(char),MAX,fp);
				while(r_num>0){
					s_num=context->client_sock->send(context->client_sock,buf,r_num,0);
					if(s_num<0){
						goto ftp_context_I_F_S_error2;
					}
					if_zero=r_num-s_num;
					buf_finist=s_num;
					time_num=10;
					while(0!=if_zero){
						s_num=context->client_sock->send(context->client_sock,buf+buf_finist,if_zero,0);
						if(s_num<0){
							goto ftp_context_I_F_S_error2;
						}
						if_zero=if_zero-s_num;
						buf_finist=buf_finist+s_num;
						time_num=time_num-1;
						if(0==time_num){
							break;
						}
					}
					if(0!=if_zero){
						goto ftp_context_I_F_S_error2;
					}
					memset(buf,0,sizeof(char)*MAX);
					r_num=fread(buf,sizeof(char),MAX,fp);
				}
			}
			fclose(fp);
			fp=NULL;
			return 0;
			goto ftp_context_I_F_S_error1;
		}
		ftp_context_I_F_S_error2:
			fclose(fp);
			fp=NULL;
		ftp_context_I_F_S_error1:
			return -1;
}

static int ftp_context_recv_I_F_S (struct FTP_SERVER_DATA_SEND *const context,char *dir,char *flag){
		printf("ftp_context_recv_I_F_S_%d\n",__LINE__);	
		char buf[MAX];
		memset(buf,0,sizeof(char)*MAX);
		FILE *fp=NULL;
		char current_dir[200];
		char file_path[200];
		int ret = 1;
		int ret2 = 0;		
		int buflen = 0;
		
		if(NULL != dir && NULL != context->client_sock){
			memset(current_dir,0,200);
			getcwd(current_dir,200);
			snprintf(file_path,200,"%s/%s",current_dir,dir);	
			fp = fopen(file_path,"wb+");
			if(NULL != fp){				
				while(ret){
					buflen = context->client_sock->recv(context->client_sock,buf,sizeof(buf),0);	//非阻塞模式	MSG_DONTWAIT
				    if(buflen < 0){
						goto ftp_context_recv_I_F_S_error;
					}
					else if(0 == buflen){							
					// 这里表示对端的socket已正常关闭.										
						printf("THE client sock is close,date aleady been send !\n");
						fclose(fp);
						fp = NULL;
						return 0;
					}
										
					ret2 = fwrite(buf,sizeof(char),buflen,fp);
					printf("buflen=%d****ret2=%d*********%d\n",buflen,ret2,__LINE__);

					if(ret2 != buflen){
						printf("fwriting erro!\n");
						goto ftp_context_recv_I_F_S_error;
					}							
					memset(buf,0,sizeof(char)*MAX);			
					}					
				}	
			fclose(fp);
			fp = NULL;			
			}
		    return -1;
			
ftp_context_recv_I_F_S_error:
			fclose(fp);
			fp = NULL;
			return -1;		
}


static int ftp_context_I_R_S(struct FTP_SERVER_DATA_SEND *const context,char *dir,char *flag){
		printf("ftp_context_I_R_S===\n");
		//Unsupported ;
		return 0;
}


static int ftp_context_recv_I_R_S(struct FTP_SERVER_DATA_SEND *const context,char *dir,char *flag){
		printf("ftp_context_recv_I_R_S===\n");
		//Unsupported ;
		return 0;
}

static int ftp_context_A_F_S(struct FTP_SERVER_DATA_SEND *const context,char *dir,char *flag){
		printf("ftp_context_A_F_S_%d===\n",__LINE__);
		char buf[MAX+2];
		memset(buf,0,sizeof(char)*(MAX+2));
		char *psrc="\n";
		char *pdst="\r\n";
		FILE *fp=NULL;
		int r_num=0;
		int s_num=0;
		int s_buf_num=0;
		int if_zero=0;
		int buf_finist=0;
		int time_num=0;
		char *poutput=(char*)malloc(MAX+MAX);
		if(NULL!=poutput){
			if(NULL!=dir&&NULL!=context->client_sock){
				fp=fopen(dir,"r");
				if(NULL!=fp){
					r_num=fread(buf,sizeof(char),MAX,fp);
					while(r_num>0){
						substitute(buf, poutput, psrc, pdst);
						s_buf_num=strlen(poutput);
						s_num=context->client_sock->send(context->client_sock,poutput,s_buf_num,0);
						if(s_num<0){
							goto ftp_context_I_R_S_error3;
						}
						if_zero=s_buf_num-s_num;
						buf_finist=s_num;
						time_num=10;
						while(0!=if_zero){
							s_num=context->client_sock->send(context->client_sock,poutput+buf_finist,if_zero,0);
							if(s_num<0){
								goto ftp_context_I_R_S_error3;
							}
							if_zero=if_zero-s_num;
							buf_finist=buf_finist+s_num;
							time_num=time_num-1;
							if(0==time_num){
								break;
							}
						}
						if(0!=if_zero){
							goto ftp_context_I_R_S_error3;
						}
						memset(buf,0,sizeof(char)*MAX);
						r_num=fread(buf,sizeof(char),MAX,fp);
					}
					fclose(fp);
					fp=NULL;
					free(poutput);
					poutput=NULL;
					return 0;
				}
			}
			goto ftp_context_I_R_S_error2;
		}
		goto ftp_context_I_R_S_error1;
		ftp_context_I_R_S_error3:
			fclose(fp);
			fp=NULL;
		ftp_context_I_R_S_error2:
			free(poutput);
			poutput=NULL;
		ftp_context_I_R_S_error1:
		return -1;
}



static int ftp_context_recv_A_F_S(struct FTP_SERVER_DATA_SEND *const context,char *dir,char *flag){
		printf("ftp_context_recv_A_F_S_%d===========\n",__LINE__);
		char buf[MAX+2];
		char current_dir[200];
		char file_path[200];
		memset(buf,0,sizeof(char)*(MAX+2));
		char *psrc="\r\n";
		char *pdst="\n";
		FILE *fp=NULL;
		char *poutput=(char*)malloc(MAX+MAX);
		int ret =1 ;
		int ret2 = 0;
		int buflen = 0;

	
		if(NULL!=poutput && NULL != dir  && NULL != context->client_sock){
			memset(current_dir,0,200);
			getcwd(current_dir,200);
			snprintf(file_path,200,"%s/%s",current_dir,dir);	

			printf("file_path=%s****%d\n",file_path,__LINE__);

			fp = fopen(file_path,"wb+");
			if(NULL!=fp){
				while(ret){				
					buflen = context->client_sock->recv(context->client_sock,buf,MAX,MSG_DONTWAIT);//非阻塞模式
					if(buflen < 0){
						goto ftp_context_recv_A_F_S_error1;
					}
					else if(0 == buflen){							
					// 这里表示对端的socket已正常关闭.		
						printf("THE client sock is close,date aleady been send !\n");
						fclose(fp);
						fp = NULL;
						free(poutput);
						poutput = NULL;
						return 0;
					}

					substitute(buf, poutput, psrc, pdst);			
					ret2 = fwrite(poutput,sizeof(char),strlen(poutput),fp);	
								
					printf("buflen=%d****ret2=%d*********%d\n",buflen,ret2,__LINE__);
					
					if(ret2 != strlen(poutput)){
						printf("fwriting erro!\n");
						goto ftp_context_recv_A_F_S_error1;
					}	
				
					memset(buf,0,sizeof(char)*MAX);
												
				}			
			}
			else 
				goto ftp_context_recv_A_F_S_error2;
			}
		else 
			goto  ftp_context_recv_A_F_S_error2;
		
		
ftp_context_recv_A_F_S_error1:
							fclose(fp);
							fp = NULL;
							free(poutput);
							poutput = NULL;
							return -1;
ftp_context_recv_A_F_S_error2:
							free(poutput);
							poutput = NULL;
							return -1;
}



static int ftp_context_A_R_S(struct FTP_SERVER_DATA_SEND *const context,char *dir,char *flag){
		printf("ftp_context_A_R_S===\n");
		
		return 0;
}



static int ftp_context_recv_A_R_S(struct FTP_SERVER_DATA_SEND *const context,char *dir,char *flag){
		printf("ftp_context_recv_A_R_S===\n");
		
		return 0;
}


static int ftp_context_send_buf_IFS(struct FTP_SERVER_DATA_SEND *const context,char *buf,int size){
		printf("ftp_context_send_buf_IFS_%d===\n",__LINE__);
		char *pinput=(char*)malloc(MAX);
		char *pbuf=buf;
		int num=size;
		int s_num=0;
		int s_buf_num=0;
		int if_zero=0;
		int buf_finist=0;
		int time_num=0;
		if(NULL!=pinput){
			memset(pinput,0,MAX);
			if(NULL!=pbuf&&NULL!=context->client_sock&&(MAX>size)){
				while(num>0){
					memcpy(pinput,pbuf,num);
					pbuf=pbuf+MAX;
					num=num-MAX;
					s_buf_num=strlen(pinput);
					s_num=context->client_sock->send(context->client_sock,pinput,s_buf_num,0);
					if(s_num<0){
						goto ftp_context_send_buf_IFS_error2;
					}
					if_zero=s_buf_num-s_num;
					buf_finist=s_num;
					time_num=10;
					while(0!=if_zero){
						printf("if_zero==%d,%d\n",if_zero,__LINE__);
						s_num=context->client_sock->send(context->client_sock,pinput+buf_finist,if_zero,0);
						if(s_num<0){
							goto ftp_context_send_buf_IFS_error2;
						}
						if_zero=if_zero-s_num;
						buf_finist=buf_finist+s_num;
						time_num=time_num-1;
						if(0==time_num){
							break;
						}
					}
					if(0!=if_zero){
						goto ftp_context_send_buf_IFS_error2;
					}
					memset(pinput,0,sizeof(char)*MAX);
				}
				free(pinput);
				pinput=NULL;
				return 0;
				
			}
			goto ftp_context_send_buf_IFS_error2;
		}
		goto ftp_context_send_buf_IFS_error1;
		ftp_context_send_buf_IFS_error2:
			free(pinput);
			pinput=NULL;
		ftp_context_send_buf_IFS_error1:
		return -1;
}
static int ftp_context_send_buf_AFS(struct FTP_SERVER_DATA_SEND *const context,char *buf,int size){
		printf("ftp_context_send_buf_AFS_%d===\n",__LINE__);
		char *pinput=NULL;
		char *poutput=NULL;
		char *pbuf=buf;
		char *psrc="\n";
		char *pdst="\r\n";
		int num=size;
		int s_num=0;
		int s_buf_num=0;
		int if_zero=0;
		int buf_finist=0;
		int time_num=0;
		pinput=(char*)malloc(MAX);
		if(NULL!=pinput){
			memset(pinput,0,MAX);
			poutput=(char*)malloc(MAX+MAX);
			if(NULL!=poutput){
				memset(poutput,0,MAX+MAX);
				if(NULL!=pbuf&&NULL!=context->client_sock&&(MAX>size)){
					while(num>0){
						memcpy(pinput,pbuf,num);
						pbuf=pbuf+MAX;
						num=num-MAX;
						substitute(pinput, poutput, psrc, pdst);
						s_buf_num=strlen(poutput);
						s_num=context->client_sock->send(context->client_sock,poutput,s_buf_num,0);
						if(s_num<0){
							goto ftp_context_send_buf_AFS_error3;
						}
						if_zero=s_buf_num-s_num;
						buf_finist=s_num;
						time_num=10;
						while(0!=if_zero){
							s_num=context->client_sock->send(context->client_sock,poutput+buf_finist,if_zero,0);
							if(s_num<0){
								goto ftp_context_send_buf_AFS_error3;
							}
							if_zero=if_zero-s_num;
							buf_finist=buf_finist+s_num;
							time_num=time_num-1;
							if(0==time_num){
								break;
							}
						}
						if(0!=if_zero){
							goto ftp_context_send_buf_AFS_error3;
						}
						memset(pinput,0,sizeof(char)*MAX);
					}
					free(poutput);
					poutput=NULL;
					free(pinput);
					pinput=NULL;
					return 0;
				}
				goto ftp_context_send_buf_AFS_error3;
			}
			goto ftp_context_send_buf_AFS_error2;
		}
		goto ftp_context_send_buf_AFS_error1;
		ftp_context_send_buf_AFS_error3:
			free(poutput);
			poutput=NULL;
		ftp_context_send_buf_AFS_error2:
			free(pinput);
			pinput=NULL;
		ftp_context_send_buf_AFS_error1:
		return -1;

}


ftp_context_tpye_send_buf_server ftp_context_type_send_buf_test(LP_FTP_SERVER_DATA_SEND const context){
		printf("ftp_context_type_send_buf_test_%d\n",__LINE__);
		LP_FTP_SERV_COMMAND_DEFAULT_VECTOR type=context->context->command_default_sets->find(context->context->command_default_sets,"TYPE");
		char type_t[20];
		if(NULL==type){
			snprintf(type_t,sizeof(type_t),"%s","I");
		}
		snprintf(type_t,sizeof(type_t),"%s",type->command_val);
		LP_FTP_CONTEXT_TYPE_SEND_BUF_VECTOR service_vector=NULL;
		if(NULL!=context){
			service_vector=context->context->comtext_type_send_buf_sets->find(context->context->comtext_type_send_buf_sets,type_t);
			if(NULL!=service_vector){
				return service_vector->service;
			}
		}
		return NULL;
}




ftp_context_tpye_stru_mode_server ftp_contest_type_stru_mode_test(LP_FTP_SERVER_DATA_SEND const context,char *flag){
		LP_FTP_SERV_COMMAND_DEFAULT_VECTOR type=context->context->command_default_sets->find(context->context->command_default_sets,"TYPE");
		LP_FTP_SERV_COMMAND_DEFAULT_VECTOR stru=context->context->command_default_sets->find(context->context->command_default_sets,"STRU");
		LP_FTP_SERV_COMMAND_DEFAULT_VECTOR mode=context->context->command_default_sets->find(context->context->command_default_sets,"MODE");
		char type_stru_mode[20];
		char *test_flag = strdupa(flag);
		if(NULL==type||NULL==stru||NULL==mode){
			snprintf(type_stru_mode,sizeof(type_stru_mode)+sizeof(test_flag),"%s%s",test_flag,"IFS");
		}
		snprintf(type_stru_mode,sizeof(type_stru_mode)+sizeof(test_flag),"%s%s%s%s",test_flag,type->command_val,stru->command_val,mode->command_val);

		LP_FTP_CONTEXT_TPYE_STRU_MODE_VECTOR service_vector=NULL;
		if(NULL!=context){
			service_vector=context->context->context_type_stru_mode_sets->find(context->context->context_type_stru_mode_sets,type_stru_mode);
			if(NULL!=service_vector){
				return service_vector->service;
			}
			else return NULL;
		}
		return NULL;
}


 
 int ftp_context_databuf_send(LP_FTP_SERVER_DATA_SEND const context,char * buf,int size){
 		ftp_context_tpye_send_buf_server service=NULL;
		int ret=0;
		printf("ftp_context_databuf_send\n");
		service=ftp_context_type_send_buf_test(context);
		if(NULL!=service){
			ret=service(context,buf,size);
			return ret;
		}
		return -1;
 }
 int ftp_context_file_send(LP_FTP_SERVER_DATA_SEND const context, char * dir){
 		printf("ftp_context_file_send_%d\n",__LINE__);
 		ftp_context_tpye_stru_mode_server service=NULL;
		int ret=0;
		char flag[4];
		snprintf(flag,4,"%s","s");
		service=ftp_contest_type_stru_mode_test(context,flag);
		if(NULL!=service){
			ret=service(context,dir,flag);
			return ret;
		}
		return -1;
 }


 int ftp_context_file_recv(LP_FTP_SERVER_DATA_SEND const context, char *dir){
 		ftp_context_tpye_stru_mode_server service_recv = NULL;
		int ret=0;
		char flag[4];
		snprintf(flag,4,"%s","r");


		printf("test=%d\n",__LINE__);
		
	    service_recv = ftp_contest_type_stru_mode_test(context,flag);
		if(NULL!=service_recv){

			printf("test=%d\n",__LINE__);
			
			ret=service_recv(context,dir,flag);
			return ret;
		}
		return -1;
 }
 
static void * ftp_create_data_sock_listener_pthread(void *arg){
		int sock= -1;
		int client_sock= -1;
		int ret = 0;
		LP_FTP_SERVER_COMMAND_CONTEXT context = (LP_FTP_SERVER_COMMAND_CONTEXT)(arg);
		context->data_listener_trigger=0;
		context->data_ready_trigger=0;
		int reuse_on=1;
		struct sockaddr_in addr;
		struct sockaddr_in client_addr;
		pthread_detach(pthread_self()); // detach myself
		prctl(PR_SET_NAME, "ftp_create_data_sock_listener_pthread");
		socklen_t client_addr_len = sizeof(client_addr);
		stSOCKET_TCP sock_tcp;
		lpSOCKET_TCP tcp = socket_tcp2_r(client_sock, &sock_tcp);
		context->client_sock=tcp;
		//create socket
		sock= socket(AF_INET,SOCK_STREAM,0);
		if(sock < 0){
			goto ftp_data_listener_create_err1;
		}
		context->client_listen_sock=sock;
		ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_on, sizeof(reuse_on));	
		if(0!=ret){
			goto ftp_data_listener_create_err2;
		}
		struct timeval timeout={10,0};
		ret = setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(struct timeval));
		if(0!=ret){
			goto ftp_data_listener_create_err2;
		}
		//bind
		memset(&addr,0,sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr=htonl(INADDR_ANY);
		addr.sin_port = htons(context->data_port);
		ret = bind(sock,(struct sockaddr *)&addr ,sizeof(addr));
		if(0!=ret){
			perror("bind fail");
//			exit(1);
			goto ftp_data_listener_create_err2;
		}
		ret = listen(sock,2);
		context->client_listen_sock=sock;
		if(0!=ret){
			goto ftp_data_listener_create_err3;
		}
		printf("ftp_create_data_sock_listener_pthread_wait_accept_%d\n",__LINE__);
		context->data_listener_trigger=1;
		client_sock = accept(sock,(struct sockaddr*)&client_addr,&client_addr_len);
		if(client_sock<0){
			goto ftp_data_listener_create_err4;
		}
		context->data_ready_trigger=1;
		context->client_sock->sock=client_sock;
		printf("ftp_create_data_sock_listener_pthread_sucess_%d\n",__LINE__);
		
		usleep(100000);
		/*
		while(0==*context->data_finist){
			usleep(1000);
		}*/
			printf("ftp_create_data_sock_listener_pthread_stop_%d\n",__LINE__);
			pthread_exit(NULL);
		ftp_data_listener_create_err4:
			close(client_sock);
		ftp_data_listener_create_err3:
			context->data_listener_trigger=0;
		ftp_data_listener_create_err2:
			close(sock);
			sock = -1;
		ftp_data_listener_create_err1:
			printf("ftp_create_data_sock_listener_pthread_fail_%d\n",__LINE__);
//			context->data_listener_trigger=1;
//			free(sock_tcp);
			pthread_exit(NULL);
}
static int ftp_create_data_sock_listener(struct FTP_SERVER_COMMAND_CONTEXT *const command_context ){
	pthread_t data_tid=(pthread_t)NULL;
	int ret=-1;
	printf("ftp_create_data_sock_listener_%d\n",__LINE__);
	ret = pthread_create(&data_tid, NULL, ftp_create_data_sock_listener_pthread, (void *)command_context);
	if(0!=ret){
		printf("create fail_%d\n",__LINE__);
		//
	}
	printf("ret=%d,%d\n",ret,__LINE__);
	return ret;
}
static int ftp_create_sock_listener(in_port_t port)
{
	int ret = 0;
	int sock = -1;
	int reuse_on = 1;
	struct sockaddr_in addr;

	// create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		goto ftp_sock_create_err1;
	}

	// port reuse active
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_on, sizeof(reuse_on));
	if(0 != ret){
		goto ftp_sock_create_err2;
	}

	// bind
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	ret = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
	if(0 != ret){
		goto ftp_sock_create_err2;
	}

	// listen
	ret = listen(sock, 5);
	if(0 != ret){
		goto ftp_sock_create_err2;
	}

	// success
	return sock;

ftp_sock_create_err2:
	close(sock);
	sock = -1;

ftp_sock_create_err1:
	return -1;
}
static ftp_command_server ftp_serv_test_command(LP_FTP_SERVER_COMMAND_CONTEXT ftp_serv,const char* command,const char*command_para,lpSOCKET_TCP tcp){
		const char* cmd_530 = "530 not logged in\r\n";
		const char* cmd_331 = "331 user name okay,need password\r\n";
		const char* cmd_550 = "550 synax error,command unrecognized\r\n";
		LP_FTP_SERVER_ATTR attr = ftp_serv->serv_attr;
		LP_FTP_SERV_COMMAND_VECTOR command_vector = attr->command_sets->find(attr->command_sets,command);
		if(NULL!=command_vector){
			if(0==strcmp(command_vector->command,"USER")){
				LP_FTP_SERV_USER_VECTOR user_vector=attr->user_sets->find(attr->user_sets,command_para);
				if(NULL!=user_vector){
					ST_FTP_SERV_COMMAND_DEFAULT_VECTOR command_default_vector;
					if(!ftp_serv->command_default_sets){
						ftp_serv->command_default_sets=FTRIE_create(sizeof(ST_FTP_SERV_COMMAND_DEFAULT_VECTOR));
							}
					snprintf(command_default_vector.command,sizeof(command_default_vector.command),"%s",command);
					snprintf(command_default_vector.command_val,sizeof(command_default_vector.command_val),"%s",command_para);
					ftp_serv->command_default_sets->add(ftp_serv->command_default_sets,command_default_vector.command,&command_default_vector);					
					return command_vector->service;
				}
					tcp->send(tcp,cmd_331,strlen(cmd_331),0);
					return NULL;
			}
			if(0==strcmp(command_vector->command,"PASS")){
				LP_FTP_SERV_COMMAND_DEFAULT_VECTOR command_default_vector_user=NULL;
				command_default_vector_user=ftp_serv->command_default_sets->find(ftp_serv->command_default_sets,"USER");
				if(NULL!=command_default_vector_user){
					LP_FTP_SERV_USER_VECTOR user_verctor=attr->user_sets->find(attr->user_sets,command_default_vector_user->command_val);
					if(NULL==command_para){
						tcp->send(tcp,cmd_331,strlen(cmd_331),0);
						return NULL;
					}
					if(0==strcmp(user_verctor->password,command_para)){
						if(!ftp_serv->command_default_sets){
							ftp_serv->command_default_sets=FTRIE_create(sizeof(ST_FTP_SERV_COMMAND_DEFAULT_VECTOR));
						}
						ST_FTP_SERV_COMMAND_DEFAULT_VECTOR command_default_vector_user_add;
						snprintf(command_default_vector_user_add.command,sizeof(command_default_vector_user_add),"%s",command);
						snprintf(command_default_vector_user_add.command_val,sizeof(command_default_vector_user_add),"%s",command_para);
						ftp_serv->command_default_sets->add(ftp_serv->command_default_sets,command_default_vector_user_add.command,&command_default_vector_user_add);
						return command_vector->service;
					}
					tcp->send(tcp,cmd_331,strlen(cmd_331),0);
					return NULL;
				}
			}
			if(NULL==ftp_serv->command_default_sets->find(ftp_serv->command_default_sets,"USER")||NULL==ftp_serv->command_default_sets->find(ftp_serv->command_default_sets,"PASS")){
				LP_FTP_SERV_COMMAND_DEFAULT_VECTOR anonymous_vector=ftp_serv->command_default_sets->find(ftp_serv->command_default_sets,"USER");
				if(0!=strcmp("anonymous",anonymous_vector->command_val)){
					tcp->send(tcp,cmd_530,strlen(cmd_530),0);
					return NULL;
				}
			}
			LP_FTP_SERV_COMMAND_DEFAULT_VECTOR command_default_vector_permission=NULL;
			command_default_vector_permission=ftp_serv->command_default_sets->find(ftp_serv->command_default_sets,"USER");
			LP_FTP_SERV_USER_VECTOR user_verctor=attr->user_sets->find(attr->user_sets,command_default_vector_permission->command_val);		
			char permission_user[20]={""};
			snprintf(permission_user,sizeof(permission_user),"%s",user_verctor->permission);
			char permission_cmd[20]={""};
			snprintf(permission_cmd,sizeof(permission_cmd),"%s",command_vector->permission);
			if(0==strcmp("2",permission_user)){
				if(0==strcmp("0",permission_cmd)||0==strcmp("1",permission_cmd)){
					tcp->send(tcp,cmd_530,strlen(cmd_530),0);
					return NULL;
				}
			}
			if(0==strcmp("1",permission_user)){
				if(0==strcmp("0",permission_cmd)){
					tcp->send(tcp,cmd_530,strlen(cmd_530),0);
					return NULL;
				}
			}
			ST_FTP_SERV_COMMAND_DEFAULT_VECTOR command_default_vector;
			if(!ftp_serv->command_default_sets){
				ftp_serv->command_default_sets=FTRIE_create(sizeof(ST_FTP_SERV_COMMAND_DEFAULT_VECTOR));
			}
			snprintf(command_default_vector.command,sizeof(command_default_vector.command),"%s",command);
			snprintf(command_default_vector.command_val,sizeof(command_default_vector.command_val),"%s",command_para);
			ftp_serv->command_default_sets->add(ftp_serv->command_default_sets,command_default_vector.command,&command_default_vector);
			return command_vector->service;
		}
		tcp->send(tcp,cmd_550,strlen(cmd_550),0);
		return NULL;
}

static int ftp_command_handle(lpSOCKET_TCP tcp,LP_FTP_SERVER_COMMAND_CONTEXT context, const char *cmd)
{
	ftp_command_server command_service = NULL;
	char *command_para = NULL;
	char *command_f = NULL;
	char *command = NULL;
	if(0==strcmp(cmd,"")){
		return -1;
	}
	char *buf = strdupa(cmd);
	char *token = NULL;
	command_f= strtok_r(buf," ",&token);
	if(NULL!=command_f){
		command_para=strtok_r(NULL,"\r\n",&token);
	}
	command= strtok_r(buf,"\r\n",&token);
	if(NULL!=(command_service = ftp_serv_test_command(context,command,command_para,tcp))){
		return command_service(context,command_para,tcp);
	}
	return 0; // if 0 server to continue, else to quit
}

static void *ftp_serv_command(void *arg)
{
	int ret = 0;
	char cmd_buf[512] = {""};
	LP_FTP_SERVER_COMMAND_CONTEXT command_context = (LP_FTP_SERVER_COMMAND_CONTEXT)(arg);
	LP_FTP_SERVER_ATTR const serv_attr = command_context->serv_attr;
	stSOCKET_TCP sock_tcp;
	lpSOCKET_TCP tcp = socket_tcp2_r(command_context->sock, &sock_tcp);
	const char *cmd_220 = "220 Operation successful\r\n";

	
	// detach myself
	pthread_detach(pthread_self()); 
	prctl(PR_SET_NAME, "ftp_serv_command");

	// push 220 to announce firstly
	ret = tcp->send(tcp, cmd_220, strlen(cmd_220), 0);

	if(strlen(cmd_220) == ret){
		while(serv_attr->listener_tid){
			ret = tcp->recv(tcp, cmd_buf, sizeof(cmd_buf), 0);
			usleep(10000);
			printf("cmd_buf::%s,%d\n",cmd_buf,__LINE__);
			if(ret < 0){
				perror("recv");
				// FIXME:
			}else{
				cmd_buf[ret] = '\0'; // end symbol
				
				if(ftp_command_handle(tcp,command_context, cmd_buf) < 0){
					printf("break_%d\n",__LINE__);
					break;
				}
			}
		}
	}
	
	FTRIE_release(command_context->command_default_sets);
	command_context->command_default_sets=NULL;
	FTRIE_release(command_context->comtext_type_send_buf_sets);
	command_context->comtext_type_send_buf_sets=NULL;
	FTRIE_release(command_context->context_type_stru_mode_sets);
	command_context->context_type_stru_mode_sets=NULL;
	close(command_context->sock);
	free(command_context);
	command_context = NULL; // very important
	pthread_exit(NULL);
}


static LP_FTP_SERVER_COMMAND_CONTEXT FTP_SERV_COMMAND_CONTEXT_create(LP_FTP_SERVER_ATTR const serv_attr,int client_sock){
		LP_FTP_SERVER_COMMAND_CONTEXT command_context = NULL;
		command_context = calloc(sizeof(ST_FTP_SERVER_COMMAND_CONTEXT), 1);
		if(NULL!=command_context&&NULL!=serv_attr){
		command_context->sock = client_sock;
		command_context->client_listen_sock =0;
		command_context->client_sock =0;
		command_context->serv_attr = serv_attr;
		command_context->data_listener_trigger=1;
		command_context->data_ready_trigger=0;
		command_context->data_port= -1;

		
		ST_FTP_SERV_COMMAND_DEFAULT_VECTOR command_defualt_vector;
		command_context->ftp_create_data_sock_listener= ftp_create_data_sock_listener;
		command_context->ftp_stop_data_sock_listener=ftp_stop_data_sock_listener;
		command_context->ftp_context_type_stru_mode_add=ftp_serv_context_tpye_str_mode_add;
		command_context->ftp_context_type_send_buf_add=ftp_serv_context_type_send_buf_add;
		command_context->command_default_sets=FTRIE_create(sizeof(ST_FTP_SERV_COMMAND_DEFAULT_VECTOR));
		command_context->context_type_stru_mode_sets=FTRIE_create(sizeof(ST_FTP_CONTEXT_TPYE_STRU_MODE_VECTOR));
		command_context->comtext_type_send_buf_sets=FTRIE_create(sizeof(ST_FTP_CONTEXT_TYPE_SEND_BUF_VECTOR));

		command_context->ftp_context_type_stru_mode_add(command_context,"sIFS",ftp_context_I_F_S);			
		command_context->ftp_context_type_stru_mode_add(command_context,"sIRS",ftp_context_I_R_S);
		command_context->ftp_context_type_stru_mode_add(command_context,"sAFS",ftp_context_A_F_S);
		command_context->ftp_context_type_stru_mode_add(command_context,"sARS",ftp_context_A_R_S);


		command_context->ftp_context_type_stru_mode_add(command_context,"rIFS",ftp_context_recv_I_F_S);			
    	command_context->ftp_context_type_stru_mode_add(command_context,"rIRS",ftp_context_recv_I_R_S);
		command_context->ftp_context_type_stru_mode_add(command_context,"rAFS",ftp_context_recv_A_F_S);
		command_context->ftp_context_type_stru_mode_add(command_context,"rARS",ftp_context_recv_A_R_S);	


		command_context->ftp_context_type_send_buf_add(command_context,"I",ftp_context_send_buf_IFS);
		command_context->ftp_context_type_send_buf_add(command_context,"A",ftp_context_send_buf_AFS);
		if(NULL!=command_context->command_default_sets){
				snprintf(command_defualt_vector.command,sizeof(command_defualt_vector.command),"%s","TYPE");
				snprintf(command_defualt_vector.command_val,sizeof(command_defualt_vector.command_val),"%s","I");
				if(-1==command_context->command_default_sets->add(command_context->command_default_sets,command_defualt_vector.command,&command_defualt_vector)){
					goto FTP_SERV_command_context_create_err1;
				}
				snprintf(command_defualt_vector.command,sizeof(command_defualt_vector.command),"%s","STRU");
				snprintf(command_defualt_vector.command_val,sizeof(command_defualt_vector.command_val),"%s","F");
				if(-1==command_context->command_default_sets->add(command_context->command_default_sets,command_defualt_vector.command,&command_defualt_vector)){
					goto FTP_SERV_command_context_create_err1;
				}
				snprintf(command_defualt_vector.command,sizeof(command_defualt_vector.command),"%s","MODE");
				snprintf(command_defualt_vector.command_val,sizeof(command_defualt_vector.command_val),"%s","S");
				if(-1==command_context->command_default_sets->add(command_context->command_default_sets,command_defualt_vector.command,&command_defualt_vector)){
					goto FTP_SERV_command_context_create_err1;
				}
				return command_context;
				
			}
			FTP_SERV_command_context_create_err1:
				printf("fail::FTP_SERV_COMMAND_CONTEXT_create\n");
				FTRIE_release(command_context->command_default_sets);
				FTRIE_release(command_context->comtext_type_send_buf_sets);
				FTRIE_release(command_context->context_type_stru_mode_sets);
				command_context->command_default_sets=NULL;
				free(command_context);
				command_context=NULL;
				return NULL;
		}
			
		return NULL;
}

static void *ftp_serv_listener(void *arg)
{
	int ret = 0;
	LP_FTP_SERVER_ATTR const serv_attr = (LP_FTP_SERVER_ATTR)(arg);
	prctl(PR_SET_NAME, "ftp_serv_listener");
	while(serv_attr->listener_trigger){
		fd_set read_fds;
		struct timeval poll_wait;
		FD_ZERO(&read_fds);
		FD_SET(serv_attr->sock, &read_fds);
		poll_wait.tv_sec = 1;
		poll_wait.tv_usec = 0;
		ret = select(serv_attr->sock + 1, &read_fds, NULL, NULL, &poll_wait);		
		if(ret > 0){
			if(FD_ISSET(serv_attr->sock, &read_fds)){
				struct sockaddr_in client_addr;
				socklen_t client_addr_len = sizeof(client_addr);
				int client_sock = accept(serv_attr->sock, (struct sockaddr*)&client_addr, &client_addr_len);
				if(client_sock < 0){
					perror("accept");
					// FIXME:
				}else{
					pthread_t command_tid = (pthread_t)NULL;
					LP_FTP_SERVER_COMMAND_CONTEXT command_context = FTP_SERV_COMMAND_CONTEXT_create(serv_attr, client_sock);				
					printf("Accpet %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
					ret = pthread_create(&command_tid, NULL, ftp_serv_command, (void *)command_context);
					if(0 != ret){
						perror("pthread create");
						FTP_SERV_COMMAND_CONTEXT_release(command_context);
						// FIXME:
					}
				}
			}
		}
	}
	// joinable or not?!
	pthread_exit(NULL);
}
LP_FTP_SERVER FTP_SERV_create(in_port_t port, const char folder[1024])
{
	int ret = 0;
	int ret2 = -1;
	LP_FTP_SERVER const ftp_serv = calloc(sizeof(ST_FTP_SERVER) + sizeof(ST_FTP_SERVER_ATTR), 1);
	if(NULL==ftp_serv){
		goto end;
	}
	
    char *int_dir = strdupa(folder);
    printf("int_dir=%s\n",int_dir);

    if(0 !=( ret2 = chdir(int_dir))){
  	   printf("The int_dir is not right !\n");
	   return NULL;
    }
	
	LP_FTP_SERVER_ATTR const serv_attr = (LP_FTP_SERVER_ATTR)(ftp_serv + 1);
	
	ftp_serv->forbidden_zero = 0;
	serv_attr->sock = -1;
	serv_attr->listen_port = port;

	serv_attr->listener_trigger = false;
	serv_attr->listener_tid = (pthread_t)NULL;
	serv_attr->command_sets = NULL;
	serv_attr->user_sets =NULL;
	//command operations
	ftp_serv->add_command=ftp_serv_add_command;
	ftp_serv->del_command=ftp_serv_del_command;

	//user operations
	ftp_serv->add_user=ftp_serv_add_user;
	ftp_serv->del_user=ftp_serv_del_user;
	// 1. create socket for command session
	serv_attr->sock =  ftp_create_sock_listener(serv_attr->listen_port);
	if(serv_attr->sock < 0){
		goto FTP_SERV_init_err1;
	}

	// 2. create a thread to listen command
	serv_attr->listener_trigger = true;
	ret = pthread_create(&serv_attr->listener_tid, NULL, ftp_serv_listener, (void *)serv_attr);
	if(0 != ret){
		perror("pthread_create");
		goto FTP_SERV_init_err2;
	}

	// success!!
	return ftp_serv;

FTP_SERV_init_err2:
	close(serv_attr->sock);
	serv_attr->sock = -1;

FTP_SERV_init_err1:
	free(ftp_serv);
	// failed!
	end:
	return NULL;
}

void FTP_SERV_release(LP_FTP_SERVER ftp_serv)
{
	if(ftp_serv){
		LP_FTP_SERVER_ATTR attr=(LP_FTP_SERVER_ATTR)(ftp_serv+1);
		FTRIE_release(attr->command_sets);
		attr->command_sets=NULL;
		FTRIE_release(attr->user_sets);
		attr->user_sets=NULL;
		close(attr->sock);
		if(attr->listener_tid){
			attr->listener_trigger = false;
			pthread_join(attr->listener_tid, NULL);
			attr->listener_tid = (pthread_t)NULL;
		}
		ftp_serv->forbidden_zero=-1;
		// release all the resource
	}
}
void FTP_SERV_COMMAND_CONTEXT_release(LP_FTP_SERVER_COMMAND_CONTEXT  command_context){
	if(command_context){
		close(command_context->sock);
		FTRIE_release(command_context->command_default_sets);
		command_context->command_default_sets=NULL;
		FTRIE_release(command_context->comtext_type_send_buf_sets);
		command_context->comtext_type_send_buf_sets=NULL;
		FTRIE_release(command_context->context_type_stru_mode_sets);
		command_context->context_type_stru_mode_sets=NULL;
		command_context->serv_attr=NULL;
		free(command_context);
		command_context=NULL;
	}
}
#if 0
int main(int argc, char *argv[])
{
	LP_FTP_SERVER ftpd = FTP_SERV_create(10021, "./");
	if(ftpd){
		getchar();
		FTP_SERV_release(ftpd);
		ftpd = NULL;
	}
	return 0;
}
#endif

