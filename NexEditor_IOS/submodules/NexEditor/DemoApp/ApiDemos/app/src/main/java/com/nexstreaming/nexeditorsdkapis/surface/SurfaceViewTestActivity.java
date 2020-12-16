package com.nexstreaming.nexeditorsdkapis.surface;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import android.view.SurfaceView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;

import java.util.ArrayList;

public class SurfaceViewTestActivity extends Activity {

    private SurfaceView surfaceView;
    private ArrayList<String> m_listfilepath;
    private nexEngine mEngine;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_surface_view_test);
        surfaceView = (SurfaceView)findViewById(R.id.surfaceview_test);
        m_listfilepath = new ArrayList<String>();
        
        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");
        nexProject project = new nexProject();
        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0),true);

        if( clip.getTotalTime() < 10000){
            Toast.makeText(getApplicationContext(),"Clip duration is lower than 10000.",Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        project.add(clip);
        clip.setRotateDegree(clip.getRotateInMeta());
        clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
        //project.getClip(0,true).getVideoClipEdit().setTrim(0,5000);

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setProject(project);
        mEngine.setView(surfaceView);
        mEngine.play();
    }

    @Override
    protected void onPause() {
        mEngine.stop();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }
}
