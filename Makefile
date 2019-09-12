DATA=~/data/BooksAll.2016.part43.utf8
THREADS=1
VALGRINDS=marc-validate.valgrind marc-print.valgrind marc-print-field.valgrind

CFLAGS=-g -Wall

BINARIES=marc

all: $(BINARIES)

marc: marc.o util.o main.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

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
