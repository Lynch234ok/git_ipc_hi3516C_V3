/*
*	IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
*
*	By downloading, copying, installing or using the software you agree to this license.
*	If you do not agree to this license, do not download, install,
*	Copy or use the software.
*
*	Copyright (C) 2012, JUAN, Co, All Rights Reserved.
*
*	Project Name:MediaBuf
*	File Name:
*
*	Writed by Frank Law at 2013 - 05 - 09 JUAN,Guangzhou,Guangdong,China
*/

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#ifndef RWLOCK_H_
#define RWLOCK_H_
#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(4)
typedef struct _RW_LOCK
{
	// public interfaces
	int (*rdlock)(struct _RW_LOCK *thiz);
	int (*tryrdlock)(struct _RW_LOCK *thiz);
	int (*wrlock)(struct _RW_LOCK *thiz);
	int (*trywrlock)(struct _RW_LOCK *thiz);
	int (*unlock)(struct _RW_LOCK *thiz);
	int32_t (*getstatus)(struct _RW_LOCK *thiz);
	
}ST_RW_LOCK,*LP_RW_LOCK;
#pragma pack()

extern LP_RW_LOCK RWLOCK_create();
extern void RWLOCK_release(LP_RW_LOCK rwlock);

#define RWLOCK_RDLOCK(ptr_lock) (ptr_lock->rdlock(ptr_lock))
#define RWLOCK_TRYRDLOCK(ptr_lock) (ptr_lock->rdlock(ptr_lock))
#define RWLOCK_WRLOCK(ptr_lock) (ptr_lock->wrlock(ptr_lock))
#define RWLOCK_TRYWRLOCK(ptr_lock) (ptr_lock->trywrlock(ptr_lock))
#define RWLOCK_UNLOCK(ptr_lock) (ptr_lock->unlock(ptr_lock))

#ifdef __cplusplus
};
#endif
#endif //__RWLOCK_H__

