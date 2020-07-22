#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#ifdef CONFIG_SUSE_PRODUCT_CODE
#include <linux/suse_version.h>
#endif
#include "shannon_port.h"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0) ||	\
	(defined(SHANNON_RHEL_RELEASE_OVER_8_0)) ||	\
	(defined(SHANNON_SUSE_RELEASE_OVER_1_12_5))
#include <linux/blk-mq.h>
#endif

#include "shannon_block.h"
#include "shannon_device.h"
#include "shannon_time.h"

extern void increase_ns_pending_bios(struct shannon_namespace *ns);
extern void decrease_ns_pending_bios(struct shannon_namespace *ns);
extern int check_and_alloc_lpmt(struct shannon_disk *sdisk, struct shannon_bio *sbio, unsigned int logicb_shift);
extern int shannon_fio_cpumask_set_enable;

int shannon_use_iosched = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0) ||	\
	(defined(SHANNON_RHEL_RELEASE_OVER_8_0)) ||	\
	defined(SHANNON_SUSE_RELEASE_OVER_1_12_5)
struct shannon_blk_mq_data {
	struct blk_mq_tag_set tag_set;
	void *original_data;
};
#endif

// Helpers
const char *get_gendisk_name(shannon_gendisk_t *gd)
{
	return ((struct gendisk *) gd)->disk_name;
}

//  genhd.h
shannon_gendisk_t *shannon_alloc_disk(int minors)
{
	return alloc_disk(minors);
}

extern struct block_device_operations shannon_ops;
extern struct block_device_operations shannon_ops_ns;
int shannon_init_gendisk(shannon_gendisk_t *disk, char *name, int major, int minor_span, int first_minor, shannon_request_queue_t *rq, void *pri)
{
	struct gendisk *gd = (struct gendisk *)disk;

	snprintf(gd->disk_name, 32, name);
	gd->major = major;
	gd->minors = minor_span;
	gd->first_minor = first_minor;
	debugs0("disk_name=%s, major=%d, minors=%d, first_minor=%d.\n",
			gd->disk_name, gd->major, gd->minors, gd->first_minor);
	gd->queue = (struct request_queue *)rq;
	gd->private_data = pri;
	gd->fops = &shannon_ops;
	/* dfX is a conventional disk; pXvolX is namespace. */
	if (*name != 'd')
		gd->fops = &shannon_ops_ns;

	return 0;
}

void shannon_set_capacity(shannon_gendisk_t *disk, shannon_sector_t size)
{
	set_capacity((struct gendisk *)disk, size);
}

void shannon_set_disk_ro(shannon_gendisk_t *disk, int flag)
{
	set_disk_ro((struct gendisk *)disk, flag);
}

void shannon_put_disk(shannon_gendisk_t *disk)
{
	put_disk((struct gendisk *)disk);
}

void shannon_add_disk(shannon_gendisk_t *disk)
{
	add_disk((struct gendisk *)disk);
}

void shannon_del_gendisk(shannon_gendisk_t *gp)
{
	del_gendisk((struct gendisk *)gp);
}

//  fs.h
int shannon_register_blkdev(unsigned int major, const char *name)
{
	return register_blkdev(major, name);
}

void shannon_unregister_blkdev(unsigned int major, const char *name)
{
	unregister_blkdev(major, name);
}

//  blkdev.h
void shannon_blk_queue_block_size(shannon_request_queue_t *queue, unsigned int logical_size, unsigned int physical_size)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)

	blk_queue_logical_block_size((struct request_queue *)queue, logical_size);
	blk_queue_physical_block_size((struct request_queue *)queue, physical_size);

#else

	blk_queue_hardsect_size((struct request_queue *)queue, logical_size);

#endif
}

void shannon_blk_queue_max_hw_sectors(shannon_request_queue_t *q, unsigned int max_hw_sectors)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)
	blk_queue_max_hw_sectors((struct request_queue *)q, max_hw_sectors);
#else
	blk_queue_max_sectors((struct request_queue *)q, max_hw_sectors);
#endif
}

void shannon_blk_queue_io_min(shannon_request_queue_t *queue, unsigned int min)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
	blk_queue_io_min((struct request_queue *)queue, min);
#endif
}

void shannon_blk_queue_io_opt(shannon_request_queue_t *queue, unsigned int opt)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
	blk_queue_io_opt((struct request_queue *)queue, opt);
#endif
}

static int shannon_should_trim_bio(shannon_bio_t *bio)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
	return ((struct bio *)bio)->bi_opf && (bio_op((struct bio *)bio) == REQ_OP_DISCARD);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	return ((struct bio *)bio)->bi_rw & REQ_DISCARD;
#else
#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 28)
	return bio_discard((struct bio *)bio);
#else
	return bio_rw_flagged((struct bio *)bio, BIO_RW_DISCARD);
#endif
#endif

#else
	return 0;
#endif
#endif

}


void shannon_queue_flag_set(int flag, shannon_request_queue_t *queue)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26)
	set_bit(flag, &(((struct request_queue *)queue)->queue_flags));
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0) &&	\
	!defined(SHANNON_SUSE_RELEASE_OVER_1_12_5)
	queue_flag_set_unlocked(flag, (struct request_queue *)queue);
#else
	blk_queue_flag_set(flag, (struct request_queue *)queue);
#endif
}


void shannon_queue_flag_clear(int flag, shannon_request_queue_t *queue)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26)
	clear_bit(flag, &(((struct request_queue *)queue)->queue_flags));
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0) &&	\
	!defined(SHANNON_SUSE_RELEASE_OVER_1_12_5)
	queue_flag_clear_unlocked(flag, (struct request_queue *)queue);
#else
	blk_queue_flag_clear(flag, (struct request_queue *)queue);
#endif
}

void shannon_trim_setting(shannon_request_queue_t *queue)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)

// RHEL 6.0 support discard in kernel 2.6.32
#ifdef SHANNON_ON_RHEL
#ifdef SHANNON_RHEL_RELEASE_OVER_6_0
	((struct request_queue *)queue)->limits.discard_granularity = PAGE_SIZE;
#endif
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	((struct request_queue *)queue)->limits.discard_granularity = PAGE_SIZE;
#endif

	((struct request_queue *)queue)->limits.max_discard_sectors = (UINT_MAX >> 9) & ~7;

// #if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37) && LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
//         ((struct request_queue *)queue)->limits.discard_zeroes_data = 1;
// #endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0) &&	\
	!defined(SHANNON_SUSE_RELEASE_OVER_1_12_5)
	queue_flag_set_unlocked(QUEUE_FLAG_DISCARD, (struct request_queue *)queue);
#else
	blk_queue_flag_set(QUEUE_FLAG_DISCARD, (struct request_queue *)queue);
#endif
#endif
}

