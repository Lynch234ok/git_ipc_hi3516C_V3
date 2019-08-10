
#include "sdk_trace.h"
#include "sdk_common.h"
#include "time.h"
#define PRODUCT_TEST_INFO_FILE_PATH "/media/tf/production_test.ini"
#include "../include/sdk/sdk_vin.h"

int sdk_venc_bps_limit(int const enc_width, int const enc_height, int const enc_fps, int const enc_bps)
{
	int const enc_size = enc_width * enc_height;
	int max_bps = 0;

	if(enc_size <= (180 * 144)){
		max_bps = 256; // kbps
	}else if(enc_size <= (360 * 144)){
		max_bps = 384;
	}else if(enc_size <= (480 * 144)){
		max_bps = 480;
	}else if(enc_size <= (360 * 288)){
		max_bps = 512; // kbps
	}else if(enc_size <= (480 * 288)){
		max_bps = 640;
	}else if(enc_size <= (720 * 288)){
		max_bps = 768;
	}else if(enc_size <= (960 * 288)){
		max_bps = 1024;
	}else if(enc_size <= (720 * 576)){
		max_bps = 1536; // kbps
	}else if(enc_size <= (960 * 576)){
		max_bps = 2048; // kbps
	}else{
		//SOC_ASSERT(0, "Bitrate limited [%dx%d]", enc_width, enc_height);
		return enc_bps;
	}

	if(enc_bps <= max_bps){
		return enc_bps;
	}

	return max_bps;
}

ssize_t sdk_yuv420sem_to_bitmap888(const void *yuv420sem, size_t yuv_width, size_t yuv_height, size_t yuv_stride,
	void *bmp888)
{
	int i = 0, ii = 0;
	
	// allocate the bitmap data
	size_t bitmap24_size = yuv_width * yuv_height * 3;
	uint8_t *bitmap_offset = bmp888;

	SOC_DEBUG("BMP [%dx%d] stride %d", yuv_width, yuv_height, yuv_stride);
	
	if(yuv420sem){
		int y, u, v, yy, vr, ug, vg, ub;
		int r, g, b;
		const uint8_t *py = (uint8_t*)(yuv420sem);
		const uint8_t *puv = (uint8_t*)(py + yuv_width * yuv_height);

		// yuv420 to rgb888
		for(i = 0; i < yuv_height; ++i){
			for(ii = 0; ii < yuv_width; ++ii){
				y = py[0];
				yy = y * 256;
				
				u = puv[1] - 128;
				ug = 88 * u;
				ub = 454 * u;

				v = puv[0] - 128;
				vg = 183 * v;
				vr = 359 * v;

				///////////////////////////////////
				// convert
				r = (yy + vr) >> 8;
				g = (yy - ug - vg) >> 8;
				b = (yy + ub) >> 8;

				if(r < 0){
					r = 0;
				}
				if(r > 255){
					r = 255;
				}
				if(g < 0){
					g = 0;
				}
				if(g > 255){
					g = 255;
				}
				if(b < 0){
					b = 0;
				}
				if(b > 255){
					b = 255;
				}

				*bitmap_offset++ = (uint8_t)b;
				*bitmap_offset++ = (uint8_t)g;
				*bitmap_offset++ = (uint8_t)r;

				//SOC_DEBUG("RGB[%3d,%3d,%3d] @ (%3d,%3d)", r, g, b, ii, i);

				///////////////////////////////////
				++py;
				if(0 != (ii % 2)){
					// even
					puv += 2;
				}
			}

			if(0 != (i % 2)){
				// try twice
				puv -= yuv_width;
			}
		}
		return bitmap24_size;
	}
	return -1;
}

