/******************************************************************************
 * File Name        : nexApplicationConfig.java
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

package com.nexstreaming.nexeditorsdk;

import android.content.Context;
import android.os.AsyncTask;
import android.media.MediaCodecList;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.MainThread;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.security.SecurityProvider;
import com.nexstreaming.app.common.nexasset.assetpackage.security.provider.BasicEncryptionProvider;
import com.nexstreaming.app.common.nexasset.assetpackage.security.provider.BasicEncryptionProviderKeySet;
import com.nexstreaming.app.common.nexasset.store.AssetLocalInstallDB;
import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.config.NexEditorDeviceProfile;
import com.nexstreaming.kminternal.kinemaster.editorwrapper.LookUpTable;
import com.nexstreaming.kminternal.kinemaster.mediainfo.MediaInfo;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;
import com.nexstreaming.kminternal.nexvideoeditor.NexThemeView;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

/**
 * This class initializes the NexEditor&trade;&nbsp;SDK.
 * {@link #init(android.content.Context, String)} } must be called first when creating the application.
 * Since the NexEditor&trade;&nbsp;SDK operates using a Singleton class, {@link #createApp(android.content.Context)}
 * must be created when making the application.
 * <p>Example code:</p>
 *     {@code public class KMSDKApplication extends Application {
            public void onCreate() {
                nexApplicationConfig.createApp(this);
                nexApplicationConfig.init(getApplicationContext(),"application naming");
            }
        }
   }
 * @since version 1.0.0
 */
public final class nexApplicationConfig {
    static private KineMasterSingleTon KMSingle;
    static private String TAG = "nexApplicationConfig";
    private static boolean pendingDevAssetNotification = false;
    private static String pendingDevAssetLoadError = null;
    private static SecurityProvider[] securityProvider = null;
    private static long AppAssetPackageVersionCode = BuildConfig.BUILD_DATE;
    private static nexAspectProfile aspectProfile =  nexAspectProfile.ar16v9;

    public static final int kAspectRatio_Mode_free = 0;
    /**
     * This indicates the possible mode of <tt>kAspectRatio</tt>. Set to this mode for a 16x9 screen ratio.  
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.3.0
     */
    public static final int kAspectRatio_Mode_16v9 = 1;

    /**
     * This indicates the possible mode of <tt>kAspecRatio</tt>. Set to this mode for a 1x1 screen ratio.
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.3.0
     */
    public static final int kAspectRatio_Mode_1v1 = 2;
    

    /**
     * This indicates the possible mode of <tt>kAspecRatio</tt>. Set to this mode for a 9x16 screen ratio. 
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.3.0
     */
    public static final int kAspectRatio_Mode_9v16 = 3;

    /**
     * This indicates the possible mode of <tt>kAspecRatio</tt>. Set to this mode for a 2x1 screen ratio.
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.5.31
     */
    public static final int kAspectRatio_Mode_2v1 = 4;

    /**
     * This indicates the possible mode of <tt>kAspecRatio</tt>. Set to this mode for a 1x2 screen ratio.
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.7.3
     */
    public static final int kAspectRatio_Mode_1v2 = 5;

    /**
     * This indicates the possible mode of <tt>kAspecRatio</tt>. Set to this mode for a 4x3 screen ratio.
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.7.7
     */
    public static final int kAspectRatio_Mode_4v3 = 6;

    /**
     * This indicates the possible mode of <tt>kAspecRatio</tt>. Set to this mode for a 3x4 screen ratio.
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.7.7
     */
    public static final int kAspectRatio_Mode_3v4 = 7;

    /**
     * This indicates the possible mode of <tt>kScreenMode</tt>. Set to this mode for normal screen. 
     * 
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.3.0
     */
    public static final int kScreenMode_normal = 0;

    private static int sAspectRatioMode = kAspectRatio_Mode_16v9;
    /**
     * This indicates the possible mode of <tt>kScreenMode</tt>. Set to this mode for horizontal dual screen.  
     * 
     * @see com.nexstreaming.nexeditorsdk.nexAnimate.MoveTrackingPath
     * @since version 1.3.0
     */
    public static final int kScreenMode_horizonDual = 1;

    private static int sScreenMode = kScreenMode_normal;


    public static final int kOverlayCoordinateMode_ScreenDimention = 0;
    public static final int kOverlayCoordinateMode_Stretching = 1;
    private static int sOverlayCoordinateMode = kOverlayCoordinateMode_ScreenDimention;

    /**
     *
     * @see #setDefaultLetterboxEffect(String)
     * @see #getDefaultLetterboxEffect()
     * @since version 2.0.0
     */
    public static String letterbox_effect_black = "black";

