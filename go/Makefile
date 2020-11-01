SRC=main.go $(wildcard src/marc/*.go)
BIN=bin/marc

$(BIN): $(SRC)
	GOPATH=$$PWD go build -o $@

clean:
	rm -f tmpfil testfile

distclean: clean
	rm -f $(BIN)
