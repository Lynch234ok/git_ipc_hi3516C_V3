
/* ‰∏¥Êó∂‰ΩøÁî®HI3516E_V1ÂÆèÂå∫ÂàÜÔºåÂêéÈù¢ÁÅØÊ≥°‰øÆÊîπÂêéÁöÑ‰ª£Á†ÅÂêàÂπ∂ÂêéÔºåÂèØ‰ª•ÂéªÊéâ */
#if  !defined(HI3516E_V1)

#if defined (LED_PWM)

#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <bsp/pwm.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include "generic.h"
#include "app_debug.h"
#include "netsdk_json.h"
#include "netsdk_private.h"
#include "led_pwm.h"

#define LED_PWM_SET_JSON        "/media/conf/led_pwm.json"          // ”√¿¥‘∂≥Ã…Ë÷√±£¥Ê π”√
#define LED_PWM_DEFAULTS_JSON   "/media/conf/custom_led_pwm.json"   // ƒ¨»œ≈‰÷√
#define LED_PWM_CUSTOM_JSON     "/media/tf/custom_led_pwm.json"     // ≤˙≤‚µƒ‘≠ ºŒƒº˛£¨∂®÷∆≤Œ ˝£¨‘⁄tfø®÷–”–¥ÀŒƒº˛æÕ∞—øΩ±¥µΩ…Ë±∏

typedef enum
{
	emLED_PWM_PRODUCT_STANDARD = 0,
	emLED_PWM_PRODUCT_AIDIE
}emLED_PWM_PRODUCT;

typedef struct led_pwm_attr
{
    stLED_PWM_config ledPwmConfig;
    //pthread_mutex_t mutex;
}st_led_pwm_attr, *lp_led_pwm_attr;

//static char led_type[5] = "mono";//test

static st_led_pwm_attr st_ledPwmAttr;

static pthread_t led_pwm_test_tid = (pthread_t)NULL;
static const ST_NSDK_MAP_STR_DEC led_pwm_product_map[] = {
	{"standard", emLED_PWM_PRODUCT_STANDARD},
	{"aiDie", emLED_PWM_PRODUCT_AIDIE}
};

/* Sets the duty cycle of the one PWM channel.
 * channel: PWM channel.
 * duty_cycle: [0, 100]
 */
static void pwm_duty_cycle_set(uint8_t channel, uint8_t duty_cycle)
{
#define PWM_PERIOD 	7500	/* Clock = 3MHz. PWM frequency = 3MHz / 7500 = 400Hz. */

	if(duty_cycle > 100)
    {
        duty_cycle = 100;
    }

    uint16_t pulse_width;
    if(duty_cycle == 0)
    {
        pulse_width = 1;
    }
    else if(duty_cycle == 100)
    {
        pulse_width = PWM_PERIOD; /* To ensure it reaches its best. */
    }
    else
    {
        pulse_width = duty_cycle * PWM_PERIOD / 100; //4000 + 225 * duty_cycle / 10;/* 16 / 100 * PWM_PERIOD + (PWM_PERIOD / 100 * (25 - 16)) * (duty_cycle / 100) */
    }

    PWM_DATA_S stPwmData = {channel, pulse_width, PWM_PERIOD, 1};
    int fd = -1;
    fd = open("/dev/pwm", 0);
    if(fd < 0)
    {
        return;
    }
    ioctl(fd, PWM_CMD_WRITE, &stPwmData);
    close(fd);
}

#if 0
#define LED_TYPE_RGB "RGB"
#define LED_COLOUR_CONFIG_FILE "/media/conf/led_colour.conf"
#define LED_COLOUR_DEFAULT 0xFFFFFF/* white */
#define LED_PWM_R_CHANNEL 1
#define LED_PWM_G_CHANNEL 2
#define LED_PWM_B_CHANNEL 3

/* Saves the colour to config file.
 * colour: RGB value.
 */
void led_colour_save(uint32_t colour)
{
    uint8_t B = (uint8_t) colour;
    colour >>= 8;
    uint8_t G = (uint8_t) colour;
    colour >>= 8;
    uint8_t R = (uint8_t) colour;

    FILE *config_file = fopen(LED_COLOUR_CONFIG_FILE, "wt");
    if(config_file)
    {
        fprintf(config_file, "%02X%02X%02X", R, G, B);
        fclose(config_file);
    }
}

