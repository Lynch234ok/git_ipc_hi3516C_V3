#ifndef __HI35XX_H__
#define __HI35XX_H__

#ifndef SDK_PLATFORM_HI3516A
#define SDK_PLATFORM_HI3516A
#endif

#define HI_PLATFORM_REG_BASE (0x20000000)
#define IO_CONFIG_BASE (HI_PLATFORM_REG_BASE+0xf0000)
#define IO_CONFIG_REG(U) (IO_CONFIG_BASE+((U)*0x0004))

#if  defined(SDK_PLATFORM_HI3516A)
#define GPIO0_BASE (HI_PLATFORM_REG_BASE+0x140000)
#else
#define GPIO0_BASE (HI_PLATFORM_REG_BASE+0x150000)
#endif
#define GPIO_BASE(X) (GPIO0_BASE + (X)*0x10000)

#define GPIO_DATA(p)	(GPIO_BASE((p))+0x3fc)
#define GPIO_DIR(p)		(GPIO_BASE((p))+0x400)
#define GPIO_IS(p)		(GPIO_BASE((p))+0x404)
#define GPIO_IBE(p)		(GPIO_BASE((p))+0x408)
#define GPIO_IEV(p)		(GPIO_BASE((p))+0x40C)
#define GPIO_IE(p)		(GPIO_BASE((p))+0x410)
#define GPIO_RIS(p)		(GPIO_BASE((p))+0x414)
#define GPIO_MIS(p)		(GPIO_BASE((p))+0x418)
#define GPIO_IC(p)		(GPIO_BASE((p))+0x41C)

#define DEBUG_GPIODRV
#ifdef DEBUG_GPIODRV
#define GPIODRV_TRACE(fmt...) \
	do{printk("\033[1;31mGPIO->[%s]:%d ", __FUNCTION__, __LINE__);printk(fmt);printk("\033[m\r\n");}while(0)
#else
#define GPIODRV_TRACE(fmt...)
#endif

#define GPIO_DIR_IN		(0)
#define GPIO_DIR_OUT	(1)
#define GPIO_PIN_HIGH	(1)
#define GPIO_PIN_LOW	(0)

#ifdef __KERNEL__
typedef struct
{
	unsigned int  grp;
	unsigned int  pin;
	unsigned int  value;
}gpio_ctrl_t;

typedef struct gpio_conf
{
	int grp;
	int pin;
	int dir;
	int level;
}gpio_conf_t;

extern void reg_write32(unsigned long addr, unsigned int data);
extern unsigned int reg_read32(unsigned long addr);

extern void reg_set32(unsigned long addr, unsigned int mask);
extern void reg_clear32(unsigned long addr, unsigned int mask);
extern int reg_test32(unsigned long addr, unsigned int mask);

extern int gpio_set_dir(int group, int pin, int io);
extern int gpio_get_dir(int group, int pin);
extern int gpio_set_pin(int group, int pin, int set);
extern int gpio_get_pin(int group, int pin);

extern void gpio_pin_init(gpio_conf_t conf);

#endif //__KERNEL__

#endif //__HI35XX_H__

