/******************************************************************************
 * File Name        : nexAssetService.java
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

package com.nexstreaming.nexeditorsdk.service;


import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.os.Environment;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.annotation.Nullable;
import android.util.Base64;
import android.util.Log;

import com.google.gson_nex.Gson;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.InstallSourceType;

import com.nexstreaming.app.common.nexasset.store.AssetLocalInstallDB;

import com.nexstreaming.app.common.nexasset.store.json.AssetStoreAPIData;
import com.nexstreaming.app.common.util.CloseUtil;
import com.nexstreaming.app.common.util.Stopwatch;
import com.nexstreaming.app.common.util.StringUtil;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;


import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.StringBufferInputStream;
import java.nio.channels.FileChannel;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class nexAssetService extends Service {

    private static final String TAG = "nexAssetService";

    private static final String ACTION_BIND = "com.nexstreaming.nexeditorsdk.service.bind";

    public static final String ACTION_ASSET_INSTALL_COMPLETED = "com.nexstreaming.nexeditorsdk.asset.install.completed";
    public static final String ACTION_ASSET_UNINSTALL_COMPLETED = "com.nexstreaming.nexeditorsdk.asset.uninstall.completed";
    public static final String ACTION_ASSET_FEATUREDLIST_COMPLETED = "com.nexstreaming.nexeditorsdk.asset.feathredlist.completed";


    private static final ExecutorService sInstallThreadExcutor = Executors.newSingleThreadExecutor();

    private static final int TYPE_JSON_DATA = 0;
    private static final int TYPE_BITMAP_DATA = 1;


    private Bitmap mCurrentThumbnail;
    private AssetStoreAPIData.AssetInfo mCurrentAssetInfo;
    private FileOutputStream mFileOutputStream;
    private long mReceivedDataSize = 0;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate() called");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy() called");
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind() called with: intent = [" + intent + "]");

        if(intent != null && intent.getAction() != null && intent.getAction().equals(ACTION_BIND)){
            if(intent.getPackage() != null){
                Intent serviceIntent = new Intent();
                serviceIntent.setPackage(getPackageName());
                serviceIntent.setAction("com.nexstreaming.app.assetstore.sdk.service.bind");
                serviceIntent.setClassName(intent.getPackage(), intent.getPackage() + ".AssetStoreService");
                Log.d(TAG, "onBind: " + serviceIntent);
                startService(serviceIntent);
            }

            return nexAssetService;
        }
        return null;
    }

    @Override
    public void onTaskRemoved(Intent rootIntent) {
        super.onTaskRemoved(rootIntent);

        Log.d(TAG, "onTaskRemoved() called");
    }

    private void onConnectionInstaller(int assetIndex, String base64ThumbnailString, String json, INexAssetConnectionCallback callback){
        if(base64ThumbnailString != null && json != null && callback != null){
            byte [] encodeByte= Base64.decode(base64ThumbnailString , Base64.DEFAULT);
            mCurrentThumbnail = BitmapFactory.decodeByteArray(encodeByte, 0, encodeByte.length);
            if(LL.D) Log.d(TAG, "onConnectionInstaller: make bitmap completed " + mCurrentThumbnail);
            mCurrentAssetInfo = new Gson().fromJson(json, AssetStoreAPIData.AssetInfo.class);
            if(LL.D) Log.d(TAG, "onConnectionInstaller: convert Asset completed " + mCurrentAssetInfo);
            try {
                callback.onConnectionCompleted(assetIndex);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    private void onReceivedAssetData(int assetIndex, String byteString, int size, long max, INexAssetInstallCallback callback){

        String assetPath = getFilesDir() + File.separator + assetIndex + ".zip";
        mReceivedDataSize += size;

        byte [] encodeByte= Base64.decode(byteString , Base64.DEFAULT);
        //if(LL.D) Log.d(TAG, "onReceivedAssetData() called with: assetIndex = [" + assetIndex + "], size = [" + size + "], max = [" + max + "]");
        //if(LL.D) Log.d(TAG, "onReceivedAssetData: ReceivedData size " + mReceivedDataSize);
        if(mFileOutputStream == null){
            File file  = new File(assetPath);
            try {
                mFileOutputStream = new FileOutputStream(file);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        }
        try {
            mFileOutputStream.write(encodeByte, 0, size);
        } catch (IOException e) {
            e.printStackTrace();
        }

        if(mReceivedDataSize == max){
            if(LL.D) Log.d(TAG, "onReceivedAssetData: Received Completed size " + mReceivedDataSize);
            try {
                if(LL.D) Log.d(TAG, "onReceivedAssetData: try close stream ");
                mFileOutputStream.close();
            } catch (IOException e) {
                if(LL.D) Log.d(TAG, "onReceivedAssetData: fileOutputStream close error", e);
            }
            /**
             * install received asset
             */
            installAsset(assetPath, mCurrentThumbnail, mCurrentAssetInfo, callback);

            /**
             * init member variable
             */
            mFileOutputStream = null;
            mCurrentAssetInfo = null;
            mCurrentThumbnail = null;
            mReceivedDataSize = 0;
        }
    }

    private void onLoadInstalledAssetList(INexAssetDataCallback callback) {
        if(callback != null){
            if (LL.D) Log.d(TAG, "internalLoadInstallAssetList() called");
            List<? extends AssetInfo> list = AssetLocalInstallDB.getInstance(this).getAssetInstalledDownloadItemItems();
            String resposne = "";
            if (list != null && list.size() > 0) {
                List<NexInstalledAssetItem> transferDatas = new ArrayList<>();
                //if (LL.D) Log.d(TAG, "internalLoadInstallAssetList() called");
                //String lang = getResources().getConfiguration().locale.getLanguage();
                for (AssetInfo assetInfo : list) {
                    if( AssetPackageManager.getInstance(this).checkExpireAsset(assetInfo) ){
                        if (LL.D) Log.d(TAG, "expire Asset Idx="+assetInfo.getAssetIdx());
                        continue;
                    }

                    int index = assetInfo.getAssetIdx();
                    String id = assetInfo.getAssetId();
                    String assetName = "";
                    if (assetInfo.getAssetName() != null) {
                        assetName = StringUtil.getLocalizedString(this,assetInfo.getAssetName());
                        /*
                        assetName = assetInfo.getAssetName().get("en");
                        if (assetInfo.getAssetName().get(lang) != null) {
                            assetName = assetInfo.getAssetName().get(lang);
                        }
                        else {
                            String locale = getResources().getConfiguration().locale.getLanguage() + "-" + getResources().getConfiguration().locale.getCountry();
                            if (assetInfo.getAssetName().get(locale) != null ) {
                                if(LL.D) Log.d(TAG, "onLoadInstalledAssetList: change locale from " + lang + " to " + locale);
                                lang = locale;
                                assetName = assetInfo.getAssetName().get(lang);
                            }
                        }
                        */
                    }
                    String categoryName = "";

                    if (assetInfo.getAssetCategory() != null) {
                        categoryName = assetInfo.getAssetCategory().getCategoryAlias();
                    }

                    if (assetInfo.getAssetSubCategory() != null) {
                        categoryName = assetInfo.getAssetSubCategory().getSubCategoryAlias();
                        if (assetInfo.getAssetSubCategory().getSubCategoryName() != null) {
                            categoryName = StringUtil.getLocalizedString(this,assetInfo.getAssetSubCategory().getSubCategoryName());
                            //categoryName = assetInfo.getAssetSubCategory().getSubCategoryName().get(lang);
                        }
                    }

                    NexInstalledAssetItem transferData = new NexInstalledAssetItem(index, id, assetName, categoryName
                            , assetInfo.getThumbUrl(), assetInfo.getInstalledTime(), assetInfo.getExpireTime()
                            , assetInfo.getMinVersion(), assetInfo.getPackageVersion()
                    );
                    //if (LL.D) Log.d(TAG, "added TransferData ");
                    transferDatas.add(transferData);
                }
                //if(LL.D) Log.d(TAG, "onLoadInstalledAssetList() called with: transferDatas = [" + transferDatas + "]");
                resposne = new Gson().toJson(transferDatas);
                try {
                    callback.onLoadAssetDatas(resposne);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }else{
                try {
                    callback.onLoadAssetDatas(null);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private AsyncTask <Context,Void,Void> checkReceivedFeaturedListAsyncTask;
    private boolean isRunningAsyncTask;
    private volatile long lastReceivedFeaturedListTime;
    private volatile boolean isUpdatedFeaturedList;
    private void onReceivedAssetInfoData(int type, int index, String data){
        //if(LL.D)
        //    Log.d(TAG, "onReceivedAssetInfoData() called with: type = [" + type + "], index = [" + index + "], data = [" + data + "]");
        lastReceivedFeaturedListTime = System.currentTimeMillis();

        if(data != null){
            switch (type){
                case TYPE_JSON_DATA :{

                    if( isRunningAsyncTask == false ){
                        isRunningAsyncTask = true;
                        isUpdatedFeaturedList = false;
                        checkReceivedFeaturedListAsyncTask = new AsyncTask<Context, Void, Void>() {
                            private Context context;
                            @Override
                            protected Void doInBackground(Context... params) {
                                context = params[0];
                                while( System.currentTimeMillis() - lastReceivedFeaturedListTime < 1000 ){
                                    Log.d(TAG, "onReceivedAssetInfoData() wait="+lastReceivedFeaturedListTime);
                                    try {
                                        Thread.sleep(500);
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }
                                }
                                return null;
                            }

                            @Override
                            protected void onPreExecute() {
                                super.onPreExecute();
                                Log.d(TAG, "onReceivedAssetInfoData() Start..");
                            }

                            @Override
                            protected void onPostExecute(Void aVoid) {

                                super.onPostExecute(aVoid);

                                Intent intent = new Intent(ACTION_ASSET_FEATUREDLIST_COMPLETED);
                                intent.putExtra("update", isUpdatedFeaturedList);
                                context.sendBroadcast(intent);
                                isUpdatedFeaturedList = false;
                                isRunningAsyncTask = false;
                                Log.d(TAG, "onReceivedAssetInfoData() End..");
                            }
                        };
                        checkReceivedFeaturedListAsyncTask.executeOnExecutor(sInstallThreadExcutor, this, null, null);
                    }
                    if( !isUpdatedFeaturedList ) {
                        isUpdatedFeaturedList = AssetLocalInstallDB.isUpdatedFeaturedList(index, data);
                    }
                    AssetLocalInstallDB.saveFeaturedList(index,data );
                    //AssetStoreAPIData.GetFeaturedAssetInfoResponse featuredAsset = new Gson().fromJson(data, AssetStoreAPIData.GetFeaturedAssetInfoResponse.class);
                    if(LL.D) Log.d(TAG, "onReceivedAssetInfoData: featuredAsset =" + index);
                }
                break;

                case TYPE_BITMAP_DATA :{
                    byte[] imageData = Base64.decode(data.getBytes(), Base64.DEFAULT);
                    Bitmap assetBitmap = BitmapFactory.decodeByteArray(imageData, 0, imageData.length);
                    AssetLocalInstallDB.saveFeaturedThumbnail(index ,assetBitmap);
            //        if(LL.D) Log.d(TAG, "onReceivedAssetInfoData: bitmap height : " + assetBitmap.getHeight());
                    if(LL.D) Log.d(TAG, "onReceivedAssetInfoData: bitmap index : " + index);
                }
                    break;
            }
        }
    }

    private void onUninstallAsset(int assetIndex, INexAssetUninstallCallback callback){
        //TODO : do uninstall asset
        nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).putUninstallItem(assetIndex);

        try {
            AssetLocalInstallDB.getInstance(getApplicationContext()).uninstallPackage(assetIndex);
            callback.onUninstallCompleted(assetIndex);
        } catch (Exception e) {
            if(LL.D) Log.w(TAG, "onUninstallAsset: error ", e);
            try {
                callback.onUninstallFailed(assetIndex, e.getMessage());
            } catch (RemoteException e1) {
                if(LL.D) Log.w(TAG, "onUninstallAsset: ", e1);
            }
        }

        /**
         * Send internal broadcast for update uninstalled asset
         */
        Intent intent = new Intent(ACTION_ASSET_UNINSTALL_COMPLETED);
        intent.putExtra("index", assetIndex);
        sendBroadcast(intent);
    }


    private void installAsset(final String assetPath, final Bitmap bitmap, final AssetStoreAPIData.AssetInfo assetInfo, final INexAssetInstallCallback callback){
        if(assetPath != null && bitmap != null && !bitmap.isRecycled() && assetInfo !=null && callback != null){
            AssetInstallTask installTask = new AssetInstallTask(this, assetPath, bitmap, assetInfo, callback);
            //installTask.executeOnExecutor(sInstallThreadExcutor);
            installTask.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
        }else{
            if(LL.D) Log.d(TAG,"installAsset fail!="+assetPath);
            if( callback != null ) {
                try {
                    callback.onInstallFailed(assetInfo.idx,"component not found!");
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private INexAssetService.Stub nexAssetService = new INexAssetService.Stub() {
        @Override
        public void connectInstaller(int assetIndex, String thumbnailByteString, String json, INexAssetConnectionCallback callback) throws RemoteException {
            onConnectionInstaller(assetIndex, thumbnailByteString, json, callback);
        }

        @Override
        public void sendAssetData(int assetIndex, String byteString, int size, long max, INexAssetInstallCallback callback) throws RemoteException {
            onReceivedAssetData(assetIndex, byteString, size, max, callback);
        }

        @Override
        public void uninstallAsset(int assetIndex, INexAssetUninstallCallback callback) throws RemoteException {
            onUninstallAsset(assetIndex, callback);
        }

        @Override
        public void loadInstalledAssetList(INexAssetDataCallback callback) throws RemoteException {
            onLoadInstalledAssetList(callback);
        }

        /**
         *
         * @param type 0 = JsonData, 1 = bitmapData
         * @param index
         * @param data
         * @throws RemoteException
         */
        @Override
        public void saveAssetInfoData(int type, int index, String data) throws RemoteException {
            onReceivedAssetInfoData(type, index, data);
        }
    };

    private static final class AssetInstallTask extends AsyncTask<Void, Void, Boolean>{

        private Context context;
        private String assetPath;
        private Bitmap bitmap;
        private AssetStoreAPIData.AssetInfo assetInfo;
        private INexAssetInstallCallback callback;
        private long updateTime = 0;
        private Stopwatch watch = new Stopwatch();

        public AssetInstallTask(Context context, String assetPath, Bitmap bitmap, AssetStoreAPIData.AssetInfo assetInfo, INexAssetInstallCallback callback) {
            watch.reset();
            watch.start();
            if(LL.D)
                Log.d(TAG, "InstallTask() called with: context = [" + context + "], assetPath = [" + assetPath + "], bitmap = [" + bitmap + "], assetInfo = [" + assetInfo + "], callback = [" + callback + "]");
            this.context = context;
            this.assetPath = assetPath;
            this.bitmap = bitmap;
            this.assetInfo = assetInfo;
            this.callback = callback;
        }

        private void sendProgress(boolean force, int progress){
            boolean send = force;

            if( !send && System.currentTimeMillis() - updateTime > 1000 ){
                send = true;
            }

            if( send ) {
                updateTime = System.currentTimeMillis();
                if(LL.D) Log.d(TAG,"sendProgress() asset("+assetInfo.idx+") progress="+progress);
                try {
                    callback.onProgressInstall(assetInfo.idx, progress, 100);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        private void sendCompleted(String errorMsg ){
            try {
                if( errorMsg != null ){

                    callback.onInstallFailed(assetInfo.idx,errorMsg);
                    Log.d(TAG, "sendCompleted install asset(" + assetInfo.idx + ") error=" + errorMsg);
                }else {
                    callback.onInstallCompleted(assetInfo.idx);
                    Log.d(TAG,"sendCompleted install asset("+assetInfo.idx +") installed");
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        protected Boolean doInBackground(Void... voids) {
            if(LL.D) Log.d(TAG, "installAsset() assetPath = [" + assetPath + "]");
            File thumbnailPath = null;
            String errorMsg = null;
            //TODO : install Asset

            sendProgress(true, 0);
            AssetInfo ai = AssetPackageManager.getInstance(context).getInstalledPackageInfoByAssetIdx(assetInfo.idx);

            if( ai != null ) {
                if(LL.D) Log.d(TAG,"already installed Asset="+assetInfo.idx);

                if( ai.getInstallSourceType() == InstallSourceType.STORE ) {
                    try {
                        AssetLocalInstallDB.getInstance(context).uninstallPackage(assetInfo.idx);
                    } catch (Exception e) {
                        //e.printStackTrace();
                        errorMsg = e.getMessage();
                        if( errorMsg == null ){
                            errorMsg = "uninstall fail asset="+assetInfo.idx;
                        }
                        File assetFile = new File(assetPath);
                        assetFile.delete();
                        sendCompleted(errorMsg);
                        return false;
                    }
                }else{
                    if(LL.D) Log.d(TAG,"installed Asset is not store type. idx="+assetInfo.idx);
                    File assetFile = new File(assetPath);
                    assetFile.delete();
                    errorMsg = "installed Asset is not store type. idx="+assetInfo.idx;
                    sendCompleted(errorMsg);
                    return false;
                }
            }

            try {
                File assetFile = new File(assetPath);
                File unzipFolder = AssetLocalInstallDB.getInstance(context).getUnzipFolder(assetInfo.idx);
                unzip(assetFile, unzipFolder,assetInfo.idx);
                assetFile.delete();
                if( bitmap != null ){
                    thumbnailPath = AssetLocalInstallDB.getInstance(context).makeThumbnail(bitmap, assetInfo.idx);
                }
                Log.d(TAG,"installed Asset idx="+assetInfo.idx+", sdklevel="+assetInfo.asset_sversion+", version="+assetInfo.asset_version);
                AssetPackageManager.getInstance(context).installStorePackage(unzipFolder, thumbnailPath, new AssetLocalInstallDB.internalStoreAssetInfo(assetInfo) );
            } catch (IOException e) {
                e.printStackTrace();
                errorMsg = e.getMessage();
                if( errorMsg == null ){
                    errorMsg = "unzip or db update fail! asset="+assetInfo.idx;
                }
                sendCompleted(errorMsg);
                return false;
            }
            //TODO : send installing progress
            sendCompleted(null);
            return true;
        }

        private void unzip(File zipPath, File outputPath,int idx ) throws IOException{

            if( LL.D ) Log.d(TAG,"Unzipping '" + zipPath + "' to '" + outputPath + "'");
            if( !outputPath.mkdirs() && !outputPath.exists() ) {
                throw new IOException("Failed to create directory: " + outputPath);
            }

            ZipInputStream in = null;
            long totalSize = zipPath.length();
            long progressSize = 0;

            in = new ZipInputStream(new FileInputStream(zipPath));

            try {
                ZipEntry zipEntry;
                while ((zipEntry = in.getNextEntry()) != null) {
                    String name = zipEntry.getName();

                    if (name.contains("..")) {
                        throw new IOException("Relative paths not allowed: "+name); // For security purposes
                    }
                    File outputFile = new File(outputPath, name);
                    if (zipEntry.isDirectory()) {
                        if (!outputFile.mkdirs() && !outputFile.exists()) {
                            throw new IOException("Failed to create directory: " + outputFile);
                        }
                        //if( LL.D ) Log.d(TAG,"  - unzip: made folder '" + name + "'");
                    } else {
                        //if( LL.D ) Log.d(TAG,"  - unzip: unzipping file '" + name + "' " + zipEntry.getCompressedSize() + "->" + zipEntry.getSize() + " (" + zipEntry.getMethod() + ")");
                        FileOutputStream out = new FileOutputStream(outputFile);
                        try {
                            AssetLocalInstallDB.copy(in, out);
                        } finally {
                            CloseUtil.closeSilently(out);
                        }
                        progressSize +=  zipEntry.getCompressedSize();
                        int progress = (int)((double) progressSize * 100 / (double)totalSize);
                        if(progress >= 100 ){
                            progress = 99;
                        }

                        sendProgress(false, progress);
                        //if(LL.D) Log.d(TAG, "unzip() progressSize [" + progressSize + "], " +  "progress [" + progress + "]," + "totalSize [" + totalSize + "]") ;
                    }
                }
            } finally {
                CloseUtil.closeSilently(in);
            }
            if( LL.D ) Log.d(TAG,"Unzipping DONE for: '" + zipPath + "' to '" + outputPath + "'");
        }

        @Override
        protected void onPostExecute(Boolean ok) {
            super.onPostExecute(ok);
            if( ok ){
                /**
                 * Send internal broadcast for update installed asset
                 */
                Intent intent = new Intent(ACTION_ASSET_INSTALL_COMPLETED);
                intent.putExtra("index", assetInfo.idx);
                intent.putExtra("category.alias", assetInfo.category_aliasName);
                context.sendBroadcast(intent);

                watch.stop();
                Log.d(TAG,"install asset("+assetInfo.idx +") time elapsed = "+watch.toString());
            }
        }

        private void saveDebugFile(File thumbnailPath, File assetFile){
            File debugFilePath = new File(Environment.getExternalStorageDirectory()+"/nex/");
            debugFilePath.mkdirs();

            try {
                //copy thumbnail
                File debugThumbFile = new File(debugFilePath, assetInfo.idx+".PNG");
                copy(thumbnailPath, debugThumbFile);

                //copy assetFile
                File debugAssetFile = new File(debugFilePath, assetInfo.idx+".zip");
                copy(assetFile, debugAssetFile);

                //copy Json
                Gson gson = new Gson();
                String json = gson.toJson(assetInfo);
                File debugJsonFile = new File(debugFilePath, assetInfo.idx+".json");
                {
                    InputStream in = new StringBufferInputStream(json);
                    OutputStream out = new FileOutputStream(debugJsonFile);

                    // Transfer bytes from in to out
                    byte[] buf = new byte[1024];
                    int len;
                    while ((len = in.read(buf)) > 0) {
                        out.write(buf, 0, len);
                    }
                    in.close();
                    out.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        private void copy(File src, File dst) throws IOException {
            FileInputStream inStream = new FileInputStream(src);
            FileOutputStream outStream = new FileOutputStream(dst);
            FileChannel inChannel = inStream.getChannel();
            FileChannel outChannel = outStream.getChannel();
            inChannel.transferTo(0, inChannel.size(), outChannel);
            inStream.close();
            outStream.close();
        }
    }
}
