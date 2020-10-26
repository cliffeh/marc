package marc;

import java.text.ParseException;

public class Util {
    /**
     * converts a subset of buf to an integer, starting at pos and going to len
     *
     * @param buf the buffer to convert
     * @param pos the position in the buffer to start at
     * @param len the number of bytes to convert
     * @return int the integer represented by the subset of the buffer, or -1
     *             if there was a non-digit character
     */
    public static int atoin(byte[] buf, int pos, int len) throws ParseException {
        int r = 0;
        for(int i = pos; i < len; i++) {
            r *= 10;
            switch(buf[i]) {
                case '0': r += 0; break;
                case '1': r += 1; break;
                case '2': r += 2; break;
                case '3': r += 3; break;
                case '4': r += 4; break;
                case '5': r += 5; break;
                case '6': r += 6; break;
                case '7': r += 7; break;
                case '8': r += 8; break;
                case '9': r += 9; break;
                // TODO is this the right kind of exception to throw?
                default: throw new ParseException("unexpected non-digit character '" + (char)buf[i] + "'", i);
            }
        }
        return r;
    }
}