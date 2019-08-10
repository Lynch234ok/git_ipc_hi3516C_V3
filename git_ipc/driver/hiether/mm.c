
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
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/workqueue.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include "hiethv100.h"
#include "mdio.h"
#include "mac.h"
#include "ctrl.h"
#include "glb.h"

static kmem_cache_t *hieth_frame_cache;

#if 0
/**	dma_corherent
  *	mode=1, to create xmit queue
  *	mode=2, to create recv queue
  */
static int hieth_create_hwq_dma_corherent(struct net_device *dev, int blk_pages, int blk_nr, int mode)
{
	int k;
	int blk_size = PAGE_SIZE*blk_pages;
	struct hieth_netdev_local *ld = netdev_priv(dev);

	for(k=0; k<blk_nr; k++) {
		int i;
		char *mb_virt;
		dma_addr_t phys_addr;
		int nr_frames = blk_size/HIETH_MAX_FRAME_SIZE;

		mb_virt = dma_alloc_coherent(NULL, blk_size, &phys_addr, GFP_DMA);
		if(mb_virt ==NULL) {
			hieth_error("dma_alloc_coherent(%d) failed!", blk_size);
			return -ENOMEM;
		}

		for(i=0; i<nr_frames; i++) {
			struct hieth_frame_desc fd;
			int ofs = i*HIETH_MAX_FRAME_SIZE;

			HIETH_FD_INIT(fd);

			fd.frm_addr = phys_addr + ofs;

			fd.priv_data = mb_virt + ofs;
			fd.mb_type = MBTYPE_DMA_COHERENT;
			fd.mb_cached = 0;
			if(i ==0)
				fd.mb_pages = blk_pages;

			if(mode ==1) {
				if(hieth_xmit_queue_pushfd_rc(ld, &fd))
					break;
			} else if(mode ==2) {
				if(hieth_recv_queue_pushfd(ld, &fd) !=0 && 
						hieth_recv_queue_pushfd_rc(ld, &fd) !=0)
					break;
			} else {
				hieth_trace(3, "unsupport mode=%d", mode);
				break;
			}

			hieth_trace_fd(5, fd);
		}

		if(i ==0)
			dma_free_coherent(NULL, blk_size, mb_virt, phys_addr);
		if(i != nr_frames)
			break;
	}

	return 0;
}

/**	kmalloc
  *	mode=1, to create xmit queue
  *	mode=2, to create recv queue
  */
static int hieth_create_hwq_kmalloc(struct net_device *dev, int blk_pages, int blk_nr, int mode)
{
	int k;
	int blk_size = PAGE_SIZE*blk_pages;
	struct hieth_netdev_local *ld = netdev_priv(dev);

	for(k=0; k<blk_nr; k++) {
		int i;
		char *mb_virt;
		dma_addr_t phys_addr;
		int nr_frames = blk_size/HIETH_MAX_FRAME_SIZE;

		mb_virt = kmalloc(blk_size, GFP_KERNEL);
		if(mb_virt ==NULL) {
			hieth_error("kmalloc(%d) failed!", blk_size);
			return -ENOMEM;
		}
		phys_addr = virt_to_phys(mb_virt);

		for(i=0; i<nr_frames; i++) {
			struct hieth_frame_desc fd;
			int ofs = i*HIETH_MAX_FRAME_SIZE;

			HIETH_FD_INIT(fd);

			fd.frm_addr = phys_addr + ofs;

			fd.priv_data = mb_virt + ofs;
			fd.mb_type = MBTYPE_KMALLOC;
			fd.mb_cached = 1;
			if(i ==0)
				fd.mb_pages = blk_pages;

			if(mode ==1) {
				if(hieth_xmit_queue_pushfd_rc(ld, &fd))
					break;
			} else if(mode ==2) {
				if(hieth_recv_queue_pushfd(ld, &fd) !=0 && 
						hieth_recv_queue_pushfd_rc(ld, &fd) !=0)
					break;
				consistent_sync(fd.priv_data, HIETH_MAX_FRAME_SIZE, DMA_FROM_DEVICE);
			} else {
				hieth_trace(3, "unsupport mode=%d", mode);
				break;
			}

			hieth_trace_fd(5, fd);
		}

		if(i ==0)
			kfree(mb_virt);
		if(i != nr_frames)
			break;
	}

	return 0;
}
#endif

