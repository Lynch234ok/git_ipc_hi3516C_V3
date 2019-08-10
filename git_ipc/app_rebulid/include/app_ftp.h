#include "ftpserver/ftp_server.h"
#include "ftpserver/ftp_server_cmd.h"

#ifndef APP_FTP_H_
#define APP_FTP_H_
#ifdef __cplusplus
extern "C" {
#endif

extern int APP_FTP_init(int port, const char *folder);
extern void APP_FTP_destroy();
extern int APP_add_ftp(const char * command, const char * permission,ftp_command_server service);
extern int APP_del_ftp(const char * command);
extern int APP_add_ftp_user(const char * username, const char * password, const char * permission);
extern int APP_del_ftp_user(const char * username);
#ifdef __cplusplus
};
#endif
#endif //APP_FTP_H_

