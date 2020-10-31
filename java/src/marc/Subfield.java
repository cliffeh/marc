package marc;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

public class Subfield {
    public char code;
    private ByteBuffer data;

    public Subfield(char code, ByteBuffer data) {
        this.code = code;
        this.data = data;
    }

    public synchronized void write(OutputStream out) throws IOException {
        out.write(code);
        data.position(0);
        while (data.hasRemaining()) {
            out.write(data.get());
        }
    }
}
