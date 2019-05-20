#ifndef __SHANNON_SCHED_H
#define __SHANNON_SCHED_H

#include "shannon_kcore.h"

#define	SHANNON_IRQ_NONE	    0
#define SHANNON_IRQ_HANDLED	    1
#define	SHANNON_IRQ_WAKE_THREAD	    2

#define SHN_TASK_RUNNING            0
#define SHN_TASK_INTERRUPTIBLE      1
#define SHN_TASK_UNINTERRUPTIBLE    2
#define __SHN_TASK_STOPPED          4
#define __SHN_TASK_TRACED           8
/* in tsk->exit_state */
#define SHN_EXIT_ZOMBIE             16
#define SHN_EXIT_DEAD               32
/* in tsk->state again */
#define SHN_TASK_DEAD               64
#define SHN_TASK_WAKEKILL           128
#define SHN_TASK_WAKING             256
#define SHN_TASK_STATE_MAX          512

#define SHN_TASK_KILLABLE           (SHN_TASK_WAKEKILL | SHN_TASK_UNINTERRUPTIBLE)
#define SHN_TASK_STOPPED            (SHN_TASK_WAKEKILL | __SHN_TASK_STOPPED)
#define SHN_TASK_TRACED             (SHN_TASK_WAKEKILL | __SHN_TASK_TRACED)

struct shannon_tasklet_struct {
	RESERVE_MEM(56);
};

typedef void shannon_task_struct_t;

extern void *shannon_current(void);
extern int shannon_signal_pending(void *p);
extern int shannon_in_softirq(void);

//  interrupt.h
extern int shannon_interrupt(int irq, void *data);
extern void shannon_disable_irq(unsigned int irq);
extern void shannon_enable_irq(unsigned int irq);
extern int shannon_request_irq(unsigned int irq, const char *devname, void *data);

extern void shannon_tasklet_schedule(struct shannon_tasklet_struct *t);
extern void shannon_tasklet_init(struct shannon_tasklet_struct *t, void (*func)(unsigned long), unsigned long data);
extern void shannon_free_irq(unsigned int irq, void *dev_id);

//  sched.h
extern void shannon_schedule(void);
extern signed long shannon_schedule_timeout(signed long timeout);
extern int shannon_wake_up_process(shannon_task_struct_t *tsk);
extern void shannon_set_current_state(long state);
extern void __shannon_set_current_state(long state);
extern void shannon_cond_resched(void);
extern int set_thread_rt(void);
extern int set_thread_normal(void);
extern int set_thread_highest_prio_normal(void);
extern int shannon_set_node_cpus_allowed(shannon_task_struct_t *k, int node);

//  kthread.h
extern shannon_task_struct_t * shannon_kthread_run(int (*threadfn)(void *data), void *data, const char *namefmt, ...);
extern int shannon_kthread_stop(shannon_task_struct_t *k);
extern int shannon_kthread_should_stop(void);

#endif /* __SHANNON_SCHED_H */
