package marc;

import java.nio.ByteBuffer;

public class ControlField extends MarcField {
    public ControlField(int tag, ByteBuffer data) {
        super(tag, data);
        System.err.println("new control field: " + tag);
    }
}
