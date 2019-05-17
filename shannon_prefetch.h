/*
 * Shannon PCIE SSD driver
 *
 * Copyright (C) 2018 Shannon Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __SHANNON_PREFETCH_H
#define __SHANNON_PREFETCH_H

#define get_cache_line_from_lba(prefetch, lba)	(((lba) / (prefetch)->lba_per_line) % (prefetch)->cache_lines)
#define lba_rounddown_cache_line(prefetch, lba)	(((lba) / (prefetch)->lba_per_line) * (prefetch)->lba_per_line)
#define lba_roundup_cache_line(prefetch, lba)	((((lba) / (prefetch)->lba_per_line) * ((prefetch)->lba_per_line + 1)) - 1)
#define get_cache_line_struct(prefetch, l)	(&(prefetch)->cache_line[(l)])
#define get_cache_slot_index_addr(slot, index)	((slot)->cache[(index)])

struct shannon_cache_slot {
	int slot_index;
	u32 cache_count;
#define SLOT_WAIT_DATA_BIT			(1)
#define SLOT_HANDLE_REQ_LIST_BIT	(2)
	unsigned long state;
	shannon_spinlock_t list_lock;
	struct shannon_list_head req_list;
	struct shannon_cache_line *cache_line;

	char **cache;
}__attribute__((aligned));

struct shannon_cache_line {
	u32 cache_line_index;

#define CACHE_LINE_DESTROYED	(0)
#define CACHE_LINE_EMPTY		(1)
#define CACHE_LINE_READY		(2)
	unsigned long line_state;
	int slot_count;
	struct shannon_prefetch *prefetch;
	logicb64_t first_lba;
	logicb64_t start_prefetch_lba;
	shannon_atomic_t user_count;
	shannon_rwlock_t cache_line_lock;
	struct shannon_cache_slot *slot;
}__attribute__((aligned));

#define PREFETCH_LBA_PER_CACHE_LINE	(32)
#define PREFETCH_CACHE_LINE_COUNT	(64)
#define PREFETCH_CACHE_PER_SLOT		(8)
struct shannon_prefetch {
#define get_prefetch_enable_state(_prefetch)	(shannon_test_bit(PREFETCH_ENABLE_BIT, &(_prefetch)->state))
#define set_prefetch_enable(_prefetch)			(shannon_set_bit(PREFETCH_ENABLE_BIT, &(_prefetch)->state))
#define set_prefetch_disable(_prefetch)			(shannon_clear_bit(PREFETCH_ENABLE_BIT, &(_prefetch)->state))
#define get_prefetch_working_state(_prefetch)	(shannon_test_bit(PREFETCH_WORKING_BIT, &(_prefetch)->state))
#define set_prefetch_working(_prefetch)			(shannon_set_bit(PREFETCH_WORKING_BIT, &(_prefetch)->state))
#define set_prefetch_sleeping(_prefetch)		(shannon_clear_bit(PREFETCH_WORKING_BIT, &(_prefetch)->state))
#define PREFETCH_ENABLE_BIT		(0)
#define PREFETCH_WORKING_BIT	(1)
	unsigned long state;

	u64 cache_hits;
	u64 wait_cache_read_hits;
	u64 cache_miss;
	u64 prefetch_count;
#define DEFAULT_SEQREAD_THRESHOLD	(128)		// do prefetch when seq read times.
	u32 seqread_threshold;
#define DEFAULT_PREFETCH_TRAFFIC_FACTOR	(2)
	u32 traffic_factor;
#define DEFAULT_PREFETCH_DISTANCE_FACTOR	(4)
	int distance_factor;
	shannon_atomic64_t seqread_count;
	u64 last_seqread_count;
	shannon_atomic64_t last_hit_lba;
	logicb64_t current_prefetch_lba;
#define DEFAULT_PREFETCH_POLL_TIMES		(4)
	int poll_times_threshold;
	int poll_times;
#define PREFETCH_SOFT_BIO_SIZE_THRESHOLD	(32 * 1024)	// 32k
#define PREFETCH_HARD_BIO_SIZE_THRESHOLD	(PREFETCH_LBA_PER_CACHE_LINE * 4096 * 2)	// 2 cache lines size
	int soft_bio_size_threshold;
	int hard_bio_size_threshold;
#define PREFETCH_LARGE_BLOCK_IO_THRESHOLD	(4)
	int large_block_io_threshold;
	shannon_spinlock_t list_lock;
	struct shannon_list_head req_list;

	struct shannon_cache_line *cache_line;
	shannon_task_struct_t *prefetch_thread;

	u32 total_size;
	u32 total_caches;
	u32 cache_lines;
	u32 slot_per_line;
	u32 lba_per_line;
	u32 cache_per_slot;
	u32 line_size;
};

#endif
