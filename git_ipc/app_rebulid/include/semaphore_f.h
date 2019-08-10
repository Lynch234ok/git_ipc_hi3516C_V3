
#include <stdio.h>
#include <stdbool.h>

#ifndef SEMAPHORE_F_H_
#define SEMAPHORE_F_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct SEMAPHORE {
	int (*pend)(struct SEMAPHORE *const _this, bool wait, int timeoutMS);
	int (*post)(struct SEMAPHORE *const _this);
	int (*value)(struct SEMAPHORE *const _this);
}ST_SEMAPHORE, *LP_SEMAPHORE;

extern LP_SEMAPHORE SEMAPHORE_create(int v);
extern void SEMAPHORE_release(LP_SEMAPHORE sem);

#ifdef __cplusplus
}
#endif
#endif //SEMAPHORE_F_H_

