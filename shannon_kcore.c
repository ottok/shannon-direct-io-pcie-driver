#include "shannon_kcore.h"
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/bitops.h>
#include <linux/bitmap.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/mempool.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/random.h>
#include <linux/prefetch.h>

/*  @BEGIN of spinlock wrapper */
#ifndef CONFIG_PROVE_LOCKING

void shannon_spin_lock_init(shannon_spinlock_t *lock)
{
	spin_lock_init((spinlock_t *)lock);
}

void shannon_spin_lock(shannon_spinlock_t *lock)
{
	spin_lock((spinlock_t *)lock);
}

void shannon_spin_lock_bh(shannon_spinlock_t *lock)
{
	spin_lock_bh((spinlock_t *)lock);
}

void shannon_spin_lock_irq(shannon_spinlock_t *lock)
{
	spin_lock_irq((spinlock_t *)lock);
}

unsigned long shannon_spin_lock_irqsave(shannon_spinlock_t *lock)
{
	unsigned long flags;
	spin_lock_irqsave((spinlock_t *)lock, flags);
	return flags;
}

void shannon_spin_unlock(shannon_spinlock_t *lock)
{
	spin_unlock((spinlock_t *)lock);
}

void shannon_spin_unlock_bh(shannon_spinlock_t *lock)
{
	spin_unlock_bh((spinlock_t *)lock);
}

void shannon_spin_unlock_irq(shannon_spinlock_t *lock)
{
	spin_unlock_irq((spinlock_t *)lock);
}

void shannon_spin_unlock_irqrestore(shannon_spinlock_t *lock, unsigned long flags)
{
	spin_unlock_irqrestore((spinlock_t *)lock, flags);
}

int shannon_spin_trylock(shannon_spinlock_t *lock)
{
	return spin_trylock((spinlock_t *)lock);
}

int shannon_spin_trylock_irq(shannon_spinlock_t *lock)
{
	return spin_trylock_irq((spinlock_t *)lock);
}

void shannon_rwlock_init(shannon_rwlock_t *lock)
{
	rwlock_init((rwlock_t *)lock);
}

void shannon_read_lock(shannon_rwlock_t *lock)
{
	read_lock((rwlock_t *)lock);
}

void shannon_read_lock_bh(shannon_rwlock_t *lock)
{
	read_lock_bh((rwlock_t *)lock);
}

void shannon_read_lock_irq(shannon_rwlock_t *lock)
{
	read_lock_irq((rwlock_t *)lock);
}

unsigned long shannon_read_lock_irqsave(shannon_rwlock_t *lock)
{
	unsigned long flags;
	read_lock_irqsave((rwlock_t *)lock, flags);
	return flags;
}

void shannon_read_unlock(shannon_rwlock_t *lock)
{
	read_unlock((rwlock_t *)lock);
}

void shannon_read_unlock_bh(shannon_rwlock_t *lock)
{
	read_unlock_bh((rwlock_t *)lock);
}

void shannon_read_unlock_irq(shannon_rwlock_t *lock)
{
	read_unlock_irq((rwlock_t *)lock);
}

void shannon_read_unlock_irqrestore(shannon_rwlock_t *lock, unsigned long flags)
{
	read_unlock_irqrestore((rwlock_t *)lock, flags);
}

int shannon_read_trylock(shannon_rwlock_t *lock)
{
	return read_trylock((rwlock_t *)lock);
}

void shannon_write_lock(shannon_rwlock_t *lock)
{
	write_lock((rwlock_t *)lock);
}

void shannon_write_lock_bh(shannon_rwlock_t *lock)
{
	write_lock_bh((rwlock_t *)lock);
}

void shannon_write_lock_irq(shannon_rwlock_t *lock)
{
	write_lock_irq((rwlock_t *)lock);
}

unsigned long shannon_write_lock_irqsave(shannon_rwlock_t *lock)
{
	unsigned long flags;
	write_lock_irqsave((rwlock_t *)lock, flags);
	return flags;
}

void shannon_write_unlock(shannon_rwlock_t *lock)
{
	write_unlock((rwlock_t *)lock);
}

void shannon_write_unlock_bh(shannon_rwlock_t *lock)
{
	write_unlock_bh((rwlock_t *)lock);
}

void shannon_write_unlock_irq(shannon_rwlock_t *lock)
{
	write_unlock_irq((rwlock_t *)lock);
}

