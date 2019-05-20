#include "shannon_workqueue.h"
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/workqueue.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
struct __delayed_work {
	struct work_struct work;
};
#endif

void shannon_init_work(struct shannon_work_struct *work, shannon_work_func_t func)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	INIT_WORK((struct work_struct *)work, (work_func_t)func);
#else
	INIT_WORK((struct work_struct *)work, (void *)func, work);
#endif
}

void shannon_init_delayed_work(struct shannon_delayed_work *work, shannon_work_func_t func)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	INIT_DELAYED_WORK((struct delayed_work *)work, (work_func_t)func);
#else
	INIT_WORK((struct work_struct *)work, (void *)func, work);
#endif
}

struct shannon_delayed_work *get_delayed_work(struct shannon_work_struct *work)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	struct delayed_work *delayed_work = container_of((struct work_struct *)work, struct delayed_work, work);
	return (struct shannon_delayed_work *)delayed_work;
#else
	return (struct shannon_delayed_work *)work;
#endif

}

int shannon_queue_work(shannon_workqueue_struct_t *wq, struct shannon_work_struct *work)
{
	return queue_work((struct workqueue_struct *)wq, (struct work_struct *)work);
}

shannon_workqueue_struct_t *shannon_create_singlethread_workqueue(const char *name)
{
	return create_singlethread_workqueue(name);
//	return alloc_ordered_workqueue("%s", WQ_MEM_RECLAIM | WQ_HIGHPRI, name);
}

shannon_workqueue_struct_t *shannon_create_workqueue(const char *name)
{
	return create_workqueue(name);
}

void shannon_destroy_workqueue(shannon_workqueue_struct_t *wq)
{
	destroy_workqueue((struct workqueue_struct *)wq);
}

void shannon_flush_workqueue(shannon_workqueue_struct_t *wq)
{
	flush_workqueue((struct workqueue_struct *)wq);
}

void shannon_cancel_delayed_work(struct shannon_delayed_work *work)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	cancel_delayed_work((struct delayed_work *)work);
#else
	cancel_delayed_work((struct work_struct *)work);
#endif
}

int shannon_queue_delayed_work(shannon_workqueue_struct_t *wq, struct shannon_delayed_work *work, unsigned long delay)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	return queue_delayed_work((struct workqueue_struct *)wq, (struct delayed_work *)work, delay);
#else
	return queue_delayed_work((struct workqueue_struct *)wq, (struct work_struct *)work, delay);
#endif
}

int shannon_schedule_delayed_work(struct shannon_delayed_work *work, unsigned long delay)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	return schedule_delayed_work((struct delayed_work *)work, delay);
#else
	return schedule_delayed_work((struct work_struct *)work, delay);
#endif
}

int shannon_schedule_work(struct shannon_work_struct *work)
{
	return schedule_work((struct work_struct *)work);
}

void shannon_work_clear_pending(struct shannon_work_struct *work)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
	clear_bit(0, &((struct work_struct *)work)->pending);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22)
	work_release((struct work_struct *)work);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0)
	work_clear_pending((struct work_struct *)work);
#else
	clear_bit(WORK_STRUCT_PENDING_BIT, work_data_bits((struct work_struct *)work));
#endif
}

int shannon_work_pending(struct shannon_work_struct *work)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
	return test_bit(0, &((struct work_struct *)work)->pending);
#else
	return work_pending((struct work_struct *)work);
#endif
}

int __shannon_cancel_delayed_work(struct shannon_delayed_work *work)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
	int ret;

	ret = del_timer(&((struct delayed_work *)work)->timer);
	if (ret)
		shannon_work_clear_pending((struct shannon_work_struct *)(&((struct delayed_work *)work)->work));
	return ret;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)
	return __cancel_delayed_work((struct delayed_work *)work);
#else
	int ret;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
	ret = del_timer(&((struct delayed_work *)work)->timer);
	if (ret)
		shannon_work_clear_pending((struct shannon_work_struct *)(&((struct delayed_work *)work)->work));
#else
	ret = del_timer(&(((struct __delayed_work *)work)->work.timer));
	if (ret)
		shannon_work_clear_pending((struct shannon_work_struct *)(&((struct __delayed_work *)work)->work));