    /**
     *
     * @see #setDefaultLetterboxEffect(String)
     * @see #getDefaultLetterboxEffect()
     * @since version 2.0.0
     */
    public static String letterbox_effect_blur10 = "com.nexstreaming.editor.fastblurOpt";

    private static String default_letterbox_effect = letterbox_effect_black;

    /**
     * This method creates a singleton instance.
     * This method must be called only once before using the NexEditor&trade;&nbsp;SDK.
     *
     * <p>Example code:</p>
     * {@code public class KMSDKApplication extends Application {
             public KMSDKApplication() {
                  super();
                  nexApplicationConfig.createApp(this);
              }
         }
     * }
     * @param Context The Android context.
     * @since version 1.0.0
     */
    public synchronized static void createApp(Context Context ){
        if(LL.D) Log.d(TAG,"call createApp tagetsdk="+Context.getApplicationInfo().targetSdkVersion);
        if(KMSingle == null) {
            KMSingle = new KineMasterSingleTon(Context);

            new Thread(new Runnable(){

                @Override
                public void run(){

                    int numCodecs = MediaCodecList.getCodecCount();
                };
            }
            ).start();
        }
    }

    /**
     * This method initializes <tt>EffectLibrary</tt> and the cache directory.
     * This method must be called once when starting the application.
     *
     * <p>Example code :</p>
     {@code public class KMSDKApplication extends Application {
        public void onCreate() {
            nexApplicationConfig.init(getApplicationContext(),"nexsdkapp");
            super.onCreate();
        }
     }
     }
     * @param AppContext The Android context.
     * @param AppName External storage path name.
     * @see #init(android.content.Context, android.content.Context, String, java.io.File, String)
     * @see #init(android.content.Context, String, java.io.File, String)
     * @see #init(android.content.Context, String, String)
     * @since version 1.0.0
     */
    public static void init(Context AppContext , String AppName){
        init(AppContext,AppName,null,null);
    }

    /**
     * This method initializes <tt>EffectLibrary</tt> and the cache directory.
     * This method must be called once when starting the application.
     *
     * @param AppContext The Android context.
     * @param AppName External storage path name.
     * @param SDK_PubKey OEM Theme public key as a <tt>String</tt>.
     * @see #init(android.content.Context, android.content.Context, String, java.io.File, String)
     * @see #init(android.content.Context, String, java.io.File, String)
     * @see #init(android.content.Context, String)
     * @since version 1.3.0
     */
    public static void init(Context AppContext , String AppName, String SDK_PubKey){
        init(AppContext,AppName,null,SDK_PubKey);
    }

    /**
     * This method initializes <tt>EffectLibrary</tt> and the cache directory.
     * This method must be called once when starting the application.
     *
     * @param AppContext The Android context.
     * @param AppName External storage path name.
     * @param PluginPath The theme plug-in path. This path will be made automatically if set to <tt>null</tt>. 
     * @param SDK_PubKey OEM Theme public key as a <tt>String</tt>.
     * @see #init(android.content.Context, android.content.Context, String, java.io.File, String)
     * @see #init(android.content.Context, String, java.io.File, String)
     * @see #init(android.content.Context, String)
     * @since version 1.3.4
     */
    public static void init(Context AppContext , String AppName, File PluginPath ,String SDK_PubKey){
        init( AppContext , null ,  AppName,  PluginPath , SDK_PubKey);
    }