void shannon_write_unlock_irqrestore(shannon_rwlock_t *lock, unsigned long flags)
{
	write_unlock_irqrestore((rwlock_t *)lock, flags);
}

int shannon_write_trylock(shannon_rwlock_t *lock)
{
	return write_trylock((rwlock_t *)lock);
}

#endif /* end of CONFIG_PROVE_LOCKING */

/*  @END of spinlock wrapper */
#if defined(__LITTLE_ENDIAN)
#define	BITOP_LE_MAGIC	0
#elif defined(__BIG_ENDIAN)
#define	BITOP_LE_MAGIC	((64-1) & ~0x7)
#endif

/*  @BEGIN of mutex wrapper */
#ifndef CONFIG_PROVE_LOCKING
void shannon_mutex_init(shannon_mutex_t *lock)
{
	mutex_init((struct mutex *)lock);
}

void shannon_mutex_init2(shannon_mutex_t *lock)
{
	mutex_init((struct mutex *)lock);
}

void shannon_mutex_lock(shannon_mutex_t *lock)
{
	mutex_lock((struct mutex *)lock);
}

int shannon_mutex_trylock(shannon_mutex_t *lock)
{
	return mutex_trylock((struct mutex *)lock);
}

void shannon_mutex_unlock(shannon_mutex_t *lock)
{
	mutex_unlock((struct mutex *)lock);
}
#endif /* end of CONFIG_PROVE_LOCKING */
/*  @END of mutex wrapper */

/*  @BEGIN of atomic wrapper */
void shannon_atomic_set(shannon_atomic_t *v, int i)
{
	atomic_set((atomic_t*)v, i);
}

void shannon_atomic64_set(shannon_atomic64_t *v, long int i)
{
	atomic64_set((atomic64_t *)v, i);
}

void shannon_atomic_add(int i, shannon_atomic_t *v)
{
	atomic_add(i, (atomic_t *)v);
}

void shannon_atomic64_add(long int i, shannon_atomic64_t *v)
{
	atomic64_add(i, (atomic64_t *)v);
}

void shannon_atomic_sub(int i, shannon_atomic_t *v)
{
	atomic_sub(i, (atomic_t *)v);
}

void shannon_atomic64_sub(long int i, shannon_atomic64_t *v)
{
	atomic64_sub(i, (atomic64_t *)v);
}

void shannon_atomic_dec(shannon_atomic_t *v)
{
	atomic_dec((atomic_t*)v);
}

void shannon_atomic64_dec(shannon_atomic64_t *v)
{
	atomic64_dec((atomic64_t*)v);
}

void shannon_atomic_inc(shannon_atomic_t *v)
{
	atomic_inc((atomic_t*)v);
}

void shannon_atomic64_inc(shannon_atomic64_t *v)
{
	atomic64_inc((atomic64_t *)v);
}

int shannon_atomic_read(const shannon_atomic_t *v)
{
	return atomic_read((atomic_t*)v);
}

long int shannon_atomic64_read(const shannon_atomic64_t *v)
{
	return atomic64_read((atomic64_t *)v);
}

int shannon_atomic_dec_and_test(shannon_atomic_t *v)
{
	return atomic_dec_and_test((atomic_t*)v);
}

int shannon_atomic_dec_return(shannon_atomic_t *v)
{
	return atomic_dec_return((atomic_t *)v);
}

long int shannon_atomic64_dec_and_test(shannon_atomic64_t *v)
{
	return atomic64_dec_and_test((atomic64_t*)v);
}

int shannon_atomic_inc_and_test(shannon_atomic_t *v)
{
	return atomic_inc_and_test((atomic_t*)v);
}

long int shannon_atomic64_inc_return(shannon_atomic64_t *v)
{
	return atomic64_inc_return((atomic64_t *)v);
}

long int shannon_atomic64_inc_and_test(shannon_atomic64_t *v)
{
	return atomic64_inc_and_test((atomic64_t*)v);
}
/*  @END of atomic wrapper */

/*  @BEGIN of bitops wrapper */
unsigned long shannon_find_first_zero_bit(const unsigned long *addr, unsigned long size)
{
	return find_first_zero_bit(addr, size);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25)

#ifdef __LITTLE_ENDIAN

#define generic_find_next_le_bit(addr, size, offset) \
			find_next_bit(addr, size, offset)

#else

