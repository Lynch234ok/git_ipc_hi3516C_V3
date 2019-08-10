#include <jaThread.h>


static ThreadFunData ThreadFunData_pan, ThreadFunData_tilt;

static JA_Boolean jaThread_terminated(TJA_Thread* pTJA_Thread)
{
	if(!pTJA_Thread)
	{
		return JA_False;
	}

	return pTJA_Thread->isTerminated;
}

static void jaThread_suspend(TJA_Thread *pTJA_Thread, int a, int b, int c)
{
	if(!pTJA_Thread)
	{
		return JA_False;
	}
}

static void* ThreadFun(void* arg)
{
	if( (ThreadFunData*)arg == NULL)
	{
		printf(" arg of Thread is NULL, ERROR!!\N");
		return NULL;
	}
	
	ThreadFunData *pstThreadFunData = (ThreadFunData*)arg;

	//线程状态改为unjoinable，使其在退出时自动释放占用的资源
	pthread_detach(pthread_self());
	pstThreadFunData->ThreadFun(&pstThreadFunData->obj_Thread
					, pstThreadFunData->argc
					, pstThreadFunData->argv);
}
	


static void jaThread_wait(TJA_Thread** ppTJA_Thread)
{
	if(*(ppTJA_Thread) == NULL)
	{
		return ;
	}

	while( !((*ppTJA_Thread)->isTerminated) )
	{
		usleep(50000);
	};
}

static int jaThread_terminate(TJA_Thread** ppTJA_Thread, JA_Boolean bVal)
{
	if(*(ppTJA_Thread) == NULL)
	{
		return -1;
	}
	
	(*ppTJA_Thread)->isTerminated = bVal;
	
	return 0;
}

static int jaThread_free(TJA_Thread** ppTJA_Thread)
{
	if(*(ppTJA_Thread) == NULL)
	{
		return -1;
	}
	
	//pthread_join( (*ppTJA_Thread)->threadID, NULL);
	//free(*(ppTJA_Thread));
	//*(ppTJA_Thread) = NULL;
	
	return 0;
}

static TJA_Thread* jaThread_create(void *pFun,  JA_Integer argc
	, TJA_StepperMotor * argv, JA_Int speed, TJA_StepperMotor *syn_Motor, enObjType objType)
{
	pthread_t Thread_id;
	int ret;

	TJA_Thread Thread;
	Thread.isTerminated = JA_False;
	Thread.terminated = jaThread_terminated;
	Thread.suspend = jaThread_suspend;
	Thread.wait = jaThread_wait;
	Thread.terminate = jaThread_terminate;

	ThreadFunData *pThreadFunData = NULL;
	switch(objType)
	{
		case OBJ_PAN:
			pThreadFunData = &ThreadFunData_pan;
			break;

		case OBJ_TILT:
			pThreadFunData = &ThreadFunData_tilt;
			break;

		default:
			return NULL;
			break;
	}
	
	pThreadFunData->ThreadFun = pFun;
	pThreadFunData->argc = argc;
	pThreadFunData->argv = argv;
	pThreadFunData->obj_Thread = Thread; //变量赋值
	pThreadFunData->obj_Thread.speed = speed;
	pThreadFunData->obj_Thread.syn_Motor = syn_Motor;
	
	ret = pthread_create(&Thread_id, NULL, (void  *)ThreadFun, (void*)pThreadFunData); 
	if(ret!=0)
	{ 
		printf("Create pthread error!\n");
		return NULL;
	} 
	
	return &(pThreadFunData->obj_Thread);
}

static TJA_Thread _JaThreadKit = {

	.threadID = -1,
	.isTerminated = JA_False,
	.create = jaThread_create,
	.terminated = jaThread_terminated,
	.suspend = jaThread_suspend,
	.wait = jaThread_wait,
	.terminate = jaThread_terminate,
	.Thread_free = jaThread_free,
};

TJA_Thread *pJaThreadKit = &_JaThreadKit;