    /**
     * This method initializes <tt>EffectLibrary</tt> and the cache directory.
     * This method must be called once when starting the application.
     *
     * @param AppContext The Android context.
     * @param otherThemeContext Public android context from other APKs.
     * @param AppName External storage path name.
     * @param PluginPath The theme plug-in path. This path will be made automatically if set to <tt>null</tt>. 
     * @param SDK_PubKey OEM Theme public key as a <tt>String</tt>.
     * @see #init(android.content.Context, String, String)
     * @see #init(android.content.Context, String, java.io.File, String)
     * @see #init(android.content.Context, String)
     * @since version 1.3.4
     */
    public static void init(Context AppContext , Context otherThemeContext , String AppName, File PluginPath ,String SDK_PubKey){

        Log.i(TAG,"KMSDK Version :"+getSDKVersion());

        //nexAssetStoreAppUtils.makeConfigAsync();
        AssetLocalInstallDB.getInstance(AppContext);
        EditorGlobal.setAppName(AppName);
//        Map<String,String> serverParams = new HashMap<String,String>();
//        serverParams.put("marketid", EditorGlobal.getMarketId());

/* todo:
        if( PluginPath != null ) {
            EffectLibrary.setPluginsDir(PluginPath);
        }else{
            EffectLibrary.setPluginsDir(EditorGlobal.getPluginDirectory());
        }
*/

// mjkong: delete for duplicated code.
//        EffectLibrary.getEffectLibrary(AppContext);
        MediaInfo.setCacheDirFromContext(AppContext);

        /* Capability JsonData Setting & Parsing */
        //nexConfig.setCapability(null);

        if(nexConfig.sConfigProperty[nexConfig.kSetUserConfig] != 0 ){
            NexEditorDeviceProfile.setAppContext(AppContext,nexConfig.sConfigProperty);
        }else{
            NexEditorDeviceProfile.setAppContext(AppContext,null);
        }

        //NexEditor.setApplicationPacakge4Protection(AppContext.getPackageName());

        if( securityProvider == null ){
            int i=-1;
            BasicEncryptionProviderKeySet s = new BasicEncryptionProviderKeySet();
            securityProvider = new SecurityProvider[s.getpKeyMap().size()];
            for ( Map.Entry<int[], String[]> entry : s.getpKeyMap().entrySet() ) {
                securityProvider[++i] = new BasicEncryptionProvider(entry.getKey(), entry.getValue());
                if (securityProvider[i] != null) {
                    AssetPackageReader.registerSecurityProvider(securityProvider[i]);
                }
            }
            if(LL.D) Log.d(TAG,"Register " + securityProvider.length + " security provider!");
        }

        if( s_asyncInitDBTask == null ) {
            loadInitDB(AppContext,PluginPath);
        }else{
            new AsyncTask<Context,Void,Void>(){

                @Override
                protected Void doInBackground(Context... params) {
                    Context AppContext = params[0];
                    loadInitDB(AppContext,null);
                    return null;
                }

                @Override
                protected void onPostExecute(Void aVoid) {
                    s_asyncInitDBTask.signalEvent(Task.Event.COMPLETE);
                }
            }.executeOnExecutor(sInitAssetDBThreadPool,AppContext);
        }
    }

    /**
     * This method must be called once when starting the application instead of {@link #init(Context, String, String)}
     * And after must call {@link #initAssetDB(Context, File)}
     * @param AppContext
     * @param AppName
     * @since 2.0.9
     * @see #initAssetDB(Context, File)
     */
    public static void initMaster(Context AppContext , String AppName){
        Log.d(TAG,"initMaster nexEditorSDK Version :"+getSDKVersion());
        EditorGlobal.setAppName(AppName);
        MediaInfo.setCacheDirFromContext(AppContext);
        if(nexConfig.sConfigProperty[nexConfig.kSetUserConfig] != 0 ){
            NexEditorDeviceProfile.setAppContext(AppContext,nexConfig.sConfigProperty);
        }else{
            NexEditorDeviceProfile.setAppContext(AppContext,null);
        }
        Log.d(TAG,"initMaster End");
    }

    /**
     * This method must be called once when starting the application.
     * called {@link #initMaster(Context, String)} api next time next.
     * @param AppContext
     * @param PluginPath
     * @since 2.0.9
     * @see #initMaster(Context, String)
     */
    public static void initAssetDB(Context AppContext,File PluginPath){
        Log.d(TAG,"initAssetDB start");
        AssetLocalInstallDB.getInstance(AppContext);
        if( securityProvider == null ){
            int i=-1;
            BasicEncryptionProviderKeySet s = new BasicEncryptionProviderKeySet();
            securityProvider = new SecurityProvider[s.getpKeyMap().size()];
            for ( Map.Entry<int[], String[]> entry : s.getpKeyMap().entrySet() ) {
                securityProvider[++i] = new BasicEncryptionProvider(entry.getKey(), entry.getValue());
                if (securityProvider[i] != null) {
                    AssetPackageReader.registerSecurityProvider(securityProvider[i]);
                }
            }
            if(LL.D) Log.d(TAG,"Register " + securityProvider.length + " security provider!");
        }

        if( s_asyncInitDBTask == null ) {
            loadInitDB(AppContext,PluginPath);
        }else{
            new AsyncTask<Context,Void,Void>(){

                @Override
                protected Void doInBackground(Context... params) {
                    Context AppContext = params[0];
                    loadInitDB(AppContext,null);
                    return null;
                }

                @Override
                protected void onPostExecute(Void aVoid) {
                    s_asyncInitDBTask.signalEvent(Task.Event.COMPLETE);
                }
            }.executeOnExecutor(sInitAssetDBThreadPool,AppContext);
        }
        Log.d(TAG,"initAssetDB End");
    }

