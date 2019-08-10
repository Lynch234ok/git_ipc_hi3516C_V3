#include "bsp/watchdog.h"



#include <linux/ioctl.h>
#include <linux/types.h>


#define    WATCHDOG_IOCTL_BASE    'W'


#define    WDIOC_GETSUPPORT     _IOR(WATCHDOG_IOCTL_BASE, 0, struct watchdog_info)
#define    WDIOC_GETSTATUS      _IOR(WATCHDOG_IOCTL_BASE, 1, int)
#define    WDIOC_GETBOOTSTATUS  _IOR(WATCHDOG_IOCTL_BASE, 2, int)
//#define    WDIOC_GETTEMP        _IOR(WATCHDOG_IOCTL_BASE, 3, int)
#define    WDIOC_SETOPTIONS     _IOR(WATCHDOG_IOCTL_BASE, 4, int)
#define    WDIOC_KEEPALIVE      _IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define    WDIOC_SETTIMEOUT     _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define    WDIOC_GETTIMEOUT     _IOR(WATCHDOG_IOCTL_BASE, 7, int)
//#define    WDIOC_SETPRETIMEOUT  _IOWR(WATCHDOG_IOCTL_BASE, 8, int)
//#define    WDIOC_GETPRETIMEOUT  _IOR(WATCHDOG_IOCTL_BASE, 9, int)
//#define    WDIOC_GETTIMELEFT    _IOR(WATCHDOG_IOCTL_BASE, 10, int)

#define    WDIOF_UNKNOWN        -1    /* Unknown flag error */
#define    WDIOS_UNKNOWN        -1    /* Unknown status error */

#define    WDIOF_OVERHEAT       0x0001    /* Reset due to CPU overheat */
#define    WDIOF_FANFAULT       0x0002    /* Fan failed */
#define    WDIOF_EXTERN1        0x0004    /* External relay 1 */
#define    WDIOF_EXTERN2        0x0008    /* External relay 2 */
#define    WDIOF_POWERUNDER     0x0010    /* Power bad/power fault */
#define    WDIOF_CARDRESET      0x0020    /* Card previously reset the CPU */
#define    WDIOF_POWEROVER      0x0040    /* Power over voltage */
#define    WDIOF_SETTIMEOUT     0x0080  /* Set timeout (in seconds) */
#define    WDIOF_MAGICCLOSE     0x0100    /* Supports magic close char */
#define    WDIOF_PRETIMEOUT     0x0200  /* Pretimeout (in seconds), get/set */
#define    WDIOF_KEEPALIVEPING  0x8000    /* Keep alive ping reply */

#define    WDIOS_DISABLECARD    0x0001    /* Turn off the watchdog timer */
#define    WDIOS_ENABLECARD     0x0002    /* Turn on the watchdog timer */
#define    WDIOS_TEMPPANIC      0x0004    /* Kernel panic on temperature trip */




#ifdef __KERNEL__

#define WATCHDOG_TRACE(fmt...) \
    do{printk("\033[1;31mWATCHDOG->[%s]:%d ", __FUNCTION__, __LINE__);printk(fmt);printk("\033[m\r\n");}while(0)

	
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/io.h>
		//#include <asm/arch/hardware.h>
#include <linux/kthread.h>
		//#include <asm/arch/clock.h>
#include <linux/string.h>
#include <linux/sched.h>



#define OSDRV_MODULE_VERSION_STRING "HISI_WDT-MDC030001 @Hi3518v100"

#define HISILICON_SCTL_BASE 0x20050000
/* define watchdog IO */
#define HIWDT_BASE      0x20040000
#define HIWDT_REG(x)    (HIWDT_BASE + (x))

#define HIWDT_LOAD      0x000
#define HIWDT_VALUE     0x004
#define HIWDT_CTRL      0x008
#define HIWDT_INTCLR    0x00C
#define HIWDT_RIS       0x010
#define HIWDT_MIS       0x014
#define HIWDT_LOCK      0xC00

