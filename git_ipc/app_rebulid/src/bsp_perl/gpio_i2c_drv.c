#include "drv_headers.h"

#include "hi3515_drv.h"
#include "gpio_i2c_drv.h" 

#define HW_REG(reg) *((volatile unsigned int *)(reg))
#define DELAY(us)       udelay(us+us)

#define PINI2C_SCL 0
#define PINI2C_SDA 1
#define PINI2C_DAT 2
#define PINI2C_NUM 3

#if defined(SDK_PLATFORM_HI3516A)
#define GPIO_I2C_NUM (1)
const unsigned long GPIOGRP[GPIO_I2C_NUM] = {6};
const unsigned long GPIOSCL[GPIO_I2C_NUM] = {1 << 0};
const unsigned long GPIOSDA[GPIO_I2C_NUM] = {1 << 3};
#endif

//Take Care DECLARE_MUTEX Macro DEFINE
static spinlock_t  i2c_sem[GPIO_I2C_NUM];

#define I2C_LOCK(x) do{spin_lock(&i2c_sem[(x)]);}while(0)
#define I2C_UNLOCK(x) do{spin_unlock(&i2c_sem[(x)]);}while(0)

unsigned long GPIO_I2C_BIT[GPIO_I2C_NUM][PINI2C_NUM];
unsigned long GPIO_I2C_DIR[GPIO_I2C_NUM];
unsigned long GPIO_I2C_PIN[GPIO_I2C_NUM][PINI2C_NUM];

EXPORT_SYMBOL(gpio_i2c_lines);
int  gpio_i2c_lines(void)
{
    return GPIO_I2C_NUM;
}

void StaticParam_Init(void)
{
    int i;
#if defined(SDK_PLATFORM_HI3516A)
	reg_write32(IO_CONFIG_REG(4), 0); // GPIO6_0, VO_DAT12
	reg_write32(IO_CONFIG_REG(7), 0); // GPIO6_3, VO_DAT15
#endif
    for(i = 0; i < GPIO_I2C_NUM; i ++) {
        GPIO_I2C_DIR[i]         = IO_ADDRESS(GPIO_DIR(GPIOGRP[i]));
        GPIO_I2C_BIT[i][PINI2C_SCL] = GPIOSCL[i];
        GPIO_I2C_BIT[i][PINI2C_SDA] = GPIOSDA[i];
        GPIO_I2C_BIT[i][PINI2C_DAT] = GPIOSCL[i] | GPIOSDA[i];
        GPIO_I2C_PIN[i][PINI2C_SCL] = IO_ADDRESS(GPIO_BASE(GPIOGRP[i]) + (GPIO_I2C_BIT[i][PINI2C_SCL] << 2));
        GPIO_I2C_PIN[i][PINI2C_SDA] = IO_ADDRESS(GPIO_BASE(GPIOGRP[i]) + (GPIO_I2C_BIT[i][PINI2C_SDA] << 2));
        GPIO_I2C_PIN[i][PINI2C_DAT] = IO_ADDRESS(GPIO_BASE(GPIOGRP[i]) + (GPIO_I2C_BIT[i][PINI2C_DAT] << 2));

        spin_lock_init(&i2c_sem[i]);
    }
}

static void i2c_clr(int Grp, int Pin)
{
    unsigned int  tmpVal;

    tmpVal  = HW_REG(GPIO_I2C_DIR[Grp]);
    tmpVal |= GPIO_I2C_BIT[Grp][Pin];
    HW_REG(GPIO_I2C_DIR[Grp]) = tmpVal;
    HW_REG(GPIO_I2C_PIN[Grp][Pin]) = 0;
}

static void  i2c_set(int Grp, int Pin)
{
    unsigned int  tmpVal;

    tmpVal  = HW_REG(GPIO_I2C_DIR[Grp]);
    tmpVal |= GPIO_I2C_BIT[Grp][Pin];
    HW_REG(GPIO_I2C_DIR[Grp]) = tmpVal;
    HW_REG(GPIO_I2C_PIN[Grp][Pin]) = GPIO_I2C_BIT[Grp][Pin];
}

