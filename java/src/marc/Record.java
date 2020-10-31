package marc;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.List;

public class Record {
    private ByteBuffer data;
    private int baseAddress;
    private List<Field> fields;

    protected Record(byte[] buf) {
        this.data = ByteBuffer.wrap(buf);
        this.fields = null;
    }

    public synchronized Record process(boolean deep) throws ParseException {
        data.position(12);
        baseAddress = Util.atoin(data, 5);

        // TODO check for appropriate terminators
        ByteBuffer directory = data.position(24).slice().limit(baseAddress - 24 - 1);
        ByteBuffer fieldData = data.position(baseAddress).slice();

        this.fields = new ArrayList<Field>();
        while (directory.hasRemaining()) { // walk the directory
            int tag = Util.atoin(directory, 3);
            int fieldLength = Util.atoin(directory, 4);
            int fieldPosition = Util.atoin(directory, 5);

            ByteBuffer current = fieldData.position(fieldPosition).slice().limit(fieldLength - 1);
            Field field = Field.isControlField(tag) ? new ControlField(tag, current)
                    : new DataField(tag, current);

            if (deep)
                field = field.process();

            fields.add(field);
        }
        return this;
    }

    public synchronized void write(OutputStream out) throws IOException {
        if (fields == null) {
            out.write(data.array());
        } else {
            data.position(0);
            for (int i = 0; i < baseAddress; i++) {
                out.write(data.get());
            }
            for (Field field : fields) {
                field.write(out);
            }
            out.write(Constants.RECORD_TERMINATOR);
        }
    }
}