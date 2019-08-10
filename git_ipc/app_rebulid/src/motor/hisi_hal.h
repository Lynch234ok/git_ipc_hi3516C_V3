#ifndef __HISI_HAL_H__
#define __HISI_HAL_H__

#define GPIO_DIR_IN		(0)
#define GPIO_DIR_OUT	(1)
#define GPIO_PIN_HIGH	(1)
#define GPIO_PIN_LOW	(0)

#define GPIO_DATA(p)	(GPIO_BASE((p))+0x3fc)
#define GPIO_DIR(p)		(GPIO_BASE((p))+0x400)
#define GPIO_IS(p)		(GPIO_BASE((p))+0x404)
#define GPIO_IBE(p)		(GPIO_BASE((p))+0x408)
#define GPIO_IEV(p)		(GPIO_BASE((p))+0x40C)
#define GPIO_IE(p)		(GPIO_BASE((p))+0x410)
#define GPIO_RIS(p)		(GPIO_BASE((p))+0x414)
#define GPIO_MIS(p)		(GPIO_BASE((p))+0x418)
#define GPIO_IC(p)		(GPIO_BASE((p))+0x41C)

#define reg_set32(addr, mask) \
	do { \
		unsigned long reg = 0; \
		reg  = reg_read32(addr); \
		reg |= mask; \
		reg_write32((addr), reg); \
	} while(0)

#define reg_clear32(addr, mask) \
	do { \
		unsigned long reg = 0; \
		reg  = reg_read32(addr); \
		reg &= ~mask; \
		reg_write32((addr), reg); \
	} while(0)

#define gpio_set_dir(grp, pin, io) \
	do { \
		if(GPIO_DIR_IN == (io)) { \
			reg_clear32(GPIO_DIR(grp), (1<<(pin))); \
		} else { \
			reg_set32(GPIO_DIR(grp), (1<<(pin))); \
		} \
	} while(0)

#define gpio_get_dir(grp, pin)      ((reg_read32(GPIO_DIR(grp)) & (1<<(pin))) ? GPIO_DIR_OUT : GPIO_DIR_IN)
#define gpio_set_pin(grp, pin, set) (reg_write32(GPIO_BASE(grp) + ((1<<(pin))<<2), (set) ? 0xff : 0))
#define gpio_get_pin(grp, pin)      ((reg_read32(GPIO_BASE(grp) + ((1<<(pin))<<2))) ? GPIO_PIN_HIGH : GPIO_PIN_LOW)

#endif //__HISI_HAL_H__