void shannon_rotational_setting(shannon_request_queue_t *queue)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	shannon_queue_flag_set(QUEUE_FLAG_NONROT, queue);
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0) ||	\
	(defined(SHANNON_RHEL_RELEASE_OVER_8_0)) ||	\
	defined(SHANNON_SUSE_RELEASE_OVER_1_12_5)
void shannon_blk_mq_free_tag_set(struct blk_mq_tag_set *tag_set)
{
	blk_mq_free_tag_set(tag_set);
}

void shannon_blk_cleanup_queue(shannon_request_queue_t *q)
{
	struct request_queue *queue = (struct request_queue *) q;
	struct shannon_blk_mq_data *data = (struct shannon_blk_mq_data*) queue->queuedata;

	if (data && shannon_use_iosched) {
		shannon_blk_mq_free_tag_set(&data->tag_set);
		shannon_kfree(data);
	}

	blk_cleanup_queue(queue);
}
#else
void shannon_blk_cleanup_queue(shannon_request_queue_t *q)
{
	blk_cleanup_queue((struct request_queue *)q);
}
#endif

//  bio.h
static void shannon_bio_endio(shannon_bio_t *bio, int error)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)

	if (error)
		bio_endio((struct bio *)bio, 0, error);
	else
		bio_endio((struct bio *)bio, ((struct bio *)bio)->bi_size, 0);

#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)

	bio_endio((struct bio *)bio, error);

#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0)
#ifdef SHANNON_ON_SUSE
#ifdef SHANNON_SUSE_RELEASE_BELOW_1_12_5
	((struct bio *)bio)->bi_error = error;
	bio_endio((struct bio *)bio);
#else
	((struct bio *)bio)->bi_status = error;
	bio_endio((struct bio *)bio);
#endif
#else
	((struct bio *)bio)->bi_error = error;
	bio_endio((struct bio *)bio);
#endif
#else
	((struct bio *)bio)->bi_status = error;
	bio_endio((struct bio *)bio);
#endif
}

int shannon_bio_flagged(shannon_bio_t *bio, unsigned int flag)
{
	return bio_flagged((struct bio *)bio, flag);
}

unsigned long shannon_bio_data_dir(shannon_bio_t *bio)
{
	return bio_data_dir((struct bio *)bio);
}

static unsigned int shannon_bio_segments(shannon_bio_t *bio)
{
	return bio_segments((struct bio *)bio);
}

static unsigned int get_bi_size(shannon_bio_t *bio)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
	return ((struct bio *)bio)->bi_iter.bi_size;
#else
	return ((struct bio *)bio)->bi_size;
#endif
}

shannon_sector_t get_bi_sector(shannon_bio_t *bio)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
	return ((struct bio *)bio)->bi_iter.bi_sector;
#else
	return ((struct bio *)bio)->bi_sector;
#endif
}

extern shannon_gendisk_t *get_gendisk_from_ns(struct shannon_namespace *ns);
extern unsigned int ns_get_logicb_size(struct shannon_namespace *ns);
extern unsigned int ns_get_logicb_shift(struct shannon_namespace *ns);
extern struct shannon_pool *ns_get_pool(struct shannon_namespace *ns);
extern struct shannon_disk *get_shannon_disk_from_ns(struct shannon_namespace *ns);
extern u64 pool_used_logicbs(struct shannon_pool *sp);
extern u64 pool_available_cap(struct shannon_pool *sp);

extern shannon_gendisk_t *get_gendisk_from_sdev(struct shannon_dev *sdev);
extern unsigned int get_logicb_size(struct shannon_dev *sdev);
extern unsigned int get_logicb_shift(struct shannon_dev *sdev);
extern struct shannon_disk *get_shannon_disk_from_sdev(struct shannon_dev *sdev);

extern int shannon_check_availability(struct shannon_dev *sdev);
extern int shannon_check_availability_ns(struct shannon_namespace *);
extern int shannon_disk_readonly(struct shannon_dev *sdev);
extern int shannon_disk_readonly_ns(struct shannon_namespace *);
extern int shannon_submit_bio(struct shannon_dev *sdev, struct shannon_bio *sbio);
extern int shannon_submit_bio_throttling_ns(struct shannon_namespace *ns, struct shannon_bio *sbio);
extern void shannon_discard(struct shannon_dev *sdev, logicb64_t start_lba, logicb64_t end_lba);
extern void shannon_discard_ns(struct shannon_namespace *ns, logicb64_t start_lba, logicb64_t end_lba, int del_ns);

#ifdef CONFIG_SHANNON_DEBUG
void set_sbio_debug_tag(struct shannon_bio *sbio, unsigned long tag)
{
	sbio->tag = tag;
}
#endif

int shannon_alloc_bounce_pages(shannon_sg_list_t *sgtable, int first_size, int total_size)
{
	int page_cnt = 0, i = 0, j = 0;
	int first_page_offest = 0, last_page_len = 0;

	struct page *page = NULL;
	shannon_sg_list_t *sge = sgtable;

	if (first_size) {
		page_cnt++;
		first_page_offest = PAGE_SIZE - first_size;
	}

	page_cnt += (total_size - first_size) / PAGE_SIZE;
	last_page_len = (total_size - first_size) % PAGE_SIZE;

	if (last_page_len)
		page_cnt++;

	for (i = 0; i < page_cnt; i++) {
		page = alloc_page(GFP_KERNEL);

		if (!page)
			goto fail_out;

		if ((i == 0) && first_page_offest)
			shannon_sg_set_page(sge, page, first_size, first_page_offest);
		else if ((i == page_cnt - 1) && last_page_len)
			shannon_sg_set_page(sge, page, last_page_len, 0);
		else
			shannon_sg_set_page(sge, page, PAGE_SIZE, 0);

		sge = shannon_sg_next(sge);
	}

	shannon_sg_mark_end(sge);

	return page_cnt;

fail_out:
	shannon_for_each_sg(sgtable, sge, i, j)
		__free_page(shannon_sg_page(sge));

	return -ENOMEM;
}

/* We presume that page_address will never return NULL, which is true for 64bit platforms */

