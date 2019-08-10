
#include "hi3516d_contrl.h"
#include "../commom/bsp_commom.h"
#include "../higpio/higpio.h"
#include <hi_type.h>

#define NEW_ENCRYP_CHIP     (1)
#define OLD_ENCRYP_CHIP     (0)

static int encrypChipFlag = OLD_ENCRYP_CHIP;
static unsigned char soundPlusVal = SOUND_PLUS_CTRL_VALUE;

static void Hi3516d_GPIO_Init(void)
{
	unsigned int reg_val = 0;

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

	BSP_write_reg(KEY_GPIO_PINMUX_ADDR, KEY_GPIO_PINMUX_VALUE);//init key pinmux
	BSP_write_reg(KEY_REC_GPIO_PINMUX_ADDR, KEY_REC_GPIO_PINMUX_VALUE);//init key pinmux
	BSP_write_reg(LED_GPIO_PINMUX_ADDR, LED_GPIO_PINMUX_VALUE);
    BSP_write_reg(LED_REC_GPIO_PINMUX_ADDR, LED_REC_GPIO_PINMUX_VALUE);
	BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);//IR LED off
	BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off

	BSP_write_reg(AO_POWER_GPIO_PINMUX_ADDR, AO_POWER_GPIO_PINMUX_VALUE); // init ao power pinmux
    BSP_gpio_pin_write(AO_POWER_GPIO_GROUP, AO_POWER_GPIO_PIN, 1); // ao power en

    BSP_write_reg(WIFI_POWER_GPIO_PINMUX_ADDR, WIFI_POWER_GPIO_PINMUX_VALUE); // init wifi power pinmux
    BSP_read_reg(WIFI_POWER_GPIO_DIR_ADDR, &reg_val);
    reg_val |= 1 << WIFI_POWER_GPIO_PIN;
    BSP_write_reg(WIFI_POWER_GPIO_DIR_ADDR, reg_val);
    BSP_gpio_pin_write(WIFI_POWER_GPIO_GROUP, WIFI_POWER_GPIO_PIN, 1); // wifi power en

    BSP_write_reg(SD_POWER_GPIO_PINMUX_ADDR, SD_POWER_GPIO_PINMUX_VALUE); // init sd card power pinmux
    BSP_gpio_pin_write(SD_POWER_GPIO_GROUP, SD_POWER_GPIO_PIN, 1); // sd card power en
}

static void Hi3516d_IRCUT_Switch(bool DNmode)
{
	if (!DNmode) {

		BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off
		BSP_gpio_pin_write(NEW_IRCUT_CTRL_GPIO_GROUP, NEW_IRCUT_CTRL_GPIO_PIN, 1);
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

static void Hi3516d_Speaker_Enable(bool Enable)
{
    unsigned int reg_val = 0;
    if (Enable) {

    }else{
    }
    // 音量控制
    BSP_read_reg(SOUND_VOL_CTRL_ADDR, &reg_val);
    reg_val &= ~(0x7f << SOUND_VOL_CTRL_BIT);  // 清零
    reg_val |= (0x00 << SOUND_VOL_CTRL_BIT);
    BSP_write_reg(SOUND_VOL_CTRL_ADDR, reg_val);

    /* 拾音器输入增益控制 */
    BSP_read_reg(SOUND_PLUS_CTRL_ADDR, &reg_val);
    reg_val &= ~(0x7f << SOUND_PLUS_CTRL_BIT);  // 清零
    reg_val |= (soundPlusVal << SOUND_PLUS_CTRL_BIT);
    BSP_write_reg(SOUND_PLUS_CTRL_ADDR, reg_val);

	//set ao enabled
	BSP_read_reg(SOUND_EN_GPIO_PINMUX_ADDR, &reg_val);
    reg_val &= ~(1 << SOUND_AO_EN_PIN);  // 清零
    reg_val |= (Enable? SOUND_EN_VALUE:((!SOUND_EN_VALUE)&0x1) << SOUND_AO_EN_PIN);
    BSP_write_reg(SOUND_EN_GPIO_PINMUX_ADDR, reg_val);

    //set ai enabled
	BSP_read_reg(SOUND_EN_GPIO_PINMUX_ADDR, &reg_val);
    reg_val &= ~(1 << SOUND_AI_EN_PIN);  // 清零
    reg_val |= ((Enable? ((!SOUND_EN_VALUE)&0x1):SOUND_EN_VALUE) << SOUND_AI_EN_PIN);
    BSP_write_reg(SOUND_EN_GPIO_PINMUX_ADDR, reg_val);

}

static int Hi3516d_Get_Photo_Val(void)
{
	if (0 == BSP_gpio_pin_read(IRCUT_PHOTOSWITCH_GPIO_GROUP, IRCUT_PHOTOSWITCH_GPIO_PIN)) {
		return 0;
	}else{
		return 1;
	}
}

static int Hi3516d_Get_Key_Val(int *val)
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

    if (0 == BSP_gpio_pin_read(KEY_REC_GPIO_GROUP, KEY_REC_GPIO_PIN)) {
		read_value = 0;
	}else{
		read_value = 1;
	}
    val[1] = read_value;

    for(i = 2; i < KEY_MAX_NUM; i++) {
        val[i] = -1;
    }

    return 0;
}

static void Hi3516d_IR_Led(bool Enable)
{
#if defined(PX_720)
    Enable = !Enable;
#endif
	if (!Enable) {

		BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 1-IRCUT_LED_VALUE);
	}else{

		BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, IRCUT_LED_VALUE);
	}
}

