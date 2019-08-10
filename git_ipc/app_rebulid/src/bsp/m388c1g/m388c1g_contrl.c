
#include "m388c1g_contrl.h"
#include "../commom/bsp_commom.h"
#include "../higpio/higpio.h"
#include <hi_type.h>


static void m388c1g_GPIO_Init(void)
{

}

static void m388c1g_IRCUT_Switch(bool DNmode)
{

}

static void m388c1g_Speaker_Enable(bool Enable)
{

}

static int m388c1g_Get_Photo_Val(void)
{

}

static int m388c1g_Get_Key_Val(int *val)
{
	//无
}

static void m388c1g_IR_Led(bool Enable)
{
	//无
}

static void m388c1g_WHITE_LIGHT_Led(bool Enable)
{

}

static void m388c1g_Led_Contrl(int LedID, bool EnableOne, bool EnableTwo)
{
	//无
}

static void m388c1g_Senser_reset(void)
{

}

//有in和out类型
static int m388c1g_Alarm(bool OCmode)
{

}

static int m388c1g_RTC_Read(void* arg)
{

}

static int m388c1g_RTC_Write(void* arg)
{

}

static void m388c1g_Audio_set_volume_val(int ai_gain, int ai_vol, int ao_gain, int ao_vol)
{

}

static void m388c1g_Wifi_power_enable(bool enable)
{

}

static void m388c1g_Sd_power_enable(bool enable)
{

}

void BSP_ContrlInit(int val, int audioHwSpec, int model_name, int ledEnabled)
{
	TJA_BSPCommom m388c1g_cmInterface = {

		.GPIO_Init			= m388c1g_GPIO_Init,
		.IRCUT_Switch 		= m388c1g_IRCUT_Switch,
		.Speaker_Enable 	= m388c1g_Speaker_Enable,
		.Get_Photo_Val 		= m388c1g_Get_Photo_Val,
		.Get_Key_Val 		= m388c1g_Get_Key_Val,
		.IR_Led 			= m388c1g_IR_Led,
		.WHITE_LIGHT_Led	= m388c1g_WHITE_LIGHT_Led,
		.Alarm 				= m388c1g_Alarm,
		.RTC_Read 			= m388c1g_RTC_Read,
		.RTC_Write 			= m388c1g_RTC_Write,
		.Led_Contrl			= m388c1g_Led_Contrl,
		.Senser_Reset		= m388c1g_Senser_reset,
		.Audio_set_volume_val = m388c1g_Audio_set_volume_val,
		.Wifi_power_enable  = m388c1g_Wifi_power_enable,
		.Sd_power_enable    = m388c1g_Sd_power_enable,
	};
	
	//BSP_sysCreate();
	BSP_ContrlCreate(m388c1g_cmInterface);
}


