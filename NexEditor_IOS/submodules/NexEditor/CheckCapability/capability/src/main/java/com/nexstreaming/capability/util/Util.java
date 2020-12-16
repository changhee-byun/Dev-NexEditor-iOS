package com.nexstreaming.capability.util;

import android.os.Environment;

import java.io.File;

/**
 * Created by Eric on 2015-11-04.
 */public class Util {

    /**
     * Like {@link android.os.Build.VERSION#SDK_INT}, but in a place where it can be conveniently
     * overridden for local testing.
     */
    public static final int SDK_INIT = android.os.Build.VERSION.SDK_INT;

    /**
     * Like {@link android.os.Build#DEVICE}, but in a place where it can be conveniently overridden
     * for local testing.
     */
    public static final String DEVICE = android.os.Build.DEVICE;

    /**
     * Like {@link android.os.Build#MANUFACTURER}, but in a place where it can be conveniently
     * overridden for local testing.
     */
    public static final String MANUFACTURER = android.os.Build.MANUFACTURER;

    /**
     * Like {@link android.os.Build#MODEL}, but in a place where it can be conveniently overridden for
     * local testing.
     */
    public static final String MODEL = android.os.Build.MODEL;

    public static boolean checkExternalMemory()
    {
        String state = Environment.getExternalStorageState();

        if (Environment.MEDIA_MOUNTED.equals(state))
        {
            File f = new File(Environment.getExternalStorageDirectory().getAbsolutePath());
            if (f.canRead() && f.canWrite())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        return false;
    }
}