/* include/linux/byteorder does not support "unsigned long" type */
static inline unsigned long ext2_swabp(const unsigned long * x)
{
#if BITS_PER_LONG == 64
	return (unsigned long) __swab64p((u64 *) x);
#elif BITS_PER_LONG == 32
	return (unsigned long) __swab32p((u32 *) x);
#else
#error BITS_PER_LONG not defined
#endif
}

/* include/linux/byteorder doesn't support "unsigned long" type */
static inline unsigned long ext2_swab(const unsigned long y)
{
#if BITS_PER_LONG == 64
	return (unsigned long) __swab64((u64) y);
#elif BITS_PER_LONG == 32
	return (unsigned long) __swab32((u32) y);
#else
#error BITS_PER_LONG not defined
#endif
}

unsigned long generic_find_next_le_bit(const unsigned long *addr, unsigned
		long size, unsigned long offset)
{
	const unsigned long *p = addr + BITOP_WORD(offset);
	unsigned long result = offset & ~(BITS_PER_LONG - 1);
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset &= (BITS_PER_LONG - 1UL);
	if (offset) {
		tmp = ext2_swabp(p++);
		tmp &= (~0UL << offset);
		if (size < BITS_PER_LONG)
			goto found_first;
		if (tmp)
			goto found_middle;
		size -= BITS_PER_LONG;
		result += BITS_PER_LONG;
	}

	while (size & ~(BITS_PER_LONG - 1)) {
		tmp = *(p++);
		if (tmp)
			goto found_middle_swap;
		result += BITS_PER_LONG;
		size -= BITS_PER_LONG;
	}
	if (!size)
		return result;
	tmp = ext2_swabp(p);
found_first:
	tmp &= (~0UL >> (BITS_PER_LONG - size));
	if (tmp == 0UL)		/* Are any bits set? */
		return result + size; /* Nope. */
found_middle:
	return result + __ffs(tmp);

found_middle_swap:
	return result + __ffs(ext2_swab(tmp));
}

#endif

#endif

unsigned long shannon_find_first_bit_le(const void *addr, unsigned long size)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 38)
        return find_next_bit_le(addr, size, 0);
#else
	return generic_find_next_le_bit(addr, size, 0);
#endif
}

unsigned long shannon_find_first_bit(const void *addr, unsigned long size)
{
	return find_next_bit(addr, size, 0);
}

unsigned long shannon_find_next_bit_le(const void *addr, unsigned long size,
                            unsigned long offset)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 38)
        return find_next_bit_le(addr, size, offset);
#else
	return generic_find_next_le_bit(addr, size, offset);
#endif
}

int shannon_test_and_clear_bit_le(int nr, void *addr)
{
	return test_and_clear_bit(nr ^ BITOP_LE_MAGIC, addr);
}

void shannon_set_bit_le(int nr, void *addr)
{
	set_bit(nr ^ BITOP_LE_MAGIC, addr);
}

int shannon_test_bit_le(int nr, const void *addr)
{
	return test_bit(nr ^ BITOP_LE_MAGIC, addr);
}

int shannon_test_and_set_bit(int nr, volatile unsigned long *addr)
{
	return test_and_set_bit(nr, addr);
}

int shannon_test_and_clear_bit(int nr, volatile unsigned long *addr)
{
	return test_and_clear_bit(nr, addr);
}

int shannon_test_bit(int nr, const volatile unsigned long *addr)
{
	return test_bit(nr, addr);
}

void shannon_set_bit(int nr, void *addr)
{
	set_bit(nr, addr);
}

void shannon_clear_bit(int nr, void *addr)
{
	clear_bit(nr, addr);
}
/*  @END of bitops wrapper */

/*  @BEGIN of bitmap wrapper */
void __shannon_bitmap_xor(unsigned long *dst, const unsigned long *bitmap1, const unsigned long *bitmap2, int bits)
{
	__bitmap_xor(dst, bitmap1, bitmap2, bits);
}

int  __shannon_bitmap_weight(const void *bitmap, unsigned int bits)
{
	return __bitmap_weight(bitmap, bits);
}
/*  @END of bitmap wrapper */

/*  @BEGIN of memory related functionalities wrapper */
//  io.h
u32 shannon_raw_readl(const volatile void __iomem *addr)
{
	return __raw_readl(addr);
}

void shannon_raw_writel(u32 value, volatile void __iomem *addr)
{
	__raw_writel(value, addr);
}