#define IRCUT_TIMER 600
bool ircut_edge_detect(int *get_gpio_status)
{
return true;
//	 isp_ir_switch->isp_ir_switch = isp_ir_switch;

	static bool ret_val = true;	
	static uint32_t count1,count2,time_flag=0,wait_flag=0,time_rev_flag =0;
//	static uint32_t set_gpio_status;
	
	struct timeval tpstart,tpend,timer_start;
	static uint32_t  timeuse,time_front,time_back,timer_front,flag_time;
	
	if( NULL == get_gpio_status)
		{
			printf("get gpio failed\n");
			return false;
		}
	if(0 == count1 && 0 == count2){
	//	set_gpio_status = *get_gpio_status;
		gettimeofday(&tpstart,NULL); 
		 time_front = (1000000*tpstart.tv_sec + tpstart.tv_usec)/1000000;
	}else if(3 < count1 && 3 < count2){
			if(4 == count1 && 4 == count2){
				count1++;
				count2++;
				*get_gpio_status = 1;//set_gpio_status
				return true;
			}
			ret_val = false;
			gettimeofday(&tpend,NULL); 
			
			time_back = (1000000*tpend.tv_sec+tpend.tv_usec)/1000000;
			flag_time = time_back-time_front;
			if((time_back-time_front) > IRCUT_TIMER ){
				count1 = 0;
				count2 = 0;
				ret_val = true;
				return ret_val;
				
			}
	}
	
	if(!(*get_gpio_status)){			           			
			count1++;
			if(count1 > 100){
				count1 = 10;
			}
	}else{			
		count2++;
		if(count2 > 100){
			count2 = 10;
		}
	 }
					
	return ret_val;

}

bool ircut_edge_detect2(int *get_gpio_status)
{
	static bool ret_val = true;
	static uint32_t count1,count2,time_flag=0,wait_flag=0,time_rev_flag =0;

	struct timeval tpstart,tpend,timer_start,check_timer;
	static uint32_t  timeuse,time_front,time_back,every_time;

	if( NULL == get_gpio_status)
		{
			printf("get gpio failed\n");
			return false;
		}
	if(0 == count1 && 0 == count2){
		gettimeofday(&tpstart,NULL);
		 time_front = (1000000*tpstart.tv_sec + tpstart.tv_usec)/1000000;
	}else if(10 < count1 && 10 < count2){
			if(11 == count1 && 11 == count2){
				count1++;
				count2++;
				*get_gpio_status = 1;//set_gpio_status
				return true;
			}
			ret_val = false;
			gettimeofday(&tpend,NULL);

			time_back = (1000000*tpend.tv_sec+tpend.tv_usec)/1000000;
			//flag_time = time_back-time_front;
			if((time_back-time_front) > IRCUT_TIMER ){
				count1 = 0;
				count2 = 0;
				ret_val = true;
				return ret_val;
			}
	}

	gettimeofday(&check_timer,NULL);
	every_time = (1000000*check_timer.tv_sec + check_timer.tv_usec)/1000000;
	if((every_time - time_front) > IRCUT_TIMER){
		count1 = 0;
		count2 = 0;
		return true;
	}
	if(!(*get_gpio_status)){
			count1++;
			if(count1 > 100){
				count1 = 10;
			}
	}else{
		count2++;
		if(count2 > 100){
			count2 = 10;
		}
	 }
	return ret_val;
}


static stIspIrcutSwitch isp_ir_switch;
static lpstIspIrcutSwitch _isp_ir_switch = NULL;

int sdk_isp_ircut_switch_init(stIspIrcutSwitch ircut_switch)
{
	_isp_ir_switch = &isp_ir_switch;	
	_isp_ir_switch->isp_ircut_switch = ircut_switch.isp_ircut_switch;
	_isp_ir_switch->isp_white_light_switch = ircut_switch.isp_white_light_switch;
	_isp_ir_switch->isp_smartmode_isp_switch = ircut_switch.isp_smartmode_isp_switch;

	return 0;
}


int isp_ircut_switch_hardware_control( uint32_t *gpio_status_old,int gpio_status_cur)
{
	gpio_status_cur = gpio_status_cur != 0 ? 1:0;
	if(*gpio_status_old != gpio_status_cur && ircut_edge_detect(&gpio_status_cur)){
		printf("%s-%d  ircut hareware switch:%d--%d\r\n", __FUNCTION__, __LINE__, *gpio_status_old, gpio_status_cur);
		*gpio_status_old = gpio_status_cur;
		printf("%d\r\n",*gpio_status_old);
		if(!gpio_status_cur){
			if(_isp_ir_switch){
				if(_isp_ir_switch->isp_ircut_switch){
					_isp_ir_switch->isp_ircut_switch(0); //daylight
				}
			}
		}else{
			if(_isp_ir_switch){
				if(_isp_ir_switch->isp_ircut_switch){
					_isp_ir_switch->isp_ircut_switch(1); //night
				}
			}
		}
	}
	return 0;
}

