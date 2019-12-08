#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/version.h>
#ifdef CONFIG_SUSE_PRODUCT_CODE
#include <linux/suse_version.h>
#endif

#include "shannon_device.h"
#include "shannon_dma.h"
#include "shannon_block.h"
#include "shannon_time.h"

#define NAMEBUF_LEN 256

void *shannon_get_this_module(void)
{
	return THIS_MODULE;
}

shannon_dev_t SHANNON_MAJOR(shannon_dev_t dev)
{
	return MAJOR(dev);
}

shannon_dev_t SHANNON_MINOR(shannon_dev_t dev)
{
	return MINOR(dev);
}

shannon_dev_t SHANNON_MKDEV(shannon_dev_t ma, shannon_dev_t mi)
{
	return MKDEV(ma, mi);
}

//  cdev.h
int shannon_cdev_add(shannon_cdev_t *p, shannon_dev_t dev, unsigned count)
{
	return cdev_add((struct cdev *)p, dev, count);
}

void shannon_cdev_del(shannon_cdev_t *p)
{
	cdev_del((struct cdev *)p);
}

void shannon_cdev_init(shannon_cdev_t *cdev, const shannon_file_operations_t *fops)
{
	cdev_init((struct cdev *)cdev, (const struct file_operations *)fops);
}

void shannon_set_cdev_owner_this_module(shannon_cdev_t *cdev)
{
	((struct cdev *)cdev)->owner = THIS_MODULE;
}

extern struct file_operations debug_cdev_fops;

void shannon_init_debug_cdev(shannon_cdev_t *cdev)
{
	cdev_init((struct cdev *)cdev, &debug_cdev_fops);
}

//  device.h
shannon_device_t *shannon_device_create(shannon_class_t *class, shannon_device_t *parent,
		shannon_dev_t devt, void *drvdata, const char *fmt, ...)
{
	va_list vargs;
	char namebuf[NAMEBUF_LEN];

	va_start(vargs, fmt);
	vsnprintf(namebuf, NAMEBUF_LEN, fmt, vargs);
	va_end(vargs);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)

	return device_create((struct class *)class, (struct device *)parent, devt, drvdata, namebuf);

#else

	return device_create((struct class *)class, (struct device *)parent, devt, namebuf);

#endif
}

void shannon_device_destroy(shannon_class_t *cls, shannon_dev_t devt)
{
	device_destroy((struct class *)cls, devt);
}

shannon_class_t *shannon_class_create(shannon_module_t *owner, char *name)
{
	return class_create((struct module *)owner, name);
}

void shannon_class_destroy(shannon_class_t *cls)
{
	class_destroy((struct class *)cls);
}

//  fs.h
int shannon_alloc_chrdev_region(shannon_dev_t *dev, unsigned baseminor, unsigned count, const char *name)
{
	return alloc_chrdev_region(dev, baseminor, count, name);
}

void shannon_unregister_chrdev_region(shannon_dev_t from, unsigned count)
{
	unregister_chrdev_region(from, count);
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28)

int shannon_disk_in_flight(shannon_gendisk_t *gdt)
{
	struct gendisk *gd = (struct gendisk *)gdt;
	return atomic_read((atomic_t *)(&gd->in_flight));
}

#elif LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 28)

static inline void shannon_part_inc_in_flight(struct hd_struct *part, int rw)
{
	atomic_inc((atomic_t *)(&part->in_flight));
	if (part->partno)
		atomic_inc((atomic_t *)(&part_to_disk(part)->part0.in_flight));
}

static inline void shannon_part_dec_in_flight(struct hd_struct *part, int rw)
{
	atomic_dec((atomic_t *)(&part->in_flight));
	if (part->partno)
		atomic_dec((atomic_t *)(&part_to_disk(part)->part0.in_flight));
}

int shannon_disk_in_flight(shannon_gendisk_t *gdt)
{
	struct gendisk *gd = (struct gendisk *)gdt;
	return atomic_read((atomic_t *)(&gd->part0.in_flight));
}

#else

static inline void shannon_part_inc_in_flight(struct hd_struct *part, int rw)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#ifdef CONFIG_SMP
	atomic_inc((atomic_t *)(&part->dkstats->in_flight[rw]));
#else
	atomic_inc((atomic_t *)(&part->dkstats.in_flight[rw]));
#endif
#else
	atomic_inc((atomic_t *)(&part->in_flight[rw]));
