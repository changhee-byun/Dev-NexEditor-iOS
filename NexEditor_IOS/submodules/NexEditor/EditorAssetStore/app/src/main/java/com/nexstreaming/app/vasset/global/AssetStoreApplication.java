package com.nexstreaming.app.vasset.global;

import android.app.Application;
import android.util.Log;

/**
 * Created by ojin.kwon on 2016-11-17.
 */

public class AssetStoreApplication extends Application{

    private static final String TAG = "AssetStoreApplication";

    @Override
    public void onCreate() {
        super.onCreate();
        if(LL.D) Log.d(TAG, "onCreate: ");
    }
    @Override
    public void onTerminate() {
        super.onTerminate();
        Log.d(TAG, "onTerminate()");
    }
}
