#include "shannon_scatter.h"
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>
#include <linux/vmalloc.h>
#include <linux/version.h>

shannon_dma_addr_t shannon_sg_dma_address(shannon_sg_list_t *sg)
{
	return sg_dma_address((struct scatterlist *)sg);
}

void shannon_sg_set_dma_address(shannon_sg_list_t *sg, shannon_dma_addr_t dma_addr)
{
	sg_dma_address((struct scatterlist *)sg) = dma_addr;
}

shannon_page *shannon_sg_page(shannon_sg_list_t *sg)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	return sg_page((struct scatterlist *)sg);
#else
	return ((struct scatterlist *)sg)->page;
#endif
}

unsigned int shannon_sg_offset(shannon_sg_list_t *sg)
{
	return ((struct scatterlist *)sg)->offset;
}

unsigned int shannon_sg_length(shannon_sg_list_t *sg)
{
	return ((struct scatterlist *)sg)->length;
}

void shannon_sg_mark_end(shannon_sg_list_t *sg)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	sg_mark_end((struct scatterlist *)sg);
#endif
}

shannon_sg_list_t *shannon_sg_alloc(unsigned int nents, shannon_gfp_t gfp_mask)
{
	return kzalloc(sizeof(struct scatterlist) * nents, gfp_mask);
}

shannon_sg_list_t *shannon_sg_vzalloc(unsigned int nents)
{
	u64 size = sizeof(struct scatterlist) * nents;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36)
	return vzalloc(size);
#else
	void *addr = vmalloc(size);
	if (addr)
		memset(addr, 0, size);

	return addr;
#endif
}

void shannon_sg_free(shannon_sg_list_t *sgl, unsigned int nents)
{
	kfree(sgl);
}

void shannon_sg_vfree(shannon_sg_list_t *sgl, unsigned int nents)
{
	vfree(sgl);
}

void shannon_sg_init_table(shannon_sg_list_t *sgl, unsigned int nents)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	sg_init_table(sgl, nents);
#else
	memset(sgl, 0, sizeof(struct scatterlist) * nents);
#endif
}

void shannon_sg_set_page(shannon_sg_list_t *sg, shannon_page *page,
			       unsigned int len, unsigned int offset)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	return sg_set_page((struct scatterlist *)sg, page, len, offset);
#else
	struct scatterlist *sg_linux = NULL;

	sg_linux = (struct scatterlist *)sg;

	sg_linux->page = page;
	sg_linux->offset = offset;
	sg_linux->length = len;
#endif
}

void *shannon_sg_virt(shannon_sg_list_t *sg)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	return sg_virt((struct scatterlist *)sg);
#else
	return page_address(((struct scatterlist *)sg)->page) + ((struct scatterlist *)sg)->offset;
#endif
}

shannon_sg_list_t *shannon_sg_next(shannon_sg_list_t *sg)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	return sg_next((struct scatterlist *)sg);
#else
	return (struct scatterlist *)sg + 1;
#endif
}
