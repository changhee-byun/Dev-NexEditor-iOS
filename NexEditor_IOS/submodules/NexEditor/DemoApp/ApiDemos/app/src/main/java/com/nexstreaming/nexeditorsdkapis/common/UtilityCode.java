/******************************************************************************
 * File Name        : UtilityCode.java
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

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.net.Uri;
import android.os.Build;
import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.Formatter;
import java.util.Locale;
import java.util.Scanner;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.zip.ZipEntry;

import dalvik.system.DexFile;

/**
 * Created by mj.kong on 2015-03-16.
 */
public class UtilityCode {
    private static String TAG = "UtilityCode";
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
        FileOutputStream out = null;
        File file = new File(context.getFilesDir(),fileName);
        try {
            //out = context.openFileOutput(fileName, Context.MODE_WORLD_READABLE);
            out = new FileOutputStream(file);
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

    public static boolean isNumeric(String str)
    {
        return str.matches("-?\\d+(\\.\\d+)?");  //match a number with optional '-' and decimal.
    }

    public static boolean checkLowPerformanceCPU() {
        String name="";
        int cores=0;
        int arch=0;

        try {
            Process p = null;
            try {
                p = new ProcessBuilder("/system/bin/getprop", "ro.board.platform").redirectErrorStream(true).start();
                BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()));
                String line = "";
                while ((line=br.readLine()) != null){
                    name = line.toUpperCase();
                }
                p.destroy();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } catch (Exception e) {
            Log.e("getCPUInfo",Log.getStackTraceString(e));}

        if (name.contains("MT6739") || name.contains("MT6737") || name.contains("MT6580") || name.contains("MSM8909") || name.contains("MSM8916") || name.contains("MT6753")) {
            Log.d(TAG, "This is low performance device! "+name);
            return true;
        }

        try {
            Scanner s = new Scanner(new File("/proc/cpuinfo"));
            while (s.hasNextLine()) {
                String[] vals = s.nextLine().split(": ");
                if (vals.length > 1) {
                    if ( vals[0].contains("processor")) cores++;
                    if ( arch==0 && vals[0].contains("CPU architecture")) arch = Integer.parseInt(vals[1]);
                    //Log.d("UTilityCode", "CPU Info : "+vals[0]+": "+vals[1]);
                }
            }
        } catch (Exception e) {Log.e("getCPUInfo",Log.getStackTraceString(e));}

        if ( cores<4 /*|| arch<8 */) {
            Log.d(TAG, "This is low performance CPU! "+name+", Cores="+cores+", Architecture="+arch);
            return true;
        } else {
            Log.d(TAG, "This is not low performance CPU! "+name+", Cores="+cores+", Architecture="+arch);
            return false;
        }
    }

    public static void launchKineMaster(Activity activity, ArrayList<String> mListFilePath) {
        ArrayList<Uri> clipUris = new ArrayList<Uri>();

        for (int i = 0; i < mListFilePath.size(); i++) {
            clipUris.add(Uri.parse(mListFilePath.get(i))); // Add your clip URIs here
        }

        Intent shareIntent = new Intent();
        shareIntent.setAction(Intent.ACTION_SEND_MULTIPLE);
        shareIntent.putParcelableArrayListExtra(Intent.EXTRA_STREAM, clipUris);
        shareIntent.setType("*/*");
        shareIntent.setClassName(
                "com.nexstreaming.app.kinemasterfree",
                "com.nexstreaming.kinemaster.ui.share.ShareIntentActivity");
        activity.startActivity(shareIntent);
    }