/* Reads colour setting from config file.
 * Returns RGB colour.
 */
uint32_t led_colour_get(void)
{
    FILE *config_file = fopen(LED_COLOUR_CONFIG_FILE, "rt");
    if(config_file)
    {
        char num_str[7];
        fgets(num_str, sizeof(num_str), config_file);
        fclose(config_file);
        uint32_t num;
        sscanf(num_str, "%X", &num);
        if(num > 0x00FFFF)
        {
            num = 0x00FFFF;
        }
        return num;
    }
    else
    {/* No such file. Try to create it. */
        config_file = fopen(LED_COLOUR_CONFIG_FILE, "wt");
        fputs(LED_COLOUR_DEFAULT, config_file);
        fclose(config_file);
        return LED_COLOUR_DEFAULT;
    }
}

/* Sets the colour of the LED.
 * R: Red channel
 * G: Green channel
 * B: Blue channel
 */
void led_colour_set(uint32_t colour)
{
    if(strcmp(led_type, LED_TYPE_RGB) == 0)
    {
        uint8_t B = (uint8_t) colour;
        colour >>= 8;
        uint8_t G = (uint8_t) colour;
        colour >>= 8;
        uint8_t R = (uint8_t) colour;

        if(strcmp(led_type, LED_TYPE_RGB) == 0)
        {
            pwm_duty_cycle_set(LED_PWM_R_CHANNEL, (float) R / 255);
            pwm_duty_cycle_set(LED_PWM_G_CHANNEL, (float) G / 255);
            pwm_duty_cycle_set(LED_PWM_B_CHANNEL, (float) B / 255);
        }
    }
}
#endif

static void led_pwm_config_struct_init(lpLED_PWM_config config)
{
    int i = 0;
	config->ledProduct = emLED_PWM_PRODUCT_STANDARD;
    config->channelCount = 0;
    config->ledSwitch = 0;
    for(i = 0; i < LED_PWM_CHANNEL_MAX; i++) {
        config->array[i].type = i + 1;
        config->array[i].num = 0;
        config->array[i].channel = i + 1;
    }

}

static int led_pwm_config_json_parse(LP_JSON_OBJECT json, lpLED_PWM_config config)
{
	char text[32];
    int i = 0;
	LP_JSON_OBJECT child_json = NULL;

	if(json && config) {
		if(NETSDK_json_check_child(json, "product")){
			if(NULL != NETSDK_json_get_string(json, "product", text, sizeof(text))){
				config->ledProduct = NETSDK_MAP_STR2DEC(led_pwm_product_map, text, emLED_PWM_PRODUCT_STANDARD);
			}
		}

        config->channelCount = NETSDK_json_get_int(json, "channelCount"); // ªÒ»°ø…”√pwm◊‹ ˝
        if(config->channelCount > LED_PWM_CHANNEL_MAX) {
			APP_TRACE("led pwm channel count > %d", LED_PWM_CHANNEL_MAX);
            return -1;
        }
        config->ledSwitch = NETSDK_json_get_int(json, "switch");
        child_json = NETSDK_json_get_child(json, "channelInfo");
        if(child_json) {
            for(i = 0; i < config->channelCount; i++) {
                LP_JSON_OBJECT info = json_object_array_get_idx(child_json, i);
                config->array[i].type = NETSDK_json_get_int(info, "type");
                config->array[i].num = NETSDK_json_get_int(info, "num");
                config->array[i].channel = NETSDK_json_get_int(info, "channel");
            }
            return 0;
        }
    }

    return -1;

}

static int led_pwm_config_load(const char *fileName, lpLED_PWM_config config)
{
	int ret = -1;
	LP_JSON_OBJECT json_conf = NETSDK_json_load(fileName);
	if(json_conf) {
		ret = led_pwm_config_json_parse(json_conf, config);
		json_object_put(json_conf);
		if(-1 == ret) {
			APP_TRACE("led pwm config false");
			return -1;
		}
	}
    else {
		//APP_TRACE("NO led pwm config file");
		return -1;
	}

	return 0;

}

