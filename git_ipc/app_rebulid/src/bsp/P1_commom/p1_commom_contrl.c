
#include "p1_commom_contrl.h"
#include "../commom/bsp_commom.h"
#include "../higpio/higpio.h"
#include <hi_type.h>

static void P1_COMMOM_GPIO_Init(void)
{
	uint32_t reg_val = 0;

	//muxpin	init IR LED ctl pin
	BSP_write_reg(IRCUT_LED_GPIO_PINMUX_ADDR, IRCUT_LED_GPIO_PINMUX_VALUE);//GPIO0_0
	//pin dir :out
	BSP_read_reg(IRCUT_LED_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<IRCUT_LED_GPIO_PIN);
	BSP_write_reg(IRCUT_LED_GPIO_DIR_ADDR, reg_val);

	//muxpin	init IRCUT ctrl pin1
	BSP_write_reg(NEW_IRCUT_CTRL_GPIO_PINMUX_ADDR, NEW_IRCUT_CTRL_GPIO_PINMUX_VALUE);//GPIO0_2
	//pin dir :out
	BSP_read_reg(NEW_IRCUT_CTRL_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<NEW_IRCUT_CTRL_GPIO_PIN);
	BSP_write_reg(NEW_IRCUT_CTRL_GPIO_DIR_ADDR, reg_val);

	//muxpin	init IRCUT ctrl pin2
	BSP_write_reg(IRCUT_CTRL_GPIO_PINMUX_ADDR, IRCUT_CTRL_GPIO_PINMUX_VALUE);//GPIO0_4
	//pin dir :out
	BSP_read_reg(IRCUT_CTRL_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<IRCUT_CTRL_GPIO_PIN);
	BSP_write_reg(IRCUT_CTRL_GPIO_DIR_ADDR, reg_val);

	//muxpin	PHOTO
	BSP_write_reg(IRCUT_PHOTOSWITCH_GPIO_PINMUX_ADDR, IRCUT_PHOTOSWITCH_GPIO_PINMUX_VALUE);//GPIO7_6
	//pin dir :in
	BSP_read_reg(IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR, &reg_val);
	reg_val &= ~(1<<IRCUT_PHOTOSWITCH_GPIO_PIN);
	BSP_write_reg(IRCUT_PHOTOSWITCH_GPIO_DIR_ADDR, reg_val);

	//muxpin	init sound pin
	BSP_write_reg(SOUND_EN_GPIO_PINMUX_ADDR, SOUND_EN_GPIO_PINMUX_VALUE);//GPIO0_3
	//pin dir :out
	BSP_read_reg(SOUND_EN_GPIO_DIR_ADDR, &reg_val);
	reg_val |= (1<<SOUND_EN_PIN);
	BSP_write_reg(SOUND_EN_GPIO_DIR_ADDR, reg_val);

	BSP_write_reg(KEY_GPIO_PINMUX_ADDR, KEY_GPIO_PINMUX_VALUE);//init key pinmux
	BSP_write_reg(LED_GPIO_PINMUX_ADDR, LED_GPIO_PINMUX_VALUE);
	BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);//IR LED off
	BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off
}

static void P1_COMMOM_IRCUT_Switch(bool DNmode)
{
	if (!DNmode) {

		BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off
		BSP_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN,1);
		usleep(1000*150);
		BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);
		BSP_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);
	}else{

		BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 1);//IR-CUT off
		BSP_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);
		usleep(1000*150);
		BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);
		BSP_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 0);
	}
}

static void P1_COMMOM_Speaker_Enable(bool Enable)
{
	if (Enable) {

		BSP_gpio_pin_write(SOUND_EN_GROUP, SOUND_EN_PIN, SOUND_EN_VALUE);
	}else{

		BSP_gpio_pin_write(SOUND_EN_GROUP, SOUND_EN_PIN, 1 - SOUND_EN_VALUE);
	}
}

static int P1_COMMOM_Get_Photo_Val(void)
{
	if (0 == BSP_gpio_pin_read(IRCUT_PHOTOSWITCH_GPIO_GROUP, IRCUT_PHOTOSWITCH_GPIO_PIN)) {
		return 0;
	}else{
		return 1;
	}
}

static int P1_COMMOM_Get_Key_Val(int *val)
{
    if(val == NULL) {
        printf("%s failed!\n", __FUNCTION__);
        return -1;
    }
    memset(val, 1, KEY_MAX_NUM);
    int read_value = 1;
    int i = 0;

    if (0 == BSP_gpio_pin_read(KEY_GPIO_GROUP, KEY_GPIO_PIN)) {
		read_value = 0;
	}else{
		read_value = 1;
	}
    val[0] = read_value;

    for(i = 1; i < KEY_MAX_NUM; i++) {
        val[i] = -1;
    }

    return 0;
}

static void P1_COMMOM_IR_Led(bool Enable)
{
	if (!Enable) {

		BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 1-IRCUT_LED_VALUE);
	}else{

		BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, IRCUT_LED_VALUE);
	}
}

static void P1_COMMOM_WHITE_LIGHT_Led(bool Enable)
{

}

static void P1_COMMOM_Led_Contrl(int LedID, bool EnableOne, bool EnableTwo)
{
	switch(LedID){
		default:
		case 0:	
			BSP_gpio_pin_write(LED_GPIO_GROUP, LED_GPIO_PIN, EnableOne);//DEF_LED_ID
			break;
		case 1:
			break;		
		case 2:
			break;				
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
	}
}

static void P1_COMMOM_Senser_reset(void)
{
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, 1-SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, 1-SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
}

//有in和out类型
static int P1_COMMOM_Alarm(bool OCmode)
{

}

static int P1_COMMOM_RTC_Read(void* arg)
{

}

static int P1_COMMOM_RTC_Write(void* arg)
{

}

static void P1_COMMOM_Audio_set_volume_val(int ai_gain, int ai_vol, int ao_gain, int ao_vol)
{

}

static void P1_COMMOM_Wifi_power_enable(bool enable)
{

}

static void P1_COMMOM_Sd_power_enable(bool enable)
{

}

void BSP_ContrlInit(int val, int audioHwSpec, int model_name, int ledEnabled)
{
	TJA_BSPCommom P1_COMMOM_cmInterface = {

		.GPIO_Init			= P1_COMMOM_GPIO_Init,
		.IRCUT_Switch 		= P1_COMMOM_IRCUT_Switch,
		.Speaker_Enable 	= P1_COMMOM_Speaker_Enable,
		.Get_Photo_Val 		= P1_COMMOM_Get_Photo_Val,
		.Get_Key_Val 		= P1_COMMOM_Get_Key_Val,
		.IR_Led 			= P1_COMMOM_IR_Led,
		.WHITE_LIGHT_Led	= P1_COMMOM_WHITE_LIGHT_Led,
		.Alarm 				= P1_COMMOM_Alarm,
		.RTC_Read 			= P1_COMMOM_RTC_Read,
		.RTC_Write 			= P1_COMMOM_RTC_Write,
		.Led_Contrl			= P1_COMMOM_Led_Contrl,
		.Senser_Reset		= P1_COMMOM_Senser_reset,
		.Audio_set_volume_val = P1_COMMOM_Audio_set_volume_val,
		.Wifi_power_enable  = P1_COMMOM_Wifi_power_enable,
		.Sd_power_enable    = P1_COMMOM_Sd_power_enable,
	};
	
	BSP_sysCreate();
	BSP_ContrlCreate(P1_COMMOM_cmInterface);
}