u32 shannon_readl(const volatile void __iomem *addr)
{
	return readl(addr);
}

u32 shannon_ioread32(void __iomem *addr)
{
	return ioread32(addr);
}

void shannon_writel(u32 value, volatile void __iomem *addr)
{
	writel(value, addr);
}

void shannon_iowrite32(u32 value, void __iomem *addr)
{
	iowrite32(value, addr);
}

u16 shannon_mem_readw(volatile void *addr)
{
	return __le16_to_cpu(*(u16 *)addr);
}

void shannon_mem_writew(u16 value, void *addr)
{
	*(u16 *)addr = __cpu_to_le16(value);
}

void shannon_cpu_to_le16s(void *addr)
{
	__cpu_to_le16s(addr);
}

u32 shannon_mem_readl(volatile void *addr)
{
	return __le32_to_cpu(*(u32 *)addr);
}

void shannon_mem_writel(u32 value, void *addr)
{
	*(u32 *)addr = __cpu_to_le32(value);
}

void shannon_cpu_to_le32s(void *addr)
{
	__cpu_to_le32s(addr);
}

u32 shannon_cpu_to_be32(u32 val)
{
	return __cpu_to_be32(val);
}

void shannon_cpu_to_le64s(void *addr)
{
	__cpu_to_le64s(addr);
}

u64 shannon_cpu_to_be64(u64 val)
{
	return __cpu_to_be64(val);
}

u64 shannon_mem_readq(volatile void *addr)
{
	return __le64_to_cpu(*(u64 *)addr);
}

void shannon_mem_writeq(u64 value, void *addr)
{
	*(u64 *)addr = __cpu_to_le64(value);
}

void __iomem *shannon_ioremap(shannon_phys_addr_t offset, unsigned long size)
{
	return ioremap(offset, size);
}

void shannon_iounmap(volatile void __iomem *addr)
{
	iounmap(addr);
}

void shannon_memcpy_fromio(void *dest, void *source, unsigned int count)
{
	memcpy_fromio(dest, source, count);
}

//  uaccess.h
long shannon_copy_from_user(void *to, const void __user * from, unsigned long n)
{
	return copy_from_user(to, from, n);
}

long shannon_copy_to_user(void __user *to,
		const void *from, unsigned long n)
{
	return copy_to_user(to, from, n);
}

//  gfp.h
void shannon_free_page(unsigned long addr)
{
	free_page(addr);
}

unsigned long __shannon_get_free_page(gfp_t gfp)
{
	return __get_free_page(gfp);
}

//  mm.h
void *shannon_page_address(shannon_page *page)
{
	return page_address((struct page *)page);
}

//  vmalloc.h
void *shannon_vzalloc(unsigned long size)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	return vzalloc(size);
#else
	void *addr = vmalloc(size);
	if (addr)
		memset(addr, 0, size);

	return addr;
#endif
}

void *shannon_vmalloc(unsigned long size)
{
	return vmalloc(size);
}

void *__shannon_vmalloc(unsigned long size, shannon_gfp_t gfp_mask, shannon_pgprot_t prot)
{
	return __vmalloc(size, gfp_mask, *((pgprot_t *)&prot));
}

void shannon_vfree(void *addr)
{
	vfree(addr);
}

void *shannon_virt_to_page(const void *addr)
{
	return virt_to_page(addr);
}

//  slab.h
shannon_kmem_cache_t *shannon_kmem_cache_create(const char *name, size_t size, size_t align,
		unsigned long flags, void (*ctor)(void *))
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)

	return kmem_cache_create(name, size, align, flags, ctor);

#else

	return kmem_cache_create(name, size, align, flags, (void (*)(void *, struct kmem_cache *, unsigned long))ctor, NULL);

#endif
}

void shannon_kmem_cache_destroy(shannon_kmem_cache_t *cachep)
{
	kmem_cache_destroy((struct kmem_cache *)cachep);
}

void *shannon_kzalloc(size_t size, shannon_gfp_t flags)
{
	return kzalloc(size, flags);
}

void *shannon_kmalloc(size_t size, shannon_gfp_t flags)
{
	return kmalloc(size, flags);
}

void shannon_kfree(const void *p)
{
	kfree(p);
}

//  mempool.h
shannon_mempool_t *shannon_mempool_create_kmalloc_pool(int min_nr, size_t size)
{
	return mempool_create_kmalloc_pool(min_nr, size);
}

