package com.nexstreaming.editorsimple;

import android.media.MediaScannerConnection;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexAspectProfile;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Calendar;


public class ExportActivity extends ActionBarActivity {
    private enum State {
        NONE(0),
        IDLE(1),
        RUN(2),
        RECORD(3),
        PAUSE(4),
        RESUME(5);

        int value;
        State(int v) {
            value = v;
        }

        int getValue() {
            return value;
        }
    }

    private static final String TAG = "ExportActivity";
    private nexEngine mEngine;
    private nexEngineListener mExportListener = null;
    private File mFile;


    private Spinner mSamplingRateSpinner;
    private ProgressBar mProgressBar;
    private Button mExportStopButton;
    private Button mExportSaveStopButton;
    private Button mPauseResumeButton;
    //private int mAspectMode = 1;
    private nexAspectProfile.ExportProfile [] exportProfiles;
    private boolean mExportCancel;
    private State mState = State.IDLE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_export);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mProgressBar = (ProgressBar) findViewById(R.id.progressbar_export);
        mProgressBar.setProgress(0);
        mProgressBar.setMax(100);

        mExportStopButton = (Button) findViewById(R.id.button_direct_export_test_export_stop);
        mExportStopButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mEngine != null) {
                    if (mState == State.RECORD || mState == State.RESUME || mState == State.PAUSE) {
                        mExportCancel = true;
                        mEngine.stop();
                    }else{
                        finish();
                    }
                }
            }
        });

        mExportSaveStopButton = (Button) findViewById(R.id.button_export_test_export_save_stop);
        mExportSaveStopButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mEngine != null) {
                    if (mState == State.RECORD || mState == State.RESUME || mState == State.PAUSE) {
                        mExportCancel = false;
                        mEngine.exportSaveStop(new nexEngine.OnCompletionListener() {
                            @Override
                            public void onComplete(int resultCode) {
                                Log.d(TAG, "Save export : " + resultCode);
                            }
                        });
                    }else{
                        finish();
                    }
                }
            }
        });

        mPauseResumeButton = (Button) findViewById(R.id.button_export_test_pause_resume);
        mPauseResumeButton.setEnabled(false);
        mPauseResumeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mState == State.RECORD || mState == State.RESUME) {
                    mEngine.exportPause();
                    mState = State.PAUSE;
                    mPauseResumeButton.setText("Resume");
                } else if (mState == State.PAUSE) {
                    mEngine.exportResume();
                    mState = State.RESUME;
                    mPauseResumeButton.setText("Pause");
                }
            }
        });

        final RadioButton high = (RadioButton)findViewById(R.id.radiobutton_export_high);
        exportProfiles = nexApplicationConfig.getAspectProfile().getExportProfiles();
        if( exportProfiles[0].getLevel() == nexAspectProfile.ExportProfileLevel_NONE ){
            high.setVisibility(View.INVISIBLE);
        }else {
            high.setText(""+exportProfiles[0].getWidth()+ " X " + exportProfiles[0].getHeight());
        }

        high.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mState == State.RECORD || mState == State.RESUME) {
                    Log.d(TAG, "State is " + mState);
                    Toast.makeText(getApplicationContext(), "already exporting..." , Toast.LENGTH_SHORT).show();
                    return ;
                }

                int width = exportProfiles[0].getWidth();
                int height = exportProfiles[0].getHeight();

                mState = State.RECORD;
                mProgressBar.setProgress(0);
                Integer samplingRate =  (Integer) mSamplingRateSpinner.getSelectedItem();
                mFile = getExportFile(width, height, samplingRate);

                mEngine.export(mFile.getAbsolutePath(), width, height, 6 * 1024 * 1024, Long.MAX_VALUE, 0, samplingRate);
            }
        });

        final RadioButton middle = (RadioButton)findViewById(R.id.radiobutton_export_middle);
        if( exportProfiles[1].getLevel() == nexAspectProfile.ExportProfileLevel_NONE ){
            middle.setVisibility(View.INVISIBLE);
        }else {
            middle.setText(""+exportProfiles[1].getWidth()+ " X " + exportProfiles[1].getHeight());
        }


        middle.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mState == State.RECORD || mState == State.RESUME) {
                    Log.d(TAG, "State is " + mState);
                    Toast.makeText(getApplicationContext(), "already exporting..." , Toast.LENGTH_SHORT).show();
                    return ;
                }

                int width = exportProfiles[1].getWidth();
                int height = exportProfiles[1].getHeight();

                mState = State.RECORD;
                mProgressBar.setProgress(0);
                Integer samplingRate =  (Integer) mSamplingRateSpinner.getSelectedItem();
                mFile = getExportFile(width, height, samplingRate);
                mEngine.export(mFile.getAbsolutePath(), width, height, 6 * 1024 * 1024, Long.MAX_VALUE, 0, samplingRate);
            }
        });

        final RadioButton low =  (RadioButton)findViewById(R.id.radiobutton_export_low);
        if( exportProfiles[2].getLevel() == nexAspectProfile.ExportProfileLevel_NONE ){
            low.setVisibility(View.INVISIBLE);
        }else {
            low.setText(""+exportProfiles[2].getWidth()+ " X " + exportProfiles[2].getHeight());
        }


        low.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mState == State.RECORD || mState == State.RESUME) {
                    Log.d(TAG, "State is " + mState);
                    Toast.makeText(getApplicationContext(), "already exporting..." , Toast.LENGTH_SHORT).show();
                    return ;
                }

                int width = exportProfiles[2].getWidth();
                int height = exportProfiles[2].getHeight();

                mState = State.RECORD;
                mProgressBar.setProgress(0);
                Integer samplingRate =  (Integer) mSamplingRateSpinner.getSelectedItem();
                mFile = getExportFile(width, height, samplingRate);
                mEngine.export(mFile.getAbsolutePath(), width, height, 6 * 1024 * 1024, Long.MAX_VALUE, 0, samplingRate);
            }
        });

        final RadioButton ori = (RadioButton)findViewById(R.id.radiobutton_export_ori);
        ori.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mState == State.RECORD || mState == State.RESUME) {
                    Log.d(TAG, "State is " + mState);
                    Toast.makeText(getApplicationContext(), "already exporting..." , Toast.LENGTH_SHORT).show();
                    return ;
                }

                for(int i = 0; i < mEngine.getProject().getTotalClipCount(true); i++) {
                    if(mEngine.getProject().getClip(0, true).getWidth() != mEngine.getProject().getClip(i, true).getWidth() ||
                            mEngine.getProject().getClip(0, true).getHeight() != mEngine.getProject().getClip(i, true).getHeight()){
                        Toast.makeText(getBaseContext(), "Contents resolution is different", Toast.LENGTH_SHORT).show();
                        return;
                    }
                }

                int width = mEngine.getProject().getClip(0, true).getWidth();
                int height = mEngine.getProject().getClip(0, true).getHeight();

                mState = State.RECORD;
                mProgressBar.setProgress(0);
                Integer samplingRate =  (Integer) mSamplingRateSpinner.getSelectedItem();
                mFile = getExportFile(width, height, samplingRate);
                mEngine.setExportVideoTrackUUID(true);
                mEngine.set360VideoForceNormalView();
                mEngine.export(mFile.getAbsolutePath(), width, height, 6 * 1024 * 1024, Long.MAX_VALUE, 0, samplingRate);
            }
        });

        mSamplingRateSpinner = (Spinner)findViewById(R.id.spinner_export_test_samplingrate);


        Integer[] items = new Integer[]{8000, 16000, 22050, 24000, 32000, 44100, 48000};
        ArrayAdapter<Integer> adapter = new ArrayAdapter<Integer>(getApplicationContext(), R.layout.simple_spinner_dropdown_item_1, items);

        mSamplingRateSpinner.setAdapter(adapter);
        mSamplingRateSpinner.setSelection(5);


        mExportListener = new nexEngineListener(){
            @Override
            public void onStateChange(int oldState, int newState) {
                Log.d(TAG, "onStateChange() old="+ oldState+", new= "+ newState);
            }

            @Override
            public void onTimeChange(int currentTime) {
                Log.d(TAG, "onTimeChange currentTime: " + currentTime);
            }

            @Override
            public void onSetTimeDone(int currentTime) {
                Log.d(TAG, "onSetTimeDone() =" + currentTime);
            }

            @Override
            public void onSetTimeFail(int err) {

            }

            @Override
            public void onSetTimeIgnored() {

            }

            @Override
            public void onEncodingDone(boolean iserror, int result) {
                Log.d(TAG, "onEncodingDone() error =" + iserror);
                if(iserror) {
                    // for KMSA - 303
                    Log.d(TAG, "Error result=" + result);

                    if(mExportCancel == false)
                    {
                        if(mFile != null) {
                            // To update files at Gallery
                            Log.i(TAG, "To update files at Gallery" + result);
                            MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mFile.getAbsolutePath()}, null, null);
                        }

                        if(nexEngine.nexErrorCode.EXPORT_USER_CANCEL.getValue() == result) {
                            Toast.makeText(getBaseContext(), "Export User Cancel without delete", Toast.LENGTH_SHORT).show();
                        }
                    }
                    else
                    {
                        Toast.makeText(getBaseContext(),"Export Fail!",Toast.LENGTH_SHORT).show();
                        if(mFile!=null) {
                            mFile.delete();
                            mFile = null;
                        }
                    }

                } else {
                    if(mFile != null) {
                        // To update files at Gallery
                        Log.i(TAG, "To update files at Gallery" + result);
                        MediaScannerConnection.scanFile(getApplicationContext(), new String[]{mFile.getAbsolutePath()}, null, null);
                    }
                }

                mProgressBar.setProgress(0);
                mPauseResumeButton.setEnabled(false);
                mState = State.IDLE;
                finish();
            }

            @Override
            public void onPlayEnd() {
                Log.i(TAG, "onPlayEnd() ");
            }

            @Override
            public void onPlayFail(int err, int iClipID) {
                Log.d(TAG,"onPlayFail : err=" + err + " iClipID=" + iClipID );
            }

            @Override
            public void onPlayStart() {
                Log.i(TAG, "onPlayStart() ");
            }

            @Override
            public void onClipInfoDone() {

            }

            @Override
            public void onSeekStateChanged(boolean isSeeking) {
                Log.d(TAG, "onSeekStateChanged() ");
            }

            @Override
            public void onEncodingProgress(int percent) {
                Log.d(TAG, "onEncodingProgress precent: " + percent);
                if(percent > 0)
                {
                    mPauseResumeButton.setEnabled(true);
                }

                mProgressBar.setProgress(percent);
            }

            @Override
            public void onCheckDirectExport(int result) {

            }

            @Override
            public void onProgressThumbnailCaching(int progress, int maxValue) {

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
            public void onPreviewPeakMeter(int iCts, int iPeakMeterValue) {

            }
        };

        mEngine = EditorActivity.getEngine(getApplicationContext());
        mEngine.setEventHandler(mExportListener);
    }

    private File getExportFile(int wid , int hei, int samplingRate) {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        File exportDir = new File(sdCardPath + File.separator + "nexEditorSimple" + File.separator + "Export");
        exportDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String export_time = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File exportFile = new File(exportDir, "EditorSimple_"+wid+"X"+hei + "_" + samplingRate + "_" + export_time+".mp4");
        return exportFile;
    }

    @Override
    public void onBackPressed() {
        Toast.makeText(getBaseContext(), "Please use cancel button if you want finish", Toast.LENGTH_SHORT).show();
    }

    @Override
    protected void onStop() {
        super.onStop();
        if( mState == State.RECORD || mState == State.RESUME || mState == State.PAUSE) {
            mEngine.stop();
            mState = State.IDLE;
        }
    }
}
