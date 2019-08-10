
#include "18ev2_contrl.h"
#include "../commom/bsp_commom.h"
#include "../higpio/higpio.h"
#include <hi_type.h>

typedef enum
{
    emAUDIO_HW_SPEC_IGNORE = 0,
    emAUDIO_HW_SPEC_1X = 100,      // Hi3518Ev200+38板+硅麦
    emAUDIO_HW_SPEC_2X = 101,      // Hi3518Ev200+38板+模拟麦
    emAUDIO_HW_SPEC_3X = 102,      // Hi3518Ev200+长条形板+模拟麦
    emAUDIO_HW_SPEC_4X = 200,      // Hi3516Dv100+38板+硅麦
    emAUDIO_HW_SPEC_5X = 300,      // Hi3518Ev200+P2_720单sensor板+普通麦
}emAUDIO_HW_SPEC;

static unsigned char soundPlusVal = SOUND_PLUS_CTRL_VALUE;
static int modelName = em_BSP_MODEL_NAME_PX;

static void v2_GPIO_Init(void)
{
	unsigned int reg_val = 0;

    if(modelName == em_BSP_MODEL_NAME_PX) {
        //muxpin    init IR LED ctl pin
        BSP_write_reg(IRCUT_LED_GPIO_PINMUX_ADDR, IRCUT_LED_GPIO_PINMUX_VALUE); // GPIO7_7
        //pin dir :out
        BSP_read_reg(IRCUT_LED_GPIO_DIR_ADDR, &reg_val);
        reg_val |= (1<<IRCUT_LED_GPIO_PIN);
        BSP_write_reg(IRCUT_LED_GPIO_DIR_ADDR, reg_val);
    }
    else if(modelName == em_BSP_MODEL_NAME_CX) {
        //muxpin    init WHITE LIGHT ctl pin
        BSP_write_reg(WHITE_LED_GPIO_PINMUX_ADDR, WHITE_LED_GPIO_PINMUX_VALUE); // GPIO7_7
        //pin dir :out
        BSP_read_reg(WHITE_LED_GPIO_DIR_ADDR, &reg_val);
        reg_val |= (1<<WHITE_LED_GPIO_PIN);
        BSP_write_reg(WHITE_LED_GPIO_DIR_ADDR, reg_val);
    }

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

    //muxpin    init sound pin
    BSP_write_reg(SOUND_EN_GPIO_PINMUX_ADDR, SOUND_EN_GPIO_PINMUX_VALUE); // GPIO4_5
    //pin dir :out
    BSP_read_reg(SOUND_EN_GPIO_DIR_ADDR, &reg_val);
    reg_val |= (1 << SOUND_EN_GPIO_PIN);
    BSP_write_reg(SOUND_EN_GPIO_DIR_ADDR, reg_val);

	BSP_write_reg(KEY_GPIO_PINMUX_ADDR, KEY_GPIO_PINMUX_VALUE);//init key pinmux
	BSP_read_reg(KEY_GPIO_DIR_ADDR, &reg_val);
	reg_val &= ~(1 << KEY_GPIO_PIN);    // 0表示设置输入
	BSP_write_reg(KEY_GPIO_DIR_ADDR, reg_val);
	BSP_read_reg(KEY_GPIO_PULL_UP_ADDR, &reg_val);
	reg_val |= (1 << KEY_GPIO_PULL_UP_BIT);  // 1表示上拉
	BSP_write_reg(KEY_GPIO_PULL_UP_ADDR, reg_val);

	BSP_write_reg(LED_GPIO_PINMUX_ADDR, LED_GPIO_PINMUX_VALUE);
	BSP_read_reg(LED_GPIO_DIR_ADDR, &reg_val);
	reg_val |= 1 << LED_GPIO_PIN;
	BSP_write_reg(LED_GPIO_DIR_ADDR, reg_val);

    if(modelName == em_BSP_MODEL_NAME_PX) {
        BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);//IR LED off
    }
    else if(modelName == em_BSP_MODEL_NAME_CX) {
        BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 1 - IRCUT_LED_VALUE);//IR LED off
        BSP_gpio_pin_write(WHITE_LED_GPIO_GROUP, WHITE_LED_GPIO_PIN, 1 - WHITE_LED_VALUE);//WHITE LED off
    }
	BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off

#if defined(PX_720)
    BSP_write_reg(KEY_REC_GPIO_PINMUX_ADDR, KEY_REC_GPIO_PINMUX_VALUE); // init key pinmux
    BSP_read_reg(KEY_REC_GPIO_DIR_ADDR, &reg_val);
    reg_val &= ~(1 << KEY_REC_GPIO_PIN);    // 0表示设置输入
    BSP_write_reg(KEY_REC_GPIO_DIR_ADDR, reg_val);
