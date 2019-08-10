#include "ftp_server_cmd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <sys/prctl.h>

typedef struct FTP_dir_list{
     char tmp_list_buf[200];
	 struct FTP_dir_list *next;
}stFTP_dir_list,*lpFTP_dir_list;


char rename_file_path[200];



int FTP_cmd_user(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp){
	printf("ftp_cmd_user================\n");	
	const char *cmd_230 = "230 Operation successful\r\n";
	const char *cmd_502 = "502 Command not implemented\r\n ";
	const char *cmd_331 = "331 User name okay,need password\r\n";
	
	if(NULL == cmd_para){
		printf("the cmd_para is NULL!\n");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
	}	

	char *user = strdupa(cmd_para);
	if(0 == strcasecmp(user,"anonymous")){
		tcp->send(tcp, cmd_230, strlen(cmd_230), 0);
		return 0;
	}
	else
		tcp->send(tcp, cmd_331, strlen(cmd_331), 0);
		return 0;
}



int FTP_cmd_type(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp)
{
	printf("ftp_cmd_type================\n");

	const char *cmd_200 = "200 Operation successful\r\n";
	const char *cmd_502 = "502 Command not implemented.\r\n ";
	

	if(NULL == cmd_para){
		printf("the cmd_para is NULL!\n");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
	}	
	char *buf = strdupa(cmd_para);
	char *token = NULL;
	char *type_name= NULL;

    printf("para:%s\n",cmd_para);
	type_name = strtok_r(buf," ",&token);
	printf("typename:%s\n",type_name);

	if(0 == strcasecmp("A",type_name) || 0 == strcasecmp("I",type_name)){
		printf("the type is ok\n");
		tcp->send(tcp,cmd_200,strlen(cmd_200),0);	
		usleep(1000);
		return 0;
	}
	else		
		printf("type erro!\n");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		usleep(1000);
		return 0;

}

int FTP_cmd_smnt(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp)
{
//fixme
	printf("ftp_cmd_smnt================\n");
	const char *cmd_230 = "220 Operation successful\r\n";
	const char *cmd_502 = "502 Command not implemented.\r\n ";
	struct stat buf;

	if(NULL == cmd_para){
		printf("the cmd_para is NULL!\n");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
	}
	char *dir_path = strdupa(cmd_para);
	lstat(dir_path,&buf);
	if(S_ISDIR(buf.st_mode)){
		printf("smnt command is no problem\n");
		tcp->send(tcp, cmd_230, strlen(cmd_230), 0);
		return 0;
	}
	else
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
}



int FTP_cmd_mkd(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp)
{

	printf("ftp_cmd_mkd================\n");
    const char *cmd_257 = "257 \"PATHNAME\"created\r\n";
	const char *cmd_502 = "502 Command not implemented\r\n";
	int ret;
	if(NULL == cmd_para){
		printf("the cmd_para  is NULL!\n ");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
	}

	char *dir_path = strdupa(cmd_para);
	ret = mkdir(dir_path,754);

	if(0 == ret ){
		printf("Dir Creat !\n");
		tcp->send(tcp,cmd_257,strlen(cmd_257),0);
		return 0;
	}
	else 
		printf("make dir erro!\n");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;	
}	


int FTP_cmd_rmd(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp)
{
	printf("ftp_cmd_rmd================\n");

	const char *cmd_250 = "250  Requested file action okay,completed\r\n";
	const char *cmd_502 = "502 Command not implemented\r\n";
	int ret;

	if(NULL == cmd_para){
		printf("the cmd_para  is NULL!\n ");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
	}
	
	char *dir_path =strdupa(cmd_para);
	ret = remove(dir_path);

	if(0 == ret){
		printf("dir deleted!\n");
		tcp->send(tcp,cmd_250,strlen(cmd_250),0);
		return 0;
	}
	else
		printf("unable to delete the dir! \n");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
}


int FTP_cmd_rein(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp)
{
	printf("FTP_cmd_rein===============\r\n");
	
	const char *cmd_200 = "200 Operation successful\r\n";
	const char *cmd_502 = "502 Command not implemented\r\n";
	
	while(1 == command_context->data_ready_trigger){
		usleep(1000);
	}
	if(0 == command_context->data_ready_trigger){	
		command_context->command_default_sets->empty(command_context->command_default_sets);	
		printf("reined,input the username\n");
		tcp->send(tcp,cmd_200,strlen(cmd_200),0);
		return 0;
	}
	else 
		printf("something erro!\n");
	    tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
	
}


int FTP_cmd_quit(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp)
{
	printf("FTP_cmd_quit===============\r\n");
	
	const char *cmd_200 = "200 Operation successful\r\n";
	const char *cmd_502 = "502 Command not implemented\r\n";
	
	while(1 == command_context->data_ready_trigger){
		usleep(1000);
	}
	if(0 == command_context->data_ready_trigger){	
		command_context->command_default_sets->empty(command_context->command_default_sets);			
		tcp->send(tcp,cmd_200,strlen(cmd_200),0);
		usleep(1000);
		tcp->close(tcp);
		printf("cmd_quit is implemented!\n");
		return -1;
	}	
	else 
		printf("cmd quit not implemented!\n");
    	tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
}