void shannon_copy_bounce_pages(shannon_bio_t *lbio, shannon_sg_list_t *sgtable, int to_sg)
{
	struct bio *bio = (struct bio *)lbio;
	shannon_sg_list_t *sge = sgtable;

	int sge_offset, sge_len;
	int bvec_offset, bvec_len;

	int copy_len;

	void *sge_addr, *bvec_addr;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
	struct bio_vec bvec;
	struct bvec_iter iter;

	sge_len = shannon_sg_length(sge);
	sge_offset = shannon_sg_offset(sge);

	bio_for_each_segment(bvec, bio, iter) {
		bvec_len = bvec.bv_len;
		bvec_offset = bvec.bv_offset;

		do {
			if (sge_len > bvec_len)
				copy_len = bvec_len;
			else
				copy_len = sge_len;

			bvec_addr = page_address(bvec.bv_page) + bvec_offset;
			sge_addr = page_address(shannon_sg_page(sge)) + sge_offset;

			if (to_sg)
				memcpy(sge_addr, bvec_addr, copy_len);
			else
				memcpy(bvec_addr, sge_addr, copy_len);

			bvec_offset += copy_len;
			bvec_len -= copy_len;

			sge_offset += copy_len;
			sge_len -= copy_len;

			if (sge_len <= 0) {
				sge = shannon_sg_next(sge);
				sge_len = shannon_sg_length(sge);
				sge_offset = shannon_sg_offset(sge);
			}
		} while (bvec_len > 0);
	}

#else
	struct bio_vec *bvec;
	int i;

	sge_len = shannon_sg_length(sge);
	sge_offset = shannon_sg_offset(sge);

	bio_for_each_segment(bvec, bio, i) {
		bvec_len = bvec->bv_len;
		bvec_offset = bvec->bv_offset;

		do {
			if (sge_len > bvec_len)
				copy_len = bvec_len;
			else
				copy_len = sge_len;

			bvec_addr = page_address(bvec->bv_page) + bvec_offset;
			sge_addr = page_address(shannon_sg_page(sge)) + sge_offset;

			if (to_sg)
				memcpy(sge_addr, bvec_addr, copy_len);
			else
				memcpy(bvec_addr, sge_addr, copy_len);

			bvec_offset += copy_len;
			bvec_len -= copy_len;

			sge_offset += copy_len;
			sge_len -= copy_len;

			if (sge_len <= 0) {
				sge = shannon_sg_next(sge);
				sge_len = shannon_sg_length(sge);
				sge_offset = shannon_sg_offset(sge);
			}
		} while (bvec_len > 0);
	}
#endif
}

