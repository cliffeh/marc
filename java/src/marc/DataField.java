package marc;

public class DataField extends MarcField {
    public byte[] indicators;
    private byte[] data;
    private MarcSubfield[] subfields;
}
