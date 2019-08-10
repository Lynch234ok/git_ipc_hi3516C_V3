
#ifndef __HISILICON_HIETH_MDIO_H
#define __HISILICON_HIETH_MDIO_H

struct hieth_mdio_local {
	unsigned long iobase;
	unsigned long iobase_phys;

	union {
		unsigned long w32_bits_flags;
		struct {
			/* mdio_bus freq-div, 1 for 1/100, 0 for 1/50 */
			int mdio_frqdiv :1;
		};
	};

	spinlock_t lock;
	unsigned long lockflags;
};

#ifdef HIETHV100_INTER

#define MDIO_RWCTRL	0x0180	/* ETH_MDIO_REG0 */
#define MDIO_RO_DATA	0x0184	/* ETH_MDIO_REG1 */
#define MDIO_PHYADDR	0x0188	/* ETH_MDIO_REG2 */
#define MDIO_RO_STAT	0x018C	/* ETH_MDIO_REG3 */
#define MDIO_ANEG_CTRL	0x0190	/* ETH_MDIO_REG4 */
#define MDIO_IRQENA	0x0194	/* ETH_MDIO_REG5 */

#define MDIO_MK_RWCTL( cpu_data_in, finish, rw, phy_exaddr, frq_div, phy_regnum) \
		( ((cpu_data_in)<<16 ) | \
		  (((finish)&0x01)<<15 ) | \
		  (((rw)&0x01)<<13 ) | \
		  (((phy_exaddr)&0x1F)<<8) | \
		  (((frq_div)&0x1)<<5) | \
		  ((phy_regnum)&0x1F) )

/* hardware set bit'15 of MDIO_REG(0) if mdio ready */
#define test_mdio_ready(ld) (hieth_readl(ld, MDIO_RWCTRL)&(1<<15))

#define mdio_start_phyread(ld, phy_id, regnum ) \
	hieth_writel(ld, MDIO_MK_RWCTL(0,0,0,phy_id,(ld)->mdio_frqdiv,regnum), MDIO_RWCTRL)
#define mdio_get_phyread_val(ld) (hieth_readl(ld, MDIO_RO_DATA) & 0xFFFF)

#define mdio_phywrite(ld, phy_id, regnum, val) \
	hieth_writel(ld, MDIO_MK_RWCTL(val,0,1,phy_id,(ld)->mdio_frqdiv,regnum), MDIO_RWCTRL)

/* write mdio registers reset value */
#define mdio_reg_reset(ld) do{ \
		hieth_writel(ld, 0x00008000, MDIO_RWCTRL); \
		hieth_writel(ld, 0x00000001, MDIO_PHYADDR); \
		hieth_writel(ld, 0x04631EA9, MDIO_ANEG_CTRL); \
		hieth_writel(ld, 0x00000000, MDIO_IRQENA); \
	} while(0)

#endif

/* APIs */

int hieth_mdio_read(struct hieth_mdio_local *ld, int phy_id, int regnum);
int hieth_mdio_write(struct hieth_mdio_local *ld, int phy_id, int regnum, int val);
int hieth_mdio_reset(struct hieth_mdio_local *ld);
int hieth_mdio_init(struct hieth_mdio_local *ld);
void hieth_mdio_exit(struct hieth_mdio_local *ld);

#endif

