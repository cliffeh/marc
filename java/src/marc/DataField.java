package marc;

import java.nio.ByteBuffer;

public class DataField extends MarcField {
    public DataField(int tag, ByteBuffer data) {
        super(tag, data);
    }
}
