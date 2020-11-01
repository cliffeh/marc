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

func Atoi(bytes []byte) (r int, e error) {
	r = 0
	for _, b := range bytes {
		r *= 10
		r += int(b) - '0'
	}
	return r, nil
}

func main() {
	in := marc.NewReader(os.Stdin)
	r, err := in.ReadRecord()
	check(err)

	fmt.Println(string(r.Leader))
}
