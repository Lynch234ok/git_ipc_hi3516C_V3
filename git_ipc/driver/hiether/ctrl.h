
#ifndef __HISILICON_HIETH_CTRL_H
#define __HISILICON_HIETH_CTRL_H

#ifdef HIETHV100_INTER

#define GLB_IRQ_STAT	0x0200
#define GLB_IRQ_ENABLE	0x0204
#define GLB_IRQ_MASK	0x0208
#define GLB_ENDIAN_MOD	0x0210
#define GLB_MACFILT_BEHAVE	0x0214
#define GLB_QUEUE_DEPTH	0x0218
#define GLB_FC_LEVEL	0x021C
#define GLB_TXINQ_ADDR	0x0220
#define GLB_TXINQ_LEN	0x0224
#define GLB_RO_QUEUE_ID	0x022C
#define GLB_RO_RXOUTQ_FD	0x0230
#define GLB_RXOUTQ_FDRDY	0x0234
#define GLB_RXINQ_ADDR	0x0238
#define GLB_ETHADDR_L4	0x023C
#define GLB_ETHADDR_H2	0x0240
#define GLB_TXQUE_FDROP	0x0244
#define GLB_RO_QUEUE_STAT	0x0248
#define GLB_FLUX_TIMECTRL	0x024C
#define GLB_FLUX_RXLIMIT	0x0250
#define GLB_FLUX_DROPCTRL	0x0254
#define GLB_MACFILT_0_L4	0x0258
#define GLB_MACFILT_0_H2	0x025C
#define GLB_MACFILT_1_L4	0x0260
#define GLB_MACFILT_1_H2	0x0264

#define BITS_ENDIAN	MK_BITS(0, 2)

/* IRQs mask bits */
#define BITS_IRQS	MK_BITS(0, 9)

/* Tx/Rx queue depth */
#define BITS_TXQ_DEP	MK_BITS(0, 6)
#define BITS_RXQ_DEP	MK_BITS(8, 6)

/* bits of GLB_RO_QUEUE_STAT */
/* check this bit to see if we can add a Tx package */
#define BITS_XMITQ_RDY	MK_BITS(24, 1)
/* check this bit to see if we can add a Rx addr */
#define BITS_RECVQ_RDY	MK_BITS(25, 1)

/* counts in queue, include currently sending */
#define BITS_XMITQ_CNT_INUSE	MK_BITS(0, 6)	
/* counts in queue, include currently sending */
#define BITS_RECVQ_CNT_RXOK	MK_BITS(8, 6)

/* bits of GLB_TXQFD_LEN */
#define BITS_TXINQ_LEN	MK_BITS(0, 11)

/* bits of GLB_RO_QUEUE_ID */
#define BITS_TXINQ_ID	MK_BITS(8, 6)
#define BITS_TXOUTQ_ID	MK_BITS(0, 6)
#define BITS_RXINQ_ID	MK_BITS(16, 6)

/* bits of GLB_RXQFD_RDY */
#define BITS_RXOUTQ_FDRDY	MK_BITS(0, 1)

/* bits of GLB_FLUX_TIMECTRL */
#define BITS_FLUX_TIME	MK_BITS(17, 10)
#define BITS_FLUX_TIMER_SCALE	MK_BITS(0, 17)

/* bits of GLB_FC_LEVEL */
#define BITS_FC_UP_LEVEL	MK_BITS(8, 6)
#define BITS_FC_DOWN_LEVEL	MK_BITS(0, 6)

/* bits of GLB_RXOUTQ_FDRDY */
#define BITS_RXPKG_RDY	MK_BITS(0, 1)
#define is_recv_packet(ld) hieth_readl_bits(ld, GLB_RXOUTQ_FDRDY, BITS_RXPKG_RDY)
#define hw_set_rxpkg_finish(ld) hieth_writel_bits(ld, 1, GLB_RXOUTQ_FDRDY, BITS_RXPKG_RDY)

