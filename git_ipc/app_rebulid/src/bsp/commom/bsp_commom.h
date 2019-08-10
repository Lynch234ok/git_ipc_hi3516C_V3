
#ifndef _BSP_COMMOM_H
#define _BSP_COMMOM_H

#include <stdlib.h>
#include <stdio.h>
//#include <ctype.h>
//#include <sys/types.h>
#include <stdbool.h> 
//#include <pthread.h>
#include "bsp/bsp.h"
#ifdef __cplusplus 
	extern "C" { 
#endif

#ifndef KEY_MAX_NUM
#define KEY_MAX_NUM     8
#endif

typedef struct jaCmCommom {

	void (*GPIO_Init)(void);
	void (*IRCUT_Switch)(bool DNmode);
	void (*Speaker_Enable)(bool Enable);	//在此接口内设置音量
	int (*Get_Photo_Val)(void);
	int (*Get_Key_Val)(int *val);
	void (*IR_Led)(bool Enable);
	void (*WHITE_LIGHT_Led)(bool Enable);
	void (*Led_Contrl)(int LedID, bool EnableOne, bool EnableTwo);
	void (*Senser_Reset)(void);
	//有in和out类型 IOmode true为out, false为in;OutValue为输出的值,有返回1，无人返回0
	int (*Alarm)(bool IOmode, int AlarmID, bool OutValue);
	int (*RTC_Read)(void* arg);
	int (*RTC_Write)(void* arg);
    void (*Audio_set_volume_val)(int ai_gain, int ai_vol, int ao_gain, int ao_vol);
    void (*Wifi_power_enable)(bool enable);
    void (*Sd_power_enable)(bool enable);
}TJA_BSPCommom;

typedef  TJA_BSPCommom* 		CMhandle;


///<以下为公共接口
extern void BSP_ContrlCreate(TJA_BSPCommom Ctr_arg);
extern int BSP_ContrlDestroy(void);

#ifdef __cplusplus 
	}
#endif

#endif

