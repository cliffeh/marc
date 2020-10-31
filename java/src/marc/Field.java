package marc;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.text.ParseException;

public abstract class Field {
    public int tag;
    protected ByteBuffer data;

    public Field(int tag, ByteBuffer data) {
        this.tag = tag;
        this.data = data;
    }

    public static boolean isControlField(int tag) {
        return (tag < 10);
    }

    public synchronized void write(OutputStream out) throws IOException {
        data.position(0);
        while(data.hasRemaining()) {
            out.write(data.get());
        }
        out.write(Constants.FIELD_TERMINATOR);
    }

    public synchronized Field process() throws ParseException {
        return this;
    }
}