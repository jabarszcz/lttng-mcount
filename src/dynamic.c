#include "dynamic.h"

#include "util/compiler.h"

__weak void dynamic_init(void)
{
}

__weak int set_instrumentation(unsigned long addr, int enable)
{
	return -1;
}

__weak int get_instrumentation(unsigned long addr,
			       enum lttng_mcount_patch *status)
{
	return -1;
}
