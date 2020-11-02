package marc

// Field basic representation of a MARC field
type Field struct {
	data []byte
	Tag  int
}