static inline unsigned int i2c_read_bit(int Grp)
{
    unsigned int tmpVal;

    //tmpVal  = HW_REG(GPIO_I2C_DIR[Grp]);
    //tmpVal &= (~GPIO_I2C_BIT[Grp][PINI2C_SDA]);
    //HW_REG(GPIO_I2C_DIR[Grp]) = tmpVal;

    //DELAY(1);
    tmpVal = HW_REG(GPIO_I2C_PIN[Grp][PINI2C_SDA]);

    return (tmpVal & GPIO_I2C_BIT[Grp][PINI2C_SDA]);
}

static void i2c_start_bit(int Grp)
{
    local_irq_disable();

    DELAY(1);

    i2c_set(Grp, PINI2C_SDA);
    i2c_set(Grp, PINI2C_SCL);
    DELAY(1);

    i2c_clr(Grp, PINI2C_SDA);
    DELAY(1);

    local_irq_enable();
}

static void i2c_stop_bit(int Grp)
{
    local_irq_disable();

    i2c_clr(Grp, PINI2C_SCL);
    DELAY(1);

    i2c_clr(Grp, PINI2C_SDA);
    DELAY(1);

    i2c_set(Grp, PINI2C_SCL);
    DELAY(1);

    i2c_set(Grp, PINI2C_SDA);
    DELAY(2);

    local_irq_enable();
}

static void i2c_send_byte(int Grp, unsigned char c)
{
    int i;

    local_irq_disable();

    for (i = 0; i < 8; i ++) {
        i2c_clr(Grp, PINI2C_SCL);
        DELAY(1);

        if (c & (1<<(7-i)))
            i2c_set(Grp, PINI2C_SDA);
        else
            i2c_clr(Grp, PINI2C_SDA);
        DELAY(1);

        i2c_set(Grp, PINI2C_SCL);
        DELAY(1);

        i2c_clr(Grp, PINI2C_SCL);
		i2c_clr(Grp, PINI2C_SDA);
		DELAY(1);
    }

    local_irq_enable();
}

static unsigned char i2c_receive_byte(int Grp)
{
    int j = 0;
    int i;
    unsigned char regvalue;

    local_irq_disable();

    for (i = 0; i < 8; i ++) {
        i2c_clr(Grp, PINI2C_SCL);
        DELAY(1);

        regvalue  = HW_REG(GPIO_I2C_DIR[Grp]);
        regvalue &= (~GPIO_I2C_BIT[Grp][PINI2C_SDA]);
        HW_REG(GPIO_I2C_DIR[Grp]) = regvalue;
        DELAY(1);

        i2c_set(Grp, PINI2C_SCL);
        DELAY(1);

        if (i2c_read_bit(Grp)) {
            j+=(1<<(7-i));
        }
        DELAY(1);

        i2c_clr(Grp, PINI2C_SCL);
		DELAY(1);
    }

    local_irq_enable();

    return j;
}

static int i2c_receive_ack(int Grp)
{
    int nack;
    unsigned char regvalue;

    local_irq_disable();

    i2c_clr(Grp, PINI2C_SCL);
    DELAY(1);

    regvalue  = HW_REG(GPIO_I2C_DIR[Grp]);
    regvalue &= (~GPIO_I2C_BIT[Grp][PINI2C_SDA]);
    HW_REG(GPIO_I2C_DIR[Grp]) = regvalue;
    DELAY(1);

    i2c_set(Grp, PINI2C_SCL);
    DELAY(1);

    nack = i2c_read_bit(Grp);
    DELAY(1);

    i2c_clr(Grp, PINI2C_SCL);
    DELAY(1);

    local_irq_enable();

    if (nack == 0)
        return 1;

    return 0;
}

