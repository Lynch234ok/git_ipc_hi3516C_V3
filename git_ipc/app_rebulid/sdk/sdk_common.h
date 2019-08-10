
#include <stdio.h>
#include <stdlib.h>

#include <sdk/sdk_isp_def.h>
#include <HI3518E_V2/include/hi_comm_isp.h>
#include <HI3518E_V2/include/mpi_ae.h>
#include <HI3518E_V2/include/mpi_isp.h>
#include <HI3518E_V2/hi_isp_cfg_def.h>
#include <HI3518E_V2/include/hi_common.h>

#ifndef SDK_COMMON_H_
#define SDK_COMMON_H_
#ifdef __cplusplus
extern "C" {
#endif
#define IRCUT_D2N_GAIN_JUDGE (0)
#define IRCUT_D2N_LUM_JUDGE (1)
#define IRCUT_DEFAULT_SCENE (0)
#define IRCUT_OVER_EXPOSE_SCENE (1)
#define IRCUT_LOW_COLORTEMP_SCENE   (2)
#define IRCUT_MID_COLORTEMP_SCENE   (3)
#define IRCUT_HIGH_COLORTEMP_SCENE  (4)

typedef struct isp_ircut_switch {
	void (*isp_ircut_switch)(uint8_t bEnable);
	void (*isp_white_light_switch)(uint8_t bEnable);
	void (*isp_smartmode_isp_switch)(uint8_t bEnable);
}stIspIrcutSwitch,*lpstIspIrcutSwitch;

enum mode_state {
	STATE_DAY = 0,
	STATE_NIGHT,
};

typedef void (*SDK_ISP_PWM_LIGHT_CONTROL)(uint32_t Exposure, uint16_t dutyCycle);

typedef struct switch_ircut_info{

	enum mode_state daynight_mode_state;
	enum mode_state pre_daynight_mode_state;
	
	float night_to_day_factor;
	float day_to_night_factor;
	float  cur_expose_factor;
	float  pre_expose_factor;

	
	uint32_t very_likely_reflect;
	uint32_t night_stable_cnt;	
	uint32_t day_stable_cnt;
	float to_day_factor[8];
	int j;
	uint32_t detect_to_night_cnt;	
	uint32_t detect_to_day_cnt;
	uint16_t u16GlobalR;
	uint16_t u16GlobalG;
	uint16_t u16GlobalB;
	uint16_t u16ColorTemp;
	bool Bayer_flag;
 	uint8_t u8AveLum;
	uint32_t IrcutDayToNight[2];
	uint32_t IrcutColorTemp[2];
	uint32_t IrcutNightToDay[5];
	float    IrcutRDG[5];
	float    IrcutBDG[5];

	bool md_alarm_state;
	bool md_alarm_stop;
	bool md_alarm_switch;
	bool md_alarm_lock;
	bool md_alarm_open_light;
	bool md_alarm_keep_nightmode;

}stSwitchIrcutInfo,*lpstSwitchIrcutInfo;

extern int sdk_venc_bps_limit(int const enc_width, int const enc_height, int const enc_fps, int const enc_bps);
extern ssize_t sdk_yuv420sem_to_bitmap888(const void *yuv420sem, size_t yuv_width, size_t yuv_height, size_t yuv_stride, void *bmp888);
extern bool ircut_edge_detect(int *get_gpio_status);

extern int sdk_isp_ircut_switch_init(stIspIrcutSwitch ircut_switch);
extern int isp_ircut_switch_hardware_control( uint32_t *gpio_status_old,int gpio_status_cur);
extern int isp_ircut_switch_sofeware_control(lpstSwitchIrcutInfo ircut_state_info);

extern int isp_ircut_switch_hardware_control_lightmode(int gpio_status_cur, lpstSwitchIrcutInfo md_info);
extern int isp_ircut_switch_hardware_control_smartmode(uint32_t *gpio_status_old, int gpio_status_cur, lpstSwitchIrcutInfo md_info);
extern void isp_md_alarm_delay_cal(lpstSwitchIrcutInfo md_alarm_info, uint32_t *alarm_keep_count, bool md_lock);
extern int isp_ircut_switch_hardware_control( uint32_t *gpio_status_old,int gpio_status_cur);
extern int isp_ircut_switch_sofeware_control_2(lpstSwitchIrcutInfo ircut_state_info);
extern int isp_ircut_switch_linkage_control(lpstSwitchIrcutInfo ircut_state_info, uint32_t gpio_status_cur);
extern bool SDK_is_array_empty(uint8_t *array, int size);
extern int SDK_clean_array(uint8_t *array, int size);
extern int SDK_set_array(uint8_t *array, int size, int x, int y,int stride);
extern bool SDK_is_array_active(uint8_t *array, int size, int repeat);
extern void SDK_ISP_PWM_light_linkage(uint32_t Exposure, uint32_t ExpTarget, uint32_t Tolerance, uint32_t ExpThreshold, 
	uint16_t *step, uint8_t stepCount, uint16_t *stepIntervel, uint8_t isReflect, SDK_ISP_PWM_LIGHT_CONTROL Ctrl);
//extern lpstIspIrcutSwitch isp_ir_switch;

#ifdef __cplusplus
};
#endif
#endif //SDK_COMMON_H_

