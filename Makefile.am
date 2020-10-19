lib_LTLIBRARIES = libmarc.la
libmarc_la_SOURCES = marc.c config.h marc.h

bin_PROGRAMS = marc
marc_SOURCES = main.c
marc_LDADD = libmarc.la

include_HEADERS = marc.h

TESTS = test/marc-dump test/marc-leaders test/marc-print test/marc-validate test/marc-xml

ACLOCAL_AMFLAGS = -I m4
