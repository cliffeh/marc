package marc;

import java.nio.ByteBuffer;

public abstract class MarcField {
    public int tag;
    private ByteBuffer data;

    public MarcField(int tag, ByteBuffer data) {
        this.tag = tag;
        this.data = data;
    }
}