void shannon_free_bounce_pages(shannon_sg_list_t *sgtable, int sg_count)
{
	shannon_sg_list_t *sge;
	int i;

	shannon_for_each_sg(sgtable, sge, sg_count, i)
		__free_page(shannon_sg_page(sge));
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
int shannon_convert_bio(struct shannon_bio *sbio, shannon_bio_t *lbio, unsigned int logicb_size)
{
	shannon_sg_list_t *sg = NULL;
	struct bio *bio = (struct bio *)lbio;
	struct bio_vec bvec;
	int offset, remained_size, map_size = 0, undone = 0;
	struct bvec_iter iter;

	int ret = 0;

	int page_offset, mappable_size;

	sbio->bio_size = get_bi_size(lbio);

	sbio->segments = shannon_bio_segments(lbio);
	sbio->sg_count = 2 * (((sbio->bio_size + logicb_size - 1)/logicb_size) + sbio->segments);

	sbio->sg = shannon_sg_alloc(sbio->sg_count, GFP_SHANNON);
	if (sbio->sg == NULL) {
		sbio->sg = shannon_sg_vzalloc(sbio->sg_count);
		if (sbio->sg == NULL) {
			shannon_err("alloc sg failed.\n");
			return -ENOMEM;
		}
		sbio->is_valloc = 1;
	}
	shannon_sg_init_table(sbio->sg, sbio->sg_count);

	sbio->start_sector = get_bi_sector(lbio);

	sbio->first_size = logicb_size - ((sbio->start_sector << 9) & (logicb_size - 1));
	sbio->first_size = sbio->first_size % logicb_size;
	sbio->first_size = min(sbio->bio_size, sbio->first_size);

	bio_for_each_segment(bvec, bio, iter) {
		if (bvec.bv_offset & 0x7) {
			sbio->need_bounce = 1;
		}
		if (bvec.bv_len & 0x7) {
			sbio->need_bounce = 1;
		}
	}

	if (sbio->need_bounce) {
		sbio->has_hole = 0;

		ret = shannon_alloc_bounce_pages(sbio->sg, sbio->first_size, sbio->bio_size);

		if (ret <= 0) {
			goto old_fashion;
		} else
			sbio->used_sg_count = ret;

		if (sbio->dma_dir == SHANNON_DMA_TODEVICE)
			shannon_copy_bounce_pages(bio, sbio->sg, 1);

		return 0;
	}


old_fashion:
	remained_size = sbio->first_size ? sbio->first_size : logicb_size;

	sbio->has_hole = 0;
	if (sbio->need_bounce) {
		sbio->has_hole = 1;
		sbio->need_bounce = 0;
	}

	sbio->used_sg_count = 0;
	bio_for_each_segment(bvec, bio, iter) {

		/* all segments' bv_offset have to be 0, except the first one. */
		if ((iter.bi_idx != bio->bi_iter.bi_idx) && (bvec.bv_offset != 0)) {
			sbio->has_hole = 1;
		}
		/* all segments have to end at 4096, except the last one. */
		if ((iter.bi_idx != (sbio->segments - 1)) && ((bvec.bv_offset + bvec.bv_len) != 4096)) {
			sbio->has_hole = 1;
		}

		offset = 0; /* the offset start from bvec->bv_offset. */

		page_offset = bvec.bv_offset + offset;

		while (offset < bvec.bv_len) {
			mappable_size = PAGE_SIZE - (page_offset & (PAGE_SIZE -1));
			if (mappable_size > (bvec.bv_len - offset))
				mappable_size = bvec.bv_len - offset;

			if (mappable_size < remained_size) {
				map_size = mappable_size;
				undone = 1;
			} else {
				map_size = remained_size;
				undone = 0;
			}
			remained_size -= map_size;
			if (remained_size == 0)
				remained_size = logicb_size;
			sg = sg ? shannon_sg_next(sg) : sbio->sg;
			shannon_sg_set_page(sg, bvec.bv_page + (page_offset / PAGE_SIZE), map_size, page_offset % PAGE_SIZE);
			sbio->used_sg_count++;

			offset += map_size;
			page_offset = bvec.bv_offset + offset;
		}
	}
	shannon_sg_mark_end(sg);

	return 0;
}
#else
int shannon_convert_bio(struct shannon_bio *sbio, shannon_bio_t *lbio, unsigned int logicb_size)
{
	shannon_sg_list_t *sg = NULL;
	struct bio *bio = (struct bio *)lbio;
	struct bio_vec *bvec;
	int i, offset, remained_size, map_size = 0, undone = 0;

	int page_offset, mappable_size;

	int ret = 0;

	sbio->bio_size = get_bi_size(lbio);

	sbio->segments = shannon_bio_segments(lbio);
	sbio->sg_count = 2 * (((sbio->bio_size + logicb_size - 1)/logicb_size) + sbio->segments);

	sbio->sg = shannon_sg_alloc(sbio->sg_count, GFP_SHANNON);
	if (sbio->sg == NULL) {
		sbio->sg = shannon_sg_vzalloc(sbio->sg_count);
		if (sbio->sg == NULL) {
			shannon_err("alloc sg failed.\n");
			return -ENOMEM;
		}
		sbio->is_valloc = 1;
	}
	shannon_sg_init_table(sbio->sg, sbio->sg_count);

	sbio->start_sector = get_bi_sector(lbio);

	sbio->first_size = logicb_size - ((sbio->start_sector << 9) & (logicb_size - 1));
	sbio->first_size = sbio->first_size % logicb_size;
	sbio->first_size = min(sbio->bio_size, sbio->first_size);

	bio_for_each_segment(bvec, bio, i) {
		if (bvec->bv_offset & 0x7) {
			sbio->need_bounce = 1;
		}
		if (bvec->bv_len & 0x7) {
			sbio->need_bounce = 1;
		}
	}

	if (sbio->need_bounce) {
		sbio->has_hole = 0;

		ret = shannon_alloc_bounce_pages(sbio->sg, sbio->first_size, sbio->bio_size);

		if (ret <= 0) {
			goto old_fashion;
		}else
			sbio->used_sg_count = ret;

		if (sbio->dma_dir == SHANNON_DMA_TODEVICE)
			shannon_copy_bounce_pages(bio, sbio->sg, 1);

		return 0;
	}


old_fashion:
	remained_size = sbio->first_size ? sbio->first_size : logicb_size;

	sbio->has_hole = 0;
	if (sbio->need_bounce) {
		sbio->has_hole = 1;
		sbio->need_bounce = 0;
	}

	sbio->used_sg_count = 0;
	bio_for_each_segment(bvec, bio, i) {

		/* all segments' bv_offset have to be 0, except the first one. */
		if ((i != bio->bi_idx) && (bvec->bv_offset != 0)) {
			sbio->has_hole = 1;
		}
		/* all segments have to end at 4096, except the last one. */
		if ((i != (sbio->segments - 1)) && ((bvec->bv_offset + bvec->bv_len) != 4096)) {
			sbio->has_hole = 1;
		}

		offset = 0; /* the offset start from bvec->bv_offset. */

		page_offset = bvec->bv_offset + offset;

		while (offset < bvec->bv_len) {
			mappable_size = PAGE_SIZE - (page_offset & (PAGE_SIZE -1));
			if (mappable_size > (bvec->bv_len - offset))
				mappable_size = bvec->bv_len - offset;

			if (mappable_size < remained_size) {
				map_size = mappable_size;
				undone = 1;
			} else {
				map_size = remained_size;
				undone = 0;
			}
			remained_size -= map_size;
			if (remained_size == 0)
				remained_size = logicb_size;
			sg = sg ? shannon_sg_next(sg) : sbio->sg;
			shannon_sg_set_page(sg, bvec->bv_page + (page_offset / PAGE_SIZE), map_size, page_offset % PAGE_SIZE);
			sbio->used_sg_count++;

			offset += map_size;
			page_offset = bvec->bv_offset + offset;
		}
	}
	shannon_sg_mark_end(sg);

	return 0;
}
#endif

void submit_sbio_task(struct shannon_work_struct *work)
{
	struct shannon_bio *sbio = container_of(work, struct shannon_bio, make_req_work);
	struct shannon_dev *sdev = (struct shannon_dev *)sbio->data;
	int ret;

	ret = shannon_submit_bio(sdev, sbio);
	if (ret) {
		shannon_end_io_acct(get_gendisk_from_sdev(sdev), sbio->bio, 0);
		if (sbio->sg) {
			if (sbio->is_valloc)
				shannon_sg_vfree(sbio->sg, sbio->sg_count);
			else
				shannon_sg_free(sbio->sg, sbio->sg_count);
		}
		shannon_bio_endio(sbio->bio, ret);
		free_sbio(sbio);
	}
}

void submit_sbio_task_ns(struct shannon_work_struct *work)
{
	struct shannon_bio *sbio = container_of(work, struct shannon_bio, make_req_work);
	struct shannon_namespace *ns = (struct shannon_namespace *)sbio->data;
	int ret;

	ret = shannon_submit_bio_throttling_ns(ns, sbio);
	if (ret) {
		shannon_end_io_acct(get_gendisk_from_ns(ns), sbio->bio, 0);
		if (sbio->sg) {
			if (sbio->is_valloc)
				shannon_sg_vfree(sbio->sg, sbio->sg_count);
			else
				shannon_sg_free(sbio->sg, sbio->sg_count);
		}
		shannon_bio_endio(sbio->bio, ret);
		free_sbio(sbio);
		decrease_ns_pending_bios(ns);
	}
}

extern int get_sdev_numa_node(struct shannon_dev *sdev);
void shannon_fio_cpumask_set(struct shannon_dev *sdev)
{
	int numa_node = get_sdev_numa_node(sdev);
	shannon_cpumask_struct_t *scpumask;

	if (numa_node < 0)
		return;
	if (shannon_get_current_comm() == NULL)
		return;
	if (0 == shannon_strcmp(shannon_get_current_comm(), "fio")) {
		scpumask = shannon_get_current_cpus_allowed();

		if(shannon_not_set_cpumask(scpumask)){
			shannon_set_node_cpus_allowed((shannon_task_struct_t *)shannon_current(), numa_node);
		}
	}

	return;
}

extern shannon_workqueue_struct_t *shannon_percpu_wq;
int shannon_make_request(shannon_request_queue_t *q, shannon_bio_t *bio)
{
	struct shannon_dev *sdev = ((struct request_queue *)q)->queuedata;
	struct shannon_bio *sbio;
	int ret;
	unsigned int logicb_size = get_logicb_size(sdev);
	unsigned int logicb_shift = get_logicb_shift(sdev);

	if (shannon_check_availability(sdev))
	{
		shannon_bio_endio(bio, -EIO);
		return 0;
	}

	if (unlikely(shannon_should_trim_bio(bio))) {
		/* in sectors */
		u64 trim_start = get_bi_sector(bio);
		u64 trim_end = trim_start + (get_bi_size(bio) >> 9);
		trim_start = _ALIGN_UP(trim_start, logicb_size/512);
		trim_end = _ALIGN_DOWN(trim_end, logicb_size/512);
		/* in logicbs */
		trim_start = trim_start >> (logicb_shift - 9);
		trim_end = trim_end >> (logicb_shift - 9);
		shannon_discard(sdev, trim_start, trim_end);
		shannon_bio_endio(bio, 0);
		return 0;
	}

	if (unlikely(get_bi_size(bio) == 0)) {
		shannon_bio_endio(bio, 0);
		return 0;
	}

	if (unlikely(shannon_disk_readonly(sdev)) && (shannon_bio_data_dir(bio) == LINUX_BIO_WRITE)) {
		shannon_bio_endio(bio, -EIO);
		return 0;
	}

	if (shannon_fio_cpumask_set_enable) {
		shannon_fio_cpumask_set(sdev);
	}

	sbio = alloc_sbio(GFP_SHANNON);
	sbio->bio = bio;
	sbio->lreq = NULL;
#ifdef CONFIG_SHANNON_ATOMIC_WRITE_VERIFY
	shannon_strncpy(sbio->atomic_write_comm, shannon_get_current_comm(), 32);
#endif
	if (shannon_bio_data_dir(bio))
		sbio->dma_dir = SHANNON_DMA_TODEVICE;
	else
		sbio->dma_dir = SHANNON_DMA_FROMDEVICE;
	SHANNON_INIT_LIST_HEAD(&sbio->global_list);
	set_sbio_debug_tag(sbio, MAKE_REQUEST_TAG);

	ret = shannon_convert_bio(sbio, bio, logicb_size);
	if (ret)
		goto free_sbio;

	if (sbio->dma_dir == SHANNON_DMA_TODEVICE) {
		if (unlikely(check_and_alloc_lpmt(get_shannon_disk_from_sdev(sdev), sbio, logicb_shift))) {
			ret = -EIO;
			goto free_sg_list;
		}
	}

	// skipped request queue and io scheduler, must do disk statistics manually.
	sbio->start_time = get_jiffies();
	shannon_start_io_acct(get_gendisk_from_sdev(sdev), sbio->bio);

	if (likely(shannon_percpu_wq)) {
		sbio->data = sdev;
		shannon_init_work(&sbio->make_req_work, submit_sbio_task);
		shannon_queue_work(shannon_percpu_wq, &sbio->make_req_work);
	} else {
		ret = shannon_submit_bio(sdev, sbio);
		if (ret)
			goto end_io_acct;
	}

	return 0;

end_io_acct:
	shannon_end_io_acct(get_gendisk_from_sdev(sdev), sbio->bio, 0);
free_sg_list:
	if (sbio->need_bounce)
		shannon_free_bounce_pages(sbio->sg, sbio->used_sg_count);
	if (sbio->sg_count) {
		if (sbio->is_valloc)
			shannon_sg_vfree(sbio->sg, sbio->sg_count);
		else
			shannon_sg_free(sbio->sg, sbio->sg_count);
	}
free_sbio:
	free_sbio(sbio);
	shannon_bio_endio(bio, ret);
	return 0;
 }

struct shannon_dev *get_shannon_dev_from_ns(struct shannon_namespace *ns);
int check_sbio_is_overwrited(struct shannon_bio *sbio, struct shannon_namespace *ns);
int shannon_make_request_ns(shannon_request_queue_t *q, shannon_bio_t *bio)
{
	struct shannon_namespace *ns = ((struct request_queue *)q)->queuedata;
	struct shannon_bio *sbio;
	int ret;
	unsigned int logicb_size = ns_get_logicb_size(ns);
	unsigned int logicb_shift = ns_get_logicb_shift(ns);

	increase_ns_pending_bios(ns);
	if (shannon_check_availability_ns(ns)) {
		shannon_bio_endio(bio, -EIO);
		decrease_ns_pending_bios(ns);
		return 0;
	}

	if (unlikely(shannon_should_trim_bio(bio))) {
		/* in sectors */
		u64 trim_start = get_bi_sector(bio);
		u64 trim_end = trim_start + (get_bi_size(bio) >> 9);
		trim_start = _ALIGN_UP(trim_start, logicb_size/512);
		trim_end = _ALIGN_DOWN(trim_end, logicb_size/512);
		/* in logicbs */
		trim_start = trim_start >> (logicb_shift - 9);
		trim_end = trim_end >> (logicb_shift - 9);
		shannon_discard_ns(ns, trim_start, trim_end, 0);
		shannon_bio_endio(bio, 0);
		decrease_ns_pending_bios(ns);
		return 0;
	}

	if (unlikely(get_bi_size(bio) == 0)) {
		shannon_bio_endio(bio, 0);
		decrease_ns_pending_bios(ns);
		return 0;
	}

	sbio = alloc_sbio(GFP_SHANNON);
	sbio->bio = bio;
	sbio->lreq = NULL;
	if (shannon_bio_data_dir(bio))
		sbio->dma_dir = SHANNON_DMA_TODEVICE;
	else
		sbio->dma_dir = SHANNON_DMA_FROMDEVICE;
	SHANNON_INIT_LIST_HEAD(&sbio->global_list);
	set_sbio_debug_tag(sbio, MAKE_REQUEST_TAG);

	ret = shannon_convert_bio(sbio, bio, logicb_size);
	if (ret)
		goto free_sbio;
	if (sbio->dma_dir == SHANNON_DMA_TODEVICE) {
		if (unlikely(check_and_alloc_lpmt(get_shannon_disk_from_ns(ns), sbio, logicb_shift))) {
			ret = -EIO;
			goto free_sg_list;
		}
	}

	if ((shannon_bio_data_dir(bio) == LINUX_BIO_WRITE)
	    && (unlikely(shannon_disk_readonly_ns(ns))
		|| (pool_used_logicbs(ns_get_pool(ns)) > pool_available_cap(ns_get_pool(ns))/8
		    && !check_sbio_is_overwrited(sbio, ns)))) {
		ret = -EIO;
		goto free_sg_list;
	}

	/* Do disk accounting manually. */
	sbio->start_time = get_jiffies();
	shannon_start_io_acct(get_gendisk_from_ns(ns), sbio->bio);

	if (likely(shannon_percpu_wq)) {
		sbio->data = ns;
		shannon_init_work(&sbio->make_req_work, submit_sbio_task_ns);
		shannon_queue_work(shannon_percpu_wq, &sbio->make_req_work);
	} else {
		ret = shannon_submit_bio_throttling_ns(ns, sbio);
		if (ret)
			goto end_io_acct;
	}

	return 0;

end_io_acct:
	shannon_end_io_acct(get_gendisk_from_ns(ns), sbio->bio, 0);
free_sg_list:
	if (sbio->sg) {
		if (sbio->is_valloc)
			shannon_sg_vfree(sbio->sg, sbio->sg_count);
		else
			shannon_sg_free(sbio->sg, sbio->sg_count);
	}
free_sbio:
	free_sbio(sbio);
	shannon_bio_endio(bio, ret);
	decrease_ns_pending_bios(ns);
	return 0;
}

//  make_request
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
static blk_qc_t shannon_make_request_wrapper(struct request_queue *q, struct bio *bio)
{
	shannon_make_request(q, bio);
	return BLK_QC_T_NONE;
}

static blk_qc_t shannon_make_request_wrapper_ns(struct request_queue *q, struct bio *bio)
{
	shannon_make_request_ns(q, bio);
	return BLK_QC_T_NONE;
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
static void shannon_make_request_wrapper(struct request_queue *q, struct bio *bio)
{
	shannon_make_request(q, bio);
}

static void shannon_make_request_wrapper_ns(struct request_queue *q, struct bio *bio)
{
	shannon_make_request_ns(q, bio);
}
#else
static int shannon_make_request_wrapper(struct request_queue *q, struct bio *bio)
{
	return shannon_make_request(q, bio);
}

static int shannon_make_request_wrapper_ns(struct request_queue *q, struct bio *bio)
{
	return shannon_make_request_ns(q, bio);
}
#endif

#define LREQ_IO_GOOD	1
#define LREQ_IO_ERROR	0

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25)

static inline void complete_buffers(struct bio *bio, int status)
{
	while (bio) {
		struct bio *xbh = bio->bi_next;

		bio->bi_next = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
		blk_finished_io(bio_sectors(bio));
#endif
		shannon_bio_endio(bio, status ? 0 : -EIO);
		bio = xbh;
	}
}

static void shannon_end_request(shannon_lreq_t *lreq)
{
	struct request *rq = (struct request *)lreq;
	struct request_queue *q = rq->q;
	unsigned long flags;

	complete_buffers(rq->bio, rq->errors);

	if (blk_fs_request(rq)) {
		const int rw = rq_data_dir(rq);
		disk_stat_add(rq->rq_disk, sectors[rw], rq->nr_sectors);
	}

	add_disk_randomness(rq->rq_disk);
	spin_lock_irqsave(q->queue_lock, flags);
	end_that_request_last(rq, rq->errors);
	spin_unlock_irqrestore(q->queue_lock, flags);
}

#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)


static void shannon_end_request(shannon_lreq_t *lreq)
{
	struct request *rq = (struct request *)lreq;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)
	blk_end_request_all(rq, rq->errors ? 0 : -EIO);
#else
	blk_end_request(rq, rq->errors ? 0 : -EIO, blk_rq_bytes(rq));
#endif
}

