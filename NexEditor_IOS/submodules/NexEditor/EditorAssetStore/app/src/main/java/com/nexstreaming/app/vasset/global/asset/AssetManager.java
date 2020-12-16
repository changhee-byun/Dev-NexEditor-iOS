package com.nexstreaming.app.vasset.global.asset;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.v4.app.NotificationCompat;
import android.util.Base64;
import android.util.Log;

import com.android.volley.VolleyError;
import com.android.volley.toolbox.ImageLoader;
import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import com.nexstreaming.app.vasset.global.download.DownloadInfo;
import com.nexstreaming.app.assetlibrary.model.NexInstalledAssetItem;
import com.nexstreaming.app.assetlibrary.utils.CommonUtils;
import com.nexstreaming.app.general.task.ResultTask;
import com.nexstreaming.app.general.task.Task;
import com.nexstreaming.app.assetlibrary.network.KMVolley;
import com.nexstreaming.app.assetlibrary.network.assetstore.StoreAssetInfo;
import com.nexstreaming.app.vasset.global.ConfigGlobal;
import com.nexstreaming.app.vasset.global.EnterActivity;
import com.nexstreaming.app.vasset.global.LL;
import com.nexstreaming.app.vasset.global.R;
import com.nexstreaming.nexeditorsdk.service.INexAssetConnectionCallback;
import com.nexstreaming.nexeditorsdk.service.INexAssetDataCallback;
import com.nexstreaming.nexeditorsdk.service.INexAssetInstallCallback;
import com.nexstreaming.nexeditorsdk.service.INexAssetService;
import com.nexstreaming.nexeditorsdk.service.INexAssetUninstallCallback;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Created by ojin.kwon on 2016-11-18.
 */

public class AssetManager {

    private static final String TAG = "AssetManager";

    private static final String ROOT_THUMBNAIL = "thumbnail";
    private static final String ROOT_ASSET = "asset";

    private static final ExecutorService sInstallThreadExcutor = Executors.newSingleThreadExecutor();

    private static final Map<String, Task> sInstallTaskMap = new HashMap<>();
    private static final Map<String, NotificationCompat.Builder> sNotificationMap = new HashMap<>();

    private ResultTask<List<NexInstalledAssetItem>> mLoadingInstalledAssetTask;

    private Context mContext;

    private String mCallingPackage;

    private INexAssetService mNexAssetService;

    private NotificationManager mNotificationManager;


    private static AssetManager sInstance;

    public static AssetManager getInstance(){
        if(sInstance == null){
            throw new IllegalStateException("AssetManager must be init");
        }
        return sInstance;
    }

    public static void init(Context context, String callingPackage)  throws IllegalStateException{
        if(LL.D) Log.d(TAG, "init() called ");
        sInstance = new AssetManager(context, callingPackage);
    }

    private AssetManager(Context context, String callingPackage) throws IllegalStateException{
        mContext = context.getApplicationContext();
        mCallingPackage = callingPackage;
        mNotificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        if(mCallingPackage == null){
            if(LL.D){
                mCallingPackage = "com.nexstreaming.nexeditorsdkapis";
            }else{
                if(LL.E) Log.e(TAG, "AssetManager: calling package is null");
                throw  new IllegalArgumentException("Calling package must be not null");
            }
        }
        if(LL.D) Log.d(TAG, "create instance : callingPackage = " + callingPackage);
        startUp();
    }

