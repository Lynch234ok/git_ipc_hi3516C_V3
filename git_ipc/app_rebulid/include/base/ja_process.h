/******************************************************************************

  Copyright (C), 2014-, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : ja_process.h
  Version       : Initial Draft
  Author        : wengson@sina.com(wenson)
  Created       : 2015/9/25
  Last Modified : 2015/9/25
  Description   : juan process API

  History       :
  1.Date        : 2015/9/25
    	Author      : wenson
 	Modification: Created file
******************************************************************************/

#ifndef __NK_PROCESS_H__
#define __NK_PROCESS_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "ja_type.h"
//#include "stdinc.h"

#ifdef WIN32
typedef unsigned long	NK_PID_t;
#else
typedef pid_t	NK_PID_t;
#endif

/*
0 find failed, else find success
*/
NK_PID_t NK_PROCESS_find_by_name(const char* task_name);
/*
* 0 find success,  else find failed
*/
int NK_PROCESS_find_by_pid(NK_PID_t pid, char *task_name);

void NK_PROCESS_kill_by_name(const char *task_name, int waitdone);
void NK_PROCESS_kill_by_pid(NK_PID_t pid, int waitdone);
void NK_PROCESS_Wait(int id);
void NK_PROCESS_wait_done(NK_PID_t pid, int *status);


int NK_PROCESS_init(void);
int NK_PROCESS_push(const char *cmdline);
int NK_PROCESS_push2(const char *cmdline, int blocked);
void NK_PROCESS_deinit(void);

#define NK_SYSTEM(cmdline)\
	do{\
		const char* bname = basename(strdupa(__FILE__));\
		int const syntax_fg = ((bname[0] >> 8) + (bname[0] & 0xff)) % 8;\
		printf("\033[1;%dm[%12s:%4d]\033[0m ", 30 + syntax_fg, bname, __LINE__);\
		printf("system cmd: %s", cmdline);\
		printf("\r\n");\
		NK_PROCESS_push2(cmdline, 1);\
	}while(0)


#ifdef __cplusplus
}
#endif

#endif


