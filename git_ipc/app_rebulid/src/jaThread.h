#ifndef _JATHREAD_H
#define _JATHREAD_H

#include <object.h>
#include <stepper_motor.h>

typedef struct tagTJA_Thread TJA_Thread;

struct tagTJA_Thread{

	pthread_t  threadID;///<Ïß³Ìid
	JA_Boolean isTerminated;
	JA_Int speed;
	TJA_StepperMotor *syn_Motor;

	TJA_Thread* (*create)(void *pFun,  JA_Integer argc, TJA_StepperMotor * argv
				, JA_Int speed, TJA_StepperMotor *syn_Motor, enObjType objType);
	
	JA_Boolean (*terminated)(TJA_Thread*);
	void (*suspend)(TJA_Thread *, int a, int b, int c);
	void (*wait)(TJA_Thread**);
	int (*terminate)(TJA_Thread**, bool);
	int (*Thread_free)(TJA_Thread** ppTJA_Thread);
};

typedef struct tagThreadFunData{
	TJA_Thread obj_Thread;
	JA_Void (*ThreadFun)(TJA_Thread * const Thread, JA_Integer argc, JA_PVoid argv[]);
	JA_Integer argc;
	TJA_StepperMotor * argv;
}ThreadFunData;

extern TJA_Thread *pJaThreadKit;

#endif