int FTP_cmd_port(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp)
{//fixme  or not
	printf("ftp_cmd_port==============\n");

    const char *cmd_200 = "200 Command okay\r\n";
	const char *cmd_502 = "502 Command not implemented\r\n";

	if(NULL == cmd_para ){
		printf("The cmd_para is NULL!");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
	}
 	printf("cmd_para :%s\n",cmd_para);

	//get addr and port from cmd_para
	//  PORT h1,h2,h3,h4,p1,p2
	char *token = NULL;
	char *tmp_buf[5] ;
	int m,n1,n2,ret,ret2;
	char ip_buf[32];
	struct sockaddr_in addr;
	struct sockaddr_in client_addr;
	int sock = -1;
	int reuse_on = 1;

	
	char *str =  strdupa(cmd_para);
	
	tmp_buf[0] = strtok_r(str,",",&token);

	for(m = 1;m < 5; m++){
		tmp_buf[m] = strtok_r(NULL,",",&token);
		if(4 == m ){
			tmp_buf[5] = strtok_r(NULL,"\r\n",&token);
		}
	}
	sprintf(ip_buf,"%s.%s.%s.%s",tmp_buf[0],tmp_buf[1],tmp_buf[2],tmp_buf[3]);
	printf("ip_buf:%s\n",ip_buf);
	
	n1 = atoi(tmp_buf[4]);
	n2 = atoi(tmp_buf[5]);
	
	in_port_t port = n1 *256 + n2;

	printf("The port :%d\n",port);
	
//creat sock then connect!
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		goto ftp_sock_create_err1;
	}
	// port reuse active
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_on, sizeof(reuse_on));
	if(0 != ret){
		goto ftp_sock_create_err2;
	}
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	addr.sin_port = htons(20);
	ret = bind(sock, (struct sockaddr *) &addr, sizeof(addr));        //   sock bind

	
	if(0 != ret){
		goto ftp_sock_create_err2;
	}

	//192.168.2.15      1025        client   
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(port);
	inet_aton(ip_buf,&client_addr.sin_addr.s_addr);



// something wrong !test it     
	usleep(5000);

	ret2 = connect(sock,(const struct sockaddr *)&client_addr,sizeof(client_addr));
	
	printf("ret2:%d\n",ret2);
	
	if(0 != ret2){
		printf("connect fault!\n");
	    goto ftp_sock_create_err1;
	}
	tcp->send(tcp,cmd_200,strlen(cmd_200),0);
	
	command_context->data_ready_trigger=1;
	command_context->data_listener_trigger = 0;
	
	send(sock,"hello,test data,sock\n",sizeof("hello,test data,sock\n"),0);
	return 0;


ftp_sock_create_err2:
	close(sock);
	sock = -1;
ftp_sock_create_err1:
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
}