    private static void loadInitDB(Context AppContext, File PluginPath){
        if ( nexConfig.getProperty(nexConfig.kIsDevicePreview) == 0 ) {
            try {
                // MATTFIX: This should probably be done in another thread
                AssetPackageManager.getInstance(AppContext).syncPackagesFromAndroidAssets(AppContext,"kmassets",AppAssetPackageVersionCode);
            } catch (IOException e) {
                throw new RuntimeException("Cannot read prepackaged assets",e);
            }
        }

        try {
            // MATTFIX: This should probably be done in another thread
            if( PluginPath != null ) {
                AssetPackageManager.getInstance(AppContext).unregisterPackagesLoadedFromFolder(PluginPath); // Force re-registering
                pendingDevAssetNotification = AssetPackageManager.getInstance(AppContext).syncPackagesFromFolder(PluginPath);
            }else{
                if ( nexConfig.getProperty(nexConfig.kIsDevicePreview) == 0 ) {
                    AssetPackageManager.getInstance(AppContext).unregisterPackagesLoadedFromFolder(EditorGlobal.getPluginAssetDirectory()); // Force re-registering
                    pendingDevAssetNotification = AssetPackageManager.getInstance(AppContext).syncPackagesFromFolder(EditorGlobal.getPluginAssetDirectory());
                } else {
                    AssetPackageManager.getInstance(AppContext).unregisterPackagesLoadedFromFolder(EditorGlobal.getPluginAssetEffectDirectory(AppContext)); // Force re-registering
                    pendingDevAssetNotification = AssetPackageManager.getInstance(AppContext).syncPackagesFromFolder(EditorGlobal.getPluginAssetEffectDirectory(AppContext));
                    AssetPackageManager.getInstance(AppContext).unregisterPackagesLoadedFromFolder(EditorGlobal.getPluginAssetTranstionDirectory(AppContext)); // Force re-registering
                    pendingDevAssetNotification = AssetPackageManager.getInstance(AppContext).syncPackagesFromFolder(EditorGlobal.getPluginAssetTranstionDirectory(AppContext));
                }
            }

        } catch (IOException e) {
            pendingDevAssetLoadError = e.getLocalizedMessage();
        }
        //AssetPackageManager.getInstance(AppContext).unregisterPackagesLoadedFromFolder(EditorGlobal.getPluginAssetDirectory());
        //AssetLocalInstallDB.getInstance(AppContext).checkInstallDB();
        //AssetLocalInstallDB.getInstance(AppContext).uninstallFromAssetStoreApp();

        LookUpTable.getLookUpTable(AppContext);
        nexOverlayPreset.getOverlayPreset(AppContext);

/*
        List<? extends ItemInfo> infos = AssetPackageManager.getInstance().getInstalledItems();

        for( ItemInfo info :  infos ){
            Log.d(TAG, "item id=" + info.getId() + ",type=" + info.getType() + ",Category=" + info.getCategory() + ",Asset ID=" + info.getAssetPackage().getAssetId() + ",path=" + info.getPackageURI());
            AssetInfo assetInfo = info.getAssetPackage();

            if( assetInfo != null ) {
                //AssetPackageManager.getInstance().
//                Log.d(TAG,"Asset type="+ AssetPackageManager.getInstance().getInstalledPackageInfoByAssetIdx(assetInfo.getAssetIdx()).getInstallSourceType() );
            }
        }
*/

    }

    private static final Executor sInitAssetDBThreadPool = Executors.newSingleThreadExecutor();
    private static Task s_asyncInitDBTask;

    /**
     * async load asset to DB in app.
     * note : must call before init() function.
     *
     * <p>Example code :</p>
     {@code nexApplicationConfig.asyncLoadAssetDB(null);
     nexApplicationConfig.init(mContext, "nexdemo");
     nexApplicationConfig.waitForLoading(mContext,new Runnable() {
     @Override
     public void run() {
        //TODO: load complate.
     }
     });
     }

     *
     * @param onCompleteListener when asset loaded call the run.
     * @see #init(Context, String)
     * @see #init(Context, Context, String, File, String)
     * @see #init(Context, String, File, String)
     * @see #init(Context, String, String)
     * @see #waitForLoading(Context, Runnable)
     * @since 2.0.7
     */
    public static void asyncLoadAssetDB(final Runnable onCompleteListener ){
        if( s_asyncInitDBTask == null ){
            s_asyncInitDBTask = new Task();
        }
        if( onCompleteListener != null ) {
            s_asyncInitDBTask.onComplete(new Task.OnTaskEventListener() {
                @Override
                public void onTaskEvent(Task t, Task.Event e) {
                    new Handler(Looper.getMainLooper()).post(onCompleteListener);
                }
            });
        }
    }
    /**
     * This method clears the resources of NexEditor&trade;&nbsp;SDK.
     *
     * This method must be called when the activity is destroyed. 
     *
     * @since 1.1.0
     */
    public static void releaseApp() {
        if(LL.D) Log.d(TAG,"call releaseApp");

/* Fixed NESA-1381
        if( securityProvider != null ) {
            for (SecurityProvider entry : securityProvider) {
                if (entry != null) {
                    AssetPackageReader.unregisterSecurityProvider(entry);
                    entry = null;
                }
            }
            if(LL.D) Log.d(TAG,"UnRegister " + securityProvider.length + " security provider!");
            securityProvider = null;
        }
*/

        if(LookUpTable.getLookUpTable() != null) {
            LookUpTable.getLookUpTable().releaseResource2LookUpTable();
        }

        nexAssetStoreAppUtils.disconnectAssetStoreAppService();

        releaseNativeEngine();
    }