/* bits of GLB_RO_RXOUTQ_FD */
#define BITS_RXPKG_ID	MK_BITS(16, 6)
#define BITS_RXPKG_LEN	MK_BITS(0, 11)
#define hw_get_rxpkg_id(ld) hieth_readl_bits(ld, GLB_RO_RXOUTQ_FD, BITS_RXPKG_ID)
#define hw_get_rxpkg_len(ld) hieth_readl_bits(ld, GLB_RO_RXOUTQ_FD, BITS_RXPKG_LEN)

#define hw_get_txqid(ld) hieth_readl_bits(ld, GLB_RO_QUEUE_ID, BITS_TXINQ_ID)
#define hw_get_rxqid(ld) hieth_readl_bits(ld, GLB_RO_QUEUE_ID, BITS_RXINQ_ID)

#define hw_xmitq_cnt_inuse(ld) hieth_readl_bits(ld, GLB_RO_QUEUE_STAT, BITS_XMITQ_CNT_INUSE)
#define hw_recvq_cnt_rxok(ld) hieth_readl_bits(ld, GLB_RO_QUEUE_STAT, BITS_RECVQ_CNT_RXOK)

#define hw_xmitq_setfd(ld, fd) do{ \
			hieth_writel(ld, (fd).frm_addr, GLB_TXINQ_ADDR); \
			hieth_writel_bits(ld, (fd).frm_len, GLB_TXINQ_LEN, BITS_TXINQ_LEN); \
		}while(0)

#define hw_recvq_setfd(ld, fd) hieth_writel(ld, (fd).frm_addr, GLB_RXINQ_ADDR)


#define HWQ_XMIT_FD(ld, id) (ld)->hwq_fd[id]

#define SWQ_XMIT_POOL_FD(ld, id) (ld)->swq_xmit_poolfd[id]

#define HWQ_RECV_FD(ld, id) (ld)->hwq_fd[ hil_vqueue_max_size(&(ld)->hwq_xmit) +(id)]

#define SWQ_RECV_FD(ld, id) (ld)->swq_recv_fd[id]


#define ASSERT_READY_FREE_BLANK_QUEUE(ld) do { \
		hieth_assert(!hil_vqueue_iscut(&ld->swq_recv_blank, &ld->swq_recv_ready)); \
		hieth_assert(!hil_vqueue_iscut(&ld->swq_recv_blank, &ld->swq_recv_free)); \
		hieth_assert(!hil_vqueue_iscut(&ld->swq_recv_ready, &ld->swq_recv_free)); } while(0)

/* bits of GLB_MACFILT_BEHAVE	*/
#define BITS_LOCALMAC_FILT_ENA	MK_BITS(8, 1)
#define BITS_MAC0_FILT_ENA	MK_BITS(7, 1)
#define BITS_MAC1_FILT_ENA	MK_BITS(6, 1)

#define BITS_LOCALMAC_FILT_CTRL	MK_BITS(5, 1)

#define hw_localmac_filt_ctrl(ld, ena, recv) do { \
		hieth_writel_bits(ld, (ena)!=0, GLB_MACFILT_BEHAVE, BITS_LOCALMAC_FILT_ENA); \
		hieth_writel_bits(ld, (recv)!=0, GLB_MACFILT_BEHAVE, BITS_LOCALMAC_FILT_CTRL); \
	} while(0)

#endif

#define DUMP_VQUEUE_RBF(level, ld) do { \
		hieth_trace(level, "DUMP_VQUEUE_RBF(" #level ", " #ld ")"); \
		hil_vqueue_dump(level, &(ld)->swq_recv_ready); \
		hil_vqueue_dump(level, &(ld)->swq_recv_blank); \
		hil_vqueue_dump(level, &(ld)->swq_recv_free); } while(0)

#define HIETH_INT_RX_RDY	(1<<0)
#define HIETH_INT_TX_FIN	(1<<1)
#define HIETH_INT_LINK_CH	(1<<2)
#define HIETH_INT_SPEED_CH	(1<<3)
#define HIETH_INT_DUPLEX_CH	(1<<4)
#define HIETH_INT_MDIO_FIN	(1<<5)
#define HIETH_INT_BUSERROR	(1<<6)
#define HIETH_INT_STATE_CH	(1<<7)
#define HIETH_INT_TXQUE_RDY	(1<<8)

