
#define HIETHV100_INTER

#include "hiethv100.h"
#include "mdio.h"
#include "mac.h"
#include "ctrl.h"
#include "glb.h"


int hieth_glb_preinit_dummy(struct hieth_netdev_local *ld)
{
	local_lock_init(ld);

	hieth_set_linkstat(ld, 0);
	hieth_writel_bits(ld, ~0, GLB_IRQ_ENABLE, BITS_IRQS); 
	hieth_irq_disable(ld, ~0);
	hieth_set_negmode(ld, HIETH_NEGMODE_CPUSET);
	hieth_set_endian_mode(ld, HIETH_LITTLE_ENDIAN);

	return 0;
}

