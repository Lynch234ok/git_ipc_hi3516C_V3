
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
#include <linux/workqueue.h>
#include <asm/atomic.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include "hiethv100.h"
#include "mdio.h"
#include "mac.h"
#include "ctrl.h"
#include "glb.h"
#include "sys.h"

#if 0
static inline void eth_fixed_memmove(void *dst, void *src, int len)
{
	int tmp = (len-20)&(~0xF);

	if(!tmp) {
		memmove(dst, src, len);
		return;
	}

	len = len - tmp - 2;

	__asm__ __volatile__(
			"ldr	r4, [%1], #4	\n"
			"strh	r4, [%0], #2	\n"
			"lsr	r4, #16		\n"

			"1:"
			"ldmia	%1!, {r5-r8}	\n"
			"pld	[%1, #16]	\n"

			"orr	r4, r4, r5, lsl#16	\n"

			"lsr	r5, #16		\n"
			"orr	r5, r5, r6, lsl#16	\n"

			"lsr	r6, #16		\n"
			"orr	r6, r6, r7, lsl#16	\n"

			"lsr	r7, r7, #16		\n"
			"orr	r7, r7, r8, lsl#16	\n"

			"stmia	%0!, {r4-r7}	\n"

			"mov	r4, r8, lsr#16	\n"

			"subs	%2, #16	\n"
			"bne	1b		\n"
			
			"mov	%2, r4	\n"

			: "=r"(dst), "=r"(src), "=r"(tmp)
			: "0"(dst), "1"(src), "2"(tmp)
			: "r4", "r5", "r6", "r7", "r8"
			);

	memmove((char*)dst+2, (char*)src, len);
	*(short *)dst = tmp;
}
#endif

/* ETH Driver Interface */

int hieth_mdiobus_driver_init(void);
void hieth_mdiobus_driver_exit(void);

int hieth_mm_init(void);
void hieth_mm_exit(void);

int hieth_queue_setup(struct net_device *dev);
int hieth_create_hwq_xmit(struct net_device *dev, int blk_size, int blk_nr, int mb_type);
int hieth_create_hwq_recv(struct net_device *dev, int blk_size, int blk_nr, int mb_type);
int hieth_queue_cleanup(struct net_device *dev);
int hieth_recvq_skb_feed(struct net_device *dev, int nr, unsigned long prio);

static int hieth_recv_mb_type = MBTYPE_KMEM_CACHE;
module_param_named(rv_strat, hieth_recv_mb_type, int, 0600);

static int hieth_recv_rbf_size = HIETH_MAX_QUEUE_DEPTH;
module_param_named(rv_qcnt, hieth_recv_rbf_size, int, 0600);

static int hieth_hwq_recv_depth = HIETH_MAX_QUEUE_DEPTH/2;
module_param_named(hwq_recv, hieth_hwq_recv_depth, int, 0600);

static int hieth_hwq_xmit_depth = HIETH_MAX_QUEUE_DEPTH/2;
module_param_named(hwq_xmit, hieth_hwq_xmit_depth, int, 0600);

static int hieth_recv_pool_free_limit = HIETH_MAX_QUEUE_DEPTH/2;
module_param_named(flimit, hieth_recv_pool_free_limit, int, 0600);

static int hieth_probe_noise = 0;
module_param_named(noise, hieth_probe_noise, int, 0600);

static struct workqueue_struct *hieth_wq;

static void wksproc_recvq_skb_feed(void *data)
{
	struct net_device *dev = data;
	struct hieth_netdev_local *ld = netdev_priv(dev);

	if(hieth_recvq_free_count(ld) <1) { /* pool dry */
		if(hieth_probe_noise)
			printk(KERN_INFO "hieth: rx-skb-pool dry!\n");
	} 
	
	DUMP_VQUEUE_RBF(7, ld);

	hieth_recvq_skb_feed(dev, ld->depth.sw_recv_rbf, GFP_KERNEL);

	DUMP_VQUEUE_RBF(7, ld);

	ld->stat.skb_feeding = 0;

	hieth_hw_recv_tryup(ld);

	DUMP_VQUEUE_RBF(7, ld);
}

