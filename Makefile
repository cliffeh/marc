# a file/symlink containing valid marc records
DATA=./testfile.marc
CFLAGS=-g -Wall

BINARIES=marc-validate marc-dump marc-print
VALGRINDS=$(patsubst %,%.valgrind,$(BINARIES))

all: $(BINARIES)

$(BINARIES): %: util.o marc.o main.o %.o
	$(CC) $(CFLAGS) -o $@ $^

valgrind: $(VALGRINDS)

$(VALGRINDS): %.valgrind: %
	valgrind --leak-check=full --log-file=$@ ./$< $(DATA) > /dev/null 2>&1

clean:
	rm -f *.o *.valgrind

realclean: clean
	rm -f $(BINARIES)
