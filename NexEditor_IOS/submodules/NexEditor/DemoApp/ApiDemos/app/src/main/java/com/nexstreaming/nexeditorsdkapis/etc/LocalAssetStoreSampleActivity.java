/******************************************************************************
 * File Name        : LocalAssetStoreSampleActivity.java
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

package com.nexstreaming.nexeditorsdkapis.etc;

import android.app.Activity;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;
import com.nexstreaming.nexeditorsdkapis.R;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class LocalAssetStoreSampleActivity extends Activity {
    private Button mButton;
    private ProgressBar mProgressBar;
    private ImageView mImageView;
    private boolean installMode;
    private int sampleAssetIdx = 717;
    private static final String localPath = Environment.getExternalStorageDirectory().getAbsolutePath() +File.separator+".localassets";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_local_asset_store_sample);

        File dir = new File(localPath);
        if( !dir.exists() ) {
            dir.mkdirs();
        }
        nexApplicationConfig.setAssetStoreRootPath(dir.getAbsolutePath());
        nexApplicationConfig.setAssetInstallRootPath(dir.getAbsolutePath());

        mImageView = (ImageView)findViewById(R.id.imageView_localAssetStore);
        mImageView.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.raw.jpg717));

        mProgressBar = (ProgressBar)findViewById(R.id.progressBar_localAssetStore);
        mButton = (Button)findViewById(R.id.button_localAssetStore);
        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                buttonProcess();
            }
        });
        checkMode( );
    }

    private void checkMode( ){

        nexAssetPackageManager.Asset asset = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).getInstalledAssetByIdx(sampleAssetIdx);
/*
        if( asset != null ){
            installMode = false;
            mButton.setText("delete");
        }else{
        */
            installMode = true;
            mButton.setText("download");
        //}
        mButton.setEnabled(true);
    }

    private void buttonProcess(){
        mButton.setEnabled(false);
        if( installMode ){
            downloadAssetComponent();
            //installAssetComponent();
            installAssetComponent3(717);

            downloadAssetComponent2();
            installAssetComponent3(100);
        }else{
            nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).uninstallPackageByAssetIdx(sampleAssetIdx);
            checkMode( );
        }
    }

    private void download(InputStream in , String filename ){
        //InputStream in = getResources().openRawResource(R.raw.jpg100);
        File thumb = new File(localPath,filename);

        try {
            int count = 0;
            FileOutputStream os = new FileOutputStream(thumb);
            byte[] bytes = new byte[32768];
            while ( (count = in.read(bytes,0, 32768)) > 0) {
                os.write(bytes,0,count);
            }

            in.close();
            os.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void downloadAssetComponent(){
        download(getResources().openRawResource(R.raw.jpg717),"717.jpg" );
        download(getResources().openRawResource(R.raw.json717),"717.json" );
        download(getResources().openRawResource(R.raw.zip717),"717.zip" );
    }

    private void downloadAssetComponent2(){
        download(getResources().openRawResource(R.raw.jpg100),"100.jpg" );
        download(getResources().openRawResource(R.raw.json100),"100.json" );
        download(getResources().openRawResource(R.raw.zip100),"100.zip" );
    }


    private void installAssetComponent(){
        int count = nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).findNewPackages();
        if( count > 0 ){
            mProgressBar.setMax(100);
            mProgressBar.setProgress(0);
            nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).installPackagesAsync(new nexAssetPackageManager.OnInstallPackageListener() {
                @Override
                public void onProgress(int countPackage, int totalPackages, int progressInstalling) {
                    mProgressBar.setProgress(progressInstalling);
                }

                @Override
                public void onCompleted(int event, int assetIdx) {
                    checkMode();
                }
            });
        }else{
            checkMode();
        }
    }

    private void installAssetComponent3(int AssetIdx){
        mProgressBar.setMax(100);
        mProgressBar.setProgress(0);

        nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).installPackagesAsync(AssetIdx,new nexAssetPackageManager.OnInstallPackageListener() {
            @Override
            public void onProgress(int countPackage, int totalPackages, int progressInstalling) {
                mProgressBar.setProgress(progressInstalling);
            }

            @Override
            public void onCompleted(int event, int assetIdx) {
                Log.d("install", "onCompleted="+assetIdx);
                if( event == 0 ) {
                    checkMode();
                }else{
                    ;
                }
            }
        });
    }


    @Override
    public void onBackPressed() {
        //super.onBackPressed();
        Intent intent = new Intent();
        setResult(RESULT_OK, intent);
        finish();
    }
}
