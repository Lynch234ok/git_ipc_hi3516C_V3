
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include "hiethv100.h"
#include "mdio.h"
#include "mac.h"
#include "ctrl.h"
#include "glb.h"
#include "sys.h"

#define HIETH_SYSREG_BASE	IO_ADDRESS(0x101e0000)

static void hieth_set_regbit(ulong addr, int bit, int shift)
{
	ulong reg;

	reg = readl(addr);

	bit = bit ? 1 : 0;

	reg &= ~(1<<shift);
	reg |= bit<<shift;

	writel(reg, addr);
}

static void hieth_reset(int rst)
{
#define REG_RESET	0x01C
#define RESET_SHIFT	12

	ulong flags;

	rst = !rst;

	local_irq_save(flags);
	hieth_set_regbit(HIETH_SYSREG_BASE + REG_RESET, rst, RESET_SHIFT);
	local_irq_restore(flags);

	udelay(100);
}

#define hieth_clk_ena() do{ writel(1<<17, HIETH_SYSREG_BASE + 0x024); udelay(100); }while(0)
#define hieth_clk_dis() do{ writel(1<<17, HIETH_SYSREG_BASE + 0x028); udelay(100); }while(0)

void hieth_sys_reset(int dev)
{
	hieth_reset(1);
	hieth_clk_ena();
	hieth_reset(0);
}

void hieth_sys_shutdown(int dev)
{
	hieth_clk_dis();
}

void hieth_sys_resume(int dev)
{
	hieth_clk_ena();
}

void hieth_sys_init(void)
{
	hieth_reset(1);
	hieth_clk_ena();
	hieth_reset(0);
}

void hieth_sys_exit(void)
{
	hieth_reset(1);
	hieth_clk_dis();
}