#endif
	return ret;
#endif
}

#define RT_MAX_LOOP_COUNT 1000

int rt_thread_fn(void *data)
{
	struct shannon_rt_workqueue_struct *rtwq = data;
	struct shannon_rt_work_struct *rtw;
	unsigned long flags;
	int ret = set_thread_rt();
	int loop_count, is_rt;

	if (ret)
		shannon_warn("Failed to set '%s' to be RT, error: %d.\n", rtwq->thread_name, ret);
	while (1) {
		loop_count = 0;
		is_rt = 1;
		while (!shannon_list_empty(&rtwq->list)) {
			flags = shannon_spin_lock_irqsave(&rtwq->lock);
			rtw = shannon_list_first_entry(&rtwq->list, struct shannon_rt_work_struct,
							list);
			shannon_list_del_init(&rtw->list);
			shannon_clear_bit(RT_WORK_STRUCT_PENDING_BIT, &rtw->flags);
			shannon_spin_unlock_irqrestore(&rtwq->lock, flags);
			rtw->func(rtw);
			++loop_count;
			if (loop_count >= RT_MAX_LOOP_COUNT) {
				if (is_rt) {
					ret = set_thread_normal();
					is_rt = 0;
				} else
					shannon_cond_resched();
			}
		}
		if (!is_rt)
			set_thread_rt();
		shannon_wake_up(&rtwq->flush_event);
		if (unlikely(shannon_kthread_should_stop()))
			break;
		flags = shannon_spin_lock_irqsave(&rtwq->lock);
		if (shannon_list_empty(&rtwq->list)) {
			shannon_set_current_state(SHN_TASK_INTERRUPTIBLE);
			shannon_spin_unlock_irqrestore(&rtwq->lock, flags);
			shannon_schedule();
		} else
			shannon_spin_unlock_irqrestore(&rtwq->lock, flags);
	}

	return 0;
}

struct shannon_rt_workqueue_struct *shannon_create_singlethread_rt_workqueue(const char *name)
{
	struct shannon_rt_workqueue_struct *rtwq = shannon_kzalloc(sizeof(*rtwq), GFP_SHANNON);
	if (!rtwq)
		return NULL;
	shannon_spin_lock_init(&rtwq->lock);
	SHANNON_INIT_LIST_HEAD(&rtwq->list);
	shannon_init_waitqueue_head(&rtwq->flush_event);
	rtwq->thread = shannon_kthread_run(rt_thread_fn, rtwq, "%s", name);
	if (SHANNON_IS_ERR(rtwq->thread)) {
		shannon_kfree(rtwq);
		return NULL;
	}
	shannon_snprintf(rtwq->thread_name, sizeof(rtwq->thread_name), "%s", name);
	return rtwq;
}

void shannon_flush_rt_workqueue(struct shannon_rt_workqueue_struct *rtwq)
{
	shannon_wake_up_process(rtwq->thread);
	shannon_wait_event(rtwq->flush_event, shannon_list_empty(&rtwq->list));
}

void shannon_destroy_rt_workqueue(struct shannon_rt_workqueue_struct *rtwq)
{
	shannon_flush_rt_workqueue(rtwq);
	shannon_kthread_stop(rtwq->thread);
	shannon_kfree(rtwq);
}

int shannon_rt_queue_work(struct shannon_rt_workqueue_struct *rtwq, struct shannon_rt_work_struct *work)
{
	unsigned long flags;
	int ret = 0;
	flags = shannon_spin_lock_irqsave(&rtwq->lock);
	if (!shannon_test_and_set_bit(RT_WORK_STRUCT_PENDING_BIT, &work->flags)) {
		BUG_ON(!shannon_list_empty(&work->list));
		shannon_list_add_tail(&work->list, &rtwq->list);
	} else
		ret = -1;
	shannon_spin_unlock_irqrestore(&rtwq->lock, flags);
	if (ret == 0)
		shannon_wake_up_process(rtwq->thread);
	return ret;
}

void shannon_init_rt_work(struct shannon_rt_work_struct *work, shannon_rt_work_func_t func)
{
	SHANNON_INIT_LIST_HEAD(&work->list);
	work->func = func;
}