static lpFTP_dir_list  FTP_cmd_list_buf(LP_FTP_SERVER_COMMAND_CONTEXT command_context, const char * cmd_para)
{
	
	const char *cmd_r_n="\r\n";
	char currentDirPath[200];
	char dir_path[200];	
	char file_dir_path[200];
	
	lpFTP_dir_list pre_dir_list = NULL;
	lpFTP_dir_list head_list = NULL;

	
	int ret =0 ;
	int ret1 = 0;
	memset(currentDirPath,0,sizeof(currentDirPath));
	memset(dir_path,0,sizeof(dir_path));
	getcwd(currentDirPath,sizeof(currentDirPath));
   	struct stat buf;	
	DIR *dirptr = NULL;
	struct dirent* dir_entry;
	if(NULL==cmd_para){
		snprintf(dir_path,sizeof(dir_path),"%s",currentDirPath);
	}
	else {
		snprintf(dir_path,sizeof(dir_path),"%s",cmd_para);
	}

	
	lstat(dir_path,&buf);
	chdir(dir_path);

	//the cmd_para of dirpath is dir
	
	if(S_ISDIR(buf.st_mode)){
		dirptr = opendir(dir_path);
		if(NULL == dirptr ){
			printf("open dir erro!\n");	
			return NULL;
		}		
		if(NULL != dirptr){
			while(NULL != (dir_entry = readdir(dirptr))){
				if(strcmp(dir_entry->d_name, "..")==0 || strcmp(dir_entry->d_name, ".")==0){		
					continue;
				}		
				snprintf(file_dir_path,200,"%s/%s",currentDirPath,dir_entry->d_name);	

	
				lpFTP_dir_list dir_list = (lpFTP_dir_list)calloc(sizeof(stFTP_dir_list),1);
				
				if(NULL == dir_list){
					return NULL;		
				}
															
				struct stat file_buf;
				
				ret = lstat(file_dir_path,&file_buf); 
				if(0 != ret){
					printf("stat erro!\n");
					closedir(dirptr);
					
					free(pre_dir_list);
	          		pre_dir_list = NULL;
					free(dir_list);
					dir_list = NULL;
					
					return NULL;
				}

// get the mode 
				int abc ;
				char mode[20] = "";
				
				abc = file_buf.st_mode & S_IFDIR;
				if(abc == S_IFDIR){
					strcat(mode,"d");
				}
				if(abc != S_IFDIR){
					strcat(mode,"-");
				}

					/////user   1_1
				abc = file_buf.st_mode & S_IRUSR;
				if(abc == S_IRUSR){
					strcat(mode,"r");
				}
				else
					strcat(mode,"-");

				
				//user   1_2
				abc = file_buf.st_mode & S_IWUSR;
				if(abc == S_IWUSR){
					strcat(mode,"w");
				}
				else
					strcat(mode,"-");


                //user    1-3
                abc = file_buf.st_mode & S_IXUSR;
				if(abc == S_IXUSR){
					strcat(mode,"x");
				}
				else
					strcat(mode,"-");

				

				//groop    1-1
				abc = file_buf.st_mode & S_IRGRP;
				if(abc == S_IWGRP){
					strcat(mode,"r");
				}
				else
					strcat(mode,"-");

				//grp    1-2
				abc = file_buf.st_mode & S_IWGRP;
				if(abc == S_IWGRP){
					strcat(mode,"w");
				}
				else
					strcat(mode,"-");


				//grp   1-3
				abc = file_buf.st_mode & S_IXGRP;
				if(abc == S_IXGRP){
					strcat(mode,"x");
				}
				else
					strcat(mode,"-");

				


				//other  1-1
				abc = file_buf.st_mode & S_IROTH;
				if(abc == S_IROTH){
					strcat(mode,"r");
				}
				else
					strcat(mode,"-");
				//other  1-2
				abc = file_buf.st_mode & S_IWOTH;
				if(abc == S_IWOTH){
					strcat(mode,"w");
				}
				else
					strcat(mode,"-");
				//other  1-3
				abc = file_buf.st_mode & S_IXOTH;
				if(abc == S_IXOTH){
					strcat(mode,"x");
				}
				else
					strcat(mode,"-");


//file_buf.st_nlink
				char  nlink[10];
				char a_link[10];
				snprintf(a_link,20,"%d",file_buf.st_nlink);
				int nlink_len = strlen(a_link);
				int n = 4-nlink_len;
				snprintf(nlink,n+1,"%s","        ");
				strcat(nlink,a_link);

//file_buf.st_uid
				char uid[10];
				char a_uid[10];
				snprintf(a_uid,10,"%d",file_buf.st_uid);
				int uid_len = strlen(a_uid);
				int m = 8-uid_len;
				strcpy(uid,a_uid);
				strncat(uid,"        ",m);
								
//file_buf.st_gid
				char gid[20];
				char a_gid[20];
				snprintf(a_gid,20,"%d",file_buf.st_gid);
				int gid_len = strlen(a_gid);
				int j = 8 - gid_len;
				strcpy(gid,a_gid);
				strncat(gid,"        ",j);

//st_size
				char a_size[10];
				char a_size_buf[10];
				snprintf(a_size_buf,10,"%d",file_buf.st_size);
				int size_len = strlen(a_size_buf);
				snprintf(a_size,8-size_len+1,"%s","           ");
				strcat(a_size,a_size_buf);

//st_time ------ thu jan 1 00:00:14 1970
                char *ctime(const time_t *clock);
			    char *p_time = NULL;
			    p_time = ctime(&file_buf.st_mtime);
				char *token = NULL;
				char *week = strtok_r(p_time," ",&token);
				
				char *month =strtok_r(NULL," ",&token);
				char *date = strtok_r(NULL," ",&token);
				char *min_sec_time = strtok_r(NULL," ",&token);
				char *year = strtok_r(NULL," ",&token);
				
	
				int file_year = atoi(year);

		
				char hour_min[8];
				snprintf(hour_min,6,"%s",min_sec_time);


			//  current year    
	            struct tm tblock ={0} ;
				time_t timer = time(NULL);
			    localtime_r(&timer,&tblock);	
				int current_year= tblock.tm_year+1900 ;//         2013      

			    char tmp_time[100];
				if( file_year == current_year){
					if(1 == strlen(date)){
						snprintf(tmp_time,100,"%s  %s %s",month,date,hour_min);
						}
					else
						snprintf(tmp_time,100,"%s %s %s",month,date,hour_min);				   
			     }
				if(file_year != current_year){
					if(1 == strlen(date)){
						snprintf(tmp_time,strlen(month)+strlen(date)+4+1+4,"%s  %s  %s",month,date,year);
					}
					else 
						snprintf(tmp_time,strlen(month)+strlen(date)+4+1+3,"%s %s  %s",month,date,year);
				}

				memset(dir_list->tmp_list_buf,0,200);
				ret1 = snprintf(dir_list->tmp_list_buf,100,"%s %s %s %s %s %s %s%s",mode,nlink,uid,gid,a_size,tmp_time,dir_entry->d_name,cmd_r_n);	
				if(ret1  < 0){

					free(dir_list);
					dir_list = NULL;

					return NULL;
				}
			
				dir_list->next = NULL;	
				

				
				if(NULL != pre_dir_list){
					pre_dir_list->next = dir_list;
				}
				
				if(NULL == head_list){
					head_list = dir_list;									
				}	

				if(NULL != head_list){
					pre_dir_list = dir_list;
				}
				
				
				}
			}
		
		closedir(dirptr);
		return  head_list;
	
		}


//the cmd_para is file
	if(S_ISREG(buf.st_mode)){	


		lpFTP_dir_list dir_list = (lpFTP_dir_list)calloc(sizeof(stFTP_dir_list),1);
				
		if(NULL == dir_list){
				return NULL;		
		}
	
		snprintf(dir_list->tmp_list_buf,200,"st_dev:%s\r\n st_size:%s \r\n st_mode:%s \r\n st_ino:%s \r\n st_rdev:%d \r\ nst_nlink:%d \r\n st_uid:%d \r\n st_gid:%d \r\n st_atime:%s \r\n st_mtime:%s \r\n st_ctime:%s \r\n st_blksize:%d \r\nst_blksize:%d \r\n st_blocks:%d \r\n ,buf.st_blocks",
		buf.st_dev,buf.st_size,buf.st_mode,buf.st_ino,buf.st_rdev,buf.st_nlink,buf.st_uid,buf.st_gid,ctime(buf.st_atime),ctime(buf.st_mtime),ctime(buf.st_ctime),buf.st_blksize);	
		dir_list->next = NULL;
		
		return dir_list;
	}

	else 
		return NULL;
}
	


