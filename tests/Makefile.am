check_PROGRAMS = looptest

looptest_SOURCES = looptest.c
looptest_CFLAGS = -pg -mfentry -mnop-mcount
looptest_LDADD = -llttng-mcount -lpthread
