package marc;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.List;

public class MarcRecord {
    private ByteBuffer data;
    private int baseAddress;
    private List<MarcField> fields;

    protected MarcRecord(byte[] buf) {
        this.data = ByteBuffer.wrap(buf);
        this.fields = null;
    }

    public int getLength() {
        return data.limit();
    }

    public MarcRecord process() throws ParseException {
        data.position(12);
        baseAddress = Util.atoin(data, 5);
        
        // TODO check for appropriate terminators
        ByteBuffer directory = data.position(24).slice().limit(baseAddress-24-1);
        ByteBuffer fieldData = data.position(baseAddress).slice();

        this.fields = new ArrayList<MarcField>();
        while(directory.hasRemaining()) { // walk the directory
            int tag = Util.atoin(directory, 3);
            int fieldLength = Util.atoin(directory, 4);
            int fieldPosition = Util.atoin(directory, 5);

            ByteBuffer current = fieldData.position(fieldPosition).slice().limit(fieldLength);
            MarcField field = MarcField.isControlField(tag) ? new ControlField(tag, current)
                    : new DataField(tag, current);
            fields.add(field);
        }
        return this;
    }

    public boolean validate() {
        return true;
    }

    public ByteBuffer getLeader() {
        return data.rewind().slice().limit(24);
    }

    public void write(OutputStream out) throws IOException {
        if (fields == null) {
            out.write(data.array());
        } else {
            byte[] buf = new byte[baseAddress];
            data.position(0).get(buf, 0, baseAddress);
            out.write(buf);
            for(MarcField field : fields) {
                field.write(out);
            }
            out.write(Constants.RECORD_TERMINATOR);
        }
    }
}