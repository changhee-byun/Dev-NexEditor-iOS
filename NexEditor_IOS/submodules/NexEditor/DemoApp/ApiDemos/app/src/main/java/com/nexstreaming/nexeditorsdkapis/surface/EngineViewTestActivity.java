package com.nexstreaming.nexeditorsdkapis.surface;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexCrop;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.RangeSeekBar;
import com.nexstreaming.nexeditorsdkapis.common.VideoTimeLineView;
import com.nexstreaming.scmbsdk.nxSCMBEntry;
import com.nexstreaming.scmbsdk.nxSCMBSDK;

import java.util.ArrayList;

public class EngineViewTestActivity extends Activity {
    private final static String TAG="EngineViewActivity";
    private nexEngineView engineView;
    private ArrayList<String> m_listfilepath;
    private nexEngine mEngine;
    private VideoTimeLineView mTimeLine;
    private RangeSeekBar mRangeSeekBar;
    private boolean isAvailableView;
    private boolean loadedDetailThumbnails = false;
    private boolean loadedMainThumbnails = false;
    private boolean isAvailableDetailThumbnail = true;
    private nexClip mClip;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_engine_view_test);

        m_listfilepath = new ArrayList<String>();

        mTimeLine = (VideoTimeLineView)findViewById(R.id.timelineview_test);
        mRangeSeekBar = (RangeSeekBar) findViewById(R.id.rangeseekbar_test);
        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");
        nexProject project = new nexProject();
        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0),false);

        if( clip == null ){
            Toast.makeText(getApplicationContext(),"not supported clip.",Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        if( clip.getTotalTime() < 1000){
            Toast.makeText(getApplicationContext(),"Clip duration is lower than 20000ms.",Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        if( clip.getSeekPointInterval() > 2000 ){
            Toast.makeText(getApplicationContext(),"Clip seek interval is higher than 2000ms.",Toast.LENGTH_LONG).show();
            finish();
            return;
        }
        mClip = clip;
        clip.loadVideoClipThumbnails(new nexClip.OnLoadVideoClipThumbnailListener() {
            @Override
            public void onLoadThumbnailResult(int event) {
                loadedMainThumbnails = true;
            }
        });

        nxSCMBSDK instance = nxSCMBSDK.createInstance(m_listfilepath.get(0));
        if( instance != null ){
            nxSCMBEntry[] entry = instance.getEntries();
            if( entry != null ){
                if( entry.length == 0 ){
                    project.add(clip);
                    clip.setRotateDegree(clip.getRotateInMeta());
                    clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                }else {
                    for (int i = 0; i < entry.length; i++) {
                        project.add(nexClip.dup(mClip));
                        project.getLastPrimaryClip().setRotateDegree(mClip.getRotateInMeta());
                        project.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
                        project.getLastPrimaryClip().getVideoClipEdit().setTrim((int)entry[i].getStartMs(), (int)entry[i].getEndMs());
                        project.getLastPrimaryClip().getVideoClipEdit().setSpeedControl(entry[i].getSpeed());
                    }
                }
            }else{
                project.add(clip);
                clip.setRotateDegree(clip.getRotateInMeta());
                clip.getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT);
            }
            instance.destroyInstance();
        }

        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngine.setProject(project);
        engineView = (nexEngineView)findViewById(R.id.engineview_test);
        mEngine.setView(engineView);
        mEngine.updateProject();

        if( project.getTotalTime() < 3000 ) {
            isAvailableDetailThumbnail = false;
        }

        mRangeSeekBar.setRangeValues(0,project.getTotalTime());

        if( project.getTotalClipCount(true) == 1 ){
            mRangeSeekBar.setSelectedMinValue(0);
            mRangeSeekBar.setSelectedMaxValue(project.getTotalTime());
        }else {
            mRangeSeekBar.setSelectedMinValue(project.getClip(1,true).getProjectStartTime());
            mRangeSeekBar.setSelectedMaxValue(project.getClip(1,true).getProjectEndTime());
        }

        mRangeSeekBar.setEnabled(false);
        mRangeSeekBar.setNotifyWhileDragging(true);
        mRangeSeekBar.setOnRangeSeekBarChangeListener(new RangeSeekBar.OnRangeSeekBarChangeListener() {
            @Override
            public void onRangeSeekBarStop(RangeSeekBar bar, boolean isMin, Object minValue, Object maxValue) {
                int min = (int)minValue;
                int max = (int)maxValue;
                boolean isMovedMin = isMin;
                Log.d(TAG,"onRangeSeekBarStop min="+min+", max="+max+", isMovedMin="+isMovedMin);
                int timeMs = isMovedMin?min:max;
                mEngine.seek(timeMs);
            }

            @Override
            public void onRangeSeekBarValuesChanged(RangeSeekBar bar, boolean isMin, Object minValue, Object maxValue) {
                int min = (int)minValue;
                int max = (int)maxValue;
                boolean isMovedMin = isMin;
                int timeMs = isMovedMin?min:max;

                Log.d(TAG,"onRangeSeekBarValuesChanged min="+min+", max="+max+", isMovedMin="+isMovedMin);
                if( isAvailableDetailThumbnail ) {
                    mEngine.seekIDRorI(timeMs);
                }else{
                    mEngine.seek(timeMs);
                }
                mTimeLine.setCurrentPlayTime(timeMs);
            }
        });

        mTimeLine.setTotalPlayTime(mEngine.getProject().getTotalTime());

        mTimeLine.setVideoTimeLineViewChangeListener(new VideoTimeLineView.VideoTimeLineViewChangeListener() {
            @Override
            public void onProgressChanged(int type, float progress, int timeMs, boolean fromUser) {

                if( fromUser ) {
                    if( isAvailableDetailThumbnail ) {
                        if (mEngine.isCacheSeekMode()) {
                            mEngine.seekFromCache(timeMs);
                        } else {
                            mEngine.seekIDRorI(timeMs);
                        }
                    }else{
                        mEngine.seek(timeMs);
                    }
                    mRangeSeekBar.setIndicateMarkValue(timeMs);
                }
            }

            @Override
            public void onStartTrackingTouch(int type, float progress, int timeMs) {

            }

            @Override
            public void onStopTrackingTouch(int type, float progress, int timeMs) {
                if( isAvailableDetailThumbnail ) {
                    if (mEngine.isCacheSeekMode()) {
                        loadedDetailThumbnails = false;
                        mEngine.stopCollectCache(timeMs);
                        mTimeLine.setViewMode(VideoTimeLineView.kViewMode_Full);
                    } else {
                        mEngine.seek(timeMs);
                        mTimeLine.setIgnoreTouchMoveEvent(false);
                    }
                }
            }

            @Override
            public void onLongHoldTouch(int type, float progress, int timeMs) {
                if( isAvailableDetailThumbnail ) {
                    loadedDetailThumbnails = false;
                    mTimeLine.setIgnoreTouchMoveEvent(true);
                    mEngine.startCollectCache(timeMs, new nexEngine.OnCompletionListener() {
                        @Override
                        public void onComplete(int resultCode) {
                            Log.d(TAG, "startCollectCache onComplete=" + resultCode);
                            loadedDetailThumbnails = true;
                            mTimeLine.setIgnoreTouchMoveEvent(false);
                            if (resultCode == 0) {
                                if( mEngine.isCacheSeekMode() ) {
                                    mTimeLine.setViewMode(VideoTimeLineView.kViewMode_Detail);
                                }
                            }
                        }
                    });
                }
            }
        });

        mTimeLine.setVideoTimeLineViewCallbacks( new VideoTimeLineView.VideoTimeLineViewCallbacks() {
            @Override
            public int waitTimeMsToLoading() {
                if( mEngine.isCacheSeekMode() ) {
                    if (loadedDetailThumbnails) {
                        return 0;
                    }
                }else {
                    if (loadedMainThumbnails) {
                        return 0;
                    }
                }
                return 500;
            }

            @Override
            public Bitmap onGetDetailThumbnail(int Time) {
                if( isAvailableDetailThumbnail ) {
                    if (mEngine != null) {
                        if (mEngine.isCacheSeekMode()) {
                            int rotate = mClip.getRotateDegree();
                            return mEngine.getThumbnailCache(Time, rotate);
                        }
                    }
                }
                return null;
            }

            @Override
            public Bitmap onGetMainThumbnail(int Time) {
                if( mEngine != null) {
                    if( !mEngine.isCacheSeekMode() ) {
                        int clipIndex = mEngine.getProject().getClipPosition(Time);
                        if( clipIndex < 0 ){
                            return null;
                        }
                        int rotate = mEngine.getProject().getClip(clipIndex,true).getRotateDegree();
                        int clipTime = mEngine.getProject().getProjectTime2ClipTimePosition(clipIndex,Time);
                        return mClip.getVideoClipTimeLineThumbnail(rotate,clipTime,false,false);
                    }
                }
                return null;
            }
        });


        engineView.setListener(new nexEngineView.NexViewListener() {
            @Override
            public void onEngineViewAvailable(int width, int height) {
                if( !isAvailableView ) {
                    isAvailableView = true;
                    //mEngine.play();
                    mEngine.seek(0);
                }
            }

            @Override
            public void onEngineViewSizeChanged(int width, int height) {

            }

            @Override
            public void onEngineViewDestroyed() {
                isAvailableView = false;
            }
        });
    }

    @Override
    protected void onResume() {
        if( isAvailableView ) {
            //mEngine.play();
            mEngine.seek(0);
        }
        super.onResume();
    }


    @Override
    protected void onPause() {
        mEngine.stop();

        super.onPause();
    }

}