#define DAY_NIGHT_CHANGER_DELAY	(5)
int isp_ircut_switch_hardware_control_lightmode(int gpio_status_cur, lpstSwitchIrcutInfo md_info)
{
	gpio_status_cur = gpio_status_cur != 0 ? 1:0;
	static uint32_t count = 0;
	static float gain_last = 0;
	float gain_current = 0;
	int state_tmp = STATE_DAY;
	if(count++ >= DAY_NIGHT_CHANGER_DELAY){
		switch(md_info->daynight_mode_state){
			case STATE_DAY:
				if(gpio_status_cur && ircut_edge_detect2(&gpio_status_cur)){
					_isp_ir_switch->isp_white_light_switch(1);//night->light_on
					md_info->daynight_mode_state = STATE_NIGHT;
					gain_last = 0;
					count = 0;
				}
			break;
			case STATE_NIGHT:
				if(md_info->cur_expose_factor < 1.8){
					if(ircut_edge_detect2(&state_tmp)){
						_isp_ir_switch->isp_white_light_switch(0);//night->light_off
						md_info->daynight_mode_state = STATE_DAY;
						count = 0;
					}
				}
			break;
			default:
			break;
		}
	}
	return 0;
}


#define MD_DELAY_D2N_THRESHOLD	(15)
#define MD_DELAY_NIGHT_KEEP_THRESHOLD	(15)
#define MD_ALARM_DELAY_TIME		(30)
#define MD_ALARM_LIMIT_TIME		(900)

int isp_ircut_switch_hardware_control_smartmode(uint32_t *gpio_status_old, int gpio_status_cur, lpstSwitchIrcutInfo md_info)
{
	lpstSwitchIrcutInfo md_alarm_info = md_info;
	static uint32_t md_d2n_delay;
	static uint32_t md_night_delay;
	if(!md_alarm_info->md_alarm_lock){
		gpio_status_cur = gpio_status_cur != 0 ? 1:0;
		if(!gpio_status_cur){
			if(gpio_status_cur != *gpio_status_old){
				*gpio_status_old = gpio_status_cur;
				_isp_ir_switch->isp_smartmode_isp_switch(1); //daylight_color
			}
			md_alarm_info->md_alarm_switch = HI_FALSE;
			md_d2n_delay = 0;
		}else{
			if(gpio_status_cur != *gpio_status_old){
				*gpio_status_old = gpio_status_cur;
				_isp_ir_switch->isp_smartmode_isp_switch(0); //night_black
			}
			md_d2n_delay += 1;
			if(md_d2n_delay >= MD_DELAY_D2N_THRESHOLD){
				md_alarm_info->md_alarm_switch = HI_TRUE;
				md_d2n_delay = 0;
			}
		}
	}

	if(md_alarm_info->md_alarm_switch){
		if(md_alarm_info->md_alarm_state){
			md_alarm_info->md_alarm_open_light = HI_TRUE;
			md_alarm_info->md_alarm_keep_nightmode = HI_FALSE;
			md_d2n_delay = MD_DELAY_D2N_THRESHOLD;
		}
	}

	if(md_alarm_info->md_alarm_keep_nightmode){
		md_alarm_info->md_alarm_switch = gpio_status_cur ? HI_TRUE : HI_FALSE;
		md_night_delay += 1;
		if(md_night_delay == MD_DELAY_NIGHT_KEEP_THRESHOLD){
			md_alarm_info->md_alarm_lock = HI_FALSE;
			md_alarm_info->md_alarm_keep_nightmode = HI_FALSE;
			md_night_delay = 0;
		}
	}else{
		md_night_delay = 0;
	}
	return 0;
}


