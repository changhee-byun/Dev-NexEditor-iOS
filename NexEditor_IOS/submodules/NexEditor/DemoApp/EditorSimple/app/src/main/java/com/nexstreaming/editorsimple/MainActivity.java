package com.nexstreaming.editorsimple;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.ClipData;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAssetStoreAppUtils;
import com.nexstreaming.nexeditorsdk.nexEngine;

import java.util.ArrayList;

public class MainActivity extends AppCompatActivity {
    private ArrayList<String> mListFilePath;
    final static String TAG = "EditorSimple";
    private Spinner vendor;
    private Switch localAssetStore;
    private boolean bLocalStore;
    private RadioGroup servers;
    private static final int MY_PERMISSION_REQUEST_STORAGE = 12345;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        checkPermission();

        servers = (RadioGroup)findViewById(R.id.radioGroup_server);

        mListFilePath = new ArrayList<String>();
        Button bt = (Button)findViewById(R.id.button_pickup);
        Log.d(TAG, "Version: v1.3");
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                String vendorName = (String)vendor.getSelectedItem();
                nexAssetStoreAppUtils.setVendor(vendorName);
                int id = servers.getCheckedRadioButtonId();
                int serverType = nexAssetStoreAppUtils.AssetStoreServerPRODUCTION;
                switch(id){
                    case R.id.radioButton_server_drift:
                        serverType = nexAssetStoreAppUtils.AssetStoreServerDRAFT;
                        break;

                    case R.id.radioButton_server_staging:
                        serverType = nexAssetStoreAppUtils.AssetStoreServerSTAGING;
                        break;
                    default:
                        break;
                }
                nexAssetStoreAppUtils.setServer(serverType);
                Intent intent = new Intent( );
                intent.setType("video/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE,false);
                intent.setAction(Intent.ACTION_GET_CONTENT);
                startActivityForResult(intent, 100);
            }
        });

        vendor = (Spinner)findViewById(R.id.spinner_vender);
        String[] vendorNames ={"NexStreaming","LGE","ZTE"};

        localAssetStore = (Switch)findViewById(R.id.switch1);
        localAssetStore.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if( isChecked ){
                    vendor.setEnabled(false);
                    bLocalStore = true;
                }else{
                    vendor.setEnabled(true);
                    bLocalStore = false;
                }
            }
        });


        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                R.layout.simple_spinner_dropdown_item_1, vendorNames);
        vendor.setAdapter(adapter);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 100 && resultCode == Activity.RESULT_OK) {

            ClipData clipData = data.getClipData();
            String filepath = null;

            mListFilePath.clear();

            if (clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                mListFilePath.add(filepath);
            } else {
                ClipData.Item item;
                Uri uri = null;
                for (int i = 0; i < clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);
                    mListFilePath.add(filepath);
                }
            }

            Intent intent = new Intent(this, EditorActivity.class);
            intent.putStringArrayListExtra("filelist", mListFilePath);
            intent.putExtra("localAssetStore",bLocalStore);
            startActivity(intent);
        }
    }

    /**
     * Permission check.
     */
    private void checkPermission() {
        if ( Build.VERSION.SDK_INT >= Build.VERSION_CODES.M ) {
            if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    != PackageManager.PERMISSION_GRANTED
                    || checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
                    != PackageManager.PERMISSION_GRANTED) {

                // Should we show an explanation?
                if (shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                    // Explain to the user why we need to write the permission.
                    Toast.makeText(this, "Read/Write external storage", Toast.LENGTH_SHORT).show();
                }

                requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE},
                        MY_PERMISSION_REQUEST_STORAGE);

                // MY_PERMISSION_REQUEST_STORAGE is an
                // app-defined int constant

            } else {
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch (requestCode) {
            case MY_PERMISSION_REQUEST_STORAGE:
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED
                        && grantResults[1] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted, yay! do the
                    // calendar task you need to do.

                } else {

                    Log.d(TAG, "Permission always deny");
                    finish();
                    // permission denied, boo! Disable the
                    // functionality that depends on this permission.
                }
                break;
        }
    }

}