    /**
     * Only Engine was released.
     * @since version 1.7.57
     */
    public static void releaseNativeEngine(){
        if( KMSingle != null ) {
            KMSingle.releaseEditor();
        }
    }

//    private static Task.MultiplexTask sLoadingTask = null;
//    private static Task.MultiplexTask sMinimumLoadingTask = null;

    /**
     * This method indicates the completion time of the effect library's initialization process with <b>Runnable</b>.
     * The effect library can be used only after the initialization process has completed.
     * <p>Example code :</p>
     *  {@code nexApplicationConfig.waitForLoading(this, new Runnable() {
            public void run() {
                setContentView(R.layout.activity_main);
                nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(MainActivity.this);
                ......
            }
        });
        }
     * @param context  The Android context.
     * @param listener  The runnable thread.
     * @see #waitForMinimumLoading(android.content.Context, Runnable)
     * @since version 1.0.0
     */
    public static void waitForLoading( Context context, final Runnable listener ) {
        if(LL.D) Log.d(TAG,"call waitForLoading");

        if( s_asyncInitDBTask != null ) {
            s_asyncInitDBTask.onComplete(new Task.OnTaskEventListener() {
                @Override
                public void onTaskEvent(Task t, Task.Event e) {
                    listener.run();
                }
            });
        }else {
            if (listener != null)
                listener.run();
        }
        if(LL.D) Log.d(TAG,"call waitForLoading end");
    }
 
    /**
     * This method gets only the essential files needed for playback.
     * when importing the entire theme and transition effect files takes too long.
     *  
     * <p>Example code :</p>
     *  {@code nexApplicationConfig.waitForMinimumLoading(this, new Runnable() {
            public void run() {
                setContentView(R.layout.activity_main);
                nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(MainActivity.this);
                ......
            }
        });
    }
     * @param context The Android context.
     * @param listener The runnable thread.
     * @see #waitForLoading(android.content.Context, Runnable)
     * @since version 1.3.43
     */
    @Deprecated
    public static void waitForMinimumLoading( Context context, final Runnable listener ) {
        if(LL.D) Log.d(TAG,"call waitForMinimumLoading");
        waitForLoading(context,listener);
    }

    /**
     * A listener class of setEffectLoadListener().
     * For more details, please refer to {@link #setEffectLoadListener(OnEffectLoadListener)}.
     *
     * @see #setEffectLoadListener(OnEffectLoadListener)
     * @since 1.5.42
     */
    @Deprecated
    public static abstract class OnEffectLoadListener{
        /**
         * Called when loading a high-priority effect has finished.
         * @since 1.5.42
         */
        public abstract void onEffectPriorityLoadComplete();

        /**
         * Called when loading all effects has finished.
         * @since 1.5.42
         */
        public abstract void onEffectLoadComplete();

        /**
         * Returns the name of the KMT package that just finished loading.
         * @param packageDesc The name of the KMT package that has loaded.
         * @param error The error value from loading. Zero for success, or a non-zero NexEditor&trade;&nbsp; error code in the event of a failure.
         * @since 1.5.42
         */
        public abstract void onEffectLoad(String packageDesc, int error);
    }

//    private static OnEffectLoadListener sLoadListener;