#else
    BSP_write_reg(P3_R_LED_GPIO_PINMUX_ADDR, P3_R_LED_GPIO_PINMUX_VALUE);//init key pinmux
	BSP_read_reg(P3_R_LED_GPIO_DIR_ADDR, &reg_val);
	reg_val |= 1 << P3_R_LED_GPIO_PIN;    // 1表示设置输出
	BSP_write_reg(P3_R_LED_GPIO_DIR_ADDR, reg_val);
#endif

	// Private Led
	BSP_write_reg(LED_PRIV_GPIO_PINMUX_ADDR, LED_PRIV_GPIO_PINMUX_VALUE);
	BSP_read_reg(LED_PRIV_GPIO_DIR_ADDR, &reg_val);
	reg_val |= 1 << LED_PRIV_GPIO_PIN;    // 1表示设置输出
	BSP_write_reg(LED_PRIV_GPIO_DIR_ADDR, reg_val);
	BSP_gpio_pin_write(LED_PRIV_GPIO_GROUP, LED_PRIV_GPIO_PIN, 1);

    BSP_write_reg(WIFI_POWER_GPIO_PINMUX_ADDR, WIFI_POWER_GPIO_PINMUX_VALUE); // init wifi power pinmux
    BSP_read_reg(WIFI_POWER_GPIO_DIR_ADDR, &reg_val);
    reg_val |= 1 << WIFI_POWER_GPIO_PIN;    // 1表示设置输出
    BSP_write_reg(WIFI_POWER_GPIO_DIR_ADDR, reg_val);
    BSP_gpio_pin_write(WIFI_POWER_GPIO_GROUP, WIFI_POWER_GPIO_PIN, 1); // wifi power en

    BSP_write_reg(SD_POWER_GPIO_PINMUX_ADDR, SD_POWER_GPIO_PINMUX_VALUE); // init sd card power pinmux
    BSP_read_reg(SD_POWER_GPIO_DIR_ADDR, &reg_val);
    reg_val |= 1 << SD_POWER_GPIO_PIN;    // 1表示设置输出
    BSP_write_reg(SD_POWER_GPIO_DIR_ADDR, reg_val);
    BSP_gpio_pin_write(SD_POWER_GPIO_GROUP, SD_POWER_GPIO_PIN, 1); // sd card power en
#if defined(PX_720)
    BSP_write_reg(LED_REC_GPIO_PINMUX_ADDR, LED_REC_GPIO_PINMUX_VALUE);
#endif

	// muxpin    bluetooth serial port
	BSP_write_reg(0x200F00C0, 0x03);
	BSP_write_reg(0x200F00C8, 0x03);

    // muxpin    bluetooth on/off
    BSP_write_reg(BT_ONOFF_GPIO_PINMUX_ADDR, BT_ONOFF_GPIO_PINMUX_VALUE);

    // bluetooth on
    // out
    BSP_read_reg(BT_ONOFF_GPIO_DIR_ADDR, &reg_val);
    reg_val |= 1 << BT_ONOFF_GPIO_PIN;    // 1表示设置输出
    BSP_write_reg(BT_ONOFF_GPIO_DIR_ADDR, reg_val);
	// low
	BSP_gpio_pin_write(BT_ONOFF_GPIO_GROUP, BT_ONOFF_GPIO_PIN, 0);
    // sleep
    usleep(500000);
    // high
    BSP_gpio_pin_write(BT_ONOFF_GPIO_GROUP, BT_ONOFF_GPIO_PIN, 1);
}

static void v2_IRCUT_Switch(bool DNmode)
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

static void v2_Speaker_Enable(bool Enable)
{
	unsigned int reg_val = 0;
    if (Enable) {

    }else{
    }
    /* 拾音器输入增益控制 */
    BSP_read_reg(SOUND_PLUS_CTRL_ADDR, &reg_val);
    reg_val &= ~(0x7f << SOUND_PLUS_CTRL_BIT);  // 清零
    reg_val |= (soundPlusVal << SOUND_PLUS_CTRL_BIT);
    BSP_write_reg(SOUND_PLUS_CTRL_ADDR, reg_val);

	//set ao enabled
	BSP_read_reg(SOUND_EN_ADDR, &reg_val);
    reg_val &= ~(1 << SOUND_AO_EN_PIN);  // 清零
    reg_val |= (Enable? SOUND_EN_VALUE:((!SOUND_EN_VALUE)&0x1) << SOUND_AO_EN_PIN);
    BSP_write_reg(SOUND_EN_ADDR, reg_val);

    //set ai enabled
	BSP_read_reg(SOUND_EN_ADDR, &reg_val);
    reg_val &= ~(1 << SOUND_AI_EN_PIN);  // 清零
    reg_val |= ((Enable? ((!SOUND_EN_VALUE)&0x1):SOUND_EN_VALUE) << SOUND_AI_EN_PIN);
    BSP_write_reg(SOUND_EN_ADDR, reg_val);

    BSP_gpio_pin_write(SOUND_EN_GPIO_GROUP, SOUND_EN_GPIO_PIN, Enable);
}

