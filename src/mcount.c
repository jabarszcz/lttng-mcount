#include <malloc.h>
#include <pthread.h>
#include <stdio.h>

#include "util/list.h"
#include "util/compiler.h"

#include "mcount-arch.h"

_Atomic(int) lttng_mcount_ready = 0;
static pthread_key_t td_key;
static pthread_once_t td_key_once = PTHREAD_ONCE_INIT;

struct lttng_mcount_stack_frame {
	unsigned long child_ip;
	unsigned long parent_ip;
	struct list_head list;
};

struct lttng_mcount_thread_data {
	int recursion_guard;
	struct list_head stack;
};

struct lttng_mcount_thread_data *mcount_get_thread_data()
{
	return pthread_getspecific(td_key);
}

static void mcount_thread_data_dtor(void *data)
{
	struct lttng_mcount_thread_data *tdp = data;
	struct lttng_mcount_stack_frame *pos = NULL, *next = NULL;

	list_for_each_entry_safe(pos, next, &tdp->stack, list) {
		list_del(&pos->list);
		free(pos);
	}
}

static void make_td_key()
{
	(void) pthread_key_create(&td_key, mcount_thread_data_dtor);
}

static void mcount_prepare()
{
	struct lttng_mcount_thread_data *tdp =
		malloc(sizeof(struct lttng_mcount_thread_data));

	tdp->recursion_guard = 1;
	INIT_LIST_HEAD(&tdp->stack);

	(void) pthread_once(&td_key_once, make_td_key);

	(void) pthread_setspecific(td_key, tdp);
}


static int mcount_should_stop()
{
	struct lttng_mcount_thread_data *tdp;

	// Check global lock for complete init
	if (unlikely(!lttng_mcount_ready))
		return -1;
	// Check thread-local lock to avoid infinite recursion
	tdp = mcount_get_thread_data();
	if (!tdp || tdp->recursion_guard)
		return -1;
	return 0;
}

int mcount_entry(unsigned long *parent_loc, unsigned long child,
		 struct mcount_regs *regs)
{
	struct lttng_mcount_thread_data *tdp;
	struct lttng_mcount_stack_frame *sf;

	if (unlikely(mcount_should_stop()))
		return -1;

	tdp = mcount_get_thread_data();
	tdp->recursion_guard = 1;

	sf = malloc(sizeof(struct lttng_mcount_stack_frame));
	sf->child_ip = child;
	sf->parent_ip = *parent_loc;
	INIT_LIST_HEAD(&sf->list);

	list_add(&sf->list, &tdp->stack);

	printf("entry in %p from %p\n", (void*)child, (void*)*parent_loc);

	tdp->recursion_guard = 0;
	return 0;
}

unsigned long mcount_exit(long *retval)
{
	struct lttng_mcount_thread_data *tdp;
	struct lttng_mcount_stack_frame *stack;
	unsigned long retaddr;

	tdp = mcount_get_thread_data();
	tdp->recursion_guard = 1;

	stack = list_entry(tdp->stack.next,
			   struct lttng_mcount_stack_frame, list);
	retaddr = stack->parent_ip;

	/* trace with lttng */
	tracepoint(uftrace, func_exit,
		   (void *)rstack->child_ip,
		   (void *)rstack->parent_ip);

	list_del(&stack->list);
	free(stack);

	tdp->recursion_guard = 0;

	return retaddr;
}

void __visible_default __monstartup(unsigned long low, unsigned long high)
{
}

void __visible_default _mcleanup(void)
{
}

__attribute__((constructor))
void mcount_init()
{
	struct lttng_mcount_thread_data *tdp;

	mcount_prepare();

	tdp = mcount_get_thread_data();
	tdp->recursion_guard = 0;
	lttng_mcount_ready = 1;
}
