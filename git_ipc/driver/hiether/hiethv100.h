
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/list.h>
#include <asm/io.h>

#ifndef __HISILICON_ETHV100_H
#define __HISILICON_ETHV100_H

#define HIETH_DEBUG	1

#define OSDRV_MODULE_VERSION_STRING "HIETHV100-M03C0301 @Hi3511v110_OSDrv_1_0_0_7 2009-03-18 20:53:51"

/* ***********************************************************
 *
 * Global varibles and defintions
 *
 *************************************************************
 */

/* configuerable values */

/* mdiobus device name, such as platform device name */
#define HIETH_MDIOBUS_NAME "hiethv100_mdiobus"
/* ethv100 device name, such as platform device name */
#define HIETH_DEV_NAME "hiethv100"

#define HIETH_MAX_QUEUE_DEPTH 64

#define HIETH_MAX_FRAME_SIZE 1600

#ifdef HIETH_DEBUG	
#define HIETH_TRACE_LEVEL 8
#define hieth_trace(level, msg...) do { \
		if((level) >= HIETH_TRACE_LEVEL) { \
			printk(KERN_INFO "hieth_trace:%s:%d: ", __FILE__, __LINE__); \
			printk(msg); \
			printk("\n"); \
		} \
	}while(0)

#define hieth_assert(cond) do{ \
		if(!(cond)) \
			printk("Assert:hieth:%s:%d\n", \
					__FILE__, \
					__LINE__); \
	}while(0)
#else
#define HIETH_TRACE_LEVEL 0xffff
#define hieth_trace(level, msg...) 
#define hieth_assert(cond) 
#endif

#define hieth_error(s...) do{ \
		printk(KERN_ERR "hieth:%s:%d: ", __FILE__, __LINE__); \
		printk(s); \
		printk("\n"); \
	}while(0)

#define hil_vqueue_assert(cond) hieth_assert(cond)
#define hil_vqueue_error(s...) hieth_error(s)
#define hil_vqueue_trace_printf(level, s...) do{ if((level) >=HIETH_TRACE_LEVEL)printk(s); }while(0)

#include "vqueue.h"

/* Error number */
#define HIETH_E_QUEUE	(-1)
#define HIETH_E_BUSY	(-2)
#define HIETH_E_FULL	(-3)
#define HIETH_E_EMPTY	(-4)

struct hieth_frame_desc {
	unsigned long frm_addr;		/* required by the controler */
	unsigned int  frm_len :11;	/* required by the controler */

	/* optional */
	unsigned int mb_cached:1;	/* is this mem-block cached */
	unsigned int mb_type  :4;	/* memory type */
	unsigned int mb_pages :4;	/* pages of the prime mem-block */

	void *priv_data;
};
#define hieth_fd_copy(newfd, fd) do{ newfd = fd; }while(0)

#define HIETH_FD_INIT(fd)	(void)memset(&(fd), 0, sizeof(fd))
#define HIETH_FD_TYPE(fd)	((fd).flags &0x03)

#define hieth_trace_fd(level, fd) hieth_trace(level, \
		#fd "<%p>={ .frm_addr=%08lx, .priv_data=%p, .frm_len=%d, " \
		".mb_type=%02x, .mb_pages=%d, .mb_cached=%d }", \
		&(fd), (fd).frm_addr, (fd).priv_data, (fd).frm_len, \
		(fd).mb_type, (fd).mb_pages, (fd).mb_cached)

/* frame desc mb_type */
#define MBTYPE_DMA_COHERENT	1
#define MBTYPE_SKBBUF	2
#define MBTYPE_KMALLOC	3
#define MBTYPE_KMEM_CACHE	4
#define MBTYPE_MMZ	5


struct hieth_netdev_local {
	unsigned long iobase;	/* virtual io addr */
	unsigned long iobase_phys; /* physical io addr */
	int irq;
	int devid;

	struct net_device_stats stats;

	unsigned long hclk;

	struct hieth_frame_desc hwq_fd[HIETH_MAX_QUEUE_DEPTH];
	struct hil_vqueue hwq_xmit;
	struct hil_vqueue hwq_recv;

	struct hieth_frame_desc *swq_recv_fd;
	struct hil_vqueue swq_recv_ready;
	struct hil_vqueue swq_recv_blank; /* blank between the front of ready and the back of free */
	struct hil_vqueue swq_recv_free;

	struct hieth_frame_desc *swq_xmit_poolfd;
	struct hil_vqueue swq_xmit_pool;

	struct {
		int hw_xmitq;
		int hw_recvq;
		int sw_xmit_pool;
		int sw_recv_rbf;
	} depth;

	struct {
		int cnt_free_limit;
		char skb_feeding;
	} stat;

	struct {
		int recv_mb_type	:8;

		int xmit_force_copy	:1;
		int recv_force_copy	:1;
	} opt;


	void *xmit_buf;
	dma_addr_t xmit_buf_dma;
	void *recv_buf;
	dma_addr_t recv_buf_dma;

	const char *phy_name;
	struct phy_device *phy;
	int link_stat;

	spinlock_t lock;
	unsigned long lockflags;

	struct tasklet_struct bf_recv;
	struct work_struct wks_skbfeed;

	struct {
		struct timer_list netif_rx_busy;
	} timer;
};

/* ***********************************************************
 *
 * Only for internal used!
 *
 * ***********************************************************
 */
#ifdef HIETHV100_INTER

/* read/write IO */
#define hieth_readl(ld, ofs) ({ unsigned long reg=readl((ld)->iobase + (ofs)); \
				hieth_trace(2, "readl(0x%04X) = 0x%08lX", (ofs), reg); \
				reg; })
#define hieth_writel(ld, v, ofs) do{ writel(v, (ld)->iobase + (ofs)); \
				hieth_trace(2, "writel(0x%04X) = 0x%08lX", (ofs), (unsigned long)(v)); \
			}while(0)

#define MK_BITS(shift, nbits)	((((shift)&0x1F)<<16) | ((nbits)&0x1F))

#define hieth_writel_bits(ld, v, ofs, bits_desc) do{ \
		unsigned long _bits_desc = bits_desc; \
		unsigned long _shift = (_bits_desc)>>16; \
		unsigned long _reg = hieth_readl(ld, ofs); \
		unsigned long _mask = ((1<<(_bits_desc & 0x1F)) - 1)<<(_shift); \
		hieth_writel(ld, (_reg &(~_mask)) | (((v)<<(_shift)) &_mask), ofs); \
	} while(0)
#define hieth_readl_bits(ld, ofs, bits_desc) ({ \
		unsigned long _bits_desc = bits_desc; \
		unsigned long _shift = (_bits_desc)>>16; \
		unsigned long _mask = ((1<<(_bits_desc & 0x1F)) - 1)<<(_shift); \
		(hieth_readl(ld, ofs)&_mask)>>(_shift); })

/* #define udelay(us)	udelay(us) */

#define local_lock_init(ld)	spin_lock_init(&(ld)->lock)
#define local_lock_exit(ld)	
#define local_lock(ld)		spin_lock_irqsave(&(ld)->lock, (ld)->lockflags)
#define local_unlock(ld)	spin_unlock_irqrestore(&(ld)->lock, (ld)->lockflags)

#endif

#endif

