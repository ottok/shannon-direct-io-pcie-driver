#ifndef __SHANNON_SCATTER_H
#define __SHANNON_SCATTER_H
#include "shannon_kcore.h"

typedef void shannon_sg_list_t;

extern shannon_dma_addr_t shannon_sg_dma_address(shannon_sg_list_t *sg);
extern void shannon_sg_set_dma_address(shannon_sg_list_t *sg, shannon_dma_addr_t dma_addr);
extern shannon_page *shannon_sg_page(shannon_sg_list_t *sg);
extern unsigned int shannon_sg_offset(shannon_sg_list_t *sg);
extern unsigned int shannon_sg_length(shannon_sg_list_t *sg);
extern void shannon_sg_mark_end(shannon_sg_list_t *sg);

#define shannon_for_each_sg(sglist, sg, nr, __i)       \
	for (__i = 0, sg = (sglist); __i < (nr); __i++, sg = shannon_sg_next(sg))


extern shannon_sg_list_t *shannon_sg_alloc(unsigned int nents, shannon_gfp_t gfp_mask);
extern shannon_sg_list_t *shannon_sg_vzalloc(unsigned int nents);
extern void shannon_sg_free(shannon_sg_list_t *sgl, unsigned int nents);
extern void shannon_sg_vfree(shannon_sg_list_t *sgl, unsigned int nents);
extern void shannon_sg_init_table(shannon_sg_list_t *sgl, unsigned int nents);

extern shannon_sg_list_t *shannon_sg_next(shannon_sg_list_t *sg);
extern void *shannon_sg_virt(shannon_sg_list_t *sg);
extern void shannon_sg_set_page(shannon_sg_list_t *sg, shannon_page *page, unsigned int len, unsigned int offset);

#endif /* __SHANNON_SCATTER_H */