static int led_pwm_save_config(char *fileName, const stLED_PWM_config setConfig)
{
	char *text;
	int i = 0;

    LP_JSON_OBJECT obj = json_object_new_object();
    LP_JSON_OBJECT array = json_object_new_array();
    LP_JSON_OBJECT info = NULL;

    //pthread_mutex_lock(&st_ledPwmAttr.mutex);

	text = NETSDK_MAP_DEC2STR(led_pwm_product_map, setConfig.ledProduct, emLED_PWM_PRODUCT_STANDARD);
	NETSDK_json_set_string(obj, "product", text);
    NETSDK_json_set_int2(obj, "channelCount", setConfig.channelCount);
    NETSDK_json_set_int2(obj, "switch", setConfig.ledSwitch);
    for(i = 0; i < setConfig.channelCount; i++) {
        info = json_object_new_object();
        NETSDK_json_set_int2(info, "type", setConfig.array[i].type);
        NETSDK_json_set_int2(info, "num", setConfig.array[i].num);
        NETSDK_json_set_int2(info, "channel", setConfig.array[i].channel);
        json_object_array_put_idx(array, i, info);
    }
    json_object_object_add(obj, "channelInfo", array);
    json_object_to_file(fileName, obj);

    if(obj) {
		json_object_put(obj);
	}
	obj = NULL;

    //led_pwm_config_load(fileName, &st_ledPwmAttr.ledPwmConfig);

    //pthread_mutex_unlock(&st_ledPwmAttr.mutex);

    return 0;

}

static void led_pwm_set_brightness(const stLED_PWM_config config)
{
#define BRIGHTNESS_MAX_NUM	100
#define BRIGHTNESS_MIN_NUM	20
    APP_TRACE("%s", __FUNCTION__);
	int i = 0;
	int brightness = BRIGHTNESS_MAX_NUM;
	int colorTemp1 = 50, colorTemp2 = 0;
	int x3, x1;
	int colorTempFlag = 0;  // ±Í÷æ «∑Ò–Ë“™øÿ÷∆…´Œ¬
	int brightnessChannel = LED_PWM_CHANNEL_A, colorTempChannel = LED_PWM_CHANNEL_C;

	/* ≈–±…Ë÷√µƒ¿‡–Õ°¢Õ®µ¿°¢ ˝÷µ */
	for(i = 0; i <config.channelCount; i++) {
		if(config.array[i].type == LED_PWM_TYPE_BRIGHTNESS) {
			brightnessChannel = config.array[i].channel;
			if(config.array[i].num <= BRIGHTNESS_MIN_NUM) {
				brightness = BRIGHTNESS_MIN_NUM;
			}
			else if(config.array[i].num > BRIGHTNESS_MAX_NUM) {
				brightness = BRIGHTNESS_MAX_NUM;
			}
			else {
				brightness = config.array[i].num;
			}
		}
		else if(config.array[i].type == LED_PWM_TYPE_COLOR_TEMP){
			colorTempChannel = config.array[i].channel;
			colorTemp1 = config.array[i].num;
			colorTempFlag = 1;
		}
	}

	/*  ˝÷µ◊™“Â “ÚŒ™øÿ÷∆…´Œ¬ «¡Ω¬∑pwm£¨∂¯∂®“Âµƒ◊÷∂Œ÷ª”–num“ª∏ˆ÷µ(0-100) */
	/* 
	   Õº æ:              ÷–º‰±Ì æ¡Ω…´Œ¬◊Ó¥Û÷µ
	   		…´Œ¬1±‰ªØ<-------------|------------->…´Œ¬2±‰ªØ
	   0-50±Ì æ…´Œ¬1µƒ÷µ0->100±‰ªØ¥À ±…´Œ¬2µƒ÷µŒ™100   (num * 2)
	   50-100±Ì æ…´Œ¬2µƒ÷µ100->0±‰ªØ¥À ±…´Œ¬1µƒ÷µŒ™100 ((100 - num) * 2) */
	if(colorTemp1 > 100) {
		colorTemp1 = 100;
	}

	if(colorTemp1 < 50) {
		colorTemp1 = colorTemp1 * 2;
		colorTemp2 = 100;
	}
	else {
		colorTemp2 = (100 - colorTemp1) * 2;
		colorTemp1 = 100;
	}

	x1 = colorTemp1 * brightness / 100;	// ◊ˆ»®÷µ
	x3 = colorTemp2 * brightness / 100;

	pwm_duty_cycle_set(brightnessChannel, x1);

	if(colorTempFlag == 1)
		pwm_duty_cycle_set(colorTempChannel, x3);

}