shannon_mempool_t *shannon_mempool_create_slab_pool(int min_nr, shannon_kmem_cache_t *kc)
{
	return mempool_create_slab_pool(min_nr, (struct kmem_cache *)kc);
}

void shannon_mempool_destroy(shannon_mempool_t *pool)
{
	mempool_destroy((mempool_t *)pool);
}

void * shannon_mempool_alloc(shannon_mempool_t *pool, shannon_gfp_t gfp_mask)
{
	return mempool_alloc((mempool_t *)pool, gfp_mask);
}

void shannon_mempool_free(void *element, shannon_mempool_t *pool)
{
	mempool_free(element, (mempool_t *)pool);
}

int get_mempool_count(shannon_mempool_t *pool)
{
	return ((mempool_t *)pool)->curr_nr;
}

/*  @END of memory related functionalities wrapper */

/*  @BEGIN of miscellaneous kernel functionalities wrapper */

//  kernel.h
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22)
static char *kvasprintf(gfp_t gfp, const char *fmt, va_list ap)
{
	unsigned int len;
	char *p;
	va_list aq;

	va_copy(aq, ap);
	len = vsnprintf(NULL, 0, fmt, aq);
	va_end(aq);

	p = kmalloc(len+1, gfp);
	if (!p)
		return NULL;

	vsnprintf(p, len+1, fmt, ap);

	return p;
}

#endif

char *shannon_kasprintf(shannon_gfp_t gfp, const char *fmt, ...)
{
	va_list ap;
	char *p;

	va_start(ap, fmt);
	p = kvasprintf(gfp, fmt, ap);
	va_end(ap);

	return p;
}

size_t shannon_strnlen(const char *s, size_t count)
{
	return strnlen(s, count);
}

int shannon_sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, INT_MAX, fmt, args);
	va_end(args);

	return i;
}

int shannon_snprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);

	return i;
}

long shannon_simple_strtol(const char *cp, char **endp, unsigned int base)
{
	return simple_strtol(cp, endp, base);
}

void shannon_dump_stack(void)
{
	dump_stack();
}

int shannon_printk_ratelimit(void)
{
	return printk_ratelimit();
}

//  string.h
void *shannon_memset(void *s, int c, size_t n)
{
	return memset(s, c, n);
}

void *shannon_memcpy(void *dest, const void *src, size_t count)
{
	return memcpy(dest, src ,count);
}

int shannon_memcmp(const void *cs, const void *ct, size_t count)
{
	return memcmp(cs, ct, count);
}

//  printk.h
asmlinkage int shannon_printk(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
#if !defined(__VMKLNX__)
	r = vprintk(fmt, args);
#else
	va_list args_copy;

	va_copy(args_copy, args);
	vmk_vLogNoLevel(VMK_LOG_URGENCY_NORMAL, fmt, args);
	r = vmk_Vsnprintf(NULL, 0, fmt, args_copy);

	va_end(args_copy);
#endif
	va_end(args);

	return r;
}

//  num_online_cpus
int get_num_online_cpus(void)
{
	return num_online_cpus();
}

unsigned long shannon_hweight_long(unsigned long w)
{
	return hweight_long(w);
}

unsigned int shannon_hweight32(unsigned int w)
{
	return hweight32(w);
}

int shannon_get_count_order(unsigned int count)
{
	return get_count_order(count);
}

/* pm_qos_params.h */
int shannon_pm_qos_value = 1;
int shannon_pm_qos_disable = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)

#include <linux/pm_qos.h>
int shannon_pm_qos_add_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 value)
{
	if (unlikely(shannon_pm_qos_disable))
		return 0;

	*l = kzalloc(sizeof(struct pm_qos_request), GFP_SHANNON);

	if (*l == NULL)
		return 0;

	pm_qos_add_request((struct pm_qos_request *)(*l), qos, value);
	return 0;
}

int shannon_pm_qos_update_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 new_value)
{
	if (unlikely(shannon_pm_qos_disable))
		return 0;

	if (*l != NULL)
		pm_qos_update_request((struct pm_qos_request *)(*l), new_value);
	return 0;
}

void shannon_pm_qos_remove_requirement(shannon_pm_qos_request_t *l, int qos, char *name)
{
	if (unlikely(shannon_pm_qos_disable))
		return;

	if (*l != NULL) {
		pm_qos_remove_request((struct pm_qos_request *)(*l));
		kfree(*l);
		*l = NULL;
	}
}