#define HIWDT_UNLOCK_VAL    0x1ACCE551

void __iomem *reg_ctl_base_va;
void __iomem *reg_wdt_base_va;
#define IO_WDT_ADDRESS(x) (reg_wdt_base_va + ((x)-(HIWDT_BASE)))

#define hiwdt_readl(x)      readl(IO_WDT_ADDRESS(HIWDT_REG(x)))
#define hiwdt_writel(v,x)   writel(v, IO_WDT_ADDRESS(HIWDT_REG(x)))

/* debug */
#define HIDOG_PFX "HiDog: "
#define hidog_dbg(params...) printk(KERN_INFO HIDOG_PFX params)

/* module param */
#define HIDOG_TIMER_MARGIN    60
static int default_margin = HIDOG_TIMER_MARGIN;    /* in seconds */
#define HIDOG_TIMER_DEMULTIPLY  9
module_param(default_margin, int, 0);
MODULE_PARM_DESC(default_margin, "Watchdog default_margin in seconds. (0<default_margin<80, default=" __MODULE_STRING(HIDOG_TIMER_MARGIN) ")");

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default=CONFIG_WATCHDOG_NOWAYOUT)");

static int nodeamon = 0;
module_param(nodeamon, int, 0);
MODULE_PARM_DESC(nodeamon, "By default, a kernel deamon feed watchdog when idle, set 'nodeamon=1' to disable this. (default=0)");



/* watchdog info */
static struct watchdog_info ident = {
    .options =  WDIOF_SETTIMEOUT |
                WDIOF_KEEPALIVEPING |
                WDIOF_MAGICCLOSE,
    .firmware_version = 0,
    .identity = "Hisilicon Watchdog",
};


/* local var */
static DEFINE_SPINLOCK(hidog_lock);
static int cur_margin = HIDOG_TIMER_MARGIN;
//static pid_t pid_hidog_deamon = -1;
struct task_struct *task_hidog_deamon = NULL;


#define HIDOG_EXIT    0
#define HIDOG_SELFCLR    1
#define HIDOG_EXTCLR    2
volatile static unsigned int hidog_state = 0;

static unsigned long driver_open = 0, orphan_timer = 0;
static int options = WDIOS_ENABLECARD;

#ifndef MHZ
#define MHZ (1000*1000)
#endif

static unsigned long rate = 3*MHZ;

static inline void hidog_set_timeout(unsigned int nr)
{
    unsigned long cnt = (~0x0)/rate;        /* max cnt */
    unsigned long flags; 

    spin_lock_irqsave(&hidog_lock, flags); 

    if( nr==0 || nr>cnt)
        cnt=~0x0; 
    else
        cnt = nr*rate;
    /* unlock watchdog registers */ 
    hiwdt_writel(HIWDT_UNLOCK_VAL, HIWDT_LOCK); 
    hiwdt_writel(cnt, HIWDT_LOAD); 
    hiwdt_writel(cnt, HIWDT_VALUE); 
    /* lock watchdog registers */ 
    hiwdt_writel(0, HIWDT_LOCK); 
    spin_unlock_irqrestore(&hidog_lock, flags); 
};

static inline void hidog_feed(void)
{
    unsigned long flags; 

    spin_lock_irqsave(&hidog_lock, flags); 
    /* unlock watchdog registers */ 
    hiwdt_writel(HIWDT_UNLOCK_VAL, HIWDT_LOCK); 
    /* clear watchdog */ 
    hiwdt_writel(0x00, HIWDT_INTCLR); 
    /* lock watchdog registers */ 
    hiwdt_writel(0, HIWDT_LOCK); 
    spin_unlock_irqrestore(&hidog_lock, flags); 
};