int FTP_cmd_pass(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp)
{
	printf("ftp_cmd_pass==============\n");
	const char *cmd_230 = "230 User logged in, proceed\r\n";
	tcp->send(tcp, cmd_230, strlen(cmd_230), 0);	
	return 0;
	
}


int FTP_cmd_cwd(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char *cmd_para,lpSOCKET_TCP tcp)
{
	//if exist ,turn to the path ,if not exist ,quit

	printf("ftp_cmd_cwd\n");
	
	char *cmd_250 = "250 Operation successful\r\n";
	const char *cmd_502 = "502 Command not implemented\r\n";

	if(NULL == cmd_para){
		printf("The cmd_para is NULL!\n");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
	}
	char *dir_path = strdupa(cmd_para);	
	
//	if(NULL == strstr(dir_path,"/media")){
//		tcp->send(tcp,cmd_250,strlen(cmd_250),0);
//		return 0;
//	}


	struct stat buf;
	lstat(dir_path,&buf);	
	if(NULL != dir_path && S_ISDIR(buf.st_mode)){
		chdir(dir_path);
		tcp->send(tcp,cmd_250,strlen(cmd_250),0);
		return 0;
	}
	else	
		printf("Command not implemented!\n");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
}




int FTP_cmd_cdup(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp)
{
	//goback to the father dir
	printf("ftp_cmd_cdup\n");
	
	
	char currentDirPath[200];
	char *fatherDirPath = NULL;

	int len,i;
	char *cmd_250 = "250 Operation successful\r\n";
	const char *cmd_502 = "502 Command not implemented\r\n";

	memset(currentDirPath,0,sizeof(currentDirPath));
	getcwd(currentDirPath,sizeof(currentDirPath));
	
	printf("dir_path:%s\n",currentDirPath);
	
	if(NULL == currentDirPath){
	    printf("directory absoultly path is null!\n");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
	}
	
	//	if(0 == strcasecmp("/media",currentDirPath)){
	//		tcp->send(tcp,cmd_250,strlen(cmd_250),0);
	//		return 0;	
	//	}
	

	
	len = strlen(currentDirPath);	
	for(i = len-1;i >= 0;i--){
		if('/' == currentDirPath[i]){
			currentDirPath[i] = '\0';
			fatherDirPath = strndupa(currentDirPath,i);
			if(NULL == fatherDirPath){
				printf("Can't cdup!\n");
				tcp->send(tcp,cmd_502,strlen(cmd_502),0);
				return 0;
			}			
			chdir(fatherDirPath);
			printf("father_path:%s\n",fatherDirPath);
			tcp->send(tcp,cmd_250,strlen(cmd_250),0);
			return 0;		
		}		
	}
	printf("it can't cdup!");
	tcp->send(tcp,cmd_502,strlen(cmd_502),0);
	return 0;
}

int FTP_cmd_pwd(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp)
{
   //  get the path ,read the name,print  the name ! 
    printf("ftp_cmd_pwd\n");   
   
	char currentDirPath[200];
	char buf[300];

	const char *cmd_257 ="257 ";
	const char *cmd_502 = "502 Command not implemented\r\n";
	const char *cmd_r_n="\r\n";

	memset(currentDirPath,0,sizeof(currentDirPath));
	getcwd(currentDirPath,sizeof(currentDirPath));

	if(NULL == currentDirPath){
		printf("directory absoultly path is null!\n");
		tcp->send(tcp,cmd_502,strlen(cmd_502),0);
		return 0;
	}
	snprintf(buf,strlen(cmd_257)+strlen(currentDirPath)+2+strlen(cmd_r_n),"%s\"%s\"%s",cmd_257,currentDirPath,cmd_r_n);
	printf("buf=%s\n",buf);
	tcp->send(tcp,buf,strlen(buf),0);
	return 0;
}


/////////////////////////////////////////////////////////////////////////////


