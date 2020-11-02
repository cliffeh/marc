package marc

import "io"

// Field basic representation of a MARC field
type Field struct {
	data []byte
	Tag  int
}

// Subfield basic representation of a MARC subfield
type Subfield struct {
	Code byte
	data []byte
}

func (f *Field) Write(w io.Writer) {
	w.Write(f.data)
}
