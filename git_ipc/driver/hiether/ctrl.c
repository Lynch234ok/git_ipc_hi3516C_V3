
#define HIETHV100_INTER

#include "hiethv100.h"
#include "ctrl.h"
#include "mac.h"

void hieth_ctrl_reset(struct hieth_netdev_local *ld)
{
	int i;

	hieth_writel(ld, 0x00000000, GLB_IRQ_ENABLE);
	hieth_writel(ld, 0x000001ff, GLB_IRQ_MASK);
	hieth_writel(ld, 0x00000000, GLB_IRQ_STAT);

	hieth_writel(ld, 0x00000000, GLB_ENDIAN_MOD);
	hieth_writel(ld, 0x0000003f, GLB_MACFILT_BEHAVE);
	hieth_writel(ld, 0x00002020, GLB_QUEUE_DEPTH);
	hieth_writel(ld, 0x00001e1d, GLB_FC_LEVEL);

	for(i=0; i<128; i++)
		hieth_writel(ld, 0x00000000, GLB_RXOUTQ_FDRDY);

	hieth_writel(ld, 0x00000000, GLB_TXQUE_FDROP);
	hieth_writel(ld, 0x07ff86a0, GLB_FLUX_TIMECTRL);
	hieth_writel(ld, 0x00000000, GLB_FLUX_RXLIMIT);
	hieth_writel(ld, 0x00000000, GLB_FLUX_DROPCTRL);
}

static inline int _hieth_irq_enable(struct hieth_netdev_local *ld, int irqs)
{
	int old;

	old = hieth_readl_bits(ld, GLB_IRQ_MASK, BITS_IRQS); 

	hieth_writel_bits(ld, old | irqs, GLB_IRQ_MASK, BITS_IRQS); 

	return old;
}

static inline int _hieth_irq_disable(struct hieth_netdev_local *ld, int irqs)
{
	int old;

	old = hieth_readl_bits(ld, GLB_IRQ_MASK, BITS_IRQS); 

	hieth_writel_bits(ld, old & (~irqs), GLB_IRQ_MASK, BITS_IRQS); 

	return old;
}

static inline int _hieth_read_irqstatus(struct hieth_netdev_local *ld)
{
	int status;

	status = hieth_readl_bits(ld, GLB_IRQ_ENABLE, BITS_IRQS); 
	status = status & hieth_readl(ld, GLB_IRQ_MASK); 
	status = status & hieth_readl(ld, GLB_IRQ_STAT); 

	return status;
}

int hieth_hw_flowctrl(struct hieth_netdev_local *ld, int ena, int up, int down)
{
	int ret = 0;

	local_lock(ld);

	hw_flowctrl_ena(ld, ena);

	if(up <=0)
		up = hieth_readl_bits(ld, GLB_FC_LEVEL, BITS_FC_UP_LEVEL);
	if(down <=0)
		down = hieth_readl_bits(ld, GLB_FC_LEVEL, BITS_FC_DOWN_LEVEL);
	if( !(up>down) || (up>ld->depth.hw_recvq))
		ret = -1;

	hieth_writel_bits(ld, up, GLB_FC_LEVEL, BITS_FC_UP_LEVEL);
	hieth_writel_bits(ld, down, GLB_FC_LEVEL, BITS_FC_DOWN_LEVEL);

	local_unlock(ld);

	return ret;
}

int hieth_hw_set_macaddress(struct hieth_netdev_local *ld, int ena, unsigned char *mac)
{
	unsigned long reg;

	hw_localmac_filt_ctrl(ld, ena, 1);

	reg = mac[1] | (mac[0] <<8);
	hieth_writel(ld, reg, GLB_ETHADDR_H2);

	reg = mac[5] | (mac[4]<<8) | (mac[3]<<16) | (mac[2]<<24);
	hieth_writel(ld, reg, GLB_ETHADDR_L4);

	return 0;
}

