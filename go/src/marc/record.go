package marc

// Record is the basic representation of a MARC record
type Record struct {
	data   []byte
	fields []Field
}

// Leader retrieves the leader (first 24 bytes) of a MARC record
func (rec Record) Leader() []byte {
	return rec.data[0:24]
}
