package marc

import "io"

// Record basic representation of a MARC record
type Record struct {
	baseAddress int
	data        []byte
	fields      []Field
}

// Leader retrieves the leader (first 24 bytes) of a MARC record
func (rec Record) Leader() []byte {
	return rec.data[0:24]
}

// Process processes and populates this Record's fields
func (rec Record) Process() Record {
	nFields := (rec.baseAddress - 24 - 1) / 12
	rec.fields = make([]Field, nFields)
	for i := 0; i < nFields; i++ {
		p := 24 + i*12
		tag := atoi(rec.data[p : p+3])
		len := atoi(rec.data[p+3 : p+7])
		pos := atoi(rec.data[p+7 : p+12])

		field := new(Field)
		field.Tag = tag
		field.data = rec.data[pos : pos+len-1]
		rec.fields[i] = *field
	}
	return rec
}

// Write writes rec out in MARC format
func (rec Record) Write(w io.Writer) {
	if rec.fields == nil {
		w.Write(rec.data)
	} else {
		w.Write(rec.data[0:rec.baseAddress])
	}
}
