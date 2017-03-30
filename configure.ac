AC_PREREQ([2.69])
AC_INIT([lttng-mcount], [0.0.0], [jalex_b at hotmail dot com])

AC_CONFIG_SRCDIR([src/mcount.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])

AM_INIT_AUTOMAKE([foreign -Wall -Werror -Wno-extra-portability])

# Checks for programs.
AC_PROG_CC
AM_PROG_AS
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC

AC_CANONICAL_HOST
AS_CASE([$host_cpu],
  [x86_64], [ARCHDIR=arch/x86_64],
  [AC_MSG_ERROR("CPU $host_cpu not supported")]
)
AC_SUBST([ARCHDIR])


AC_CONFIG_FILES([Makefile src/Makefile])

LT_INIT
AC_OUTPUT