/* ƒø«∞Œ¥ µœ÷ */
static void led_pwm_set_rgb(const stLED_PWM_config config)
{
    APP_TRACE("led_pwm_set_rgb");

}

/*
    led pwm config∫œ∑®–‘ºÏ≤È
    ÷ª“™”–“ª∏ˆ≤Œ ˝≤ª∫œ∑®æÕ≈–∂œŒ™≤ªø…”√£¨√ª◊ˆ∆‰À¸»›¥Ì–‘≤Ÿ◊˜
    »Áπ˚configΩ·ππÃÂ∂®“ÂªÚ∆‰≥…‘±∂®“Â”–±‰ªØ£¨–Ë“™∏¸∏ƒcheckÃıº˛
*/
static int led_pwm_config_check(const stLED_PWM_config config)
{
    int ret = 0;
    int i = 0;
    if(config.channelCount <= LED_PWM_CHANNEL_MAX) {
        if((config.ledSwitch != LED_PWM_SWITCH_STATUS_ON) 
            && (config.ledSwitch != LED_PWM_SWITCH_STATUS_OFF)) {
            APP_TRACE("led pwm switch %d undefined", config.ledSwitch);
            return -1;
        }
        for(i = 0; i < config.channelCount; i++) {
            if((config.array[i].type != LED_PWM_TYPE_BRIGHTNESS) 
                && (config.array[i].type != LED_PWM_TYPE_COLOR_TEMP) 
                && (config.array[i].type != LED_PWM_TYPE_RGB)) {
                APP_TRACE("led pwm type %d undefined", config.array[i].type);
                ret = -1;
                break;
            }
            if((config.array[i].channel != LED_PWM_CHANNEL_A) 
                && (config.array[i].channel != LED_PWM_CHANNEL_B) 
                && (config.array[i].channel != LED_PWM_CHANNEL_C)) {
                APP_TRACE("led pwm channel %d undefined", config.array[i].channel);
                ret = -1;
                break;
            }
        }
    }
    else {
        APP_TRACE("led pwm channel count %d > %d", config.channelCount, LED_PWM_CHANNEL_MAX);
        ret = -1;
    }

    return ret;
}

/*
    ∏˘æ›≈‰÷√∏ƒ±‰pwm’ºøÌ±»∏ƒ±‰led◊¥Ã¨
*/
static int led_pwm_change(const stLED_PWM_config setConfig)
{
    int i = 0;
    int ret = 0;

	if(led_pwm_config_check(setConfig) == -1) {
        return -1;
    }

	/* πÿ±’PWM */
	if(setConfig.ledSwitch == LED_PWM_SWITCH_STATUS_OFF) {
		for(i = 0; i < setConfig.channelCount; i++) {
			pwm_duty_cycle_set(setConfig.array[i].channel, LED_PWM_SWITCH_STATUS_OFF);
		}
		return 0;
	}

    for(i = 0 ; i < setConfig.channelCount; i++) {
        if(setConfig.array[i].type == LED_PWM_TYPE_BRIGHTNESS) {
            led_pwm_set_brightness(setConfig);
        }
        else if(setConfig.array[i].type == LED_PWM_TYPE_COLOR_TEMP) {
            led_pwm_set_brightness(setConfig);
        }
        else if(setConfig.array[i].type == LED_PWM_TYPE_RGB) {
            led_pwm_set_rgb(setConfig);
        }
        else {
            ret = -1;
            APP_TRACE("pwm %d not support type %d", setConfig.array[i].channel, setConfig.array[i].type);
        }
    }

    return ret;

}


static void led_pwm_change_light(stLED_PWM_config config, unsigned int light_num, int time_s)
{
    int i = 0;
    unsigned int time_us = time_s * 1000000;
    for(i = 0; i < config.channelCount; i++) {
        config.array[i].num = light_num;
        led_pwm_change(config);
    }
    usleep(time_us);

}