#endif
	if (part->partno)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#ifdef CONFIG_SMP
		atomic_inc((atomic_t *)(&part_to_disk(part)->part0.dkstats->in_flight[rw]));
#else
		atomic_inc((atomic_t *)(&part_to_disk(part)->part0.dkstats.in_flight[rw]));
#endif
#else
		atomic_inc((atomic_t *)(&part_to_disk(part)->part0.in_flight[rw]));
#endif
}

static inline void shannon_part_dec_in_flight(struct hd_struct *part, int rw)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#ifdef CONFIG_SMP
	atomic_dec((atomic_t *)(&part->dkstats->in_flight[rw]));
#else
	atomic_dec((atomic_t *)(&part->dkstats.in_flight[rw]));
#endif
#else
	atomic_dec((atomic_t *)(&part->in_flight[rw]));
#endif
	if (part->partno)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#ifdef CONFIG_SMP
		atomic_dec((atomic_t *)(&part_to_disk(part)->part0.dkstats->in_flight[rw]));
#else
		atomic_dec((atomic_t *)(&part_to_disk(part)->part0.dkstats.in_flight[rw]));
#endif
#else
		atomic_dec((atomic_t *)(&part_to_disk(part)->part0.in_flight[rw]));
#endif
}

int shannon_disk_in_flight(shannon_gendisk_t *gdt)
{
	struct gendisk *gd = (struct gendisk *)gdt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)

#ifdef RHEL_RELEASE_CODE
#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7, 6)
	return atomic_read(&gd->part0.in_flight[0]) + atomic_read(&gd->part0.in_flight[1]);
#else
	return part_in_flight(&gd->part0);
#endif
#else
#ifdef SUSE_PRODUCT_CODE
#if SUSE_PRODUCT_CODE < SUSE_PRODUCT(1, 15, 0, 0)
	return part_in_flight(&gd->part0);
#else
	return atomic_read(&gd->part0.in_flight[0]) + atomic_read(&gd->part0.in_flight[1]);
#endif
#else
	return part_in_flight(&gd->part0);
#endif
#endif

#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#ifdef CONFIG_SMP
	return atomic_read((atomic_t *)(&gd->part0.dkstats->in_flight[0])) + atomic_read((atomic_t *)(&gd->part0.dkstats->in_flight[1]));
#else
	return atomic_read((atomic_t *)(&gd->part0.dkstats.in_flight[0])) + atomic_read((atomic_t *)(&gd->part0.dkstats.in_flight[1]));
#endif
#else
	return atomic_read(&gd->part0.in_flight[0]) + atomic_read(&gd->part0.in_flight[1]);
#endif
#endif
}

#endif

void shannon_start_io_acct(shannon_gendisk_t *gdt, shannon_bio_t *p)
{
	struct bio *bio = (struct bio *)p;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	int cpu, rw;
	struct gendisk *gd = (struct gendisk *)gdt;
	struct hd_struct *part;

	rw = shannon_bio_data_dir(bio);
	cpu = part_stat_lock();
	part = disk_map_sector_rcu(gd, get_bi_sector(bio));
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)

#ifdef RHEL_RELEASE_CODE
#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7, 6)
	part_round_stats(gd->queue, cpu, part);
#else
	part_round_stats(cpu, part);
#endif
#else
#ifdef SUSE_PRODUCT_CODE
#if SUSE_PRODUCT_CODE < SUSE_PRODUCT(1, 15, 0, 0)
	part_round_stats(cpu, part);
#else
	part_round_stats(gd->queue, cpu, part);
#endif
#else
	part_round_stats(cpu, part);
#endif
#endif

#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	update_io_ticks(part, get_jiffies());
#else
	part_round_stats(gd->queue, cpu, part);
#endif
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	part_stat_inc(part, ios[rw]);
	part_stat_add(part, sectors[rw], bio_sectors(bio));
#else
	part_stat_inc(cpu, part, ios[rw]);
	part_stat_add(cpu, part, sectors[rw], bio_sectors(bio));
#endif
	shannon_part_inc_in_flight(part, rw);
	part_stat_unlock();
#else
	int rw;
	struct gendisk *gd = (struct gendisk *)gdt;

	rw = shannon_bio_data_dir(bio);
	preempt_disable();
	disk_round_stats(gd);
	disk_stat_inc(gd, ios[rw]);
	disk_stat_add(gd, sectors[rw], bio_sectors(bio));
	atomic_inc((atomic_t *)(&gd->in_flight));
	preempt_enable();
