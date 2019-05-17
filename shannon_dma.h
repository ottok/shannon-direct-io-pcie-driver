#ifndef __SHANNON_DMA_H
#define __SHANNON_DMA_H

#include "shannon_kcore.h"
#include "shannon_scatter.h"

#define SHANNON_DMA_TODEVICE	1
#define SHANNON_DMA_FROMDEVICE	2

#define SHANNON_DMA_BIT_MASK(n)	(((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))

#if defined(CONFIG_SHANNON_EMU) || defined(CONFIG_SHANNON_EMU_MODULE)

#include <linux/pci.h>
#include <linux/version.h>
#include <linux/dma-mapping.h>

static inline shannon_dma_addr_t shannon_dma_map_one_sg_page(shannon_pci_dev_t *pdev, shannon_sg_list_t *sgl, int dir)
{
	return (shannon_dma_addr_t)shannon_sg_virt(sgl);
}

static inline void shannon_dma_unmap_one_sg_page(shannon_pci_dev_t *pdev, shannon_sg_list_t *sgl, int dir)
{
	return;
}

static inline int shannon_dma_map_sg(shannon_pci_dev_t *pdev, shannon_sg_list_t *sgl, int nents, int dir)
{
	int i;

	for (i = 0; i < nents; i++) {
		shannon_sg_dma_address(sgl) = (shannon_dma_addr_t)shannon_sg_virt(sgl);
		sgl = shannon_sg_next(sgl);
	}
	return nents;
}

static inline void shannon_dma_unmap_sg(shannon_pci_dev_t *pdev, shannon_sg_list_t *sg, int nents, int dir)
{
	return;
}

static inline shannon_dma_addr_t shannon_dma_map_single(shannon_pci_dev_t *pdev, void *ptr, shannon_size_t size, int dir)
{
	return (shannon_dma_addr_t)ptr;
}

static inline void shannon_dma_unmap_single(shannon_pci_dev_t *pdev, shannon_dma_addr_t addr, shannon_size_t size, int dir)
{
	return;
}

static inline int shannon_dma_mapping_error(shannon_pci_dev_t *pdev, shannon_dma_addr_t dma_addr)
{
	return 0;
}

static inline int shannon_dma_set_mask(shannon_pci_dev_t *pdev, u64 mask)
{
	return dma_set_mask(&((struct pci_dev *)pdev)->dev, mask);
}

static inline int shannon_dma_set_coherent_mask(shannon_pci_dev_t *pdev, u64 mask)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 34)

	if (!dma_supported(&((struct pci_dev *)pdev)->dev, mask))
		return -EIO;
	((struct pci_dev *)pdev)->dev.coherent_dma_mask = mask;
	return 0;

#else

	return dma_set_coherent_mask(&((struct pci_dev *)pdev)->dev,mask);

#endif
}

static inline void * shannon_dma_alloc_coherent(shannon_pci_dev_t *pdev, shannon_size_t size, shannon_dma_addr_t *dma_handle, shannon_gfp_t gfp)
{
	return dma_alloc_coherent(&((struct pci_dev *)pdev)->dev, size, dma_handle, gfp);
}

static inline void shannon_dma_free_coherent(shannon_pci_dev_t *pdev, shannon_size_t size, void *vaddr, shannon_dma_addr_t bus)
{
	dma_free_coherent(&((struct pci_dev *)pdev)->dev, size, vaddr, bus);
}

#else

extern shannon_dma_addr_t shannon_dma_map_one_sg_page(shannon_pci_dev_t *pdev, shannon_sg_list_t *sg, int dir);
extern void shannon_dma_unmap_one_sg_page(shannon_pci_dev_t *pdev, shannon_sg_list_t *sg, int dir);
extern int shannon_dma_map_sg(shannon_pci_dev_t *pdev, shannon_sg_list_t *sg, int nents, int dir);
extern void shannon_dma_unmap_sg(shannon_pci_dev_t *pdev, shannon_sg_list_t *sg, int nents, int dir);
extern shannon_dma_addr_t shannon_dma_map_single(shannon_pci_dev_t *pdev, void *ptr, shannon_size_t size, int dir);
extern void shannon_dma_unmap_single(shannon_pci_dev_t *pdev, shannon_dma_addr_t addr, shannon_size_t size, int dir);
extern int shannon_dma_mapping_error(shannon_pci_dev_t *pdev, shannon_dma_addr_t dma_addr);

extern int shannon_dma_set_mask(shannon_pci_dev_t *pdev, u64 mask);
extern int shannon_dma_set_coherent_mask(shannon_pci_dev_t *pdev, u64 mask);
extern void * shannon_dma_alloc_coherent(shannon_pci_dev_t *pdev, shannon_size_t size, shannon_dma_addr_t *dma_handle, gfp_t gfp);
extern void shannon_dma_free_coherent(shannon_pci_dev_t *pdev, shannon_size_t size, void *vaddr, shannon_dma_addr_t bus);
extern shannon_dma_addr_t shannon_dma_map_page(shannon_pci_dev_t *pdev, void *ptr, shannon_size_t offset, shannon_size_t size, int dir);
extern void shannon_dma_unmap_page(shannon_pci_dev_t *pdev, shannon_dma_addr_t addr, shannon_size_t size, int dir);


#endif	/* #if defined(CONFIG_SHANNON_EMU) || defined(CONFIG_SHANNON_EMU_MODULE) */

#endif	/* #ifndef __SHANNON_DMA_H */