int hieth_hw_get_macaddress(struct hieth_netdev_local *ld, unsigned char *mac)
{
	unsigned long reg;

	reg = hieth_readl(ld, GLB_ETHADDR_H2);
	mac[0] = (reg>>8) & 0xff;
	mac[1] = reg & 0xff;

	reg = hieth_readl(ld, GLB_ETHADDR_L4);
	mac[2] = (reg>>24) & 0xff;
	mac[3] = (reg>>16) & 0xff;
	mac[4] = (reg>>8) & 0xff;
	mac[5] = reg & 0xff;

	return 0;
}

static inline int _test_xmit_queue_ready(struct hieth_netdev_local *ld)
{
	return hieth_readl_bits(ld, GLB_RO_QUEUE_STAT, BITS_XMITQ_RDY);
}

static inline int _test_recv_queue_ready(struct hieth_netdev_local *ld)
{
	return hieth_readl_bits(ld, GLB_RO_QUEUE_STAT, BITS_RECVQ_RDY);
}


static void _queue_error_handler(struct hieth_netdev_local *ld)
{
	hieth_error("xmit/recv queue error, reset queue!");
	BUG();
}

int hieth_irq_enable(struct hieth_netdev_local *ld, int irqs)
{
	int old;

	local_lock(ld);
	old = _hieth_irq_enable(ld, irqs);
	local_unlock(ld);

	return old;
}

int hieth_irq_disable(struct hieth_netdev_local *ld, int irqs)
{
	int old;

	local_lock(ld);
	old = _hieth_irq_disable(ld, irqs);
	local_unlock(ld);

	return old;
}

int hieth_read_irqstatus(struct hieth_netdev_local *ld)
{
	int status;

	local_lock(ld);
	status = _hieth_read_irqstatus(ld);
	local_unlock(ld);

	return status;
}

int hieth_read_raw_irqstatus(struct hieth_netdev_local *ld)
{
	int status;

	local_lock(ld);
	status = hieth_readl_bits(ld, GLB_IRQ_STAT, BITS_IRQS); 
	local_unlock(ld);

	return status;
}

int hieth_clear_irqstatus(struct hieth_netdev_local *ld, int irqs)
{
	int status;

	local_lock(ld);
	hieth_writel_bits(ld, irqs, GLB_IRQ_STAT, BITS_IRQS);
	status = _hieth_read_irqstatus(ld);
	local_unlock(ld);

	return status;
}

int hieth_set_endian_mode(struct hieth_netdev_local *ld, int mode)
{
	int old;

	local_lock(ld);
	old = hieth_readl_bits(ld, GLB_ENDIAN_MOD, BITS_ENDIAN);
	hieth_writel_bits(ld, mode, GLB_ENDIAN_MOD, BITS_ENDIAN);
	local_unlock(ld);

	return old;
}

int hieth_set_hwq_depth(struct hieth_netdev_local *ld)
{
	hieth_assert( ld->depth.hw_xmitq>0 && ld->depth.hw_recvq>0 && 
			(ld->depth.hw_xmitq + ld->depth.hw_recvq) <= HIETH_MAX_QUEUE_DEPTH);

	local_lock(ld);

	if( (ld->depth.hw_xmitq + ld->depth.hw_recvq) > HIETH_MAX_QUEUE_DEPTH) {
		local_unlock(ld);
		return -1;
	}

	hieth_writel_bits(ld, ld->depth.hw_xmitq, GLB_QUEUE_DEPTH, BITS_TXQ_DEP);
	hieth_writel_bits(ld, ld->depth.hw_recvq, GLB_QUEUE_DEPTH, BITS_RXQ_DEP);

	hil_vqueue_init(&ld->hwq_xmit, ld->depth.hw_xmitq);
	hil_vqueue_init(&ld->hwq_recv, ld->depth.hw_recvq);

	hil_vqueue_init(&ld->swq_xmit_pool, ld->depth.sw_xmit_pool);

	hil_vqueue_init(&ld->swq_recv_ready, ld->depth.sw_recv_rbf);
	hil_vqueue_init(&ld->swq_recv_blank, ld->depth.sw_recv_rbf);
	hil_vqueue_init(&ld->swq_recv_free, ld->depth.sw_recv_rbf);

	local_unlock(ld);

	return 0;
}

