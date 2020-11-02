package marc

import "io"

// Field basic representation of a MARC field
type Field struct {
	data []byte
	Tag  int
}

func (f Field) Write(w io.Writer) {
	w.Write(f.data)
}
