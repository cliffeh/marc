package marc

import (
	"fmt"
	"io"
)

type Record struct {
	Leader    []byte
	Directory []byte
	Data      []byte
}

type RecordReader struct {
	rd io.Reader
}

func NewReader(rd io.Reader) *RecordReader {
	r := new(RecordReader)
	r.rd = rd
	return r
}

func (rr RecordReader) ReadRecord() (r *Record, e error) {
	r = new(Record)
	r.Leader = make([]byte, 24)
	n, e := rr.rd.Read(r.Leader)
	if n != 24 {
		return nil, fmt.Errorf("leader fewer than 24 bytes")
	}

	return r, e
}