static inline int queue_work_skb_feed(struct net_device *dev)
{
	struct hieth_netdev_local *ld = netdev_priv(dev);

	if(ld->stat.skb_feeding)
		return 0;

	if(hieth_recvq_free_count(ld) > ld->stat.cnt_free_limit)
		return 0;

	if(!list_empty(&ld->wks_skbfeed.entry))
		return 0;

	ld->stat.skb_feeding = 1;

	INIT_WORK(&ld->wks_skbfeed, wksproc_recvq_skb_feed, dev);

	return queue_work(hieth_wq, &ld->wks_skbfeed);
}
		

static void hieth_all_restart(struct net_device *dev)
{
	BUG();
}

static void hieth_adjust_link(struct net_device *dev)
{
	int stat = 0;
	struct hieth_netdev_local *ld = netdev_priv(dev);

	stat |= (ld->phy->link) ? HIETH_LINKED : 0;
	stat |= (ld->phy->duplex==DUPLEX_FULL) ? HIETH_DUP_FULL : 0;
	stat |= (ld->phy->speed == SPEED_100) ? HIETH_SPD_100M : 0;

	if(stat != ld->link_stat) {
		hieth_set_linkstat(ld, stat);
		phy_print_status(ld->phy);
		ld->link_stat = stat;
	}
}

static int proc_recv_copymode(struct net_device *dev, struct hieth_frame_desc *fd)
{
	int frmlen = fd->frm_len;
	struct sk_buff *skb =NULL;
	struct hieth_netdev_local *ld = netdev_priv(dev);

	skb = dev_alloc_skb(frmlen +2);
	if(skb ==NULL) {
		hieth_error("alloc_skb(%d) failed!", frmlen);
		return -ENOMEM;
	}
	skb_reserve(skb, 2);
	memcpy(skb->data, fd->priv_data, frmlen);
	//eth_fixed_memmove(skb->data, fd->priv_data, frmlen);

	/* you have to invalid cpu cache before hw use it */
	if(fd->mb_cached)
		consistent_sync(fd->priv_data, frmlen, DMA_FROM_DEVICE);

	skb->dev = dev;
	skb_put(skb, frmlen);
	skb->protocol = eth_type_trans(skb, dev);
	dev->last_rx = jiffies;

	ld->stats.rx_packets++;
	ld->stats.rx_bytes += skb->len;

	return netif_rx(skb);
}

static int proc_recv_skbbuff(struct net_device *dev, struct hieth_frame_desc *fd)
{
	int ret;
	struct hieth_netdev_local *ld = netdev_priv(dev);
	struct sk_buff *skb = fd->priv_data;

	skb->dev = dev;

	/* if address align to 4, then move to align 2 */
	if(!((ulong)skb->data & 0x03)) { 
		//eth_fixed_memmove((char*)skb->data +2, skb->data, fd->frm_len);
		memmove(skb->data +2, skb->data, fd->frm_len);
		skb_reserve(skb, 2);
	}

	skb_put(skb, fd->frm_len);
	skb->protocol = eth_type_trans(skb, dev);
	dev->last_rx = jiffies;

	ld->stats.rx_packets++;
	ld->stats.rx_bytes += skb->len;

	skb_get(skb);
	ret = netif_rx(skb);
	if(ret == NET_RX_SUCCESS)
		kfree_skb(skb);

	return ret;
}

static void hieth_bfproc_recv(unsigned long data)
{
#define pause_netif_rx() do { \
		unsigned long _pnr_flags; \
		local_irq_save(_pnr_flags); \
		if (!test_and_set_bit(TASKLET_STATE_SCHED, &ld->bf_recv.state)) { \
			ld->timer.netif_rx_busy.function = hieth_bfproc_recv; \
			ld->timer.netif_rx_busy.data = data; \
			ld->timer.netif_rx_busy.expires = jiffies + msecs_to_jiffies(50); \
			add_timer(&ld->timer.netif_rx_busy); \
		} \
		local_irq_restore(_pnr_flags); \
	} while(0)

	int feed = 0;
	int ret = 0;
	struct net_device *dev = (void*)data;
	struct hieth_netdev_local *ld = netdev_priv(dev);
	struct hieth_frame_desc *fd;

	while( !ret && (fd = hieth_recv_getfd(ld)) ) {

		if(HIETH_INVALID_RXPKG_LEN(fd->frm_len)) {
			ld->stats.rx_errors ++;
			ld->stats.rx_length_errors ++;
			continue;
		}

		switch(fd->mb_type) {
			case MBTYPE_SKBBUF:
				ret = proc_recv_skbbuff(dev, fd);
				if(ret != NET_RX_SUCCESS) {
					pause_netif_rx();
					break;
				}

				if(hieth_recv_next_dropfd(ld, fd)) {
					ld->stats.rx_errors ++;
					ld->stats.rx_fifo_errors ++;
				}
				feed = 1;
				break;

			case MBTYPE_DMA_COHERENT:
			case MBTYPE_KMALLOC:
			case MBTYPE_KMEM_CACHE:
				ret = proc_recv_copymode(dev, fd);
				if(ret != NET_RX_SUCCESS) {
					pause_netif_rx();
					break;
				}

				if(hieth_recv_next_reusefd(ld, fd)) {
					ld->stats.rx_errors ++;
					ld->stats.rx_fifo_errors ++;
				}
				break;
			default:
				hieth_error("unknown fd.mb_type=%d, abort proccess.", fd->mb_type);
				return ;
		}
	}

	hieth_hw_recv_tryup(ld);

	if(feed)
		queue_work_skb_feed(dev);

	return ;
}

