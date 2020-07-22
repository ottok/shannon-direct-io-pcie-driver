#ifndef __SHANNON_KCORE_H
#define __SHANNON_KCORE_H


/*
 * Here is the most fundamental linux header files. To ensure portabilty,
 * these headers must just contain macros and inline functions.
 * Risk: proprietary pre-compiled .o_shipped includes these macros from
 * pre-compilation platform, if re-compilation platform has a different
 * definition of these, will cause serious errors. Hopefully, we aim at
 * portabilty just in x86_64 linux platforms, so it may works well.
 * TODO: remove these header files, provide cross platform portabilty.
 */
#include <linux/types.h>
#include <linux/gfp.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/highmem.h>
#include <linux/topology.h>
#include "shannon_list.h"

#define RESERVE_MEM(bytes) char mem[bytes] __attribute__ ((aligned(8)))

#define SHANNON_DEFAULT_RATELIMIT_INTERVAL	(5 * HZ)
#define SHANNON_DEFAULT_RATELIMIT_BURST		500
#define SHANNON_DEFAULT_RECOVER_RATELIMIT_INTERVAL	(5 * HZ)
#define SHANNON_DEFAULT_RECOVER_RATELIMIT_BURST		1000


//  @BEGIN of spinlock wrapper
//  spinlock.h
struct __shannon_spinlock {
//	reserve enough mem space for all spinlock implementations
	RESERVE_MEM(128);
};

struct __shannon_rwlock {
	RESERVE_MEM(128);
};

typedef struct __shannon_spinlock shannon_spinlock_t;
typedef struct __shannon_rwlock shannon_rwlock_t;

#ifdef CONFIG_PROVE_LOCKING

#define shannon_spin_lock_init(_lock)				\
	do {							\
		spin_lock_init((spinlock_t *)_lock);		\
	} while (0)

#define shannon_rwlock_init(_lock)				\
	do {							\
		rwlock_init((rwlock_t *)_lock);		\
	} while (0)

#define shannon_spin_lock(lock)         spin_lock((spinlock_t *)(lock))
#define shannon_spin_lock_bh(lock)      spin_lock_bh((spinlock_t *)(lock))
#define shannon_spin_lock_irq(lock)     spin_lock_irq((spinlock_t *)(lock))
#define shannon_spin_unlock(lock)       spin_unlock((spinlock_t *)(lock))
#define shannon_spin_unlock_bh(lock)    spin_unlock_bh((spinlock_t *)(lock))
#define shannon_spin_unlock_irq(lock)   spin_unlock_irq((spinlock_t *)(lock))
#define shannon_spin_trylock(lock)      spin_trylock((spinlock_t *)(lock))

#define shannon_read_lock(lock)         read_lock((rwlock_t *)(lock))
#define shannon_read_lock_bh(lock)      read_lock_bh((rwlock_t *)(lock))
#define shannon_read_lock_irq(lock)     read_lock_irq((rwlock_t *)(lock))
#define shannon_read_unlock(lock)       read_unlock((rwlock_t *)(lock))
#define shannon_read_unlock_bh(lock)    read_unlock_bh((rwlock_t *)(lock))
#define shannon_read_unlock_irq(lock)   read_unlock_irq((rwlock_t *)(lock))
#define shannon_read_trylock(lock)      read_trylock((rwlock_t *)(lock))

#define shannon_write_lock(lock)         write_lock((rwlock_t *)(lock))
#define shannon_write_lock_bh(lock)      write_lock_bh((rwlock_t *)(lock))
#define shannon_write_lock_irq(lock)     write_lock_irq((rwlock_t *)(lock))
#define shannon_write_unlock(lock)       write_unlock((rwlock_t *)(lock))
#define shannon_write_unlock_bh(lock)    write_unlock_bh((rwlock_t *)(lock))
#define shannon_write_unlock_irq(lock)   write_unlock_irq((rwlock_t *)(lock))
#define shannon_write_trylock(lock)      write_trylock((rwlock_t *)(lock))

#else

extern void shannon_spin_lock_init(shannon_spinlock_t *lock);
extern void shannon_spin_lock(shannon_spinlock_t *lock);
extern void shannon_spin_lock_bh(shannon_spinlock_t *lock);
extern void shannon_spin_lock_irq(shannon_spinlock_t *lock);
extern void shannon_spin_unlock(shannon_spinlock_t *lock);
extern void shannon_spin_unlock_bh(shannon_spinlock_t *lock);
extern void shannon_spin_unlock_irq(shannon_spinlock_t *lock);
extern int shannon_spin_trylock(shannon_spinlock_t *lock);
extern int shannon_spin_trylock_irq(shannon_spinlock_t *lock);

