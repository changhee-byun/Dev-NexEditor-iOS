package com.nexstreaming.app.assetstore.zte;

import android.Manifest;
import android.annotation.TargetApi;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;

import com.nexstreaming.app.assetlibrary.download.DownloadHelper;
import com.nexstreaming.app.assetlibrary.network.VendorList;
import com.nexstreaming.app.assetlibrary.network.assetstore.AssetStoreSession;
import com.nexstreaming.app.assetstore.zte.asset.AssetManager;
import com.nexstreaming.app.assetstore.zte.ui.BaseActivity;
import com.nexstreaming.app.assetstore.zte.ui.MainActivity;

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

        if(hasPermission()){
            startApplication();
        }else{
            requestPermission();
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    private void bindService(){}

    private void startApplication(){
        AssetManager.init(this, getCallingPackage());
        DownloadHelper.getInstance(this);
        String action = getIntent().getAction();
        Intent intent = new Intent(action);
        intent.setData(getIntent().getData());

        boolean goSetting = getIntent().getBooleanExtra(ConfigGlobal.ASSET_STORE_SETTING, false);
        boolean canMultiSelect = getIntent().getBooleanExtra(ConfigGlobal.ASSET_STORE_MULTI_SELECT, true);
        String assetIndex = getIntent().getStringExtra(ConfigGlobal.ASSET_STORE_ASSET_INDEX);
        String vendor = getIntent().getStringExtra(ConfigGlobal.ASSET_STORE_VENDOR);
        int mimeType = getIntent().getIntExtra(ConfigGlobal.ASSET_STORE_MIME_TYPE, 0);

//        if(vendor == null){
//            vendor = "NexStreaming";
//        }
        AssetStoreSession session = AssetStoreSession.getInstance(this);
        VendorList.Vendor vendorDefine = VendorList.getVendorDefine("ZTE");
        session.setBaseURL(vendorDefine.base_url);
        session.setEdition("TEST");
        session.setUcode(vendorDefine.app_ucode);
        session.setClientID("1234");
//        session.setEdition(UserInfo.getDeviceMaker());
//        session.setUcode(vendorDefine.app_ucode);
//        session.setClientID(UserInfo.getAppUuid(this));
        session.setAssetEnv(AssetStoreSession.AssetEnv.PRODUCTION);
        session.setMarketId(vendorDefine.market_id);
        session.beginSession();
        session.precacheCategories();


        intent.setClass(this, MainActivity.class);
        intent.putExtra(ConfigGlobal.ASSET_STORE_MIME_TYPE, mimeType);
        startActivityForResult(intent, REQUEST_ASSET_STATUS);

//        if(goSetting){
//            intent.setClass(this, SettingsActivity.class);
//            startActivityForResult(intent, REQUEST_ASSET_STATUS);
//        }else if(!canMultiSelect && assetIndex != null){
//            try{
//                Intent detailIntent = AssetDetailActivity.makeIntent(this, Integer.parseInt(assetIndex));
//                detailIntent.putExtra(ConfigGlobal.ASSET_STORE_MULTI_SELECT, canMultiSelect);
//                startActivityForResult(detailIntent, REQUEST_ASSET_STATUS);
//            }catch (NumberFormatException e){
//                Log.w(TAG, "Invalid Asset index : " + assetIndex, e);
//                intent.setClass(this, MainActivity.class);
//                intent.putExtra(ConfigGlobal.ASSET_STORE_MIME_TYPE, mimeType);
//                startActivityForResult(intent, REQUEST_ASSET_STATUS);
//            }
//        }else{
//            intent.setClass(this, MainActivity.class);
//            intent.putExtra(ConfigGlobal.ASSET_STORE_MIME_TYPE, mimeType);
//            startActivityForResult(intent, REQUEST_ASSET_STATUS);
//        }
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