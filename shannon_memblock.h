/* Definitions for Shannon hardware/software interface.
 * Copyright (c) 2017, Shannon technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __SHANNON_MEMBLOCK_H
#define __SHANNON_MEMBLOCK_H

#include <linux/types.h>
#define MAP_TABLE	(0x1)
#define TEMP_TABLE	(0x2)
#define MAP_TABLE_MEMBLOCK_SIZE_SHIFT	(22)				// 4MB
#define TEMP_TABLE_MEMBLOCK_SIZE_SHIFT	(20)				// 1MB
#define MAP_TABLE_ENTRY_SIZE	(sizeof(u32))
#define TEMP_TABLE_ENTRY_SIZE	(sizeof(u8))

//#define get_slot_from_lpmt(sdisk, lba)		(((((lba) / ((sdisk)->strip_size * (sdisk)->sdev_count)) * (sdisk)->strip_size)) + (lba) % (sdisk)->strip_size)
#define get_slot_from_lpmt(sdisk, lba)	({	\
	u64 __slot;		\
	u32 _strip_size = (sdisk)->strip_size * (sdisk)->sdev_count;	\
	if ((sdisk)->sdev_count == 1)	\
		__slot = (lba);	\
	else if ((_strip_size & (_strip_size - 1)) == 0)	/* order of 2 */\
		__slot = (((lba) >> find_first_bit((void *)&_strip_size, sizeof(u32))) << (sdisk)->strip_size_shift) + ((lba) & ((sdisk)->strip_size - 1));	\
	else	\
		__slot = (((((lba) / ((sdisk)->strip_size * (sdisk)->sdev_count)) << (sdisk)->strip_size_shift)) + ((lba) & ((sdisk)->strip_size - 1)));	\
			\
	__slot;	\
})
#define get_lba_from_lpmt_slot(sdev, sdisk, slot)	(((slot) >> (sdisk)->strip_size_shift) * ((sdisk)->strip_size * (sdisk)->sdev_count) + \
		((slot) & ((sdisk)->strip_size - 1)) + (sdev)->sdev_id * (sdisk)->strip_size)

#define LPMT_INVALID	(~0x0u)
struct shannon_memblock_pool {
	u32 memblock_size;
	u32 entry_size;
#define DEFAULT_MIN_THRESHOLD	(30)
	shannon_atomic_t min_threshold;
	shannon_atomic_t max_threshold;
	shannon_atomic_t free_cnt;
	shannon_atomic_t used_cnt;
	struct shannon_work_struct alloc_work;
	shannon_workqueue_struct_t *memblock_wq;

	shannon_spinlock_t list_lock;
	u8 *last;
	u8 *free_list;
};

struct scatter_memblock {
	u64 total_size;
	u64 memblock_size;
	u32 memblock_size_shift;
	u32 entry_size;
	u32 entrys_per_memblock;
	u32 memblock_count;
#define MEMBLOCK_LOCK_CNT	(16)		// it must be order of 2
	shannon_mutex_t list_lock[MEMBLOCK_LOCK_CNT];

	u8 **memblock_list;

	struct shannon_memblock_pool *mpool;
};

extern int check_and_alloc_memblock(struct scatter_memblock *smb, logicb64_t offset);

#endif /* __SHANNON_MEMBLOCK_H */