int hieth_get_hwq_xmit_depth(struct hieth_netdev_local *ld)
{
	int depth;

	local_lock(ld);
	depth = hieth_readl_bits(ld, GLB_QUEUE_DEPTH, BITS_TXQ_DEP);
	local_unlock(ld);

	return depth;
}

int hieth_get_hwq_recv_depth(struct hieth_netdev_local *ld)
{
	int depth;

	local_lock(ld);
	depth = hieth_readl_bits(ld, GLB_QUEUE_DEPTH, BITS_RXQ_DEP);
	local_unlock(ld);

	return depth;
}

int hieth_hw_xmitq_ready(struct hieth_netdev_local *ld)
{
	int ret;

	local_lock(ld);
	ret = _test_xmit_queue_ready(ld);
	local_unlock(ld);

	return ret;
}

int hieth_xmit_add_tosend(struct hieth_netdev_local *ld, struct hieth_frame_desc *fd)
{
	int ret= 1;
	int hwid, vqid;

	hieth_assert(!HIETH_INVALID_TXQFD_ADDR(fd->frm_addr));

	local_lock(ld);

	if(!_test_xmit_queue_ready(ld))
		goto _trans_exit;

	vqid = hil_vqueue_push_back(&ld->hwq_xmit);
	if(vqid <0)
		goto _trans_exit;

	hw_xmitq_setfd(ld, *fd);
	hieth_fd_copy(HWQ_XMIT_FD(ld, vqid), *fd);
	hwid = hw_get_txqid(ld);

	if(hwid !=vqid) {
		hieth_error("Tx queue error, hwid=%d, vqid=%d", hwid, vqid);
		_queue_error_handler(ld);
		ret = -1;
		goto _trans_exit;
	}

	ret = 0;

_trans_exit:
	local_unlock(ld);
	return ret;
}

int hieth_xmit_pushfd_pool(struct hieth_netdev_local *ld, struct hieth_frame_desc *fd)
{
	int vqid;

	hieth_assert(!HIETH_INVALID_RXQFD_ADDR(fd->frm_addr));

	local_lock(ld);

	vqid = hil_vqueue_push_back(&ld->swq_xmit_pool);
	if(vqid <0) {
		local_unlock(ld);
		return HIETH_E_FULL;
	}

	hieth_fd_copy(SWQ_XMIT_POOL_FD(ld, vqid), *fd);

	local_unlock(ld);
	return 0;
}

int _xmit_popfd_pool(struct hieth_netdev_local *ld, struct hieth_frame_desc *fd)
{
	int vqid;

	vqid = hil_vqueue_pop_front(&ld->swq_xmit_pool);
	if(vqid <0) {
		return HIETH_E_EMPTY;
	}

	hieth_fd_copy(*fd, SWQ_XMIT_POOL_FD(ld, vqid));

	return 0;
}
int hieth_xmit_popfd_pool(struct hieth_netdev_local *ld, struct hieth_frame_desc *fd)
{
	int ret;

	local_lock(ld);
	ret = _xmit_popfd_pool(ld, fd);
	local_unlock(ld);

	return ret;
}

static int _xmit_queue_popfd(struct hieth_netdev_local *ld, struct hieth_frame_desc *outfd, int force)
{
	int vqcnt, hwcnt;

	vqcnt = hil_vqueue_count(&ld->hwq_xmit);
	hwcnt = hw_xmitq_cnt_inuse(ld);
	if(!force && vqcnt < hwcnt) 
		goto _queue_error;

	if(force || vqcnt > hwcnt) {
		int id = hil_vqueue_front(&ld->hwq_xmit);
		if(id <0) {
			if(force)
				return -1;
			goto _queue_error;
		}

		hieth_fd_copy(*outfd, HWQ_XMIT_FD(ld, id));
		hil_vqueue_pop_front(&ld->hwq_xmit);
		return 0;
	}

	return -1;

_queue_error:
	hieth_error("vqcnt=%d < hwcnt=%d", vqcnt, hwcnt);
	_queue_error_handler(ld);

	return -1;
}

