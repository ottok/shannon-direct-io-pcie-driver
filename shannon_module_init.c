#include <linux/fs.h>
#include <linux/bio.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/errno.h>
#include <linux/genhd.h>
#include <linux/kdev_t.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/time.h>
#include <linux/completion.h>
#include <linux/random.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>

#include <linux/fcntl.h>  /* O_ACCMODE */
#include <linux/hdreg.h>  /* HDIO_GETGEO */

#include <linux/cpu.h>
#include <asm/processor.h>
#ifdef CONFIG_SUSE_PRODUCT_CODE
#include <linux/suse_version.h>
#endif

#include "shannon_port.h"

#define SHANNON_CTRL_MINORS 64

int shannon_scsi_mode = 0;
module_param(shannon_scsi_mode, int, S_IRUGO|S_IWUSR);

struct shannon_dev;
struct shannon_memblock_pool;
extern int shannon_memblock_pool_init(struct shannon_memblock_pool *mpool, u32 memblock_size);
extern void shannon_memblock_pool_destroy(struct shannon_memblock_pool *mpool);
extern u64 get_shannon_dev_sectors(struct shannon_dev *dev);
extern u64 get_shannon_ns_sectors(struct shannon_namespace *ns);
extern int sh_increase_users(struct shannon_dev *dev);
extern int sh_decrease_users(struct shannon_dev *dev);
extern int sh_increase_users_ns(struct shannon_namespace *);
extern int sh_decrease_users_ns(struct shannon_namespace *);

struct shannon_list_head shannon_dev_list;
extern shannon_spinlock_t device_bitmap_lock;
shannon_workqueue_struct_t *shannon_percpu_wq = NULL;

extern struct shannon_list_head shannon_pool_list;
extern shannon_mutex_t pool_sem;
struct shannon_pool;
extern struct shannon_pool *spool_get_reference(struct shannon_pool *);
extern void spool_put_reference(struct shannon_pool *);

//  blkdev operations
static int shannon_revalidate(struct gendisk *disk)
{
	struct shannon_dev *dev = disk->private_data;
	set_capacity(disk, get_shannon_dev_sectors(dev));
	return 0;
}

static int shannon_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	struct shannon_dev *dev = bdev->bd_disk->private_data;

	/*
	 * get geometry: we have to fake one...  trim the size to a
	 * multiple of 2048 (1M): tell we have 32 sectors, 64 heads,
	 * whatever cylinders.
	 */
	geo->heads     = 32;
	geo->sectors   = 32;
	geo->cylinders = get_shannon_dev_sectors(dev) / (geo->heads * geo->sectors);
	return 0;
}

static int shannon_revalidate_ns(struct gendisk *disk)
{
	struct shannon_namespace *ns = disk->private_data;
	set_capacity(disk, get_shannon_ns_sectors(ns));
	return 0;
}

static int shannon_getgeo_ns(struct block_device *bdev, struct hd_geometry *geo)
{
	struct shannon_namespace *ns = bdev->bd_disk->private_data;

	/*
	 * get geometry: we have to fake one...  trim the size to a
	 * multiple of 2048 (1M): tell we have 32 sectors, 64 heads,
	 * whatever cylinders.
	 */
	geo->heads     = 32;
	geo->sectors   = 32;
	geo->cylinders = get_shannon_ns_sectors(ns) / (geo->heads * geo->sectors);
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)

static int shannon_open(struct block_device *bdev, fmode_t mode)
{
	struct shannon_dev *dev = bdev->bd_disk->private_data;
	return sh_increase_users(dev);
}

