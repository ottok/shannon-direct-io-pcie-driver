/*
 * Shannon Err Injection
 *
 * Copyright (C) 2015 Shannon Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifdef CONFIG_SHANNON_FLASH_ERR_VERIFY

/* lun failure */
void mark_fake_bad_lun(struct shannon_dev *sdev, int lun)
{
	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return;

	shannon_set_bit(lun, sdev->fake_bad_lun);
}

void mark_fake_cmd_timeout_lun(struct shannon_dev *sdev, int lun)
{
	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return;

	shannon_set_bit(lun, sdev->fake_cmd_timeout_lun);
}

int is_fake_bad_lun(struct shannon_dev *sdev, int lun)
{
	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return 0;

	return shannon_test_bit(lun, sdev->fake_bad_lun);
}

int is_fake_cmd_timeout_lun(struct shannon_dev *sdev, int lun)
{
	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return 0;

	return shannon_test_bit(lun, sdev->fake_cmd_timeout_lun);
}

/* lun failure for read */
void mark_fake_rd_bad_lun(struct shannon_dev *sdev, int lun)
{
	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return;

	shannon_set_bit(lun, sdev->fake_rd_bad_lun);
}

int is_fake_rd_bad_lun(struct shannon_dev *sdev, int lun)
{
	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return 0;

	return shannon_test_bit(lun, sdev->fake_rd_bad_lun);
}

/* write failure */
void mark_fake_wr_bad_lunppa(struct shannon_dev *sdev, int lun, int ppa)
{
	unsigned long offset, bit_offset;
	void *bit_base;

	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return;

	offset = (unsigned long)lun * sdev->sb_count * sdev->planes * sdev->pages_in_eblock + ppa;
	bit_offset = offset % 64;
	bit_base = (u64 *)sdev->fake_wr_bad_lunppa + offset/64;

	shannon_set_bit(bit_offset, bit_base);
}

int is_fake_wr_bad_lunppa(struct shannon_dev *sdev, int lun, int ppa)
{
	unsigned long offset, bit_offset;
	void *bit_base;
	int blk_index = ppa / sdev->pages_in_eblock;

	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return 0;

	offset = (unsigned long)lun * sdev->sb_count * sdev->planes * sdev->pages_in_eblock + ppa;
	bit_offset = offset % 64;
	bit_base = (u64 *)sdev->fake_wr_bad_lunppa + offset/64;

	if (shannon_test_bit(bit_offset, bit_base))
		return 1;

	if (is_fake_bad_lun(sdev, lun)) {
		if ((blk_index < sdev->mbr_eblocks) && sdev->fake_bad_lun_skipmbr)
			return 0;
		else
			return 1;
	} else
		return 0;
}

/* read failure */
void mark_fake_rd_bad_lunpba(struct shannon_dev *sdev, int lun, int pba)
{
	unsigned long offset, bit_offset;
	void *bit_base;

	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return;

	offset = (unsigned long)lun * sdev->sb_count * sdev->logicbs_in_sibling_eblock + pba;
	bit_offset = offset % 64;
	bit_base = (u64 *)sdev->fake_rd_bad_lunpba + offset/64;

	shannon_set_bit(bit_offset, bit_base);
}

int is_fake_rd_bad_lunpba(struct shannon_dev *sdev, int lun, int pba)
{
	unsigned long offset, bit_offset;
	void *bit_base;
	int blk_index = (pba / sdev->logicbs_in_page) / sdev->pages_in_eblock;

	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return 0;

	offset = (unsigned long)lun * sdev->sb_count * sdev->logicbs_in_sibling_eblock + pba;
	bit_offset = offset % 64;
	bit_base = (u64 *)sdev->fake_rd_bad_lunpba + offset/64;

	if (shannon_test_bit(bit_offset, bit_base))
		return 1;

	if (is_fake_bad_lun(sdev, lun)) {
		if ((blk_index < sdev->mbr_eblocks) && sdev->fake_bad_lun_skipmbr)
			return 0;
		else
			return 1;
	} else if (is_fake_rd_bad_lun(sdev, lun)) {
		if ((blk_index < sdev->mbr_eblocks) && sdev->fake_bad_lun_skipmbr)
			return 0;
		else
			return 1;
	} else
		return 0;
}

