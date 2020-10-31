package marc;

import java.nio.ByteBuffer;

public class ControlField extends Field {
    public ControlField(int tag, ByteBuffer data) {
        super(tag, data);
    }
}
