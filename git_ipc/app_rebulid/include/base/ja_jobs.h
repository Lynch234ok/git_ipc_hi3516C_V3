/******************************************************************************

  Copyright (C), 2014-, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : ja_job.h
  Version       : Initial Draft
  Author        : wengson@sina.com(wenson)
  Created       : 2015/8/4
  Last Modified : 2015/8/4
  Description   : juan jobs management

  History       :
  1.Date        : 2015/8/4
    	Author      : wenson
 	Modification: Created file
******************************************************************************/

#ifndef __JA_JOBS_H__
#define __JA_JOBS_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "ja_type.h"

/*
 * JOB MANAGEMENT APIs
 */

typedef int (*fJA_JOB_HANDLE)(void *context, JA_UINT32 data, void *user_data);
typedef void (*fJA_JOB_DESTROY_HANDLE)(JA_UINT32 data);

typedef struct JA_JOB_MGNT
{
    int dummy;
}stJA_JOB_MGNT, *lpJA_JOB_MGNT;

lpJA_JOB_MGNT JA_JOB_MGNT_new(const char *name, JA_UINT32 maxQueue, JA_UINT32 maxThreds, JA_UINT32 stackSize,
                              fJA_JOB_HANDLE handler, fJA_JOB_DESTROY_HANDLE fDestroy, void *user_data, int userDataSize);
void JA_JOB_MGNT_del(lpJA_JOB_MGNT jobs);

int JA_JOB_MGNT_start(lpJA_JOB_MGNT jobs);
int JA_JOB_MGNT_stop(lpJA_JOB_MGNT jobs, JA_BOOL block);
int JA_JOB_MGNT_stop2(lpJA_JOB_MGNT jobs, JA_BOOL block, JA_BOOL dropTask);

int JA_JOB_MGNT_init(const char *name, JA_UINT32 maxQueue, JA_UINT32 maxThreds,
                         fJA_JOB_HANDLE handler, void *user_data, int userDataSize);
void JA_JOB_MGNT_deinit(void);

void JA_JOB_MGNT_set_hook(lpJA_JOB_MGNT jobs, fJA_JOB_HANDLE handler, void *user_data, int userDataSize);

void JA_JOB_MGNT_set_contex(lpJA_JOB_MGNT jobs, void *contexGrp, int grpSize);
int JA_JOB_MGNT_push_job(lpJA_JOB_MGNT jobs, JA_UINT32 data);
int JA_JOB_MGNT_pop_job(lpJA_JOB_MGNT jobs, JA_UINT32 data);
int JA_JOB_MGNT_reset(lpJA_JOB_MGNT jobs);
/*
* data macth rule: the data buffer specific by pointer
* this buffer first four bytes value match to data
* for example, push data is a struct as: 
* typedef struct {
*   Object *obj;
*   int type;
*  }Arg_t;
* the mach data is obj pointer , not the Arg_t pointer
*/
int JA_JOB_MGNT_pop_job2(lpJA_JOB_MGNT jobs, JA_UINT32 data);


/*
* process utils
*/
int JA_PROCESS_init();
int JA_PROCESS_push(const char *cmdline);
void JA_PROCESS_deinit();


/*
* main Loop APIs

*/

typedef struct JA_MLOOP
{
	int dummy;
}stJA_MLOOP, *lpJA_MLOOP;

typedef struct JA_CONTEXT
{
    int dummy;
}stJA_CONTEXT, *lpJA_CONTEXT;

typedef struct JA_CTX_SOURCE
{
    int dummy;
}stJA_CTX_SOURCE, *lpJA_CTX_SOURCE;

lpJA_MLOOP JA_MLOOP_new(const char *name);
void JA_MLOOP_del(lpJA_MLOOP mloop);

lpJA_MLOOP JA_ACCEPT_MLOOP_new(const char *name, JA_UINT32 maxQueue, JA_UINT32 maxThreds,
                               fJA_JOB_HANDLE handler, void *user_data, int userDataSize);

int JA_MLOOP_start(lpJA_MLOOP mloop);
int JA_MLOOP_stop(lpJA_MLOOP mloop);

lpJA_CONTEXT JA_CONTEXT_new(const char *name);
void JA_CONTEXT_del(lpJA_CONTEXT ctx);

typedef void (*fJA_CONTEXT_HOOK)(void *data, void *user_data);

int JA_CONTEXT_add_timer_source(lpJA_CONTEXT ctx, JA_UINT32 time_ms, fJA_CONTEXT_HOOK hook, void *data, void *user_data);
int JA_CONTEXT_add_read_source(lpJA_CONTEXT ctx, JA_UINT32 fd, fJA_CONTEXT_HOOK hook, void *data, void *user_data );
int JA_CONTEXT_add_write_source(lpJA_CONTEXT ctx, JA_UINT32 fd, fJA_CONTEXT_HOOK hook, void *data, void *user_data );


#ifdef __cplusplus
}
#endif


#endif //__JA_JOBS_H__

