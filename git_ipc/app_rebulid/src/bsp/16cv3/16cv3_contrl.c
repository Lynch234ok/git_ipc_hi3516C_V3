
#include "16cv3_contrl.h"
#include "../commom/bsp_commom.h"
#include "../higpio/higpio.h"
#include <hi_type.h>

typedef unsigned int uint32_t;
static int sound_input_gain_level = 1;
static bool g_ledEnabled = true;
static void v2_GPIO_Init(void)
{
	uint32_t reg_val = 0;

#if !defined(HI3516E_V1)
		//muxpin	init IR LED ctl pin
		BSP_write_reg(IRCUT_LED_GPIO_PINMUX_ADDR, IRCUT_LED_GPIO_PINMUX_VALUE); // GPIO6_7
		//pin dir :out
		BSP_read_reg(IRCUT_LED_GPIO_DIR_ADDR, &reg_val);
		reg_val |= (1<<IRCUT_LED_GPIO_PIN);
		BSP_write_reg(IRCUT_LED_GPIO_DIR_ADDR, reg_val);
#endif

		//muxpin	init WHITE LED ctl pin
		BSP_write_reg(WHITE_LED_GPIO_PINMUX_ADDR, WHITE_LED_GPIO_PINMUX_VALUE); // GPIO6_6
		//pin dir :out
		BSP_read_reg(WHITE_LED_GPIO_DIR_ADDR, &reg_val);
		reg_val |= (1<<WHITE_LED_GPIO_PIN);
		BSP_write_reg(WHITE_LED_GPIO_DIR_ADDR, reg_val);

		//muxpin	init WHITE LED ctl pin
		BSP_write_reg(WHITE_LED_GPIO6_7_PINMUX_ADDR, WHITE_LED_GPIO6_7_PINMUX_VALUE); // GPIO6_7
		//pin dir :out
		BSP_read_reg(WHITE_LED_GPIO6_7_DIR_ADDR, &reg_val);
		reg_val |= (1<<WHITE_LED_GPIO6_7_PIN);
		BSP_write_reg(WHITE_LED_GPIO6_7_DIR_ADDR, reg_val);
	
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
		
#if defined(HI3516E_V1)	
		//For 16Ev100
		//muxpin	init sound pin
		BSP_write_reg(SOUND_EN_GPIO_PINMUX_ADDR, SOUND_EN_GPIO_PINMUX_VALUE);// GPIO8_0
		//pin dir :out
		BSP_read_reg(SOUND_EN_GPIO_DIR_ADDR, &reg_val);
		reg_val |= (1 << SOUND_EN_GPIO_PIN);
		BSP_write_reg(SOUND_EN_GPIO_DIR_ADDR, reg_val);
#endif
		
	
		BSP_write_reg(KEY_GPIO_PINMUX_ADDR, KEY_GPIO_PINMUX_VALUE);//init key pinmux
		BSP_read_reg(KEY_GPIO_DIR_ADDR, &reg_val);
		reg_val &= ~(1 << KEY_GPIO_PIN);	// 0±Ì æ…Ë÷√ ‰»Î
		BSP_write_reg(KEY_GPIO_DIR_ADDR, reg_val);
		BSP_read_reg(KEY_GPIO_PULL_UP_ADDR, &reg_val);
		reg_val |= (1 << KEY_GPIO_PULL_UP_BIT);  // 1±Ì æ…œ¿≠
		BSP_write_reg(KEY_GPIO_PULL_UP_ADDR, reg_val);

        if(true == g_ledEnabled)
        {
            BSP_write_reg(LED_GPIO_PINMUX_ADDR, LED_GPIO_PINMUX_VALUE);
            BSP_read_reg(LED_GPIO_DIR_ADDR, &reg_val);
            reg_val |= 1 << LED_GPIO_PIN;
            BSP_write_reg(LED_GPIO_DIR_ADDR, reg_val);
            BSP_gpio_pin_write(LED_GPIO_GROUP, LED_GPIO_PIN, 0); // led run off
        }

#if !defined(HI3516E_V1)
		BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 0);//IR LED off
