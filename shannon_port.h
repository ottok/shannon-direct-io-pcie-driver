#ifndef __SHANNON_PORT_H
#define __SHANNON_PORT_H

#include "shannon_kcore.h"

#include "shannon_time.h"
#include "shannon_workqueue.h"
#include "shannon_list.h"
#include "shannon_file.h"
#include "shannon_scatter.h"
#include "shannon_dma.h"
#include "shannon_pci.h"
#include "shannon_device.h"
#include "shannon_block.h"
#include "shannon_sched.h"
#include "shannon_sysfs.h"
#include "shannon_waitqueue.h"
#include "shannon_scsi.h"
#include "shannon_memblock.h"

/*
 * Place function definitions from proprietary codes to share them with open source codes.
 */
extern void *get_shannon_dev_from_list(struct shannon_list_head *list);
extern void *get_miscdevice_from_shannon_dev(struct shannon_dev *sdev);
extern void *get_shannon_pool_from_list(struct shannon_list_head *list);
extern void *get_miscdevice_from_shannon_pool(void *spool);

/*
 * Functions implemented in shannon_module_init.c.
 * TODO: Should use a seperate shannon_module_init.h
 */
extern struct shannon_list_head shannon_dev_list;
#define FOR_SDEV       0
#define FOR_POOL       1
extern int shannon_create_miscdevice(void *misc, char *cdev_name, char *nodename, int type);
extern int shannon_destroy_miscdevice(void *misc);

#endif /* __SHANNON_PORT_H */
