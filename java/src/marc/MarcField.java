package marc;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

public abstract class MarcField {
    public int tag;
    private ByteBuffer data;

    public MarcField(int tag, ByteBuffer data) {
        this.tag = tag;
        this.data = data;
    }

    public static boolean isControlField(int tag) {
        return (tag < 10);
    }

    public void write(OutputStream out) throws IOException {
        byte[] buf = new byte[data.limit()];
        data.position(0).get(buf, 0, buf.length);
        out.write(buf);
    }
}