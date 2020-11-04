package marc

import (
	"bytes"
	"io"
)

// Field basic functions for a MARC field
type Field interface {
	Write(w io.Writer)
	Process() Field
}

// ControlField representation of a MARC control field
type ControlField struct {
	data []byte
	Tag  int
}

// DataField representation of a MARC data field
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

// NewControlField generate a new control field
func NewControlField(tag int, data []byte) *ControlField {
	f := new(ControlField)
	f.Tag = tag
	f.data = data
	return f
}

// NewDataField generate a new data field
func NewDataField(tag int, data []byte) *DataField {
	f := new(DataField)
	f.Tag = tag
	f.data = data
	return f
}

// Process process the ControlField
func (f *ControlField) Process() Field {
	return f // no additional processing necessary
}

// Process process the DataField
func (f *DataField) Process() Field {
	pieces := bytes.Split(f.data[2:], []byte{SubfieldDelimiter})
	f.subfields = make([]Subfield, len(pieces)-1)
	for i, piece := range pieces[1:] {
		f.subfields[i].Code = piece[0]
		f.subfields[i].data = piece[1:]
	}
	return f
}

func (f *ControlField) Write(w io.Writer) {
	w.Write(f.data)
}

func (f *DataField) Write(w io.Writer) {
	// write indicators
	w.Write(f.data[0:2])
	for _, subfield := range f.subfields {
		w.Write([]byte{SubfieldDelimiter})
		w.Write([]byte{subfield.Code})
		w.Write(subfield.data)
	}
}
