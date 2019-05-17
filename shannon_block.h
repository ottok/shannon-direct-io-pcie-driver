#ifndef __SHANNON_BLOCK_H
#define __SHANNON_BLOCK_H

#include "shannon_list.h"
#include "shannon_config.h"
#include "shannon_kcore.h"
#include "shannon_scatter.h"
#include "shannon_dma.h"
#include "shannon_workqueue.h"
#include "shannon_scsi.h"

// Core block device layer functions are defined in this header file.
typedef void shannon_gendisk_t;
typedef void shannon_request_queue_t;
typedef void shannon_bio_t;
typedef void shannon_lreq_t;
typedef unsigned long shannon_sector_t;
typedef u32 logicb_t;
typedef u64 logicb64_t;

#define _ALIGN_UP(addr,size) (((addr)+((typeof(addr))(size)-1))&(~((typeof(addr))(size)-1)))
#define _ALIGN_DOWN(addr,size) ((addr)&(~((typeof(addr))(size)-1)))

#define MAKE_REQUEST_TAG		0x7878787878780000
#define LINUX_REQ_TAG			0xdefdefdefdef0000

#define LINUX_BIO_READ       0
#define LINUX_BIO_WRITE      1

#define LREQ_READ	0
#define LREQ_WRITE	1


struct shannon_namespace;
struct shannon_bio {
#ifdef CONFIG_SHANNON_DEBUG
	unsigned long tag;
#ifdef CONFIG_SHANNON_DEBUG_REQS
	struct shannon_list_head debug_list;
#endif
#endif
	unsigned long start_time;
	shannon_atomic_t user_count;
	int dma_dir;
	int corrected_bits;  /* ECC corrected bits */
#define HAVE_BLANK_SECTOR	1 /* bit 0 */
#define HAVE_ERROR_SECTOR	2 /* bit 1 */
	int status;
#define BUFFER_WRITE		1
#define DIRECT_WRITE		2
	int write_method;

	shannon_sector_t start_sector;
	unsigned int bio_size;
	unsigned int first_size;
	int has_hole;
	logicb_t logicbs;
	int segments;

	u16 ns_id;
	u16 ns_seq_num;
	unsigned long wait_dev_pick_mask;

	struct shannon_work_struct make_req_work;
	struct shannon_work_struct cb_work;
	void (*callback)(struct shannon_bio *sbio);
	void *data;
	void *data2;
	int may_sleep_in_callback;

	/* you have the alternative of bio or virt_addr */
	void *virt_addr;
	shannon_dma_addr_t dma_address;

	int latency;	/* in ms */
	struct shannon_list_head req_list;
	struct shannon_list_head global_list;

	shannon_bio_t *bio;
	shannon_lreq_t *lreq;
	int sg_count;
	int used_sg_count;
	shannon_sg_list_t *sg;

	int overlap;
	int need_bounce;

	/* for scsi */
	struct shannon_scsi_private *hostdata;
	void *scsi_cmnd;
	unsigned char *cmd;
	struct shannon_work_struct scsi_work;
};

/* expose functions implemented in shannon_main.c */
extern struct shannon_bio *alloc_sbio(gfp_t gfp_mask);
extern void free_sbio(struct shannon_bio *sbio);

#ifdef CONFIG_SHANNON_DEBUG
extern void set_sbio_debug_tag(struct shannon_bio *, unsigned long);
#else
static inline void set_sbio_debug_tag(struct shannon_bio *sbio, unsigned long tag){}
#endif

//  genhd.h
extern shannon_gendisk_t *shannon_alloc_disk(int minors);
extern int shannon_init_gendisk(shannon_gendisk_t *disk, char *name, int major, int minor_span, int first_minor, shannon_request_queue_t *rq, void *pri);
extern void shannon_set_capacity(shannon_gendisk_t *disk, shannon_sector_t size);
extern void shannon_set_disk_ro(shannon_gendisk_t *disk, int flag);
extern void shannon_put_disk(shannon_gendisk_t *disk);
extern void shannon_add_disk(shannon_gendisk_t *disk);
extern void shannon_del_gendisk(shannon_gendisk_t *gp);

//  fs.h
extern int shannon_register_blkdev(unsigned int major, const char *name);
extern void shannon_unregister_blkdev(unsigned int major, const char *name);

struct shannon_namespace;

//  blkdev.h
struct shannon_dev;
struct shannon_disk;
extern shannon_request_queue_t *shannon_create_blkqueue(void *, shannon_spinlock_t *lock, int);
extern void shannon_blk_queue_block_size(shannon_request_queue_t *queue, unsigned int, unsigned int);
extern void shannon_blk_queue_max_hw_sectors(shannon_request_queue_t *, unsigned int);
extern void shannon_blk_queue_io_min(shannon_request_queue_t *queue, unsigned int min);
extern void shannon_blk_queue_io_opt(shannon_request_queue_t *queue, unsigned int opt);
extern void shannon_blk_cleanup_queue(shannon_request_queue_t *q);
extern void shannon_trim_setting(shannon_request_queue_t *queue);
extern void shannon_rotational_setting(shannon_request_queue_t *queue);

//  bio.h
#define BIO_RW_PRIO	16
extern shannon_sector_t get_bi_sector(shannon_bio_t *bio);
extern int shannon_bio_flagged(shannon_bio_t *bio, unsigned int flag);
extern unsigned long shannon_bio_data_dir(shannon_bio_t *bio);
extern void shannon_complete_fs_io(void *hostdata, shannon_gendisk_t *gd, struct shannon_bio *sbio);

// shannon_scsi.c
extern void end_scsi_cmnd(struct shannon_bio *sbio, enum shannon_scsi_cmd_status status, unsigned char *buf);


#endif /* __SHANNON_BLOCK_H */