void isp_md_alarm_delay_cal(lpstSwitchIrcutInfo md_alarm_info, uint32_t *alarm_keep_count, bool md_lock)
{
	static time_t timer_start, timer_end, timer_tmp;
	struct timespec tpstart, tpend;
	static bool delay_enable;
	static bool is_exec = false;
	if(md_lock){
		md_alarm_info->md_alarm_lock = HI_FALSE;
		md_alarm_info->md_alarm_stop = HI_FALSE;
		is_exec = false;
		return;
	}
	if(md_alarm_info->md_alarm_open_light){
		md_alarm_info->md_alarm_lock = HI_TRUE;
		*alarm_keep_count = MD_ALARM_DELAY_TIME;
		md_alarm_info->md_alarm_open_light = HI_FALSE;
		if(!is_exec){
			_isp_ir_switch->isp_white_light_switch(1);//light_on
			_isp_ir_switch->isp_smartmode_isp_switch(1);
			is_exec = true;
		}
		if(!delay_enable){
			clock_gettime(CLOCK_MONOTONIC, &tpstart);
			timer_start = tpstart.tv_sec;
			timer_end   = tpstart.tv_sec;
			delay_enable = HI_TRUE;
			timer_tmp = 0;
		}
	}
	if(*alarm_keep_count > 0){

		if(delay_enable){
			clock_gettime(CLOCK_MONOTONIC, &tpend);
			timer_end   = tpend.tv_sec;
			timer_tmp   = timer_end - timer_start;
		}
		if(timer_tmp > MD_ALARM_LIMIT_TIME){
			md_alarm_info->md_alarm_stop = HI_TRUE;
			if(sdk_vin){
				sdk_vin->set_md_alarm_stop(&(md_alarm_info->md_alarm_stop));
			}
			delay_enable = HI_FALSE;
			timer_start = timer_end;
			timer_tmp = 0;
			_isp_ir_switch->isp_smartmode_isp_switch(0);
			_isp_ir_switch->isp_white_light_switch(0);
			md_alarm_info->md_alarm_open_light = HI_FALSE;
			md_alarm_info->md_alarm_switch = HI_FALSE;
			sleep(15);
			md_alarm_info->md_alarm_stop = HI_FALSE;
			if(sdk_vin){
				sdk_vin->set_md_alarm_stop(&(md_alarm_info->md_alarm_stop));
			}
			md_alarm_info->md_alarm_lock = HI_FALSE;
			md_alarm_info->md_alarm_keep_nightmode = HI_FALSE;
			*alarm_keep_count = 0;
			is_exec = false;
			return;
		}

		*alarm_keep_count -= 1;
		sleep(1);
		if(*alarm_keep_count == 0){
			md_alarm_info->md_alarm_stop = HI_TRUE;
			if(sdk_vin){
				sdk_vin->set_md_alarm_stop(&(md_alarm_info->md_alarm_stop));
			}
			_isp_ir_switch->isp_smartmode_isp_switch(0);
			_isp_ir_switch->isp_white_light_switch(0);
			md_alarm_info->md_alarm_open_light = HI_FALSE;
			md_alarm_info->md_alarm_switch 	= HI_FALSE;
			sleep(8);
			md_alarm_info->md_alarm_stop = HI_FALSE;
			if(sdk_vin){
				sdk_vin->set_md_alarm_stop(&(md_alarm_info->md_alarm_stop));
			}
			//md_alarm_info->md_alarm_switch 	= HI_TRUE;
			md_alarm_info->md_alarm_keep_nightmode = HI_TRUE;
			delay_enable = HI_FALSE;
			timer_start = timer_end;
			timer_tmp = 0;
			is_exec = false;
		}
	}
}

#define IRCUR_TURN_LOG (0)
#if IRCUR_TURN_LOG

#include <stdarg.h>
#include <time.h>
#include <../api/include/generic.h>

static inline int
SYSTEM(const char *fmt, ...)
{
	char command[4 * 1024];
	int ret = 0;
	va_list var;

	va_start(var, fmt);
	vsnprintf(command, sizeof(command), fmt, var);
	va_end(var);

	return system(command);
}
#endif


#define IRCUT_DETECT_CNT  3
bool isp_ircut_switch_bayer_judge(lpstSwitchIrcutInfo ircut_state_info, float RDG[2], float BDG[2])
{
	if((float)ircut_state_info->u16GlobalR/ircut_state_info->u16GlobalG > RDG[0] && 
		(float)ircut_state_info->u16GlobalR/ircut_state_info->u16GlobalG < RDG[1] && 
		(float)ircut_state_info->u16GlobalB/ircut_state_info->u16GlobalG > BDG[0] && 
		(float)ircut_state_info->u16GlobalB/ircut_state_info->u16GlobalG < RDG[1]){
		//printf("Reflect Light!\n");
		return false;
	}else{
		return true;
	}
}