static int hidog_set_heartbeat(int t)
{
    int ret = 0;
    unsigned int cnt = (~0x0)/rate; 

    if( t==0 ) {
        printk(KERN_ERR HIDOG_PFX "set heartbeat to 0, heartbeat will not be changed.\n");
        t = cur_margin;
        ret = 1;
    } else if( t>cnt ) {
        printk(KERN_ERR HIDOG_PFX "set heartbeat range error, t = %d\n", t);
        printk(KERN_ERR HIDOG_PFX "force heartbeat to %d\n", cnt);
        t = cnt;
        ret = -1;
    }

    cur_margin = t;

    hidog_set_timeout(t);
    hidog_feed();

    if(NULL != task_hidog_deamon)
        wake_up_process(task_hidog_deamon);    

    return ret;
}

static int hidog_keepalive(void)
{
    hidog_feed();
    return 0;
}

static inline void hidog_start(void)
{
    unsigned long flags;
    unsigned long t;
    
    spin_lock_irqsave(&hidog_lock, flags);
    /* unlock watchdog registers */
    hiwdt_writel(HIWDT_UNLOCK_VAL, HIWDT_LOCK);
    hiwdt_writel(0x00, HIWDT_CTRL);
    hiwdt_writel(0x00, HIWDT_INTCLR);
    hiwdt_writel(0x03, HIWDT_CTRL);
    /* lock watchdog registers */
    hiwdt_writel(0, HIWDT_LOCK);
    /* enable watchdog clock --- set the frequency to 3MHz */
    t = readl(reg_ctl_base_va);
    writel(t & ~0x00800000, reg_ctl_base_va);
    spin_unlock_irqrestore(&hidog_lock, flags);
    
    options = WDIOS_ENABLECARD;
}

static inline void hidog_stop(void)
{
    unsigned long flags;

    spin_lock_irqsave(&hidog_lock, flags);

    /* unlock watchdog registers */
    hiwdt_writel(HIWDT_UNLOCK_VAL, HIWDT_LOCK);

    /* stop watchdog timer */
    hiwdt_writel(0x00, HIWDT_CTRL);
    hiwdt_writel(0x00, HIWDT_INTCLR);

    /* lock watchdog registers */
    hiwdt_writel(0, HIWDT_LOCK);

    spin_unlock_irqrestore(&hidog_lock, flags);

    hidog_set_timeout(0);

    options = WDIOS_DISABLECARD;
}

static int hidog_open(struct inode *inode, struct file *file)
{
    int ret = 0;

    if (test_and_set_bit(0, &driver_open))
        return -EBUSY;
    if (!test_and_clear_bit(0, &orphan_timer))
        __module_get(THIS_MODULE);
    /*
     *    Activate timer
     */
    hidog_keepalive();
    ret = nonseekable_open(inode, file);
    if(ret == 0)
        hidog_state = HIDOG_EXTCLR;

    return ret;
}
static int hidog_release(struct inode *inode, struct file *file)
{
    /*
     *    Shut off the timer.
     *     Lock it in if it's a module and we set nowayout
     */
    if (nowayout == 0) {
        //cur_margin = default_margin;
        hidog_state = HIDOG_SELFCLR;
        hidog_set_heartbeat(cur_margin);
        module_put(THIS_MODULE);
    } else {
        printk(KERN_CRIT HIDOG_PFX "Unexpected close, not stopping watchdog!\n");
        set_bit(0, &orphan_timer);
        hidog_keepalive();
    }
    clear_bit(0, &driver_open);

    if(options == WDIOS_DISABLECARD)
        printk(KERN_INFO HIDOG_PFX "Watchdog is disabled!\n");

    return 0;
}

