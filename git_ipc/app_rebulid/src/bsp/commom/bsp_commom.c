
#include "bsp_commom.h"

static TJA_BSPCommom BSP_contrl;
static CMhandle BSP_contrl_handle = NULL;


void BSP_GPIO_Init(void)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return ;
	}
	
	BSP_contrl_handle->GPIO_Init();
}

void BSP_IRCUT_Switch(bool DNmode)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return ;
	}

	BSP_contrl_handle->IRCUT_Switch(DNmode);
}

void BSP_Speaker_Enable(bool Enable)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return ;
	}

	BSP_contrl_handle->Speaker_Enable(Enable);
}

int BSP_Get_Photo_Val(void)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return -1;
	}

	return BSP_contrl_handle->Get_Photo_Val();
}

/* 功能:获取所有按键状态值，最大可返回KEY_MAX_NUM个按键状态 */
/* 参数，返回KEY_MAX_NUM大小的数组 val[0]表示第一个按键，以此类推 */
/* 按键状态，0->down，1->up 如果硬件上没有对应的按键状态 -> -1 */
int BSP_Get_Key_Val(int *val)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return -1;
	}

	return BSP_contrl_handle->Get_Key_Val(val);
}



void BSP_IR_Led(bool Enable)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return ;
	}

	BSP_contrl_handle->IR_Led(Enable);
}

void BSP_WHITE_LIGHT_Led(bool Enable)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return ;
	}

	BSP_contrl_handle->WHITE_LIGHT_Led(Enable);
}

//有in和out类型 IOmode true为out, false为in
int BSP_Alarm(bool IOmode, int AlarmID, bool OutValue)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return -1;
	}

	return BSP_contrl_handle->Alarm(IOmode, AlarmID, OutValue);
}

int BSP_RTC_Read(void* arg)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return -1;
	}

	return BSP_contrl_handle->RTC_Read(arg);
}

int BSP_RTC_Write(void* arg)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return -1;
	}

	return BSP_contrl_handle->RTC_Write(arg);
}

void BSP_Led_Contrl(int LedID, bool EnableOne, bool EnableTwo)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return ;
	}

	BSP_contrl_handle->Led_Contrl(LedID, EnableOne, EnableTwo);
}

int BSP_Senser_reset(void)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return -1;
	}

	BSP_contrl_handle->Senser_Reset();
}

int BSP_ContrlDestroy(void)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return -1;
	}

}

void BSP_Audio_set_volume_val(int ai_gain, int ai_vol, int ao_gain, int ao_vol)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return ;
	}

	BSP_contrl_handle->Audio_set_volume_val(ai_gain, ai_vol, ao_gain, ao_vol);

}

void BSP_Wifi_power_enable(bool Enable)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return ;
	}

	BSP_contrl_handle->Wifi_power_enable(Enable);
}

void BSP_Sd_power_enable(bool Enable)
{
	if (!BSP_contrl_handle) {
		printf("Handle is Null, Error: %s, %s, %d\n", __FILE__,  __func__, __LINE__);
		return ;
	}

	BSP_contrl_handle->Sd_power_enable(Enable);
}

void BSP_ContrlCreate(TJA_BSPCommom Ctr_arg)
{
	TJA_BSPCommom* Public = BSP_contrl_handle = &BSP_contrl;

	Public->GPIO_Init			= Ctr_arg.GPIO_Init;
	Public->IRCUT_Switch 		= Ctr_arg.IRCUT_Switch;
	Public->Speaker_Enable 		= Ctr_arg.Speaker_Enable;
	Public->Get_Photo_Val		= Ctr_arg.Get_Photo_Val;
	Public->Get_Key_Val			= Ctr_arg.Get_Key_Val;
	Public->IR_Led 				= Ctr_arg.IR_Led;
	Public->WHITE_LIGHT_Led		= Ctr_arg.WHITE_LIGHT_Led;
	Public->Alarm 				= Ctr_arg.Alarm;
	Public->RTC_Read 			= Ctr_arg.RTC_Read;
	Public->RTC_Write 			= Ctr_arg.RTC_Write;
	Public->Led_Contrl			= Ctr_arg.Led_Contrl;
	Public->Senser_Reset		= Ctr_arg.Senser_Reset;
    Public->Audio_set_volume_val = Ctr_arg.Audio_set_volume_val;
    Public->Wifi_power_enable   = Ctr_arg.Wifi_power_enable;
    Public->Sd_power_enable     = Ctr_arg.Sd_power_enable;
}


