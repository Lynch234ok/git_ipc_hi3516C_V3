
/* 临时使用HI3516E_V1宏区分，后面灯泡修改后的代码合并后，可以去掉 */
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
	unsigned char ledProduct;			// �ƿز�Ʒ �ֱ��׼(����)������
    unsigned char channelCount;         //  pwm��·���������LED_PWM_CHANNEL_MAX
    unsigned char ledSwitch;            // led ���� LED_PWM_SWITCH_STATUS[0, 1]
    struct
    {
        unsigned char type;             // led���ͣ�LED_PWM_TYPE[����/ɫ��/RGB]
        unsigned int num;               // pwmռ��ȿ�����ֵ��Ŀǰ���Ƶķ�Χ��[0,255]
        unsigned char channel;          // pwm ͨ�� LED_PWM_CHANNEL[1, 2, 3]
    }array[LED_PWM_CHANNEL_MAX];
}stLED_PWM_config, *lpLED_PWM_config;

/* �ж��Ƿ��ж���pwm */
extern int LED_PWM_is_pwm(void);
extern int LED_PWM_reset(void);
extern int LED_PWM_set(const stLED_PWM_config config);
extern int LED_PWM_get(lpLED_PWM_config config);

/* LED pwm initialization. */
extern void LED_PWM_init(void);

extern void LED_PWM_destroy();

/* ֻ���ڲ��⹤�ߣ�����LED pwm�����⹤��Ĭ������һ·pwm */
extern int LED_PWM_custom(int enabled, const int channelCount);

#endif  /* endif _LED_PWM_H_ */

#endif  /* endif LED_PWM */

#else   /* else HI3516E_V1 */

#ifndef _LED_PWM_H_
#define _LED_PWM_H_

/**
 * pwm初始化，设置寄存器为PWM工作方式，并且设置clock 3MHz
 * @param   chn1     true设置为pwm通道1|false不设置
 * @param   chn2     true设置为pwm通道2|false不设置
 * @param   chn3     true设置为pwm通道3|false不设置
 * @return  0成功|-1失败
 */
extern int LED_PWM_init(bool chn1, bool chn2, bool chn3);

/**
 * pwm对应通道脉宽设置数值越大pwm高电平维持时间越长
 * @param   channel        pwm通道,1|2|3
 * @param   duty_cycle     pwm脉宽周期设置,范围[0-100]
 */
extern void LED_PWM_duty_cycle_set(uint8_t channel, uint16_t duty_cycle);

#endif  /* endif _LED_PWM_H_ */

#endif  /* endif !defined(HI3516E_V1)  */
