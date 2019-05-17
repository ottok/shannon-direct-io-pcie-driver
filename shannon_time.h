#ifndef __SHANNON_TIME_H
#define __SHANNON_TIME_H

#include "shannon_kcore.h"

#define from_timer(var, callback_timer, timer_fieldname) \
	container_of(callback_timer, typeof(*var), timer_fieldname)

//  timer.h
struct __shannon_timer_list {
	RESERVE_MEM(160);
};

struct shannon_timeval {
	u64 tv_sec;
	u64 tv_usec;
};

typedef struct __shannon_timer_list shannon_timer_list;

extern int get_HZ(void);
extern void shannon_init_timer(shannon_timer_list *timer);
extern void shannon_add_timer(shannon_timer_list *timer, unsigned long expires);
extern void shannon_set_timer_context(shannon_timer_list *timer, void (*f)(shannon_timer_list *));
extern int shannon_timer_has_function(shannon_timer_list *timer);
extern int shannon_del_timer_sync(shannon_timer_list *timer);
extern int shannon_del_timer(shannon_timer_list *timer);
extern int shannon_mod_timer(shannon_timer_list *timer, unsigned long expires);
extern int shannon_timer_pending(shannon_timer_list * timer);

//  delay.h
extern void shannon_msleep(unsigned int msecs);
extern void shannon_udelay(unsigned long usecs);

extern unsigned long get_jiffies(void);
extern void shannon_do_gettimeofday(struct shannon_timeval *tv);

//  jiffies.h
extern unsigned int shannon_jiffies_to_msecs(const unsigned long j);
extern unsigned int shannon_jiffies_to_usecs(const unsigned long j);
extern unsigned long shannon_msecs_to_jiffies(const unsigned int m);
extern unsigned long shannon_usecs_to_jiffies(const unsigned int u);

#endif /* __SHANNON_TIME_H */
