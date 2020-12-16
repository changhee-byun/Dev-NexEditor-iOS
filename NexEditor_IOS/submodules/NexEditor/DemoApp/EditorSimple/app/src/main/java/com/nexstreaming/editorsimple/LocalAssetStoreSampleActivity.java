package com.nexstreaming.editorsimple;

import android.content.Intent;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.ActionBarActivity;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;
import com.nexstreaming.nexeditorsdk.service.nexAssetService;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class LocalAssetStoreSampleActivity extends ActionBarActivity {
    private Button mButton;
    private ProgressBar mProgressBar;
    private ImageView mImageView;
    private boolean installMode;
    private int sampleAssetIdx = 100;
    private static final String localPath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator+".localassets";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_local_asset_store_sample);

        //1.
        File dir = new File(localPath);
        if( !dir.exists() ) {
            dir.mkdirs();
        }
        nexApplicationConfig.setAssetStoreRootPath(dir.getAbsolutePath());
        nexApplicationConfig.setAssetInstallRootPath(dir.getAbsolutePath());

        mImageView = (ImageView)findViewById(R.id.imageView_localAssetStore);
        mImageView.setImageBitmap(BitmapFactory.decodeResource(getResources(), R.raw.jpg100));

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
        if( asset != null ){
            installMode = false;
            mButton.setText("delete");
        }else{
            installMode = true;
            mButton.setText("download");
        }
        mButton.setEnabled(true);
    }

    private void buttonProcess(){
        mButton.setEnabled(false);
        if( installMode ){
            downloadAssetComponent();
            installAssetComponent();
        }else{
            nexAssetPackageManager.getAssetPackageManager(getApplicationContext()).uninstallPackageByAssetIdx(sampleAssetIdx);
            checkMode( );
            Intent intent = new Intent(nexAssetService.ACTION_ASSET_UNINSTALL_COMPLETED);
            intent.putExtra("index", sampleAssetIdx);
            getApplicationContext().sendBroadcast(intent);
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
                public void onCompleted(int event) {
                    checkMode();
                    Intent intent = new Intent(nexAssetService.ACTION_ASSET_INSTALL_COMPLETED);
                    intent.putExtra("index", 0);
                    intent.putExtra("category.alias", "Template");
                    getApplicationContext().sendBroadcast(intent);
                }
            });
        }else{
            checkMode();
        }
    }
}
