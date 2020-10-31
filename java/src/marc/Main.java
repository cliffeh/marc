package marc;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PushbackInputStream;
import java.text.ParseException;
import java.util.zip.GZIPInputStream;

public class Main {
    public static void main(String[] args) {
        String[] infiles = (args.length == 0) ? new String[] { "-" } : args;
        try {
            for (int i = 0; i < infiles.length; i++) {
                InputStream in;
                if("-".equals(infiles[i])) {
                    in = System.in;
                } else {
                    in = new FileInputStream(infiles[i]);
                }

                // check if it's gzipped
                PushbackInputStream pb = new PushbackInputStream(in, 2);
                byte[] bytes = new byte[2];
                pb.read(bytes);
                pb.unread(bytes);
                int head = ((int) bytes[0] & 0xff) | ((bytes[1] << 8) & 0xff00);
                if (GZIPInputStream.GZIP_MAGIC == head)
                    in = new GZIPInputStream(pb);
                else
                    in = pb; // plaintext

                Reader reader = new Reader(in);
                Record rec = null;
                while ((rec = reader.read()) != null) {
                    rec.process(true).write(System.out);
                }
                in.close();
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
