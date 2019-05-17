#ifndef __SHANNON_DEVICE_H
#define __SHANNON_DEVICE_H

#include "shannon_kcore.h"
#include "shannon_workqueue.h"
#include "shannon_block.h"
#include "shannon_file.h"

struct __shannon_cdev {
	RESERVE_MEM(256);   // 144bits in 2.6.18, 104bits in 2.6.32
};
typedef struct __shannon_cdev shannon_cdev_t;

struct __shannon_miscdevice {
    RESERVE_MEM(256);
};
typedef struct __shannon_miscdevice shannon_miscdevice_t;

extern struct file_operations shannon_ctrl_cdev_fops;
extern struct file_operations debug_cdev_fops;

struct debug_cdev {
#define SCATTER_MEMBLOCK_TYPE	(0x11)
#define NORMAL_TYPE				(0x22)
	char type;
	int minor;
	char *buf;
	unsigned long size;
	shannon_cdev_t cdev;
};

typedef void shannon_class_t;
typedef void shannon_module_t;

typedef unsigned int shannon_dev_t;

extern void *shannon_get_this_module(void);
shannon_dev_t SHANNON_MAJOR(shannon_dev_t dev);
shannon_dev_t SHANNON_MINOR(shannon_dev_t dev);
shannon_dev_t SHANNON_MKDEV(shannon_dev_t ma, shannon_dev_t mi);

//  device.h
extern shannon_device_t *shannon_device_create(shannon_class_t *class, shannon_device_t *parent,
		shannon_dev_t devt, void *drvdata, const char *fmt, ...);
extern void shannon_device_destroy(shannon_class_t *cls, shannon_dev_t devt);
extern shannon_class_t *shannon_class_create(shannon_module_t *owner, char *name);
extern void shannon_class_destroy(shannon_class_t *cls);

//  cdev.h
extern int shannon_cdev_add(shannon_cdev_t *p, dev_t dev, unsigned count);
extern void shannon_cdev_del(shannon_cdev_t *p);
extern void shannon_cdev_init(shannon_cdev_t *cdev, const shannon_file_operations_t *fops);
extern void shannon_init_debug_cdev(shannon_cdev_t *cdev);
extern void shannon_set_cdev_owner_this_module(shannon_cdev_t *cdev);


//  fs.h
extern int shannon_alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name);
extern void shannon_unregister_chrdev_region(dev_t from, unsigned count);

// disk statistics
extern int shannon_disk_in_flight(shannon_gendisk_t *gdt);
extern void shannon_start_io_acct(shannon_gendisk_t *gdt, shannon_bio_t *bio);
extern void shannon_end_io_acct(shannon_gendisk_t *gdt, shannon_bio_t *bio, unsigned long duration);
extern unsigned long shannon_read_sectors(shannon_gendisk_t *gdt);
extern unsigned long shannon_write_sectors(shannon_gendisk_t *gdt);
extern unsigned long shannon_read_ios(shannon_gendisk_t *gdt);
extern unsigned long shannon_write_ios(shannon_gendisk_t *gdt);
extern unsigned long shannon_read_msecs(shannon_gendisk_t *gdt);
extern unsigned long shannon_write_msecs(shannon_gendisk_t *gdt);

#endif /* __SHANNON_DEVICE_H */