int hieth_xmit_popfd_sendok(struct hieth_netdev_local *ld, struct hieth_frame_desc *outfd)
{
	int ret;

	local_lock(ld);
	ret = _xmit_queue_popfd(ld, outfd, 0);
	local_unlock(ld);

	return ret;
}

int hieth_xmit_popfd_all_force(struct hieth_netdev_local *ld, struct hieth_frame_desc *outfd)
{
	int ret;

	local_lock(ld);
	ret = _xmit_popfd_pool(ld, outfd);
	if(ret <0) {
		ret = _xmit_queue_popfd(ld, outfd, 1);
	}
	local_unlock(ld);

	return ret;
}

int hieth_hw_recvq_ready(struct hieth_netdev_local *ld)
{
	int ret;

	local_lock(ld);
	ret = _test_recv_queue_ready(ld);
	local_unlock(ld);

	return ret;
}

int hieth_recv_packet_ready(struct hieth_netdev_local *ld)
{
	int ret;

	local_lock(ld);
	ret = is_recv_packet(ld);
	local_unlock(ld);

	return ret;
}

static int _recv_hwq_fill(struct hieth_netdev_local *ld)
{
	int count = 0;

	while(_test_recv_queue_ready(ld)) {
		int freeid, vqid, hwid;

		DUMP_VQUEUE_RBF(6, ld);

		freeid = hil_vqueue_front(&ld->swq_recv_free);
		if(freeid <0) {
			hieth_trace(5, "swq_recv_free empty, swq_recv_ready=%d, swq_recv_blank=%d, hwq_recv=%d",
					hil_vqueue_count(&ld->swq_recv_ready), 
					hil_vqueue_count(&ld->swq_recv_blank),
					hil_vqueue_count(&ld->hwq_recv));
			break;
		}
		hwid = hw_get_rxqid(ld);
		vqid = hil_vqueue_back(&ld->hwq_recv);
		if(vqid>=0 && hwid != vqid) {
			hieth_error("check last queue status failed, hwid=%d, vqid=%d", hwid, vqid);
			goto _queue_error;
		}

		vqid = hil_vqueue_push_back(&ld->hwq_recv);
		if(vqid <0) {
			hieth_error("hardware queue ready, software queue busy?");
			goto _queue_error;
		}
		hieth_fd_copy(HWQ_RECV_FD(ld, vqid), SWQ_RECV_FD(ld, freeid));
		hw_recvq_setfd(ld, HWQ_RECV_FD(ld, vqid));

		vqid = hil_vqueue_pop_front(&ld->swq_recv_free);
		hieth_assert(vqid ==freeid);

		count ++;

		ASSERT_READY_FREE_BLANK_QUEUE(ld);
		DUMP_VQUEUE_RBF(6, ld);
	}

	return count;

_queue_error:
	_queue_error_handler(ld);

	return -1;
}

int hieth_recv_hwq_fill(struct hieth_netdev_local *ld)
{
	int ret;

	local_lock(ld);
	ret = _recv_hwq_fill(ld);
	local_unlock(ld);

	return ret;
}