bool isp_ircut_gain_to_bayer_judge(lpstSwitchIrcutInfo ircut_state_info, float RDG[2], float BDG[2])
{
	bool state;	
    if(ircut_state_info->u16ColorTemp < ircut_state_info->IrcutColorTemp[0]){
        ircut_state_info->night_to_day_factor = ircut_state_info->IrcutNightToDay[IRCUT_LOW_COLORTEMP_SCENE];
        RDG[0] = ircut_state_info->IrcutRDG[IRCUT_LOW_COLORTEMP_SCENE];
        BDG[0] = ircut_state_info->IrcutBDG[IRCUT_LOW_COLORTEMP_SCENE];

    }else if(ircut_state_info->u16ColorTemp >= ircut_state_info->IrcutColorTemp[0] && ircut_state_info->u16ColorTemp <= ircut_state_info->IrcutColorTemp[1]){
        ircut_state_info->night_to_day_factor = ircut_state_info->IrcutNightToDay[IRCUT_MID_COLORTEMP_SCENE];
        RDG[0] = ircut_state_info->IrcutRDG[IRCUT_MID_COLORTEMP_SCENE];
        BDG[0] = ircut_state_info->IrcutBDG[IRCUT_MID_COLORTEMP_SCENE];

    }else if(ircut_state_info->u16ColorTemp > ircut_state_info->IrcutColorTemp[1]){
        ircut_state_info->night_to_day_factor = ircut_state_info->IrcutNightToDay[IRCUT_HIGH_COLORTEMP_SCENE];
        RDG[0] = ircut_state_info->IrcutRDG[IRCUT_HIGH_COLORTEMP_SCENE];
        BDG[0] = ircut_state_info->IrcutBDG[IRCUT_HIGH_COLORTEMP_SCENE];
    }else{
        //use default val
    }

    if(0){//for debug
        ircut_state_info->night_to_day_factor = ircut_state_info->IrcutNightToDay[IRCUT_DEFAULT_SCENE];
        RDG[0] = ircut_state_info->IrcutRDG[IRCUT_DEFAULT_SCENE];
        BDG[0] = ircut_state_info->IrcutBDG[IRCUT_DEFAULT_SCENE];
    }

	if(ircut_state_info->cur_expose_factor < ircut_state_info->night_to_day_factor){

		if(ircut_state_info->cur_expose_factor < ircut_state_info->IrcutNightToDay[IRCUT_OVER_EXPOSE_SCENE]){
			RDG[0] = ircut_state_info->IrcutRDG[IRCUT_OVER_EXPOSE_SCENE];
			BDG[0] = ircut_state_info->IrcutBDG[IRCUT_OVER_EXPOSE_SCENE];
		}

		if(isp_ircut_switch_bayer_judge(ircut_state_info, RDG, BDG) && ircut_state_info->Bayer_flag){
			state = true;
		}else{
			state = false;
		}
		
	}else{
		state = false;
	}
	
	return state;
}

bool isp_ircut_gain_or_lum_judge(lpstSwitchIrcutInfo ircut_state_info)
{
	if(0 == ircut_state_info->IrcutDayToNight[IRCUT_D2N_GAIN_JUDGE]){
		return ircut_state_info->u8AveLum < ircut_state_info->IrcutDayToNight[IRCUT_D2N_LUM_JUDGE] ? true : false;
	}else if(0 == ircut_state_info->IrcutDayToNight[IRCUT_D2N_LUM_JUDGE]){
		return ircut_state_info->cur_expose_factor > ircut_state_info->IrcutDayToNight[IRCUT_D2N_GAIN_JUDGE] ? true : false;
	}
	return false;
}