    public void startUp(){
        Intent serviceIntent = new Intent("com.nexstreaming.nexeditorsdk.service.bind");
        serviceIntent.setClassName(mCallingPackage, "com.nexstreaming.nexeditorsdk.service.nexAssetService");
        mContext.bindService(serviceIntent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    public void dispose(){
        mContext.unbindService(serviceConnection);
    }

    public boolean isInstalledAsset(StoreAssetInfo assetInfo){
        boolean b = false;
        if(assetInfo != null && mLoadingInstalledAssetTask != null && mLoadingInstalledAssetTask.didSignalEvent(Task.Event.COMPLETE)){
            List<NexInstalledAssetItem> list = mLoadingInstalledAssetTask.getResult();
            for(NexInstalledAssetItem asset : list){
                if(asset.index == assetInfo.getAssetIndex()){
                    return true;
                }
            }
        }
        return b;
    }


    public ResultTask<List<NexInstalledAssetItem>> loadInstalledAsset(){
        if(LL.D) Log.d(TAG, "loadInstalledAsset() called");
        if(mLoadingInstalledAssetTask == null){
            mLoadingInstalledAssetTask = new ResultTask<>();
            if(mNexAssetService != null){

                try {
                    mNexAssetService.loadInstalledAssetList(new INexAssetDataCallback.Stub() {
                        @Override
                        public void onLoadAssetDatas(String json) throws RemoteException {
                            if(LL.D)
                                Log.d(TAG, "onLoadAssetDatas() called with: json = [" + json + "]");
                            if(json != null){
                                try{
                                    List<NexInstalledAssetItem> assetItems = new Gson().fromJson(json, new TypeToken<List<NexInstalledAssetItem>>(){}.getType());
                                    if(LL.D) Log.d(TAG, "onLoadAssetDatas: load Completed asset count : " + assetItems.size());
                                    mLoadingInstalledAssetTask.sendResult(assetItems);
                                }catch (Exception e){
                                    mLoadingInstalledAssetTask.sendFailure(new Task.SimpleTaskError(e, null));
                                }
                            }
                        }
                    });
                } catch (RemoteException e) {
                    Log.e(TAG, "loadInstalledAsset: Service connection Error", e);
                }
            }
        }
        return mLoadingInstalledAssetTask;
    }

    public Task getInstallAssetTask(StoreAssetInfo assetInfo){
        Task task = null;
        if(assetInfo != null){
            task = sInstallTaskMap.get(String.valueOf(assetInfo.getAssetIndex()));
        }
        return task;
    }

    public Task installAsset(final StoreAssetInfo assetInfo, final DownloadInfo downloadInfo){
        String key = String.valueOf(assetInfo.getAssetIndex());
        Task task = sInstallTaskMap.get(key);
        if(task == null){
            final Task newTask = new Task();
            task = newTask;
            sInstallTaskMap.put(key, newTask);
            KMVolley.getInstance(mContext).getImageLoader().get(assetInfo.getAssetThumbnailURL_S(), new ImageLoader.ImageListener() {
                @Override
                public void onResponse(final ImageLoader.ImageContainer response, boolean isImmediate) {
                    if(LL.D) Log.d(TAG, "Try store thumbnail");
                    if(response != null && response.getBitmap() != null && !response.getBitmap().isRecycled()
                            && mNexAssetService != null && downloadInfo != null){
                        new AsyncTask<Void, Void, Void>(){

                            @Override
                            protected Void doInBackground(Void... voids) {
                                /**
                                 * Bitmap to Bytes
                                 */
                                ByteArrayOutputStream baos = new  ByteArrayOutputStream();
                                response.getBitmap().compress(Bitmap.CompressFormat.PNG,100, baos);
                                byte [] b = baos.toByteArray();

                                /**
                                 * convert Bitmap to String by Base64 for sending SDK
                                 */
                                String temp= Base64.encodeToString(b, Base64.DEFAULT);

                                /**
                                 * Asset Object to Json
                                 */
                                String assetJson = null;
                                try {
                                    JSONObject jsonObject = new JSONObject(new Gson().toJson(assetInfo));
                                    if(jsonObject.optJSONObject("wrapped") != null){
                                        assetJson = jsonObject.optJSONObject("wrapped").toString();
                                    }else{
                                        assetJson = jsonObject.toString();
                                    }
                                } catch (JSONException e) {
                                    Log.e(TAG, "downloadFinishAsset: json Parsing error", e);
                                }

                                /**
                                 * Try to connect to SDK Service
                                 */
                                try {
                                    mNexAssetService.connectInstaller(assetInfo.getAssetIndex(), temp, assetJson, new INexAssetConnectionCallback.Stub(){

                                        @Override
                                        public void onConnectionCompleted(int assetIndex) throws RemoteException {
                                            if(LL.D)
                                                Log.d(TAG, "onConnectionCompleted() called with: assetIndex = [" + assetIndex + "]");
                                            showInstallProgressNotification(assetInfo, 0);
                                            InputStream in = null;
                                            try {

                                                final File assetFile = new File(downloadInfo.getDestinationPath());
                                                in = new FileInputStream(assetFile);

                                                // Transfer bytes from in to out
                                                byte[] buf = new byte[1024 * 64];
                                                int len;
                                                while ((len = in.read(buf)) > 0) {
                                                    String byteString = Base64.encodeToString(buf, Base64.DEFAULT);
                                                    mNexAssetService.sendAssetData(assetIndex, byteString, len, assetFile.length(), new INexAssetInstallCallback.Stub() {
                                                        @Override
                                                        public void onProgressInstall(int assetIndex, int percent, int max) throws RemoteException {
                                                            if(LL.D)
                                                                Log.d(TAG, "onProgressInstall() called with: assetIndex = [" + assetIndex + "], percent = [" + percent + "], max = [" + max + "]");
                                                            newTask.setProgress(percent, max);
                                                            showInstallProgressNotification(assetInfo, percent);
                                                        }

                                                        @Override
                                                        public void onInstallCompleted(int assetIndex) throws RemoteException {
                                                            if(LL.D)
                                                                Log.d(TAG, "onInstallCompleted() called with: assetIndex = [" + assetIndex + "]");
                                                            newTask.signalEvent(Task.Event.COMPLETE);
                                                            mLoadingInstalledAssetTask = null;
                                                            File file = new File(getAssetDownloadPath(mContext, assetIndex));
                                                            if(file.exists()){
                                                                if(LL.D) Log.d(TAG, "onInstallCompleted: try delete download asset file");
                                                                if(file.delete()){
                                                                    if(LL.D) Log.d(TAG, "onInstallCompleted: deleted comleted asset index = " + assetIndex);
                                                                }else{
                                                                    if(LL.D) Log.d(TAG, "onInstallCompleted: delete fail");
                                                                }
                                                            }
                                                            loadInstalledAsset();
                                                            mNotificationManager.cancel(assetIndex);
                                                            String index = String.valueOf(assetIndex);
                                                            sNotificationMap.remove(index);
                                                            sInstallTaskMap.remove(index);
                                                            showNotification(assetInfo);
                                                        }

                                                        @Override
                                                        public void onInstallFailed(int assetIndex, String reason) throws RemoteException {
                                                            newTask.sendFailure(new Task.SimpleTaskError(new Exception(reason), null));
                                                            mNotificationManager.cancel(assetIndex);
                                                            String index = String.valueOf(assetIndex);
                                                            sNotificationMap.remove(index);
                                                            sInstallTaskMap.remove(index);
                                                        }
                                                    });
                                                }
                                                in.close();
                                            } catch (FileNotFoundException e) {
                                                e.printStackTrace();
                                            } catch (IOException e) {
                                                e.printStackTrace();
                                            }
                                        }
                                    });
                                } catch (RemoteException e) {
                                    Log.e(TAG, "downloadFinishAsset: service connection error", e);
                                }
                                return null;
                            }
                        }.executeOnExecutor(sInstallThreadExcutor);
                    }else{
                        newTask.sendFailure(new Task.SimpleTaskError(new Exception("Service is not binded"), null));
                    }
                }

                @Override
                public void onErrorResponse(VolleyError error) {
                    if(LL.E) Log.w(TAG, "onErrorResponse: ", error);
                }
            });
        }
        return task;
    }

    public Task uninstallAsset(int index){
        final Task task = new Task();
        if(mNexAssetService != null){
            try {
                mNexAssetService.uninstallAsset(index, new INexAssetUninstallCallback.Stub() {
                    @Override
                    public void onUninstallCompleted(int assetIndex) throws RemoteException {
                        task.signalEvent(Task.Event.COMPLETE);
                        mLoadingInstalledAssetTask = null;
                        loadInstalledAsset();
                    }

                    @Override
                    public void onUninstallFailed(int assetIndex, String reason) throws RemoteException {
                        task.sendFailure(new Task.SimpleTaskError(new Exception(reason), null));
                    }
                });
            } catch (RemoteException e) {
                if(LL.W) Log.w(TAG, "uninstallAsset: ", e);
                task.sendFailure(new Task.SimpleTaskError(e, null));
            }
        }
        return task;
    }

    private void onBindService(INexAssetService service){
        mNexAssetService = service;
        loadInstalledAsset();
    }

    private void onUnbindService(){
        mNexAssetService = null;
    }

    private void showInstallProgressNotification(StoreAssetInfo assetInfo, int progress){
        if(LL.D)
            Log.d(TAG, "showInstallProgressNotification() called with: assetInfo = [" + assetInfo + "], progress = [" + progress + "]");
        if(assetInfo != null){
            String id = String.valueOf(assetInfo.getAssetIndex());
            NotificationCompat.Builder builder = sNotificationMap.get(id);
            if(builder == null){
                builder = new NotificationCompat.Builder(mContext);
                sNotificationMap.put(id, builder);

                builder.setContentTitle(mContext.getString(R.string.installing))
                        .setColor(mContext.getResources().getColor(R.color.colorPrimary))
                        .setPriority(NotificationCompat.PRIORITY_MAX)
                        .setContentText(assetInfo.getAssetTitle())
                        .setOngoing(true)
                        .setOnlyAlertOnce(true)
                        .setProgress(100, progress, false)
                        .setSmallIcon(android.R.drawable.stat_sys_download);
            }else{
                builder.setProgress(100, progress, false);
            }
            mNotificationManager.notify(assetInfo.getAssetIndex(), builder.build());
        }
    }

    private void showNotification(final StoreAssetInfo assetInfo){
        if(assetInfo != null){
            new Handler(mContext.getMainLooper()).post(new Runnable() {
                @Override
                public void run() {
                    KMVolley.getInstance(mContext).getImageLoader().get(assetInfo.getAssetThumbnailURL_S(), new ImageLoader.ImageListener() {
                        @Override
                        public void onResponse(ImageLoader.ImageContainer response, boolean isImmediate) {
                            if(response != null && response.getBitmap() != null
                                    && !response.getBitmap().isRecycled() && assetInfo != null){

                                final String lang = CommonUtils.getLanguage(mContext);
                                String title = assetInfo.getAssetTitle();
                                if(assetInfo.getAssetNameMap() != null && assetInfo.getAssetNameMap().get(lang) != null){
                                    title = assetInfo.getAssetNameMap().get(lang);
                                }
                                String category = assetInfo.getSubCategoryAliasName();
                                if(assetInfo.getSubCategoryNameMap() != null && assetInfo.getSubCategoryNameMap().get(lang) != null){
                                    category = assetInfo.getSubCategoryNameMap().get(lang);
                                }

                                Intent intent = new Intent(mContext, EnterActivity.class);
                                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                                intent.putExtra(ConfigGlobal.ASSET_STORE_ASSET_INDEX, String.valueOf(assetInfo.getAssetIndex()));
                                intent.putExtra(ConfigGlobal.ASSET_STORE_MULTI_SELECT, false);
                                intent.putExtra("package", mCallingPackage);

                                PendingIntent pendingIntent = PendingIntent.getActivity(mContext, assetInfo.getAssetIndex(), intent, PendingIntent.FLAG_UPDATE_CURRENT);
                                NotificationCompat.Builder builder = new NotificationCompat.Builder(mContext);
                                builder.setContentTitle(mContext.getString(R.string.asset_install_completed))
                                        .setContentText(title)
                                        .setSubText(category)
                                        .setPriority(NotificationCompat.PRIORITY_MAX)
                                        .setSmallIcon(R.drawable.ic_launcher)
                                        .setLargeIcon(response.getBitmap())
                                        .setAutoCancel(true)
                                        .setContentIntent(pendingIntent);
                                mNotificationManager.notify(assetInfo.getAssetIndex(), builder.build());
                            }
                        }

                        @Override
                        public void onErrorResponse(VolleyError error) {

                        }
                    });
                }
            });
        }
    }

    public static String getAssetDownloadPath(Context context, int assetIndex){
        return getAssetDownloadPath(context, String.valueOf(assetIndex));
    }

    public static String getAssetDownloadPath(Context context, String name){
        File f = new File(getAssetFileRootPath(context));
        if(!f.exists()){
            f.mkdirs();
        }
        return f.getAbsolutePath() + File.separator + name + ".zip";
    }

    public static String getAssetImageThumbnailPath(Context context, int assetIndex){
        return getAssetImageThumbnailPath(context, String.valueOf(assetIndex));
    }


    public static String getAssetImageThumbnailPath(Context context, String name){
        File f = new File(getThumbnailRootPath(context));
        if(!f.exists()){
            f.mkdirs();
        }
        return f.getAbsolutePath() + File.separator + name + ".jpg";
    }

    public static String getThumbnailRootPath(Context context){
        return context.getFilesDir() + File.separator + ROOT_THUMBNAIL;
    }

    public static String getAssetFileRootPath(Context context){
        return context.getFilesDir() + File.separator + ROOT_ASSET;
    }

    private ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
            INexAssetService service  = INexAssetService.Stub.asInterface(iBinder);
            onBindService(service);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            onUnbindService();
        }
    };
}
