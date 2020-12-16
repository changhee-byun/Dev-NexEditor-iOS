package com.nexstreaming.editordemo.activity;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.ClipData;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.editordemo.kmsdk.kmEngine;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.utility.convertFilepathUtil;
import com.nexstreaming.editordemo.utility.dataParcel;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;


public class kmIntroActivity extends Activity {
    private static final int EDITOR_DEMO_PERMISSION_REQUEST_STORAGE = 100;

    private static final String TAG = "KM_IntroActivity";

    private TextView m_version;

//    private Button m_button1;
    private Button m_button2;
    private Button m_button3;
    private Button m_button4;

    private ArrayList<String> m_listfilepath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if(kmEngine.getState() == kmEngine.EngineState.NonInitializedState) {
            kmEngine.createEngine();
            nexApplicationConfig.waitForMinimumLoading(this, new Runnable() {
                @Override
                public void run() {
                    Log.d(TAG, "Initialized routines completes");

                }
            });
        }

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
            checkPermission();

        setContentView(R.layout.activity_km_intro);

        String versionName = null;
        try {
            PackageInfo i = getPackageManager().getPackageInfo(getPackageName(), 0);
            versionName = i.versionName;
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        m_listfilepath = new ArrayList<>();

        m_version = (TextView)findViewById(R.id.intro_version);
        m_version.setText("SDK Ver."+nexApplicationConfig.getSDKVersion()+"."+nexApplicationConfig.getDevelopString()+"/APP Ver."+versionName);

// mjkong: delete for request from PM1.
//        m_button1 = (Button)findViewById(R.id.theme);
//        m_button1.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                Intent intent = new Intent();
//                intent.setType(intent.normalizeMimeType("*/*;*/*"));
//                intent.setAction(Intent.ACTION_GET_CONTENT);
//                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
//                startActivityForResult(intent, 100);
//            }
//        });

        m_button2 = (Button) findViewById(R.id.gallery_slideshow);
        m_button2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setType(intent.normalizeMimeType("*/*;*/*"));
                intent.setAction(Intent.ACTION_GET_CONTENT);
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
                startActivityForResult(intent, 200);
            }
        });

        m_button3 = (Button) findViewById(R.id.new_video_editor);
        m_button3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setType(intent.normalizeMimeType("*/*;*/*"));
                intent.setAction(Intent.ACTION_GET_CONTENT);
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
                startActivityForResult(intent, 300);
            }
        });

        m_button4 = (Button) findViewById(R.id.template);
        m_button4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent();
                intent.setType(intent.normalizeMimeType("*/*;*/*"));
                intent.setAction(Intent.ACTION_GET_CONTENT);
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
                startActivityForResult(intent, 400);
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        dataParcel.releaseDataTheme();
        kmEngine.releaseEngine();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if(resultCode == 0) {
            return;
        }

        ClipData clipData = data.getClipData();
        String filepath = null;

        m_listfilepath.clear();

        if(clipData == null) {
            filepath = convertFilepathUtil.getPath(this, data.getData());
            m_listfilepath.add(filepath);
        } else {
            ClipData.Item item;
            Uri uri = null;
            for(int i=0; i<clipData.getItemCount(); i++) {
                item = clipData.getItemAt(i);
                uri = item.getUri();
                filepath = convertFilepathUtil.getPath(this, uri);
                m_listfilepath.add(filepath);
            }
        }

        // 16:9 Aspect Mode
        nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_16v9);

        if(requestCode == 100) {
            dataParcel parcel = dataParcel.getDataParcel(null, m_listfilepath, null);

            Intent intent = new Intent(getBaseContext(), kmThemeListActivity.class);
            intent.putExtra("parcelData", parcel);
            startActivity(intent);
        } else if(requestCode == 200) {
            dataParcel parcel = dataParcel.getDataParcel(null, m_listfilepath, null);

            Intent intent = new Intent(getBaseContext(), kmTransitionListActivity.class);
            intent.putExtra("parcelData", parcel);
            startActivity(intent);
        } else if(requestCode == 300) {
            dataParcel parcel = dataParcel.getDataParcel(null, m_listfilepath, null);

            Intent intent = new Intent(getBaseContext(), kmEditPreviewActivity.class);
            intent.putExtra("parcelData", parcel);
            startActivity(intent);
        } else if(requestCode == 400) {
            dataParcel parcel = dataParcel.getDataParcel(null, m_listfilepath, null);

            Intent intent = new Intent(getBaseContext(), kmAspectModeListActivity.class);
            intent.putExtra("parcelData", parcel);
            startActivity(intent);
        }
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch (requestCode) {
            case EDITOR_DEMO_PERMISSION_REQUEST_STORAGE:
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED
                        && grantResults[1] == PackageManager.PERMISSION_GRANTED) {
                    writeFile();
                    // permission was granted, yay! do the
                    // calendar task you need to do.
                } else {
                    Log.d(TAG, "Permission always deny");
                    // permission denied, boo! Disable the
                    // functionality that depends on this permission.
                }
                break;
        }
    }

    /**
     * Permission check.
     */
    @TargetApi(Build.VERSION_CODES.M)
    private void checkPermission() {
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
                    EDITOR_DEMO_PERMISSION_REQUEST_STORAGE);
        }
    }

    /**
     * Create file example.
     */
    private void writeFile() {
        File file = new File(Environment.getExternalStorageDirectory().getPath() + File.separator + "temp.txt");
        try {
            Log.d(TAG, "create new File : " + file.createNewFile());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