void mark_fake_rd_bad_lunpba_raidmutex(struct shannon_dev *sdev, int lun, int pba)
{
	int i;
	int beginlun = (lun / sdev->max_luns_in_group) * sdev->max_luns_in_group;
	int endlun = beginlun + sdev->max_luns_in_group;

	for (i = beginlun; i < endlun; i++)
		if (is_fake_rd_bad_lunpba(sdev, i, pba))
			return;

	mark_fake_rd_bad_lunpba(sdev, lun, pba);
}

/* erase failure */
void mark_fake_er_bad_block(struct shannon_dev *sdev, int lun, int blk)
{
	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return;

	shannon_set_bit(lun * sdev->eblocks_in_lun + blk, sdev->fake_er_bad_block);
}

int is_fake_er_bad_block(struct shannon_dev *sdev, int lun, int blk)
{
	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return 0;

	if (shannon_test_bit(lun * sdev->eblocks_in_lun + blk, sdev->fake_er_bad_block))
		return 1;

	if (is_fake_bad_lun(sdev, lun)) {
		if ((blk < sdev->mbr_eblocks) && sdev->fake_bad_lun_skipmbr)
			return 0;
		else
			return 1;
	} else
		return 0;
}

/* twin read failure */
void mark_fake_twin_rd_bad_lunpba(struct shannon_dev *sdev, int lun, int pba, int head_index)
{
	unsigned long offset, bit_offset, base_offset;
	void *bit_base;

	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return;

	pba = pba % sdev->logicbs_in_sibling_eblock;
	offset = (unsigned long)lun * sdev->logicbs_in_sibling_eblock + pba;
	bit_offset = offset % 64;
	base_offset = (head_index == 0) ? 0 : ((unsigned long)sdev->lun_count * sdev->logicbs_in_sibling_eblock/64);
	bit_base = (u64 *)sdev->fake_twin_rd_bad_lunpba + base_offset + offset/64;

	shannon_set_bit(bit_offset, bit_base);
}

int is_fake_twin_rd_bad_lunpba(struct shannon_dev *sdev, int lun, int pba, int head_index)
{
	unsigned long offset, bit_offset, base_offset;
	void *bit_base;

	if (!shannon_atomic_read(&sdev->rwe_err_buf_done))
		return 0;

	if (sdev->twin_read_err == 0)
		return 0;

	pba = pba % sdev->logicbs_in_sibling_eblock;
	offset = (unsigned long)lun * sdev->logicbs_in_sibling_eblock + pba;
	bit_offset = offset % 64;
	base_offset = (head_index == 0) ? 0 : ((unsigned long)sdev->lun_count * sdev->logicbs_in_sibling_eblock/64);
	bit_base = (u64 *)sdev->fake_twin_rd_bad_lunpba + base_offset + offset/64;

	if (shannon_test_bit(bit_offset, bit_base))
		return 1;

	return 0;
}

static inline unsigned get_random(unsigned min, unsigned max)
{
	unsigned rand;

	if (min >= max)
		return max;

	shannon_get_random_bytes(&rand, sizeof(rand));
	rand = rand % (max - min + 1) + min;
	return rand;
}