static irqreturn_t hieth_net_isr(int irq, void *dev_id, struct pt_regs * regs)
{
	struct net_device *dev = dev_id;
	struct hieth_netdev_local *ld = netdev_priv(dev);
	int ints;

	ints = hieth_read_irqstatus(ld);
	hieth_clear_irqstatus(ld, ints);

	if(ints & HIETH_INT_TXQUE_RDY) {
		hieth_irq_disable(ld, HIETH_INT_TXQUE_RDY);
		netif_wake_queue(dev);
	}

	if( (ints & HIETH_INT_RX_RDY) && 
			(hieth_hw_recv_tryup(ld) >0) ) {

		if(!timer_pending(&ld->timer.netif_rx_busy))
			tasklet_schedule(&ld->bf_recv);

		/* Just Clear Rx Interrupt again */
		hieth_clear_irqstatus(ld, HIETH_INT_RX_RDY);
	}

	return IRQ_HANDLED;
}

#define hieth_reset_dev(ld) do { \
		hieth_sys_reset((ld)->devid); \
		hieth_mac_reset(ld); \
		hieth_ctrl_reset(ld); \
		hieth_sys_reset((ld)->devid); } while(0)

static int hieth_net_open(struct net_device *dev)
{
	int ret = -1;
	struct hieth_netdev_local *ld = netdev_priv(dev);

	hieth_trace(3, "null");

	try_module_get(THIS_MODULE);

	/* init tasklet */
	ld->bf_recv.next = NULL;
	ld->bf_recv.state = 0;
	ld->bf_recv.func = hieth_bfproc_recv;
	ld->bf_recv.data = (unsigned long)dev;
	atomic_set(&ld->bf_recv.count, 0);

	hieth_reset_dev(ld);

	/* setup hardware */
	hieth_glb_preinit_dummy(ld);
	hieth_set_hwq_depth(ld);
	hieth_hw_flowctrl(ld, 1, ld->depth.hw_recvq*2/3, ld->depth.hw_recvq/3);
	hieth_hw_set_macaddress(ld, 1, dev->dev_addr);

	ret = hieth_queue_setup(dev);
	if(ret)
		goto _error_exit;

	ret = request_irq(dev->irq, hieth_net_isr, SA_SHIRQ, dev->name, dev);
	if(ret) {
		hieth_error("request_irq %d failed!", dev->irq);
		goto _error_exit;
	}

	netif_carrier_off(dev);
	netif_start_queue(dev);

	ld->link_stat = 0;
	phy_start(ld->phy);

	hieth_irq_enable(ld, HIETH_INT_RX_RDY);

	INIT_WORK(&ld->wks_skbfeed, NULL, NULL);
	queue_work_skb_feed(dev);

	hieth_hw_recv_tryup(ld);

	hieth_trace(3, "null");

	return ret;

_error_exit:
	hieth_reset_dev(ld);
	hieth_sys_shutdown(ld->devid);
	hieth_queue_cleanup(dev);

	module_put(THIS_MODULE);

	return ret;
}

static int hieth_net_close(struct net_device *dev)
{
	struct hieth_netdev_local *ld = netdev_priv(dev);

	hieth_glb_preinit_dummy(ld);
	phy_stop(ld->phy);
	free_irq(dev->irq, dev);

	hieth_reset_dev(ld);
	hieth_sys_shutdown(ld->devid);

	flush_workqueue(hieth_wq);

	hieth_queue_cleanup(dev);

	module_put(THIS_MODULE);

	return 0;
}

