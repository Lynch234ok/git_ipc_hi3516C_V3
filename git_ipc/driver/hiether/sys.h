
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/list.h>
#include <asm/io.h>

#ifndef __HISILICON_ETHV100_SYS_H
#define __HISILICON_ETHV100_SYS_H

/* init chip env */
void hieth_sys_init(void);
void hieth_sys_exit(void);

void hieth_sys_reset(int dev);

void hieth_sys_shutdown(int dev);
void hieth_sys_resume(int dev);

#endif

