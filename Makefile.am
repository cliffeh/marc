lib_LTLIBRARIES = libmarc.la
libmarc_la_SOURCES = marc.c util.c config.h marc.h util.h

bin_PROGRAMS = marc
marc_SOURCES = main.c
marc_LDADD = libmarc.la

include_HEADERS = marc.h

check_PROGRAMS = test/dynamic-alloc
check_SCRIPTS = test/marc-dump test/marc-leaders test/marc-print test/marc-validate test/marc-xml \
    test/test-dynamic-alloc test/valgrind-dump test/valgrind-leaders test/valgrind-print \
    test/valgrind-validate test/valgrind-xml test/valgrind-dynamic-alloc

TESTS = $(check_SCRIPTS)

test_dynamic_alloc_LDADD = libmarc.la

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = README.md LICENSE $(check_SCRIPTS)

CLEANFILES = test/*.output test/*.valgrind
