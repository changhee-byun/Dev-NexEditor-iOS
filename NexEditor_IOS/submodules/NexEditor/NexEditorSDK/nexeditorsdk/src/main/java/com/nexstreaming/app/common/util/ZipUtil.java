package com.nexstreaming.app.common.util;

import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.SecretKeySpec;

public class ZipUtil {

    private static final String LOG_TAG = "ZipUtil";

    private ZipUtil(){} // Prevent instantiation

    public static void unzip(File zipPath, File outputPath) throws IOException {

        if( LL.D ) Log.d(LOG_TAG,"Unzipping '" + zipPath + "' to '" + outputPath + "'");
        if( !outputPath.mkdirs() && !outputPath.exists() ) {
            throw new IOException("Failed to create directory: " + outputPath);
        }
        ZipInputStream in = new ZipInputStream(new FileInputStream(zipPath));
        try {
            ZipEntry zipEntry;
            while ((zipEntry = in.getNextEntry()) != null) {
                String name = zipEntry.getName();
                if (name.contains("..")) {
                    throw new IOException("Relative paths not allowed"); // For security purposes
                }
                File outputFile = new File(outputPath, name);
                if (zipEntry.isDirectory()) {
                    if (!outputFile.mkdirs() && !outputFile.exists()) {
                        throw new IOException("Failed to create directory: " + outputFile);
                    }
                    if( LL.D ) Log.d(LOG_TAG,"  - unzip: made folder '" + name + "'");
                } else {
                    if( LL.D ) Log.d(LOG_TAG,"  - unzip: unzipping file '" + name + "' " + zipEntry.getCompressedSize() + "->" + zipEntry.getSize() + " (" + getZipMethod(zipEntry.getMethod()) + ")");
                    FileOutputStream out = new FileOutputStream(outputFile);
                    try {
                        copy(in,out);
                    } finally {
                        CloseUtil.closeSilently(out);
                    }
                }
            }
        } finally {
            CloseUtil.closeSilently(in);
        }
        if( LL.D ) Log.d(LOG_TAG,"Unzipping DONE for: '" + zipPath + "' to '" + outputPath + "'");

    }

    private static String getZipMethod(int method) {
        switch(method) {
            case ZipEntry.DEFLATED:
                return "DEFLATED";
            case ZipEntry.STORED:
                return "STORED";
            default:
                return "UNKNOWN_" + method;
        }
    }

    private static void copy(InputStream input, OutputStream output) throws IOException {
        byte[] copybuf = new byte[1024*4];
        long count = 0;
        int n = 0;
        while (-1 != (n = input.read(copybuf))) {
            output.write(copybuf, 0, n);
            count += n;
        }
    }

    public static boolean isZipFile(File path){
        try {
            InputStream in = new FileInputStream(path);
            byte [] magic = new byte[2];
            try {
                in.read(magic);
                in.close();
                if(magic[0]=='P' && magic[1]=='K'){
                    return true;
                }else{
                    Log.d(LOG_TAG,"magic[0]="+magic[0]+",magic[1]="+magic[1]);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return false;
    }

}
