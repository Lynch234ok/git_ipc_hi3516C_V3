
#ifndef _TFCARD_THREAD_H_
#define _TFCARD_THREAD_H_

typedef  NK_PVoid HTFTHREAD;

typedef void *(*APIS_PTHREAD_FUNC)(void *);

HTFTHREAD	TFCARD_THR_Create(NK_Int nThrPriority, NK_Int nThrStackSize,APIS_PTHREAD_FUNC pFunc,NK_PVoid arg);
NK_Int 		TFCARD_THR_Destory(HTFTHREAD hThread);

NK_Boolean 	TFCARD_THR_Triger(HTFTHREAD hThread);
NK_Void 	TFCARD_THR_Cancel(HTFTHREAD hThread);
NK_Void 	TFCARD_THR_SetName(NK_PChar pThrName);
NK_PVoid 	TFCARD_THR_GetUserCtx(NK_PVoid hThread);

NK_Int 		TFCARD_THR_Lock(HTFTHREAD hThread);
NK_Int		TFCARD_THR_UnLock(HTFTHREAD hThread);

#define TFCARD_THR_CreateEx(pFunc,arg) TFCARD_THR_Create(0, 0, pFunc, arg)

#endif