int shannon_pm_qos_is_required(int qos)
{
	if (unlikely(shannon_pm_qos_disable))
		return 1;

	return (pm_qos_request(qos) == shannon_pm_qos_value);
}


#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)

#include <linux/pm_qos_params.h>
int shannon_pm_qos_add_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 value)
{
	if (unlikely(shannon_pm_qos_disable))
		return 0;

	*l = kzalloc(sizeof(struct pm_qos_request_list), GFP_SHANNON);

	if (*l == NULL)
		return 0;

	pm_qos_add_request((struct pm_qos_request_list *)(*l), qos, value);
	return 0;
}

int shannon_pm_qos_update_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 new_value)
{
	if (unlikely(shannon_pm_qos_disable))
		return 0;

	if (*l != NULL)
		pm_qos_update_request((struct pm_qos_request_list *)(*l), new_value);

	return 0;
}

void shannon_pm_qos_remove_requirement(shannon_pm_qos_request_t *l, int qos, char *name)
{
	if (unlikely(shannon_pm_qos_disable))
		return;

	if (*l != NULL) {
	    pm_qos_remove_request((struct pm_qos_request_list *)(*l));
	    kfree(*l);
	    *l = NULL;
	}
}

int shannon_pm_qos_is_required(int qos)
{
	if (unlikely(shannon_pm_qos_disable))
		return 1;

	return (pm_qos_request(qos) == shannon_pm_qos_value);
}

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)

#include <linux/pm_qos_params.h>
int shannon_pm_qos_add_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 value)
{
	if (unlikely(shannon_pm_qos_disable))
		return 0;

	*l = (struct pm_qos_request_list *)pm_qos_add_request(qos, value);
	return 0;
}

int shannon_pm_qos_update_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 new_value)
{
	if (unlikely(shannon_pm_qos_disable))
		return 0;

	if (*l != NULL)
		pm_qos_update_request((struct pm_qos_request_list *)(*l), new_value);
	return 0;
}

void shannon_pm_qos_remove_requirement(shannon_pm_qos_request_t *l, int qos, char *name)
{
	if (unlikely(shannon_pm_qos_disable))
		return;

	if (*l != NULL) {
		pm_qos_remove_request((struct pm_qos_request_list *)(*l));
		*l = NULL;
	}
}

int shannon_pm_qos_is_required(int qos)
{
	if (unlikely(shannon_pm_qos_disable))
		return 1;

	return (pm_qos_request(qos) == shannon_pm_qos_value);
}

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)

#include <linux/pm_qos_params.h>
int shannon_pm_qos_add_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 value)
{
	if (unlikely(shannon_pm_qos_disable))
		return 0;

	return pm_qos_add_requirement(qos, name, value);
}

int shannon_pm_qos_update_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 new_value)
{
	if (unlikely(shannon_pm_qos_disable))
		return 0;

	return pm_qos_update_requirement(qos, name, new_value);
}

void shannon_pm_qos_remove_requirement(shannon_pm_qos_request_t *l, int qos, char *name)
{
	if (unlikely(shannon_pm_qos_disable))
		return;

	pm_qos_remove_requirement(qos, name);
}

int shannon_pm_qos_is_required(int qos)
{
	if (unlikely(shannon_pm_qos_disable))
		return 1;

	return (pm_qos_requirement(qos) == shannon_pm_qos_value);
}

#else

int shannon_pm_qos_add_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 value)
{
	return 0;
}

int shannon_pm_qos_update_requirement(shannon_pm_qos_request_t *l, int qos, char *name, s32 new_value)
{
	return 0;
}

void shannon_pm_qos_remove_requirement(shannon_pm_qos_request_t *l, int qos, char *name)
{
}

int shannon_pm_qos_is_required(int qos)
{
	return 1;
}

#endif

// err.h
void *SHANNON_ERR_PTR(long error)
{
    return ERR_PTR(error);
}

long SHANNON_IS_ERR(const void *ptr)
{
    return IS_ERR(ptr);
}

long SHANNON_PTR_ERR(const void *ptr)
{
    return PTR_ERR(ptr);
}

long SHANNON_IS_ERR_OR_NULL(const void *ptr)
{
	return !ptr || IS_ERR_VALUE((unsigned long)ptr);
}

