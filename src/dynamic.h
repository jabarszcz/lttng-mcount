#ifndef _DYNAMIC_H_
#define _DYNAMIC_H_

#include "util/compiler.h"

enum lttng_mcount_patch {NO_PATCH = -1, NOP = 0, CALL};

void dynamic_init(void);

__visible_default
int set_instrumentation(unsigned long addr, int enable);

__visible_default
int get_instrumentation(unsigned long addr,
			enum lttng_mcount_patch *status);

#endif // _DYNAMIC_H_
