
#include <stdio.h>
#include <stdbool.h>

#ifndef MUTEX_F_H_
#define MUTEX_F_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct MUTEX {
	int (*tryLock)(struct MUTEX *const _this);
	int (*lock)(struct MUTEX *const _this);
	int (*unlock)(struct MUTEX *const _this);
}ST_MUTEX, *LP_MUTEX;

extern LP_MUTEX MUTEX_create();
extern void MUTEX_release(LP_MUTEX m);

#ifdef __cplusplus
}
#endif
#endif //MUTEX_F_H_

