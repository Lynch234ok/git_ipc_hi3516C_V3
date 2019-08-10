
/* ä¸´æ—¶ä½¿ç”¨HI3516E_V1å®åŒºåˆ†ï¼Œåé¢ç¯æ³¡ä¿®æ”¹åçš„ä»£ç åˆå¹¶åï¼Œå¯ä»¥å»æ‰ */
#if  !defined(HI3516E_V1)

#if defined (LED_PWM)
#ifndef _LED_PWM_H_
#define _LED_PWM_H_


#define LED_TYPE_RGB "RGB"

#define LED_COLOUR_CONFIG_FILE "/media/conf/led_colour.conf"
#define LED_COLOUR_DEFAULT 0xFFFFFF/* white */

#define LED_PWM_R_CHANNEL 1
#define LED_PWM_G_CHANNEL 2
#define LED_PWM_B_CHANNEL 3

#define LED_PWM_CHANNEL_MAX     3

enum LED_PWM_TYPE
{
    LED_PWM_TYPE_BRIGHTNESS = 1,
    LED_PWM_TYPE_COLOR_TEMP,
    LED_PWM_TYPE_RGB,
};

enum LED_PWM_SWITCH_STATUS
{
    LED_PWM_SWITCH_STATUS_OFF = 0,
    LED_PWM_SWITCH_STATUS_ON,
};

enum LED_PWM_CHANNEL
{
    LED_PWM_CHANNEL_A = 1,
    LED_PWM_CHANNEL_B,
    LED_PWM_CHANNEL_C,
};

typedef struct LED_PWM_config
{
	unsigned char ledProduct;			// µÆ¿Ø²úÆ· ·Ö±ğ±ê×¼(µÆÅİ)¡¢°®µû
    unsigned char channelCount;         //  pwm×ÜÂ·Êı£¬×î´óÊıLED_PWM_CHANNEL_MAX
    unsigned char ledSwitch;            // led ¿ª¹Ø LED_PWM_SWITCH_STATUS[0, 1]
    struct
    {
        unsigned char type;             // ledÀàĞÍ£¬LED_PWM_TYPE[ÁÁ¶È/É«ÎÂ/RGB]
        unsigned int num;               // pwmÕ¼¿í±È¿ØÖÆÊıÖµ£¬Ä¿Ç°ÏŞÖÆµÄ·¶Î§ÊÇ[0,255]
        unsigned char channel;          // pwm Í¨µÀ LED_PWM_CHANNEL[1, 2, 3]
    }array[LED_PWM_CHANNEL_MAX];
}stLED_PWM_config, *lpLED_PWM_config;

/* ÅĞ¶ÏÊÇ·ñÓĞ¶¨ÖÆpwm */
extern int LED_PWM_is_pwm(void);
extern int LED_PWM_reset(void);
extern int LED_PWM_set(const stLED_PWM_config config);
extern int LED_PWM_get(lpLED_PWM_config config);

/* LED pwm initialization. */
extern void LED_PWM_init(void);

extern void LED_PWM_destroy();

/* Ö»ÓÃÓÚ²ú²â¹¤¾ß£¬¶¨ÖÆLED pwm£¬²ú²â¹¤¾ßÄ¬ÈÏÉèÖÃÒ»Â·pwm */
extern int LED_PWM_custom(int enabled, const int channelCount);

#endif  /* endif _LED_PWM_H_ */

#endif  /* endif LED_PWM */

#else   /* else HI3516E_V1 */

#ifndef _LED_PWM_H_
#define _LED_PWM_H_

/**
 * pwmåˆå§‹åŒ–ï¼Œè®¾ç½®å¯„å­˜å™¨ä¸ºPWMå·¥ä½œæ–¹å¼ï¼Œå¹¶ä¸”è®¾ç½®clock 3MHz
 * @param   chn1     trueè®¾ç½®ä¸ºpwmé€šé“1|falseä¸è®¾ç½®
 * @param   chn2     trueè®¾ç½®ä¸ºpwmé€šé“2|falseä¸è®¾ç½®
 * @param   chn3     trueè®¾ç½®ä¸ºpwmé€šé“3|falseä¸è®¾ç½®
 * @return  0æˆåŠŸ|-1å¤±è´¥
 */
extern int LED_PWM_init(bool chn1, bool chn2, bool chn3);

/**
 * pwmå¯¹åº”é€šé“è„‰å®½è®¾ç½®æ•°å€¼è¶Šå¤§pwmé«˜ç”µå¹³ç»´æŒæ—¶é—´è¶Šé•¿
 * @param   channel        pwmé€šé“,1|2|3
 * @param   duty_cycle     pwmè„‰å®½å‘¨æœŸè®¾ç½®,èŒƒå›´[0-100]
 */
extern void LED_PWM_duty_cycle_set(uint8_t channel, uint16_t duty_cycle);

#endif  /* endif _LED_PWM_H_ */

#endif  /* endif !defined(HI3516E_V1)  */
