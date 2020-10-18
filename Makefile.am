bin_PROGRAMS = marc-dump marc-leaders marc-print marc-validate marc-xml

MAIN_COMMON_SOURCE_FILES = main.c marc.c config.h marc.h

marc_dump_SOURCES = marc-dump.c $(MAIN_COMMON_SOURCE_FILES)
marc_leaders_SOURCES = marc-leaders.c $(MAIN_COMMON_SOURCE_FILES)
marc_print_SOURCES = marc-print.c $(MAIN_COMMON_SOURCE_FILES)
marc_validate_SOURCES = marc-validate.c $(MAIN_COMMON_SOURCE_FILES)
marc_xml_SOURCES = marc-xml.c $(MAIN_COMMON_SOURCE_FILES)

TESTS = test/marc-dump test/marc-leaders test/marc-print test/marc-validate test/marc-xml