static int shannon_open_ns(struct block_device *bdev, fmode_t mode)
{
	struct shannon_namespace *ns = bdev->bd_disk->private_data;
	return sh_increase_users_ns(ns);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
static void shannon_release(struct gendisk *gd, fmode_t mode)
{
	struct shannon_dev *dev = gd->private_data;
	sh_decrease_users(dev);
}

static void shannon_release_ns(struct gendisk *gd, fmode_t mode)
{
	struct shannon_namespace *ns = gd->private_data;
	sh_decrease_users_ns(ns);
}
#else
static int shannon_release(struct gendisk *gd, fmode_t mode)
{
	struct shannon_dev *dev = gd->private_data;
	sh_decrease_users(dev);
	return 0;
}

static int shannon_release_ns(struct gendisk *gd, fmode_t mode)
{
	struct shannon_namespace *ns = gd->private_data;
	sh_decrease_users_ns(ns);
	return 0;
}
#endif

struct block_device_operations shannon_ops = {
	.owner		= THIS_MODULE,
	.getgeo		= shannon_getgeo,
	.revalidate_disk = shannon_revalidate,
	.open		= shannon_open,
	.release	= shannon_release,
};

#else

int shannon_open(struct inode *inode, struct file *filp)
{
	struct shannon_dev *dev = inode->i_bdev->bd_disk->private_data;
	return sh_increase_users(dev);
}

int shannon_open_ns(struct inode *inode, struct file *filp)
{
	struct shannon_namespace *ns = inode->i_bdev->bd_disk->private_data;
	return sh_increase_users_ns(ns);
}

int shannon_release(struct inode *inode, struct file *filp)
{
	struct shannon_dev *dev = inode->i_bdev->bd_disk->private_data;
	sh_decrease_users(dev);
	return 0;
}

int shannon_release_ns(struct inode *inode, struct file *filp)
{
	struct shannon_namespace *ns = inode->i_bdev->bd_disk->private_data;
	sh_decrease_users_ns(ns);
	return 0;
}

struct block_device_operations shannon_ops = {
	.owner		= THIS_MODULE,
	.getgeo		= shannon_getgeo,
	.revalidate_disk = shannon_revalidate,
	.open		= shannon_open,
	.release	= shannon_release,
};

#endif

struct block_device_operations shannon_ops_ns = {
	.owner		= THIS_MODULE,
	.getgeo		= shannon_getgeo_ns,
	.revalidate_disk = shannon_revalidate_ns,
	.open		= shannon_open_ns,
	.release	= shannon_release_ns,
};

//  control miscdevice
static int shannon_ctrl_miscdevice_open(struct inode *inode, struct file *filp)
{
	struct shannon_dev *sdev = NULL;
	int minor = iminor(inode);
	struct miscdevice *c = NULL;
	struct shannon_list_head *list;

	shannon_spin_lock_bh(&device_bitmap_lock);

	shannon_list_for_each(list, &shannon_dev_list) {
		sdev = get_shannon_dev_from_list(list);
		c = (struct miscdevice *)get_miscdevice_from_shannon_dev(sdev);
		if (c->minor == minor)
			break;
	}

	shannon_spin_unlock_bh(&device_bitmap_lock);

	if ((c == NULL) || (c->minor != minor)) {
		filp->private_data = NULL;
		return -ENODEV;
	} else {
		filp->private_data = sdev;
		return 0;
	}
}

static int shannon_ctrl_miscdevice_release(struct inode *inode, struct file *filp)
{
	return 0;
}

extern long  shannon_ioctl(shannon_file_t *filp, unsigned int cmd, unsigned long arg);

static long  shannon_ioctl_wrapper(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return shannon_ioctl(filp, cmd, arg);
}

struct file_operations shannon_ctrl_miscdevice_fops = {
	.owner		= THIS_MODULE,
	.open		= shannon_ctrl_miscdevice_open,
	.release	= shannon_ctrl_miscdevice_release,
	.unlocked_ioctl	= shannon_ioctl_wrapper,
};


static void miscdevice_init(struct miscdevice *md, char *cdev_name)
{
	memset(md, 0, sizeof(*md));
	md->minor = MISC_DYNAMIC_MINOR;
	md->name  = cdev_name;
	md->fops  = &shannon_ctrl_miscdevice_fops;
}

static int shannon_pool_miscdevice_open(struct inode *inode, struct file *filp)
{
	void *spool = NULL;
	int minor = iminor(inode);
	struct miscdevice *c = NULL;
	struct shannon_list_head *list;

	shannon_mutex_lock(&pool_sem);
	shannon_list_for_each(list, &shannon_pool_list) {
		spool = get_shannon_pool_from_list(list);
		c = (struct miscdevice *)get_miscdevice_from_shannon_pool(spool);
		if (c->minor == minor) {
			spool = spool_get_reference(spool);
			break;
		}
	}
	shannon_mutex_unlock(&pool_sem);

	if ((c == NULL) || (c->minor != minor)) {
		filp->private_data = NULL;
		return -ENODEV;
	} else {
		filp->private_data = spool;
		return 0;
	}
}

static int shannon_pool_miscdevice_release(struct inode *inode, struct file *filp)
{
	void *spool = filp->private_data;
	spool_put_reference(spool);
	return 0;
}

extern long shannon_pool_ioctl(shannon_file_t *filp, unsigned int cmd, unsigned long arg);
static long shannon_pool_ioctl_wrapper(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return shannon_pool_ioctl(filp, cmd, arg);
}

struct file_operations shannon_pool_miscdevice_fops = {
	.owner		= THIS_MODULE,
	.open		= shannon_pool_miscdevice_open,
	.release	= shannon_pool_miscdevice_release,
	.unlocked_ioctl	= shannon_pool_ioctl_wrapper,
};

static void pool_miscdevice_init(struct miscdevice *md, char *cdev_name, char *nodename)
{
	memset(md, 0, sizeof(*md));
	md->minor = MISC_DYNAMIC_MINOR;
	md->name  = cdev_name;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	md->nodename = nodename;
#endif
	md->fops  = &shannon_pool_miscdevice_fops;
}

int shannon_create_miscdevice(void *misc_dev, char *cdev_name, char *nodename, int type)
{
	struct miscdevice *misc = (struct miscdevice *)misc_dev;
	int ret;
#if defined(__VMKLNX__)
	int minor=254;
#endif

	if (type == FOR_SDEV)
		miscdevice_init(misc, cdev_name);
	else
		pool_miscdevice_init(misc, cdev_name, nodename);

#if defined(__VMKLNX__)
	do
	{
	    misc->minor = minor--;
#endif
	    ret = misc_register(misc);
#if defined(__VMKLNX__)
	}
	while (ret < 0 && minor > 0);
#endif

	return ret;
}

int shannon_destroy_miscdevice(void *misc_dev)
{
    struct miscdevice *misc = (struct miscdevice *)misc_dev;

    misc_deregister(misc);

    return 0;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
#define DEFINE_PCI_DEVICE_TABLE(_table) \
	const struct pci_device_id _table[] __devinitdata
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
#define DEFINE_PCI_DEVICE_TABLE(_table) \
	const struct pci_device_id _table[]
#endif


//  module init & exit
#define PCI_VENDOR_ID_SHANNON	0x1cb0

static struct pci_device_id shannon_id_table[] = {
#if !defined(CONFIG_SHANNON_EMU) && !defined(CONFIG_SHANNON_EMU_MODULE)
	// { PCI_DEVICE(PCI_VENDOR_ID_XILINX, 0x0007), },
	{ PCI_DEVICE(PCI_VENDOR_ID_XILINX, 0x6024), },
	{ PCI_DEVICE(PCI_VENDOR_ID_SHANNON, 0x0265), },
	{ PCI_DEVICE(PCI_VENDOR_ID_SHANNON, 0x0275), },
	{ PCI_DEVICE(PCI_VENDOR_ID_SHANNON, 0x1275), },
	{ PCI_DEVICE(PCI_VENDOR_ID_SHANNON, 0x2275), },
	{ PCI_DEVICE(PCI_VENDOR_ID_SHANNON, 0x1285), },
	{ PCI_DEVICE(PCI_VENDOR_ID_SHANNON, 0x3275), },
	{ PCI_DEVICE(PCI_VENDOR_ID_SHANNON, 0x05a5), },
	{ PCI_DEVICE(PCI_VENDOR_ID_SHANNON, 0x25a5), },
	{ PCI_DEVICE(PCI_VENDOR_ID_SHANNON, 0x35a5), },
#else
	{ PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x0774), }, /* Nvidia Audio device */
	{ PCI_DEVICE(0x8086, 0x27d8), },  /* for testing */
	{ PCI_DEVICE(0x8086, 0x1c20), },  /* for testing */
	{ PCI_DEVICE(0x1013, 0x00b8), },  /* for testing */
#endif
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, shannon_id_table);

extern int shannon_scsi_probe(struct pci_dev *pdev, const struct pci_device_id *id);
extern void shannon_scsi_remove(struct pci_dev *pdev);
extern void shannon_remove(void *data, shannon_pci_dev_t *pdev);
extern int shannon_probe(shannon_pci_dev_t *pdev, const shannon_pci_device_id_t *id, struct shannon_scsi_private *data);
extern void __shannon_pci_reset_prepare(struct pci_dev *pdev);
extern void __shannon_pci_reset_finished(struct pci_dev *pdev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
static void shannon_remove_wrapper(struct pci_dev *pdev)
#else
static void __devexit shannon_remove_wrapper(struct pci_dev *pdev)
#endif
{
	if (shannon_scsi_mode)
		shannon_scsi_remove(pdev);
	else
		shannon_remove(NULL, pdev);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
static int shannon_probe_wrapper(struct pci_dev *pdev, const struct pci_device_id *id)
#else
static int __devinit shannon_probe_wrapper(struct pci_dev *pdev, const struct pci_device_id *id)
#endif
{
	if (shannon_scsi_mode)
		return shannon_scsi_probe(pdev, id);
	else
		return shannon_probe(pdev, id, NULL);
}

void shannon_pci_reset_prepare(struct pci_dev *pdev)
{
	__shannon_pci_reset_prepare(pdev);
}

void shannon_pci_reset_finished(struct pci_dev *pdev)
{
	__shannon_pci_reset_finished(pdev);
}

void shannon_pci_reset_notify(struct pci_dev *pdev, bool prepare)
{
	if (prepare)
		shannon_pci_reset_prepare(pdev);
	else
		shannon_pci_reset_finished(pdev);
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
	#ifdef SUSE_PRODUCT_CODE
		#if SUSE_PRODUCT_CODE < SUSE_PRODUCT(1, 15, 0, 0)
			struct pci_error_handlers shannon_pci_error_handlers = {
				.reset_notify = shannon_pci_reset_notify,
			};
		#else	// SUSE_PRODUCT_CODE >= SUSE_PRODUCT(1, 15, 0, 0)
			struct pci_error_handlers shannon_pci_error_handlers = {
				.reset_prepare = shannon_pci_reset_prepare,
				.reset_done = shannon_pci_reset_finished,
			};
		#endif	// end of SUSE_PRODUCT_CODE >= SUSE_PRODUCT(1, 15, 0, 0)
	#else	// (ifndef SUSE_PRODUCT_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
			struct pci_error_handlers shannon_pci_error_handlers = {
				.reset_prepare = shannon_pci_reset_prepare,
				.reset_done = shannon_pci_reset_finished,
			};
		#else
			struct pci_error_handlers shannon_pci_error_handlers = {
				.reset_notify = shannon_pci_reset_notify,
			};
		#endif
	#endif
#else
	#ifdef RHEL_RELEASE_CODE
		#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7, 2)
			struct pci_driver_rh shannon_pci_driver_rh = {
				.size = sizeof(struct pci_driver_rh),
				.reset_notify = shannon_pci_reset_notify,
			};
		#endif
	#endif
	struct pci_error_handlers shannon_pci_error_handlers = {NULL};
#endif

static struct pci_driver shannon_driver = {
	.name           = "shannon",
	.id_table       = shannon_id_table,
	.probe          = shannon_probe_wrapper,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
	.remove         = shannon_remove_wrapper,
#else
	.remove         = __devexit_p(shannon_remove_wrapper),
#endif
	.shutdown       = shannon_remove_wrapper,
	.err_handler = &shannon_pci_error_handlers,
#ifdef RHEL_RELEASE_CODE
#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7, 2)
	.pci_driver_rh	= &shannon_pci_driver_rh,
#endif
#endif
};

extern struct shannon_list_head shannon_pool_list;
extern shannon_mutex_t pool_sem;
extern int shannon_major;
extern int shannon_auto_attach;

extern int shannon_sector_size;
module_param(shannon_sector_size, int, S_IRUGO|S_IWUSR);
extern int shannon_debug_level;
module_param(shannon_debug_level, int, S_IRUGO|S_IWUSR);
extern int shannon_force_rw;
module_param(shannon_force_rw, int, S_IRUGO|S_IWUSR);
module_param(shannon_major, int, 0);
module_param(shannon_auto_attach, int, 0);
extern int shannon_pm_qos_value;
extern int shannon_pm_qos_disable;
module_param(shannon_pm_qos_value, int, S_IRUGO|S_IWUSR);
module_param(shannon_pm_qos_disable, int, S_IRUGO|S_IWUSR);
extern int shannon_use_iosched;
module_param(shannon_use_iosched, int, S_IRUGO|S_IWUSR);
extern int shannon_buffer_write;
module_param(shannon_buffer_write, int, S_IRUGO|S_IWUSR);
extern int shannon_disable_intervel_refresh_mbr;
module_param(shannon_disable_intervel_refresh_mbr, int, S_IRUGO|S_IWUSR);
extern int shannon_never_hang;
module_param(shannon_never_hang, int, S_IRUGO|S_IWUSR);
extern int shannon_high_performance;
module_param(shannon_high_performance, int, S_IRUGO|S_IWUSR);
int shannon_use_percpu_wq;
module_param(shannon_use_percpu_wq, int, S_IRUGO|S_IWUSR);
extern int shannon_init_temp;
module_param(shannon_init_temp, int, S_IRUGO|S_IWUSR);
extern int shannon_do_pci_reset;
module_param(shannon_do_pci_reset, int, S_IRUGO|S_IWUSR);
extern int shannon_do_snapread;
module_param(shannon_do_snapread, int, S_IRUGO|S_IWUSR);
extern int shannon_memblock_prealloc;
module_param(shannon_memblock_prealloc, int, S_IRUGO|S_IWUSR);
extern int shannon_force_reclaim_activeblock;
module_param(shannon_force_reclaim_activeblock, int, S_IRUGO|S_IWUSR);
extern int shannon_load_readonly;
module_param(shannon_load_readonly, int, S_IRUGO|S_IWUSR);
extern int shannon_fast_boot_enable;
module_param(shannon_fast_boot_enable, int, S_IRUGO|S_IWUSR);
extern int shannon_skip_first_read;
module_param(shannon_skip_first_read, int, S_IRUGO|S_IWUSR);
extern int shannon_prefetch_enable;
module_param(shannon_prefetch_enable, int, S_IRUGO|S_IWUSR);
extern int shannon_high_prio_gc_thread;
module_param(shannon_high_prio_gc_thread, int, S_IRUGO|S_IWUSR);
extern int shannon_background_trim;
module_param(shannon_background_trim, int, S_IRUGO|S_IWUSR);
extern int shannon_prealloc;
module_param(shannon_prealloc, int, S_IRUGO|S_IWUSR);
extern int shannon_skip_epilog;
module_param(shannon_skip_epilog, int, S_IRUGO|S_IWUSR);

extern int shannon_use_rt_comp_thread;
module_param(shannon_use_rt_comp_thread, int, 0);

extern int shannon_vendor_cmd;
module_param(shannon_vendor_cmd, int, S_IRUGO|S_IWUSR);

extern int shannon_overlap_write;
module_param(shannon_overlap_write, int, S_IRUGO|S_IWUSR);
extern int shannon_dynamic_irq_delay;
module_param(shannon_dynamic_irq_delay, int, S_IRUGO|S_IWUSR);

extern int shannon_alloc_mempool(void);
extern void shannon_free_mempool(void);

int has_dma_delay = 0;
int check_has_dma_delay(void)
{
#ifdef CONFIG_X86
	struct cpuinfo_x86 *c = NULL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	c = &cpu_data(0);
#else
	c = &cpu_data[0];
#endif /* end of LINUX_VERSION_CODE */
	has_dma_delay = !(c->x86_vendor == X86_VENDOR_INTEL);
#ifdef CONFIG_SHANNON_DMA_REORDER
	has_dma_delay = 1;
#endif

	shannon_info("cpu vendor_id: %s, has_dma_delay: %s.\n", c->x86_vendor_id, has_dma_delay ? "yes" : "no");
#else /* else of CONFIG_X86 */
#ifdef CONFIG_SHANNON_DMA_REORDER
	has_dma_delay = 1;
#else
	has_dma_delay = 0;
#endif
#endif /*end of CONFIG_X86 */

	return 0;
}

static int __init shannon_init(void)
{
	int result = -ENOMEM;

	if (shannon_scsi_mode) {
		shannon_use_percpu_wq = 1;
		shannon_pm_qos_disable = 1;
	}

	check_has_dma_delay();

	shannon_spin_lock_init(&device_bitmap_lock);
	SHANNON_INIT_LIST_HEAD(&shannon_dev_list);
	shannon_major = register_blkdev(shannon_major, "shannon");
	SHANNON_INIT_LIST_HEAD(&shannon_dev_list);
	SHANNON_INIT_LIST_HEAD(&shannon_pool_list);
	shannon_mutex_init(&pool_sem);
	if (shannon_use_percpu_wq) {
		shannon_percpu_wq = shannon_create_workqueue("shn_percpu_wq");
		if (shannon_percpu_wq == NULL) {
			shannon_err("alloc shannon_percpu_wq workqueue failed!\n");
			goto unregister_blkdev;
		}
	}

	if (shannon_alloc_mempool())
		goto destroy_wq;

	result = pci_register_driver(&shannon_driver);
	if (result)
		goto free_mempool;

	return 0;

free_mempool:
	shannon_free_mempool();
destroy_wq:
	if (shannon_percpu_wq)
		shannon_destroy_workqueue(shannon_percpu_wq);
unregister_blkdev:
	unregister_blkdev(shannon_major, "shannon");
	return result;
}

static void __exit shannon_exit(void)
{
	pci_unregister_driver(&shannon_driver);
	shannon_free_mempool();

	if (shannon_percpu_wq)
		shannon_destroy_workqueue(shannon_percpu_wq);
	unregister_blkdev(shannon_major, "shannon");
}

MODULE_LICENSE("GPL");
MODULE_VERSION("3.3.0");
module_init(shannon_init);
module_exit(shannon_exit);
