/******************************************************************************
 * File Name        : ApiDemosConfig.java
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

package com.nexstreaming.nexeditorsdkapis;

import android.app.Application;
import android.content.Context;
import android.util.Log;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexConfig;
import com.nexstreaming.nexeditorsdk.nexEngine;
//import com.nexstreaming.nexeditorsdkapis.modules.FaceDetectorSZ;
import static com.nexstreaming.nexeditorsdk.nexApplicationConfig.letterbox_effect_black;

/**
 * Created by jeongwook.yoon on 2015-03-05.
 */
public class ApiDemosConfig extends Application {
    private static final String LOG_TAG = "ApiDemosConfig";
    private static ApiDemosConfig instance;
    private nexEngine mEngin = null;
    private Context mContext;
    private Object appLock = new Object();
    private boolean mIsNexEditorInitialized = false;

    public static ApiDemosConfig getApplicationInstance() {
        if( instance == null ) {
            Log.e(LOG_TAG, "getApplicationInstance : Returning NULL!");
        }
        return instance;
    }

    public ApiDemosConfig() {
        super();
        instance = this;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mContext = getApplicationContext();
        initApp();
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
        Log.d(LOG_TAG,"call onLowMemory");
    }

    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);
        Log.d(LOG_TAG,"call onTrimMemory level="+level);
    }

    public void initApp() {
        synchronized (appLock) {
            if (mIsNexEditorInitialized == false) {
                Log.d(LOG_TAG, "KMSDK Initialize !!");

                nexApplicationConfig.createApp(mContext);
                nexConfig.set(3840*2160*3/2*2, 4, 250, false, 3840*2160);
                nexConfig.setProperty(nexConfig.kDeviceMaxLightLevel,600);
                nexConfig.setProperty(nexConfig.kDeviceMaxGamma,2400);
                //nexConfig.setProperty(nexConfig.kNativeLogLevel,0);
                nexApplicationConfig.init(mContext, "nexdemo");
                nexApplicationConfig.setDefaultLetterboxEffect(letterbox_effect_black);

                //FaceDetectorSZ.createInstance(mContext);
                //nexApplicationConfig.getExternalModuleManager().registerModule( "com.nexstreaming.nexeditorsdkapis.modules.FaceDetectorSZ");
                nexApplicationConfig.getExternalModuleManager().registerModule( "com.nexstreaming.nexeditorsdkapis.modules.FaceDetectorExt");
                nexApplicationConfig.getExternalModuleManager().registerModule( "com.nexstreaming.nexeditorsdkapis.modules.AniGifExport");
                mIsNexEditorInitialized = true;
            } else {
                Log.d(LOG_TAG, "already KMSDK Initialized !!");
            }
        }
    }

    public void releaseAPP() {
        synchronized (appLock) {
            if( mEngin != null) {
                mEngin.stop();
                mEngin = null;
                nexApplicationConfig.releaseApp();
                mIsNexEditorInitialized = false;
            }
        }
    }


    public nexEngine getEngine() {
        synchronized (appLock) {
            if (mEngin == null) {
                Log.d(LOG_TAG, "getEditor : creating editor instance");
                createtEngine();
            }
            return mEngin;
        }
    }

    private void createtEngine( ) {
        if( mEngin != null )
            return;
        mEngin = new nexEngine(mContext);
//        nexEngine.setExportVideoTrackUUID(true);
        mEngin.setLoadListAsync(true);
    }

    public void releaseEngine(){
        synchronized (appLock) {
            if( mEngin != null) {
                mEngin.stop();
                mEngin = null;
                nexApplicationConfig.releaseNativeEngine();
                mIsNexEditorInitialized = false;
            }
        }
    }
}
