/******************************************************************************
 * File Name        : nexAssetStoreAppUtils.java
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

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.store.AssetLocalInstallDB;
import com.nexstreaming.app.common.nexasset.store.AssetStoreClient;
import com.nexstreaming.app.common.nexasset.store.VendorList;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.nexeditorsdk.exception.nexSDKException;
import com.nexstreaming.nexeditorsdk.service.nexAssetStoreClient;

import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

/**
 * In nexEditorSDK, new Asset such as Template, Effect , Sticker , Font , Audio , Filter can only be downloaded and installed from the Asset Store App. 
 *
 * nexAssetStoreAppUtils is composed of APIs that help calling the Asset Store App from NexStreaming while developing an app that uses nexEditorSDK. 
 *
 * Example code : 1 uses nexAssetStoreAppUtils and checks whether or not the Asset Store App is installed and currently running. 
 * <p>Example code : 1</p>
 *     {@code  if( nexAssetStoreAppUtils.isInstalledAssetStoreApp(getApplicationContext()) ){
 *            Log.d(TAG, "Asset store installed");
 *            if( !nexAssetStoreAppUtils.isServiceRunningCheck(getApplicationContext())) {
 *                Log.d(TAG, "Asset store service was not working");
 *                nexAssetStoreAppUtils.sendAssetStoreAppServiceIntent(getApplicationContext());
 *            }else{
 *                Log.d(TAG, "Asset store service was working");
 *            }
 *        }else{
 *            Log.d(TAG, "Asset store was not installed");
 *        }
 *    }
 *
 *
 *  Example code : 2 sends an Intent to the Asset Store App and runs it; this is the receiving part when the App is stopped. 
 *  After getting a response, it uses {@link nexAssetPackageManager} to install the downloaded Assets from the Asset Store App.
 *
 * <p>Example code : 2</p>
 *     {@code
 *     private int AssetStoreRequestCode = 0;
 *     ......
 *        Button bt_install = (Button) findViewById(R.id.button_template_man_install);
 *            bt_install.setOnClickListener(new View.OnClickListener() {
 *                public void onClick(View v) {
 *                    AssetStoreRequestCode = nexAssetStoreAppUtils.runAssetStoreApp(TemplateManagerActivity.this ,null);
 *                }
 *            });
 *        .......
 *
 *        protected void onActivityResult(int requestCode, int resultCode, Intent data) {
 *            super.onActivityResult(requestCode, resultCode, data);
 *            if(requestCode == AssetStoreRequestCode && resultCode == Activity.RESULT_OK) {
 *                Log.d(TAG,"onActivityResult from Asset Store");
 *                //use nexAssetPackageManager
 *            }
 *        }
 *       }
 *
 */
public class nexAssetStoreAppUtils {
    private static final Executor sInstallThreadPool = Executors.newSingleThreadExecutor();

    private static final int assetStoreAppResultCode = 0x5689;

    private static final int getAssetStoreSDKLevel = 7;
    private static final int  AssetStoreProtocolVersion = 1;

    private static final String TAG="nexAssetStoreAppUtils";

    private static final String kAssetStoreMimeType = "AssetStore.intent.extra.MimeType"; //int
    private static final String kAssetStoreMimeTypeExtra = "AssetStore.intent.extra.MimeTypeExtra"; //String
    private static final String kAssetStoreAssetID = "AssetStore.intent.extra.AssetID"; //String
//    private static final String kAssetStoreMultiSelect = "AssetStore.intent.extra.MultiSelect"; //boolean
    private static final String kAssetStoreVendor = "AssetStore.intent.extra.Vendor"; //String
    private static final String kAssetStoreAssetImageUrl = "AssetStore.intent.extra.AssetImageUrl"; //String
    private static final String kAssetStoreAssetSDKLevel = "AssetStore.intent.extra.SDKLevel"; //int
    private static final String kAssetStoreAssetConnectServer = "AssetStore.intent.extra.ConnectServer"; //int
    private static final String kAssetStoreAssetProtocolVersion = "AssetStore.intent.extra.ProtocolVersion"; //String
    private static final String kAssetStoreAvailableCategorys = "AssetStore.intent.extra.UiSet.Categorys"; //int
    private static final String kAssetStoreMarketId = "AssetStore.intent.extra.MarketId"; //String
    private static final String kAssetStoreLaunchMode = "AssetStore.intent.extra.Launch.Mode";// int
    private static final String kAssetStoreDenyFeaturedList = "AssetStore.intent.extra.DenyFeaturedList"; //boolean
    /**
     * The developer can choose a Template when running the Asset Store App.
     * @see #setMimeType(int)
     * @since 1.7.0
     */
    public static final int AssetStoreMimeType_Template = 0x1;