static void i2c_send_ack(int Grp)
{
    local_irq_disable();

    i2c_clr(Grp, PINI2C_SCL);
    DELAY(1);

    i2c_set(Grp, PINI2C_SDA);
    DELAY(1);

    i2c_set(Grp, PINI2C_SCL);
    DELAY(1);

    i2c_clr(Grp, PINI2C_SCL);
    DELAY(1);

    i2c_clr(Grp, PINI2C_SDA);
    DELAY(1);

    local_irq_enable();
}

EXPORT_SYMBOL(gpio_i2c_rd_arr);
int gpio_i2c_rd_arr(int Grp, unsigned char dev,
    unsigned char rarr[], int rsize,
    unsigned char darr[], int dsize)
{
    int i;

    I2C_LOCK(Grp);

    i2c_start_bit(Grp);

    i2c_send_byte(Grp, (unsigned char)(dev));
    i2c_receive_ack(Grp);

    for(i = 0; i < rsize; i ++) {
        i2c_send_byte(Grp, rarr[i]);
        i2c_receive_ack(Grp);
    }

    if(dsize > 0) {
        i2c_start_bit(Grp);

        i2c_send_byte(Grp, (unsigned char)(dev) | 1);
        i2c_receive_ack(Grp);

	    for(i = 0; i < dsize-1; i ++) {
	        darr[i] = i2c_receive_byte(Grp);
	        i2c_send_ack(Grp);
	    }

	    darr[i] = i2c_receive_byte(Grp);
	    i2c_receive_ack(Grp);
    }

	i2c_stop_bit(Grp);

	I2C_UNLOCK(Grp);
    
    return 0;
}

EXPORT_SYMBOL(gpio_i2c_wr_arr);
int gpio_i2c_wr_arr(int Grp, unsigned char dev,
    unsigned char rarr[], int rsize,
    unsigned char darr[], int dsize)
{
    int i;

    I2C_LOCK(Grp);

    i2c_start_bit(Grp);

    i2c_send_byte(Grp, (unsigned char)(dev));
    i2c_receive_ack(Grp);

    for(i = 0; i < rsize; i ++) {
        i2c_send_byte(Grp, rarr[i]);
        i2c_receive_ack(Grp);
    }

    if(dsize > 0) {
	    for(i = 0; i < dsize-1; i ++) {
	        i2c_send_byte(Grp, darr[i]);
	        i2c_receive_ack(Grp);
	    }

	    i2c_send_byte(Grp, darr[i]);
	    i2c_receive_ack(Grp);
    }

    i2c_stop_bit(Grp);

    I2C_UNLOCK(Grp);
    
    return 0;
}

EXPORT_SYMBOL(gpio_i2c_read);
int gpio_i2c_read(unsigned char devaddress, unsigned char address)
{
    int rxdata;
    int ret ;
    int Grp = 0;

    I2C_LOCK(Grp);

    i2c_start_bit(Grp);
    i2c_send_byte(Grp, (unsigned char)(devaddress));
    ret = i2c_receive_ack(Grp);
    i2c_send_byte(Grp, address);
    i2c_receive_ack(Grp);
    i2c_start_bit(Grp);
    i2c_send_byte(Grp, (unsigned char)(devaddress) | 1);
    i2c_receive_ack(Grp);
    rxdata = i2c_receive_byte(Grp);
    i2c_receive_ack(Grp);
    i2c_stop_bit(Grp);
    
    I2C_UNLOCK(Grp);
    return (unsigned char)rxdata;
}

EXPORT_SYMBOL(gpio_i2c_write);
int gpio_i2c_write(unsigned char devaddress, unsigned char address, unsigned char data)
{
    int Grp = 0;
    I2C_LOCK(Grp);
    
    i2c_start_bit(Grp);
    i2c_send_byte(Grp, (unsigned char)(devaddress));
    i2c_receive_ack(Grp);
    i2c_send_byte(Grp, address);
    i2c_receive_ack(Grp);
    i2c_send_byte(Grp, data); 
    i2c_receive_ack(Grp);
    i2c_stop_bit(Grp);

    I2C_UNLOCK(Grp);

    return 0 ;
}

