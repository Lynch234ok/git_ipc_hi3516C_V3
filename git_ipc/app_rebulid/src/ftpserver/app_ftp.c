#include "app_ftp.h"
static LP_FTP_SERVER  ftp_serv = NULL;
int APP_FTP_init(int port, const char *folder)
{
	if(0==port||NULL==folder){
		return -1;
	}
	if(NULL==ftp_serv){
		ftp_serv = FTP_SERV_create(port, folder);
	}
	return 0;
}
int APP_add_ftp(const char*command,const char * permission,ftp_command_server service){
	if(NULL!=ftp_serv){
		return ftp_serv->add_command(ftp_serv,command,permission,service);
	}
	return -1;
}
int APP_del_ftp(const char * command){
	if(NULL!=command){
		return ftp_serv->del_command(ftp_serv,command);
		}
	return -1;
}

int APP_add_ftp_user(const char* username, const char *password,const char *permission){
	if(NULL!=username&&NULL!=password&&NULL!=permission){
		return ftp_serv->add_user(ftp_serv,username,password,permission);
	}
	return -1;
}
int APP_del_ftp_user(const char* username){
	if(NULL!=username){
		return ftp_serv->del_user(ftp_serv,username);
	}
	return -1;
}
void APP_FTP_destroy()
{
	if(NULL != ftp_serv){
		FTP_SERV_release(ftp_serv);
		return 0;
		}
	return -1;
}