//string.h
size_t shannon_strlen(const char *s)
{
	return strlen(s);
}

//random.h
void shannon_get_random_bytes(void *buf, int bytes)
{
	get_random_bytes(buf, bytes);
}

//prefetch.h
void shannon_prefetch(void *addr)
{
	prefetch(addr);
}

void shannon_prefetchw(void *addr)
{
	prefetchw(addr);
}

void shannon_spin_lock_prefetch(void *addr)
{
	spin_lock_prefetch(addr);
}

struct shannon_ratelimit_state {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	raw_spinlock_t lock;
#else
	spinlock_t lock;
#endif

	int interval;
	int burst;
	int printed;
	int missed;
	unsigned long begin;
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)

#define DEFINE_SHANNON_RATELIMIT_STATE(name, interval_init, burst_init)		\
									\
	struct shannon_ratelimit_state name = {					\
		.lock		= __RAW_SPIN_LOCK_UNLOCKED(name.lock),	\
		.interval	= interval_init,			\
		.burst		= burst_init,				\
	}
#else

#define DEFINE_SHANNON_RATELIMIT_STATE(name, interval_init, burst_init)		\
									\
	struct shannon_ratelimit_state name = {					\
		.lock		= __SPIN_LOCK_UNLOCKED(name.lock),	\
		.interval	= interval_init,			\
		.burst		= burst_init,				\
	}
#endif
static DEFINE_SHANNON_RATELIMIT_STATE(shannon_rs, SHANNON_DEFAULT_RATELIMIT_INTERVAL, SHANNON_DEFAULT_RATELIMIT_BURST);
static DEFINE_SHANNON_RATELIMIT_STATE(recover_rs, SHANNON_DEFAULT_RECOVER_RATELIMIT_INTERVAL, SHANNON_DEFAULT_RECOVER_RATELIMIT_BURST);

inline int shannon_ratelimit(struct shannon_ratelimit_state *rs, const char *func)
{
	unsigned long flags;
	int ret;

	if (!rs->interval)
		return 1;

	/*
	 * If we contend on this state's lock then almost
	 * by definition we are too busy to print a message,
	 * in addition to the one that will be printed by
	 * the entity that is holding the lock already:
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	if (!raw_spin_trylock_irqsave(&rs->lock, flags))
#else
	if (!spin_trylock_irqsave(&rs->lock, flags))
#endif
		return 0;

	if (!rs->begin)
		rs->begin = jiffies;

	if (time_after(jiffies, rs->begin + rs->interval)) {
		if (rs->missed)
			printk(KERN_ERR "shn_log: %s: %d callbacks suppressed\n",
				func, rs->missed);
		rs->begin   = 0;
		rs->printed = 0;
		rs->missed  = 0;
	}
	if (rs->burst && rs->burst > rs->printed) {
		rs->printed++;
		ret = 1;
	} else {
		rs->missed++;
		ret = 0;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	raw_spin_unlock_irqrestore(&rs->lock, flags);
#else
	spin_unlock_irqrestore(&rs->lock, flags);
#endif

	return ret;
}

asmlinkage int shannon_printk_ratelimited(const char *fmt, ...)
{
	va_list args;
	int r;

	if (shannon_ratelimit(&shannon_rs, __func__)) {
		va_start(args, fmt);
#if !defined(__VMKLNX__)
		r = vprintk(fmt, args);
#else
		va_list args_copy;

		va_copy(args_copy, args);
		vmk_vLogNoLevel(VMK_LOG_URGENCY_NORMAL, fmt, args);
		r = vmk_Vsnprintf(NULL, 0, fmt, args_copy);

		va_end(args_copy);
#endif
		va_end(args);

		return r;
	}
	return 0;
}

asmlinkage int shannon_recover_printk_ratelimited(const char *fmt, ...)
{
	va_list args;
	int r;

	if (shannon_ratelimit(&recover_rs, __func__)) {
		va_start(args, fmt);
#if !defined(__VMKLNX__)
		r = vprintk(fmt, args);
#else
		va_list args_copy;

		va_copy(args_copy, args);
		vmk_vLogNoLevel(VMK_LOG_URGENCY_NORMAL, fmt, args);
		r = vmk_Vsnprintf(NULL, 0, fmt, args_copy);

		va_end(args_copy);
#endif
		va_end(args);

		return r;
	}
	return 0;
}
/*  @END of miscellaneous kernel functionalities wrapper */
