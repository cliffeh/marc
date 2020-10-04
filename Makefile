# a file/symlink containing valid marc records
DATA=./testfile.marc
LIMIT=100
CFLAGS=-g -Wall

BINSRC=$(wildcard bin/marc-*.c)
BINARIES=$(patsubst %.c,%,$(BINSRC))
VALGRINDS=$(patsubst bin/%,%.valgrind,$(BINARIES))

all: $(BINARIES)

$(BINARIES): %: %.o bin/main.o marc.o
	$(CC) $(CFLAGS) -o $@ $^

valgrind: $(VALGRINDS)

$(VALGRINDS): %.valgrind: bin/%
	if [ -z $$(which valgrind) ] ; then	echo "you don't appear to have valgrind installed; skipping..."; else valgrind --leak-check=full --log-file=$@ $< --limit $(LIMIT) $(DATA) > /dev/null 2>&1 && cat $@; fi

test: bin/marc-dump
	bin/marc-dump < $(DATA) | diff $(DATA) - && echo "basic dump test passed"

clean:
	rm -f *.o bin/*.o *.valgrind

realclean: clean
	rm -f $(BINARIES)
