/*
 * Generic waiting primitives.
 *
 * (C) 2004 William Irwin, Oracle
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/wait.h>
#include <linux/hash.h>
#include <linux/version.h>
#ifdef CONFIG_SUSE_PRODUCT_CODE
#include <linux/suse_version.h>
#endif

#include "shannon_waitqueue.h"

static void __shannon_wake_up_common(shannon_wait_queue_head_t *q, unsigned int mode,
		int nr_exclusive, int wake_flags, void *key)
{
	shannon_wait_queue_t *curr, *next;

	shannon_list_for_each_entry_safe(curr, next, &q->task_list, task_list) {
		unsigned flags = curr->flags;

		if (curr->func(curr, mode, wake_flags, key) &&
				(flags & WQ_FLAG_EXCLUSIVE) && !--nr_exclusive)
			break;
	}
}

void __shannon_wake_up(shannon_wait_queue_head_t *q, unsigned int mode,
		int nr_exclusive, void *key)
{
	unsigned long flags;

	spin_lock_irqsave((spinlock_t *)&q->lock, flags);
	__shannon_wake_up_common(q, mode, nr_exclusive, 0, key);
	spin_unlock_irqrestore((spinlock_t *)&q->lock, flags);
}

void __shannon_init_waitqueue_head(shannon_wait_queue_head_t *q, struct shannon_lock_class_key *key)
{
	spin_lock_init((spinlock_t *)&q->lock);
	lockdep_set_class((spinlock_t *)&q->lock, (struct lock_class_key *)key);
	SHANNON_INIT_LIST_HEAD(&q->task_list);
}

static inline void __shannon_add_wait_queue(shannon_wait_queue_head_t *head, shannon_wait_queue_t *new)
{
	shannon_list_add(&new->task_list, &head->task_list);
}

int shannon_waitqueue_active(shannon_wait_queue_head_t *q)
{
	return !shannon_list_empty(&q->task_list);
}

void shannon_add_wait_queue(shannon_wait_queue_head_t *q, shannon_wait_queue_t *wait)
{
	unsigned long flags;

	wait->flags &= ~WQ_FLAG_EXCLUSIVE;
	spin_lock_irqsave((spinlock_t *)&q->lock, flags);
	__shannon_add_wait_queue(q, wait);
	spin_unlock_irqrestore((spinlock_t *)&q->lock, flags);
}

/*
 * Note: we use "set_current_state()" _after_ the wait-queue add,
 * because we need a memory barrier there on SMP, so that any
 * wake-function that tests for the wait-queue being active
 * will be guaranteed to see waitqueue addition _or_ subsequent
 * tests in this thread will see the wakeup having taken place.
 *
 * The spin_unlock() itself is semi-permeable and only protects
 * one way (it only protects stuff inside the critical region and
 * stops them from bleeding out - it would still allow subsequent
 * loads to move into the critical region).
 */
void shannon_prepare_to_wait(shannon_wait_queue_head_t *q, shannon_wait_queue_t *wait, int state)
{
	unsigned long flags;

	wait->flags &= ~WQ_FLAG_EXCLUSIVE;
	spin_lock_irqsave((spinlock_t *)&q->lock, flags);
	if (shannon_list_empty(&wait->task_list))
		__shannon_add_wait_queue(q, wait);
	set_current_state(state);
	spin_unlock_irqrestore((spinlock_t *)&q->lock, flags);
}


/**
 * finish_wait - clean up after waiting in a queue
 * @q: waitqueue waited on
 * @wait: wait descriptor
 *
 * Sets current thread back to running state and removes
 * the wait descriptor from the given waitqueue if still
 * queued.
 */
void shannon_finish_wait(shannon_wait_queue_head_t *q, shannon_wait_queue_t *wait)
{
	unsigned long flags;

	__set_current_state(TASK_RUNNING);
	/*
	 * We can check for list emptiness outside the lock
	 * IFF:
	 *  - we use the "careful" check that verifies both
	 *    the next and prev pointers, so that there cannot
	 *    be any half-pending updates in progress on other
	 *    CPU's that we haven't seen yet (and that might
	 *    still change the stack area.
	 * and
	 *  - all other users take the lock (ie we can only
	 *    have _one_ other CPU that looks at or modifies
	 *    the list).
	 */
	if (!shannon_list_empty_careful(&wait->task_list)) {
		spin_lock_irqsave((spinlock_t *)&q->lock, flags);
		shannon_list_del_init(&wait->task_list);
		spin_unlock_irqrestore((spinlock_t *)&q->lock, flags);
	}
}

int shannon_autoremove_wake_function(shannon_wait_queue_t *wait, unsigned mode, int sync, void *key)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0)
#ifdef SUSE_PRODUCT_CODE
#if SUSE_PRODUCT_CODE < SUSE_PRODUCT(1, 15, 0, 0)
	int ret = default_wake_function((wait_queue_t *)wait, mode, sync, key);
#else
	int ret = default_wake_function((wait_queue_entry_t *)wait, mode, sync, key);
#endif
#else
	int ret = default_wake_function((wait_queue_t *)wait, mode, sync, key);
#endif
#else
	int ret = default_wake_function((wait_queue_entry_t *)wait, mode, sync, key);
#endif

	if (ret)
		shannon_list_del_init(&wait->task_list);
	return ret;
}