/*
	LED¡¡∞µ—≠ª∑øÿ÷∆£¨”√”⁄≤‚ ‘
*/
static void *led_pwm_test(void *arg)
{
#define LED_PWM_TEST_COUNT	3

	int i = 0, j = 0;
	stLED_PWM_config config = *(lpLED_PWM_config)arg;

	prctl(PR_SET_NAME, "led_pwm_test");

	for(i = 0; i < LED_PWM_TEST_COUNT; i ++) {
        config.ledSwitch = LED_PWM_SWITCH_STATUS_ON;
        led_pwm_change_light(config, 100, 2);
        config.ledSwitch = LED_PWM_SWITCH_STATUS_OFF;
        led_pwm_change_light(config, 0, 3);
	}

	led_pwm_test_tid = (pthread_t)NULL;
	pthread_exit(NULL);
}

/*
    ≈–∂œ «∑Ò”–ø…”√µƒpwm
*/
int LED_PWM_is_pwm(void)
{
    /* ≈–∂œ «∑Ò”–pwm */
    if((st_ledPwmAttr.ledPwmConfig.channelCount > 0) 
        && (st_ledPwmAttr.ledPwmConfig.channelCount <= LED_PWM_CHANNEL_MAX)) {
        return 0;
    }

    return -1;

}

/*
    led pwm configª÷∏¥≥ˆ≥ß…Ë÷√
*/
int LED_PWM_reset(void)
{
    return REMOVE_FILE(LED_PWM_SET_JSON);

}

/*
    …Ë÷√led pwm
*/
int LED_PWM_set(const stLED_PWM_config config)
{
	int i = 0;
    if(led_pwm_change(config) == 0) {
		/* …Ë÷√÷–∏¸∏ƒµƒ÷µ”–LED_PWM_config{ledSwitch, num} */
		st_ledPwmAttr.ledPwmConfig.ledSwitch = config.ledSwitch;
		for(i = 0; i < config.channelCount; i++) {
			st_ledPwmAttr.ledPwmConfig.array[i].num = config.array[i].num;
		}

		/* ºŸ»Á «±Í◊ºµƒµ∆≈›≤˙∆∑£¨–Ë“™±£¥Ê…Ë÷√÷µµΩŒƒº˛÷– */
		if(st_ledPwmAttr.ledPwmConfig.ledProduct == emLED_PWM_PRODUCT_STANDARD) {
			led_pwm_save_config(LED_PWM_SET_JSON, st_ledPwmAttr.ledPwmConfig);
		}
        APP_TRACE("pwm set ok");
        return 0;
    }

    return -1;
}

/*
    ªÒ»°led pwm≈‰÷√
*/
int LED_PWM_get(lpLED_PWM_config config)
{
    int i = 0;
    if(config) {
        config->channelCount = st_ledPwmAttr.ledPwmConfig.channelCount;
        config->ledSwitch = st_ledPwmAttr.ledPwmConfig.ledSwitch;
        for(i = 0; i < st_ledPwmAttr.ledPwmConfig.channelCount; i++) {
            config->array[i].type = st_ledPwmAttr.ledPwmConfig.array[i].type;
            config->array[i].num = st_ledPwmAttr.ledPwmConfig.array[i].num;
            config->array[i].channel = st_ledPwmAttr.ledPwmConfig.array[i].channel;
        }

        return 0;
    }

    return -1;

}