static int v2_Get_Photo_Val(void)
{
	if (0 == BSP_gpio_pin_read(IRCUT_PHOTOSWITCH_GPIO_GROUP, IRCUT_PHOTOSWITCH_GPIO_PIN)) {
		return 0;
	}else{
		return 1;
	}
}

static int v2_Get_Key_Val(int *val)
{
    if(val == NULL) {
        printf("%s failed!\n", __FUNCTION__);
        return -1;
    }
    memset(val, 1, KEY_MAX_NUM);
    int read_value = 1;
    int i = 1;

    if (0 == BSP_gpio_pin_read(KEY_GPIO_GROUP, KEY_GPIO_PIN)) {
		read_value = 0;
	}else{
		read_value = 1;
	}
    val[0] = read_value;

#if defined(PX_720)
    if (0 == BSP_gpio_pin_read(KEY_REC_GPIO_GROUP, KEY_REC_GPIO_PIN)) {
		read_value = 0;
	}else{
		read_value = 1;
	}
    val[1] = read_value;
    i = 2;
#endif

    for(; i < KEY_MAX_NUM; i++) {
        val[i] = -1;
    }

    return 0;
}

static void v2_IR_Led(bool Enable)
{
    if(modelName == em_BSP_MODEL_NAME_PX) {
        if (Enable) {
            BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 1);
        }
        else {
            BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);
        }
    }
    else if(modelName == em_BSP_MODEL_NAME_CX) {
        if (Enable) {
            BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, IRCUT_LED_VALUE);
        }
        else {
            BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 1-IRCUT_LED_VALUE);
        }
    }
}

static void v2_WHITE_LIGHT_Led(bool Enable)
{
    if(modelName == em_BSP_MODEL_NAME_CX) {
        if (Enable) {
            BSP_gpio_pin_write(WHITE_LED_GPIO_GROUP, WHITE_LED_GPIO_PIN, WHITE_LED_VALUE);
        }
        else {
            BSP_gpio_pin_write(WHITE_LED_GPIO_GROUP, WHITE_LED_GPIO_PIN, 1-WHITE_LED_VALUE);
        }
    }
}

static void v2_Led_Contrl(int LedID, bool EnableOne, bool EnableTwo)
{
	switch(LedID){
		default:
		case 0: 
            {
                BSP_gpio_pin_write(LED_GPIO_GROUP, LED_GPIO_PIN, EnableOne);//DEF_LED_ID
#if !defined(PX_720)
                if(EnableOne) {
                    BSP_gpio_pin_write(P3_R_LED_GPIO_GROUP, P3_R_LED_GPIO_PIN, 0);
                }
                else {
                    BSP_gpio_pin_write(P3_R_LED_GPIO_GROUP, P3_R_LED_GPIO_PIN, 1);
                }
#endif
            }
            break;
		case 1:
#if defined(PX_720)
            BSP_gpio_pin_write(LED_REC_GPIO_GROUP, LED_REC_GPIO_PIN, EnableOne); // REC_LED_ID

#endif
			break;		
		case 2:
			BSP_gpio_pin_write(LED_PRIV_GPIO_GROUP, LED_PRIV_GPIO_PIN,
                               EnableOne ? 0 : 1);
			break;				
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
	}
}

static void V2_Senser_reset(void)
{
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, 1-SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, 1-SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
}

//有in和out类型
static int v2_Alarm(bool OCmode)
{

}

static int v2_RTC_Read(void* arg)
{

}

static int v2_RTC_Write(void* arg)
{

}

