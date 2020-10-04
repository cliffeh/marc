# a file/symlink containing valid marc records
DATA=./testfile.marc
CFLAGS=-g -Wall

BINSRC=$(wildcard bin/marc-*.c)
BINARIES=$(patsubst %.c,%,$(BINSRC))
VALGRINDS=$(patsubst bin/%,%.valgrind,$(BINARIES))

all: $(BINARIES)

$(BINARIES): %: util.o marc.o bin/main.o %.o
	$(CC) $(CFLAGS) -o $@ $^

valgrind: $(VALGRINDS)

$(VALGRINDS): %.valgrind: bin/%
	valgrind --leak-check=full --log-file=$@ $< $(DATA) > /dev/null 2>&1 && cat $@

test: bin/marc-dump
	bin/marc-dump < $(DATA) | diff $(DATA) - && echo "basic dump test passed"

clean:
	rm -f *.o bin/*.o *.valgrind

realclean: clean
	rm -f $(BINARIES)