    /**
     * If {@link #waitForLoading(Context, Runnable)} or {@link #waitForMinimumLoading(Context, Runnable)} is called, effect loading will begin.
     * To check package completion of the effects being loaded, use this API.
     *
     * <p>Example code :</p>
     *    {@code nexApplicationConfig.setEffectLoadListener(new nexApplicationConfig.OnEffectLoadListener() {
            public void onEffectPriorityLoadComplete() { }

            public void onEffectLoadComplete() { Log.d(LOG_TAG,"EffectLoadComplete"); }

            public void onEffectLoad(String s, int i) { Log.d(LOG_TAG,"onEffectLoad="+s); }
        });
        }
     * @param listener OnEffectLoadListener
     * @since 1.5.42
     */
    @Deprecated
    public static void setEffectLoadListener(OnEffectLoadListener listener){
        //sLoadListener = listener;

    }

    private final static int SDK_VERSION_Major = 2;
    private final static int SDK_VERSION_Minor = 12;
    private final static int SDK_VERSION_Patch = 41; 
    //private final static int SDK_VERSION_DevCode = 2;
	
    /**
     * This method gets the version information of the NexEditor&trade;&nbsp;SDK as a <tt>String</tt>. 
     * 
     * @return The version of the NexEditor&trade;&nbsp;SDK in use, in the format <tt>Major.Minor.Patch</tt>.
     * @since version 1.0.0
     * @see #getDevelopString()
     */
    public static String getSDKVersion() {
        return String.format("%d.%d.%d",SDK_VERSION_Major,SDK_VERSION_Minor,SDK_VERSION_Patch);
    }

    /**
     * This method gets the build version of the NexEditor&trade;&nbsp;SDK.
     * The build version is a 4 digit number and it will be reset when the SDK version is updated.
     * @return <tt>Official</tt> if the version is official; otherwise <tt>Develop</tt>. 
     * @since version 1.3.0
     */
    public static String getDevelopString(){
        if((SDK_VERSION_Minor %2) ==0 ){
            return "Develop";
        }
        return "Official";
    }

    /**
     * This method sets the aspect ratio of the output screen. 
     *
     * <p>Example code :</p>
     *  {@code
            nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_16v9);
        }
     * @param aspectRatioMode  The aspect ratio mode to set, as an <tt>integer</tt>.  This will be 
     *                         either 1 for <tt>kAspectRatio_Mode_16v9</tt> or 2 for <tt>kAspectRatio_Mode_1v1</tt>.
     * 
     * @see #getAspectRatioMode()
     * @see #getAspectRatio()
     * @since version 1.1.0
     * 
     */
    public static void setAspectMode(int aspectRatioMode ){
        aspectProfile = nexAspectProfile.getAspectProfile(aspectRatioMode);
        if( aspectProfile != null) {
            sAspectRatioMode = aspectRatioMode;
            NexThemeView.setAspectRatio(aspectProfile.getAspectRatio());
        }
    }

    /**
     * This method gets the aspect ratio of the output screen. 
     *
     * <p>Example code :</p>
     *  {@code
            nexApplicationConfig.getAspectMode();
        }
     * @return  1 if set to <tt>kAspectRatio_Mode_16v9</tt>; 2 if set to <tt>kAspectRatio_Mode_1v1</tt>; 3 if set to <tt>kAspectRatio_Mode_9v16</tt>; 4 if set to <tt>kAspectRatio_Mode_2v1</tt>; 5 if set to <tt>kAspectRatio_Mode_1v2</tt>; 6 if set to <tt>kAspectRatio_Mode_4v3</tt>; 7 if set to <tt>kAspectRatio_Mode_3v4</tt>.
     * 
     * @see #setAspectMode()
     * @since version 2.1.5
     * 
     */
	public static int getAspectMode() {
		return sAspectRatioMode;
	}

    /** 
     *  This method gets the aspect ratio of the output screen as a <tt>float</tt>.  
     * 
     * <p>Example code :</p>
     *  {@code    nexEffectPreviewView(Context context) {
                super(context);
                super.setAspectRatio(nexApplicationConfig.getAspectRatio());
            }
        }
     * @return  1 if set to <tt>kAspectRatio_Mode_1v1</tt>; 2 if set to <tt>kAspectRatio_Mode_16v9</tt>.
     * 
     * @see #setAspectMode(int)
     * @see #getAspectRatioMode()
     * @since version 1.1.0
     * 
     */
    public static float getAspectRatio(){
        if( aspectProfile != null ){
            return aspectProfile.getAspectRatio();
        }
 /*
        if( sAspectRatioMode == kAspectRatio_Mode_1v1 ) {
            return 1f;
        }else if( sAspectRatioMode == kAspectRatio_Mode_9v16 ){
            return 9f/16f;
        }else if( sAspectRatioMode == kAspectRatio_Mode_2v1 ){
            return 2f/1f;
        }else if( sAspectRatioMode == kAspectRatio_Mode_1v2 ){
            return 1f/2f;
        }
        */
        return 16f/9f;
    }
 
