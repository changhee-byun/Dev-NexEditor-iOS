package com.nexstreaming.app.common.util;

public class FourCC {
    private FourCC() {
    }

    public static final int fromChars(char a, char b, char c, char d) {
        return ((a & 0xFF) << 24) | ((b & 0xFF) << 16) | ((c & 0xFF) << 8) | ((d & 0xFF));
    }

    public static final int fromString(String s) {
        int fourcc = 0;
        for(int i=0; i<4; i++)
            fourcc = (fourcc << 8) | s.charAt(i);
        return fourcc;
    }

    public static final String toString(int fourcc) {
        StringBuilder sb = new StringBuilder();
        sb.append((char)((fourcc>>24)&0xFF));
        sb.append((char)((fourcc>>16)&0xFF));
        sb.append((char)((fourcc>>8)&0xFF));
        sb.append((char)((fourcc)&0xFF));
        return sb.toString();
    }

}
