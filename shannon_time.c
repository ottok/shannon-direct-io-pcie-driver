#include "shannon_time.h"
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <asm/param.h>

int get_HZ(void)
{
	return HZ;
}

//  timer.h
void shannon_init_timer(shannon_timer_list *timer)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	init_timer((struct timer_list *)timer);
	((struct timer_list *)timer)->function = NULL;
#else
	__init_timer((struct timer_list *)timer, NULL, 0);
#endif
}

void shannon_add_timer(shannon_timer_list *timer, unsigned long expires)
{
	((struct timer_list *)timer)->expires = expires;
	add_timer((struct timer_list *)timer);
}

int shannon_del_timer_sync(shannon_timer_list *timer)
{
	return del_timer_sync((struct timer_list *)timer);
}

int shannon_del_timer(shannon_timer_list *timer)
{
	return del_timer((struct timer_list *)timer);
}

int shannon_mod_timer(shannon_timer_list *timer, unsigned long expires)
{
	return mod_timer((struct timer_list *)timer, expires);
}

int shannon_timer_pending(shannon_timer_list * timer)
{
	return timer_pending((struct timer_list *)timer);
}

void shannon_set_timer_context(shannon_timer_list *timer, void (*f)(shannon_timer_list *))
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	((struct timer_list *)timer)->data = (unsigned long)timer;
	((struct timer_list *)timer)->function = (void (*) (unsigned long))f;
#else
	((struct timer_list *)timer)->function = (void (*) (struct timer_list *))f;
#endif
}

int shannon_timer_has_function(shannon_timer_list *timer)
{
	return ((struct timer_list *)timer)->function ? 1 : 0;
}

//  delay.h
void shannon_msleep(unsigned int msecs)
{
	msleep(msecs);
}

void shannon_udelay(unsigned long usecs)
{
	udelay(usecs);
}


unsigned long get_jiffies(void)
{
	return jiffies;
}

void shannon_getnstimeofday(struct shannon_timeval *tv)
{
	struct timespec time;
	getnstimeofday(&time);
	tv->tv_sec = time.tv_sec;
	tv->tv_usec = time.tv_nsec/1000;
}

void shannon_do_gettimeofday(struct shannon_timeval *tv)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 0, 0)
	struct timeval time;
	do_gettimeofday(&time);
	tv->tv_sec = time.tv_sec;
	tv->tv_usec = time.tv_usec;
#else
	shannon_getnstimeofday(tv);
#endif
}

// jiffies.h
unsigned int shannon_jiffies_to_msecs(const unsigned long j)
{
	return jiffies_to_msecs(j);
}

unsigned int shannon_jiffies_to_usecs(const unsigned long j)
{
	return jiffies_to_usecs(j);
}

unsigned long shannon_msecs_to_jiffies(const unsigned int m)
{
	return msecs_to_jiffies(m);
}

unsigned long shannon_usecs_to_jiffies(const unsigned int u)
{
	return usecs_to_jiffies(u);
}
