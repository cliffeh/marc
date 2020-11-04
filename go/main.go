package main

import (
	"bufio"
	"compress/gzip"
	"io"
	"os"

	"marc"
)

func check(e error) {
	if e != nil {
		panic(e)
	}
}

func main() {
	br := bufio.NewReader(os.Stdin)
	testBytes, err := br.Peek(2)
	check(err)
	var in io.Reader
	if testBytes[0] == 31 && testBytes[1] == 139 { // gzip
		in, err = gzip.NewReader(br)
		check(err)
	} else {
		in = br
	}

	mr := marc.NewReader(in)
	out := bufio.NewWriter(os.Stdout)
	for rec, err := mr.ReadRecord(); rec != nil; rec, err = mr.ReadRecord() {
		check(err)
		rec.Process().Write(out)
	}
	out.Flush()
}
