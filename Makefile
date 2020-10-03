# a file/symlink containing valid marc records
DATA=./testfile.marc
CFLAGS=-g -Wall

BINARIES=marc-validate marc-dump marc-print marc-leaders
VALGRINDS=$(patsubst %,%.valgrind,$(BINARIES))

all: $(BINARIES)

$(BINARIES): %: util.o marc.o main.o %.o
	$(CC) $(CFLAGS) -o $@ $^

valgrind: $(VALGRINDS)

$(VALGRINDS): %.valgrind: %
	valgrind --leak-check=full --log-file=$@ ./$< $(DATA) > /dev/null 2>&1

test: marc-dump
	./marc-dump < $(DATA) | diff $(DATA) - && echo "basic dump test passed"

clean:
	rm -f *.o *.valgrind

realclean: clean
	rm -f $(BINARIES)
