#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define WRITE_LOG_ERROR(fmt, ...) 
#define WRITE_LOG_INFO(fmt, ...) 

typedef struct tag_MMAP_Node
{
	unsigned int Start_P;
	unsigned int Start_V;
	unsigned int length;
    unsigned int refcount;
	struct tag_MMAP_Node * next;
}TMMAP_Node_t;

TMMAP_Node_t * pTMMAPNode = NULL;

#define PAGE_SIZE 0x1000
#define PAGE_SIZE_MASK 0xfffff000

static int fd = -1;
static const char dev[]="/dev/mem";


/* no need considering page_size of 4K */
void * memmap(unsigned int phy_addr, unsigned int size)
{
	unsigned int phy_addr_in_page;
	unsigned int page_diff;

	unsigned int size_in_page;

	TMMAP_Node_t * pTmp;
	TMMAP_Node_t * pNew;
	
	void *addr=NULL;

	if(size == 0)
	{
		WRITE_LOG_ERROR("memmap():size can't be zero!\n");
		return NULL;
	}

	/* check if the physical memory space have been mmaped */
	pTmp = pTMMAPNode;
	while(pTmp != NULL)
	{
		if( (phy_addr >= pTmp->Start_P) && 
			( (phy_addr + size) <= (pTmp->Start_P + pTmp->length) ) )
		{
            pTmp->refcount++;   /* referrence count increase by 1  */
			return (void *)(pTmp->Start_V + phy_addr - pTmp->Start_P);
		}

		pTmp = pTmp->next;
	}

	/* not mmaped yet */
	if(fd < 0)
	{
		/* dev not opened yet, so open it */
		fd = open (dev, O_RDWR | O_SYNC);
		if (fd < 0)
		{
			WRITE_LOG_ERROR("memmap():open %s error!\n", dev);
			return NULL;
		}
	}

	/* addr align in page_size(4K) */
	phy_addr_in_page = phy_addr & PAGE_SIZE_MASK;
	page_diff = phy_addr - phy_addr_in_page;

	/* size in page_size */
	size_in_page =((size + page_diff - 1) & PAGE_SIZE_MASK) + PAGE_SIZE;

	addr = mmap ((void *)0, size_in_page, PROT_READ|PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);
	if (addr == MAP_FAILED)
	{
		WRITE_LOG_ERROR("memmap():mmap @ 0x%x error!\n", phy_addr_in_page);
		return NULL;
	}

	/* add this mmap to MMAP Node */
	pNew = (TMMAP_Node_t *)malloc(sizeof(TMMAP_Node_t));
    if(NULL == pNew)
    {
        WRITE_LOG_ERROR("memmap():malloc new node failed!\n");
        return NULL;
    }
	pNew->Start_P = phy_addr_in_page;
	pNew->Start_V = (unsigned int)addr;
	pNew->length = size_in_page;
    pNew->refcount = 1;
	pNew->next = NULL;
	
	if(pTMMAPNode == NULL)
	{
		pTMMAPNode = pNew;
	}
	else
	{
		pTmp = pTMMAPNode;
		while(pTmp->next != NULL)
		{
			pTmp = pTmp->next;
		}

		pTmp->next = pNew;
	}

	return (void *)(addr+page_diff);
}

/*****************************************************************************
 Prototype    : memunmap
 Description  : 
 Input        : void * addr_mapped  
 Output       : None
 Return Value : On success, returns 0, on failure -1 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2005/12/21
    Author       : Z42136
    Modification : Created function

*****************************************************************************/
int memunmap(void * addr_mapped)
{
	TMMAP_Node_t * pPre;
	TMMAP_Node_t * pTmp;

    if(pTMMAPNode == NULL)
    {
        WRITE_LOG_ERROR("memunmap(): address have not been mmaped!\n");
        return -1;
    }

	/* check if the physical memory space have been mmaped */
	pTmp = pTMMAPNode;
    pPre = pTMMAPNode;

    do
	{
		if( ((unsigned int)addr_mapped >= pTmp->Start_V) && 
			((unsigned int)addr_mapped <= (pTmp->Start_V + pTmp->length)) )
		{
            pTmp->refcount--;   /* referrence count decrease by 1  */
            if(0 == pTmp->refcount)
            {
                WRITE_LOG_INFO("memunmap(): map node will be remove!\n");

                /* delete this map node from pMMAPNode */
                if(pTmp == pTMMAPNode)
                {
                    pTMMAPNode = NULL;
                }
                else
                {
                    pPre->next = pTmp->next;
                }

                /* munmap */
                if(munmap((void *)pTmp->Start_V, pTmp->length) != 0 )
                {
                    WRITE_LOG_INFO("memunmap(): munmap failed!\n");
                }

                free(pTmp);
            }
            
            return 0;
		}

        pPre = pTmp;
		pTmp = pTmp->next;
	}while(pTmp != NULL);

    WRITE_LOG_ERROR("memunmap(): address have not been mmaped!\n");
    return -1;
}