#endif

}

void shannon_end_io_acct(shannon_gendisk_t *gdt, shannon_bio_t *p, unsigned long duration)
{
	struct bio *bio = (struct bio *)p;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	int cpu, rw;
	struct gendisk *gd = (struct gendisk *)gdt;
	struct hd_struct *part;

	rw = shannon_bio_data_dir(bio);
	cpu = part_stat_lock();
	part = disk_map_sector_rcu(gd, get_bi_sector(bio));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	update_io_ticks(part, get_jiffies());
	part_stat_add(part, nsecs[rw], shannon_jiffies_to_usecs(duration) * 1000);
	part_stat_add(part, time_in_queue, duration);
#else
	part_stat_add(cpu, part, nsecs[rw], shannon_jiffies_to_usecs(duration) * 1000);
#endif
#else
	part_stat_add(cpu, part, ticks[rw], duration);
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)

#ifdef RHEL_RELEASE_CODE
#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7, 6)
	part_round_stats(gd->queue, cpu, part);
#else
	part_round_stats(cpu, part);
#endif
#else
#ifdef SUSE_PRODUCT_CODE
#if SUSE_PRODUCT_CODE < SUSE_PRODUCT(1, 15, 0, 0)
	part_round_stats(cpu, part);
#else
	part_round_stats(gd->queue, cpu, part);
#endif
#else
	part_round_stats(cpu, part);
#endif
#endif

#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
	part_round_stats(gd->queue, cpu, part);
#endif
#endif
	shannon_part_dec_in_flight(part, rw);
	part_stat_unlock();
#else
	int rw;
	struct gendisk *gd = (struct gendisk *)gdt;

	rw = shannon_bio_data_dir(bio);
	preempt_disable();
	disk_stat_add(gd, ticks[rw], duration);
	disk_round_stats(gd);
	atomic_dec((atomic_t *)(&gd->in_flight));
	preempt_enable();
#endif
}

unsigned long shannon_read_sectors(shannon_gendisk_t *gdt)
{
	struct gendisk *gd = (struct gendisk *)gdt;

	if (gd == NULL)
		return 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	return part_stat_read(&gd->part0, sectors[READ]);
#else
	return disk_stat_read(gd, sectors[READ]);
#endif
}

unsigned long shannon_write_sectors(shannon_gendisk_t *gdt)
{
	struct gendisk *gd = (struct gendisk *)gdt;

	if (gd == NULL)
		return 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	return part_stat_read(&gd->part0, sectors[WRITE]);
#else
	return disk_stat_read(gd, sectors[WRITE]);
#endif
}

unsigned long shannon_read_ios(shannon_gendisk_t *gdt)
{
	struct gendisk *gd = (struct gendisk *)gdt;

	if (gd == NULL)
		return 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	return part_stat_read(&gd->part0, ios[READ]);
#else
	return disk_stat_read(gd, ios[READ]);
#endif
}

unsigned long shannon_write_ios(shannon_gendisk_t *gdt)
{
	struct gendisk *gd = (struct gendisk *)gdt;

	if (gd == NULL)
		return 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	return part_stat_read(&gd->part0, ios[WRITE]);
#else
	return disk_stat_read(gd, ios[WRITE]);
#endif
}

unsigned long shannon_read_msecs(shannon_gendisk_t *gdt)
{
	struct gendisk *gd = (struct gendisk *)gdt;

	if (gd == NULL)
		return 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	return part_stat_read(&gd->part0, nsecs[READ])/1000000;
#else
	return jiffies_to_msecs(part_stat_read(&gd->part0, ticks[READ]));
#endif
#else
	return jiffies_to_msecs(disk_stat_read(gd, ticks[READ]));
#endif
}

unsigned long shannon_write_msecs(shannon_gendisk_t *gdt)
{
	struct gendisk *gd = (struct gendisk *)gdt;

	if (gd == NULL)
		return 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	return part_stat_read(&gd->part0, nsecs[WRITE])/1000000;
#else
	return jiffies_to_msecs(part_stat_read(&gd->part0, ticks[WRITE]));
#endif
#else
	return jiffies_to_msecs(disk_stat_read(gd, ticks[WRITE]));
#endif
}
