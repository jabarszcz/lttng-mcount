#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <lttng-mcount/dynamic.h>

__attribute__((noinline))
static void foo()
{
	asm(""); // Avoid optimization
}

static void *toggle_loop(void *_unused)
{
	unsigned long target = (unsigned long) foo;
	while(1) {
		enum lttng_mcount_patch status;
		(void) get_instrumentation(target, &status);
		if (status == NO_PATCH) {
			printf("no patch for foo\n");
			exit(1);
		}

		(void) set_instrumentation(target, !status);
	}

	return NULL;
}

/* 
 * Test program used to verify if concurrent execution and mutation of
 * the program lead to problems like segfaults or illegal
 * instructions.
 *
 * If there is no problem, it will run indefinitely, otherwise it will
 * crash.
 */
int main(int argc, char **argv)
{
	pthread_t t;
	(void) pthread_create(&t, NULL, toggle_loop, NULL);

	// This thight loop will create lots of events when tracing is
	// enabled
	while(1) {
		foo();
	}
	return 0;
}
