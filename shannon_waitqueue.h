#ifndef __SHANNON_WAITQUEUE_H
#define __SHANNON_WAITQUEUE_H

#include "shannon_kcore.h"
#include "shannon_sched.h"
#include "shannon_list.h"

struct __shannon_wait_queue_head {
	shannon_spinlock_t lock;
	struct shannon_list_head task_list;
};
typedef struct __shannon_wait_queue_head shannon_wait_queue_head_t;

struct shannon_lock_class_key {
	RESERVE_MEM(16);
};

typedef struct __shannon_wait_queue shannon_wait_queue_t;
typedef int (*shannon_wait_queue_func_t)(shannon_wait_queue_t *wait, unsigned mode, int flags, void *key);

struct __shannon_wait_queue {
	unsigned int flags;
	void *private;
	shannon_wait_queue_func_t func;
	struct shannon_list_head task_list;
};

extern void __shannon_wake_up(shannon_wait_queue_head_t *q, unsigned int mode, int nr_exclusive, void *key);
extern void __shannon_init_waitqueue_head(shannon_wait_queue_head_t *q, struct shannon_lock_class_key *key);
extern int shannon_waitqueue_active(shannon_wait_queue_head_t *q);
extern void shannon_prepare_to_wait(shannon_wait_queue_head_t *q, shannon_wait_queue_t *wait, int state);
extern void shannon_finish_wait(shannon_wait_queue_head_t *q, shannon_wait_queue_t *wait);

extern int shannon_autoremove_wake_function(shannon_wait_queue_t *wait, unsigned mode, int sync, void *key);

#define shannon_init_waitqueue_head(q)				\
	do {							\
		static struct shannon_lock_class_key __key;	\
								\
		__shannon_init_waitqueue_head((q), &__key);	\
	} while (0)

#define shannon_wake_up(x)	__shannon_wake_up(x, (SHN_TASK_INTERRUPTIBLE | SHN_TASK_UNINTERRUPTIBLE), 1, NULL)

#define SHANNON_DEFINE_WAIT_FUNC(name, function)				\
	shannon_wait_queue_t name = {						\
		.private	= shannon_current(),				\
		.func		= function,					\
		.task_list	= SHANNON_LIST_HEAD_INIT((name).task_list),	\
	}

#define SHANNON_DEFINE_WAIT(name) SHANNON_DEFINE_WAIT_FUNC(name, shannon_autoremove_wake_function)

#define __shannon_wait_event(wq, condition)					\
do {										\
	SHANNON_DEFINE_WAIT(__wait);						\
										\
	for (;;) {								\
		shannon_prepare_to_wait(&wq, &__wait, SHN_TASK_UNINTERRUPTIBLE);	\
		if (condition)							\
		break;								\
		shannon_schedule();						\
	}									\
	shannon_finish_wait(&wq, &__wait);					\
} while (0)

#define shannon_wait_event(wq, condition)				\
do {									\
	if (condition)							\
		break;							\
	__shannon_wait_event(wq, condition);				\
} while (0)

#define __shannon_wait_event_interruptible(wq, condition, ret)		\
do {									\
	SHANNON_DEFINE_WAIT(__wait);					\
									\
	for (;;) {							\
		shannon_prepare_to_wait(&wq, &__wait, SHN_TASK_INTERRUPTIBLE);	\
		if (condition)						\
			break;						\
		if (!shannon_signal_pending(shannon_current())) {		\
			shannon_schedule();				\
			continue;					\
		}							\
		ret = -ERESTARTSYS;					\
		break;							\
	}								\
	shannon_finish_wait(&wq, &__wait);				\
} while (0)

#define shannon_wait_event_interruptible(wq, condition)			\
({									\
	int __ret = 0;							\
	if (!(condition))						\
		__shannon_wait_event_interruptible(wq, condition, __ret);	\
	__ret;								\
})


#endif /* __SHANNON_WAITQUEUE_H */
