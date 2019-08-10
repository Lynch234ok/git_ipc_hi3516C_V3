#ifndef __MEM_MAP_H__
#define __MEM_MAP_H__

extern void * memmap(unsigned int phy_addr, unsigned int size);
extern int memunmap(void * addr_mapped);

#endif //__MEM_MAP_H__