int isp_ircut_switch_sofeware_control(lpstSwitchIrcutInfo ircut_state_info)
{
	lpstSwitchIrcutInfo ircut_state = ircut_state_info;	
	int save_state = ircut_state->daynight_mode_state;
    int ircut_detect_cnt = IRCUT_DETECT_CNT;
	float RDG[2] = {0, 1.280000}, BDG[2] = {0, 1.280000};

#if IRCUR_TURN_LOG
			time_t timep;
			struct tm *p;
			time(&timep);
			p = gmtime(&timep);
#endif

    if(access(PRODUCT_TEST_INFO_FILE_PATH, F_OK) != -1) {
        ircut_detect_cnt = 0;
    }

	switch (ircut_state->daynight_mode_state){
		case STATE_DAY:
			if(isp_ircut_gain_or_lum_judge(ircut_state)){
				ircut_state->detect_to_night_cnt += 1;
				if(ircut_state->detect_to_night_cnt >= ircut_detect_cnt){
					ircut_state->daynight_mode_state = STATE_NIGHT; 
					ircut_state->detect_to_night_cnt = 0;									
#if IRCUR_TURN_LOG
					if(-1 != access("/media/conf/netsdk/",F_OK)){
						SYSTEM("echo 'D->N[%d-%d %d:%d:%d]\t<GAIN:%f>\t<LUM:%d>\tR/G[%f]\tB/G[%f]\r' >> %s\n",(p->tm_mon)+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec,ircut_state->cur_expose_factor,ircut_state->u8AveLum,(float)ircut_state->u16GlobalR/ircut_state->u16GlobalG,(float)ircut_state->u16GlobalB/ircut_state->u16GlobalG, "/media/conf/netsdk/d2n_clock.txt");
					}
#endif
					if(_isp_ir_switch){
						if(_isp_ir_switch->isp_ircut_switch){
							_isp_ir_switch->isp_ircut_switch(1); //night
						}
					}
					SOC_INFO("switch night");
				}
			}else {
				ircut_state->detect_to_night_cnt = 0;
			}

			break;
		case STATE_NIGHT:
			if(isp_ircut_gain_to_bayer_judge(ircut_state_info, RDG, BDG)){
				ircut_state->detect_to_day_cnt += 1;
				if(ircut_state->detect_to_day_cnt >= ircut_detect_cnt){
					SOC_INFO("switch day ");
					ircut_state->daynight_mode_state = STATE_DAY;
					ircut_state->detect_to_day_cnt = 0;
#if IRCUR_TURN_LOG
				if(-1 != access("/media/conf/netsdk/",F_OK)){
					SYSTEM("echo 'N->D[%d-%d %d:%d:%d]\t<G:%f>\t<LUM:%d>\tR/G[%f]\tB/G[%f]\r' >> %s\n",(p->tm_mon)+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec,ircut_state->cur_expose_factor,ircut_state->u8AveLum,(float)ircut_state->u16GlobalR/ircut_state->u16GlobalG,(float)ircut_state->u16GlobalB/ircut_state->u16GlobalG, "/media/conf/netsdk/n2d_clock.txt");
				}
#endif
					_isp_ir_switch->isp_ircut_switch(0); //day
				}
			}else {
				ircut_state->detect_to_day_cnt = 0;
			}		
			break;
		default:
			break;

	}

	ircut_state->pre_daynight_mode_state = save_state;
	return 0;
}


//涓撲笟鏈夌嚎鍒嗘敮杞垏鎹㈡帶鍒堕�昏緫

