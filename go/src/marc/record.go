package marc

type Record struct {
	Leader    []byte
	Directory []byte
	Data      []byte
}
