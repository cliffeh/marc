lib_LTLIBRARIES = libmarc.la
libmarc_la_SOURCES = marc.c config.h marc.h

bin_PROGRAMS = marc-dump marc-leaders marc-print marc-validate marc-xml

marc_dump_SOURCES = marc-dump.c main.c
marc_dump_LDADD = libmarc.la
marc_leaders_SOURCES = marc-leaders.c main.c
marc_leaders_LDADD = libmarc.la
marc_print_SOURCES = marc-print.c main.c
marc_print_LDADD = libmarc.la
marc_validate_SOURCES = marc-validate.c main.c
marc_validate_LDADD = libmarc.la
marc_xml_SOURCES = marc-xml.c main.c
marc_xml_LDADD = libmarc.la

TESTS = test/marc-dump test/marc-leaders test/marc-print test/marc-validate test/marc-xml

ACLOCAL_AMFLAGS = -I m4
