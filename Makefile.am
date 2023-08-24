lib_LTLIBRARIES = libmarc.la
libmarc_la_SOURCES = marc.c util.c

bin_PROGRAMS = marc
marc_SOURCES = main.c
marc_LDADD = libmarc.la popt/libpopt.a

include_HEADERS = marc.h
noinst_HEADERS = util.h

check_PROGRAMS = test/dynamic-alloc
check_SCRIPTS = test/marc-dump test/marc-leaders test/marc-print test/marc-validate test/marc-xml \
    test/test-dynamic-alloc test/valgrind-dump test/valgrind-leaders test/valgrind-print \
    test/valgrind-validate test/valgrind-xml test/valgrind-dynamic-alloc

SUBDIRS = popt

TESTS = $(check_SCRIPTS)
TEST_INPUTS = test/marc-leaders.txt.gz test/marc-print.txt.gz test/testfile.marc.gz test/testfile.xml.gz

# tell our test scripts where to find their input
TESTS_ENVIRONMENT = SRCDIR=$(srcdir)/test BUILDDIR=$(builddir)/test

test_dynamic_alloc_LDADD = libmarc.la

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = README.md LICENSE $(check_SCRIPTS) $(TEST_INPUTS)

CLEANFILES = test/*.output test/*.valgrind

