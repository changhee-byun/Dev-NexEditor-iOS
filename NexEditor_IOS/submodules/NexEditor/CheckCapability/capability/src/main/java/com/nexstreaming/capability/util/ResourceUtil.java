package com.nexstreaming.capability.util;

import android.content.Context;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by Eric on 2015-10-29.
 */
public class ResourceUtil {

    /**
     @brief
     Raw 자원의 파일 보존
     */
    public static void raw2file(Context context, int resID, String fileName) throws Exception {
        InputStream in = context.getResources().openRawResource(resID);
        in2file(context, in, fileName);
    }
    /**
     @brief
     입력 스트림의 파일 보존
     */

    public static void in2file(Context context, InputStream in, String fileName) throws Exception {
        int size;
        byte[] w = new byte[1024];
        OutputStream out = null;
        try {
            out = context.openFileOutput(fileName, Context.MODE_WORLD_READABLE);
            while (true) {
                size = in.read(w);
                if (size <= 0)
                    break;
                out.write(w, 0, size);
            }
        } catch (Exception e) {
            throw e;
        } finally {
            try {
                if (in != null) {
                    in.close();
                }
                if (out != null) {
                    out.close();
                }
            } catch (Exception e2) {
            }
        }
    }
}
