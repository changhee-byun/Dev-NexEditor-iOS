package com.nexstreaming.nexeditorsdk.service;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.annotation.NonNull;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.store.AssetLocalInstallDB;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.sdk.assetstore.IRemoteServiceCallback;
import com.nexstreaming.sdk.assetstore.IRemoteService;

import java.io.FileNotFoundException;
import java.util.List;


/**
 * Sample Code 이므로 상용버전에서는 그대로 사용하시면 안됩니다.
 * AssetStoreClient 는 singleton 으로 구현되어있으며 Vasset Service 에 바인딩 연결 및 해제를 할 수 있습니다.
 */
public class nexAssetStoreClient {

    /**
     * Vasset Service 에 바인딩 연결 및 해제시 호출되는 리스너 입니다.
     */
    public interface OnConnectionListener {
        /**
         * 바인딩 연결시 호출됩니다.
         */
        void onConnected();

        /**
         * 바인딩 해제시 호출됩니다.
         */
        void onDisconnected();
    }

    private static final String TAG = nexAssetStoreClient.class.getSimpleName();
    private static final String ASSET_STORE_SERVICE_NAME = "RemoteAssetStoreService";
    private static final String ACTION_ASSET_CLIENT_BIND = "com.nexstreaming.app.assetstore.action.bind.RemoteAssetStoreService";
    private static final String KEY_FEATURED_LIST = "featured_list";
    private static final String KEY_FILE_URI = "file_uri";

    private final Context context;
    private ServiceConnection serviceConnection;
    private IRemoteService service;
    private String vendorPackageName;
    private String marketId;
    private int editorSdkLevel;
    private int assetStoreEnvironment;
    private static int serviceNum=0;
    private int instanceNum;

    /**
     * AssetStoreClient 객체를 Singleton 으로 생성하여 반환합니다.
     *
     * @param context               NonNull - Application Context
     * @param vendorPackageName     NonNull - ex) com.nexstreaming.app.vasset.global
     * @param marketId              NonNull - ex) default, default2
     * @param editorSdkLevel        NonNull - ex) 3
     * @param assetStoreEnvironment NonNull - STAGING(2) 상태,  PRODUCTION(3) 상태가 있으며 기본적으로 정수값 3 을 전달해야 합니다.
     * @return Nonnull AssetStoreClient
     */
    public static nexAssetStoreClient create(@NonNull Context context,
                                             @NonNull String vendorPackageName,
                                             @NonNull String marketId,
                                             int editorSdkLevel,
                                             int assetStoreEnvironment) {

        return new nexAssetStoreClient(context, vendorPackageName, marketId, editorSdkLevel, assetStoreEnvironment);
    }

