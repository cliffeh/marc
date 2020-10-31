package marc;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.List;

public class DataField extends MarcField {
    private byte[] indicators;
    private List<MarcSubfield> subfields;

    public DataField(int tag, ByteBuffer data) {
        super(tag, data);
        indicators = new byte[2];
        subfields = null;
        System.err.println("new data field: " + tag);
    }

    @Override
    public synchronized MarcField process() throws ParseException {
        data.position(0);

        indicators[0] = data.get();
        indicators[1] = data.get();

        subfields = new ArrayList<MarcSubfield>();

        byte b;
        while ((b = data.get()) != Constants.FIELD_TERMINATOR && data.hasRemaining()) {
            if (b != Constants.SUBFIELD_DELIMITER) {
                throw new ParseException("expected subfield delimiter, got byte '" + b + "'", 2);
            }
            char code = (char)data.get();
            data.mark();
            int limit = 0;
            while(data.hasRemaining() && ((b = data.get()) != Constants.SUBFIELD_DELIMITER)) {
                // System.err.println("b: " + b);
                limit++;
            }
            int pos = data.position() - 1;
            System.err.println("char: " + code + " ; pos: " + pos);
            ByteBuffer subfieldData = data.reset().slice().limit(limit);
            subfields.add(new MarcSubfield(code, subfieldData));
            data.position(pos);
        }

        return this;
    }

    @Override
    public synchronized void write(OutputStream out) throws IOException {
        if (subfields == null) {
            super.write(out);
        } else {
            out.write(indicators[0]);
            out.write(indicators[1]);
            for (MarcSubfield subfield : subfields) {
                out.write(Constants.SUBFIELD_DELIMITER);
                subfield.write(out);
            }
            out.write(Constants.FIELD_TERMINATOR);
        }
    }
}
