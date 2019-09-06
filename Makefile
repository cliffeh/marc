DATA=~/data/BooksAll.2016.part43.utf8

CFLAGS=-g -Wall

BINARIES=marc

all: $(BINARIES)

marc: marc.o util.o main.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

marc.valgrind: marc
	valgrind \
		--leak-check=full --log-file=$@ \
		./$< validate -i $(DATA) \
		> /dev/null 2>&1

clean:
	rm -f *.o

realclean: clean
	rm -f $(BINARIES) *.valgrind
