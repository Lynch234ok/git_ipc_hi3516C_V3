#ifndef  _FTP_SERVER_CMD_H_
#define _FTP_SERVER_CMD_H_
#include "ftp_server.h"
extern int FTP_cmd_user(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_pass(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_mkd(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_rmd(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_cwd(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_cdup(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_quit(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_smnt(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_port(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_rein(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_type(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_pwd(LP_FTP_SERVER_COMMAND_CONTEXT command_context, const char * cmd_para, lpSOCKET_TCP tcp);
extern int FTP_cmd_put(LP_FTP_SERVER_COMMAND_CONTEXT command_context, const char * cmd_para, lpSOCKET_TCP tcp);
extern int FTP_cmd_get(LP_FTP_SERVER_COMMAND_CONTEXT command_context, const char * cmd_para, lpSOCKET_TCP tcp);
extern int  FTP_cmd_stor(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_pasv(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_retr(LP_FTP_SERVER_COMMAND_CONTEXT COMMAND_CONTEXT,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_list(LP_FTP_SERVER_COMMAND_CONTEXT command_context, const char * cmd_para, lpSOCKET_TCP tcp);
extern int FTP_cmd_dele(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char * cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_rnfr(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char *cmd_para,lpSOCKET_TCP tcp);
extern int FTP_cmd_rnto(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char *cmd_para,lpSOCKET_TCP tcp);

//extern int FTP_cmd_syst(LP_FTP_SERVER_COMMAND_CONTEXT command_context,const char *cmd_para,lpSOCKET_TCP tcp); //work with the command of FEAT.




#endif
