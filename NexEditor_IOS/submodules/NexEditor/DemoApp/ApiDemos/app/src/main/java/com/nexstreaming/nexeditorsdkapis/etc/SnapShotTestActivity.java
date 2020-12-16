/******************************************************************************
 * File Name        : SnapShotTestActivity.java
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
import android.content.Context;
import android.hardware.Camera;
import android.media.CamcorderProfile;
import android.media.MediaRecorder;
import android.media.MediaScannerConnection;
import android.os.Environment;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;


public class SnapShotTestActivity extends Activity {
    private static final String TAG = "SnapShotTest";
    private ProgressBar mProgressExportTime;
    private Button mButtonRecordStartStop;
    private Button mButtonExport;
    private Button mButtonDirectExport;
    private TextView mTextViewAddedClicpCount;

    private nexEngine mEngin;
    private nexProject mProject;

    private Camera mCamera;
    private MyCameraSurfaceView mCameraSurfaceView;
    private MediaRecorder mMediaRecorder;

    private String mCurrentRecordedPath;

    private int mAddedClipCount;

    private File mExportedFile;
    private Long mTime;

    boolean mRecording;

    enum ExportMode {EXPORT, DIRECT_EXPORT};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_snap_shot_test);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mRecording = false;
        mAddedClipCount = 0;

        //Get Camera for preview
        mCamera = getCameraInstance();
        if (mCamera == null) {
            Toast.makeText(getApplicationContext(),
                    "Fail to get Camera",
                    Toast.LENGTH_LONG).show();
        }

        mCameraSurfaceView = new MyCameraSurfaceView(getApplicationContext(), mCamera);
        FrameLayout myCameraPreview = (FrameLayout) findViewById(R.id.framelayout_snap_shot_test_videoview);
        myCameraPreview.addView(mCameraSurfaceView);

        mTextViewAddedClicpCount = (TextView) findViewById(R.id.textview_snap_shot_test_added_clip_count);

        mButtonRecordStartStop = (Button) findViewById(R.id.button_snap_shot_test_recordstart);
        mButtonRecordStartStop.setOnClickListener(mButtonRecordStartStopOnClickListener);

        mButtonExport = (Button) findViewById(R.id.button_snap_shot_test_export);
        mButtonExport.setOnClickListener(mButtonExportOnClickListener);

        mButtonDirectExport = (Button) findViewById(R.id.button_snap_shot_test_direct_export);
        mButtonDirectExport.setOnClickListener(mButtonDirectExportOnClickListener);


        mProgressExportTime = (ProgressBar) findViewById(R.id.progress_snap_shot_test_exporttime);
        mProgressExportTime.setMax(100);
        mProgressExportTime.setProgress(0);

        mProject = new nexProject();

        mEngin = ApiDemosConfig.getApplicationInstance().getEngine();
        mEngin.setEventHandler(mEnginListener);
        mEngin.setProject(mProject);

    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        mEngin.stop();
        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG, "onPause");
        if ( mRecording ) {
            mButtonRecordStartStop.performClick();
        } else {
            releaseMediaRecorder();       // if you are using MediaRecorder, release it first
            
        }
        releaseCamera();              // release the camera immediately on pause event
        mEngin.stop();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
    }

    @Override
    protected void onStart() {
        super.onResume();
        Log.d(TAG, "onStart");
        if ( mCamera == null && mRecording == false ) {
            mCamera = getCameraInstance();
        }
    }

    /**------------------------------------------------------------------------
     *                          Private Function                              *
     ------------------------------------------------------------------------**/
    private Camera getCameraInstance(){
        // TODO Auto-generated method stub
        Camera c = null;
        try {
            c = Camera.open(); // attempt to get a Camera instance
        }
        catch (Exception e){
            // Camera is not available (in use or does not exist)
        }
        return c; // returns null if camera is unavailable
    }

    private String getSnapShotFilePath() {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String snapShot = "SnapShot";
        String snapShotDirPath = sdCardPath + File.separator + "KM" + File.separator + snapShot;

        File exportDir = new File(snapShotDirPath);
        exportDir.mkdirs();

        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));

        String filePath = "KineMaster_" + snapShot + "_" + export_time+".mp4";

        return snapShotDirPath + File.separator + filePath;
    }

    private File getExportFile(int width , int height, ExportMode mode) {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String export = null;

        if(mode == ExportMode.EXPORT) {
            export = "export";
        } else if(mode == ExportMode.DIRECT_EXPORT) {
            export = "direct";
        }


        File exportDir = new File(sdCardPath + File.separator + "KM" + File.separator + export);
        exportDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File exportFile = new File(exportDir, "NexEditor_"+width+"X"+height +"_"+ export_time+".mp4");
        return exportFile;
    }

    private boolean prepareMediaRecorder(){
        mCamera = getCameraInstance();

        mCamera.setDisplayOrientation(90);
        //mCamera.setDisplayOrientation(getCameraRotation());

        mMediaRecorder = new MediaRecorder();

        mCamera.unlock();
        mMediaRecorder.setCamera(mCamera);

        mMediaRecorder.setOrientationHint(90);
        //mMediaRecorder.setOrientationHint(getCameraRotation());
        mMediaRecorder.setAudioSource(MediaRecorder.AudioSource.CAMCORDER);
        mMediaRecorder.setVideoSource(MediaRecorder.VideoSource.CAMERA);
        mMediaRecorder.setProfile(CamcorderProfile.get(CamcorderProfile.QUALITY_1080P));

        if(mCurrentRecordedPath == null) {
            Log.e(TAG, "mCurrentRecordedPath is NULL");
            return false;
        }

        mMediaRecorder.setOutputFile(mCurrentRecordedPath);
        mMediaRecorder.setMaxDuration(60000); // Set max duration 60 sec.
        mMediaRecorder.setMaxFileSize(50000000); // Set max file size 50M
        mMediaRecorder.setPreviewDisplay(mCameraSurfaceView.getHolder().getSurface());

        try {
            mMediaRecorder.prepare();
        } catch (IllegalStateException e) {
            releaseMediaRecorder();
            return false;
        } catch (IOException e) {
            releaseMediaRecorder();
            return false;
        }
        return true;

    }

    private void releaseMediaRecorder(){
        if (mMediaRecorder != null) {
            mMediaRecorder.reset();   // clear recorder configuration
            mMediaRecorder.release(); // release the recorder object
            mMediaRecorder = new MediaRecorder();
            mCamera.lock();           // lock camera for later use
        }
    }

    private void releaseCamera(){
        if (mCamera != null){
            mCamera.release();        // release the camera for other applications
            mCamera = null;
        }
    }

    private int getCameraId() {
        int cameraId = -1;
        int numberOfCameras = Camera.getNumberOfCameras();
        for (int i = 0; i < numberOfCameras; i++) {
            Camera.CameraInfo info = new Camera.CameraInfo();
            Camera.getCameraInfo(i, info);
            if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                Log.d(TAG, "Camera found");
                cameraId = i;
                break;
            }
        }

        return cameraId;
    }

    private int getCameraRotation() {
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(getCameraId(), info);
        int rotation = SnapShotTestActivity.this.getWindowManager().getDefaultDisplay().getRotation();
        int degrees = 0 ;
        switch ( rotation ) {
            case Surface.ROTATION_0 : degrees = 0 ; break ;
            case Surface.ROTATION_90 : degrees = 90 ; break ;
            case Surface.ROTATION_180 : degrees = 180 ; break ;
            case Surface.ROTATION_270 : degrees = 270 ; break ;
        }

        int result ;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360 ;
            result = (360 - result) % 360 ;   // compensate the mirror
        } else {   // back-facing
            result = (info.orientation - degrees + 360) % 360 ;
        }

        return result;
    }


    /**------------------------------------------------------------------------
     *                              Listener                                  *
     ------------------------------------------------------------------------**/
    View.OnClickListener mButtonRecordStartStopOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            Log.d(TAG, "mButtonRecordStartOnClickListener onClick: mRecording="+mRecording);
            try{
                if(mRecording) {
                    // stop recording and release camera
                    mMediaRecorder.stop();  // stop the recording
                    releaseMediaRecorder(); // release the MediaRecorder object

                    //Exit after saved
                    //finish();
                    mButtonRecordStartStop.setText("REC");
                    mRecording = false;

                    // add clip to project
                    nexClip clip = new nexClip(mCurrentRecordedPath);
                    if(clip == null) {
                        Log.e(TAG, "clip is null");
                        return ;
                    }

                    if(clip.getRotateInMeta() == 270 || clip.getRotateInMeta() == 90) {
                        clip.setRotateDegree(clip.getRotateInMeta());
                    }

                    mProject.add(clip);
                    mAddedClipCount++;

                    mTextViewAddedClicpCount.setText("addedClip: " + mAddedClipCount);

                    // release
                    mCurrentRecordedPath = null;
                } else {

                    //Release Camera before MediaRecorder start
                    releaseCamera();
                    mCurrentRecordedPath = getSnapShotFilePath();

                    if(!prepareMediaRecorder()){
                        Toast.makeText(getApplicationContext(),
                                "Fail in prepareMediaRecorder()!\n - Ended -",
                                Toast.LENGTH_LONG).show();
                        finish();
                    }

                    mMediaRecorder.start();
                    mRecording = true;
                    mButtonRecordStartStop.setText("STOP");
                }
            }catch (Exception ex) {
                ex.printStackTrace();
                Log.d(TAG, "mButtonRecordStartOnClickListener onClick exception: mRecording="+mRecording);
                releaseMediaRecorder();
                mButtonRecordStartStop.setText("REC");
                mRecording = false;
                mCurrentRecordedPath = null;
            }
        }
    };


    View.OnClickListener mButtonExportOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            Log.d(TAG, "mButtonExportOnClickListener onClick: ");
            if ( mProject.getTotalClipCount(true) == 0 ) {
                Log.d(TAG, "mButtonExportOnClickListener onClick: No clip in the project!");
                return;
            }
            mEngin.updateProject();
            mTime = System.currentTimeMillis();
            Toast.makeText(getApplicationContext(), "export start!!", Toast.LENGTH_SHORT).show();
            mExportedFile = getExportFile(mProject.getClip(0, true).getWidth(), mProject.getClip(0, true).getHeight(), ExportMode.EXPORT);


            mEngin.export(mExportedFile.getAbsolutePath(),
                    mProject.getClip(0, true).getWidth(),
                    mProject.getClip(0, true).getHeight(),
                    mProject.getClip(0, true).getVideoBitrate(),
                    Long.MAX_VALUE,
                    0,
                    44100);
        }
    };

    View.OnClickListener mButtonDirectExportOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            Log.d(TAG, "mButtonDirectExportOnClickListener onClick: ");
            if ( mProject.getTotalClipCount(true) == 0 ) {
                Log.d(TAG, "mButtonDirectExportOnClickListener onClick: No clip in the project!");
                return;
            }
            mEngin.updateProject();
            mTime = System.currentTimeMillis();
            Toast.makeText(getApplicationContext(), "direct start!!", Toast.LENGTH_SHORT).show();
            mExportedFile = getExportFile(mProject.getClip(0, true).getWidth(), mProject.getClip(0, true).getHeight(), ExportMode.DIRECT_EXPORT);
            mEngin.directExport(mExportedFile.getAbsolutePath(), Long.MAX_VALUE, mProject.getTotalTime(), 0);
        }
    };

    nexEngineListener mEnginListener = new nexEngineListener() {
        @Override
        public void onStateChange(int oldState, int newState) {
            Log.d(TAG, "onStateChange oldState: " + oldState + ", newState: " + newState);
        }

        @Override
        public void onTimeChange(int currentTime) {
            Log.d(TAG, "onTimeChange currentTime: " + currentTime);
        }

        @Override
        public void onSetTimeDone(int currentTime) {
            Log.d(TAG, "onSetTimeDone currentTime: " + currentTime);
        }

        @Override
        public void onSetTimeFail(int err) {
            Log.d(TAG, "onSetTimeFail err: " + err);
        }

        @Override
        public void onSetTimeIgnored() {
            Log.d(TAG, "onSetTimeIgnored");
        }

        @Override
        public void onEncodingDone(boolean iserror, int result) {
            Log.d(TAG, "onEncodingDone iserror: " + iserror + ", result: " + result);
            if(iserror) {
                Log.i(TAG, "Error result=" + result);
                mExportedFile.delete();
                mExportedFile = null;
                Log.d(TAG,"Export Fail!");
                Toast.makeText(getApplicationContext(), "Export Fail!", Toast.LENGTH_SHORT).show();
                return;
            } else {
                if(mExportedFile != null) {
                    // To update files at Gallery
                    Log.d(TAG, "To update files at Gallery" + result);

                    MediaScannerConnection.scanFile(getApplicationContext(), new String[] { mExportedFile.getAbsolutePath() }, null, null);

                    Long spendTime = System.currentTimeMillis() - mTime;
                    Toast.makeText(SnapShotTestActivity.this, "spend time =" + spendTime, Toast.LENGTH_LONG).show();
                }
            }
        }

        @Override
        public void onPlayEnd() {
            Log.d(TAG, "onPlayEnd");
        }

        @Override
        public void onPlayFail(int err, int iClipID) {
            Log.d(TAG, "onPlayFail err: " + err + ", iClipID: " + iClipID);
        }

        @Override
        public void onPlayStart() {
            Log.d(TAG, "onPlayStart");
        }

        @Override
        public void onClipInfoDone() {
            Log.d(TAG, "onClipInfoDone");
        }

        @Override
        public void onSeekStateChanged(boolean isSeeking) {
            Log.d(TAG, "onSeekStateChanged isSezeking: " + isSeeking);
        }

        @Override
        public void onEncodingProgress(int percent) {
            Log.d(TAG, "onEncodingProgress percent: " + percent);
            mProgressExportTime.setProgress(percent);
        }

        @Override
        public void onCheckDirectExport(int result) {
            Log.d(TAG, "onCheckDirectExport result: " + result);
        }

        @Override
        public void onProgressThumbnailCaching(int progress, int maxValue) {
            Log.d(TAG, "onProgressThumbnailCaching progress: " + progress + ", maxValue: " + maxValue);
        }

        @Override
        public void onFastPreviewStartDone(int err, int startTime, int endTime) {

        }

        @Override
        public void onFastPreviewStopDone(int err) {

        }

        @Override
        public void onFastPreviewTimeDone(int err) {

        }

        @Override
        public void onPreviewPeakMeter(int iCts, int iLeftValue, int iRightValue) {

        }
    };


    /**------------------------------------------------------------------------
     *                            Inner Class                                 *
     ------------------------------------------------------------------------**/
    public class MyCameraSurfaceView extends SurfaceView implements SurfaceHolder.Callback {

        private SurfaceHolder mHolder;

        public MyCameraSurfaceView(Context context, Camera camera) {
            super(context);
            // Install a SurfaceHolder.Callback so we get notified when the
            // underlying surface is created and destroyed.
            mHolder = getHolder();
            mHolder.addCallback(this);
            // deprecated setting, but required on Android versions prior to 3.0
            mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int weight,
                                   int height) {
            // If your preview can change or rotate, take care of those events here.
            // Make sure to stop the preview before resizing or reformatting it.

            if (mHolder.getSurface() == null || mCamera == null){
                // preview surface does not exist
                return;
            }

            // stop preview before making changes
            try {
                mCamera.stopPreview();
            } catch (Exception e){
                // ignore: tried to stop a non-existent preview
            }

            // make any resize, rotate or reformatting changes here
//            mCamera.setDisplayOrientation(getCameraRotation());

            mCamera.setDisplayOrientation(90);


            // start preview with new settings
            try {
                mCamera.setPreviewDisplay(mHolder);
                mCamera.startPreview();

            }
            catch(Exception e) {

            }
        }

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            // The Surface has been created, now tell the camera where to draw the preview.
            try {
                if ( mCamera != null ) {
                    mCamera.setPreviewDisplay(holder);
                    mCamera.startPreview();
                }
            } catch(IOException e) {
            }
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {

        }
    }
}
