
#ifndef _BSP_H
#define _BSP_H

#include <stdlib.h>
#include <stdio.h>
//#include <ctype.h>
//#include <sys/types.h>
#include <stdbool.h> 
//#include <pthread.h>

#ifdef __cplusplus 
	extern "C" { 
#endif

#define DAYLIGHT	0
#define NIGHTLIGHT	1
#define IRLED_DAY	0
#define IRLED_NIGHT	1
#define ALARM_IN				true
#define ALARM_OUT				false
#define ALARM_OUT_HIGHT			true
#define ALARM_OUT_LOW			false
#define BSP_KEY_PRESS	(0)
#define BSP_KEY_RELEASE (1)
#define BSP_KEY_NULL    (-1)
#ifndef KEY_MAX_NUM
#define KEY_MAX_NUM     8
#endif

enum
{
    em_BSP_MODEL_NAME_PX = 0,
    em_BSP_MODEL_NAME_CX,
    em_BSP_MODEL_NAME_NONE
}em_BSP_MODEL_NAME;

///<以下为公共接口
extern void BSP_ContrlInit(int val, int audioHwSpec, int model_name, bool ledEnabled);
extern void BSP_GPIO_Init(void);
extern void BSP_IRCUT_Switch(bool DNmode);
extern void BSP_Speaker_Enable(bool Enable);
extern int BSP_Get_Photo_Val(void);

/* 功能:获取所有按键状态值，最大可返回KEY_MAX_NUM个按键状态 */
/* 参数，返回KEY_MAX_NUM大小的数组 val[0]表示第一个按键，以此类推 */
/* 按键状态，0->down，1->up 如果硬件上没有对应的按键状态 -> -1 */
extern int BSP_Get_Key_Val(int *val);
extern void BSP_IR_Led(bool Enable);
extern void BSP_WHITE_LIGHT_Led(bool Enable);
extern void BSP_Led_Contrl(int LedID, bool EnableOne, bool EnableTwo);
extern int BSP_Alarm(bool IOmode, int AlarmID, bool OutValue);
extern int BSP_RTC_Read(void* arg);
extern int BSP_RTC_Write(void* arg);
extern int BSP_Senser_reset(void);
extern void BSP_Audio_set_volume_val(int ai_gain, int ai_vol, int ao_gain, int ao_vol);
extern void BSP_Wifi_power_enable(bool Enable);
extern void BSP_Sd_power_enable(bool Enable);

#ifdef __cplusplus 
	}
#endif

#endif

