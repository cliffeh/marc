package marc;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

public class MarcSubfield {
    public char code;
    private ByteBuffer data;

    public MarcSubfield(char code, ByteBuffer data) {
        this.code = code;
        this.data = data;
    }

    public void write(OutputStream out) throws IOException {
        out.write(code);
        data.position(0);
        while (data.hasRemaining()) {
            out.write(data.get());
        }
    }
}
