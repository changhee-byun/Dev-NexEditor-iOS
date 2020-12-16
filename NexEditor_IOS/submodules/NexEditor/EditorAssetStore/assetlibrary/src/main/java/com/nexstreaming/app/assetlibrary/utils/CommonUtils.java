package com.nexstreaming.app.assetlibrary.utils;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.util.Log;

import com.nexstreaming.app.assetlibrary.LL;

/**
 * Created by ojin.kwon on 2016-11-21.
 */

public class CommonUtils {

    public static String formatFileSize(Context ctx, long size) {
        if( size < 1 ) {
            return "?";
        }
        if( size<1024 ) {
            return size + " B";
        }
        int exp = (int)(Math.log10(size) / 3.0103);
        return String.format(ctx.getResources().getConfiguration().locale,"%.1f %sB", size / Math.pow(1024, exp), "KMGTPE".charAt(exp-1));
    }

    /**
     * Check network connectivity (wi-fi)
     * @param context
     * @return connected state
     */
    public static boolean isWifiConnected(Context context) {
        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetworkInfo = connectivityManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        if(activeNetworkInfo != null && activeNetworkInfo.isAvailable() && activeNetworkInfo.isConnected()) return true;
        if(LL.D) Log.d("NetworkManager.java", "isWifiConnected : false");
        return false;
    }

    /**
     * Check network connectivity (mobile)
     * @param context
     * @return connected state
     */
    public static boolean isMobileConnected(Context context) {
        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = connectivityManager.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        if(networkInfo != null && networkInfo.isAvailable() && networkInfo.isConnected()) {
            if(LL.D) Log.d("NetworkManager.java", "isMobileConnected : true");
            return true;
        }
        if(LL.D) Log.d("NetworkManager.java", "isMobileConnected : false");
        return false;
    }

    /**
     * Check network connectivity (mobile, wi-fi)
     * @return connected state
     */
    public static boolean isOnline(Context context) { // network 연결 상태 확인
        // 08 Nov 2016, mark.lee
        if (context == null)
            return false;

        boolean connected = false;

        if(isWifiConnected(context))
            return true;

        if(isMobileConnected(context))
            return true;

        return connected;
    }

    public static String getLanguage(Context context){
        return context.getResources().getConfiguration().locale.getLanguage();
    }
}