int hieth_hw_recv_tryup(struct hieth_netdev_local *ld)
{
	local_lock(ld);

	while(is_recv_packet(ld)) {
		int vqid, hwid, rlen;
		struct hieth_frame_desc *hwfd;

		hwid = hw_get_rxpkg_id(ld);
		rlen = hw_get_rxpkg_len(ld);
		hw_set_rxpkg_finish(ld);

		DUMP_VQUEUE_RBF(6, ld);

		/* prepare for recv frame from hardware */
		vqid = hil_vqueue_front(&ld->hwq_recv);
		if(vqid <0) {
			hieth_error("hardware packet ready, buf software queue empty?");
			goto _queue_error;
		}

		if(vqid !=hwid) {
			hieth_error("hwid=%d, vqid=%d", hwid, vqid);
			goto _queue_error;
		}

		hwfd = &HWQ_RECV_FD(ld, vqid);
		hwfd->frm_len = rlen;

		/* put frame to ready-queue */
		vqid = hil_vqueue_push_back(&ld->swq_recv_ready);
		if(vqid <0) {
			hieth_error("swq_recv_ready full?");
			goto _queue_error;
		}
		hieth_fd_copy(SWQ_RECV_FD(ld, vqid), *hwfd);

		vqid = hil_vqueue_pop_front(&ld->hwq_recv);
		hieth_assert(vqid ==hwid);

		ASSERT_READY_FREE_BLANK_QUEUE(ld);
		DUMP_VQUEUE_RBF(6, ld);
	}

	/* fill hardware receive queue again */
	if(_recv_hwq_fill(ld)<0)
		goto _error_exit;

	local_unlock(ld);

	return hil_vqueue_count(&ld->swq_recv_ready);

_queue_error:
	_queue_error_handler(ld);
_error_exit:
	local_unlock(ld);

	return -1;
}

struct hieth_frame_desc *hieth_recv_getfd(struct hieth_netdev_local *ld)
{
	int vqid; 
	struct hieth_frame_desc *curfd =NULL;

	local_lock(ld);
	vqid = hil_vqueue_front(&ld->swq_recv_ready);
	if(vqid >=0)
		curfd = &SWQ_RECV_FD(ld, vqid);
	local_unlock(ld);

	return curfd;
}

int _recv_next(struct hieth_netdev_local *ld, struct hieth_frame_desc *curfd, int act)
{
	struct hieth_frame_desc fd_bak;
	struct hieth_frame_desc *cmpfd;
	int rdyid, freeid;
	int tmp;

	if(curfd ==NULL)
		return -1;

	/* starting restore fd */
	DUMP_VQUEUE_RBF(6, ld);
	
	/* get current id */
	rdyid = hil_vqueue_front(&ld->swq_recv_ready);
	if(rdyid <0)
		goto _queue_error;

	/* check if curfd is right */
	cmpfd = &SWQ_RECV_FD(ld, rdyid);
	if(cmpfd != curfd) {
		hieth_error("curfd=%p, ld->swq_recv_ready[%d]=%p", curfd, rdyid, cmpfd);
		goto _queue_error;
	}

	/* increase blank queue */
	if(hil_vqueue_push_back(&ld->swq_recv_blank) <0)
		goto _queue_error;

	/* check if drop this fd */
	if(act ==0) {
	        tmp = hil_vqueue_pop_front(&ld->swq_recv_ready);
		hieth_assert(tmp ==rdyid);

		goto _end_restore_fd;
	}

	/* decreate blank queue */
	if(hil_vqueue_pop_front(&ld->swq_recv_blank) <0)
		goto _queue_error;

	/* add to free queue */
	freeid = hil_vqueue_push_back(&ld->swq_recv_free);
	if(freeid <0)
		goto _queue_error;
	curfd = NULL;

        tmp = hil_vqueue_pop_front(&ld->swq_recv_ready);
	hieth_assert(tmp ==rdyid);

	/* check if need to copy fd */
	if(freeid !=rdyid)
		hieth_fd_copy(SWQ_RECV_FD(ld, freeid), SWQ_RECV_FD(ld, rdyid));

	hieth_assert(hil_vqueue_count(&ld->swq_recv_blank)==0 ? freeid ==rdyid : freeid !=rdyid);

_end_restore_fd:
	ASSERT_READY_FREE_BLANK_QUEUE(ld);
	DUMP_VQUEUE_RBF(6, ld);

	return 0;

_queue_error:
	if(act && curfd)
		hieth_fd_copy(fd_bak, *curfd);
	_queue_error_handler(ld);
	if(act && curfd) {
		freeid = hil_vqueue_push_back(&ld->swq_recv_free);
		if(freeid <0) {
			hieth_error("I don't known what hanppened, amazing!");
			return -1;
		}
		hieth_fd_copy(SWQ_RECV_FD(ld, freeid), fd_bak);
	}

	return 0;
}