#else /* LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0) */

static void shannon_end_request(shannon_lreq_t *lreq, int error)
{
	struct request *rq = (struct request *)lreq;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0) ||	\
	(defined(SHANNON_RHEL_RELEASE_OVER_8_0)) ||	\
	defined(SHANNON_SUSE_RELEASE_OVER_1_12_5)
	blk_mq_end_request(rq, error ? BLK_STS_OK : BLK_STS_IOERR);
#else
	blk_end_request(rq, error ? 0 : -EIO, blk_rq_bytes(rq));
#endif
}

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
static void shannon_set_lreq_errors(shannon_lreq_t *lreq, int errors)
{
	struct request *rq = (struct request *)lreq;

	rq->errors = errors;
}
#endif

extern void shannon_scsi_end_io_acct(struct shannon_scsi_private *hostdata, struct shannon_bio *sbio, unsigned long duration);
void shannon_complete_fs_io(void *hostdata, shannon_gendisk_t *gd, struct shannon_bio *sbio)
{
	/* A fs sbio is converted from either a bio or a request*/
	if (sbio->bio) {
		if (sbio->need_bounce) {
			if (sbio->dma_dir == SHANNON_DMA_FROMDEVICE)
				shannon_copy_bounce_pages(sbio->bio, sbio->sg, 0);
			shannon_free_bounce_pages(sbio->sg, sbio->used_sg_count);
		}
		// If we skipped request queue and io scheduler, must do disk statistics manually.
		shannon_end_io_acct(gd, sbio->bio, get_jiffies() - sbio->start_time);
		if (likely(sbio->status == 0))
			shannon_bio_endio(sbio->bio, 0);
		else {
			shannon_err("%s: Error sectors!, status=0x%x, dma_dir=%d.\n", get_gendisk_name(gd), sbio->status, sbio->dma_dir);
			shannon_bio_endio(sbio->bio, -EIO);
		}
	} else if (sbio->lreq) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
		if (likely(sbio->status == 0))
			shannon_set_lreq_errors(sbio->lreq, LREQ_IO_GOOD);
		else {
			shannon_set_lreq_errors(sbio->lreq, LREQ_IO_ERROR);
			shannon_err("%s: Error sectors!, status=0x%x.\n", get_gendisk_name(gd), sbio->status);
		}
		shannon_end_request(sbio->lreq);
#else
		shannon_end_request(sbio->lreq, sbio->status ? LREQ_IO_ERROR : LREQ_IO_GOOD);
#endif
	} else if (sbio->scsi_cmnd) {
		shannon_scsi_end_io_acct((struct shannon_scsi_private *)hostdata, sbio, get_jiffies() - sbio->start_time);
		if (likely(sbio->status == 0))
			end_scsi_cmnd(sbio, STATUS_CODE_GOOD, NULL);
		else
			end_scsi_cmnd(sbio, STATUS_CODE_CHECK_CONDITION, NULL);
	} else {
		shannon_err("%s: BUG! A fs sbio isn't related to a bio or a request!", get_gendisk_name(gd));
		BUG();
	}
}


