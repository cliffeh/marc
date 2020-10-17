# a file/symlink containing valid marc records
DATA=testfile.marc.gz
CFLAGS=-g -Wall
LDFLAGS+=-lz

BINSRC=$(wildcard marc-*.c)
BINARIES=$(patsubst %.c,%,$(BINSRC))
VALGRINDS=$(patsubst %,%.valgrind,$(BINARIES))

all: $(BINARIES)

$(BINARIES): %: %.o main.o marc.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

valgrind: $(VALGRINDS)

$(VALGRINDS): %.valgrind: %
	valgrind --leak-check=full --log-file=$@ ./$< -o /dev/null $(DATA)

test: $(BINARIES)
	./marc-validate $(DATA) -o /dev/null && echo "basic validation passed"
	./marc-dump $(DATA) -o .dumptest && zcat $(DATA) | diff .dumptest - && rm -f .dumptest && echo "basic dump test passed"
	./marc-print $(DATA) -o /dev/null && echo "basic print test passed"

clean:
	rm -f *.o *.valgrind .dumptest

realclean: clean
	rm -f $(BINARIES)
