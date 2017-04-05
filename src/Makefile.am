AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = liblttng-mcount.la

liblttng_mcount_la_SOURCES = @ARCHDIR@/fentry.S @ARCHDIR@/arch-dynamic.c \
	dynamic.c mcount.c
liblttng_mcount_la_CFLAGS = -include config.h -I@ARCHDIR@/ \
	-fvisibility=hidden
liblttng_mcount_la_LIBADD = -llttng-ust

