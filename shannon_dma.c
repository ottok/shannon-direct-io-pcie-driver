#include "shannon_dma.h"
#include <linux/dma-mapping.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/scatterlist.h>

#if !defined(CONFIG_SHANNON_EMU) && !defined(CONFIG_SHANNON_EMU_MODULE)
int shannon_dma_set_mask(shannon_pci_dev_t *pdev, u64 mask)
{
	return dma_set_mask(&((struct  pci_dev *)pdev)->dev, mask);
}

int shannon_dma_set_coherent_mask(shannon_pci_dev_t *pdev, u64 mask)
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

void * shannon_dma_alloc_coherent(shannon_pci_dev_t *pdev, shannon_size_t size, shannon_dma_addr_t *dma_handle, shannon_gfp_t gfp)
{
	return dma_alloc_coherent(&((struct pci_dev *)pdev)->dev, size, dma_handle, gfp);
}

void shannon_dma_free_coherent(shannon_pci_dev_t *pdev, shannon_size_t size, void *vaddr, shannon_dma_addr_t bus)
{
	dma_free_coherent(&((struct pci_dev *)pdev)->dev, size, vaddr, bus);
}


int shannon_dma_mapping_error(shannon_pci_dev_t *pdev, shannon_dma_addr_t dma_addr)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)

	return dma_mapping_error(&((struct pci_dev *)pdev)->dev, dma_addr);

#else

	return dma_mapping_error(dma_addr);

#endif
}

/* loop forever until dma_map_page returns no error */
shannon_dma_addr_t shannon_dma_map_one_sg_page(shannon_pci_dev_t *pdev, shannon_sg_list_t *sg, int dir)
{
	shannon_dma_addr_t dma_handle;

	while (1)
	{
	    dma_handle = dma_map_page(&((struct pci_dev *)pdev)->dev, shannon_sg_page(sg), \
		    shannon_sg_offset(sg), shannon_sg_length(sg), \
		    dir == SHANNON_DMA_FROMDEVICE ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
	    if (shannon_dma_mapping_error(pdev, dma_handle)) {
		    shannon_err("dma_map_page error!");
		    udelay(1);
	    } else {
		    shannon_sg_set_dma_address(sg, dma_handle);
		    break;
	    }
	}

	return dma_handle;
}

void shannon_dma_unmap_one_sg_page(shannon_pci_dev_t *pdev, shannon_sg_list_t *sg, int dir)
{
	dma_unmap_page(&((struct pci_dev *)pdev)->dev, shannon_sg_dma_address(sg), shannon_sg_length(sg),
		dir == SHANNON_DMA_FROMDEVICE ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
}

void shannon_dma_unmap_sg(shannon_pci_dev_t *pdev, shannon_sg_list_t *sgl, int nents, int dir)
{
	dma_unmap_sg(&((struct pci_dev *)pdev)->dev, (struct scatterlist *)sgl, nents, dir);
}

/* map all sgl entries or not at all, return num of mapped entries */
int shannon_dma_map_sg(shannon_pci_dev_t *pdev, shannon_sg_list_t *sgl, int nents, int dir)
{
	return dma_map_sg(&((struct pci_dev *)pdev)->dev, (struct scatterlist *)sgl, nents, dir);
}

shannon_dma_addr_t shannon_dma_map_single(shannon_pci_dev_t *pdev, void *ptr, shannon_size_t size, int dir)
{
	return dma_map_single(&((struct pci_dev *)pdev)->dev, ptr, size, \
			dir == SHANNON_DMA_FROMDEVICE ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
}

void shannon_dma_unmap_single(shannon_pci_dev_t *pdev, shannon_dma_addr_t addr, shannon_size_t size, int dir)
{
	dma_unmap_single(&((struct pci_dev *)pdev)->dev, addr, size, \
			dir == SHANNON_DMA_FROMDEVICE ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
}

shannon_dma_addr_t shannon_dma_map_page(shannon_pci_dev_t *pdev, void *ptr, shannon_size_t offset, shannon_size_t size, int dir)
{
	return dma_map_page(&((struct pci_dev *)pdev)->dev, (struct page *)ptr, offset, size, \
			dir == SHANNON_DMA_FROMDEVICE ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
}

void shannon_dma_unmap_page(shannon_pci_dev_t *pdev, shannon_dma_addr_t addr, shannon_size_t size, int dir)
{
	dma_unmap_page(&((struct pci_dev *)pdev)->dev, addr, size, \
			dir == SHANNON_DMA_FROMDEVICE ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
}
#endif /* !defined(CONFIG_SHANNON_EMU) && !defined(CONFIG_SHANNON_EMU_MODULE) */