extern void shannon_rwlock_init(shannon_rwlock_t *lock);
extern void shannon_read_lock(shannon_rwlock_t *lock);
extern void shannon_read_lock_bh(shannon_rwlock_t *lock);
extern void shannon_read_lock_irq(shannon_rwlock_t *lock);
extern void shannon_read_unlock(shannon_rwlock_t *lock);
extern void shannon_read_unlock_bh(shannon_rwlock_t *lock);
extern void shannon_read_unlock_irq(shannon_rwlock_t *lock);
extern int shannon_read_trylock(shannon_rwlock_t *lock);
extern void shannon_write_lock(shannon_rwlock_t *lock);
extern void shannon_write_lock_bh(shannon_rwlock_t *lock);
extern void shannon_write_lock_irq(shannon_rwlock_t *lock);
extern void shannon_write_unlock(shannon_rwlock_t *lock);
extern void shannon_write_unlock_bh(shannon_rwlock_t *lock);
extern void shannon_write_unlock_irq(shannon_rwlock_t *lock);
extern int shannon_write_trylock(shannon_rwlock_t *lock);

#endif
extern unsigned long shannon_spin_lock_irqsave(shannon_spinlock_t *lock);
extern void shannon_spin_unlock_irqrestore(shannon_spinlock_t *lock, unsigned long flags);
//  @END of spinlock wrapper

//  @BEGIN of mutex wrapper
//  mutex.h
struct __shannon_semaphore {
//	reserve enough mem space for all semaphore implementations
	RESERVE_MEM(256);
};

typedef struct __shannon_semaphore shannon_mutex_t;

#ifdef CONFIG_PROVE_LOCKING

#define shannon_mutex_init(_lock)			\
	do {						\
		mutex_init((struct mutex *)_lock);	\
	} while (0)

#define shannon_mutex_lock(lock)        mutex_lock((struct mutex *)(lock))
#define shannon_mutex_trylock(lock)     mutex_trylock((struct mutex *)(lock))
#define shannon_mutex_unlock(lock)      mutex_unlock((struct mutex *)(lock))

#else

extern void shannon_mutex_init(shannon_mutex_t *lock);
extern void shannon_mutex_lock(shannon_mutex_t *lock);
extern void shannon_mutex_unlock(shannon_mutex_t *lock);
extern int shannon_mutex_trylock(shannon_mutex_t *lock);
#endif
//  @END of mutex wrapper

//  @BEGIN of atomic wrapper
struct __shannon_atomic {
//	reserve enough mem space for all atomic_t implementations: int in 32bit systems, long int in 64bit systems
	RESERVE_MEM(sizeof(long int));
};

typedef struct __shannon_atomic shannon_atomic_t;
typedef struct __shannon_atomic shannon_atomic64_t;

extern void shannon_atomic_set(shannon_atomic_t *v, int i);
extern void shannon_atomic_add(int i, shannon_atomic_t *v);
extern int shannon_atomic_add_return(int i, shannon_atomic_t *v);
extern void shannon_atomic_sub(int i, shannon_atomic_t *v);
extern int shannon_atomic_sub_return(int i, shannon_atomic_t *v);
extern void shannon_atomic_dec(shannon_atomic_t *v);
extern void shannon_atomic_inc(shannon_atomic_t *v);
extern int shannon_atomic_read(const shannon_atomic_t *v);
extern int shannon_atomic_dec_and_test(shannon_atomic_t *v);
extern int shannon_atomic_inc_and_test(shannon_atomic_t *v);
extern int shannon_atomic_dec_return(shannon_atomic_t *v);
extern int shannon_atomic_inc_return(shannon_atomic_t *v);
extern long int shannon_atomic64_inc_return(shannon_atomic64_t *v);


extern void shannon_atomic64_set(shannon_atomic64_t *v, long int i);
extern void shannon_atomic64_add(long int i, shannon_atomic64_t *v);
extern void shannon_atomic64_sub(long int i, shannon_atomic64_t *v);
extern void shannon_atomic64_dec(shannon_atomic64_t *v);
extern void shannon_atomic64_inc(shannon_atomic64_t *v);
extern long int shannon_atomic64_read(const shannon_atomic64_t *v);
extern long int shannon_atomic64_dec_and_test(shannon_atomic64_t *v);
extern long int shannon_atomic64_inc_and_test(shannon_atomic64_t *v);
//  @END of atomic wrapper

