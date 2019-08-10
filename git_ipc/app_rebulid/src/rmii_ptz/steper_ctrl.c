#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include "steper_ctrl.h"
#include "step.h"
#include <sys/prctl.h>


typedef struct _steper_attr{
	int trigger_h;
	int trigger_v;
	pthread_t pid_h;
	pthread_t pid_v;
	unsigned int step_h;
	unsigned int step_v;
	int operation_h;
	int operation_v;
	int operation_limit_h;
	int operation_limit_v;
	int speed;
	int is_auto;
}ST_STEPER_ATTR,*LP_STEP_ATTR;

static ST_STEPER_ATTR stSteper;

static void steperH_sig_routine(){

	stSteper.step_h = (stSteper.step_h + stSteper.operation_h)%8;
	stepH(stSteper.step_h);
	//signal(SIGALRM, steperH_sig_routine);
	if(step_get_hlimit()){
		if(stSteper.is_auto){
			//change size when runing in auto mode
			stSteper.operation_h *= -1;
			printf("change size h :%d\r\n", stSteper.operation_h);
		}else{
			//stop the steper
			printf("stop horizontal step\r\n");
			stSteper.trigger_h = 0;
		}
	}
    return;
}

static void steperV_proc()
{
	prctl(PR_SET_NAME, "rmii_ptz_steperV_proc");
	while(stSteper.trigger_v){
		stSteper.step_v = (stSteper.step_v + stSteper.operation_v)%8;
		stepV(stSteper.step_v);
		if(step_get_vlimit()){
			printf("stop vertical step\r\n");
			stSteper.trigger_v = 0;
		}
		usleep(3000);
	}
	stepV_stop();
	pthread_exit(NULL);
}

static void steperH_proc()
{	
#if 0
	struct itimerval value, ovalue; 		 //(1)
	signal(SIGALRM, steperH_sig_routine);
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 5000;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 5000;
	setitimer(ITIMER_REAL, &value, &ovalue);	 //(2)

	while(stSteper.trigger_h){	
		usleep(200000);
	}
#else
	prctl(PR_SET_NAME, "rmii_ptz_steperH_proc");
	while(stSteper.trigger_h){
		stSteper.step_h = (stSteper.step_h + stSteper.operation_h)%8;
		stepH(stSteper.step_h);
		if(0 == stSteper.step_h%2 && step_get_hlimit()){
			if(stSteper.is_auto){
				//change size when runing in auto mode
				stSteper.operation_h *= -1;
				printf("change size h :%d\r\n", stSteper.operation_h);
			}else{
				//stop the steper
				stSteper.operation_limit_h = stSteper.operation_h;
				printf("stop horizontal step\r\n");
				stSteper.trigger_h = 0;
			}
		}
		usleep(2000);
	}
#endif
	stepH_stop();
	pthread_exit(NULL);
}

int STEPER_init()
{
	step_gpio_init();
	stSteper.trigger_h = 0;
	stSteper.trigger_v = 0;
	stSteper.step_h = 0;
	stSteper.step_v = 0;
	stSteper.pid_h = NULL;
	stSteper.pid_v = NULL;
	stSteper.operation_limit_h = 0;
	stSteper.operation_limit_v = 0;
	stepH_stop();
	stepV_stop();
}

int STEPER_destroy()
{
	stSteper.trigger_h = 0;
	stSteper.trigger_v = 0;
	pthread_join(stSteper.pid_h, NULL);
	pthread_join(stSteper.pid_v, NULL);
	stSteper.pid_h = NULL;
	stSteper.pid_v = NULL;
}

int STEPER_startV(int operation, int speed)
{
		if(0 == stSteper.trigger_v){
		stSteper.trigger_v = 1;
		stSteper.speed = speed;
		if(0 < operation){
			//right
			stSteper.operation_v = 1;
		}else if(0 == operation){
			//there is no auto mode in vertical
			stSteper.operation_v = 1;
		}else{
			//left
			stSteper.operation_v = -1;
		}
		
		pthread_create(&stSteper.pid_v, NULL, steperV_proc, NULL);
		return 0;
	}
	return -1;
}

int STEPER_stopV()
{
	if(stSteper.trigger_v){
		stSteper.trigger_v = 0;
		pthread_join(stSteper.pid_v, NULL);
		stSteper.pid_v = NULL;
	}
	stepV_stop();
	return 0;
}

int STEPER_startH(int operation, int speed)
{
	if(0 == stSteper.trigger_h){
		stSteper.trigger_h = 1;
		stSteper.speed = speed;
		if(0 < operation){
			//right
			stSteper.operation_h = 1;
		}else if(0 == operation){
			//auto
			stSteper.is_auto = 1;
			stSteper.operation_h = 1;
		}else{
			//left
			stSteper.operation_h = -1;
		}

		pthread_create(&stSteper.pid_h, NULL, steperH_proc, NULL);
		return 0;
	}
	return -1;
}

int STEPER_stopH()
{
	if(stSteper.trigger_h){
		stSteper.trigger_h = 0;
		pthread_join(stSteper.pid_h, NULL);
		stSteper.pid_h = NULL;
		stSteper.is_auto = 0;
	}
	stepH_stop();
	return 0;
}