/* LED pwm initialization. */
void LED_PWM_init(void)
{
	int i = 0;
    int ret = 0;
	char file[64] = {0};
    led_pwm_config_struct_init(&st_ledPwmAttr.ledPwmConfig);
    /* ºŸ»Á¥Ê‘⁄LED_PWM_CUSTOM_JSONæÕª·◊‘∂Ø»•∏˘æ›≤˙≤‚∂®÷∆≤Œ ˝…Ë÷√LED_PWM_DEFAULTS_JSON */
    if(IS_FILE_EXIST(LED_PWM_CUSTOM_JSON)) {
		if(IS_FILE_EXIST(LED_PWM_SET_JSON)) {
            REMOVE_FILE(LED_PWM_SET_JSON);
        }
        if(COPY_FILE(LED_PWM_CUSTOM_JSON, LED_PWM_DEFAULTS_JSON) == -1) {
            APP_TRACE("%s copy %s fail", LED_PWM_CUSTOM_JSON, LED_PWM_DEFAULTS_JSON);
            return;
        }
    }

	/*
		standard£¨–Ë“™±£¥Ê…Ë÷√÷µ
		atdie£¨≤ª–Ë“™±£¥Ê…Ë÷√÷µ
		“ªø™ º√ª”–LED_PWM_SET_JSON‘Ú÷±Ω” π”√LED_PWM_DEFAULTS_JSON
		ºŸ»ÁLED_PWM_DEFAULTS_JSON…Ë÷√µƒ «standard‘Ú…Ë÷√∫Û(LED_PWM_set)æÕ∞—…Ë÷√÷µ±£¥ÊµΩLED_PWM_SET_JSON
	*/
	if(IS_FILE_EXIST(LED_PWM_SET_JSON)) {
		snprintf(file, sizeof(file), "%s", LED_PWM_SET_JSON);
	}
	else {
		snprintf(file, sizeof(file), "%s", LED_PWM_DEFAULTS_JSON);
	}
    ret = led_pwm_config_load(file, &st_ledPwmAttr.ledPwmConfig);
    if(ret == 0) {
        //pthread_mutex_init(&st_ledPwmAttr.mutex, NULL);  // ƒø«∞√ª”√Õæ∆¡±Œ

        /* ∏˘æ›pwm◊‹¬∑ ˝£¨∫ÕÕ®µ¿µƒ¿‡–Õ£¨…Ë÷√√ø∏ˆÕ®µ¿µƒ ˝÷µ/◊¥Ã¨ */
        if(led_pwm_config_check(st_ledPwmAttr.ledPwmConfig) == -1) {
            //APP_TRACE("led pwm init false");
            return;
        }

		/* ƒø«∞PWM2∂º «”√”⁄SENSOR_PWDN */
		for(i = 0; i < st_ledPwmAttr.ledPwmConfig.channelCount; i++) {
			if(st_ledPwmAttr.ledPwmConfig.array[i].channel == LED_PWM_CHANNEL_A) {
				system("himm 0x200F00EC 0");/* GPIO7_3 as PWM1. */
			}
			else if(st_ledPwmAttr.ledPwmConfig.array[i].channel == LED_PWM_CHANNEL_C) {
				system("himm 0x200F00F4 0");/* GPIO7_5 as PWM3. */
			}
		}
        //system("himm 0x200F00F0 0");/* GPIO7_4 as PWM2. */
        system("himm 0x20030038 2");/* Set PWM clock source to 3MHz. */

        led_pwm_change(st_ledPwmAttr.ledPwmConfig);

        //APP_TRACE("led pwm init success");
    }
    else {
        //APP_TRACE("led pwm init false");
    }

}

void LED_PWM_destroy()
{
	APP_TRACE("%s", __FUNCTION__);

}

/* ÷ª”√”⁄≤˙≤‚π§æﬂ£¨∂®÷∆LED pwm£¨≤˙≤‚π§æﬂƒ¨»œ…Ë÷√“ª¬∑pwm */
int LED_PWM_custom(int enabled, const int channelCount)
{
	int i = 0;
	stLED_PWM_config config;

	if(IS_FILE_EXIST(LED_PWM_DEFAULTS_JSON)) {
		unlink(LED_PWM_DEFAULTS_JSON);
		remove(LED_PWM_DEFAULTS_JSON);
	}
	if(enabled == 1) {
		config.ledProduct = emLED_PWM_PRODUCT_STANDARD;
		if(channelCount > LED_PWM_CHANNEL_MAX) {
			config.channelCount = LED_PWM_CHANNEL_MAX;
		}
		else {
			config.channelCount = channelCount;
		}

		config.ledSwitch = LED_PWM_SWITCH_STATUS_ON;
		for(i = 0 ;i < channelCount; i ++) {
			config.array[i].type = LED_PWM_TYPE_BRIGHTNESS + i;
			config.array[i].num = 100;
			config.array[i].channel = LED_PWM_CHANNEL_A + i;
		}
		led_pwm_save_config(LED_PWM_DEFAULTS_JSON, config);

		LED_PWM_init();

		/* —≠ª∑øÿ÷∆led pwm¡¡∞µ */
		if(!led_pwm_test_tid) {
	        pthread_create(&led_pwm_test_tid, 0, led_pwm_test, (void *)&st_ledPwmAttr.ledPwmConfig);
	    }
	}
	else {
		APP_TRACE("no custom led pwm");
	}

	return 0;
}