#endif

		BSP_gpio_pin_write(WHITE_LED_GPIO_GROUP, WHITE_LED_GPIO_PIN, 1);//WHITE LED off
		BSP_gpio_pin_write(WHITE_LED_GPIO6_7_GROUP, WHITE_LED_GPIO6_7_PIN, 1);//WHITE LED off
		BSP_gpio_pin_write(IRCUT_CTRL_GPIO_GROUP, IRCUT_CTRL_GPIO_PIN, 0);//IR-CUT off

		//sensor reset muxpin
		BSP_write_reg(SENSOR_RESET_GPIO_PINMUX_ADDR, SENSOR_RESET_GPIO_PINMUX_VALUE); // GPIO0_7

        // wifi power
        BSP_read_reg(WIFI_POWER_GPIO_DIR_ADDR, &reg_val);
        reg_val |= 1 << WIFI_POWER_GPIO_PIN;
        BSP_write_reg(WIFI_POWER_GPIO_DIR_ADDR, reg_val);
        BSP_gpio_pin_write(WIFI_POWER_GPIO_GROUP, WIFI_POWER_GPIO_PIN, 1); // wifi power en

#if !defined(HI3516E_V1)
        BSP_write_reg(SD_POWER_GPIO_PINMUX_ADDR, SD_POWER_GPIO_PINMUX_VALUE); // init sd card power pinmux
        BSP_read_reg(SD_POWER_GPIO_DIR_ADDR, &reg_val);
        reg_val |= 1 << SD_POWER_GPIO_PIN;
        BSP_write_reg(SD_POWER_GPIO_DIR_ADDR, reg_val);
        BSP_gpio_pin_write(SD_POWER_GPIO_GROUP, SD_POWER_GPIO_PIN, 1); // sd card power en
#endif

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
	uint32_t reg_val = 0;
		if (Enable) {
	
		}else{
		}
		uint32_t  sound_plus_ctrl_value = 0;
		if(1 == sound_input_gain_level){
			sound_plus_ctrl_value = SOUND_PLUS_CTRL_VALUE_LARGE;
		}else{
			sound_plus_ctrl_value = SOUND_PLUS_CTRL_VALUE; //default value
		}

		/*  ∞“Ù∆˜ ‰»Î‘ˆ“Êøÿ÷∆ */
		BSP_read_reg(SOUND_PLUS_CTRL_ADDR, &reg_val);
		reg_val &= ~(0x7f << SOUND_PLUS_CTRL_BIT);	// «Â¡„
		reg_val |= (sound_plus_ctrl_value << SOUND_PLUS_CTRL_BIT);
		BSP_write_reg(SOUND_PLUS_CTRL_ADDR, reg_val);

		/* acodec line in chn select*/
		BSP_read_reg(SOUND_PLUS_CTRL_ADDR, &reg_val);
		reg_val &= ~(0xFF << SOUND_CHN_SET_BIT);	// «Â¡„
		reg_val |= (SOUND_CHN_SET_VALUE << SOUND_CHN_SET_BIT);
		BSP_write_reg(SOUND_PLUS_CTRL_ADDR, reg_val);
	
		//set ao enabled
		BSP_read_reg(SOUND_EN_ADDR, &reg_val);
		reg_val &= ~(1 << SOUND_AO_EN_PIN);  // «Â¡„
		reg_val |= (Enable? SOUND_EN_VALUE:((!SOUND_EN_VALUE)&0x1) << SOUND_AO_EN_PIN);
		BSP_write_reg(SOUND_EN_ADDR, reg_val);
	
		//set ai enabled
		BSP_read_reg(SOUND_EN_ADDR, &reg_val);
		
		reg_val &= ~(1 << SOUND_AI_EN_PIN);  // «Â¡„
		reg_val |= ((Enable? ((!SOUND_EN_VALUE)&0x1):SOUND_EN_VALUE) << SOUND_AI_EN_PIN);
		BSP_write_reg(SOUND_EN_ADDR, reg_val);
		