//  @BEGIN of bitops wrapper
extern unsigned long shannon_find_first_zero_bit(const unsigned long *addr, unsigned long size);
extern unsigned long shannon_find_first_bit_le(const void *addr, unsigned long size);
extern unsigned long shannon_find_first_bit(const void *addr, unsigned long size);
extern unsigned long shannon_find_next_bit_le(const void *addr, unsigned long size, unsigned long offset);
#define shannon_for_each_set_bit_le(bit, addr, size) \
        for ((bit) = shannon_find_first_bit_le((addr), (size));            \
             (bit) < (size);                                    \
             (bit) = shannon_find_next_bit_le((addr), (size), (bit) + 1))
extern int shannon_test_and_set_bit(int nr, volatile unsigned long *addr);
extern int shannon_test_and_clear_bit(int nr, volatile unsigned long *addr);
extern int shannon_test_and_clear_bit_le(int nr, void *addr);
extern int shannon_test_bit(int nr, const volatile unsigned long *addr);
extern int shannon_test_bit_le(int nr, const void *addr);
extern void shannon_set_bit(int nr, void *addr);
extern void shannon_set_bit_le(int nr, void *addr);
extern void shannon_clear_bit(int nr, void *addr);
//  @END of bitops wrapper

//  @BEGIN of bitmap wrapper
extern void __shannon_bitmap_xor(unsigned long *dst, const unsigned long *bitmap1, const unsigned long *bitmap2, int bits);
extern int __shannon_bitmap_weight(const void *bitmap, unsigned int bits);
extern unsigned long shannon_hweight_long(unsigned long w);
extern unsigned int shannon_hweight32(unsigned int w);
extern int shannon_get_count_order(unsigned int count);
//  @END of bitmap wrapper

//  @BEGIN of memory related functionalities wrapper
#ifndef __iomem
#define __iomem
#endif

#ifndef __user
#define __user
#endif

#ifndef __bitwise__
#define __bitwise__
#endif

typedef struct shannon_pgprot { unsigned long pgprot; } shannon_pgprot_t;
#define SHANNON_PAGE_KERNEL	(*((shannon_pgprot_t *) &PAGE_KERNEL))

typedef unsigned __bitwise__	shannon_gfp_t;
typedef unsigned __bitwise__	shannon_fmode_t;
typedef long long		shannon_loff_t;
typedef unsigned long		shannon_size_t;
typedef long			shannon_ssize_t;
typedef unsigned int		shannon_mode_t;

typedef u64 shannon_phys_addr_t;    //	long enough for 64bit systems
typedef u64 shannon_resource_size_t;    //	long enough for 64bit systems
typedef dma_addr_t shannon_dma_addr_t;

//  io.h
extern u32 shannon_raw_readl(const volatile void __iomem *addr);
extern void shannon_raw_writel(u32 value, volatile void __iomem *addr);
extern u32 shannon_readl(const volatile void __iomem *addr);
extern u32 shannon_ioread32(void __iomem *addr);
extern void shannon_writel(u32 value, volatile void __iomem *addr);
extern void shannon_iowrite32(u32 value, void __iomem *addr);
extern u16 shannon_mem_readw(volatile void *addr);
extern void shannon_mem_writew(u16 value, void *addr);
extern void shannon_cpu_to_le16s(void *addr);
extern u32 shannon_mem_readl(volatile void *addr);
extern void shannon_mem_writel(u32 value, void *addr);
extern void shannon_cpu_to_le32s(void *addr);
extern u32 shannon_cpu_to_be32(u32 val);
extern void shannon_cpu_to_le64s(void *addr);
extern u64 shannon_cpu_to_be64(u64 val);
extern u64 shannon_mem_readq(volatile void *addr);
extern void shannon_mem_writeq(u64, void *addr);
extern void __iomem *shannon_ioremap(shannon_phys_addr_t offset, unsigned long size);
extern void shannon_iounmap(volatile void __iomem *addr);
extern void shannon_memcpy_fromio(void *dest, void *source, unsigned int count);

//  uaccess.h
extern long shannon_copy_from_user(void *to, const void __user * from, unsigned long n);
extern long shannon_copy_to_user(void __user *to, const void *from, unsigned long n);

//  gfp.h
extern void shannon_free_page(unsigned long addr);
extern unsigned long __shannon_get_free_page(shannon_gfp_t gfp);

// topology.h
extern int shannon_numa_node_id(void);

// nodemask.h
#define shannon_for_each_online_node(node) for_each_online_node(node)
extern int shannon_num_online_nodes(void);