static void hieth_net_timeout(struct net_device *dev)
{
}

static int hieth_xmit_que_recycle(struct net_device *dev)
{
	int ret = -1;
	struct hieth_frame_desc fd;
	struct hieth_netdev_local *ld = netdev_priv(dev);

	while( !(ret = hieth_xmit_popfd_sendok(ld, &fd))) {

		if(fd.mb_type ==MBTYPE_DMA_COHERENT ||
				fd.mb_type ==MBTYPE_KMALLOC ||
				fd.mb_type ==MBTYPE_KMEM_CACHE) {
			if(hieth_xmit_pushfd_pool(ld, &fd)) {
				hieth_all_restart(dev);

				ld->stats.tx_errors++;
				ld->stats.tx_fifo_errors++;
				return -1;
			}
			continue;
		} else {
			hieth_error("unknown fd.mb_type=%d, abort recycle.", fd.mb_type);
			continue;
		}
	}

	return ret;
}

static int hard_start_xmit_copy(struct sk_buff *skb, struct net_device *dev)
{
	int ret = 0;
	struct hieth_frame_desc fd;
	struct hieth_netdev_local *ld = netdev_priv(dev);

	if(hieth_xmit_popfd_pool(ld, &fd)) {
		ld->stats.tx_errors++;
		ld->stats.tx_fifo_errors++;
		return NETDEV_TX_BUSY;
	}

	memcpy(fd.priv_data, skb->data, skb->len);
	fd.frm_len = skb->len + 4; /* for recalc CRC, 4 bytes more is needed */
	if(fd.mb_cached)
		consistent_sync(fd.priv_data, skb->len, DMA_TO_DEVICE);

	hieth_trace_fd(3, fd);

	ret = hieth_xmit_add_tosend(ld, &fd);
	if(ret <0) {
		ld->stats.tx_errors++;
		ld->stats.tx_fifo_errors++;
		return NETDEV_TX_BUSY;
	} else if(ret >0) {
		ld->stats.tx_dropped++;
		if(hieth_xmit_pushfd_pool(ld, &fd)) 
			hieth_all_restart(dev);
		return NETDEV_TX_BUSY;
	}

	dev->trans_start = jiffies;

	ld->stats.tx_packets++;
	ld->stats.tx_bytes += skb->len;

	return NETDEV_TX_OK;
}

static int hieth_net_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int ret = NETDEV_TX_OK;
	struct hieth_netdev_local *ld = netdev_priv(dev);

	hieth_xmit_que_recycle(dev);

	/* FIXME: here need to fix */
	if(ld->opt.xmit_force_copy) {
		ret = hard_start_xmit_copy(skb, dev);
		if(ret ==NETDEV_TX_OK)
			kfree_skb(skb);
	} else {
		hieth_error("currently not support hardware direct-xmit.");
		ret = NETDEV_TX_BUSY;
	}

	hieth_clear_irqstatus(ld, HIETH_INT_TXQUE_RDY);
	if(!hieth_hw_xmitq_ready(ld)) {
		netif_stop_queue(dev);
		hieth_irq_enable(ld, HIETH_INT_TXQUE_RDY);
	}

	return ret;
}

static struct net_device_stats *hieth_net_get_stats(struct net_device *dev)
{
	struct hieth_netdev_local *ld = netdev_priv(dev);

	return &ld->stats;
}

static void hieth_net_set_multicast_list(struct net_device *dev)
{
}

static int hieth_net_set_mac_address(struct net_device *dev, void *p)
{
	struct hieth_netdev_local *ld = netdev_priv(dev);
	struct sockaddr *skaddr = p;

	memcpy(dev->dev_addr, skaddr->sa_data, dev->addr_len);

	hieth_hw_set_macaddress(ld, 1, dev->dev_addr);

	return 0;
}

static void print_mac_address(const char *pre_msg, const unsigned char *mac, const char *post_msg)
{
	int i;
	
	if(pre_msg)
		printk(pre_msg);

	for(i=0; i<6; i++)
		printk("%02X%s", mac[i], i==5 ? "" : ":");

	if(post_msg)
		printk(post_msg);
}

