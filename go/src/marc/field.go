package marc

import "io"

// Field basic representation of a MARC field
type Field interface {
	Write(w io.Writer)
	Process() Field
}

type ControlField struct {
	data []byte
	Tag  int
}

type DataField struct {
	data      []byte
	Tag       int
	subfields []Subfield
}

// Subfield basic representation of a MARC subfield
type Subfield struct {
	Code byte
	data []byte
}

func (f ControlField) Process() Field {
	return f
}

func (f ControlField) Write(w io.Writer) {
	w.Write(f.data)
}