#if defined(HI3516E_V1)	
		//For 16Ev100
		//ao
		BSP_read_reg(SOUND_AO_PLUS_CTRL_ADDR, &reg_val);
		reg_val &= ~(0x7f << SOUND_AO_PLUS_CTRL_BIT); 
		reg_val |= (SOUND_AO_PLUS_CTRL_VALUE << SOUND_AO_PLUS_CTRL_BIT);
		BSP_write_reg(SOUND_AO_PLUS_CTRL_ADDR, reg_val);

#endif

    BSP_gpio_pin_write(SOUND_EN_GPIO_GROUP, SOUND_EN_GPIO_PIN, Enable);

}

#ifdef IRCUT_DETECT_BYADC
int ADC_Value(void)
{
    static int initflg = 0;
    unsigned long tmpReg;

    if(0 == initflg)
    {
        initflg = !initflg;
        BSP_write_reg(0x12040104, 0x1); //SAR_ADC_CH2, GPIO8_2
        usleep(10*1000);
        BSP_write_reg(0x120e0000, 0x20400);
        BSP_write_reg(0x120e0024, 0x3fc);      // active 8bit
        BSP_write_reg(0x120e0028, 0x3ff);
    }

    BSP_write_reg(0x120e001c, 0xf); //Start ADC
    usleep(15*1000);
    BSP_read_reg(0x120e0034, &tmpReg);
    return ((tmpReg & 0x3FF) >> 2); //Get Value
}
#endif

static int v2_Get_Photo_Val(void)
{
#ifdef IRCUT_DETECT_BYADC
    static int gDayNight = 0;
    int tmpValue = ADC_Value();
    int ii = 0;

    for(ii = 0; ii < 5; ii ++)
    {
        tmpValue += ADC_Value();
        tmpValue /= 2;
    }

    //      printf("\n --%s tmpValue = 0x%02X gDayNight = %s --\n", __FUNCTION__, tmpValue, gDayNight ? "Night" : "Day");

    if(tmpValue >= 0xEB)    // Night
    {
        gDayNight = 1;
    }
    if(tmpValue < 0xE7)     // Day
    {
        gDayNight = 0;
    }

    return gDayNight;
#else
	if (0 == BSP_gpio_pin_read(IRCUT_PHOTOSWITCH_GPIO_GROUP, IRCUT_PHOTOSWITCH_GPIO_PIN)) {
			return 0;
		}else{
			return 1;
		}
#endif
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

    for(; i < KEY_MAX_NUM; i++) {
        val[i] = -1;
    }

    return 0;

}

static void v2_IR_Led(bool Enable)
{
#if !defined(HI3516E_V1)
	if (Enable) {
		BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, IRCUT_LED_VALUE);
	}else{
		BSP_gpio_pin_write(IRCUT_LED_GPIO_GROUP, IRCUT_LED_GPIO_PIN, 1-IRCUT_LED_VALUE);
	}
#endif

}

static void v2_WHITE_LIGHT_Led(bool Enable)
{
	if(Enable) {
		BSP_gpio_pin_write(WHITE_LED_GPIO_GROUP, WHITE_LED_GPIO_PIN, 1 - WHITE_LED_VALUE);
		BSP_gpio_pin_write(WHITE_LED_GPIO6_7_GROUP, WHITE_LED_GPIO6_7_PIN, 1 - WHITE_LED_GPIO6_7_VALUE);
	}else{
		BSP_gpio_pin_write(WHITE_LED_GPIO_GROUP, WHITE_LED_GPIO_PIN, WHITE_LED_VALUE);
		BSP_gpio_pin_write(WHITE_LED_GPIO6_7_GROUP, WHITE_LED_GPIO6_7_PIN, WHITE_LED_GPIO6_7_VALUE);
	}
}