static int hieth_ioctl(struct net_device *net_dev, struct ifreq *rq, int cmd)
{
        struct hieth_netdev_local *ld = netdev_priv(net_dev);

        if (!netif_running(net_dev)) return -EINVAL;

        if (!ld->phy) return -EINVAL; // PHY not controllable

        return phy_mii_ioctl(ld->phy, if_mii(rq), cmd);
}


static void hieth_get_drvinfo(struct net_device *net_dev, struct ethtool_drvinfo *info)
{
	strcpy (info->driver, "hislicon hiether v100 driver");
	strcpy (info->version, OSDRV_MODULE_VERSION_STRING);
	strcpy (info->bus_info, "platform");
}

static u32 hieth_get_link(struct net_device *net_dev)
{
	struct hieth_netdev_local *ld = netdev_priv(net_dev);

	return ld->link_stat;
}

static int hieth_get_settings(struct net_device *net_dev, struct ethtool_cmd *cmd)
{
	struct hieth_netdev_local *ld = netdev_priv(net_dev);
	
	if (ld->phy)
		return phy_ethtool_gset(ld->phy, cmd);

	return -EINVAL;
}

static int hieth_set_settings(struct net_device *net_dev, struct ethtool_cmd *cmd)
{
	struct hieth_netdev_local *ld = netdev_priv(net_dev);
	
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (ld->phy)
		return phy_ethtool_sset(ld->phy, cmd);

	return -EINVAL;
}

static struct ethtool_ops hieth_ethtools_ops = {
	.get_drvinfo	= hieth_get_drvinfo,
	.get_link	= hieth_get_link,
	.get_settings	= hieth_get_settings,
	.set_settings	= hieth_set_settings,
};

static int __init hieth_dev_probe_init(struct net_device *dev)
{
	struct hieth_netdev_local *ld = netdev_priv(dev);

	/* set netdev functions */
	dev->open	= hieth_net_open;
	dev->stop	= hieth_net_close;
	dev->tx_timeout	= hieth_net_timeout;
	dev->watchdog_timeo	= 3*HZ;
	dev->hard_start_xmit	= hieth_net_hard_start_xmit;
	dev->get_stats		= hieth_net_get_stats;
	dev->set_multicast_list	= hieth_net_set_multicast_list;
	dev->set_mac_address	= hieth_net_set_mac_address;
	dev->do_ioctl 		= &hieth_ioctl;
	dev->ethtool_ops	= &hieth_ethtools_ops;

	/* setup hardware info */
	hieth_hw_get_macaddress(ld, dev->dev_addr);
	if(!is_valid_ether_addr(dev->dev_addr)) {
		print_mac_address(KERN_WARNING "Invalid HW-MAC Address: ", dev->dev_addr, "\n");
		random_ether_addr(dev->dev_addr);
		print_mac_address(KERN_WARNING "Set Random MAC address: ", dev->dev_addr, "\n");
	}

	return 0;
}