static ssize_t hidog_write(struct file *file, const char __user *data, size_t len, loff_t *ppos)
{
    /*
     *      *    Refresh the timer.
     *           */
    if(len) {        
        size_t i;

        nowayout = 0;

        for (i = 0; i != len; i++) {
            char c;

            if (get_user(c, data + i))
                return -EFAULT;
            if (c == 'V')           
                nowayout = 1;
        }
        hidog_keepalive();
    }

    return len;

}
static long hidog_ioctl(struct file *file,
    unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    int __user *p = argp;
    int new_margin;
    int new_options;

    switch (cmd) {
        case WDIOC_GETSUPPORT:
            return copy_to_user(argp, &ident,
                sizeof(ident)) ? -EFAULT : 0;
        case WDIOC_GETSTATUS:
        case WDIOC_GETBOOTSTATUS:
            return put_user(options, p);

        case WDIOC_KEEPALIVE:
            hidog_keepalive();
            return 0;

        case WDIOC_SETTIMEOUT:
            if (get_user(new_margin, p))
                return -EFAULT;
            if (hidog_set_heartbeat(new_margin))
                return -EINVAL;
            hidog_keepalive();
            return 0;

        case WDIOC_GETTIMEOUT:
            return put_user(cur_margin, p);

        case WDIOC_SETOPTIONS:
            if(nowayout)
                return -WDIOS_UNKNOWN;

            if (get_user(new_options, p))
                return -EFAULT;
            if(new_options & WDIOS_ENABLECARD) {
                hidog_start();
                hidog_set_heartbeat(cur_margin);
                return 0;
            } else if (new_options & WDIOS_DISABLECARD) {
                hidog_stop();
                return 0;
            } else
                return -WDIOS_UNKNOWN;

        default:
            return -ENOIOCTLCMD;
    }

}
static int hidog_notifier_sys(struct notifier_block *this, unsigned long code,
    void *unused)
{
    if(code==SYS_DOWN || code==SYS_HALT) {
        /* Turn the WDT off */
        hidog_stop();
    }
    return NOTIFY_DONE;
}

/*
 *    Kernel Interfaces
 */



static struct file_operations hidog_fops = {
    .owner        = THIS_MODULE,
    .llseek        = no_llseek,
    .write        = hidog_write,
//    .ioctl        = hidog_ioctl,
    .unlocked_ioctl = hidog_ioctl,
    .open        = hidog_open,
    .release    = hidog_release,
};

static struct miscdevice hidog_miscdev = {
    .minor        = WATCHDOG_MINOR,
    .name        = "watchdog",
    .fops        = &hidog_fops,
};

static struct notifier_block hidog_notifier = {
    .notifier_call    = hidog_notifier_sys,
};

static char banner[] __initdata = KERN_INFO "JUAN Watchdog Timer: 0.01 initialized. default_margin=%d sec (nowayout= %d, nodeamon= %d)\n";
static int hidog_deamon(void *data)
{
    struct sched_param param = { .sched_priority = 99 };

    sched_setscheduler(current, SCHED_FIFO, &param);
    current->flags |= PF_NOFREEZE;

    set_current_state(TASK_INTERRUPTIBLE);

    while(hidog_state != HIDOG_EXIT)
    {
        switch(hidog_state) {
        case HIDOG_SELFCLR:
            if(options & WDIOS_ENABLECARD)
              //  hidog_feed();
            break;
        case HIDOG_EXTCLR:
            break;
        default:
            break;
        }
        /* sleep */
        /* when self feed dog, only use the default margin */
        schedule_timeout_interruptible(msecs_to_jiffies(default_margin*1000/2)+1);
    }

    set_current_state(TASK_RUNNING);

    return 0;
}

