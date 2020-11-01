package marc

type Record struct {
	data   []byte
	fields []Field
}

func (rec Record) Leader() []byte {
	return rec.data[0:24]
}