    /**
     * The developer can choose a Clip Effect when running the Asset Store App. 
     * @see #setMimeType(int)
     * @since 1.7.0
     */
    public static final int AssetStoreMimeType_Effect = 0x2;

    /**
     * The developer can choose a Transition Effect when running the Asset Store App.
     * @see #setMimeType(int)
     * @since 1.7.0
     */
    public static final int AssetStoreMimeType_Transition = 0x4;

    /**
     * The developer can choose Audio when running the Asset Store App.
     * @see #setMimeType(int)
     * @since 1.7.0
     */
    public static final int AssetStoreMimeType_Audio = 0x8;

    /**
     * The developer can choose a Filter when running the Asset Store App. 
     * @see #setMimeType(int)
     * @since 1.7.0
     */
    public static final int AssetStoreMimeType_Filter = 0x10;

    /**
     * The developer can choose a Background when running the Asset Store App. 
     * @see #setMimeType(int)
     * @since 1.7.0
     */
    public static final int AssetStoreMimeType_Background = 0x20;

    /**
     * The developer can choose a Sticker when running the Asset Store App. 
     * @see #setMimeType(int)
     * @since 1.7.0
     */
    public static final int AssetStoreMimeType_Overlay = 0x40;

    /**
     * The developer can choose an enhanced effect when running the Asset Store App. 
     * @see #setMimeType(int)
     * @since 1.7.0
     */
    public static final int AssetStoreMimeType_RenderItem = 0x80;

    /**
     * The developer can choose a Font when running the Asset Store App.
     * @see #setMimeType(int)
     * @since 1.7.1
     */
    public static final int AssetStoreMimeType_Font = 0x100;

    /**
     * The developer can choose a Title Effect when running the Asset Store App.
     * @see #setMimeType(int)
     * @since 1.7.1
     */
    public static final int AssetStoreMimeType_TitleTemplate = 0x200;


    public static final int AssetStoreMimeType_BeatTemplate = 0x400;


    /**
     * The developer can choose a All Collage when running the Asset Store App.
     * @see #setMimeType(int)
     * @since 2.0.0
     */
    public static final int AssetStoreMimeType_IntegratedCollage = 0x1000;

    /**
     * The developer can choose a Static Collage when running the Asset Store App.
     * @see #setMimeType(int)
     * @since 2.0.0
     */
    public static final int AssetStoreMimeType_StaticCollage = 0x2000;

    /**
     * The developer can choose a Dynamic Collage when running the Asset Store App.
     * @see #setMimeType(int)
     * @since 2.0.0
     */
    public static final int AssetStoreMimeType_DynamicCollage = 0x4000;


    /**
     * The developer can choose a separately categorized Asset when running the Asset Store App. 
     *
     * To do this, setMimeType(AssetStoreMimeType_Extra) must be inserted; MimeType agreed with NexStreaming should be entered in the form of \c String using setMimeTypeExtra(String).
     * @see #setMimeType(int)
     * @see #setMimeTypeExtra(String)
     * @see #runAssetStoreApp(Activity, String)
     * @since 1.7.0
     */
    public static final int AssetStoreMimeType_Extra = 0x80000000;

    /**
     * @deprecated
     */
    @Deprecated
    public static final int AssetStoreServerDRAFT = 1;

    /**
     * @deprecated
     */
    @Deprecated
    public static final int AssetStoreServerSTAGING = 2;

    /**
     * @deprecated
     */
    @Deprecated
    public static final int AssetStoreServerPRODUCTION = 3;

    private static String vendor = "NexStreaming";
    private static int moveCategory = AssetStoreMimeType_Template;
    private static int availableCategory = 0;
    private static String  mimeTypeExtra ;
    private static String  marketId = "default2";
    private static int  serverType ;
    private static boolean denyFeaturedList = false;

    /**
     * This sets the name of the vendor that uses nexEditorSDK. This value should be matched with Nexstreaming. 
     *
     * The Asset Store App screen may be different depending on this value.
     * @param vendor The name of the vendor that uses nexEditorSDK.
     * @since 1.7.0
     */
    public static void setVendor(String vendor){
        nexAssetStoreAppUtils.vendor = vendor;
    }

    /**
     * This sets the mimeType which can be selected in the Asset Store App.
     * @param mimeType
     * @since 1.7.0
     */
    public static void setMimeType(int mimeType){
        nexAssetStoreAppUtils.moveCategory = mimeType;
    }

