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
    }

    @Override
    public MarcField process() throws ParseException {
        data.position(0);

        indicators[0] = data.get();
        indicators[1] = data.get();

        byte b = data.get();
        if (b != Constants.SUBFIELD_DELIMITER) {
            throw new ParseException("expected subfield delimiter, got byte '" + b + "'", 2);
        }

        subfields = new ArrayList<MarcSubfield>();

        // TODO implement

        return this;
    }

    @Override
    public void write(OutputStream out) throws IOException {
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
