
#define HIETHV100_INTER

#include "hiethv100.h"
#include "mac.h"


static int _set_linkstat(struct hieth_netdev_local *ld, int mode)
{
	int old;

	old = hieth_readl_bits(ld, MAC_STAT_SET, BITS_MACSTAT); 
	hieth_writel_bits(ld, mode, MAC_STAT_SET, BITS_MACSTAT); 

	return old; 
}

static int _set_negmode(struct hieth_netdev_local *ld, int mode)
{
	int old;

	old = hieth_readl_bits(ld, MAC_NEGMODE_SEL, BITS_NEGMODE);
	hieth_writel_bits(ld, mode, MAC_NEGMODE_SEL, BITS_NEGMODE);

	return old;
}

int hieth_set_linkstat(struct hieth_netdev_local *ld, int mode)
{
	unsigned long old;

	local_lock(ld);
	old = _set_linkstat(ld, mode);
	local_unlock(ld);

	return old;
}

int hieth_get_linkstat(struct hieth_netdev_local *ld)
{
	unsigned long old;

	local_lock(ld);
	old = hieth_readl_bits(ld, MAC_RO_STAT, BITS_MACSTAT);
	local_unlock(ld);

	return old;
}

int hieth_set_mac_leadcode_cnt_limit(struct hieth_netdev_local *ld, int cnt)
{
	int old;

	local_lock(ld);
	old = hieth_readl_bits(ld, MAC_PORT_TIMING, BITS_PRE_CNT_LIMIT);
	hieth_writel_bits(ld, cnt, MAC_PORT_TIMING, BITS_PRE_CNT_LIMIT);
	local_unlock(ld);

	return old;
}

int hieth_set_mac_trans_interval_bits(struct hieth_netdev_local *ld, int nbits)
{
	int old;
	int linkstat, negmode;

	local_lock(ld);

	negmode = _set_negmode(ld, HIETH_NEGMODE_CPUSET);
	linkstat = _set_linkstat(ld, 0);
	udelay(1000);

	old = hieth_readl_bits(ld, MAC_PORT_TIMING, BITS_IPG); 
	hieth_writel_bits(ld, nbits, MAC_PORT_TIMING, BITS_IPG); 
	udelay(100);

	_set_negmode(ld, negmode);
	_set_linkstat(ld, linkstat);

	local_unlock(ld);

	return old;
}

int hieth_set_mac_fc_interval(struct hieth_netdev_local *ld, int para)
{
	int old;

	local_lock(ld);
	old = hieth_readl_bits(ld, MAC_PORT_TIMING, BITS_FC_INTER); 
	hieth_writel_bits(ld, para, MAC_PORT_TIMING, BITS_FC_INTER); 
	local_unlock(ld);

	return old;
}

int hieth_set_negmode(struct hieth_netdev_local *ld, int mode)
{
	int old;

	local_lock(ld);
	old = _set_negmode(ld, mode);
	local_unlock(ld);

	return old;
}

void hieth_mac_reset(struct hieth_netdev_local *ld)
{
	hieth_writel(ld, 0x01e01fff, MAC_PORT_TIMING);
	hieth_writel(ld, 0x00000001, MAC_NEGMODE_SEL);
	hieth_writel(ld, 0x00000000, MAC_STAT_SET);
	hieth_writel(ld, 0x00000000, MAC_STAT_CHANGE);
	hieth_writel(ld, 0x0000010e, MAC_IPG_CTRL);
	hieth_writel(ld, 0x202755ee, MAC_PORT_CTRL);
}

