package marc;

import java.io.IOException;
import java.text.ParseException;

public class Main {
    public static void main(String[] args) {
        try {
            MarcReader in = new MarcReader(System.in);
            MarcRecord rec = null;
            while ((rec = in.read()) != null) {
                rec.process().write(System.out);
            }
            System.out.flush();
        } catch (IOException ioe) {
            ioe.printStackTrace();
            System.exit(1);
        } catch (ParseException pe) {
            pe.printStackTrace();
            System.exit(1);
        }
    }
}