    /**
     * This method gets the aspect ratio of the output screen. The screen width will be doubled in the dual screen mode. 
     * 
     * <p>Example code :</p>
     *  {@code    public nexEngineView(Context context) {
                super(context);
                super.setAspectRatio(nexApplicationConfig.getAspectRatioInScreenMode());
            }
        }
     * @return The aspect ratio of the output screen in the format of width / height. 
     * @since version 1.3.52
     */
    public static float getAspectRatioInScreenMode(){
        float w = 16f;
        float h = 9f;
        if( aspectProfile != null ){
            w = aspectProfile.getWidth();
            h = aspectProfile.getHeight();
        }
        /*
        if( sAspectRatioMode == kAspectRatio_Mode_1v1 ) {
            w = 1f;
            h = 1f;
        }else if( sAspectRatioMode == kAspectRatio_Mode_9v16 ){
            w = 9f;
            h = 16f;
        }else if( sAspectRatioMode == kAspectRatio_Mode_2v1 ){
            w = 2f;
            h = 1f;
        }else if( sAspectRatioMode == kAspectRatio_Mode_1v2 ){
            w = 1f;
            h = 2f;
        }
*/
        if( sScreenMode == kScreenMode_horizonDual ){
            w *= 2f;
        }

        Log.d("ScreenMode", "w="+w+", h="+h);

        return w/h;
    }

    /** 
     * This method gets the aspect ratio of the output screen as an <tt>integer</tt>.  
     *
     * <p>Example code :</p>
     *  {@code if(nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v1 )
        }
     * @return 1 if set to <tt>kAspectRatio_Mode_1v1</tt>; 2 if set to <tt>kAspectRatio_Mode_16v9</tt>. 
     * @since version 1.1.0
     * @see #setAspectMode(int)
     * @see #getAspectRatio()
     */
    public static int getAspectRatioMode(){
        return sAspectRatioMode;
    }
 
    /**
     * This method sets the mode for the output screen.  
     *
     * <p>Example code :</p>
     *  {@code nexApplicationConfig.setScreenMode(nexApplicationConfig.kScreenMode_normal);
        }
     * @param mode The screen mode as an <tt>integer</tt>. 0 for normal screen mode or 1 for dual screen mode. 
     * @see #getScreenMode()
     * @since version 1.3.52
     */
    public static void setScreenMode(int mode){
        Log.d("ScreenMode", "setScreenMode="+mode);
        sScreenMode = mode;
    }
 
    /**
     * This method gets the mode currently set to the output screen.
     *
     * <p>Example code :</p>
     *  {@code mVideoEditor.setScreenMode(nexApplicationConfig.getScreenMode());
        }
     * @return The screen mode currently set to output screen as an <tt>integer</tt>. 0 for normal screen mode or 1 for dual screen mode. 
     * @see #setScreenMode(int)
     * @since version 1.3.52
     */
    public static int getScreenMode(){
        return sScreenMode ;
    }

    /**
     * This method gets the supported API level.
     *
     * @return The supported API level.
     * @since version 1.5.19
     */
    public static final int getApiLevel(){ return EditorGlobal.ApiLevel; }

    /**
     * This enumeration defines the enumeration types of features provided by the NexEditor&tm;&nbsp;SDK.
     *  
     * These enumeration types include:
     * - <b>UnLimited</b>: All features are available to use.
     * - <b>OverlayImageLimited</b>: OverlayImage, OverlayVideo, OverlayAnimate features can not be used. 
     * - <b>OverlayVideoLimited</b>: OverlayVideo, OverlayAnimate features can not be used. 
     * - <b>OverlayAnimateLimited</b>: OverlayAnimate feature can not be used. 
     *
     * @since version 1.5.19
     */
    public enum APILevel {
        UnLimited( EditorGlobal.kUnLimited ),
        OverlayImageLimited(EditorGlobal.kOverlayImageLimited),
        OverlayVideoLimited(EditorGlobal.kOverlayVideoLimited),
        OverlayAnimateLimited(EditorGlobal.kOverlayAnimateLimited);

        private final int internalValue;
        APILevel(int val){
            internalValue = val;
        }

        int getValue(){return internalValue;}
        public static APILevel fromValue( int value ) {
            for( APILevel v : APILevel.values() ) {
                if( v.getValue()==value )
                    return v;
            }
            return null;
        }
    }