static int __init hidog_init(void)
{
    hidog_start();

    hidog_state = HIDOG_SELFCLR;
    if(!nodeamon) {
        #if 0
        struct task_struct *p_dog;
        p_dog = kthread_create(hidog_deamon, NULL, "hidog");
        if(IS_ERR(p_dog) <0) {
            printk(KERN_ERR HIDOG_PFX "create hidog_deamon failed!\n");
            return -1;        
        pid_hidog_deamon = p_dog->pid;        
        #endif

        struct task_struct *p_dog;
		p_dog = kthread_create(hidog_deamon, NULL, "hidog");
		if(IS_ERR(p_dog) <0) {
			printk(KERN_ERR HIDOG_PFX "create hidog_deamon failed!\n");
			return -1;
		}
		wake_up_process(p_dog);
		task_hidog_deamon = p_dog;
    }
    //printk(KERN_INFO OSDRV_MODULE_VERSION_STRING);
    return 0;
}

static int __init watchdog_init(void)
{
    int ret = 0;

    reg_wdt_base_va = (void __iomem *)IO_ADDRESS(HIWDT_BASE);
    if (NULL == reg_wdt_base_va)
    {
        printk(KERN_ERR HIDOG_PFX "function %s line %u failed\n", 
            __FUNCTION__, __LINE__);
        return -1;
    }
    reg_ctl_base_va = (void __iomem *)IO_ADDRESS(HISILICON_SCTL_BASE);
    if (NULL == reg_ctl_base_va)
    {
        printk(KERN_ERR HIDOG_PFX "function %s line %u failed\n", 
            __FUNCTION__, __LINE__);
        iounmap(reg_wdt_base_va);
        return -1;
    }
    
    cur_margin = default_margin;

    /* Check that the default_margin value is within it's range ; if not reset to the default */
    if(hidog_set_heartbeat(default_margin)) {
        default_margin = HIDOG_TIMER_MARGIN;
        hidog_set_heartbeat(HIDOG_TIMER_MARGIN);
        printk(KERN_WARNING HIDOG_PFX "default_margin value must be 0<default_margin<%lu, using %d\n",
            ~0x0/rate, HIDOG_TIMER_MARGIN);
    }

    ret = register_reboot_notifier(&hidog_notifier);
    if(ret) {
        printk(KERN_ERR HIDOG_PFX "cannot register reboot notifier (err=%d)\n", ret);
        goto watchdog_init_errA;
    }

    ret = misc_register(&hidog_miscdev);
    if(ret) {
        printk (KERN_ERR HIDOG_PFX "cannot register miscdev on minor=%d (err=%d)\n",
            WATCHDOG_MINOR, ret);
        goto watchdog_init_errB;
    }

    printk(banner, default_margin, nowayout, nodeamon);

    ret = hidog_init();
    if(ret) {
        goto watchdog_init_errC;
    }
    //printk(KERN_INFO OSDRV_MODULE_VERSION_STRING "\n");
    return ret;

watchdog_init_errC:
    misc_deregister(&hidog_miscdev);
watchdog_init_errB:
    unregister_reboot_notifier(&hidog_notifier);
watchdog_init_errA:
    return ret;
}

static void __exit hidog_exit(void)
{
    hidog_set_timeout(0);
    hidog_stop();
#if 0
    for(hidog_state=HIDOG_EXIT; !nodeamon; ) {
        #if 0
		struct task_struct *p_dog = find_task_by_vpid(pid_hidog_deamon);    
        if(p_dog == NULL)
            break;
        #endif
        
        struct task_struct *p_dog = task_hidog_deamon;
		if(p_dog == NULL)
			break;
		wake_up_process(p_dog);
		yield();
    }
#endif
    hidog_state=HIDOG_EXIT;
    if(!nodeamon) {
        struct task_struct *p_dog = task_hidog_deamon;
        if(p_dog == NULL)
            return;
        wake_up_process(p_dog);
        kthread_stop(p_dog);
        yield();
    }
    
    task_hidog_deamon = NULL;
}


static void __exit watchdog_exit(void)
{    
    misc_deregister(&hidog_miscdev);
    unregister_reboot_notifier(&hidog_notifier);

    hidog_exit();
    reg_ctl_base_va = NULL;
    reg_wdt_base_va = NULL;
}


module_init(watchdog_init);
module_exit(watchdog_exit);
/*
MODULE_AUTHOR("Frank Law");
MODULE_DESCRIPTION("Guangzhou JUAN Watchdog Device Driver");
MODULE_LICENSE("LGPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
MODULE_VERSION("v" WATCHDOG_VERSION_STR);
*/
	MODULE_AUTHOR("JUAN");
	MODULE_DESCRIPTION("Guangzhou JUAN Watchdog Device Driver");
	MODULE_LICENSE("GPL");
	MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
	MODULE_VERSION("HI_VERSION=" OSDRV_MODULE_VERSION_STRING);


#else //__KERNEL__

#define WATCHDOG_TRACE(fmt...) \
    do{printf("\033[1;31mWATCHDOG->[%s]:%d ", __FUNCTION__, __LINE__);printf(fmt);printf("\033[m\r\n");}while(0)

#define WATCHDOG_DEV "/dev/watchdog"

typedef struct WatchDog
{
	uint32_t autofeed_trigger;
	pthread_t autofeed_tid;

	int timeout_s;
	int fid;
	
}WatchDog_t;
static WatchDog_t* _watchdog = NULL;

int WATCHDOG_init(int timeout_s)
{
	if(!_watchdog){
		_watchdog = calloc(sizeof(WatchDog_t), 1);
		//assert(_watchdog);
		// init
		_watchdog->autofeed_tid = (pthread_t)NULL;
		_watchdog->autofeed_trigger = false;
		_watchdog->timeout_s = timeout_s;
		_watchdog->fid = open(WATCHDOG_DEV, O_RDWR);
		if(_watchdog->fid < 0){
			_watchdog->fid = 0;
		}
		WATCHDOG_set_timeout(timeout_s);
		WATCHDOG_TRACE("get watchdog timeout:%lu \r\n", WATCHDOG_get_timeout());
		WATCHDOG_enable();
		return 0;
	}
	return -1;
}

void WATCHDOG_destroy()
{
	if(_watchdog){
		WATCHDOG_disable();
		if(_watchdog->fid > 0){
			close(_watchdog->fid);
			_watchdog->fid = 0;
		}
		free(_watchdog);
		_watchdog = NULL;
	}
}

static int watchdog_enable(int flag)
{
	int ret = 0;
	int status = flag ? WDIOS_ENABLECARD : WDIOS_DISABLECARD;
	if(_watchdog->fid > 0){
		ret = ioctl(_watchdog->fid, WDIOC_SETOPTIONS, &status);
		if(0 == ret){	
			return 0;
		}
	}
	WATCHDOG_TRACE("%s Failed!", flag ? "Enable" : "Disable");
	return -1;
}


int WATCHDOG_enable()
{
	return watchdog_enable(true);
}

int WATCHDOG_disable()
{
	return watchdog_enable(false);
}

int WATCHDOG_get_timeout()
{
	int ret = 0;
	int timeout_s = 0;
	if(_watchdog->fid > 0){
		ret = ioctl(_watchdog->fid, WDIOC_GETTIMEOUT, &timeout_s);
		if(0 == ret){	
			return timeout_s;
		}
	}
	WATCHDOG_TRACE("Get Timeout Failed!");
	return -1;
}

// 1 - 1431 second, 0 driver default
int WATCHDOG_set_timeout(int timeout_s)
{
	int ret = 0;
	if(!(timeout_s >= 0 && timeout_s <= 2862)){
		WATCHDOG_TRACE("Timeout Out of Range!");
		return -1;
	}
	if(_watchdog->fid > 0){
		ret = ioctl(_watchdog->fid, WDIOC_SETTIMEOUT, &timeout_s);
		if(0 == ret){
			_watchdog->timeout_s = WATCHDOG_get_timeout();
			return 0;
		}
	}
	WATCHDOG_TRACE("Set Timeout Failed!");
	return 0;
}

static int watchdog_keepalive()
{
	int ret = 0;
	int arg = 0;
	if(_watchdog->fid > 0){
		ret = ioctl(_watchdog->fid, WDIOC_KEEPALIVE, &arg);
		if(0 == ret){
			return 0;
		}
	}
//	WATCHDOG_TRACE("Keepalive Failed!");
	return -1;
}

int WATCHDOG_feed()
{
	return watchdog_keepalive();
}

#endif //__KERNEL__

