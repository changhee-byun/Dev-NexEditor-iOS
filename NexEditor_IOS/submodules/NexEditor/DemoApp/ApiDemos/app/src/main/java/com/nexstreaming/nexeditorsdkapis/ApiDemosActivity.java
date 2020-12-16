/******************************************************************************
 * File Name        : ApiDemosActivity.java
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

package com.nexstreaming.nexeditorsdkapis;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.ClipData;
import android.content.DialogInterface;
import android.content.Intent;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdkapis.common.ActivitySave;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;
import com.nexstreaming.nexeditorsdkapis.etc.Template20TestActivity;
import com.nexstreaming.nexeditorsdkapis.qatest.AutoQATestActivity;
import com.nexstreaming.nexeditorsdkapis.qatest.AutoTestActivity;
import com.nexstreaming.nexeditorsdkapis.thumbnail.ThumbnailActivity;

import java.io.File;
import java.util.ArrayList;
import java.util.Set;
import java.util.Collections;
import android.support.annotation.NonNull;

public class ApiDemosActivity extends ListActivity {
    private final String LOG_TAG = "ApiDemo";

    private ArrayList<String> mListFilePath;
    private ArrayList<String> mMenuList;
    private int mSelectedMenuIndex;
    private AlertDialog mDialogAspect;
    private PermissionManager mPermissionManager = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.d(LOG_TAG,"onCreate" );

        setContentView(R.layout.activity_main);

        setTitle("Apis Demos ("+nexApplicationConfig.getSDKVersion()+")");
        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            mPermissionManager = new PermissionManager(this);
            mPermissionManager.setPermissionFlags(
                    PermissionManager.REQUEST_STORAGE |
                            PermissionManager.REQUEST_CAMERA |
                            PermissionManager.REQUEST_MICROPHONE);
            mPermissionManager.requestPermissions();
        }

        mListFilePath = new ArrayList<String>();
        mMenuList = new ArrayList<String>();

        // mj.kong
        if(BuildConfig.directTemplate) {
            int position = 0;
            for( DemoActivityList.DemoActivity activity_ :  DemoActivityList.DemoActivityList ) {
                String naming = activity_.activity.getName();
                String naming_ = getPackageName()+".etc.Template20TestActivity";
                if(naming.equals(naming_)) {
                    mSelectedMenuIndex = position;
                    break;
                }
                position++;
            }
            if( DemoActivityList.DemoActivityList[position].callGallery() ){

                Intent intent = new Intent( );
                intent.setType("*/*");
                String[] mimetypes = new String[]{"image/*", "video/*"};
                intent.putExtra(Intent.EXTRA_MIME_TYPES, mimetypes);
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
                intent.setAction(Intent.ACTION_GET_CONTENT);

                startActivityForResult(intent, 100);
            }
            nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_16v9);
            return;
        }
        //

        for( DemoActivityList.DemoActivity activity :  DemoActivityList.DemoActivityList ){
            mMenuList.add(activity.label());
        }

        final Intent intent = getIntent();
        String auto_test = (intent != null && intent.getExtras() != null) ? intent.getStringExtra("auto_test"):null;
        if( auto_test != null && auto_test.equals("enable")) {

            int ratio = Integer.parseInt(intent.getStringExtra("ratio"));

            Log.e(LOG_TAG, "Auto test Ratio : " + ratio);

            nexApplicationConfig.setAspectMode(ratio);

            final String test_root = intent.getStringExtra("test_root");
            final String result_root = intent.getStringExtra("result_root");

            new Thread(new Runnable() {
                public void run() {
                    String test_action = intent.getStringExtra("test_action");
                    String[] actions = test_action.split(",");
                    for( String action : actions ) {

                        switch (action) {
                            case "thumbnail":
                                Bundle b = intent.getExtras();
                                Intent testIntent = new Intent(getBaseContext(), ThumbnailActivity.class);
                                Set<String> keys = b.keySet();
                                for(String key : keys ) {
                                    testIntent.putExtra(key, b.getCharSequence(key));
                                    Log.e(LOG_TAG, key + " " + b.getCharSequence(key));
                                }

                                startActivity(testIntent);
//                                test_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + test_root + "thumbnail/";
//                                result_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + result_root + "thumbnail/";
//
//                                Log.e(LOG_TAG, "Thumbnail auto test with : " + test_path);
//                                test_thumbnail(test_path, result_path);
                                break;
                            case "direct":
//                                test_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + test_root + "direct/";
//
//                                Log.e(LOG_TAG, "Direct auto test with : " + test_path);
//                                // test_direct(test_path);
                                break;
                            case "template":
//                                test_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + test_root + "template/";
//                                result_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + result_root + "template/";
//
//                                Log.e(LOG_TAG, "Template auto test with : " + test_path);
//                                test_template(test_path, result_path);
                                break;
                        }
                    }
//                    deleteFlagFile(Environment.getExternalStorageDirectory().getAbsoluteFile() + test_root);
                }
            }).start();





            /*
            Bundle b = intent.getExtras();
            Intent testIntent = new Intent(getBaseContext(), AutoTestActivity.class);
            Set<String> keys = b.keySet();
            for(String key : keys ) {
                testIntent.putExtra(key, b.getCharSequence(key));
                Log.e(LOG_TAG, key + " " + b.getCharSequence(key));
            }

            startActivity(testIntent);
            */
            Collections.sort(mMenuList);
            setListAdapter(new ArrayAdapter<String>(ApiDemosActivity.this, android.R.layout.simple_list_item_1, mMenuList));
            return;
        }

        Collections.sort(mMenuList);

        setListAdapter(new ArrayAdapter<String>(ApiDemosActivity.this, android.R.layout.simple_list_item_1, mMenuList));
        mDialogAspect = displaySelectDialog();
        mDialogAspect.show();
        nexEngine.ExportProfile[] profiles = nexEngine.getExportProfile();
        for( nexEngine.ExportProfile p : profiles ){
            Log.d(LOG_TAG,"ExportProfile="+p.toString() );
        }

    }

    private AlertDialog displaySelectDialog() {
        LayoutInflater factory = LayoutInflater.from(this);
        final View dialogView = factory.inflate(R.layout.dialog_main_select_aspect, null);

        Button btn16v9 = (Button)dialogView.findViewById(R.id.button_dialog_main_16v9);

        btn16v9.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_16v9);
                mDialogAspect.dismiss();
            }
        });

        Button btn9v16 = (Button)dialogView.findViewById(R.id.button_dialog_main_9v16);

        btn9v16.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_9v16);
                mDialogAspect.dismiss();
            }
        });

        Button btn1v1 = (Button)dialogView.findViewById(R.id.button_dialog_main_1v1);

        btn1v1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_1v1);
                mDialogAspect.dismiss();
            }
        });

        Button btn2v1 = (Button)dialogView.findViewById(R.id.button_dialog_main_2v1);

        btn2v1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_2v1);
                mDialogAspect.dismiss();
            }
        });

        Button btn1v2 = (Button)dialogView.findViewById(R.id.button_dialog_main_1v2);

        btn1v2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_1v2);
                mDialogAspect.dismiss();
            }
        });

        Button btn4v3 = (Button)dialogView.findViewById(R.id.button_dialog_main_4v3);
        btn4v3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_4v3);
                mDialogAspect.dismiss();
            }
        });

        TextView apiLevel = (TextView) dialogView.findViewById(R.id.textview_dialog_main_api_level);
        apiLevel.setText("API Level: "+nexApplicationConfig.getApiLevel());


        return new AlertDialog.Builder(this)
                .setView(dialogView)
                .setCancelable(false)
                .create();
    }

    public void loadDialog() {
        String label = DemoActivityList.DemoActivityList[mSelectedMenuIndex].label();
        if( ActivitySave.existSaveData(label) ) {
            final AlertDialog.Builder builder = new AlertDialog.Builder(ApiDemosActivity.this);
            builder.setMessage("save data found! do you want last state")
                    .setPositiveButton("Load save", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            Intent intent = DemoActivityList.makeDemoIntent(getBaseContext(), mSelectedMenuIndex);
                            startActivity(intent);

                        }
                    })
                    .setNegativeButton("Delete save", new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            // User cancelled the dialog
                            ActivitySave.setSaveData(null,null);
                            ActivitySave.setLabel(DemoActivityList.DemoActivityList[mSelectedMenuIndex].label());
                            if (DemoActivityList.DemoActivityList[mSelectedMenuIndex].callGallery()) {
                                Intent intent = DemoActivityList.makeGalleryIntent(mSelectedMenuIndex);
                                startActivityForResult(intent, 100);
                            } else {
                                Intent intent = DemoActivityList.makeDemoIntent(getBaseContext(), mSelectedMenuIndex, null);
                                startActivity(intent);
                            }
                        }
                    }).create().show();
        }else{
            ActivitySave.setLabel(DemoActivityList.DemoActivityList[mSelectedMenuIndex].label());
            if (DemoActivityList.DemoActivityList[mSelectedMenuIndex].callGallery()) {
                Intent intent = DemoActivityList.makeGalleryIntent(mSelectedMenuIndex);
                startActivityForResult(intent, 100);
            } else {
                Intent intent = DemoActivityList.makeDemoIntent(getBaseContext(), mSelectedMenuIndex, null);
                startActivity(intent);
            }
        }

    }

    @Override
    public void onListItemClick(ListView parent,View v, int position, long ID){
        super.onListItemClick(parent, v, position, ID);
        String label = mMenuList.get(position);
        for(int i =0; i<mMenuList.size();i++){
            if(label.equals(DemoActivityList.DemoActivityList[i].label())){
                mSelectedMenuIndex = i;
                break;
            }
        }

        Log.d(LOG_TAG,"onListItemClick() select item="+mSelectedMenuIndex);
        loadDialog();
        /*
        if( ActivitySave.existSaveData(label) ){
            Intent intent = DemoActivityList.makeDemoIntent(getBaseContext(), mSelectedMenuIndex);
            startActivity(intent);
        }else {

            if (DemoActivityList.DemoActivityList[mSelectedMenuIndex].callGallery()) {
                Intent intent = DemoActivityList.makeGalleryIntent(mSelectedMenuIndex);
                startActivityForResult(intent, 100);
            } else {
                Intent intent = DemoActivityList.makeDemoIntent(getBaseContext(), mSelectedMenuIndex, null);
                startActivity(intent);
            }
        }
        */
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        Log.d(LOG_TAG, String.format("onActivityResult(%d %d)", requestCode, resultCode));
        if(requestCode == 100 ) {

            if( resultCode == Activity.RESULT_OK ) {

                Log.d(LOG_TAG, "startActivityTime start1");
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
                        Log.d(LOG_TAG, "Selected file = " + filepath);
                    }
                }

                Intent intent = DemoActivityList.makeDemoIntent(getBaseContext(), mSelectedMenuIndex, mListFilePath);

                intent.putExtra("startActivityTime", System.nanoTime());

                Log.d(LOG_TAG, "startActivityTime start2");

                startActivity(intent);
                if( BuildConfig.directTemplate ) {
                    finish();
                }
            }
            else if( resultCode == Activity.RESULT_CANCELED ) {
                if (BuildConfig.directTemplate) {
                    finish();
                }
            }
        }

    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if( mPermissionManager != null )
            mPermissionManager.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }
}
