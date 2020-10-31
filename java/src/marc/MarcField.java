package marc;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.text.ParseException;

public abstract class MarcField {
    public int tag;
    protected ByteBuffer data;

    public MarcField(int tag, ByteBuffer data) {
        this.tag = tag;
        this.data = data;
    }

    public static boolean isControlField(int tag) {
        return (tag < 10);
    }

    public void write(OutputStream out) throws IOException {
        data.position(0);
        while(data.hasRemaining()) {
            out.write(data.get());
        }
    }

    public MarcField process() throws ParseException {
        return this;
    }
}