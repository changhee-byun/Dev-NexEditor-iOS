package com.nexstreaming.app.vasset.global;

import android.Manifest;
import android.annotation.TargetApi;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;

import com.nexstreaming.app.assetlibrary.utils.UserInfo;
import com.nexstreaming.app.general.task.ResultTask;
import com.nexstreaming.app.general.task.Task;
import com.nexstreaming.app.vasset.global.asset.AssetManager;
import com.nexstreaming.app.vasset.global.download.DownloadHelper;
import com.nexstreaming.app.assetlibrary.network.assetstore.AssetStoreSession;
import com.nexstreaming.app.assetlibrary.network.assetstore.StoreFeaturedAssetInfo;
import com.nexstreaming.app.vasset.global.ui.AssetDetailActivity;
import com.nexstreaming.app.vasset.global.ui.BaseActivity;
import com.nexstreaming.app.vasset.global.ui.MainActivity;
import com.nexstreaming.app.vasset.global.ui.SettingsActivity;

/**
 * Created by ojin.kwon on 2016-11-18.
 */

public class EnterActivity extends BaseActivity {

    private static final String TAG = "EnterActivity";

    private static final int REQUEST_STORAGE_PERMISSION = 0x741;
    private static final int REQUEST_ASSET_STATUS = 0x554;

    private static final String[] REQUEST_PERMISSIONS = {Manifest.permission.READ_EXTERNAL_STORAGE
            , Manifest.permission.WRITE_EXTERNAL_STORAGE};

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String callingPackage = getCallingPackage();
        if(callingPackage == null){
            callingPackage = getIntent().getStringExtra("package");
        }

