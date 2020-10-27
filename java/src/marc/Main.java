package marc;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.text.ParseException;

public class Main {
    public static void main(String[] args) {
        try {
            MarcReader in = new MarcReader(System.in);
            MarcRecord rec = null;
            while ((rec = in.read()) != null) {
                rec.write(System.out);
            }
        } catch (IOException ioe) {
            ioe.printStackTrace();
            System.exit(1);
        } catch (ParseException pe) {
            pe.printStackTrace();
            System.exit(1);
        }
    }
}