//  mm.h
typedef void shannon_page;
extern void *shannon_page_address(shannon_page *page);
extern void *shannon_virt_to_page(const void *addr);
extern int shannon_page_to_nid(shannon_page *page);

//  vmalloc.h
extern void *shannon_vzalloc(unsigned long size);
extern void *shannon_vmalloc(unsigned long size);
extern void shannon_vfree(void *addr);
extern void *shannon_vmalloc_to_page(void *vmalloc_addr);

//  slab.h
typedef void shannon_kmem_cache_t;

extern shannon_kmem_cache_t *shannon_kmem_cache_create(const char *, size_t, size_t, unsigned long, void (*)(void *));
extern void shannon_kmem_cache_destroy(shannon_kmem_cache_t *cachep);
extern void *shannon_kzalloc(size_t size, shannon_gfp_t flags);
extern void *shannon_kmalloc(size_t size, shannon_gfp_t flags);
extern void *__shannon_vmalloc(unsigned long size, shannon_gfp_t gfp_mask, shannon_pgprot_t prot);
extern void shannon_kfree(const void *);

//  mempool.h
#define GFP_SHANNON	GFP_NOIO
typedef void shannon_mempool_t;

extern shannon_mempool_t *shannon_mempool_create_kmalloc_pool(int min_nr, size_t size);
extern shannon_mempool_t *shannon_mempool_create_kmalloc_pool_node(int min_nr, size_t size, int nid);
extern shannon_mempool_t *shannon_mempool_create_slab_pool(int min_nr, shannon_kmem_cache_t *kc);
extern shannon_mempool_t *shannon_mempool_create_slab_pool_node(int min_nr, shannon_kmem_cache_t *kc, int nid);
extern void shannon_mempool_destroy(shannon_mempool_t *pool);
extern void *shannon_mempool_alloc(shannon_mempool_t *pool, shannon_gfp_t gfp_mask);
extern void shannon_mempool_free(void *element, shannon_mempool_t *pool);
extern int get_mempool_count(shannon_mempool_t *pool);

//  @END of memory related functionalities wrapper

//  @BEGIN of miscellaneous kernel functionalities wrapper
//  string.h
extern void *shannon_memset(void *s, int c, size_t n);
extern void *shannon_memcpy(void *dest, const void *src, size_t count);
extern int shannon_memcmp(const void *cs, const void *ct, size_t count);
extern size_t shannon_strnlen(const char *s, size_t count);
extern char *shannon_strncpy(char *dest, const char *src, size_t count);

