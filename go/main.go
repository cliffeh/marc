package main

import (
	"bufio"
	"os"

	"marc"
)

func check(e error) {
	if e != nil {
		panic(e)
	}
}

func main() {
	in := marc.NewReader(os.Stdin)
	out := bufio.NewWriter(os.Stdout)
	for rec, err := in.ReadRecord(); rec != nil; rec, err = in.ReadRecord() {
		check(err)
		rec.Process().Write(out)
	}
	out.Flush()
}
