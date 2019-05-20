/* Definitions for Shannon hardware/software interface.
 * Copyright (c) 2012, Shannon technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __SHANNON_BOOT_ERRNO_H
#define __SHANNON_BOOT_ERRNO_H

#define FIND_LAST_PAGE_STRIPE_FIRST_ERR		(0x0)
#define find_last_page_stripe_err(_err)		(FIND_LAST_PAGE_STRIPE_FIRST_ERR + (_err))

#define RECOVER_LPMT_TAIL_FIRST_ERR	(0x10)
#define recover_lpmt_tail_err(_err)	(RECOVER_LPMT_TAIL_FIRST_ERR + (_err))

#define RECOVER_LPMT_DATA_ASYNC_FIRST_ERR	(0x20)
#define recover_lpmt_data_async_err(_err)	(RECOVER_LPMT_DATA_ASYNC_FIRST_ERR + (_err))

#define HANDLE_LPMT_DATA_TAIL_FIRST_ERR			(0x30)
#define handle_lpmt_data_tail_err(_err)			(HANDLE_LPMT_DATA_TAIL_FIRST_ERR + (_err))

#define HANDLE_LPMT_DATA_HEAD_FIRST_ERR			(0x40)
#define handle_lpmt_data_head_err(_err)			(HANDLE_LPMT_DATA_HEAD_FIRST_ERR + (_err))

#define HANDLE_LPMT_FIRST_ERR				(0x50)
#define handle_lpmt_err(_err)				(HANDLE_LPMT_FIRST_ERR + (_err))

#define HANDLE_PBA_TABLE_FIRST_ERR			(0x60)
#define handle_pba_table_err(_err)			(HANDLE_PBA_TABLE_FIRST_ERR + (_err))

#define HANDLE_SB_INFO_FIRST_ERR			(0x70)
#define handle_sb_info_err(_err)			(HANDLE_SB_INFO_FIRST_ERR + (_err))

#define HANDLE_NS_INFO_FIRST_ERR			(0x80)
#define handle_ns_info_err(_err)			(HANDLE_NS_INFO_FIRST_ERR + (_err))

#define HANDLE_MEMBLOCK_INFO_FIRST_ERR		(0x90)
#define handle_memblock_info_err(_err)		(HANDLE_MEMBLOCK_INFO_FIRST_ERR + (_err))

#define CHECK_FINISHED_FIRST_ERR			(0xa0)
#define check_finished_err(_err)			(CHECK_FINISHED_FIRST_ERR + (_err))

#define GENERAL_FIRST_ERR					(0x100)
#define general_err(_err)					(GENERAL_FIRST_ERR + (_err))
#endif /* __SHANNON_BOOT_ERRNO_H */
