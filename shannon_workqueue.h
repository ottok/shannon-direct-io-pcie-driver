#ifndef __SHANNON_WORKQUEUE_H
#define __SHANNON_WORKQUEUE_H

#include "shannon_kcore.h"
#include "shannon_list.h"
#include "shannon_sched.h"
#include "shannon_waitqueue.h"

struct shannon_work_struct {
	RESERVE_MEM(216);
};

struct shannon_delayed_work {
	RESERVE_MEM(300);
};

typedef void (*shannon_work_func_t)(struct shannon_work_struct *work);
typedef void shannon_workqueue_struct_t;	//since we only use pointers of struct workqueue_struct


extern void shannon_init_work(struct shannon_work_struct *work, shannon_work_func_t func);
extern void shannon_init_delayed_work(struct shannon_delayed_work *work, shannon_work_func_t func);
extern struct shannon_delayed_work *get_delayed_work(struct shannon_work_struct *work);
extern int shannon_queue_work(shannon_workqueue_struct_t *wq, struct shannon_work_struct *work);
extern int shannon_work_pending(struct shannon_work_struct *work);
extern int shannon_queue_delayed_work(shannon_workqueue_struct_t*wq, struct shannon_delayed_work *work, unsigned long delay);
extern int shannon_schedule_delayed_work(struct shannon_delayed_work *work, unsigned long delay);
extern int shannon_schedule_work(struct shannon_work_struct *work);
extern void shannon_cancel_delayed_work(struct shannon_delayed_work *work);
extern void shannon_work_clear_pending(struct shannon_work_struct *work);

extern shannon_workqueue_struct_t *shannon_create_singlethread_workqueue(const char *name);
extern shannon_workqueue_struct_t *shannon_create_workqueue(const char *name);
extern void shannon_destroy_workqueue(shannon_workqueue_struct_t *wq);
extern void shannon_flush_workqueue(shannon_workqueue_struct_t *wq);

struct shannon_rt_workqueue_struct {
	shannon_spinlock_t lock;
	struct shannon_list_head list;
	shannon_wait_queue_head_t flush_event;
	shannon_task_struct_t *thread;
	char thread_name[32];
};

struct shannon_rt_work_struct;
typedef void (*shannon_rt_work_func_t)(struct shannon_rt_work_struct *work);
struct shannon_rt_work_struct {
#define RT_WORK_STRUCT_PENDING_BIT        0
	unsigned long flags;
	shannon_rt_work_func_t func;
	struct shannon_list_head list;
};

extern void shannon_init_rt_work(struct shannon_rt_work_struct *work, shannon_rt_work_func_t func);
extern int shannon_rt_queue_work(struct shannon_rt_workqueue_struct *rtwq, struct shannon_rt_work_struct *work);
extern struct shannon_rt_workqueue_struct *shannon_create_singlethread_rt_workqueue(const char *name);
extern void shannon_destroy_rt_workqueue(struct shannon_rt_workqueue_struct *rtwq);
extern void shannon_flush_rt_workqueue(struct shannon_rt_workqueue_struct *rtwq);
#endif /* __SHANNON_WORKQUEUE_H */
