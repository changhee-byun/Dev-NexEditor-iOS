package com.nexstreaming.editordemo.utility;

import android.content.Context;
import android.content.res.Resources;
import android.util.DisplayMetrics;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.Formatter;
import java.util.Locale;

/**
 * Created by mj.kong on 2015-03-16.
 */
public class utilityCode {
    public static String stringForTime(int timeMs) {
        StringBuilder mFormatBuilder = new StringBuilder();
        Formatter mFormatter = new Formatter(mFormatBuilder, Locale.getDefault());

        int totalSeconds = timeMs / 1000;

        int seconds = totalSeconds % 60;
        int minutes = (totalSeconds / 60) % 60;
        int hours   = totalSeconds / 3600;

        mFormatBuilder.setLength(0);
        if (hours > 0) {
            return mFormatter.format("%d:%02d:%02d", hours, minutes, seconds).toString();
        } else {
            return mFormatter.format("%02d:%02d", minutes, seconds).toString();
        }
    }

    public static void raw2file(Context context, int resID, String fileName) throws Exception {
        InputStream in = context.getResources().openRawResource(resID);
        in2file(context, in, fileName);
    }

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
            };
        } catch (Exception e) {
            throw e;
        } finally {
            try {
                if (in != null) {
                    in.close();
                    in = null;
                }
                if (out != null) {
                    out.close();
                    out = null;
                }
            } catch (Exception e2) {
            }
        }
    }

    /**
     * This method converts dp unit to equivalent pixels, depending on device density.
     *
     * @param dp A value in dp (density independent pixels) unit. Which we need to convert into pixels
     * @param context Context to get resources and device specific display metrics
     * @return A float value to represent px equivalent to dp depending on device density
     */
    public static float convertDpToPixel(float dp, Context context){
        float px = (dp + 0.5f) * context.getResources().getDisplayMetrics().density;
        return px;
    }

    /**
     * This method converts device specific pixels to density independent pixels.
     *
     * @param px A value in px (pixels) unit. Which we need to convert into db
     * @param context Context to get resources and device specific display metrics
     * @return A float value to represent dp equivalent to px value
     */
    public static float convertPixelsToDp(float px, Context context){
        float dp = px / context.getResources().getDisplayMetrics().density;
        return dp;
    }
}
