package marc;

import java.io.IOException;
import java.io.OutputStream;

public class MarcRecord {
    private byte[] leader, data;
    public MarcField[] fields;

    protected MarcRecord(byte[] leader, byte[] data) {
        this.leader = leader;
        this.data = data;
    }

    public void write(OutputStream out) throws IOException {
        out.write(leader);
        out.write(data);
    }
}