#define IRCUT_DETECT_CNT  3
int isp_ircut_switch_sofeware_control_2(lpstSwitchIrcutInfo ircut_state_info)
{
	lpstSwitchIrcutInfo ircut_state = ircut_state_info;	
	int save_state = ircut_state->daynight_mode_state;	 
	switch (ircut_state->daynight_mode_state) {
		case STATE_DAY:
			if(ircut_state->cur_expose_factor > ircut_state->day_to_night_factor ){
				ircut_state->detect_to_night_cnt += 1;				
				ircut_state->day_stable_cnt = 0;
				if(ircut_state->detect_to_night_cnt >=  IRCUT_DETECT_CNT){
					if(ircut_state->very_likely_reflect == 1){

						int m = ircut_state->j;
						while(ircut_state->pre_expose_factor  <  ircut_state->to_day_factor[m]){
							m += 1;
							if(m < 0){
								m = 0;
								break;
							}else if(m > 7){	
								m = 7;
								break;
							}					
						}
						ircut_state->night_to_day_factor = ircut_state->to_day_factor[m]; //  1/2
						ircut_state->j = m;
	
					}
					
					ircut_state->daynight_mode_state = STATE_NIGHT; 
					ircut_state->detect_to_night_cnt = 0;				
					ircut_state->very_likely_reflect = 1;
					_isp_ir_switch->isp_ircut_switch(1); //night
					SOC_INFO("switch night");

				}
				

			}else{	
			
				ircut_state->detect_to_night_cnt = 0;
				ircut_state->day_stable_cnt += 1;
				if(ircut_state->day_stable_cnt > IRCUT_DETECT_CNT){
					ircut_state->day_stable_cnt = 0;
					ircut_state->very_likely_reflect = 0;
					ircut_state->j = 0;
					ircut_state->night_to_day_factor = ircut_state->to_day_factor[ircut_state->j]; //  1/2

				}						
			}
	
			break;
		case STATE_NIGHT:
			if(ircut_state->cur_expose_factor <= ircut_state->night_to_day_factor && ircut_state->very_likely_reflect){
				ircut_state->detect_to_day_cnt += 1;	
				ircut_state->night_stable_cnt = 0;
				if(ircut_state->detect_to_day_cnt >= IRCUT_DETECT_CNT){					
					SOC_INFO("switch day ");
					ircut_state->very_likely_reflect = 1;
					ircut_state->daynight_mode_state = STATE_DAY;
					ircut_state->detect_to_day_cnt = 0;
					ircut_state->pre_expose_factor = ircut_state->cur_expose_factor;
					_isp_ir_switch->isp_ircut_switch(0); //day

				}

			}else if(ircut_state->cur_expose_factor <= ircut_state->night_to_day_factor && !ircut_state->very_likely_reflect){
				ircut_state->detect_to_day_cnt += 1;			
				ircut_state->night_stable_cnt = 0;				
				if(ircut_state->detect_to_day_cnt >= IRCUT_DETECT_CNT){	
					SOC_INFO("switch day ");
					ircut_state->very_likely_reflect = 0;
					ircut_state->daynight_mode_state = STATE_DAY;						
					ircut_state->detect_to_day_cnt = 0;
					ircut_state->pre_expose_factor = ircut_state->cur_expose_factor;
					_isp_ir_switch->isp_ircut_switch(0); //day

				}

			}else {
				ircut_state->detect_to_day_cnt = 0;			
				ircut_state->night_stable_cnt += 1;
				
				int k;
				if(ircut_state->cur_expose_factor  > ircut_state->night_to_day_factor && ircut_state->night_stable_cnt > IRCUT_DETECT_CNT) 
				{
								
					k = ircut_state->j -1;
					if(k < 0){
						k = 0;
					}else if(k > 7){	
						k = 7;
					}

					while(ircut_state->cur_expose_factor > ircut_state->to_day_factor[k]){
						k  -= 1;
						if(k < 0){
							k = 0;
							break;
						}				
					}
					ircut_state->night_to_day_factor = ircut_state->to_day_factor[k + 1];
					ircut_state->j = k + 1;

					if(ircut_state->cur_expose_factor > ircut_state->to_day_factor[0]){
						ircut_state->night_to_day_factor = ircut_state->to_day_factor[0];
					}

					ircut_state->night_stable_cnt = 0;
					ircut_state->very_likely_reflect = 0;	
							
				}
			
			}		
			break;
		default:
			break;

	}

	ircut_state->pre_daynight_mode_state = save_state;


return 0;
}






#define ISP_GPIO_DAYLIGHT (0)
#define ISP_GPIO_NIGHT (1)

int isp_ircut_switch_linkage_control(lpstSwitchIrcutInfo ircut_state_info, uint32_t gpio_status_cur)
{
	if(ISP_GPIO_DAYLIGHT == ircut_state_info->daynight_mode_state)
	{	
		gpio_status_cur = gpio_status_cur != 0 ? 1:0;

		if(ircut_state_info->daynight_mode_state != gpio_status_cur){
			isp_ircut_switch_sofeware_control_2(ircut_state_info);
		}
	}
	else if(ISP_GPIO_NIGHT == ircut_state_info->daynight_mode_state){
		isp_ircut_switch_hardware_control(&(ircut_state_info->daynight_mode_state), gpio_status_cur); // night to day	
	}
	return 0;	
}




bool SDK_is_array_empty(uint8_t *array, int size)
{
	int i,sum = 0;
	while(size > 0){
		if(*array > 0){
			return false;
		}
		array++;
		size--;
	}
	return true;
}

int SDK_clean_array(uint8_t *array, int size)
{
	memset(array, 0, size);
	//printf("%s\r\n", __FUNCTION__);
	return 0;
}