#define HIETH_BIG_ENDIAN	0
#define HIETH_LITTLE_ENDIAN	3

/* for each bits, set '1' enable the intterrupt, and '0' takes no effects */
int hieth_irq_enable(struct hieth_netdev_local *ld, int irqs);	/* return last irq_enable status */
int hieth_irq_disable(struct hieth_netdev_local *ld, int irqs);	/* return last irq_enable status */
int hieth_read_irqstatus(struct hieth_netdev_local *ld);	/* return irqstatus */
int hieth_read_raw_irqstatus(struct hieth_netdev_local *ld);
int hieth_clear_irqstatus(struct hieth_netdev_local *ld, int irqs);	/* return irqstatus after clean */

int hieth_set_endian_mode(struct hieth_netdev_local *ld, int mode);

/* Tx/Rx queue operation */
int hieth_set_hwq_depth(struct hieth_netdev_local *ld);
int hieth_get_hwq_xmit_depth(struct hieth_netdev_local *ld);
int hieth_get_hwq_recv_depth(struct hieth_netdev_local *ld);

#define HIETH_INVALID_TXQFD_ADDR(addr) ((addr)&0x3)
#define HIETH_INVALID_RXQFD_ADDR(addr) ((addr)&0x3)
#define HIETH_INVALID_RXPKG_LEN(len) (!((len)>=42 && (len)<=HIETH_MAX_FRAME_SIZE))

/* test if I can add a fd to the Tx queue */
int hieth_hw_xmitq_ready(struct hieth_netdev_local *ld);
/* return 0 if success, return non-zero if failed */
int hieth_xmit_add_tosend(struct hieth_netdev_local *ld, struct hieth_frame_desc *fd);
/* return 0 if success, otherwise return non-zero */
int hieth_xmit_popfd_sendok(struct hieth_netdev_local *ld, struct hieth_frame_desc *outfd);
/* return 0 if success, otherwise return non-zero */
int hieth_xmit_popfd_all_force(struct hieth_netdev_local *ld, struct hieth_frame_desc *outfd);

/* return 0 if success, return non-zero if failed */
int hieth_xmit_pushfd_pool(struct hieth_netdev_local *ld, struct hieth_frame_desc *fd);
/* return 0 if success, return non-zero if failed */
int hieth_xmit_popfd_pool(struct hieth_netdev_local *ld, struct hieth_frame_desc *fd);

/* test if I can add a fd to the Rx queue */
int hieth_hw_recvq_ready(struct hieth_netdev_local *ld);
/* test if there is packet to recv */
int hieth_recv_packet_ready(struct hieth_netdev_local *ld);

int hieth_recv_hwq_fill(struct hieth_netdev_local *ld);
int hieth_hw_recv_tryup(struct hieth_netdev_local *ld);
struct hieth_frame_desc *hieth_recv_getfd(struct hieth_netdev_local *ld);
int hieth_recv_next_reusefd(struct hieth_netdev_local *ld, struct hieth_frame_desc *curfd);
int hieth_recv_next_dropfd(struct hieth_netdev_local *ld, struct hieth_frame_desc *curfd);
int hieth_recv_add_freefd(struct hieth_netdev_local *ld, struct hieth_frame_desc *freefd);
int hieth_recv_popfd_free(struct hieth_netdev_local *ld, struct hieth_frame_desc *outfd);
int hieth_recv_popfd_all_force(struct hieth_netdev_local *ld, struct hieth_frame_desc *outfd);

int hieth_recvq_blank_count(struct hieth_netdev_local *ld);
#define hieth_recvq_free_count(ld) hil_vqueue_count(&(ld)->swq_recv_free)
#define hieth_recvq_ready_count(ld) hil_vqueue_count(&(ld)->swq_recv_ready)

void hieth_ctrl_reset(struct hieth_netdev_local *ld);

int hieth_hw_flowctrl(struct hieth_netdev_local *ld, int ena, int up, int down);

int hieth_hw_set_macaddress(struct hieth_netdev_local *ld, int ena, unsigned char *mac);
int hieth_hw_get_macaddress(struct hieth_netdev_local *ld, unsigned char *mac);

#endif

