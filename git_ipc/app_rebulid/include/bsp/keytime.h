
#ifndef _KEY_TIME_H
#define _KEY_TIME_H

#include "bsp/bsp.h"

#define TIME_0S                 (0)
#define TIME_3S                 (3)
#define MINI_TIME				(2)
#define MID_TIME				(5)
#define MAX_TIME				(10)
#define LED_MIN_MS				(100)
#define LED_MAX_MS				(1000)
#define LED_MIN_MODE			(0)
#define LED_MAX_MODE			(1)
#define LED_DARK_MODE			(2)
#define LED_LIGHT_MODE			(3)

typedef void (*funBackCall)(void);

typedef struct tagArg_Thread {

	funBackCall funZero;
	funBackCall funOne;
	funBackCall funTwo;
	funBackCall funThree;
    funBackCall funFour;
}stArg_Thread;

typedef struct KeyInfo
{
    int keyNum;
    int keyVal[KEY_MAX_NUM];
}stKeyInfo, *lpKeyInfo;

extern int 
initKeyTime(funBackCall callBackZero, funBackCall callBackOne, funBackCall callBackTwo, funBackCall callBackThree, funBackCall callBackFour);

extern void KEY_startGetKeyAttr(lpKeyInfo KeyInfo);
extern void KEY_stopGetKeyAttr();

extern int KeyTime_destroy();

#define LED_CTRL
// LedMode 为0对应0.5秒的闪烁周期，为1对应1秒的闪烁周期


#define DEF_LED_ID         0
#define LED_REC_ID         1
#define LED_PRIV_ID        2
#define LED_NUM            6

typedef struct  LED_ATTR{
	bool LedStatus[LED_NUM];
	int LedMode[LED_NUM];
	
}ST_LED_ATTR,*LP_LED_ATTR;

extern int initLedContrl(int LedID, bool LedStatus, int LedMode);

extern int  KEY_LED_get_mode(int LedID);

extern int KEY_LED_set_mode(int LedID,bool LedStatus,int LedMode);

extern int KEY_LED_set_mode2(int LedID,bool LedStatus,int LedMode);

extern int KEY_LED_get_keep_time(int LedID);

extern int KEY_LED_set_keep_time(int LedID);

int KEY_LED_ledDestroy();

#endif