static void * FTP_cmd_list_f(void* arg){
	printf("FTP_cmd_list_%d===============\n",__LINE__);
	pthread_detach(pthread_self()); // detach myself
	prctl(PR_SET_NAME, "FTP_cmd_list_f");
	const char *cmd_450="450 requested file action not taken\r\n";
	const char *cmd_150_0="150 Directory listing \r\n";
	const char *cmd_150_1="150 Document Information listing \r\n";
	const char *cmd_226="226 Operation successful \r\n";

	int ret=0;
	LP_FTP_COMMAND_SERVER_PTHREAD command_pthread=(LP_FTP_COMMAND_SERVER_PTHREAD)(arg);
	LP_FTP_SERVER_COMMAND_CONTEXT command_context=command_pthread->command_context;
	lpSOCKET_TCP tcp=command_pthread->tcp;
	char *cmd_para=NULL;
	char cmd_para_f[50];
	if(NULL!=command_pthread->command_para){
		snprintf(cmd_para_f,sizeof(cmd_para_f),"%s",command_pthread->command_para);
		cmd_para=cmd_para_f;
	}
	
	if(1!=command_context->data_ready_trigger){
		tcp->send(tcp,cmd_450,strlen(cmd_450),0);
		if(1==command_context->data_listener_trigger){
			close(command_pthread->command_context->client_listen_sock);
		}
		goto end_file1;
	}
	
	LP_FTP_SERVER_DATA_SEND command_context_send=calloc(sizeof(ST_FTP_SERVER_DATA_CONTEXT),1);
	stSOCKET_TCP sock_tcp;
	command_context_send->client_sock= socket_tcp2_r(command_context->client_sock->sock, &sock_tcp);
	command_context_send->context=command_context;
	command_context_send->client_listen_sock=command_pthread->client_listen_sock;
	command_pthread->create_trigger=1;



	
	
	lpFTP_dir_list lpheadlist=FTP_cmd_list_buf(command_context_send->context, cmd_para);
	lpFTP_dir_list lpcurlist=lpheadlist;

	tcp->send(tcp,cmd_150_0,strlen(cmd_150_0),0);
	usleep(1000);
	while(NULL!=lpcurlist){
		if(NULL!=lpcurlist->tmp_list_buf){
			ret=ftp_context_databuf_send(command_context_send, lpcurlist->tmp_list_buf, strlen(lpcurlist->tmp_list_buf));
			if(-1==ret){
				goto error1;
			}
		}
		lpcurlist=lpcurlist->next;
	}
	tcp->send(tcp,cmd_226,strlen(cmd_226),0);
	goto end_file2;
	error1:
		tcp->send(tcp,cmd_450,strlen(cmd_450),0);
		
	end_file2:
		usleep(1000);
		close(command_context_send->client_sock->sock);
		close(command_context_send->client_listen_sock);
		free(command_context_send);
		lpcurlist=lpheadlist;
		while(NULL!=lpcurlist){
			lpheadlist=lpcurlist->next;
			free(lpcurlist);
			lpcurlist=lpheadlist;
		}
	end_file1:
		pthread_exit(NULL);
}


int FTP_cmd_pasv(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp){
	printf("ftp_cmd_pasv_f============\n");
	const char  *cmd_227 = "227 PASV ok ";
	const char *cmd_450 ="450 requested file action not taken\r\n";
	const char *cmd_r_n="\r\n";
	char buf[64];
	char tmd_buf[64];
	struct sockaddr_in addr;
	int length = sizeof addr;
	in_port_t port=1024;
	int ret =-1,m;
	char*  s_b[4];
	command_context->data_listener_trigger=0;
	for(port=1023;0==command_context->data_listener_trigger;port++){
		command_context->data_port=port+1;
		ret=command_context->ftp_create_data_sock_listener(command_context);
		usleep(1000);
	}
	printf("potr::%d,%d\n",port,__LINE__);
	printf("data_ready_trigger:%d\n",command_context->data_ready_trigger);
	
	if(0==ret&&1==command_context->data_listener_trigger){
		printf("command_context->data_listener_create sucess,%d\n",__LINE__);			
		int port_1 = port/256;
		int	port_2 = port%256;
		
		getsockname(tcp->sock,(struct sockaddr *)&addr,&length);
		sprintf(tmd_buf,"%s",inet_ntoa(addr.sin_addr));
	

		char *taken = NULL;
		s_b[0] = strdupa(strtok_r(tmd_buf,".",&taken));
   		if(s_b[0] != NULL){
			for(m = 1;m< 3;m++){
				s_b[m] = strdup(strtok_r(NULL,".",&taken));
			}
    		s_b[3] = strdup(strtok_r(NULL,"\r\n",&taken));
		}
		
		sprintf(buf,"%s(%s,%s,%s,%s,%u,%u)%s",cmd_227,s_b[0],s_b[1],s_b[2],s_b[3],port_1,port_2,cmd_r_n);
		tcp->send(tcp,buf,strlen(buf),0);
  		free(s_b[1]);
		free(s_b[2]);
		free(s_b[3]);
	}
	else {
		perror("data_listen create fail\n");
		tcp->send(tcp,cmd_450,strlen(cmd_450),0);
	}
	return 0;
}





