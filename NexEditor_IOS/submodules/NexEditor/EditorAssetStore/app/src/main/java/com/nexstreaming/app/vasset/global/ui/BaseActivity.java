package com.nexstreaming.app.vasset.global.ui;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.MenuItem;

import com.nexstreaming.app.vasset.global.R;
import com.nexstreaming.nexeditorsdk.service.INexAssetService;


/**
 * Created by ojin.kwon on 2016-11-17.
 */

public class BaseActivity extends AppCompatActivity {

    private static final String TAG = "BaseActivity";

    public static final String ACTION_UPDATE_ASSET_LIST = "com.nexstreaming.app.vasset.global.update.asset.list";

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_UPDATE_ASSET_LIST);
        intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        registerReceiver(broadcastReceiver, intentFilter);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(broadcastReceiver);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()){
            case android.R.id.home :
                onSelectedHome();
                return true;
            case R.id.action_setting :
                onSelectedSetting();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    protected void onSelectedHome(){
        onBackPressed();
    }

    protected void onSelectedSetting(){
        Intent intent = new Intent(this, SettingsActivity.class);
        startActivity(intent);
    }

    protected void onUpdateAssetList(){

    }

    protected void onChangedNetworkConnectivity(){

    }

    private BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if(intent != null){
                String action = intent.getAction();
                if(ACTION_UPDATE_ASSET_LIST.equals(action)){
                    onUpdateAssetList();
                }else if(ConnectivityManager.CONNECTIVITY_ACTION.equals(action)){
                    onChangedNetworkConnectivity();
                }
            }

        }
    };
}