    /**
     * This method checks whether or not an API is available to use.
     *
     * <p>Example code :</p>
     *  {@code if(isSupportedApi(nexApplicationConfig.APILevel.OverlayImageLimited)) {
         // ToDo.  
        } else {}}
     * @param level API level to be compared.
     * @return <tt>TRUE</tt> if the API is available.
     * @since version 1.5.19
     * @see com.nexstreaming.nexeditorsdk.nexApplicationConfig.APILevel#fromValue(int)
     */
    public static final boolean isSupportedApi(APILevel level){
        return EditorGlobal.isSupportedApi(level.getValue());
    }

    /**
     * @brief This sets the version of the preload Assts in assets/kmassets of the application.
     * Those preload Assets are installed when the application first starts.
     * After that, the application reads just hte install DB to run the application quickly.
     * One problem is that the application does not install again even if assets/kmassets has been edited.
     * If the developer wants to manage and update assets/kmassets, use this method to replace the value with a different one.
     * Default value is the date when the SDK was built.
     * @param versionCode User custom version.
     * @since 1.7.4
     */
    public static final void setAppAssetPackageVersionCode(long versionCode) {
        AppAssetPackageVersionCode = versionCode;
    }

    /**
     * @brief If the application installs Assets on a local disk of a device, it sets the AssetStoreRootPath where the Assets-to-be-installed will be located.
     * @warning The set path does not install any Asset packages that are placed.
     * To find out how to install Asset packages on a local disk, contact Nexstreaming.
     * @param path AssetStore Root Path
     * @since 1.7.4
     * @see nexAssetPackageManager#installPackageFromStorePath(int)
     * @see nexAssetPackageManager#installPackagesAsync(nexAssetPackageManager.OnInstallPackageListener)
     * @see #setAssetInstallRootPath(String)
     */
    public static final void setAssetStoreRootPath(String path){
        AssetLocalInstallDB.setAssetStoreRootPath(path);
    }

    /**
     * This method sets the Path where Asset is installed. By default, it is App file directory.
     * This API should only be used to install Assets from other Asset Stores other than VASSET.
     * If the Path is set to a public directory, App developer should take care of the integrity of installed assets.
     * @param path Path where Asset is installed
     * @since 1.7.7
     * @see nexAssetPackageManager#installPackageFromStorePath(int)
     * @see nexAssetPackageManager#installPackagesAsync(nexAssetPackageManager.OnInstallPackageListener)
     * @see #setAssetStoreRootPath(String)
     */
    public static final void setAssetInstallRootPath(String path){
        AssetLocalInstallDB.setInstalledAssetPath(path);
    }


    /**
     * This method sets the size of a canvas. It is used to set a different Aspect instead of the default Aspect.
     * @param profile Aspect profile
     * @since 1.7.7
     * @see #getAspectProfile()
     */
    public static void setAspectProfile(nexAspectProfile profile ){
        sAspectRatioMode = profile.getAspectMode();
        aspectProfile = profile;
        NexThemeView.setAspectRatio(aspectProfile.getAspectRatio());
    }

    /**
     * This method returns the current Aspect profile.
     * @return current Aspect profile
     * @since 1.7.7
     * @see #setAspectProfile(nexAspectProfile)
     */
    public static final nexAspectProfile getAspectProfile(){
        return aspectProfile;
    }

    /**
     *
     * @param effect_id {@link #letterbox_effect_black} or {@link #letterbox_effect_blur10}
     * @since 2.0.0
     */
    public static void setDefaultLetterboxEffect(String effect_id){

        default_letterbox_effect = effect_id;
    }

    /**
     *
     * @return {@link #letterbox_effect_black} or {@link #letterbox_effect_blur10}
     * @since 2.0.0
     */
    public static String getDefaultLetterboxEffect(){

        return default_letterbox_effect;
    }

    /**
     *
     * @return
     * @since 2.0.7
     */
    public static nexExternalModuleManager getExternalModuleManager(){
        return nexExternalModuleManager.getInstance();
    }

    /**
     * @since 2.0.14
     */
    public static void setOverlayCoordinateMode(int mode){
        sOverlayCoordinateMode = mode;
    }

    /**
     * @since 2.0.14
     */
    public static int getOverlayCoordinateMode(){
        return sOverlayCoordinateMode;
    }

    /**
     * @since 2.0.14
     */
    public static void removeAllTempFiles(){
        if( KMSingle != null ){
            File tempClipDir = KMSingle.getApplicationContext().getFilesDir();
            File[] fileList = tempClipDir.listFiles();
            for(int i = 0 ; i< fileList.length ; i++ ){
                File file = fileList[i];
                if( file.isFile() ){
                    if( file.getName().startsWith(EditorGlobal.PrefixTempFile) ){
                        if( !file.delete() ){
                            Log.d(TAG,"delete fail! file ="+file.getName());
                        }
                    }
                }
            }
        }
    }
}
