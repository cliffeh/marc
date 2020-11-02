package main

import (
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
	rec, err := in.ReadRecord()
	check(err)

	rec.Process().Write(os.Stdout)
}