static void * FTP_cmd_retr_f(void *arg){
	printf("FTP_cmd_retr_f_%d===============\n",__LINE__);
	pthread_detach(pthread_self()); // detach myself
	prctl(PR_SET_NAME, "FTP_cmd_retr_f");
	LP_FTP_COMMAND_SERVER_PTHREAD command_pthread=(LP_FTP_COMMAND_SERVER_PTHREAD)(arg);
	LP_FTP_SERVER_COMMAND_CONTEXT command_context=command_pthread->command_context;
	lpSOCKET_TCP tcp=command_pthread->tcp;
	char cmd_para[50];
	snprintf(cmd_para,sizeof(cmd_para),"%s",command_pthread->command_para);
	command_pthread->create_trigger=1;
	const char *cmd_450="450 requested file action not taken\r\n";
	const char *cmd_226= "226 operation successful\r\n";
	const char *cmd_225="225 data connection open;no transfer starting.\r\n";
	const char *cmd_f="150 Opening BINARY connection for ";
	const char *cmd_m="(";
	const char *cmd_e=" bytes)\r\n";
	char cmd_150[200];
	char file_name[200];
	memset(file_name,0,sizeof(file_name));
	memset(cmd_150,0,sizeof(cmd_150));
   	struct stat file_buf;
	int ret=0;
	
	if(1!=command_pthread->data_ready_trigger){
		printf("command_pthread->data_ready_trigger=%d,%d\n",command_pthread->data_ready_trigger,__LINE__);
		tcp->send(tcp,cmd_450,strlen(cmd_450),0);
		if(1==command_pthread->data_listener_trigger){
			close(command_pthread->client_listen_sock);
		}
		pthread_exit(NULL);
	}

	LP_FTP_SERVER_DATA_SEND command_context_send=calloc(sizeof(ST_FTP_SERVER_DATA_SEND),1);
	stSOCKET_TCP sock_tcp;
	command_context_send->client_sock= socket_tcp2_r(command_context->client_sock->sock, &sock_tcp);
	command_context_send->context=command_context;
	command_context_send->client_listen_sock=command_context->client_listen_sock;	
	if(NULL!=cmd_para){
		snprintf(file_name,sizeof(file_name),"%s",cmd_para);
		lstat(file_name,&file_buf);
			if(S_ISREG(file_buf.st_mode)){
				snprintf(cmd_150,sizeof(cmd_150),"%s%s%s%d%s",cmd_f,cmd_para,cmd_m,file_buf.st_size,cmd_e);
				tcp->send(tcp,cmd_150,strlen(cmd_150),0);
				ret=ftp_context_file_send(command_context_send, cmd_para);
				if(0==ret){
					tcp->send(tcp,cmd_226,strlen(cmd_226),0);
					goto stop_listener;
				}
				else {
					tcp->send(tcp,cmd_225,strlen(cmd_225),0);
					goto stop_listener;
				}
			}
	}
	tcp->send(tcp,cmd_450,strlen(cmd_450),0);
	stop_listener:
		usleep(1000);
		close(command_context_send->client_sock->sock);
		close(command_context_send->client_listen_sock);
		free(command_context_send);
		pthread_exit(NULL);
 }
 int FTP_cmd_retr(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp){
 	pthread_t retr_tid=(pthread_t)NULL;
	int ret =0;
	const char *cmd_450="450 requested file action not taken\r\n";
	if(NULL==cmd_para){
		goto FTP_cmd_retr_f_n_error1;
	}
	usleep(1000);
	LP_FTP_COMMAND_SERVER_PTHREAD COMMAND_CONTEXT_pthread =NULL;
	COMMAND_CONTEXT_pthread =calloc(sizeof(ST_FTP_COMMAND_SERVER_PTHREAD),1);
	if(NULL==COMMAND_CONTEXT_pthread){
		goto FTP_cmd_retr_f_n_error1;
	}
	COMMAND_CONTEXT_pthread->command_context=COMMAND_CONTEXT;
	COMMAND_CONTEXT_pthread->command_para=cmd_para;
	COMMAND_CONTEXT_pthread->tcp=tcp;
	COMMAND_CONTEXT_pthread->create_trigger=0;
	COMMAND_CONTEXT_pthread->data_listener_trigger=COMMAND_CONTEXT->data_listener_trigger;
	COMMAND_CONTEXT_pthread->data_ready_trigger=COMMAND_CONTEXT->data_ready_trigger;
	COMMAND_CONTEXT_pthread->client_listen_sock=COMMAND_CONTEXT->client_listen_sock;
	ret = pthread_create(&retr_tid, NULL, FTP_cmd_retr_f, (void *)COMMAND_CONTEXT_pthread);
	if(0!=ret){
		goto FTP_cmd_retr_f_n_error2;
	}
	while(1!=COMMAND_CONTEXT_pthread->create_trigger){
		usleep(10);
	}
	goto end;
	FTP_cmd_retr_f_n_error2:
		free(COMMAND_CONTEXT_pthread);
		COMMAND_CONTEXT_pthread=NULL;
	FTP_cmd_retr_f_n_error1:
		if(1==COMMAND_CONTEXT_pthread->data_ready_trigger&&1==COMMAND_CONTEXT_pthread->client_listen_sock){
			close(COMMAND_CONTEXT_pthread->client_sock->sock);
			close(COMMAND_CONTEXT_pthread->client_listen_sock);
		}
		else if(1==COMMAND_CONTEXT_pthread->client_listen_sock){
			close(COMMAND_CONTEXT_pthread->client_listen_sock);
		}
	tcp->send(tcp,cmd_450,strlen(cmd_450),0);
	end:
	free(COMMAND_CONTEXT_pthread);
	COMMAND_CONTEXT_pthread=NULL;
	return 0;
 }
 int FTP_cmd_list(LP_FTP_SERVER_COMMAND_CONTEXT command_context, const char * cmd_para, lpSOCKET_TCP tcp){
	pthread_t list_tid=(pthread_t)NULL;
	const char *cmd_450="450 requested file action not taken\r\n";
	int ret =0;
	usleep(1000);
	LP_FTP_COMMAND_SERVER_PTHREAD command_context_pthread=NULL;
	command_context_pthread=calloc(sizeof(ST_FTP_COMMAND_SERVER_PTHREAD),1);
	if(NULL==command_context_pthread){
		goto FTP_cmd_list_f_n_error1;
	}
	stSOCKET_TCP sock_tcp;
	command_context_pthread->client_sock=socket_tcp2_r(command_context->client_sock->sock, &sock_tcp);
	command_context_pthread->client_listen_sock=command_context->client_listen_sock;
	command_context_pthread->command_context=command_context;
	command_context_pthread->tcp=tcp;
	command_context_pthread->create_trigger=0;
	command_context_pthread->data_listener_trigger=command_context->data_ready_trigger;
	command_context_pthread->data_ready_trigger=command_context->data_ready_trigger;
	command_context_pthread->command_para=cmd_para;
	ret = pthread_create(&list_tid,NULL,FTP_cmd_list_f,(void*)command_context_pthread);
	if(0!=ret){
		goto FTP_cmd_list_f_n_error2;
	}
	while(1!=command_context_pthread->create_trigger){
		usleep(10);
	}
	goto end;
	FTP_cmd_list_f_n_error2:
		free(command_context_pthread);
		command_context_pthread=NULL;
	FTP_cmd_list_f_n_error1:
		if(1==command_context_pthread->data_ready_trigger&&1==command_context_pthread->data_listener_trigger){
			close(command_context_pthread->client_sock->sock);
			close(command_context_pthread->client_listen_sock);
		}
		else if(1==command_context->data_listener_trigger){
			close(command_context_pthread->client_listen_sock);
		}
	tcp->send(tcp,cmd_450,strlen(cmd_450),0);
	return 0;
	end:
	free(command_context_pthread);
	command_context_pthread=NULL;
	return 0;
 }
