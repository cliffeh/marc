package marc

import (
	"fmt"
	"io"
)

// RecordReader basic MARC record reader
type RecordReader struct {
	rd io.Reader
}

// NewReader creates a new MARC record reader
func NewReader(rd io.Reader) *RecordReader {
	r := new(RecordReader)
	r.rd = rd
	return r
}

// ReadRecord reads a single MARC record
func (rr RecordReader) ReadRecord() (r *Record, e error) {
	r = new(Record)
	buf := make([]byte, 24)
	n, e := rr.rd.Read(buf)
	if n != 24 {
		return nil, fmt.Errorf("leader fewer than 24 bytes")
	}

	length, e := atoi(buf[0:5])
	r.data = make([]byte, length)
	copy(r.data, buf)
	n, e = rr.rd.Read(r.data[24:])
	if n < length-24 {
		return nil, fmt.Errorf("too few bytes; expected %d, got %d", length, n+24)
	}

	return r, e
}
