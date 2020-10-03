# a valid file containing marc records (or a symlink to one)
DATA=./testfile.marc
VALGRINDS=marc-validate.valgrind marc-print.valgrind marc-print-field.valgrind

CFLAGS=-g -Wall

BINARIES=marc-validate marc-dump
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