EXPORT_SYMBOL(gpio_i2c_probe);
int gpio_i2c_probe(unsigned char devaddress)
{
    int ret;
    int Grp = 0;
    
    I2C_LOCK(Grp);
    
    i2c_start_bit(Grp);
    i2c_send_byte(Grp, (unsigned char)(devaddress));
    ret = i2c_receive_ack(Grp);
    i2c_stop_bit(Grp);

    I2C_UNLOCK(Grp);

    return ret;
}

EXPORT_SYMBOL(gpio_i2c_readX);
int gpio_i2c_readX(int Grp, unsigned char devaddress, unsigned char address)
{
    int rxdata;

    I2C_LOCK(Grp);

    i2c_start_bit(Grp);

    i2c_send_byte(Grp, (unsigned char)(devaddress));
    i2c_receive_ack(Grp);

    i2c_send_byte(Grp, address);
    i2c_receive_ack(Grp);
     
    i2c_start_bit(Grp);

    i2c_send_byte(Grp, (unsigned char)(devaddress) | 1);
    i2c_receive_ack(Grp);

    rxdata = i2c_receive_byte(Grp);
    i2c_receive_ack(Grp);

    i2c_stop_bit(Grp);
    
    I2C_UNLOCK(Grp);

    return (unsigned char)rxdata;
}

EXPORT_SYMBOL(gpio_i2c_writeX);
int gpio_i2c_writeX(int Grp, unsigned char devaddress, unsigned char address, unsigned char data)
{
    I2C_LOCK(Grp);
    
    i2c_start_bit(Grp);

    i2c_send_byte(Grp, (unsigned char)(devaddress));
    i2c_receive_ack(Grp);

    i2c_send_byte(Grp, address);
    i2c_receive_ack(Grp);

    i2c_send_byte(Grp, data); 
    i2c_receive_ack(Grp);

    i2c_stop_bit(Grp);

    I2C_UNLOCK(Grp);

    return 0 ;
}

EXPORT_SYMBOL(gpio_i2c_probeX);
int gpio_i2c_probeX(int Grp, unsigned char devaddress)
{
    int ret;
    
    I2C_LOCK(Grp);
    
    i2c_start_bit(Grp);
    i2c_send_byte(Grp, (unsigned char)(devaddress));
    ret = i2c_receive_ack(Grp);
    i2c_stop_bit(Grp);

    I2C_UNLOCK(Grp);

    return ret;
}

EXPORT_SYMBOL(gpio_i2c_readX_KeyPad_Fix);
int gpio_i2c_readX_KeyPad_Fix(
    int Grp,
    unsigned char Dev,
    void (*Func[2])(void))
{
    int rxdata;

    I2C_LOCK(Grp);

    if(Func[0]) {
        (*Func[0])();
    }

    i2c_start_bit(Grp);

    i2c_send_byte(Grp, (unsigned char)(Dev) | 1);
    i2c_receive_ack(Grp);

    rxdata = i2c_receive_byte(Grp);
    i2c_receive_ack(Grp);

    i2c_stop_bit(Grp);

    if(Func[1]) {
        (*Func[1])();
    }

    I2C_UNLOCK(Grp);

    return (unsigned char)rxdata;
}

EXPORT_SYMBOL(gpio_i2c_writeX_KeyPad_Fix);
int gpio_i2c_writeX_KeyPad_Fix(
    int Grp,
    unsigned char Dev,
    unsigned char rarr[], int rsize,
    unsigned char darr[], int dsize,
    void (*Func[2])(void))
{
    int i;

    I2C_LOCK(Grp);

    if(Func[0]) {
        (*Func[0])();
    }

    i2c_start_bit(Grp);

    i2c_send_byte(Grp, Dev);
    i2c_receive_ack(Grp);

    for(i = 0; i < rsize; i ++) {
        i2c_send_byte(Grp, rarr[i]);
        i2c_receive_ack(Grp);
    }

    for(i = 0; i < dsize; i ++) {
        i2c_send_byte(Grp, darr[i]);
        i2c_receive_ack(Grp);
    }

    i2c_stop_bit(Grp);

    if(Func[1]) {
        (*Func[1])();
    }

    I2C_UNLOCK(Grp);

    return 0;
}

