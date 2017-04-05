#ifndef _UTIL_COMPILER_H_
#define _UTIL_COMPILER_H_

#ifndef likely
# define likely(x)		__builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
# define unlikely(x)		__builtin_expect(!!(x), 0)
#endif

#define __weak  __attribute__((weak))
#define __visible_default  __attribute__((visibility("default")))

#endif //_UTIL_COMPILER_H_
