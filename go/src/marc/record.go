package marc

import "io"

// Record is the basic representation of a MARC record
type Record struct {
	data   []byte
	fields []Field
}

// Leader retrieves the leader (first 24 bytes) of a MARC record
func (rec Record) Leader() []byte {
	return rec.data[0:24]
}

func (rec Record) Write(w io.Writer) {
	if rec.fields == nil {
		w.Write(rec.data)
	}
}
