package marc;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.List;

public class MarcRecord {
    private ByteBuffer data;
    private List<MarcField> fields;

    private MarcRecord() {
    }

    protected MarcRecord(byte[] data) {
        this.data = ByteBuffer.wrap(data);
    }

    public int getLength() {
        return data.limit();
    }

    public static MarcRecord fromData(byte[] data) throws ParseException {
        MarcRecord rec = new MarcRecord();

        int recordLength = data.length;
        int baseAddress = Util.atoin(data, 12, 5);
        int fieldCount = (baseAddress - 24 - 1) / 12;

        List<MarcField> fields = new ArrayList<MarcField>();
        for (int i = 0; i < fieldCount; i++) {
            int directoryPosition = 24 + i * 12;

            // process the directory
            int tag = Util.atoin(data, directoryPosition, 3);
            int fieldLength = Util.atoin(data, directoryPosition + 3, 4);
            int fieldPosition = Util.atoin(data, directoryPosition + 7, 5);
        }

        return rec;
    }

    public boolean validate() {
        return true;
    }

    public ByteBuffer getLeader() {
        return data.rewind().slice().limit(24);
    }

    public void write(OutputStream out) throws IOException {
        if (fields == null)
            out.write(data.array());
    }
}