void update_fake_pattern(struct shannon_dev *sdev, int next_sb, int head_index)
{
	int ch, pl, step;
	int luns[2], lun, luns_needed = 2;
	struct shannon_sb *sb;
	struct sub_group *group;
	int limit;
	int grp, offset, sector, pba;
	u64 luns_mask[4]; /* max luns in groups is 256 */

	if (sdev->twin_read_err == 0)
		return;

	sb = sdev->sbs + next_sb;

	/* select a valid raid group */
	grp = get_random(0, sdev->parity_groups - 1);
	limit = 0;
	group = &sb->sub_group[grp];
	while (group->phy_index < 0 || shannon_atomic_read(&group->available_luns) < luns_needed) {
		if (++limit >= sdev->parity_groups) {
			shannon_info("No valid group for twin_read_err found in sb_index=%d\n", sb->sb_index);
			return;
		}
		grp = (grp + 1) % sdev->parity_groups;
		group = &sb->sub_group[grp];
	}
	shannon_info("%s: simulate read errors at sb_index=%d,group=%d,head=%d,", __func__, sb->sb_index, grp, head_index);

	/* select valid luns, parity lun is also included */
	shannon_memset(luns_mask, 0, sizeof(luns_mask));
	limit = 0;
	for (lun = 0; lun < luns_needed; lun++) {
		offset = get_random(0, sdev->max_luns_in_group - 1);

		while (is_bad_lun(sb, group->start_lun + offset) || shannon_test_and_set_bit(offset%64, (void *)((u64 *)luns_mask + offset/64))) {
			if (++limit >= sdev->max_luns_in_group) {
				shannon_info("No valid luns for twin_read_err found in group=%d, sb_index=%d\n", grp, sb->sb_index);
				return;
			}
			offset = (offset + 1) % sdev->max_luns_in_group;
		}

		luns[lun] = (group->start_lun + offset) % sdev->max_luns_in_group;
		limit = 0;
	}
	shannon_info("lun0=%d,lun1=%d,\n", luns[0], luns[1]);

	shannon_info("pba list:\n");
	/* select sectors */
	step = get_random(0, sdev->shared_pages - 1);
	for (ch = step; ch < sdev->mbr.pages_in_eblock; ch += step) {
		step = get_random(0, sdev->shared_pages * 4);
		step = get_random(step, sdev->shared_pages * 8) ? : 1;
		pl = (sdev->planes == 1) ? 0 : get_random(0, sdev->planes - 1);
		sector = get_random(0, sdev->logicbs_in_page - 1);

		pba = ch * sdev->logicbs_in_page + pl * sdev->logicbs_in_eblock + sector;
		mark_fake_twin_rd_bad_lunpba(sdev, luns[0], pba, head_index);
		mark_fake_twin_rd_bad_lunpba(sdev, luns[1], pba, head_index);
		shannon_info("%d ", pba);
	}

	return;
}

