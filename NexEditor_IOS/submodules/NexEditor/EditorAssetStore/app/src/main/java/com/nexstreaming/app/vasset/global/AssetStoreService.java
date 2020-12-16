package com.nexstreaming.app.vasset.global;

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Base64;
import android.util.Log;

import com.android.volley.VolleyError;
import com.android.volley.toolbox.ImageLoader;
import com.google.gson.Gson;
import com.nexstreaming.app.assetlibrary.network.KMVolley;
import com.nexstreaming.app.assetlibrary.network.assetstore.AssetStoreSession;
import com.nexstreaming.app.assetlibrary.network.assetstore.StoreAssetInfo;
import com.nexstreaming.app.assetlibrary.network.assetstore.StoreFeaturedAssetInfo;
import com.nexstreaming.app.assetlibrary.utils.UserInfo;
import com.nexstreaming.app.general.task.ResultTask;
import com.nexstreaming.app.general.task.Task;
import com.nexstreaming.nexeditorsdk.service.INexAssetService;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;

public class AssetStoreService extends Service{

    private final static String TAG = "AssetStoreService";

    private static final int TYPE_JSON_LIST = 0;
    private static final int TYPE_BITMAP_DATA = 1;

    private INexAssetService mNexAssetService;

    public AssetStoreService() {

    }

    @Override
    public void onCreate() {
        super.onCreate();
        if(LL.D) Log.d(TAG, "onCreate() called");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        String callingPackage = intent.getPackage();
        if(callingPackage != null){
            if(LL.D) Log.d(TAG, "onStartCommand: calling package : " + callingPackage);
            Intent serviceIntent = new Intent("com.nexstreaming.nexeditorsdk.service.bind");
            serviceIntent.setClassName(callingPackage, "com.nexstreaming.nexeditorsdk.service.nexAssetService");
            bindService(serviceIntent, serviceConnection, Context.BIND_AUTO_CREATE);
        }
        return START_STICKY;
    }

    private void onServiceBind(INexAssetService service){
        AssetStoreSession session = AssetStoreSession.getInstance(this);
        session.setBaseURL(ConfigGlobal.API_SERVER_URL);
        session.setEdition(ConfigGlobal.API_EDITION);
        session.setUcode(ConfigGlobal.API_APP_UCODE);
        session.setMarketId(ConfigGlobal.API_MARKET_ID);
        session.setClientID(UserInfo.getAppUuid(this));
        session.setAssetEnv(ConfigGlobal.DEFAULT_ASSET_ENV);
        session.beginSession();
        session.precacheCategories();
        loadFeaturedList();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if(mNexAssetService != null){
            unbindService(serviceConnection);
        }
        if(LL.D) Log.d(TAG, "onDestroy() called");
    }

    private void loadFeaturedList(){
        if(LL.D) Log.d(TAG, "loadFeaturedList() called");
//        Hot Assets
        loadFeaturedAssets(1);
        //New Assets
        loadFeaturedAssets(2);
    }

    private void loadFeaturedAssets(final int index){
        AssetStoreSession.getInstance(this).getFeaturedAssets(index, 0).onResultAvailable(new ResultTask.OnResultAvailableListener<StoreFeaturedAssetInfo>() {
            @Override
            public void onResultAvailable(ResultTask<StoreFeaturedAssetInfo> task, Task.Event event, StoreFeaturedAssetInfo result) {
                saveFeaturedAssets(index, result);
            }
        }).onFailure(new Task.OnFailListener() {
            @Override
            public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                Log.w(TAG, "loadFeaturedAssets failure : index [" + index + "]");
            }
        });
    }

    private synchronized void saveFeaturedAssets(final int index, StoreFeaturedAssetInfo assetInfo){
        if(LL.D) Log.d(TAG, "saveFeaturedAssets() called with: index = [" + index + "], assetInfo = [" + assetInfo + "]");
        if(assetInfo != null){
            String json = new Gson().toJson(assetInfo);
            JSONObject jsonObject = null;
            try {
                jsonObject = new JSONObject(json);
            } catch (JSONException e) {
                Log.w(TAG, "FeturedList parsing error", e);
            }

            if(jsonObject != null){
                JSONObject assetJson = jsonObject.optJSONObject("wrapped");
                if(assetJson != null){
                    json = assetJson.toString();
                }else{
                    json = jsonObject.toString();
                }
                saveDataToNexService(TYPE_JSON_LIST, index, json);
            }

            for(final StoreAssetInfo storeAssetInfo : assetInfo.getFeaturedAssetList()){
                KMVolley.getInstance(AssetStoreService.this).getImageLoader().get(storeAssetInfo.getAssetThumbnailURL_S(), new ImageLoader.ImageListener() {
                    @Override
                    public void onErrorResponse(VolleyError error) {

                    }

                    @Override
                    public void onResponse(ImageLoader.ImageContainer response, boolean isImmediate) {
                        if(response != null && response.getBitmap() != null && !response.getBitmap().isRecycled()){
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
                            saveDataToNexService(TYPE_BITMAP_DATA, storeAssetInfo.getAssetIndex(), temp);
                        }
                    }
                });
            }
        }
    }

    private synchronized void saveDataToNexService(int type, int index, String data){
        if(LL.D)
            Log.d(TAG, "saveDataToNexService() called with: type = [" + type + "], index = [" + index + "], data = [" + data + "]");
        try {
            mNexAssetService.saveAssetInfoData(type, index, data);
        } catch (RemoteException e) {
            Log.w(TAG, "saveDataToNexService: service disconnect", e);
        }
    }


    private ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
            mNexAssetService = INexAssetService.Stub.asInterface(iBinder);
            onServiceBind(mNexAssetService);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            mNexAssetService = null;
        }
    };
}
