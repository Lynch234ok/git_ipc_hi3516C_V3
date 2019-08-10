
#include "tfcard_export.h"

#include <sys/prctl.h>
#include <pthread.h>
#include <errno.h>

typedef struct  tagTFTHREAD_
{
	NK_Char		strName[32];	////线程名字
	NK_Boolean	bDestory;		////1代表已经在销毁中,
	NK_Boolean	bTrigger;		////
	NK_Boolean	bRuning;		////
	NK_PVoid	pUserCtx;		////指向用户上下文
	NK_Boolean	bUsedCtx;		////是否使用
	pthread_t 	thr;
    pthread_mutex_t lock;
} stTFTHREAD,*pstTFTHREAD;

#define   TF_THREAD_STACK_SIZE  8*1024

HTFTHREAD TFCARD_THR_Create(NK_Int nThrPriority, NK_Int nThrStackSize,APIS_PTHREAD_FUNC pFunc,NK_PVoid arg)
{
	NK_Int nRet = 0;
	NK_Int nThreadStackSize = TF_THREAD_STACK_SIZE;
	NK_Int nTimes = 10;
	pstTFTHREAD pThread = (pstTFTHREAD)malloc(sizeof(stTFTHREAD));
	if(NK_Nil == pThread)
	{
		return NK_Nil;
	}
	//memset(&pThread , 0 , sizeof(stTHREAD));
    pthread_mutex_init(&pThread->lock, NK_Nil);

	nThreadStackSize = (1024 > nThrStackSize) ? TF_THREAD_STACK_SIZE : nThrStackSize;
    pthread_mutex_trylock(&pThread->lock); /////锁定创建线程

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, nThreadStackSize);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);	/////线程分离

	pThread->bDestory = NK_FALSE;
	pThread->bRuning = NK_TRUE;
	pThread->bTrigger = NK_TRUE;
	pThread->bUsedCtx = NK_FALSE;
	pThread->pUserCtx = (NK_Nil == arg) ? NK_Nil : arg;
	if(0 != pthread_create(&pThread->thr, &attr, pFunc, (void *)pThread))
	{
		pthread_attr_destroy(&attr);
		pthread_mutex_unlock(&pThread->lock);/////解锁创建线程
		goto ERROR;
	}
	pthread_attr_destroy(&attr);
    pthread_mutex_unlock(&pThread->lock); /////解锁创建线程

	while(nTimes--)
	{
		if(NK_TRUE == pThread->bUsedCtx)
		{
			break;
		}
		usleep(1000);
	}

	return (HTFTHREAD)pThread;

ERROR:
	if(NK_Nil != pThread)
	{
		pthread_mutex_destroy(&pThread->lock);
		free(pThread);
	}
	return NK_Nil;

}

NK_Int TFCARD_THR_Destory(HTFTHREAD hThread)
{
	NK_Int nTimes = 1000;
	pstTFTHREAD pThread = (pstTFTHREAD)hThread;
	if(NK_Nil == pThread)
	{
		return NK_FALSE;
	}
	if(NK_TRUE == pThread->bDestory)	////防止二次调用销毁
	{
		return NK_FALSE;
	}
	////等待线程状态退出线程
    pthread_mutex_lock(&pThread->lock);
	pThread->bTrigger = NK_FALSE;
	pThread->bDestory = NK_TRUE;		////正在销毁
	pthread_mutex_unlock(&pThread->lock);
	while(nTimes--)
	{
		if(NK_FALSE == pThread->bRuning)
		{
			break;
		}
		pthread_mutex_lock(&pThread->lock);
		pThread->bTrigger = NK_FALSE;
		pThread->bDestory = NK_TRUE; ////正在销毁
		pthread_mutex_unlock(&pThread->lock);
		usleep(20*1000);
	}

	//pthread_cancel(pThread->thr);
	//pthread_join(pThread->thr, NULL);
	pthread_mutex_destroy(&pThread->lock);
	free(pThread);
	pThread = NK_Nil;
	return NK_TRUE;
}

NK_Boolean TFCARD_THR_Triger(HTFTHREAD hThread)
{
	pstTFTHREAD pThread = (pstTFTHREAD)hThread;
	if(NK_Nil == pThread)
	{
		return NK_FALSE;
	}
	return pThread->bTrigger;
}

NK_Void TFCARD_THR_Cancel(HTFTHREAD hThread)
{
	pstTFTHREAD pThread = (pstTFTHREAD)hThread;
	if(NK_Nil == pThread)
	{
		return ;
	}
    pthread_mutex_lock(&pThread->lock);
	pThread->bRuning = NK_FALSE;
	pThread->bTrigger = NK_FALSE;
    pthread_mutex_unlock(&pThread->lock);
}

NK_Void TFCARD_THR_SetName(NK_PChar pThrName)
{
	if(NK_Nil == pThrName)
	{
		return ;
	}
	prctl(PR_SET_NAME, pThrName);
}

NK_PVoid TFCARD_THR_GetUserCtx(HTFTHREAD hThread)
{
	pstTFTHREAD pThread = hThread;
	if(NK_Nil == pThread)
	{
		return NK_Nil;
	}
	pThread->bUsedCtx = NK_TRUE;
	if(NK_Nil == pThread->pUserCtx)
	{
		return NK_Nil;
	}
	return pThread->pUserCtx;
}


NK_Int TFCARD_THR_Lock(HTFTHREAD hThread)
{
	pstTFTHREAD pThread = (HTFTHREAD)hThread;
	if(NK_Nil == pThread)
	{
		return NK_FALSE;
	}
	pthread_mutex_lock(&pThread->lock);/////加锁
	return NK_TRUE;
}
NK_Int TFCARD_THR_UnLock(NK_PVoid hThread)
{
	pstTFTHREAD pThread = hThread;
	if(NK_Nil == pThread)
	{
		return NK_FALSE;
	}
    pthread_mutex_unlock(&pThread->lock); /////解锁
	return NK_TRUE;
}

