package marc

import "io"

// Record basic representation of a MARC record
type Record struct {
	baseAddress int
	data        []byte
	fields      []Field
}

// Leader retrieves the leader (first 24 bytes) of a MARC record
func (r *Record) Leader() []byte {
	return r.data[0:24]
}

// Process processes and populates this Record's fields
func (r *Record) Process(deep bool) *Record {
	nFields := (r.baseAddress - 24 - 1) / 12
	r.fields = make([]Field, nFields)
	for i := 0; i < nFields; i++ {
		p := 24 + i*12
		tag := atoi(r.data[p : p+3])
		len := atoi(r.data[p+3 : p+7])
		pos := r.baseAddress + atoi(r.data[p+7:p+12])

		field := new(ControlField)
		field.Tag = tag
		field.data = r.data[pos : pos+len-1]
		if deep {
			field.Process()
		}
		r.fields[i] = *field
	}
	return r
}

// Write writes rec out in MARC format
func (r *Record) Write(w io.Writer) {
	if r.fields == nil {
		w.Write(r.data)
	} else {
		w.Write(r.data[0:r.baseAddress])
		for _, field := range r.fields {
			field.Write(w)
			w.Write([]byte{FieldTerminator})
		}
		w.Write([]byte{RecordTerminator})
	}
}