static inline sector_t shannon_blk_rq_pos(struct request *rq)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)
	return blk_rq_pos(rq);
#else
	return rq->hard_sector;
#endif
}



static inline unsigned int shannon_blk_rq_bytes(struct request *rq)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
	return blk_rq_bytes(rq);
#else
	if (blk_fs_request(rq))
		return rq->hard_nr_sectors << 9;
	return rq->data_len;
#endif
}

static unsigned int shannon_lreq_segments(shannon_lreq_t *lreq)
{
	struct request *rq = (struct request *)lreq;
	struct bio *bio = NULL;
	unsigned int segments = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	__rq_for_each_bio(bio, rq)
		segments += shannon_bio_segments(bio);
#else
	rq_for_each_bio(bio, rq)
		segments += shannon_bio_segments(bio);
#endif

	return segments;
}

const char *get_cdev_name(struct shannon_dev *sdev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
int shannon_convert_lreq(struct shannon_dev *sdev, struct shannon_bio *sbio, shannon_lreq_t *lreq)
{
	shannon_sg_list_t *sg = NULL;
	struct bio_vec bvec;
	struct request *rq = (struct request *)lreq;
	int i, offset, remained_size, map_size = 0, undone = 0;
	unsigned int logicb_size = get_logicb_size(sdev);
	struct req_iterator iter;

	sbio->segments = shannon_lreq_segments(lreq);
	sbio->sg_count = 2 * sbio->segments + 1;
	sbio->sg = shannon_sg_alloc(sbio->sg_count, GFP_SHANNON);
	if (sbio->sg == NULL) {
		shannon_err("%s: alloc sg failed.\n", get_cdev_name(sdev));
		return -ENOMEM;
	}
	shannon_sg_init_table(sbio->sg, sbio->sg_count);

	sbio->bio_size = shannon_blk_rq_bytes(rq);
	sbio->start_sector = shannon_blk_rq_pos(rq);

	sbio->first_size = logicb_size - ((sbio->start_sector << 9) & (logicb_size - 1));
	sbio->first_size = sbio->first_size % logicb_size;
	sbio->first_size = min(sbio->bio_size, sbio->first_size);


	remained_size = sbio->first_size ? sbio->first_size : logicb_size;

	sbio->has_hole = 0;
	sbio->used_sg_count = 0;

	i = 0;
	rq_for_each_segment(bvec, rq, iter)
		{
		/* all segments' bv_offset have to be 0, except the first one. */
		if ((i != 0) && (bvec.bv_offset != 0))
			sbio->has_hole = 1;
		/* all segments have to end at 4096, except the last one. */
		if ((i != (sbio->segments - 1)) && ((bvec.bv_offset + bvec.bv_len) != 4096))
			sbio->has_hole = 1;

		offset = 0; /* the offset start from bvec->bv_offset. */
		while (offset < bvec.bv_len) {
			if ((bvec.bv_len - offset) < remained_size) {
				map_size = bvec.bv_len - offset;
				undone = 1;
			} else {
				map_size = remained_size;
				undone = 0;
			}
			remained_size -= map_size;
			if (remained_size == 0)
				remained_size = logicb_size;
			sg = sg ? shannon_sg_next(sg) : sbio->sg;
			shannon_sg_set_page(sg, bvec.bv_page, map_size, bvec.bv_offset + offset);
			sbio->used_sg_count++;

			offset += map_size;
		}
		i++;
	}

	shannon_sg_mark_end(sg);

	return 0;
}
#else
int shannon_convert_lreq(struct shannon_dev *sdev, struct shannon_bio *sbio, shannon_lreq_t *lreq)
{
	shannon_sg_list_t *sg = NULL;
	struct bio_vec *bvec;
	struct request *rq = (struct request *)lreq;
	int i, offset, remained_size, map_size = 0, undone = 0;
	unsigned int logicb_size = get_logicb_size(sdev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	struct req_iterator iter;
#else
	struct bio *bio;
	int j;
#endif

	sbio->segments = shannon_lreq_segments(lreq);
	sbio->sg_count = 2 * sbio->segments + 1;
	sbio->sg = shannon_sg_alloc(sbio->sg_count, GFP_SHANNON);
	if (sbio->sg == NULL) {
		shannon_err("%s: alloc sg failed.\n", get_cdev_name(sdev));
		return -ENOMEM;
	}
	shannon_sg_init_table(sbio->sg, sbio->sg_count);

	sbio->bio_size = shannon_blk_rq_bytes(rq);
	sbio->start_sector = shannon_blk_rq_pos(rq);

	sbio->first_size = logicb_size - ((sbio->start_sector << 9) & (logicb_size - 1));
	sbio->first_size = sbio->first_size % logicb_size;
	sbio->first_size = min(sbio->bio_size, sbio->first_size);


	remained_size = sbio->first_size ? sbio->first_size : logicb_size;

	sbio->has_hole = 0;
	sbio->used_sg_count = 0;

	i = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	rq_for_each_segment(bvec, rq, iter)
#else
	rq_for_each_bio(bio, rq)
		bio_for_each_segment(bvec, bio, j)
#endif
		{
		/* all segments' bv_offset have to be 0, except the first one. */
		if ((i != 0) && (bvec->bv_offset != 0))
			sbio->has_hole = 1;
		/* all segments have to end at 4096, except the last one. */
		if ((i != (sbio->segments - 1)) && ((bvec->bv_offset + bvec->bv_len) != 4096))
			sbio->has_hole = 1;

		offset = 0; /* the offset start from bvec->bv_offset. */
		while (offset < bvec->bv_len) {
			if ((bvec->bv_len - offset) < remained_size) {
				map_size = bvec->bv_len - offset;
				undone = 1;
			} else {
				map_size = remained_size;
				undone = 0;
			}
			remained_size -= map_size;
			if (remained_size == 0)
				remained_size = logicb_size;
			sg = sg ? shannon_sg_next(sg) : sbio->sg;
			shannon_sg_set_page(sg, bvec->bv_page, map_size, bvec->bv_offset + offset);
			sbio->used_sg_count++;

			offset += map_size;
		}
		i++;
	}

	shannon_sg_mark_end(sg);

	return 0;
}
#endif

int shannon_disk_xfer_request(struct shannon_dev *sdev, struct request *rq)
{
	int ret;
	struct shannon_bio *sbio = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	unsigned int logicb_size = get_logicb_size(sdev);
#endif
	unsigned int logicb_shift = get_logicb_shift(sdev);


	if (shannon_check_availability(sdev))
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
		rq->errors = LREQ_IO_ERROR;
		shannon_end_request(rq);
#else
		shannon_end_request(rq, LREQ_IO_ERROR);
#endif
		return 0;
	}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 19)
	if (!blk_fs_request(rq))
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
	if (rq->cmd_type != REQ_TYPE_FS)
#else
	if (blk_rq_is_passthrough(rq))
#endif
	{
		debugs1("skip non-fs request.\n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
		rq->errors = LREQ_IO_ERROR;
		shannon_end_request(rq);
#else
		shannon_end_request(rq, LREQ_IO_ERROR);
#endif
		return 0;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
	if (unlikely(req_op(rq) == REQ_OP_DISCARD)) {
#else
	if (unlikely(rq->cmd_flags & REQ_DISCARD)) {
#endif
		/* in sectors */
		u64 trim_start = shannon_blk_rq_pos(rq);
		u64 trim_end = trim_start + (shannon_blk_rq_bytes(rq) >> 9);
		trim_start = _ALIGN_UP(trim_start, logicb_size/512);
		trim_end = _ALIGN_DOWN(trim_end, logicb_size/512);
		/* in logicbs */
		trim_start = trim_start >> (logicb_shift - 9);
		trim_end = trim_end >> (logicb_shift - 9);
		shannon_discard(sdev, trim_start, trim_end);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
		rq->errors = LREQ_IO_GOOD;
		shannon_end_request(rq);
#else
		shannon_end_request(rq, LREQ_IO_GOOD);
#endif
		return 0;
	}
#endif

	if (unlikely(shannon_blk_rq_bytes(rq) == 0)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
		rq->errors = LREQ_IO_GOOD;
		shannon_end_request(rq);
#else
		shannon_end_request(rq, LREQ_IO_GOOD);
#endif
		return 0;
	}

	if (unlikely(shannon_disk_readonly(sdev)) && (rq_data_dir(rq) == LREQ_WRITE)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
		rq->errors = LREQ_IO_ERROR;
		shannon_end_request(rq);
#else
		shannon_end_request(rq, LREQ_IO_ERROR);
#endif
		return 0;
	}

	sbio = alloc_sbio(GFP_SHANNON);
	sbio->bio = NULL;
	sbio->lreq = rq;
	if (rq_data_dir(rq))
		sbio->dma_dir = SHANNON_DMA_TODEVICE;
	else
		sbio->dma_dir = SHANNON_DMA_FROMDEVICE;
	set_sbio_debug_tag(sbio, LINUX_REQ_TAG);

	ret = shannon_convert_lreq(sdev, sbio, rq);
	if (ret)
		goto free_sbio;

	if (sbio->dma_dir == SHANNON_DMA_TODEVICE) {
		if (unlikely(check_and_alloc_lpmt(get_shannon_disk_from_sdev(sdev), sbio, logicb_shift))) {
			ret = -EIO;
			goto free_sg_list;
		}
	}


	ret = shannon_submit_bio(sdev, sbio);
	if (ret)
		goto free_sg_list;

	return 0;

free_sg_list:
	if (sbio->sg) {
		if (sbio->is_valloc)
			shannon_sg_vfree(sbio->sg, sbio->sg_count);
		else
			shannon_sg_free(sbio->sg, sbio->sg_count);
	}
free_sbio:
	free_sbio(sbio);
	if (ret == -ENOMEM) {
		/* will enqueue this request again */
		return -ENOMEM;
	} else {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
		rq->errors = LREQ_IO_ERROR;
		shannon_end_request(rq);
#else
		shannon_end_request(rq, LREQ_IO_ERROR);
#endif
		return 0;
	}
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0) ||	\
	(defined(SHANNON_RHEL_RELEASE_OVER_8_0)) ||	\
	defined(SHANNON_SUSE_RELEASE_OVER_1_12_5)
blk_status_t shannon_disk_request(struct blk_mq_hw_ctx *hctx,
		const struct blk_mq_queue_data *bd)
{
	int result;
	blk_status_t ret = BLK_STS_OK;
	struct request *rq = bd->rq;
	struct shannon_blk_mq_data *data = rq->q->queuedata;
	struct shannon_dev *sdev = data->original_data;

	blk_mq_start_request(rq);

	result = shannon_disk_xfer_request(sdev, rq);
	if (result == -ENOMEM)
		ret = BLK_STS_IOERR;
	return ret;
}
#else // LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
void shannon_disk_request(struct request_queue *q)
{
	struct shannon_dev *sdev = q->queuedata;
	struct request *rq;
	int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)
	while ((rq = blk_peek_request(q))) {
		blk_start_request(rq);
#else
	while ((rq = elv_next_request(q))) {
		blkdev_dequeue_request(rq);
#endif
		spin_unlock_irq(q->queue_lock);

		ret = shannon_disk_xfer_request(sdev, rq);
		if (ret == -ENOMEM) {
			spin_lock_irq(q->queue_lock);
			blk_requeue_request(q, rq);
			spin_unlock_irq(q->queue_lock);
		}
		spin_lock_irq(q->queue_lock);
	}
}
#endif // LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0) &&	\
	!defined(SHANNON_SUSE_RELEASE_OVER_1_12_5)
static int shannon_elevator_change(struct request_queue *q, char *name)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	elevator_exit(q->elevator);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
	return elevator_change(q, name);
#else
	elevator_exit(q, q->elevator);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
	q->elevator = NULL;
#endif
	return elevator_init(q, name);
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0) ||	\
	(defined(SHANNON_RHEL_RELEASE_OVER_8_0)) ||	\
	defined(SHANNON_SUSE_RELEASE_OVER_1_12_5)
struct blk_mq_ops shannon_blk_mq_ops = {
	.queue_rq = shannon_disk_request,
};

int shannon_blk_mq_init_tag_set(struct blk_mq_tag_set *tag_set)
{
	int ret;

	tag_set->ops = &shannon_blk_mq_ops;
	tag_set->nr_hw_queues = 1;
	tag_set->queue_depth = 128;
	tag_set->numa_node = NUMA_NO_NODE;
	tag_set->flags =
		BLK_MQ_F_SHOULD_MERGE | BLK_MQ_F_NO_SCHED;
	tag_set->driver_data = NULL;
	ret = blk_mq_alloc_tag_set(tag_set);

	return ret;
}

static shannon_request_queue_t *shannon_init_queue(void *data, shannon_spinlock_t *lock)
{
	struct request_queue *queue = NULL;
	struct shannon_blk_mq_data *blk_mq_data = NULL;
	int ret;

	blk_mq_data = (struct shannon_blk_mq_data *) shannon_kzalloc(sizeof(struct shannon_blk_mq_data), GFP_SHANNON);
	if (blk_mq_data == NULL) {
		shannon_err("Cannot allocate blk mq data.\n");
		goto end;
	}

	ret = shannon_blk_mq_init_tag_set(&blk_mq_data->tag_set);
	if (ret) {
		shannon_err("Allocate tag set failed with ret = %d.\n", ret);
		goto free_blk_mq_data;
	}

	queue = blk_mq_init_queue(&blk_mq_data->tag_set);
	if (SHANNON_IS_ERR(queue)) {
		shannon_err("Request queue creation failed, err = %ld", (long) SHANNON_PTR_ERR(queue));
		goto free_tag_set;
	} else if (queue == NULL) {
		shannon_err("Request queue creation failed, returned NULL.\n");
		goto free_tag_set;
	}
	else {
		blk_mq_data->original_data = data;
		queue->queuedata = blk_mq_data;
	}

	return queue;

free_tag_set:
	blk_mq_free_tag_set(&blk_mq_data->tag_set);
free_blk_mq_data:
	shannon_kfree(blk_mq_data);
end:
	return NULL;
}
#else // LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
static shannon_request_queue_t *shannon_init_queue(void *data, shannon_spinlock_t *lock)
{
	struct request_queue *queue = NULL;

	shannon_spin_lock_init(lock);
	queue = blk_init_queue((request_fn_proc *)shannon_disk_request,
		(spinlock_t *)lock);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0)
	if (queue) {
		static char elevator_name[] = "noop";
		if (shannon_elevator_change(queue, elevator_name)) {
			shannon_err("Failed to initialize noop io scheduler.\n");
			blk_cleanup_queue(queue);
			queue = NULL;
		} else {
			shannon_info("using noop io scheduler.\n");
			queue->queuedata = data;
		}
	}
#endif

	return queue;
}
#endif // LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)

static shannon_request_queue_t *shannon_alloc_queue(void *data, int ns)
{
	struct request_queue *queue = NULL;

	queue = blk_alloc_queue(GFP_SHANNON);
	if (queue) {
		if (ns)
			blk_queue_make_request(queue, shannon_make_request_wrapper_ns);
		else
			blk_queue_make_request(queue, shannon_make_request_wrapper);
		queue->queuedata = data;
	}

	return queue;
}

shannon_request_queue_t *shannon_create_blkqueue(void *data, shannon_spinlock_t *lock, int ns)
{
	if (shannon_use_iosched)
		return shannon_init_queue(data, lock);
	else
		return shannon_alloc_queue(data, ns);
}
