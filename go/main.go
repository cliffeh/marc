package main

import (
	"fmt"
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

	fmt.Println(string(rec.Leader()))
}
