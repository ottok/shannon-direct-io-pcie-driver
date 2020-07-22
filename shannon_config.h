#ifndef SHANNON_RELEASE
#define SHANNON_RELEASE
#endif
/*
 * Header file for configuration switches.
 * Copyright (C) 2014 Shannon Technology
 */

#ifndef __SHANNON_CONFIG_H
#define __SHANNON_CONFIG_H

//"Shannon DEBUG Helpers"
#define CONFIG_SHANNON_DEBUG

/*
 * More detailed switches depend on CONFIG_SHANNON_DEBUG
 */
#if defined(CONFIG_SHANNON_DEBUG)

//some platforms only support 64 bit coherent DMA
//#define	CONFIG_SHANNON_DMA_QUEUE_64BIT

//some platforms maybe reorder DMA package
#define	CONFIG_SHANNON_DMA_REORDER

//verify config
//#define	CONFIG_SHANNON_FLASH_ERR_VERIFY
//#define	CONFIG_SHANNON_PLVERIFY

//"Shannon DEBUG Character Devices"
//Remember to turn off when build releases
#ifndef SHANNON_RELEASE
#define CONFIG_SHANNON_DEBUG_CDEV
#define	CONFIG_SHANNON_DEBUG_DUMP

//"Modify Erase Count"
//Modify the erase count of a super block.
//Default: off
//#define CONFIG_SHANNON_MOD_ERASE_COUNT

//"Show outstanding requests in debugfs"
#define CONFIG_SHANNON_DEBUG_REQS

#define CONFIG_SINGLE_HEAD_VERIFY
#endif

//"Record PBA table in GC"
//Record a block's pba table when starting reclaim it, and show the pba table in debugfs.
//Default:off
//#define CONFIG_RECORD_PBA_TABLE_IN_GC

//"Shannon Statistics Information"
//Default:off
//#define CONFIG_SHANNON_STATISTICS

//"Shannon Epilog Character Devices"
//This feature needs too much memory! Default:off
//#define CONFIG_SHANNON_EPILOG_CDEV

//"Shannon debug function for flash write/read/erase failure verify"
//Default:off
//#define CONFIG_SHANNON_FLASH_ERR_VERIFY

//"Shannon debug function for atomic write feature verity"
//Default:off
//#define CONFIG_SHANNON_ATOMIC_WRITE_VERIFY

#endif /* defined(CONFIG_SHANNON_DEBUG) */

#define CONFIG_READ_SKIP_BAD_BLOCKS

/*
 * More detailed switches depend on CONFIG_SHANNON_DEBUG_CDEV
 */
#if defined(CONFIG_SHANNON_DEBUG_CDEV)

//"Shannon Verbose DEBUG Helpers"
//Default:off
//#define CONFIG_SHANNON_VERBOSE_DEBUG

#endif /* defined(CONFIG_SHANNON_DEBUG_CDEV) */

//"Buffer Write Verify"
//Read check after buffered write.
//Default:off
//#define CONFIG_SHANNON_BUFFER_WRITE_VERIFY

/*
 * GC Method Choice
 * Should chose one only, default: CONFIG_SHANNON_GC_BALANCE
 */

//#define CONFIG_SHANNON_GC_GREEDY
#define CONFIG_SHANNON_GC_BALANCE

/*
 * Fast boot data verify
 * Default:off
 */
//#define CONFIG_SHANNON_FAST_BOOT_VERIFY

#endif /* __SHANNON_CONFIG_H */
