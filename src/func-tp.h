#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER lttng_mcount

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "func-tp.h"

#if !defined(_TRACEPOINT_FUNC_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TRACEPOINT_FUNC_TP_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Copyright (C) 2017 Jean-Alexandre Barszcz <jalex_b@hotmail.com>
 * Copyright (C) 2011-2013  Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <lttng/tracepoint.h>

TRACEPOINT_EVENT_CLASS(lttng_mcount, func_class,
	TP_ARGS(void *, func_addr, void *, call_site),
	TP_FIELDS(
		ctf_integer_hex(unsigned long, addr,
			(unsigned long) func_addr)
		ctf_integer_hex(unsigned long, call_site,
			(unsigned long) call_site)
	)
)

TRACEPOINT_EVENT_INSTANCE(lttng_mcount, func_class,
	func_entry,
	TP_ARGS(void *, func_addr, void *, call_site)
)

TRACEPOINT_LOGLEVEL(lttng_mcount, func_entry,
	TRACE_DEBUG_FUNCTION)

TRACEPOINT_EVENT_INSTANCE(lttng_mcount, func_class,
	func_exit,
	TP_ARGS(void *, func_addr, void *, call_site)
)

TRACEPOINT_LOGLEVEL(lttng_mcount, func_exit,
	TRACE_DEBUG_FUNCTION)

#endif /* _TRACEPOINT_FUNC_TP_H */

/* This part must be outside ifdef protection */
#include <lttng/tracepoint-event.h>

#ifdef __cplusplus
}
#endif
