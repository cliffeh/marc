package marc

const (
	// RecordTerminator byte terminates a MARC record
	RecordTerminator = 0x1d

	// FieldTerminator byte terminates a MARC field
	FieldTerminator = 0x1e

	// SubfieldDelimiter byte delimits a MARC field subfield
	SubfieldDelimiter = 0x1f
)