    /**
     * Vasset Service 에 바인딩 합니다.
     *
     * @param connectionListener Vasset Service 에 바인딩 연결 및 해제시 호출되는 리스너 입니다.
     * @return bind 연결 가능시엔 true 를 반환합니다. 불가능하다면 false 를 반환합니다.
     */
    public boolean connect(OnConnectionListener connectionListener) {
        Log.d(TAG, "["+instanceNum+"] connect: " + vendorPackageName);
        serviceConnection = new ClientServiceConnection(connectionListener);
        Intent serviceIntent = new Intent(ACTION_ASSET_CLIENT_BIND);
        serviceIntent.setPackage(context.getPackageName());
        serviceIntent.setClassName(vendorPackageName, vendorPackageName + "." + ASSET_STORE_SERVICE_NAME);
        return context.bindService(serviceIntent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    /**
     * 연결된 Vasset Service 를 바인딩 해제 합니다.
     * 주의: 반드시 사용하는 컴포넌트 LifeCycle 에 맞춰서 연결 해제를 해야 합니다.
     */
    public void disconnect() {
        Log.d(TAG, "["+instanceNum+"] disconnect: " + vendorPackageName);
        if (serviceConnection != null) {
            context.unbindService(serviceConnection);
            service = null;
            serviceConnection = null;
        }
    }

    /**
     * Vasset 으로부터 FeaturedList 를 bundle 에 json 형태로 가져오며 DB 저장 및 썸네일 이미지를 저장합니다.
     *
     * @param featuredType hot = 1 new = 2
     * @throws IllegalArgumentException Service 연결이되지 않은 상태에서 호출시 IllegalStateException 이 발생할 수 있습니다.
     */
    public void getFeaturedList(final int featuredType) {
        if (service != null) {
            try {
                service.getFeaturedList(marketId,
                        editorSdkLevel,
                        assetStoreEnvironment
                        , featuredType, new IRemoteServiceCallback.Stub() {
                            @Override
                            public void onResult(Bundle bundle) throws RemoteException {
                                if (bundle != null) {
                                    String assets = bundle.getString(KEY_FEATURED_LIST);
                                    Log.i(TAG, "["+instanceNum+"] onCall: - " + assets);
                                    if( assets != null ) {
                                        AssetLocalInstallDB.saveFeaturedList(featuredType, assets);
                                        saveRemoteAssetThumbnail(featuredType);
                                    }
                                }
                            }
                        });
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * todo: UI Thread check
     *
     * @param featuredType hot = 1 new = 2
     */
    private void saveRemoteAssetThumbnail(int featuredType) {
        List<AssetLocalInstallDB.remoteAssetItem> list = AssetLocalInstallDB.getInstance(KineMasterSingleTon.getApplicationInstance().getApplicationContext()).getRemoteAssetItem(featuredType);
        //Log.i(TAG, "RemoteAssetThumbnail: - featuredType=" + featuredType+", list.size="+list.size());
        for (AssetLocalInstallDB.remoteAssetItem item : list) {
            String thumbnailURL = item.thumbnailURL;
            final int idx = item.idx;
            //Log.i(TAG, "RemoteAssetThumbnail: - thumbnailURL=" + thumbnailURL+", idx="+idx);
            if (service != null){
                try {
                    service.getImage(thumbnailURL, new IRemoteServiceCallback.Stub() {
                        @Override
                        public void onResult(Bundle bundle) throws RemoteException {
                            if (bundle != null) {
                                Uri fileUri = bundle.getParcelable(KEY_FILE_URI);
                                try {
                                    Bitmap bm = BitmapFactory.decodeStream(context.getContentResolver().openInputStream(fileUri));
                                    Log.i(TAG, "["+instanceNum+"] RemoteAssetThumbnail: - idx=" + idx);
                                    if( bm != null ) {
                                        AssetLocalInstallDB.saveFeaturedThumbnail(idx, bm);
                                    }
                                } catch (FileNotFoundException e) {
                                    e.printStackTrace();
                                }
                            }
                        }
                    });
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    /**
     * 생성자 - Singleton 으로 구현되어있으므로 create() 로 생성해야 합니다.
     *
     * @param context               NonNull - Application Context
     * @param vendorPackageName     NonNull - ex) com.nexstreaming.app.vasset.global
     * @param marketId              NonNull - ex) default, default2
     * @param editorSdkLevel        NonNull - ex) 3
     * @param assetStoreEnvironment NonNull - STAGING(2) 상태,  PRODUCTION(3) 상태가 있으며 기본적으로 정수값 3 을 전달해야 합니다.
     */
    private nexAssetStoreClient(Context context, String vendorPackageName, String marketId, int editorSdkLevel, int assetStoreEnvironment) {
        serviceNum++;
        instanceNum = serviceNum;
        this.context = context;
        this.vendorPackageName = vendorPackageName;
        this.marketId = marketId;
        this.editorSdkLevel = editorSdkLevel;
        this.assetStoreEnvironment = assetStoreEnvironment;
    }

    /**
     * ServiceConnection 을 통하여 바인딩 성공 유무를 알 수 있습니다.
     */
    private class ClientServiceConnection implements ServiceConnection {

        private OnConnectionListener onConnectionListener;

        ClientServiceConnection(OnConnectionListener onConnectionListener) {
            if (onConnectionListener == null) {
                throw new IllegalArgumentException("OnConnectionListener must be not null");
            }
            this.onConnectionListener = onConnectionListener;
        }

        @Override
        public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
            Log.i(TAG,"["+instanceNum+"] onServiceConnected.");
            service = IRemoteService.Stub.asInterface(iBinder);
            onConnectionListener.onConnected();
        }

        @Override
        /**
         * 해당 콜백은 서비스를 호스팅하는 프로세스에서 예기치 않은 오류가 발생하였을 경우 호출됩니다.
         * 참고: https://developer.android.com/reference/android/content/ServiceConnection.html
         */
        public void onServiceDisconnected(ComponentName componentName) {
            Log.i(TAG,"["+instanceNum+"] onServiceDisconnected.");

            onConnectionListener.onDisconnected();
            service = null;
            //onConnectionListener = null;
        }
    }

    public boolean isConnected() {
        return (service!=null);
    }

}