#endif  /* endif LED_PWM */

#else   /* else HI3516E_V1 */

#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <bsp/pwm.h>
#include <stdlib.h>
#include "led_pwm.h"


#define PWM_DEV_FILE     "/dev/pwm"


/**
 * pwmÂàùÂßãÂåñÔºåËÆæÁΩÆÂØÑÂ≠òÂô®‰∏∫PWMÂ∑•‰ΩúÊñπÂºèÔºåÂπ∂‰∏îËÆæÁΩÆclock 3MHz
 * @param   chn1     trueËÆæÁΩÆ‰∏∫pwmÈÄöÈÅì1|false‰∏çËÆæÁΩÆ
 * @param   chn2     trueËÆæÁΩÆ‰∏∫pwmÈÄöÈÅì2|false‰∏çËÆæÁΩÆ
 * @param   chn3     trueËÆæÁΩÆ‰∏∫pwmÈÄöÈÅì3|false‰∏çËÆæÁΩÆ
 * @return  0ÊàêÂäü|-1Â§±Ë¥•
 */
int LED_PWM_init(bool chn1, bool chn2, bool chn3)
{
#if  defined(HI3516E_V1)
	if(chn1){
		system("himm 0x12040008 0x1"); // GPIO6_6 as PWM1
	}
	system("himm 0x12010038 0x2"); //Set PWM clock source to 3MHz
#else
    if(chn1) {
        system("himm 0x200F00EC 0"); // GPIO7_3 as PWM1
    }
    if(chn2) {
        system("himm 0x200F00F0 0"); // GPIO7_4 as PWM2
    }
    if(chn3) {
        system("himm 0x200F00F4 0"); // GPIO7_5 as PWM3
    }

    system("himm 0x20030038 2"); // Set PWM clock source to 3MHz
#endif
    return 0;

}

/**
 * pwmÂØπÂ∫îÈÄöÈÅìËÑâÂÆΩËÆæÁΩÆÊï∞ÂÄºË∂äÂ§ßpwmÈ´òÁîµÂπ≥Áª¥ÊåÅÊó∂Èó¥Ë∂äÈïø
 * @param   channel        pwmÈÄöÈÅì,1|2|3
 * @param   duty_cycle     pwmËÑâÂÆΩÂë®ÊúüËÆæÁΩÆ,ËåÉÂõ¥[0-1000]
 */
#define PWM_RANGE (1000)
void LED_PWM_duty_cycle_set(uint8_t channel, uint16_t duty_cycle)
{
#define PWM_PERIOD 	7500	/* Clock = 3MHz. PWM frequency = 3MHz / 7500 = 400Hz. */

	if(duty_cycle > PWM_RANGE)
	{
		duty_cycle = PWM_RANGE;
	}

	uint16_t pulse_width = 1;

	//printf("duty_cycle:%d\r\n", duty_cycle);
	PWM_DATA_S stPwmData = {channel, pulse_width, PWM_PERIOD, 1};

	if(duty_cycle == 0)
	{
		pulse_width = 1;
		stPwmData.enable = 0;
	}
	else if(duty_cycle == PWM_RANGE)
	{
		pulse_width = PWM_PERIOD; /* To ensure it reaches its best. */
	}
	else
	{
		pulse_width = duty_cycle * PWM_PERIOD / PWM_RANGE; //4000 + 225 * duty_cycle / 10;/* 16 / 100 * PWM_PERIOD + (PWM_PERIOD / 100 * (25 - 16)) * (duty_cycle / 100) */
	}
	stPwmData.duty = pulse_width;

	int fd = -1;
	fd = open(PWM_DEV_FILE, 0);
	if(fd < 0)
	{
		return;
	}
	ioctl(fd, PWM_CMD_WRITE, &stPwmData);
	close(fd);
}

#endif  /* endif !defined(HI3516E_V1)  */