int hieth_recv_next_reusefd(struct hieth_netdev_local *ld, struct hieth_frame_desc *curfd)
{
	int ret;

	local_lock(ld);
	ret = _recv_next(ld, curfd, 1);
	local_unlock(ld);

	return ret;
}

int hieth_recv_next_dropfd(struct hieth_netdev_local *ld, struct hieth_frame_desc *curfd)
{
	int ret;

	local_lock(ld);
	ret = _recv_next(ld, curfd, 0);
	local_unlock(ld);

	return ret;
}

static int _calc_recvq_blank(struct hieth_netdev_local *ld)
{
	int res = ld->depth.sw_recv_rbf;

	res -= hil_vqueue_count(&ld->swq_recv_ready);
	res -= hil_vqueue_count(&ld->swq_recv_free);
	res -= hil_vqueue_count(&ld->hwq_recv);

	return res;
}

int hieth_recvq_blank_count(struct hieth_netdev_local *ld)
{
	int res;

	local_lock(ld);
	res = _calc_recvq_blank(ld);
	local_unlock(ld);

	return res;
}

int hieth_recv_add_freefd(struct hieth_netdev_local *ld, struct hieth_frame_desc *freefd)
{
	int vqid;

	local_lock(ld);

	if(_calc_recvq_blank(ld) <= 0) {
		local_unlock(ld);
		return -1;
	}

	if(hil_vqueue_count(&ld->swq_recv_blank) ==0) {
		vqid = hil_vqueue_push_front(&ld->swq_recv_free);
	} else {
		vqid = hil_vqueue_pop_front(&ld->swq_recv_blank);
		hieth_assert(vqid >=0);

		vqid = hil_vqueue_push_back(&ld->swq_recv_free);
		hieth_assert(vqid >=0);
	}

	hieth_assert(vqid >=0);

	hieth_fd_copy(SWQ_RECV_FD(ld, vqid), *freefd);

	ASSERT_READY_FREE_BLANK_QUEUE(ld);

	local_unlock(ld);

	return 0;
}

int hieth_recv_popfd_free(struct hieth_netdev_local *ld, struct hieth_frame_desc *outfd)
{
	int vqid;

	local_lock(ld);

	vqid = hil_vqueue_pop_front(&ld->swq_recv_free);
	if(vqid <0) {
		local_unlock(ld);
		return -1;
	}

	hieth_fd_copy(*outfd, SWQ_RECV_FD(ld, vqid));

	if(hil_vqueue_push_back(&ld->swq_recv_blank) <0) {
		_queue_error_handler(ld);
		local_unlock(ld);
		return -1;
	}

	ASSERT_READY_FREE_BLANK_QUEUE(ld);

	local_unlock(ld);

	return 0;
}

int hieth_recv_popfd_all_force(struct hieth_netdev_local *ld, struct hieth_frame_desc *outfd)
{
	int vqid;

	local_lock(ld);

	vqid = hil_vqueue_pop_front(&ld->swq_recv_free);
	if(vqid<0) {
		vqid = hil_vqueue_pop_front(&ld->swq_recv_ready);
		if(vqid >=0)
			hil_vqueue_push_back(&ld->swq_recv_blank);
	} 
	if(vqid >=0)
		hieth_fd_copy(*outfd, SWQ_RECV_FD(ld, vqid));

	if(vqid<0) {
		vqid = hil_vqueue_pop_front(&ld->hwq_recv);
		if(vqid >=0)
			hieth_fd_copy(*outfd, HWQ_RECV_FD(ld, vqid));
	}

	ASSERT_READY_FREE_BLANK_QUEUE(ld);

	local_unlock(ld);

	return vqid<0 ? -1 : 0;
}