/* init */
int init_err_injection(struct shannon_dev *sdev)
{
	int fake_bad_lun_size, fake_rd_bad_lunpba_size, fake_wr_bad_lunppa_size, fake_er_bad_block_size, fake_twin_rd_bad_lunpba_size;

	shannon_info("%s: pages_in_eblock=%d, lun_count=%d, sb_count=%d, logicbs_in_sibling_eblock=%d, planes=%d.\n",
			sdev->cdev_name, sdev->pages_in_eblock, sdev->lun_count, sdev->sb_count, sdev->logicbs_in_sibling_eblock, sdev->planes);

	fake_bad_lun_size = (((unsigned long)sdev->lun_count + 8 * sizeof(long) - 1) / (8 * sizeof(long))) * sizeof(long);
	sdev->fake_bad_lun = shannon_vmalloc(fake_bad_lun_size);
	if (NULL == sdev->fake_bad_lun) {
		shannon_err_dev(sdev, "Allocate memory length=%ld for fake_bad_lun failed!\n", fake_bad_lun_size);
		goto free;
	}
	shannon_memset(sdev->fake_bad_lun, 0x00, fake_bad_lun_size);

	sdev->fake_cmd_timeout_lun = shannon_vmalloc(fake_bad_lun_size);
	if (NULL == sdev->fake_cmd_timeout_lun) {
		shannon_err_dev(sdev, "Allocate memory length=%ld for fake_cmd_timeout_lun failed!\n", fake_bad_lun_size);
		goto free;
	}
	shannon_memset(sdev->fake_cmd_timeout_lun, 0x00, fake_bad_lun_size);

	sdev->fake_rd_bad_lun = shannon_vmalloc(fake_bad_lun_size);
	if (NULL == sdev->fake_rd_bad_lun) {
		shannon_err_dev(sdev, "Allocate memory length=%ld for fake_rd_bad_lun failed!\n", fake_bad_lun_size);
		goto free;
	}
	shannon_memset(sdev->fake_rd_bad_lun, 0x00, fake_bad_lun_size);

	fake_rd_bad_lunpba_size = ((unsigned long)sdev->lun_count * sdev->sb_count * sdev->logicbs_in_sibling_eblock/8UL + 4095) & ~4095UL;
	sdev->fake_rd_bad_lunpba = shannon_vmalloc(fake_rd_bad_lunpba_size);
	if (NULL == sdev->fake_rd_bad_lunpba) {
		shannon_err_dev(sdev, "Allocate memory length=%ld, for fake_rd_bad_lunpba failed!\n", fake_rd_bad_lunpba_size);
		goto free;
	}
	shannon_memset(sdev->fake_rd_bad_lunpba, 0, fake_rd_bad_lunpba_size);

	fake_wr_bad_lunppa_size = ((unsigned long)sdev->pages_in_eblock * sdev->planes * sdev->sb_count * sdev->lun_count/8UL + 4095) & ~4095;
	sdev->fake_wr_bad_lunppa = shannon_vmalloc(fake_wr_bad_lunppa_size);
	if (NULL == sdev->fake_wr_bad_lunppa) {
		shannon_err_dev(sdev, "Allocate memory length=%ld for fake_wr_bad_lunppa failed!\n", fake_wr_bad_lunppa_size);
		goto free;
	}
	shannon_memset(sdev->fake_wr_bad_lunppa, 0x00, fake_wr_bad_lunppa_size);

	fake_er_bad_block_size = (((unsigned long)sdev->eblocks_in_lun * sdev->lun_count + 7)/8UL + 4095) & ~4095;
	sdev->fake_er_bad_block = shannon_vmalloc(fake_er_bad_block_size);
	if (NULL == sdev->fake_er_bad_block) {
		shannon_err_dev(sdev, "Allocate memory length=%ld for fake_er_bad_block!\n", fake_er_bad_block_size);
		goto free;
	}
	shannon_memset(sdev->fake_er_bad_block, 0x00, fake_er_bad_block_size);

	/* each for the hot and cold wr_sb */
	fake_twin_rd_bad_lunpba_size = ((unsigned long)sdev->lun_count * sdev->logicbs_in_sibling_eblock * 2/8UL + 4095) & ~4095UL;
	sdev->fake_twin_rd_bad_lunpba = shannon_vmalloc(fake_twin_rd_bad_lunpba_size);
	if (NULL == sdev->fake_twin_rd_bad_lunpba) {
		shannon_err_dev(sdev, "Allocate memory length=%ld, for fake_twin_rd_bad_lunpba failed!\n", fake_twin_rd_bad_lunpba_size);
		goto free;
	}
	shannon_memset(sdev->fake_twin_rd_bad_lunpba, 0x00, fake_twin_rd_bad_lunpba_size);

	shannon_atomic_set(&sdev->rwe_err_buf_done, 1);

	return 0;
free:
	if (sdev->fake_er_bad_block)
		shannon_vfree(sdev->fake_er_bad_block);
	if (sdev->fake_wr_bad_lunppa)
		shannon_vfree(sdev->fake_wr_bad_lunppa);
	if (sdev->fake_rd_bad_lunpba)
		shannon_vfree(sdev->fake_rd_bad_lunpba);
	if (sdev->fake_rd_bad_lun)
		shannon_vfree(sdev->fake_rd_bad_lun);
	if (sdev->fake_bad_lun)
		shannon_vfree(sdev->fake_bad_lun);
	if (sdev->fake_cmd_timeout_lun)
		shannon_vfree(sdev->fake_cmd_timeout_lun);
	if (sdev->fake_twin_rd_bad_lunpba)
		shannon_vfree(sdev->fake_twin_rd_bad_lunpba);

	return -ENOMEM;
}

void release_err_injection(struct shannon_dev *sdev)
{
	shannon_vfree(sdev->fake_bad_lun);
	shannon_vfree(sdev->fake_cmd_timeout_lun);
	shannon_vfree(sdev->fake_rd_bad_lun);
	shannon_vfree(sdev->fake_rd_bad_lunpba);
	shannon_vfree(sdev->fake_wr_bad_lunppa);
	shannon_vfree(sdev->fake_er_bad_block);
	shannon_vfree(sdev->fake_twin_rd_bad_lunpba);
}
#else
inline int init_err_injection(struct shannon_dev *sdev)
{
	return 0;
}

void release_err_injection(struct shannon_dev *sdev) {}
#endif
