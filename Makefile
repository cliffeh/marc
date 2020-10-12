# a file/symlink containing valid marc records
DATA=test/testfile.marc
LIMIT=100
CFLAGS=-g -Wall

BINSRC=$(wildcard marc-*.c)
BINARIES=$(patsubst %.c,%,$(BINSRC))
VALGRINDS=$(patsubst %,%.valgrind,$(BINARIES))

all: $(BINARIES)

$(BINARIES): %: %.o main.o marc.o
	$(CC) $(CFLAGS) -o $@ $^

valgrind: $(VALGRINDS)

$(VALGRINDS): %.valgrind: %
	if [ -z $$(which valgrind) ] ; then	echo "you don't appear to have valgrind installed; skipping..."; else valgrind --leak-check=full --log-file=$@ ./$< --limit $(LIMIT) $(DATA) > /dev/null 2>&1 && cat $@; fi

test: $(BINARIES)
	./marc-dump < $(DATA) | diff $(DATA) - && echo "basic dump test passed"
	./marc-print -l 10 $(DATA) > /dev/null && echo "basic print test passed"

clean:
	rm -f *.o *.valgrind

realclean: clean
	rm -f $(BINARIES)