int FTP_cmd_dele(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp)
{
	const char *cmd_226= "226 Operation successful\r\n";
	const char *cmd_450="450 requested file action not taken\r\n";
	int ret;

	if(NULL == cmd_para ){
		printf("The cmd_para is NULL!\n");
		tcp->send(tcp,cmd_450,strlen(cmd_450),0);
		return 0;
	}

	char *file_path = strdupa(cmd_para);

	ret = remove(file_path);

	if(0 == ret){
	printf("file has been deleted!\n");
	tcp->send(tcp,cmd_226,strlen(cmd_226),0);
	return 0;
    }
    else
	printf("unable to delete the dir! \n");
	tcp->send(tcp,cmd_450,strlen(cmd_450),0);
	return 0;	
}





static void * FTP_cmd_stor_f(void *arg){
	printf("FTP_cmd_stor_f_%d===============\n",__LINE__);
	pthread_detach(pthread_self()); // detach myself
	prctl(PR_SET_NAME, "FTP_cmd_stor_f");
	LP_FTP_COMMAND_SERVER_PTHREAD command_pthread=(LP_FTP_COMMAND_SERVER_PTHREAD)(arg);
	LP_FTP_SERVER_COMMAND_CONTEXT command_context=command_pthread->command_context;
	lpSOCKET_TCP tcp=command_pthread->tcp;
	char cmd_para[50];
	snprintf(cmd_para,sizeof(cmd_para),"%s",command_pthread->command_para);
	command_pthread->create_trigger=1;
	const char *cmd_450="450 requested file action not taken\r\n";
	const char *cmd_226= "226 operation successful\r\n";
	const char *cmd_225="225 data connection open;no transfer starting.\r\n";
	const char *cmd_150="150 Ok to send data \r\n";
	
	char file_name[200];
	memset(file_name,0,sizeof(file_name));

	if(1!=command_pthread->data_ready_trigger){
		printf("command_pthread->data_ready_trigger=%d,%d\n",command_pthread->data_ready_trigger,__LINE__);
		tcp->send(tcp,cmd_450,strlen(cmd_450),0);
		if(1==command_pthread->data_listener_trigger){
			close(command_pthread->client_listen_sock);
		}
		pthread_exit(NULL);
	}
	
	LP_FTP_SERVER_DATA_SEND command_context_recv = calloc(sizeof(ST_FTP_SERVER_DATA_SEND),1);
	stSOCKET_TCP sock_tcp;

	command_context_recv->client_sock= socket_tcp2_r(command_context->client_sock->sock, &sock_tcp);
	command_context_recv->context=command_context;
	command_context_recv->client_listen_sock=command_context->client_listen_sock;	



	int ret=0;
	
	if(NULL!=cmd_para){
		snprintf(file_name,sizeof(file_name),"%s",cmd_para);
		tcp->send(tcp,cmd_150,strlen(cmd_150),0);
		ret = ftp_context_file_recv(command_context_recv, file_name);
		
		if(0==ret){
			tcp->send(tcp,cmd_226,strlen(cmd_226),0);
			goto stop_listener;
		}
		else {
			tcp->send(tcp,cmd_225,strlen(cmd_225),0);
			goto stop_listener;
		}	
	}
		
		tcp->send(tcp,cmd_450,strlen(cmd_450),0);


stop_listener:
			usleep(10000);
			close(command_context_recv->client_sock->sock);
			close(command_context_recv->client_listen_sock);
			free(command_context_recv);
			pthread_exit(NULL);
 }


