#include "drv_headers.h"

#include "hi3515_drv.h"

#if   defined(SDK_PLATFORM_HI3516A)
#define GPIO_GRP_CNT (15)
#endif
#define GPIO_PIN_CNT (8)

EXPORT_SYMBOL(reg_write32);
void reg_write32(unsigned long addr, unsigned int data)
{
	writel(data, IO_ADDRESS(addr));
}

EXPORT_SYMBOL(reg_read32);
unsigned int reg_read32(unsigned long addr)
{
	return (unsigned int)readl(IO_ADDRESS(addr));
}

EXPORT_SYMBOL(reg_set32);
void reg_set32(unsigned long addr, unsigned int mask)
{
	unsigned int reg = reg_read32(addr);
	reg |= mask;
	reg_write32(addr, reg);
}

EXPORT_SYMBOL(reg_clear32);
void reg_clear32(unsigned long addr, unsigned int mask)
{
	unsigned int reg = reg_read32(addr);
	reg &= ~mask;
	reg_write32(addr, reg);
}

EXPORT_SYMBOL(reg_test32);
int reg_test32(unsigned long addr, unsigned int mask)
{
	return ((reg_read32(addr) & mask) != 0);
}

EXPORT_SYMBOL(gpio_set_dir);
int gpio_set_dir(int group, int pin, int io)
{
	if(group >= GPIO_GRP_CNT || pin >= GPIO_PIN_CNT){
		GPIODRV_TRACE("invalid argrument (%d,%d)", group, pin);
		return -1;
	}
	if(GPIO_DIR_IN == io){
		reg_clear32(GPIO_DIR(group), (1<<pin));
	}else{
		reg_set32(GPIO_DIR(group), (1<<pin));
	}
	return 0;
}

EXPORT_SYMBOL(gpio_get_dir);
int gpio_get_dir(int group, int pin)
{
	if(group >= GPIO_GRP_CNT || pin >= GPIO_PIN_CNT){
		GPIODRV_TRACE("invalid argrument (%d,%d)", group, pin);
		return -1;
	}
	return (reg_read32(GPIO_DIR(group)) & (1<<pin)) ? GPIO_DIR_OUT : GPIO_DIR_IN;
}

EXPORT_SYMBOL(gpio_set_pin);
int gpio_set_pin(int group, int pin, int set)
{
	if(group >= GPIO_GRP_CNT || pin >= GPIO_PIN_CNT){
		GPIODRV_TRACE("invalid argrument (%d,%d)", group, pin);
		return -1;
	}
	reg_write32(GPIO_BASE(group) + ((1<<pin)<<2), set ? 0xff : 0);
	return 0;
}

EXPORT_SYMBOL(gpio_get_pin);
int gpio_get_pin(int group, int pin)
{
	if(group >= GPIO_GRP_CNT || pin >= GPIO_PIN_CNT){
		GPIODRV_TRACE("invalid argrument (%d,%d)", group, pin);
		return -1;
	}
	return (reg_read32(GPIO_BASE(group) + ((1<<pin)<<2))) ? GPIO_PIN_HIGH : GPIO_PIN_LOW;
}

EXPORT_SYMBOL(gpio_pin_init);
void gpio_pin_init(gpio_conf_t conf)
{
	gpio_set_dir(conf.grp, conf.pin, conf.dir);
	if(GPIO_DIR_OUT == conf.dir){
		gpio_set_pin(conf.grp, conf.pin, conf.level);
	}
}

static void io_config(void)
{
}

static int  __init hi3515_base_init(void)
{
	io_config();
	printk("HI3515 base @ %s %s\r\n", __TIME__, __DATE__);
	return 0;
}

static void __exit hi3515_base_exit(void)
{
}

module_init(hi3515_base_init);
module_exit(hi3515_base_exit);

MODULE_AUTHOR("Law @ Guangzhou JUAN");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HI3515 base control");

