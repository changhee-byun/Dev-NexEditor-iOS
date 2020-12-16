/******************************************************************************
 * File Name        : AutoTestUtils.java
 * Description      :
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

package com.nexstreaming.nexeditorsdkapis.common;

import android.graphics.Bitmap;
import android.os.Debug;

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

public class AutoTestUtils {

    public static String checkMemoryStatus() {
        StringBuilder sb = new StringBuilder();

        long totalMemory = Runtime.getRuntime().totalMemory();
        long useMemory = Runtime.getRuntime().totalMemory() - Runtime.getRuntime().freeMemory();
        long maxMemory = Runtime.getRuntime().maxMemory();
        long nativeMemory = Debug.getNativeHeapAllocatedSize();

        sb.append("VM Total Memory \t: " + totalMemory + "\n");
        sb.append("VM Use Memory \t: " + useMemory + "\n");
        sb.append("VM Max Memory \t: " + maxMemory + "\n");
        sb.append("Native Memory \t: " + nativeMemory + "\n");

        return sb.toString();
    }

    public static File savebitmap(Bitmap bitmap, String filename) {
        OutputStream outStream = null;

        File file = new File(filename);
        if (file.exists()) {
            file.delete();
            file = new File(filename);
        }
        try {
            // make a new bitmap from your file
            outStream = new FileOutputStream(file);
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, outStream);
            outStream.flush();
            outStream.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        // Log.e("file", "" + file);
        return file;

    }

    public static File savelog(String log, String filename) {
        File file = new File(filename);
        if (file.exists()) {
            file.delete();
            file = new File(filename);
        }
        try {
            // make a new bitmap from your file

            OutputStreamWriter fos = new OutputStreamWriter(new FileOutputStream(file));
            fos.write(log);
            fos.flush();
            fos.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        // Log.e("file", "" + file);
        return file;
    }

    public static void deleteFlagFile(String path) {
        File file = new File(path, "now_testing");
        if (file.exists()) {
            file.delete();
        }
        file = null;
    }

}