/**	kmem_cache
  *	mode=1, to create xmit queue
  *	mode=2, to create recv queue
  */
static int hieth_create_hwq_kmem_cache(struct net_device *dev, int nr, int mode)
{
	int i;
	struct hieth_netdev_local *ld = netdev_priv(dev);

	for(i=0; i<nr; i++) {
		char *mb_virt;
		dma_addr_t phys_addr;
		struct hieth_frame_desc fd;

		mb_virt = kmem_cache_alloc(hieth_frame_cache, SLAB_KERNEL);
		if(mb_virt ==NULL) {
			hieth_error("kmem_cache_alloc failed!");
			return -ENOMEM;
		}
		phys_addr = virt_to_phys(mb_virt);

		HIETH_FD_INIT(fd);

		fd.frm_addr = phys_addr;

		fd.priv_data = mb_virt;
		fd.mb_type = MBTYPE_KMEM_CACHE;
		fd.mb_cached = 1;

		if(mode ==1) { /* for the xmit queue */
			if(hieth_xmit_pushfd_pool(ld, &fd))
				break;
		} else if(mode ==2) { /* for the recv queue */
			if(hieth_recv_add_freefd(ld, &fd) !=0 )
				break;
			consistent_sync(fd.priv_data, HIETH_MAX_FRAME_SIZE, DMA_FROM_DEVICE);
		} else {
			hieth_trace(3, "unsupport mode=%d", mode);
			break;
		}

		hieth_trace_fd(5, fd);
	}

	return 0;
}

/**	skbbuff
  */
int hieth_recvq_skb_feed(struct net_device *dev, int nr, unsigned long prio)
{
	struct hieth_netdev_local *ld = netdev_priv(dev);

	for(; nr!=0; nr--) {
		struct sk_buff *skb;
		dma_addr_t phys_addr;
		struct hieth_frame_desc fd;

		skb = alloc_skb(HIETH_MAX_FRAME_SIZE+2, prio);
		if(skb ==NULL) {
			hieth_error("alloc_skb failed!");
			return -ENOMEM;
		}

		phys_addr = virt_to_phys(skb->data);

		HIETH_FD_INIT(fd);

		fd.frm_addr = phys_addr;

		fd.priv_data = skb;
		fd.mb_type = MBTYPE_SKBBUF;
		fd.mb_cached = 1;

		if(hieth_recv_add_freefd(ld, &fd) !=0 ) {
			kfree_skb(skb);
			break;
		}
		consistent_sync(skb->data, HIETH_MAX_FRAME_SIZE, DMA_FROM_DEVICE);

		hieth_trace_fd(5, fd);
	}

	return nr;
}

int hieth_create_hwq_xmit(struct net_device *dev, int blk_size, int blk_nr, int mb_type)
{
	int ret = 0;

	switch(mb_type) {
#if 0
		case MBTYPE_DMA_COHERENT:
			ret = hieth_create_hwq_dma_corherent(dev, blk_size, blk_nr, 1);
			break;
		case MBTYPE_KMALLOC:
			ret = hieth_create_hwq_kmalloc(dev, blk_size, blk_nr, 1);
			break;
#endif
		case MBTYPE_KMEM_CACHE:
			ret = hieth_create_hwq_kmem_cache(dev, blk_nr, 1);
			break;
		default:
			hieth_trace(5, "unsupport mb_type=%d", mb_type);
			break;
	}

	return ret;
}

