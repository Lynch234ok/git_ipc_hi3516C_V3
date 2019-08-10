
#ifndef __HISILICON_HIETH_MAC_H
#define __HISILICON_HIETH_MAC_H

#ifdef HIETHV100_INTER

#define MAC_PORT_TIMING		0x01A0	/* ETH_MAC_REG0 */
#define MAC_NEGMODE_SEL		0x01A4	/* ETH_MAC_REG1 */
#define MAC_STAT_SET		0x01AC	/* ETH_MAC_REG2 */
#define MAC_RO_STAT		0x01B0	/* ETH_MAC_REG3 */
#define MAC_STAT_CHANGE		0x01B4	/* ETH_MAC_REG4 */
#define MAC_IPG_CTRL		0x01B8	/* ETH_MAC_REG5 */
#define MAC_PORT_CTRL		0x01BC	/* ETH_MAC_REG6 */

/* bits of MAC_STAT_SET	and MAC_RO_STAT	*/
#define BITS_MACSTAT	MK_BITS(0, 3)

/* bits of MAC_NEGMODE_SEL */
#define BITS_NEGMODE	MK_BITS(0, 1)

/* bits of MAC_PORT_TIMING */
#define BITS_PRE_CNT_LIMIT	MK_BITS(23, 3)
#define BITS_IPG	MK_BITS(16, 7)
#define BITS_FC_INTER	MK_BITS(0, 16)

/* bits of MAC_PORT_CTRL */
#define BITS_FLOWCTRL_ENA	MK_BITS(18, 1)

#define hw_flowctrl_ena(ld, en) hieth_writel_bits(ld, (en)!=0, MAC_PORT_CTRL, BITS_FLOWCTRL_ENA)

#endif

#define HIETH_SPD_100M	(1<<2)
#define HIETH_LINKED	(1<<1)
#define HIETH_DUP_FULL	1

int hieth_set_mac_leadcode_cnt_limit(struct hieth_netdev_local *ld, int cnt);
int hieth_set_mac_trans_interval_bits(struct hieth_netdev_local *ld, int nbits);
int hieth_set_mac_fc_interval(struct hieth_netdev_local *ld, int para);

int hieth_set_linkstat(struct hieth_netdev_local *ld, int mode);
int hieth_get_linkstat(struct hieth_netdev_local *ld);

#define HIETH_NEGMODE_CPUSET	1
#define HIETH_NEGMODE_AUTO	0

int hieth_set_negmode(struct hieth_netdev_local *ld, int mode);

void hieth_mac_reset(struct hieth_netdev_local *ld);

#endif