int SDK_set_array(uint8_t *array, int size, int x, int y,int stride)
{
	if(y*stride+x < size){
		*(array + y*stride+x) += 1;
		/*if(*(array + y*stride+x) >= 2){
			return 0;
		}*/
	}
	return -1;
}

bool SDK_is_array_active(uint8_t *array, int size, int repeat)
{
	while(size > 0){
		if(*array >= (uint8_t)repeat){
			//printf("ACTIVE:%s\r\n", __FUNCTION__);
			return true;
		}
		array++;
		size--;
	}
	return false;
}

#define SDK_ISP_PWM_LIGHT_DUTY_CYCLE_MAX (100)
#define SDK_ISP_PWM_LIGHT_DUTY_CYCLE_MIN (0)
#define SDK_ISP_PWM_LIGHT_INIT_DUTY_CYCLE SDK_ISP_PWM_LIGHT_DUTY_CYCLE_MAX
#define SDK_ISP_PWM_LIGHT_CHECK_COUNT (10)
#define SDK_ISP_PWM_OVER_STABLE_COUNT (3)
/*
* PWM light control related Exposure
* @param   Exposure: 当前曝光量, 单位为曝光时间*增益
* @param   ExpTarget:  需要达到的目标亮度, 单位为曝光时间*增益
* @param   Tolerance:  达到目标亮度的容忍偏差, 单位为曝光时间*增益
* @param   ExpThreshold: 开灯的曝光量阈值,单位为曝光时间*增益
* @param   step: PWM步进的步伐,最大64个步伐
* @param   stepCount: PWM步进的步伐个数
* @param   stepIntervel: PWM步进的等待时间，单位ms
* @param   isReflect:当前环境是否存在反光现象
* @param   SDK_ISP_PWM_LIGHT_CONTROL:PWM控制接口回调
*/

void SDK_ISP_PWM_light_linkage(uint32_t Exposure, uint32_t ExpTarget, uint32_t Tolerance, uint32_t ExpThreshold, 
	uint16_t *step, uint8_t stepCount, uint16_t *stepIntervel, uint8_t isReflect, SDK_ISP_PWM_LIGHT_CONTROL Ctrl)
{
	uint32_t targetMin = ExpTarget - Tolerance> 0 ? ExpTarget - Tolerance:0;
	uint32_t targetMax = ExpTarget + Tolerance;
	static uint32_t lastExposure = -1;
	static int8_t lastDutyCycle = 0, checkCount = 0, stableCount = 0;
	uint8_t duty_cycle = 0;

	if(!Ctrl){
		return;
	}
	//printf("exp:[%d %d] %d    step:%d\n", targetMin, targetMax, Exposure, step[lastDutyCycle]);
	if(lastDutyCycle > 0){
		//LED is opened
		if(Exposure > targetMax){
			if(stableCount <= 0){
				if(lastDutyCycle < stepCount-1){
					Ctrl(Exposure, step[++lastDutyCycle]);
					usleep(stepIntervel[lastDutyCycle]*1000);
				}else{
					//LED is lightest
				}
			}else{
				//env stable checking
				stableCount--;
				//printf("env stable checking %d\n", stableCount);
			}
		}else if(Exposure < targetMin){
			if(stableCount <= 0){
				if(isReflect && step[lastDutyCycle-1] == SDK_ISP_PWM_LIGHT_DUTY_CYCLE_MIN){
					//light is lowest, and there is reflecting;
					//printf("reflecting\n");
				}else{
					Ctrl(Exposure, step[--lastDutyCycle]);
					usleep(stepIntervel[lastDutyCycle]*1000);
				}
			}else{
				//env stable checking
				stableCount--;
				//printf("env stable checking %d\n", stableCount);
			}
		}else{
			//targetMin <= Exposure <= targetMax
			//it's stable
			stableCount = SDK_ISP_PWM_OVER_STABLE_COUNT;
		}
	}else{
		//LED is closed
		if(Exposure > ExpThreshold){
			//need to open light
			checkCount++;
			//printf("checkCount = %d\n", checkCount);
			if(checkCount >= SDK_ISP_PWM_LIGHT_CHECK_COUNT){
				lastDutyCycle = stepCount/2;
				Ctrl(Exposure, step[lastDutyCycle]);
				checkCount = 0;
			}
		}else{
			//reset counting
			checkCount = 0;
		}
	}

}