static int __init hieth_dev_probe(struct device *device)
{
	int ret = -1;
	struct net_device *netdev = NULL;
	struct platform_device *platdev;
	struct hieth_netdev_local *ld;
	struct resource *iores, *irqres;
	struct phy_device *phydev = NULL;

	netdev = alloc_etherdev(sizeof(*ld));
	if(netdev ==NULL)
		return -ENOMEM;

	ld = netdev_priv(netdev);

	platdev = to_platform_device(device);
	iores = platform_get_resource(platdev, IORESOURCE_MEM, 0);
	irqres = platform_get_resource(platdev, IORESOURCE_IRQ, 0);
	if(!iores || !irqres) {
		hieth_error("get resource failed!");
		goto _error_exit;
	}

	ld->iobase = IO_ADDRESS(iores->start);
	ld->iobase_phys = iores->start;
	ld->irq = irqres->start;

	netdev->irq = ld->irq;

	SET_MODULE_OWNER(netdev);
	SET_NETDEV_DEV(netdev, device);
        dev_set_drvdata(device, netdev);

	hieth_dev_probe_init(netdev);

	/* Configures BEGIN */
	/* These shuold be set by MODULE_PARAMETERS */
	ld->depth.hw_xmitq = hieth_hwq_xmit_depth;
	ld->depth.hw_recvq = hieth_hwq_recv_depth;
	ld->depth.sw_xmit_pool = ld->depth.hw_xmitq+1;
	ld->depth.sw_recv_rbf = hieth_recv_rbf_size;

	ld->opt.xmit_force_copy	= 1;
	ld->opt.recv_force_copy	= 0;
	ld->opt.recv_mb_type	= hieth_recv_mb_type;

	ld->stat.cnt_free_limit = hieth_recv_pool_free_limit;

	//ld->phy_name = "0:01"; // FIXME
	ld->phy_name = "0:02";

	ld->hclk = 135*MHZ;

	/* END */

	init_timer(&ld->timer.netif_rx_busy);

	if(hieth_probe_noise) {
		printk(KERN_INFO " hw_xmitq_depth: %d\n", ld->depth.hw_xmitq);
		printk(KERN_INFO " hw_recvq_depth: %d\n", ld->depth.hw_recvq);
		printk(KERN_INFO "  sw_xmitq_pool: %d\n", ld->depth.sw_xmit_pool);
		printk(KERN_INFO "  sw_recvq_pool: %d\n", ld->depth.sw_recv_rbf);
		printk(KERN_INFO "free_feed_limit: %d\n", ld->stat.cnt_free_limit);
		printk(KERN_INFO "xmit_force_copy: %s\n", ld->opt.xmit_force_copy?"Yes":"No");
		printk(KERN_INFO "recv_force_copy: %s\n", ld->opt.recv_force_copy?"Yes":"No");
		printk(KERN_INFO "   recv_mb_type: %d\n", ld->opt.recv_mb_type);
	}

	phydev = phy_connect(netdev, ld->phy_name, hieth_adjust_link, 0, PHY_INTERFACE_MODE_MII);
	if(IS_ERR(phydev)) {
		hieth_error("connect to phy_device %s failed!", ld->phy_name);
		goto _error_exit;
	}
	ld->phy = phydev;

	ret = register_netdev(netdev);
	if(ret) {
		hieth_error("register_netdev %s failed!", netdev->name);
		goto _error_exit;
	}

	hieth_reset_dev(ld);
	hieth_sys_shutdown(ld->devid);

	return ret;

_error_exit:
	if(phydev && !IS_ERR(phydev))
		phy_disconnect(phydev);

	if(netdev)
		free_netdev(netdev);

	return ret;
}

static int __exit hieth_dev_remove(struct device *device)
{
	struct net_device *netdev = dev_get_drvdata(device);
	struct hieth_netdev_local *ld = netdev_priv(netdev);

	dev_set_drvdata(device, NULL);

	phy_disconnect(ld->phy);
	unregister_netdev(netdev);
	free_netdev(netdev);

	return 0;
}

static struct device_driver hieth_dev_driver ={
	.owner = THIS_MODULE,

	.name = HIETH_DEV_NAME,
	.bus = &platform_bus_type,
	.probe = hieth_dev_probe,
	.remove = hieth_dev_remove,
};

static int __init hieth_init(void)
{
	int ret = 0;

	printk(KERN_INFO OSDRV_MODULE_VERSION_STRING "\n");
	printk(KERN_INFO "Hisilicon ETHv100 net controler.\n");

	if(hieth_mm_init())
		return -1;

	hieth_sys_init();

	if(hieth_mdiobus_driver_init()) {
		hieth_sys_exit();
		hieth_mm_exit();

		return -1;
	}

	ret = driver_register(&hieth_dev_driver);
	if(ret) {
		hieth_error("register netdevice driver failed!");

		hieth_mdiobus_driver_exit();
		hieth_mm_exit();
		hieth_sys_exit();

		return -1;
	}

	hieth_wq = create_singlethread_workqueue("hieth_skber");
	if(hieth_wq ==NULL) {
		hieth_error("create_singlethread_workqueue(hiethv100_daemon) failed!");

		driver_unregister(&hieth_dev_driver);
		hieth_mdiobus_driver_exit();
		hieth_mm_exit();
		hieth_sys_exit();

		return -1;
	}

	return ret;
}

static void __exit hieth_exit(void)
{
	destroy_workqueue(hieth_wq);

	driver_unregister(&hieth_dev_driver);

	hieth_mdiobus_driver_exit();

	hieth_mm_exit();

	hieth_sys_exit();
}

module_init(hieth_init);
module_exit(hieth_exit);

MODULE_DESCRIPTION("Hisilicon ETHv100 driver whith MDIO support");
MODULE_AUTHOR("Jiandong Liu");
MODULE_LICENSE("GPL");

MODULE_VERSION("HI_VERSION=" OSDRV_MODULE_VERSION_STRING);

