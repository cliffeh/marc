package marc;

import java.io.IOException;
import java.io.OutputStream;

public class MarcRecord {
    private int length;
    private byte[] data;
    private MarcField[] fields;

    protected MarcRecord(byte[] data) {
        this.data = data;
        this.length = data.length;
    }

    public boolean validate() {
        return true;
    }

    public void write(OutputStream out) throws IOException {
        if (fields == null)
            out.write(data);
    }
}