int FTP_cmd_stor(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp){
 	pthread_t retr_tid=(pthread_t)NULL;
	int ret =0;
	const char *cmd_450="450 requested file action not taken\r\n";
	if(NULL==cmd_para){
		goto FTP_cmd_retr_f_n_error1;
	}
	usleep(1000);
	LP_FTP_COMMAND_SERVER_PTHREAD COMMAND_CONTEXT_pthread =NULL;
	COMMAND_CONTEXT_pthread =calloc(sizeof(ST_FTP_COMMAND_SERVER_PTHREAD),1);
	if(NULL==COMMAND_CONTEXT_pthread){
		goto FTP_cmd_retr_f_n_error1;
	}
	
	COMMAND_CONTEXT_pthread->command_context=COMMAND_CONTEXT;
	COMMAND_CONTEXT_pthread->command_para=cmd_para;
	COMMAND_CONTEXT_pthread->tcp=tcp;
	COMMAND_CONTEXT_pthread->create_trigger=0;
	COMMAND_CONTEXT_pthread->data_listener_trigger=COMMAND_CONTEXT->data_listener_trigger;
	COMMAND_CONTEXT_pthread->data_ready_trigger=COMMAND_CONTEXT->data_ready_trigger;
	COMMAND_CONTEXT_pthread->client_listen_sock=COMMAND_CONTEXT->client_listen_sock;
	ret = pthread_create(&retr_tid, NULL, FTP_cmd_stor_f, (void *)COMMAND_CONTEXT_pthread);

	if(0!=ret){
		goto FTP_cmd_retr_f_n_error2;
	}
	while(1!=COMMAND_CONTEXT_pthread->create_trigger){
		usleep(10);
	}
	goto end;

FTP_cmd_retr_f_n_error2:
	free(COMMAND_CONTEXT_pthread);
	COMMAND_CONTEXT_pthread=NULL;

FTP_cmd_retr_f_n_error1:
	if(1==COMMAND_CONTEXT_pthread->data_ready_trigger&&1==COMMAND_CONTEXT_pthread->client_listen_sock){
		close(COMMAND_CONTEXT_pthread->client_sock->sock);
		close(COMMAND_CONTEXT_pthread->client_listen_sock);
	}
	else if(1==COMMAND_CONTEXT_pthread->client_listen_sock){
		close(COMMAND_CONTEXT_pthread->client_listen_sock);
	}


	tcp->send(tcp,cmd_450,strlen(cmd_450),0);
end:
	free(COMMAND_CONTEXT_pthread);
	COMMAND_CONTEXT_pthread=NULL;
	return 0;
}


int FTP_cmd_rnfr(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char *cmd_para,lpSOCKET_TCP tcp){
	printf("FTP_cmd_rnfr_%d===============\n",__LINE__);

	const char *cmd_350 = "350 Operation successful\r\n";
	char current_dir[200];
	
	memset(current_dir,0,sizeof(current_dir));	
	getcwd(current_dir,sizeof(current_dir));

	char *old_name = strdupa(cmd_para);

	snprintf(rename_file_path,200,"%s/%s",current_dir,old_name);
	printf("file_path=%s***%d\n",rename_file_path,__LINE__);


	tcp->send(tcp,cmd_350,strlen(cmd_350),0);
	return 0;
}



int FTP_cmd_rnto(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char *cmd_para,lpSOCKET_TCP tcp){
	
	printf("FTP_cmd_rnto_%d===============\n",__LINE__);

	const char *cmd_250 ="250 Operation successful\r\n";

	char *current_dir[200];
	memset(current_dir,0,sizeof(current_dir));	
	getcwd(current_dir,sizeof(current_dir));
	
	char  new_file_path[200];
	char *new_name = strdupa(cmd_para);
	
	snprintf(new_file_path,200,"%s/%s",current_dir,new_name);
	rename(rename_file_path,new_file_path);
	
	printf("reneme!%d\n",__LINE__);

	tcp->send(tcp,cmd_250,strlen(cmd_250),0);
	return 0;

}

/*
int FTP_cmd_syst(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char *cmd_para,lpSOCKET_TCP tcp){
//this cmd need the cmd_feat,can fix it.

	printf("FTP_cmd_syst_%d===============\n",__LINE__);

	char *command_s = "uname -s";
	char *command_r = "uname -r";
	const char *cmd_215 = "215 ";
	const char *cmd_r_n ="\r\n";
	const char *cmd_502 = "502 Command not implemented\r\n";
	char cmd_buf[100];


 	FILE *fp;
	FILE *fp2;
	char sys_name[100];
	char sys_type[100];
	char *pname;
	char *ptype;
	char *token =NULL;
	char *token2= NULL;

	fp = popen(command_s,"r");
	fp2 = popen(command_r,"r");
	
	if(NULL == fp || NULL == fp2){
		printf("popen the command erro!\n");
		tcp->send(tcp,cmd_502,sizeof(cmd_502),0);
		return 0;
	}
	
   	fgets(sys_name,sizeof(sys_name),fp);
	pname = strtok_r(sys_name,"\r\n",&token);
	
	fgets(sys_type,sizeof(sys_type),fp2);
	ptype = strtok_r(sys_type,"\r\n",&token);

	snprintf(cmd_buf,100,"%s %s Type: %s%s",cmd_215,pname,ptype,cmd_r_n);
	printf("cmd_buf=%s\n",cmd_buf);


	pclose(fp);
	pclose(fp2);
	
	tcp->send(tcp,cmd_buf,sizeof(cmd_buf),0);
	return 0;
}

*/

