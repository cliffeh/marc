package marc;

import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;

public class MarcReader {
    private InputStream in;
    
    public MarcReader(InputStream in) {
        this.in = in;
    }

    public synchronized Record read() throws IOException, ParseException {
        byte[] leader = new byte[24];
        
        int n = in.readNBytes(leader, 0, 24);
        if(n == 0) { // eof
            return null;
        } else if(n < 24) {
            throw new IOException("fewer than 24 bytes received for leader; got " + n + " bytes");
        }

        int len = Util.atoin(leader, 0, 5); // throws ParseException

        byte[] data = new byte[len];
        System.arraycopy(leader, 0, data, 0, 24);
        n = in.readNBytes(data, 24, len-24); 
        
        if(n < (len-24)) {
            throw new IOException("not enough bytes; expected " + (len-24) + ", got " + n);
        }

        return new Record(data);
    }
}