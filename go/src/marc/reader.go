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
func (rr *RecordReader) ReadRecord() (r *Record, e error) {
	r = new(Record)
	buf := make([]byte, 24)
	n, e := io.ReadFull(rr.rd, buf)
	if n != 24 {
		return nil, fmt.Errorf("leader fewer than 24 bytes")
	}

	length := atoi(buf[0:5])
	r.data = make([]byte, length)
	copy(r.data, buf)
	n, e = io.ReadFull(rr.rd, r.data[24:])
	if n < length-24 {
		return nil, fmt.Errorf("too few bytes; expected %d, got %d", length, n+24)
	}
	r.baseAddress = atoi(r.data[12:17])

	return r, e
}
