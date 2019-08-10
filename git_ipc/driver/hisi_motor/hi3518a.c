#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/bitops.h>
#include <linux/delay.h>
#include <linux/kernel.h>

#include <linux/sched.h>
#include <linux/kthread.h>

#include "hi3518a.h"
#include "hi_motor.h"

volatile unsigned long AfTimerTick = 0;

static irqreturn_t hi3518a_timer2_isr(int irq, void *data)
{
	/* clean all interrupts */
	writel(0x0, HI3518A_TIMER2_BASE + HI3518A_TIMER2_INTCLR);
	//TODO
	AfTimerTick++;

	return IRQ_HANDLED;
}

static int hi35xx_motor_timer_init(void)
{
	int ret = 0;
	unsigned long tmp = 0;
	
	tmp = readl(SYS_CTRL_BASE);
	tmp &= ~(TIMEREN2SEL);
	tmp &= ~(TIMEREN2OV);
	writel(tmp, SYS_CTRL_BASE);//选择使用3MHz时钟进行计数；

	writel(0x0, HI3518A_TIMER2_BASE + HI3518A_TIMER2_CONTROL);
	writel(HI35XX_TIMER_COUNTER, HI3518A_TIMER2_BASE + HI3518A_TIMER2_LOAD);
	writel(HI35XX_TIMER_COUNTER, HI3518A_TIMER2_BASE + HI3518A_TIMER2_VALUE);
	writel(CFG_TIMER_CONTROL, HI3518A_TIMER2_BASE + HI3518A_TIMER2_CONTROL);

	ret = request_irq(HI3518A_TIMER2_IRQ, hi3518a_timer2_isr,
			IRQF_DISABLED | IRQF_TIMER, HI3518A_TIMER2_NAME, (void *)NULL);
	if (unlikely(ret)) {
		himotor_trace(HIMOTOR_ERR, "request timer2 irq(%d) failed, ret=%d!\n", HI3518A_TIMER2_IRQ, ret);
		goto error1;
	}


	return 0;
error1:
	return -1;
}

int hi35xx_motor_init(void)
{
	int ret = 0;
	himotor_trace(HIMOTOR_DEBUG, "enter hi3518a_motor_init.\n");

	ret = hi35xx_motor_timer_init();
	if (unlikely(ret))
		goto error;

	himotor_trace(HIMOTOR_DEBUG, "leave hi3518a_motor_init.\n");

	return 0;

error:
	return -1;
}

int hi35xx_motor_exit(void)
{
	free_irq(HI3518A_TIMER2_IRQ, NULL);
	return 0;
}
