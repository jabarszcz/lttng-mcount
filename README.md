# lttng-mcount
Semi-dynamic instrumentation of function entry/exit with lttng

This project aims to allow instrumentation of a program in a similar
fashion to the lttng-ust-cyg-profile library, with the difference
that it makes it possible to enable/disable tracing of specific
functions at runtime.

The library interface to query and change the instrumentation is
provided in the file `dynamic.h`.

Most of the original code of the project comes from
[uftrace](https://github.com/namhyung/uftrace), but the actual tracing
is done with lttng.

## How it works

The library patches the beginning of functions where a NOP has been
inserted by the compiler and can put a call a tracing routine or
switch back to a NOP. This ensures low overhead when not tracing
because only a single NOP is executed. The instrumentation put before
the function also hijacks the return address to call function exit
instrumentation.

The options used by gcc to insert NOPs are the following:

    -pg -mfentry -mnop-mcount

> *Note: If dynamic behavior is not wanted and the tracing of all
> functions should be permanently activated, only put the two options
> `-pg -mfentry`.*

The library provides two functions to dynamically control
instrumentation of specific program functions:

    enum lttng_mcount_patch {NO_PATCH = -1, NOP = 0, CALL};
    
    int set_instrumentation(unsigned long addr, int enable);
    int get_instrumentation(unsigned long addr,
                            enum lttng_mcount_patch *status);

Using these, a program can control it's own instrumentation
dynamically. Alternatively, another library can use this capability to
provide a user interface, for instance. (*TODO insert link to TUI lib*)

An example program can be found in `tests/`.

The lttng events generated are `lttng_mcount:func_entry` and
`lttng_mcount:func_exit`. Beside the name, they are identical to those
from lttng-ust-cyg-profile.

## Drawbacks

* Relies on gcc and flags specific to x86
* Concurrent execution/mutation of a program might lead to problems
* Recompilation needed