//  printk.h
#ifndef KERN_ERR /* Make sure no redefinition errors */
#define KERN_EMERG	"<0>"	/* system is unusable			*/
#define KERN_ALERT	"<1>"	/* action must be taken immediately	*/
#define KERN_CRIT	"<2>"	/* critical conditions			*/
#define KERN_ERR	"<3>"	/* error conditions			*/
#define KERN_WARNING	"<4>"	/* warning conditions			*/
#define KERN_NOTICE	"<5>"	/* normal but significant condition	*/
#define KERN_INFO	"<6>"	/* informational			*/
#define KERN_DEBUG	"<7>"	/* debug-level messages			*/
#endif
extern int shannon_printk(const char *fmt, ...);
extern int shannon_printk_ratelimited(const char *fmt, ...);
extern int shannon_recover_printk_ratelimited(const char *fmt, ...);
extern int shannon_debug_level;
#define shannon_dbg(level, format, arg...)		\
	do {						\
		if (level <= shannon_debug_level)		\
			shannon_printk(KERN_ERR "shn_dbg: %s(): " format, __func__, ##arg);	\
	} while(0)

#define debugs0( ... ) shannon_dbg(0, __VA_ARGS__ )
#define debugs1( ... ) shannon_dbg(1, __VA_ARGS__ )
#define debugs2( ... ) shannon_dbg(2, __VA_ARGS__ )
#define debugs3( ... ) shannon_dbg(3, __VA_ARGS__ )
#define debugs4( ... ) shannon_dbg(4, __VA_ARGS__ )

#define shannon_info(format, arg...)			\
	shannon_printk_ratelimited(KERN_ERR "shn_info: " format, ##arg)
#define shannon_recover_info(format, arg...)			\
	shannon_recover_printk_ratelimited(KERN_ERR "shn_info: " format, ##arg)
#define shannon_log(format, arg...)			\
	shannon_printk_ratelimited(KERN_ERR "shn_log: " format, ##arg)
#define shannon_warn(format, arg...)			\
	shannon_printk_ratelimited(KERN_ERR "shn_warn: %s(): " format, __func__, ##arg)
#define shannon_alarm(format, arg...)			\
	shannon_printk(KERN_ERR "shn_alarm: %s(): " format, __func__, ##arg)
#define shannon_err(format, arg...)			\
	shannon_printk(KERN_ERR "shn_err: %s(): " format, __func__, ##arg)
#define shannon_recover_err(format, arg...)			\
	shannon_recover_printk_ratelimited(KERN_ERR "shn_err: %s(): " format, __func__, ##arg)
#define shannon_fatal(format, arg...)			\
	shannon_printk(KERN_ERR "shn_fatal: %s(): " format, __func__, ##arg)

//  kernel.h
extern char *shannon_kasprintf(shannon_gfp_t gfp, const char *fmt, ...);
extern int shannon_sprintf(char * buf, const char * fmt, ...);
extern int shannon_snprintf(char * buf, size_t size, const char * fmt, ...);
extern void shannon_dump_stack(void);
extern long shannon_simple_strtol(const char *cp, char **endp, unsigned int base);
extern int shannon_printk_ratelimit(void);

//  num_online_cpus
extern int get_num_online_cpus(void);

/* pm_qos_params.h */
#define SHANNON_PM_QOS_CPU_DMA_LATENCY		1
#define SHANNON_PM_QOS_DEFAULT_VALUE		-1

typedef void *shannon_pm_qos_request_t;

extern int shannon_pm_qos_add_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 value);
extern int shannon_pm_qos_update_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 new_value);
extern void shannon_pm_qos_remove_requirement(shannon_pm_qos_request_t *l, int qos, char *name);
extern int shannon_pm_qos_is_required(int qos);

// err.h
extern void *SHANNON_ERR_PTR(long error);
extern long SHANNON_IS_ERR(const void *ptr);
extern long SHANNON_PTR_ERR(const void *ptr);
extern long SHANNON_IS_ERR_OR_NULL(const void *ptr);

//string.h
extern size_t shannon_strlen(const char *s);
extern int shannon_strcmp(const char *cs, const char *ct);

//random.h
extern void shannon_get_random_bytes(void *buf, int bytes);

//prefetch.h
extern void shannon_prefetch(void *addr);
extern void shannon_prefetchw(void *addr);
extern void shannon_spin_lock_prefetch(void *addr);


//  @END of miscellaneous kernel functionalities wrapper

typedef void shannon_device_t;
typedef void shannon_pci_dev_t;
typedef void shannon_pci_device_id_t;
typedef void shannon_msix_entry_t;

#ifdef CONFIG_X86
#define shannon_barrier()    asm volatile("mfence":::"memory")
#else
#define shannon_barrier()    barrier()
#endif

struct shannon_dev;
struct shannon_disk;
struct shannon_namespace;
const char *get_cdev_name_safe(struct shannon_dev *sdev);
const char *get_disk_name(struct shannon_disk *sdisk);
#define shannon_err_dev(sdev, format, arg...)		\
	shannon_err("%s: " format, get_cdev_name_safe(sdev), ##arg)
#define shannon_warn_dev(sdev, format, arg...)		\
	shannon_warn("%s: " format, get_cdev_name_safe(sdev), ##arg)
#define shannon_alarm_dev(sdev, format, arg...)		\
	shannon_alarm("%s: " format, get_cdev_name_safe(sdev), ##arg)
#define shannon_err_disk(sdisk, format, arg...)		\
	shannon_err("%s: " format, get_disk_name(sdisk), ##arg)
#define shannon_warn_disk(sdisk, format, arg...)		\
	shannon_warn("%s: " format, get_disk_name(sdisk), ##arg)
#define shannon_recover_err_dev(sdev, format, arg...)		\
	shannon_recover_err("%s: " format, get_cdev_name_safe(sdev), ##arg)
#define shannon_err_ns(ns, format, arg...)			\
	shannon_err("%s: " format, get_ns_name_safe(ns), ##arg)
#define shannon_warn_ns(ns, format, arg...)			\
	shannon_warn("%s: " format, get_ns_name_safe(ns), ##arg)
#define shannon_err_pool(spool, format, arg...)			\
	shannon_err("%s: " format, get_pool_name_safe(spool), ##arg)
#define shannon_warn_pool(spool, format, arg...)			\
	shannon_warn("%s: " format, get_pool_name_safe(spool), ##arg)

#endif /* __SHANNON_KCORE_H */