static void Hi3516d_WHITE_LIGHT_Led(bool Enable)
{

}

static void Hi3516d_Led_Contrl(int LedID, bool EnableOne, bool EnableTwo)
{
	switch(LedID){
		default:
		case 0:	
			BSP_gpio_pin_write(LED_GPIO_GROUP, LED_GPIO_PIN, EnableOne);//DEF_LED_ID
			break;
		case 1:
            BSP_gpio_pin_write(LED_REC_GPIO_GROUP, LED_REC_GPIO_PIN, EnableOne);//REC_LED_ID
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

static void Hi3516d_Senser_reset(void)
{
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, 1-SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, 1-SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
}

//有in和out类型
static int Hi3516d_Alarm(bool OCmode)
{

}

static int Hi3516d_RTC_Read(void* arg)
{

}

static int Hi3516d_RTC_Write(void* arg)
{

}

/*
参数1 ai volume val
    麦采集音量值对应实际的增益值(注，函数内没做任何判断，所以需要传入规定的值，如10)
    10	-> 0x5
    20	-> 0x6
    30	-> 0x7
    40	-> 0x8
    50	-> 0x9
    60	-> 0xa
    70	-> 0xb
    80	-> 0xc
    90  -> 0xd
    100 -> 0xe
    旧麦建议使用音量值40
    新麦建议使用音量值50
*/
static void Hi3516d_Audio_set_volume_val(int ai_gain, int ai_vol, int ao_gain, int ao_vol)
{
#if 0
    unsigned char plusVal[11] = {
        0x4, 0x5, 0x6, 0x7, 0x8, 0x9,
        0xa, 0xb, 0xc, 0xd, 0xe
    };

	if((ai_gain >= 0) && (ai_gain <= 100)) {
		soundPlusVal = plusVal[ai_gain / 10];
	}

	 // 音量控制
	unsigned int reg_val = 0;
    BSP_read_reg(SOUND_VOL_CTRL_ADDR, &reg_val);
    reg_val &= ~(0x7f << SOUND_VOL_CTRL_BIT);  // 清零
    if(encrypChipFlag == NEW_ENCRYP_CHIP) {
        reg_val |= (NEW_SOUND_VOL_VALUE << SOUND_VOL_CTRL_BIT);
    }
    else {
        reg_val |= (OLD_SOUND_VOL_VALUE << SOUND_VOL_CTRL_BIT);
    }
    BSP_write_reg(SOUND_VOL_CTRL_ADDR, reg_val);
#endif
}

static void Hi3516d_Wifi_power_enable(bool enable)
{
    int val = 0;
    if(enable) {
        val = 1;
    }
    else {
        val = 0;
    }
    BSP_gpio_pin_write(WIFI_POWER_GPIO_GROUP, WIFI_POWER_GPIO_PIN, val); // wifi power en

}

static void Hi3516d_Sd_power_enable(bool enable)
{
    int val = 0;
    if(enable) {
        val = 1;
    }
    else {
        val = 0;
    }
    BSP_gpio_pin_write(SD_POWER_GPIO_GROUP, SD_POWER_GPIO_PIN, val); // sd card power en

}

void BSP_ContrlInit(int val, int audioHwSpec, int model_name, bool ledEnabled)
{
	TJA_BSPCommom Hi3516d_cmInterface = {

		.GPIO_Init			= Hi3516d_GPIO_Init,
		.IRCUT_Switch 		= Hi3516d_IRCUT_Switch,
		.Speaker_Enable 	= Hi3516d_Speaker_Enable,
		.Get_Photo_Val 		= Hi3516d_Get_Photo_Val,
		.Get_Key_Val 		= Hi3516d_Get_Key_Val,
		.IR_Led 			= Hi3516d_IR_Led,
		.WHITE_LIGHT_Led	= Hi3516d_WHITE_LIGHT_Led,
		.Alarm 				= Hi3516d_Alarm,
		.RTC_Read 			= Hi3516d_RTC_Read,
		.RTC_Write 			= Hi3516d_RTC_Write,
		.Led_Contrl			= Hi3516d_Led_Contrl,
		.Senser_Reset		= Hi3516d_Senser_reset,
		.Audio_set_volume_val = Hi3516d_Audio_set_volume_val,
		.Wifi_power_enable  = Hi3516d_Wifi_power_enable,
		.Sd_power_enable    = Hi3516d_Sd_power_enable,
	};
	
	BSP_sysCreate();
	BSP_ContrlCreate(Hi3516d_cmInterface);
    encrypChipFlag = val;
}


