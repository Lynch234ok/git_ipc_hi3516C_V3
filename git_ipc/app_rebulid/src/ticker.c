
//
// a timer task module
//

#include "ticker.h"
#include "app_debug.h"
#include <sys/prctl.h>

typedef struct TICKER_TASK {
	time_t timestamp;
	time_t interleave;
	fTICKER_HANDLER handler;
	int hash;
	long lPara;
	void *pPara;
}ST_TICKER_TASK, *LP_TICKER_TASK;

typedef struct TICKER {
	
	bool listener_trigger;
	pthread_t listener_tid;

	ST_TICKER_TASK task[32];
	
}ST_TICKER, *LP_TICKER;

static ST_TICKER _ticker = {
	.listener_trigger = false,
	.listener_tid = (pthread_t)NULL,
};
static LP_TICKER _p_ticker = NULL;
#include "netsdk.h"

static void* ticker_listener(void* arg)
{
	int i = 0;
	time_t last_t = time(NULL);
	prctl(PR_SET_NAME, "ticker_listener");
	while(_p_ticker->listener_trigger){
		time_t const cur_t = time(NULL);
		
		// second changed
		if(last_t != cur_t){
			// for earch task
			for(i = 0; i < sizeof(_p_ticker->task) / sizeof(_p_ticker->task[0]) && _p_ticker->listener_trigger; ++i){
				LP_TICKER_TASK const task = &_p_ticker->task[i];

				// lockup an actived task
				if(NULL != task->handler){
					if(cur_t < task->timestamp){
						// update task timestamp to a previous
						task->timestamp = cur_t;
					}else if(cur_t - task->timestamp >= task->interleave){
						// update timestamp
						task->timestamp = cur_t;
						// handling
						/*if(task->pPara){
							APP_TRACE("%d %d %d",((LP_NSDK_VENC_CH)(task->pPara))->id, ((LP_NSDK_VENC_CH)(task->pPara))->frameRate, ((LP_NSDK_VENC_CH)(task->pPara))->constantBitRate);
						}*/
						task->handler(task->lPara, task->pPara);
					}
				}
			}
			last_t = cur_t;
		}
		usleep(200000);
	}
	pthread_exit(NULL);
}

static void ticker_listener_start()
{
	int ret = 0;
	if(!_p_ticker->listener_tid){
		_p_ticker->listener_trigger = true;
		ret = pthread_create(&_p_ticker->listener_tid, 0, ticker_listener, NULL);
		//assert(0 == ret);
	}
}

static void ticker_listener_stop()
{
	if(_p_ticker->listener_tid){
		_p_ticker->listener_trigger = false;
		pthread_join(_p_ticker->listener_tid, NULL);
		_p_ticker->listener_tid = (pthread_t)NULL;
	}
}

int TICKER_add_task(fTICKER_HANDLER const task_handler, time_t interl, bool immediate) // interleave uint: sec must be greater than 1
{
	return TICKER_add_task2(task_handler, interl, immediate, 0, 0, NULL, 0);
}


int TICKER_add_task2(fTICKER_HANDLER const task_handler, time_t interl, bool immediate, int hash, long lPara, void *pPara,int size) // interleave uint: sec must be greater than 1
{
	int i = 0;
	if(!task_handler){
		// invalid task
		return -1;
	}
	if(_p_ticker){
		TICKER_del_task2(task_handler, hash);
		for(i = 0; i < sizeof(_p_ticker->task) / sizeof(_p_ticker->task[0]); ++i){
			LP_TICKER_TASK const task = &_p_ticker->task[i];
			// find a position for the new task
			if(!task->handler){
				if(pPara){
					task->pPara = calloc(size, 1);
					memcpy(task->pPara, pPara, size);
				}else{
					task->pPara = NULL;
				}
				task->hash = hash;
				task->lPara = lPara;
				task->interleave = interl;
				if(task->interleave <= 0){
					task->interleave = 1; // at least 1 second
				}
				task->timestamp = time(NULL) + task->interleave;
				task->handler = task_handler;
				/*if(task->pPara){
					APP_TRACE("%d %d", ((LP_NSDK_VENC_CH)(task->pPara))->frameRate, ((LP_NSDK_VENC_CH)(task->pPara))->constantBitRate);
				}*/
				if(immediate){
					task->handler(task->lPara, task->pPara);
				}
				APP_TRACE("Add new task(%d)", i);
				return 0;
			}
		}
	}
	return -1;
}


void TICKER_del_task(fTICKER_HANDLER const task_handler)
{
	if(_p_ticker){
		int i = 0;
		for(i = 0; i < sizeof(_p_ticker->task) / sizeof(_p_ticker->task[0]); ++i){
			LP_TICKER_TASK const task = &_p_ticker->task[i];
			if(task_handler == task->handler){
				if(task->pPara){
					free(task->pPara);
					task->pPara = NULL;
				}
				task->hash = 0;
				task->handler = NULL; // very important at the 1st step
				APP_TRACE("Delete task(%d)", i);
				return;
			}
		}
	}
}

void TICKER_del_task2(fTICKER_HANDLER const task_handler, int hash)
{
	if(_p_ticker){
		int i = 0;
		for(i = 0; i < sizeof(_p_ticker->task) / sizeof(_p_ticker->task[0]); ++i){
			LP_TICKER_TASK const task = &_p_ticker->task[i];
			if(task_handler == task->handler && task->hash == hash){
				if(task->pPara){
					free(task->pPara);
					task->pPara = NULL;
				}
				task->hash = 0;
				task->handler = NULL; // very important at the 1st step
				APP_TRACE("Delete task(%d)", i);
				return;
			}
		}
	}
}


int TICKER_init()
{
	if(!_p_ticker){
		_p_ticker = &_ticker;
		// init	
		_p_ticker->listener_trigger = false;
		_p_ticker->listener_tid = (pthread_t)NULL;
		memset(_p_ticker->task, 0, sizeof(_p_ticker->task));
		// start listener
		ticker_listener_start();
		return 0;
	}
	return -1;
}

void TICKER_destroy()
{
	if(_p_ticker){
		// stop daemon
		ticker_listener_stop();
		_p_ticker = NULL;
	}
    printf("%s(%d) finish!!!\n", __FUNCTION__, __LINE__);
}

LP_TICKER_TIME TICKER_time_in()
{
	
	LP_TICKER_TIME ticker_time = calloc(sizeof(ST_TICKER_TIME), 1);
	gettimeofday(ticker_time, NULL);
	return ticker_time;
}

int TICKER_time_out(LP_TICKER_TIME ticker_time)
{
	if(ticker_time){
		struct timeval tv_now, tv_diff;
		gettimeofday(&tv_now, NULL);
		timersub(&tv_now, ticker_time, &tv_diff);
		free(ticker_time);
		return tv_diff.tv_sec * 1000 + tv_diff.tv_usec / 1000;
	}
	return -1;
}