    public static void retrievAPK(Context ctx) {
        String sourceDir = ctx.getApplicationInfo().sourceDir;
        JarFile containerJar = null;

        try {
            containerJar = new JarFile(sourceDir);
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        Log.d(TAG, "APK path = "+sourceDir);

        Enumeration<JarEntry> entries =  containerJar.entries();
        while(entries.hasMoreElements()) {
            JarEntry e = entries.nextElement();
            Log.d(TAG, "APK Entry = "+e.getName());
        }
    }

    public static byte[] readFileinAPK(Context ctx, String file) {
        try {
            String sourceDir = ctx.getApplicationInfo().sourceDir;
            JarFile containerJar = null;
            containerJar = new JarFile(sourceDir);
            ZipEntry ze = containerJar.getEntry(file);
            byte[] bytes = new byte[(int)ze.getSize()];
            if (ze != null) {
                InputStream in = containerJar.getInputStream(ze);

                in.read(bytes);

                Log.d(TAG, String.format("Device Information [%s] [%s]", Build.MODEL, Build.DEVICE));
                Log.d(TAG, String.format("APK path=[%s] File Infor=[%s, %d, %d]", sourceDir, ze.getName(), ze.getSize(), ze.getCompressedSize()));

                /*
                Formatter formatter = new Formatter();
                for (byte b : bytes) {
                    formatter.format("%02x ", b);
                }
                String hex = formatter.toString();
                Log.d(TAG, "File Content :"+hex);
                */

                return bytes;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        return null;
    }

    public static String getMD5(final byte[] bytes, int offsets) 	{
        String MD5 = "";
        try{
            MessageDigest md = MessageDigest.getInstance("MD5");
            md.update(bytes, offsets, bytes.length-offsets);
            byte byteData[] = md.digest();
            StringBuffer sb = new StringBuffer();
            for(int i = 0 ; i < byteData.length ; i++){
                sb.append(Integer.toString((byteData[i]&0xff) + 0x100, 16).substring(1));
            }
            MD5 = sb.toString();

        }catch(NoSuchAlgorithmException e){
            e.printStackTrace();
            MD5 = null;
        }
        return MD5;
    }

    public static String getAPKEntryMD5(Context ctx, final String file, int offset) {
        byte[] content = readFileinAPK(ctx, file);
        return getMD5(content, offset);
    }

    public static void showSupportedCodecList() {
        MediaCodecList mcl = new MediaCodecList(MediaCodecList.REGULAR_CODECS);
        for(  MediaCodecInfo codecInfo : mcl.getCodecInfos()) {
            String[] types = codecInfo.getSupportedTypes();

            for (int i = 0; i < types.length; i++) {
                MediaCodecInfo.CodecCapabilities capabilities = codecInfo.getCapabilitiesForType(types[i]);
                Log.d(TAG, String.format("Codec = [%s,%s,%s]", codecInfo.isEncoder()==true?"Encoder":"Decoder", codecInfo.getName(), types[i]));
            }
        }
    }

    public static boolean isSupportedClip(String codecType, int width, int height, int framerate) {
        if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.LOLLIPOP) {
            return false;
        }
        MediaCodecList mcl = new MediaCodecList(MediaCodecList.REGULAR_CODECS);
        for (MediaCodecInfo codecInfo : mcl.getCodecInfos()) {
            if (codecInfo.isEncoder()) {
                //Log.d(TAG, String.format("Skip encocder! [%s]", codecInfo.getName()));
                continue;
            }
            String[] types = codecInfo.getSupportedTypes();

            for (int i = 0; i < types.length && types[i].contains("video/"); i++) {
                if (types[i].contains(codecType)
                        || (codecType.equalsIgnoreCase("video/h264") && types[i].equalsIgnoreCase("video/avc"))
                        || (codecType.equalsIgnoreCase("video/avc") && types[i].equalsIgnoreCase("video/h264"))
                        || (codecType.equalsIgnoreCase("video/mpeg4v") && types[i].contains("video/mp4v")) ) {
                    Log.d(TAG, String.format("Codec information is matched! [%s,%s,%s]", codecType, types[i], codecInfo.getName()));

                } else {
                    //Log.d(TAG, String.format("Codec information is mismatched! [%s,%s,%s]", codecType, types[i], codecInfo.getName()));
                    continue;
                }

                MediaCodecInfo.CodecCapabilities capabilities = codecInfo.getCapabilitiesForType(types[i]);
                MediaCodecInfo.VideoCapabilities videoCapabilities = capabilities.getVideoCapabilities();

                if (videoCapabilities == null) {
                    continue;
                }

                if (videoCapabilities.areSizeAndRateSupported(width, height, framerate)) {
                    Log.d(TAG, String.format("the MediaCodecInfo name is: %s, and the MediaCodecInfo type is %s. This phone can edit this clip.", codecInfo.getName(), types[i]));
                    return true;
                }
            }
        }
        Log.d(TAG, String.format("This phone can't edit this clip[%s,%d,%d,%d].", codecType, width, height, framerate));
        return false;
    }
}
