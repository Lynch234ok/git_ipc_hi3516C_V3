
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

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include "hiethv100.h"
#include "mdio.h"


/* MDIO Bus Interface */

static int hieth_mdiobus_read(struct mii_bus *bus, int phy_id, int regnum)
{
	return hieth_mdio_read(bus->priv, phy_id, regnum);
}

static int hieth_mdiobus_write(struct mii_bus *bus, int phy_id, int regnum, u16 val)
{
	return hieth_mdio_write(bus->priv, phy_id, regnum, val);
}

static int hieth_mdiobus_reset(struct mii_bus *bus)
{
	return hieth_mdio_reset(bus->priv);
}

/* Initialize and Probe Functions */

static int __init hieth_mdiobus_probe(struct device *device)
{
	int i;
	int ret = 0;
	struct mii_bus *newbus;
	struct hieth_mdio_local *ld;
	struct platform_device *platdev;
	struct resource *res;

	platdev = to_platform_device(device);
	res = platform_get_resource(platdev, IORESOURCE_MEM, 0);
	if(!res) {
		hieth_error("get ioresource failed!");
		return -1;
	}

	newbus = kzalloc(sizeof(*newbus) + sizeof(*ld) + sizeof(int)*PHY_MAX_ADDR, GFP_KERNEL);
	if(newbus ==NULL)
		return -ENOMEM;
	ld = (void *)(&newbus[1]);
	newbus->priv = ld;

	newbus->name = "Hisilicon ETHv100 MDIO Bus";
	newbus->dev  = device;
	newbus->read = hieth_mdiobus_read;
	newbus->write= hieth_mdiobus_write;
	newbus->reset= hieth_mdiobus_reset;
	newbus->id   = 0;
	newbus->irq  = (void*)((char*)newbus + sizeof(*newbus) + sizeof(*ld));

	for(i=0; i<PHY_MAX_ADDR; i++)
		newbus->irq[i] = PHY_POLL;

	ld->iobase = IO_ADDRESS(res->start);
	ld->iobase_phys = res->start;
	ld->mdio_frqdiv = 1;

	hieth_mdio_init(ld);

	ret = mdiobus_register(newbus);
	if(ret){
		hieth_error("register mdiobus failed!\n");
		kfree(newbus);

		return ret;
	}

        dev_set_drvdata(device, newbus);

	return ret;
}

static int __exit hieth_mdiobus_remove(struct device *device)
{
	struct mii_bus *mdiobus = dev_get_drvdata(device);
	struct hieth_mdio_local *ld = mdiobus->priv;

	mdiobus_unregister(mdiobus);
	dev_set_drvdata(device, NULL);
	hieth_mdio_exit(ld);
	kfree(mdiobus);

	return 0;
}

static struct device_driver hieth_mdiobus_driver ={
	.owner = THIS_MODULE,

	.name = HIETH_MDIOBUS_NAME,
	.bus = &platform_bus_type,
	.probe = hieth_mdiobus_probe,
	.remove = hieth_mdiobus_remove,
};

int hieth_mdiobus_driver_init(void)
{
	if(driver_register(&hieth_mdiobus_driver)) {
		hieth_error("register mdiobus failed!");
		return -1;
	}

	return 0;
}

void hieth_mdiobus_driver_exit(void)
{
	driver_unregister(&hieth_mdiobus_driver);
}


