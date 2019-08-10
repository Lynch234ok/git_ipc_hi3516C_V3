
//
// a timer task module
//

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#ifndef TICKER_H_
#define TICKER_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct timeval ST_TICKER_TIME,*LP_TICKER_TIME;
extern LP_TICKER_TIME TICKER_time_in();
extern int TICKER_time_out(LP_TICKER_TIME ticker_time);

typedef void (*fTICKER_HANDLER)(long lPara, void *pPara);
extern int TICKER_add_task(fTICKER_HANDLER const task_handler, time_t interl, bool immediate);
extern int TICKER_add_task2(fTICKER_HANDLER const task_handler, time_t interl, bool immediate, int hash, long lPara, void *pPara, int size);
extern void TICKER_del_task(fTICKER_HANDLER const task_handler);
extern void TICKER_del_task2(fTICKER_HANDLER const task_handler, int hash);


extern int TICKER_init();
extern void TICKER_destroy();


#ifdef __cplusplus
};
#endif
#endif //TICKER_H_