static DEFINE_IOCTL(gi2c_ioctl, file, cmd, arg)
{
    int ret = 0;

    void __user *argp = (unsigned int __user *)arg;
    GI2C_ARR_STRUCT gi2c_Hdr;

    switch(cmd) {
    case GPIO_I2C_ARR_RD: {
        ret = copy_from_user(&gi2c_Hdr, argp, sizeof(gi2c_Hdr));
        if(ret) {
            return ret;
        }
        if((gi2c_Hdr.Grp >= GPIO_I2C_NUM)
        || (gi2c_Hdr.RSize + gi2c_Hdr.DSize > sizeof(gi2c_Hdr.Buf))) {
            printk("RD:gi2c_Hdr.Grp = %d\n", gi2c_Hdr.Grp);
            printk("RD:gi2c_Hdr.Rsiz= %d\n", gi2c_Hdr.RSize);
            printk("RD:gi2c_Hdr.DSiz= %d\n", gi2c_Hdr.DSize);
            return -1;
        }

        gpio_i2c_rd_arr(
            gi2c_Hdr.Grp,
            gi2c_Hdr.Dev,
            &gi2c_Hdr.Buf[0],
            gi2c_Hdr.RSize,
            &gi2c_Hdr.Buf[gi2c_Hdr.RSize],
            gi2c_Hdr.DSize);

        if ((ret = copy_to_user(argp,
                &gi2c_Hdr,
                sizeof(gi2c_Hdr)))) {

                return ret;
        }
    }
    break;

    case GPIO_I2C_ARR_WR: {
        ret = copy_from_user(&gi2c_Hdr, argp, sizeof(gi2c_Hdr));
        if(ret) {
            return ret;
        }
        if((gi2c_Hdr.Grp >= GPIO_I2C_NUM)
        || (gi2c_Hdr.RSize + gi2c_Hdr.DSize > sizeof(gi2c_Hdr.Buf))) {
            printk("WR:gi2c_Hdr.Grp = %d\n", gi2c_Hdr.Grp);
            printk("WR:gi2c_Hdr.Rsiz= %d\n", gi2c_Hdr.RSize);
            printk("WR:gi2c_Hdr.DSiz= %d\n", gi2c_Hdr.DSize);
            return -1;
        }

        return gpio_i2c_wr_arr(
            gi2c_Hdr.Grp,
            gi2c_Hdr.Dev,
            &gi2c_Hdr.Buf[0],
            gi2c_Hdr.RSize,
            &gi2c_Hdr.Buf[gi2c_Hdr.RSize],
            gi2c_Hdr.DSize);
    }
    break;

    default:
        return -1;
    }

    return ret;
}

int gi2c_open(struct inode * inode, struct file * file)
{
    return 0;
}

int gi2c_close(struct inode * inode, struct file * file)
{
    return 0;
}

static struct file_operations gi2c_fops = {
    .owner      = THIS_MODULE,
    .ioctl      = gi2c_ioctl,
    .open       = gi2c_open,
    .release    = gi2c_close
};

static struct miscdevice gi2c_dev = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "gpio_i2c",
    .fops   = &gi2c_fops,
};

int gpio_i2c_init(void)
{
    StaticParam_Init();

    printk("GPIO I2C for HI35xx @ %s %s\n", __TIME__, __DATE__);

    if(misc_register(&gi2c_dev)) {
       printk("Register gi2c device Failed!\n");
       return -1;
    }

    return 0;
}

void gpio_i2c_exit(void)
{
    misc_deregister(&gi2c_dev);
}

#ifndef GPIO_DEV_WITH_I2C
module_init(gpio_i2c_init);
module_exit(gpio_i2c_exit);

MODULE_LICENSE("GPL");
#endif