        try{
            AssetManager.init(this, callingPackage);
        }catch (Exception e){
            Log.e(TAG, "onCreate: VASSET must be called by other application", e);
            finish();
            return;
        }
        DownloadHelper.getInstance(this);
        startApplication();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        AssetManager.getInstance().dispose();
    }

    private void startApplication(){
        String action = getIntent().getAction();
        final Intent intent = new Intent(action);
        intent.setData(getIntent().getData());

        boolean goSetting = getIntent().getBooleanExtra(ConfigGlobal.ASSET_STORE_SETTING, false);
        boolean canMultiSelect = getIntent().getBooleanExtra(ConfigGlobal.ASSET_STORE_MULTI_SELECT, true);
        String assetIndex = getIntent().getStringExtra(ConfigGlobal.ASSET_STORE_ASSET_INDEX);
//        String vendor = getIntent().getStringExtra(ConfigGlobal.ASSET_STORE_VENDOR);
        final int mimeType = getIntent().getIntExtra(ConfigGlobal.ASSET_STORE_MIME_TYPE, 0);

        AssetStoreSession session = AssetStoreSession.getInstance(this);
        session.setBaseURL(ConfigGlobal.API_SERVER_URL);
        session.setEdition(ConfigGlobal.API_EDITION);
        session.setUcode(ConfigGlobal.API_APP_UCODE);
        session.setMarketId(ConfigGlobal.API_MARKET_ID);
        session.setClientID(UserInfo.getAppUuid(this));
        session.setAssetEnv(ConfigGlobal.DEFAULT_ASSET_ENV);
        session.beginSession();
        session.precacheCategories();

        if(goSetting){
            intent.setClass(this, SettingsActivity.class);
            startActivityForResult(intent, REQUEST_ASSET_STATUS);
        }else if(!canMultiSelect && assetIndex != null){
            try{
                int index = 0;
                if( assetIndex.startsWith("FL") ){
                    index = Integer.parseInt(assetIndex.substring(2));
                    if(LL.D) Log.d(TAG, "Go to featured asset detail");
                    AssetStoreSession.getInstance(this).getFeaturedAssets(index, 0).onResultAvailable(new ResultTask.OnResultAvailableListener<StoreFeaturedAssetInfo>() {
                        @Override
                        public void onResultAvailable(ResultTask<StoreFeaturedAssetInfo> task, Task.Event event, StoreFeaturedAssetInfo result) {
                            if(result != null
                                    && result.getFeaturedAssetList() != null
                                    && result.getFeaturedAssetList().size() > 0){

                                    Intent detailIntent = AssetDetailActivity.makeIntent(EnterActivity.this, result.getFeaturedAssetList().get(0).getAssetIndex());
                                    detailIntent.putExtra(ConfigGlobal.ASSET_STORE_MULTI_SELECT, false);
                                    startActivityForResult(detailIntent, REQUEST_ASSET_STATUS);
                            }else{
                                intent.setClass(EnterActivity.this, MainActivity.class);
                                intent.putExtra(ConfigGlobal.ASSET_STORE_MIME_TYPE, mimeType);
                                startActivityForResult(intent, REQUEST_ASSET_STATUS);
                            }
                        }
                    }).onFailure(new Task.OnFailListener() {
                        @Override
                        public void onFail(Task t, Task.Event e, Task.TaskError failureReason) {
                            if(LL.E) Log.w(TAG, "Go to featured asset failure", failureReason.getException());
                            intent.setClass(EnterActivity.this, MainActivity.class);
                            intent.putExtra(ConfigGlobal.ASSET_STORE_MIME_TYPE, mimeType);
                            startActivityForResult(intent, REQUEST_ASSET_STATUS);
                        }
                    });
                }else {
                    index = Integer.parseInt(assetIndex);
                    Intent detailIntent = AssetDetailActivity.makeIntent(this, index);
                    detailIntent.putExtra(ConfigGlobal.ASSET_STORE_MULTI_SELECT, canMultiSelect);
                    startActivityForResult(detailIntent, REQUEST_ASSET_STATUS);
                }
            }catch (NumberFormatException e){
                Log.w(TAG, "Invalid Asset index : " + assetIndex, e);
                intent.setClass(this, MainActivity.class);
                intent.putExtra(ConfigGlobal.ASSET_STORE_MIME_TYPE, mimeType);
                startActivityForResult(intent, REQUEST_ASSET_STATUS);
            }
        }else{
            intent.setClass(this, MainActivity.class);
            intent.putExtra(ConfigGlobal.ASSET_STORE_MIME_TYPE, mimeType);
            startActivityForResult(intent, REQUEST_ASSET_STATUS);
        }
    }

    private boolean hasPermission() {
        boolean hasPermission = false;
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.M){
            boolean readExt = (PackageManager.PERMISSION_GRANTED==checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE));
            boolean writeExt = (PackageManager.PERMISSION_GRANTED==checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE));
            if( !readExt || !writeExt ) {
                boolean readExtRationale = shouldShowRequestPermissionRationale(Manifest.permission.READ_EXTERNAL_STORAGE);
                boolean writeExtRationale = shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE);
                if( LL.D ) Log.d(TAG, "kmpermission: readExt=" + readExt + " writeExt=" + writeExt + " readExtRationale=" + readExtRationale + " writeExtRationale=" + writeExtRationale);
                hasPermission = false;
            } else {
                if( LL.D ) Log.d(TAG, "kmpermission: readExt=" + readExt + " writeExt=" + writeExt );
                hasPermission = true;
            }
        }else{
            hasPermission = true;
        }
        return hasPermission;
    }

    @TargetApi(Build.VERSION_CODES.M)
    private void requestPermission(){
        requestPermissions(REQUEST_PERMISSIONS, REQUEST_STORAGE_PERMISSION);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if( requestCode == REQUEST_STORAGE_PERMISSION) {
            boolean readExt = false;
            boolean writeExt = false;
            int n = Math.min(permissions.length, grantResults.length);
            for( int i=0; i<n; i++ ) {
                if( permissions[i].equals(Manifest.permission.READ_EXTERNAL_STORAGE) ) {
                    readExt = ( grantResults[i]==PackageManager.PERMISSION_GRANTED);
                } else if( permissions[i].equals(Manifest.permission.WRITE_EXTERNAL_STORAGE) ) {
                    writeExt = ( grantResults[i]==PackageManager.PERMISSION_GRANTED);
                }
            }
            if( readExt && writeExt) {
                startApplication();
            } else {
               finish();
            }
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        switch (requestCode){
            case REQUEST_STORAGE_PERMISSION :
                if(hasPermission()){
                    startApplication();
                }else{
                    finish();
                }
                break;

            case REQUEST_ASSET_STATUS :
                setResult(resultCode);
                finish();
        }
    }
}