/*
参数1 ai gain val
    麦采集音量值对应实际的增益值(注，函数内没做任何判断，所以需要传入规定的值，如10)
    10	-> 0x08 4db
    20	-> 0x10 8db
    30	-> 0x1c 14db
    40	-> 0x2c 22db
    50	-> 0x3c 30db
    60	-> 0x1e 34db
    70	-> 0x26 38db
    80	-> 0x2e 42db
    90	-> 0x3a 48db
    100	-> 0x3e 50db

参数2 ai volume val
    麦采集音量值
    10	-> 0x26
    20	-> 0x22
    30	-> 0x1e
    40	-> 0x1a
    50	-> 0x16
    60	-> 0x12
    70	-> 0xe
    80	-> 0xa
    90	-> 0x6
    100	-> 0x2
*/
/*
参数3 ao gain val
	10	-> 0x9 -3db
    20	-> 0x8 -2db
    30	-> 0x7 -1db
    40	-> 0x6 0db
    50	-> 0x5 1db
    60	-> 0x4 2db
    70	-> 0x3 3db
    80	-> 0x2 4db
    90	-> 0x1 5db
    100	-> 0x0 6db
*/
static void v2_Audio_set_volume_val(int ai_gain, int ai_vol, int ao_gain, int ao_vol)
{
    unsigned char aiPlusVal[11] = {
        0x0, 0x8, 0x10, 0x1c, 0x2c, 0x3c, 0x1e,
        0x26, 0x2e, 0x3a, 0x3e
    };

    unsigned char aoPlusVal[11] = {
        0x9, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4,
        0x3, 0x2, 0x1, 0x0
    };

    /*unsigned char volVal[11] = {
        0x7f, 0x26, 0x22, 0x1e, 0x1a, 0x16,
        0x12, 0xe, 0xa, 0x6, 0x2
    };*/
	unsigned int reg_val = 0;

	if((ai_gain >= 0) && (ai_gain <= 100)) {
		soundPlusVal = aiPlusVal[ai_gain / 10];

		/* 拾音器输入增益控制 */
	    BSP_read_reg(SOUND_PLUS_CTRL_ADDR, &reg_val);
	    reg_val &= ~(0x7f << SOUND_PLUS_CTRL_BIT);  // 清零
	    reg_val |= (soundPlusVal << SOUND_PLUS_CTRL_BIT);
	    BSP_write_reg(SOUND_PLUS_CTRL_ADDR, reg_val);
	}

	if((ao_gain >= 0) && (ao_gain <= 100)) {

		/* 输出增益控制 */
		BSP_read_reg(AO_PLUS_CTRL_ADDR, &reg_val);
		reg_val &= ~(0x7f << AO_PLUS_CTRL_BIT);	// 清零
		reg_val |= (aoPlusVal[ao_gain / 10] << AO_PLUS_CTRL_BIT);
		BSP_write_reg(AO_PLUS_CTRL_ADDR, reg_val);
	}


}

static void v2_Wifi_power_enable(bool enable)
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

static void v2_Sd_power_enable(bool enable)
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

/* 
    参数传入 1 表示新加密片
    参数传入 0 表示就加密片
*/
void BSP_ContrlInit(int val, int audioHwSpec, int model_name, bool ledEnabled)
{
	TJA_BSPCommom v2_cmInterface = {

		.GPIO_Init			= v2_GPIO_Init,
		.IRCUT_Switch 		= v2_IRCUT_Switch,
		.Speaker_Enable 	= v2_Speaker_Enable,
		.Get_Photo_Val 		= v2_Get_Photo_Val,
		.Get_Key_Val 		= v2_Get_Key_Val,
		.IR_Led 			= v2_IR_Led,
		.WHITE_LIGHT_Led	= v2_WHITE_LIGHT_Led,
		.Alarm 				= v2_Alarm,
		.RTC_Read 			= v2_RTC_Read,
		.RTC_Write 			= v2_RTC_Write,
		.Led_Contrl			= v2_Led_Contrl,
		.Senser_Reset		= V2_Senser_reset,
		.Audio_set_volume_val = v2_Audio_set_volume_val,
		.Wifi_power_enable  = v2_Wifi_power_enable,
		.Sd_power_enable    = v2_Sd_power_enable,
	};
	BSP_sysCreate();
	BSP_ContrlCreate(v2_cmInterface);
    if((audioHwSpec == emAUDIO_HW_SPEC_1X) 
        || (audioHwSpec == emAUDIO_HW_SPEC_2X)
        || (audioHwSpec == emAUDIO_HW_SPEC_5X)) {
        soundPlusVal = SOUND_PLUS_CTRL_VALUE;
    }
    else if(audioHwSpec == emAUDIO_HW_SPEC_3X) {
        soundPlusVal = 0x28;
    }
    else {
        soundPlusVal = SOUND_PLUS_CTRL_VALUE;
    }
    modelName = model_name;
}