int hieth_create_hwq_recv(struct net_device *dev, int blk_size, int blk_nr, int mb_type)
{
	int ret = 0;

	switch(mb_type) {
#if 0
		case MBTYPE_DMA_COHERENT:
			ret = hieth_create_hwq_dma_corherent(dev, blk_size, blk_nr, 2);
			break;
		case MBTYPE_KMALLOC:
			ret = hieth_create_hwq_kmalloc(dev, blk_size, blk_nr, 2);
			break;
#endif
		case MBTYPE_KMEM_CACHE:
			printk(KERN_INFO "receive queue setup with %d kmem_cache buffers\n", blk_nr);
			ret = hieth_create_hwq_kmem_cache(dev, blk_nr, 2);
			break;
		case MBTYPE_SKBBUF:
			printk(KERN_INFO "delayed to setup %d skb queue items.\n", blk_nr);
			break;
		default:
			hieth_trace(5, "unsupport mb_type=%d", mb_type);
			break;
	}

	return ret;
}

int hieth_queue_setup(struct net_device *dev)
{
	struct hieth_netdev_local *ld = netdev_priv(dev);

	/* alloc fd buffer */
	ld->swq_recv_fd = vmalloc(sizeof(struct hieth_frame_desc) *
					(ld->depth.sw_recv_rbf+ld->depth.sw_xmit_pool));
	ld->swq_xmit_poolfd = ld->swq_recv_fd + ld->depth.sw_recv_rbf;

	switch(ld->opt.recv_mb_type) {
		case MBTYPE_KMEM_CACHE:
			hieth_create_hwq_recv(dev, 0, ld->depth.sw_recv_rbf, MBTYPE_KMEM_CACHE);
			break;
		case MBTYPE_SKBBUF:
			hieth_create_hwq_recv(dev, 0, ld->depth.sw_recv_rbf, MBTYPE_SKBBUF);
			break;
		default:
			hieth_trace(5, "unsupport ld->opt.recv_mb_type=%d", ld->opt.recv_mb_type);
			break;
	}

	hieth_trace(5, "***********************************");
	hieth_create_hwq_xmit(dev, 0, ld->depth.sw_xmit_pool, MBTYPE_KMEM_CACHE);

	return 0;
}

int hieth_queue_cleanup(struct net_device *dev)
{
	struct hieth_netdev_local *ld = netdev_priv(dev);

	while(1) {
		struct hieth_frame_desc fd;

		if( hieth_xmit_popfd_all_force(ld, &fd) &&
			hieth_recv_popfd_all_force(ld, &fd) )
			break;

		hieth_trace_fd(5, fd);

		switch(fd.mb_type) {
			case MBTYPE_DMA_COHERENT:
				if(fd.mb_pages)
					dma_free_coherent(NULL, PAGE_SIZE*fd.mb_pages, fd.priv_data, fd.frm_addr);
				break;
			case MBTYPE_KMALLOC:
				if(fd.mb_pages)
					kfree(fd.priv_data);
				break;
			case MBTYPE_KMEM_CACHE:
				kmem_cache_free(hieth_frame_cache, fd.priv_data);
				break;
			case MBTYPE_SKBBUF:
				kfree_skb((struct sk_buff *)fd.priv_data);
				break;
			default:
				hieth_trace(5, "unsupport fd.mb_type=%d", fd.mb_type);
				break;
		}
	}

	vfree(ld->swq_recv_fd);

	return 0;
}

int hieth_mm_init(void)
{
	hieth_frame_cache = kmem_cache_create("hiethv100_frame_cache", 
					HIETH_MAX_FRAME_SIZE, 0,
					SLAB_HWCACHE_ALIGN, NULL, NULL);
	if(hieth_frame_cache ==NULL) {
		hieth_error("create hieth_frame_cache failed!");
		return -1;
	}

	return 0;
}

void hieth_mm_exit(void)
{
	if(kmem_cache_destroy(hieth_frame_cache))
		hieth_error("attention, destroy hieth_frame_cache failed!");
}

