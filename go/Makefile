SRC=main.go $(wildcard src/marc/*.go)

marc: $(SRC)
	GOPATH=$$PWD go build -o marc

clean:
	rm -rf pkg

distclean: clean
	rm -f marc
