#include "shannon_sched.h"
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
#include <linux/sched/rt.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#include <linux/sched/debug.h>
#include <uapi/linux/sched/types.h>
#endif

void *shannon_current(void)
{
	return current;
}

int shannon_signal_pending(void *p)
{
	struct task_struct *task = (struct task_struct *)p;
	return signal_pending(task);
}

int shannon_in_softirq(void)
{
	return in_softirq();
}

//  interrupt.h
irqreturn_t shannon_interrupt_wrapper(int irq, void *data
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
		                                    , struct pt_regs *regs
#endif
						        )
{
	shannon_interrupt(irq, data);
	return IRQ_HANDLED;
}

void shannon_disable_irq(unsigned int irq)
{
	disable_irq(irq);
}

void shannon_enable_irq(unsigned int irq)
{
	enable_irq(irq);
}

int shannon_request_irq(unsigned int irq, const char *devname, void *data)
{
	return request_irq(irq, shannon_interrupt_wrapper, 0, devname, data);
}

void shannon_tasklet_schedule(struct shannon_tasklet_struct *t)
{
	tasklet_schedule((struct tasklet_struct *)t);
}

void shannon_tasklet_init(struct shannon_tasklet_struct *t, void (*func)(unsigned long), unsigned long data)
{
	tasklet_init((struct tasklet_struct *)t, func, data);
}

void shannon_free_irq(unsigned int irq, void *dev_id)
{
	free_irq(irq, dev_id);
}


//  sched.h
int shannon_wake_up_process(shannon_task_struct_t *tsk)
{
	return wake_up_process((struct task_struct *)tsk);
}

void shannon_schedule(void)
{
	schedule();
}

signed long __sched shannon_schedule_timeout(signed long timeout)
{
	return schedule_timeout(timeout);
}

void shannon_set_current_state(long state)
{
	set_current_state(state);
}

void __shannon_set_current_state(long state)
{
	__set_current_state(state);
}

void shannon_cond_resched(void)
{
	cond_resched();
}

int set_thread_normal(void)
{
	struct sched_param param = { .sched_priority = 0 };
	return sched_setscheduler(current, SCHED_NORMAL, &param);
}

int set_thread_highest_prio_normal(void)
{
	struct sched_param param = { .sched_priority = MAX_PRIO - 1 };
	return sched_setscheduler(current, SCHED_NORMAL, &param);
}

int set_thread_rt(void)
{
	struct sched_param param = { .sched_priority = MAX_USER_RT_PRIO-1 };
	return sched_setscheduler(current, SCHED_FIFO, &param);
}

//  kthread.h
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

shannon_task_struct_t * shannon_kthread_run(int (*threadfn)(void *data), void *data, const char *namefmt, ...)
{
	shannon_task_struct_t *task;
	va_list ap;
	char *p;

	va_start(ap, namefmt);
	p = kvasprintf(GFP_SHANNON, namefmt, ap);
	va_end(ap);

	task = kthread_run(threadfn, data, p);
	shannon_kfree(p);

	return task;
}

int shannon_kthread_stop(shannon_task_struct_t *k)
{
	return kthread_stop((struct task_struct *)k);
}

int shannon_kthread_should_stop(void)
{
	return kthread_should_stop();
}

int shannon_set_node_cpus_allowed(shannon_task_struct_t *k, int node)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
	if (node != NUMA_NO_NODE) {
		const struct cpumask *cpumask = cpumask_of_node(node);

		if (!cpumask_empty(cpumask))
			return set_cpus_allowed_ptr((struct task_struct *)k, cpumask);
	}
#endif

	return 0;
}

bool shannon_not_set_cpumask(shannon_cpumask_struct_t *scpumask)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	return cpumask_full((cpumask_t*)scpumask);
#else
	return bitmap_full(((cpumask_t *)scpumask)->bits, (unsigned int)NR_CPUS);
#endif
}

shannon_cpumask_struct_t *shannon_get_current_cpus_allowed(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
	return &current->cpus_mask;
#else
	return &current->cpus_allowed;
#endif
}

const char *shannon_get_current_comm(void)
{
	return current->comm;
}
