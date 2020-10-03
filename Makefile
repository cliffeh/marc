# a valid file containing marc records (or a symlink to one)
DATA=./testfile.marc
VALGRINDS=marc-validate.valgrind marc-print.valgrind marc-print-field.valgrind

CFLAGS=-g -Wall

BINARIES=marc-validate marc-dump

all: $(BINARIES)

$(BINARIES): %: util.o marc.o main.o %.o
	$(CC) $(CFLAGS) -o $@ $^

valgrind: $(VALGRINDS)

marc-validate.valgrind: marc
	valgrind \
		--leak-check=full --log-file=$@ \
		./$< validate --threads $(THREADS) $(DATA) \
		> /dev/null 2>&1

marc-print.valgrind: marc
	valgrind \
		--leak-check=full --log-file=$@ \
		./$< print --threads $(THREADS) $(DATA) \
		> /dev/null 2>&1

marc-print-field.valgrind: marc
	valgrind \
		--leak-check=full --log-file=$@ \
		./$< print --field 245a --threads $(THREADS) $(DATA) \
		> /dev/null 2>&1

clean:
	rm -f *.o *.valgrind

realclean: clean
	rm -f $(BINARIES)
