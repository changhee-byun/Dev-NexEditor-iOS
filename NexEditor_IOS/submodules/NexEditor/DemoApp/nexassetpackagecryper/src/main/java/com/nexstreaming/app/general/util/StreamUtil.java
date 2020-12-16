package com.nexstreaming.app.general.util;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class StreamUtil {

    private StreamUtil(){} // Prevent instantiation

    public static long copy(InputStream input, OutputStream output) throws IOException {
        byte[] copybuf = new byte[1024*4];
        long count = 0;
        int n = 0;
        while (-1 != (n = input.read(copybuf))) {
            output.write(copybuf, 0, n);
            count += n;
        }
        return count;
    }

}