    /**
     * The developer can choose a separately categorized Asset when running the Asset Store App. 
     *
     * To do this, setMimeType(AssetStoreMimeType_Extra) must be inserted; MimeType agreed with NexStreaming should be entered in the form of \c String using setMimeTypeExtra(String).
     * @param mimeType
     * @see #setMimeType(int)
     * @see nexAssetStoreAppUtils#AssetStoreMimeType_Extra
     * @since 1.7.0
     */
    public static void setMimeTypeExtra(String mimeType){
        nexAssetStoreAppUtils.mimeTypeExtra = mimeType;
    }

    /**
     * set Market id. default value is "default2"
     * @param id Market id.
     * @since 2.0.0
     */
    public static void setMarketId(String id){
        marketId = id;
    }

    /**
     * Must set availableCategorys
     * @param mimeTypes
     * @since 2.0.0
     */
    public static void setAvailableCategorys(int mimeTypes ){
        availableCategory = mimeTypes;
    }

    public static void setDenyFeaturedList(boolean deny){
        denyFeaturedList = deny;
    }

    /**
     * @deprecated
     * @param serverType
     */
    @Deprecated
    public static void setServer(int serverType){
        nexAssetStoreAppUtils.serverType = serverType;
    }

    static void makeConfigAsync(){
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                AssetStoreClient.makeConfig();
                return null;
            }
        }.executeOnExecutor(sInstallThreadPool);
    }

    static nexAssetStoreClient assetStoreClient;
    static boolean isConnected = false;
    static nexAssetStoreClient.OnConnectionListener assetStoreConnectionListener = new nexAssetStoreClient.OnConnectionListener() {
        @Override
        public void onConnected() {
            assetStoreClient.getFeaturedList(nexAssetPackageManager.Mode_Hot);
            assetStoreClient.getFeaturedList(nexAssetPackageManager.Mode_New);
            Log.d(TAG,"sendAssetStoreAppServiceIntent Connected.");
        }

        @Override
        public void onDisconnected() {
            Log.d(TAG,"sendAssetStoreAppServiceIntent Disconnected.");
        }
    };
    /**
     * This runs the Asset Store App service. 
     * @param appContext The value of getApplicationContext().
     * @since 1.7.0
     */
    public static void sendAssetStoreAppServiceIntent(Context appContext){
        if(EditorGlobal.USE_VASSET ) {
            Intent intent = new Intent();

            intent.setClassName(VendorList.getInstance().getAssetStoreAppPackageName(vendor), VendorList.getInstance().getAssetStoreAppServiceName(vendor));
            intent.putExtra(kAssetStoreVendor, vendor);
            intent.putExtra(kAssetStoreAssetSDKLevel, getAssetStoreSDKLevel);
            intent.putExtra(kAssetStoreAssetProtocolVersion, AssetStoreProtocolVersion);
            intent.putExtra(kAssetStoreDenyFeaturedList,denyFeaturedList);
            intent.putExtra(kAssetStoreMarketId,marketId);
            intent.setPackage(appContext.getPackageName());
            if( Build.VERSION.SDK_INT < 26 ) {
                appContext.startService(intent);
            }else{

                if(  assetStoreClient == null ) {
                    assetStoreClient = nexAssetStoreClient.create(appContext,
                            VendorList.getInstance().getAssetStoreAppPackageName(nexAssetStoreAppUtils.vendorName()),
                            marketId,
                            getAssetStoreSDKLevel,
                            3
                    );
                }

                if( !assetStoreClient.isConnected() ) {

                    boolean isSuccess = assetStoreClient.connect(assetStoreConnectionListener);

                    if (!isSuccess) {
                        Log.d(TAG, "sendAssetStoreAppServiceIntent bind fail!. call startService.");
                        assetStoreClient = null;
                        appContext.startService(intent);
                    } else {
                        Log.d(TAG, "sendAssetStoreAppServiceIntent new bindService");
                    }
                }else{
                    Log.d(TAG, "sendAssetStoreAppServiceIntent already connected");
                    if( assetStoreClient != null ) {
                        try {
                            assetStoreClient.getFeaturedList(nexAssetPackageManager.Mode_Hot);
                            assetStoreClient.getFeaturedList(nexAssetPackageManager.Mode_New);
                        }catch (IllegalStateException e){

                        }
                    }
                }
            }
        }
    }

    /**
     * @since 2.0.14
     */
    public static void disconnectAssetStoreAppService(){
        if( assetStoreClient != null) {
            assetStoreClient.disconnect();
            assetStoreClient = null;
        }
    }


    /**
     * This checks whether or not the Asset Store App is currently running.
     * @param appContext The value of getApplicationContext()
     * @return \c TRUE - Service running, \c FALSE - Service stopped.
     * @since 1.7.0
     */
    public static boolean isServiceRunningCheck(Context appContext){
        if(EditorGlobal.USE_VASSET ) {
            Context context = appContext;
            boolean ret = false;
            if (context == null) {
                context = KineMasterSingleTon.getApplicationInstance().getApplicationContext();
            }

            ActivityManager manager = (ActivityManager) context.getSystemService(Activity.ACTIVITY_SERVICE);

            for (ActivityManager.RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
                if (VendorList.getInstance().getAssetStoreAppServiceName(vendor).equals(service.service.getClassName())) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * This checks whether or not the Asset Store App is installed on the device.
     * @param appContext The value of getApplicationContext()
     * @return true \c TRUE - App exists, \c FALSE - App does not exist.
     * @since 1.7.0
     */
    public static boolean isInstalledAssetStoreApp(Context appContext){
        if(EditorGlobal.USE_VASSET ) {
            Context context = appContext;
            boolean ret = false;
            if (context == null) {
                context = KineMasterSingleTon.getApplicationInstance().getApplicationContext();
            }
            try {
                PackageInfo pi = context.getPackageManager().getPackageInfo(VendorList.getInstance().getAssetStoreAppPackageName(vendor), PackageManager.GET_ACTIVITIES);
                if (pi.applicationInfo.enabled) {
                    ret = true;
                }
            } catch (PackageManager.NameNotFoundException e) {

            }
            return ret;
        }
        return false;
    }

    /**
     * This checks whether or not KineMaster is installed on the device.
     * @param appContext The value of getApplicationContext()
     * @return \c TRUE - App exists, \c FALSE - App does not exist.
     * @since 1.7.0
     */
    public static boolean isInstalledKineMaster(Context appContext){
        Context context = appContext;
        boolean ret = false;
        if( context == null ){
            context = KineMasterSingleTon.getApplicationInstance().getApplicationContext();
        }
        try {
            PackageInfo pi = context.getPackageManager().getPackageInfo(VendorList.getInstance().getKineMasterPackageName(vendor), PackageManager.GET_ACTIVITIES);
            if(pi.applicationInfo.enabled) {
                ret = true;
            }
        } catch (PackageManager.NameNotFoundException e) {

        }
        return ret;
    }

    /**
     * This runs AssetStoreApp.
     * @param activity The activity which will call this API.
     * @param assetIdx If assetID does exists, this redirects to the Asset screen. 
     *                If null is input, it redirects to a certain screen depending  on the set values of setMimeType() and setVendor().
     * @return The requestCode value in response of void onActivityResult(int requestCode, int resultCode, Intent data) of the Activity that called this API after AssetStoreApp is stopped.
     * @since 1.7.0
     */
    public static int runAssetStoreApp(Activity activity, String assetIdx){
        return runAssetStoreApp(activity, assetIdx, -1);
    }

    /**
     * This runs AssetStoreApp.
     * @param activity The activity which will call this API.
     * @param assetIdx If assetID does exists, this redirects to the Asset screen.
     *                If null is input, it redirects to a certain screen depending  on the set values of setMimeType() and setVendor().
     * @param intentFlag If ui want to set intent flag, pass it.
     *                   When you launch it in same task, please set Intent.FLAG_ACTIVITY_SINGLE_TOP. Or when you launch it in separate task, playse set  Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK.
     * @return The requestCode value in response of void onActivityResult(int requestCode, int resultCode, Intent data) of the Activity that called this API after AssetStoreApp is stopped.
     * @since 1.7.0
     */
    public static int runAssetStoreApp(Activity activity, String assetIdx, int intentFlag){
        if(EditorGlobal.USE_VASSET ) {
            if( availableCategory == 0 ){
                throw new nexSDKException("must called 'setAvailableCategorys(int mimeTypes)'");
            }


            Intent intent = new Intent();
            intent.setPackage(activity.getPackageName());
            intent.setAction("com.nexstreaming.app.assetstore.start.application");
           
            if (assetIdx != null) {
                if (!assetIdx.startsWith("FL")) {
                    int idx = Integer.parseInt(assetIdx);
                    String url = AssetLocalInstallDB.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getThumbnailUrl(idx);
                    intent.putExtra(kAssetStoreAssetImageUrl, url);
                }

                intent.putExtra(kAssetStoreAssetID, assetIdx);
            } else {
                intent.putExtra(kAssetStoreMimeType, moveCategory);
            }

            if (serverType > 0) {
                intent.putExtra(kAssetStoreAssetConnectServer, serverType);
            }

            if( marketId != null ) {
                intent.putExtra(kAssetStoreMarketId, marketId);
            }
            intent.putExtra(kAssetStoreVendor, vendor);
            intent.putExtra(kAssetStoreAssetSDKLevel, getAssetStoreSDKLevel);
            intent.putExtra(kAssetStoreAssetProtocolVersion, AssetStoreProtocolVersion);
            intent.putExtra(kAssetStoreAvailableCategorys, availableCategory);
            intent.putExtra(kAssetStoreDenyFeaturedList,denyFeaturedList);
            if( mimeTypeExtra != null) {
                intent.putExtra(kAssetStoreMimeTypeExtra, mimeTypeExtra);
            }

            if( Build.VERSION.SDK_INT < 26 ) {
                if (vendor.startsWith("TINNO") || intentFlag > 0) {
                    if (intentFlag > 0) {
                        intent.putExtra(kAssetStoreLaunchMode, intentFlag);
                        Log.d(TAG, "Set intent flag value=" + intentFlag);
                    }
                    intent.setClassName(VendorList.getInstance().getAssetStoreAppPackageName(vendor), VendorList.getInstance().getAssetStoreAppPackageName(vendor) + ".EnterActivity");
                    activity.startActivity(intent);
                } else {
                    intent.setClassName(VendorList.getInstance().getAssetStoreAppPackageName(vendor), VendorList.getInstance().getAssetStoreAppPackageName(vendor) + ".AssetStoreService");
                    activity.startService(intent);
                }
            }else{
                if( assetStoreClient != null ) {
                    if (intentFlag > 0) {
                        intent.putExtra(kAssetStoreLaunchMode, intentFlag);
                        Log.d(TAG, "Set intent flag value=" + intentFlag);
                    }
                    intent.setClassName(VendorList.getInstance().getAssetStoreAppPackageName(vendor), VendorList.getInstance().getAssetStoreAppPackageName(vendor) + ".EnterActivity");
                    activity.startActivity(intent);
                }else{
                    intent.setClassName(VendorList.getInstance().getAssetStoreAppPackageName(vendor), VendorList.getInstance().getAssetStoreAppPackageName(vendor) + ".AssetStoreService");
                    activity.startService(intent);
                }
            }

            return assetStoreAppResultCode;
        }
        return 0;
    }

    /**
     * This redirects to the VAssetStore link in GooglePlay.
     * @param appContext The value of getApplicationContext()
     * @since 1.7.0
     */
    public static void moveGooglePlayAssetStoreLink(Context appContext){
        if(EditorGlobal.USE_VASSET ) {
            Context context = appContext;
            boolean ret = false;
            if (context == null) {
                context = KineMasterSingleTon.getApplicationInstance().getApplicationContext();
            }

            Intent marketLaunch = new Intent(Intent.ACTION_VIEW);
            marketLaunch.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
            marketLaunch.setData(Uri.parse("market://details?id=" + VendorList.getInstance().getAssetStoreAppPackageName(vendor)));
            context.startActivity(marketLaunch);
        }
    }

    /**
     * This redirects to the KinemMaster link in GooglePlay.
     * @param appContext The value of getApplicationContext()
     * @since 1.7.0
     */
    public static void moveGooglePlayKineMaster(Context appContext){
        Context context = appContext;
        boolean ret = false;
        if( context == null ){
            context = KineMasterSingleTon.getApplicationInstance().getApplicationContext();
        }

        Intent marketLaunch = new Intent(Intent.ACTION_VIEW);
        marketLaunch.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
        marketLaunch.setData(Uri.parse("market://details?id=" + VendorList.getInstance().getKineMasterPackageName(vendor)));
        context.startActivity(marketLaunch);
    }

    /**
     * @hide
     *
     */
    public static String vendorName(){
        return vendor;
    }

    /**
     * @hide
     */
    public static int getSDKLevel(){ return getAssetStoreSDKLevel;}

    /**
     * @hide
     */
    public static void saveFeaturedList(int index, String data){
        AssetLocalInstallDB.saveFeaturedList(index,data );
    }

    /**
     * @hide
     */
    public static boolean isUpdatedFeaturedList(int index, String data){
        return AssetLocalInstallDB.isUpdatedFeaturedList(index,data );
    }

    /**
     * @hide
     */
    public static void saveFeaturedThumbnail(int index, Bitmap assetThumbBitmap){
        AssetLocalInstallDB.saveFeaturedThumbnail(index,assetThumbBitmap );
    }

}