static void v2_Led_Contrl(int LedID, bool EnableOne, bool EnableTwo)
{
    switch(LedID) {
        default:
        case 0: 
            {
                if(true == g_ledEnabled)
                {
                    BSP_gpio_pin_write(LED_GPIO_GROUP, LED_GPIO_PIN, EnableOne);//DEF_LED_ID
                }
            }
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

static void V2_Senser_reset(void)
{
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, 1-SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);
	BSP_gpio_pin_write(SENSOR_RESET_GPIO_GOUP, SENSOR_RESET_GPIO_PIN, 1-SENSOR_RESET_GPIO_VALUE); //reset sensor 
	usleep(2000);


}

//”–in∫Õout¿‡–Õ
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
≤Œ ˝1 ai volume val
    ¬Û≤…ºØ“Ù¡ø÷µ∂‘”¶ µº µƒ‘ˆ“Ê÷µ(◊¢£¨∫Ø ˝ƒ⁄√ª◊ˆ»Œ∫Œ≈–∂œ£¨À˘“‘–Ë“™¥´»ÎπÊ∂®µƒ÷µ£¨»Á10)
    10	-> 0x5
    20	-> 0x6
    30	-> 0x7
    40	-> 0x8
    50	-> 0x9
    60	-> 0xa
    70	-> 0xb
    80	-> 0xc
    90	-> 0xd
    100	-> 0xe
    æ…¬ÛΩ®“È π”√“Ù¡ø÷µ40
    –¬¬ÛΩ®“È π”√“Ù¡ø÷µ50
*/
static void v2_Audio_set_volume_val(int ai_gain, int ai_vol, int ao_gain, int ao_vol)
{
//    unsigned char plusVal[10] = {
//        0x5, 0x6, 0x7, 0x8, 0x9,
//        0xa, 0xb, 0xc, 0xd, 0xe
//    };
//
	uint32_t reg_val = 0;
	printf("Audio set volume val.\n");

	if( 50 == ai_gain){	//30db
	    sound_input_gain_level = 0;
		/*ÊãæÈü≥Âô®Èü≥È¢ëÂ¢ûÁõä*/
		BSP_read_reg(SOUND_PLUS_CTRL_ADDR, &reg_val);
		reg_val &= ~(0x7f << SOUND_PLUS_CTRL_BIT);	//Ê∏ÖÈõ∂
		reg_val |= (SOUND_PLUS_CTRL_VALUE << SOUND_PLUS_CTRL_BIT);
		BSP_write_reg(SOUND_PLUS_CTRL_ADDR, reg_val);
	}else if(100 == ai_gain){	 // 60db
	     sound_input_gain_level = 1;
		 /*ÊãæÈü≥Âô®Èü≥È¢ëÂ¢ûÁõä*/
		 BSP_read_reg(SOUND_PLUS_CTRL_ADDR, &reg_val);
		 reg_val &= ~(0x7f << SOUND_PLUS_CTRL_BIT);  //Ê∏ÖÈõ∂
		 reg_val |= (SOUND_PLUS_CTRL_VALUE_LARGE << SOUND_PLUS_CTRL_BIT);
		 BSP_write_reg(SOUND_PLUS_CTRL_ADDR, reg_val);
	}
}

static void hi3516cv3_Wifi_power_enable(bool enable)
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

static void hi3516cv3_Sd_power_enable(bool enable)
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
    ≤Œ ˝¥´»Î 1 ±Ì æ–¬º”√‹∆¨
    ≤Œ ˝¥´»Î 0 ±Ì ææÕº”√‹∆¨
*/
void BSP_ContrlInit(int val, int audioHwSpec, int model_name, bool ledEnabled)
{
	printf("%s-%d\n", __FUNCTION__, __LINE__);

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
		.Wifi_power_enable  = hi3516cv3_Wifi_power_enable,
		.Sd_power_enable    = hi3516cv3_Sd_power_enable,
	};
	BSP_sysCreate();
	BSP_ContrlCreate(v2_cmInterface);

	g_ledEnabled = ledEnabled;

}


