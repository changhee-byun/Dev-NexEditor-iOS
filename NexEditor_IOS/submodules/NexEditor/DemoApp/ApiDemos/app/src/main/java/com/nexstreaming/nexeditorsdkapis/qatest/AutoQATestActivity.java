package com.nexstreaming.nexeditorsdkapis.qatest;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.session.MediaController;
import android.net.Uri;
//import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.Constants;
import com.nexstreaming.nexeditorsdkapis.effect.EffectPreviewActivity;

import java.io.IOException;
import java.util.concurrent.BlockingQueue;

import static com.nexstreaming.nexeditorsdkapis.qatest.TestCaseManager.*;

public class AutoQATestActivity extends Activity {
    private ListView mList;
    private Button mTestRun;
    private ProgressBar mProgress;
    private TestCaseManager tests = new TestCaseManager();
    private AutoQATestAdapter mAdapter;
    private AlertDialog mDlgPromotionVideo;
    private AlertDialog mDialogResult;

    private OnVideoViewPlayListener mVideoViewPlayListener = new OnVideoViewPlayListener() {
        @Override
        public void onPlay( final VideoViewTestCase tc) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    setShowDlgPromotionVideo(true, tc);
                }
            });
        }
    };

    private OnEditorPlayListener mEditorPlayListener = new OnEditorPlayListener() {
        @Override
        public void onPlay(final VideoViewTestCase tc) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    setShowDlgnexEditorVideo(true, tc);
                }
            });
        }
    };

    private OnStateChangeListener mStateChangeListener = new OnStateChangeListener() {
        @Override
        public void onStateChange(int index, TestCaseResult result) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mProgress.setProgress(tests.getCurrentPosition() + 1);
                    mAdapter.notifyDataSetChanged();
                }
            });
        }

        @Override
        public void onComplete() {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mAdapter.notifyDataSetChanged();
                    Toast.makeText(getApplicationContext(), "Test Complete!", Toast.LENGTH_SHORT).show();
                    mTestRun.setText("Start");
                }
            });
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_auto_qatest);
        QAConfig.setContext(getApplicationContext());

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mList = (ListView)findViewById(R.id.listView_qatest);
        mProgress = (ProgressBar)findViewById(R.id.progressBar_qatest);

        mProgress.setMax(getTotalCount());

        mAdapter = new AutoQATestAdapter(TestCaseManager.getReport());

        mList.setAdapter(mAdapter);

        mList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                boolean on = true;
                if( tests == null )
                    return;

                on = !tests.isRunning();

                if( on ) {
                    mDialogResult = displayResultDialog(position);
                    mDialogResult.show();
                }
            }
        });

        mTestRun = (Button)findViewById(R.id.button_qatest_all);
        mTestRun.setText("Start");
        mTestRun.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                boolean on = true;

                if (tests == null) {
                    mTestRun.setText("Stop");
                    makeTestManager();
                    tests.start();
                } else {
                    on = !tests.isRunning();
                    if (on) {
                        mTestRun.setText("Stop");
                        makeTestManager();
                        tests.start();
                    } else {
                        tests.cancel();
                    }
                }

                if (tests != null)

                    if (on) {
                        mTestRun.setText("Stop");
                    }
            }
        });

    }

    private void makeTestManager(){
        tests = new TestCaseManager();

        tests.setOnVideoViewPlayListener(mVideoViewPlayListener);
        tests.setStateChangeListener(mStateChangeListener);
        tests.setOnEditorPlayListener(mEditorPlayListener);
    }

    private AlertDialog displayResultDialog(int index) {
        LayoutInflater factory = LayoutInflater.from(this);
        final View dialogView = factory.inflate(R.layout.dialog_qatest_result, null);
        final int pos = index;
        Button bntok = (Button)dialogView.findViewById(R.id.button_qatest_dialog_ok);

        bntok.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mDialogResult.dismiss();
            }
        });

        Button bntretest = (Button)dialogView.findViewById(R.id.button_qatest_dialog_retest);

        bntretest.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mDialogResult.dismiss();
                makeTestManager();
                tests.setSingleTest(pos);
            }
        });


        TextView report = (TextView) dialogView.findViewById(R.id.textview_qatest_dialog_result);
        byte[] log = getReportLog(index);
        if( log != null ) {
            String result = new String(log,0,log.length);
            report.setText(result);
        }else{
            report.setText("Log file read error!");
            bntretest.setVisibility(View.INVISIBLE);
        }

        return new AlertDialog.Builder(this)
                .setView(dialogView)
                .setCancelable(false)
                .create();
    }

    public void setShowDlgPromotionVideo(boolean isShow, final VideoViewTestCase tc) {
        if (mDlgPromotionVideo != null) {
            mDlgPromotionVideo.cancel();
            mDlgPromotionVideo = null;
        }

        if (isShow) {

            View li = LayoutInflater.from(this).inflate(R.layout.activity_video_view, null);
            final VideoView videoView = (VideoView) li.findViewById(R.id.videoview);

//            videoView.setVideoPath(path);
            videoView.setMediaController(new android.widget.MediaController(this));

            videoView.requestFocus();

            tc.Run(videoView);

            AlertDialog.Builder builder = new AlertDialog.Builder(this);

            builder.setMessage(tc.getMessage() )
                    .setNeutralButton("OK", new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            if( tc.Ok() ) {
                                videoView.stopPlayback();
                                dialog.cancel();
                            }else{
                                mDlgPromotionVideo.setMessage(tc.getMessage());
                                tc.Run(videoView);
                            }
                        }
                    }).setNegativeButton("NOK", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            if( tc.Nok() ) {
                                videoView.stopPlayback();
                                dialog.cancel();
                            }else{
                                mDlgPromotionVideo.setMessage(tc.getMessage());
                                tc.Run(videoView);
                            }
                        }
            }).setView(li);
            mDlgPromotionVideo = builder.show();

        }

    }

    public void setShowDlgnexEditorVideo(boolean isShow, final VideoViewTestCase tc) {
        if (mDlgPromotionVideo != null) {
            mDlgPromotionVideo.cancel();
            mDlgPromotionVideo = null;
        }

        if (isShow) {

            View li = LayoutInflater.from(this).inflate(R.layout.dialog_nexeditor_videoplay, null);
            nexEngineView videoView = (nexEngineView) li.findViewById(R.id.engineview_qatest);
            nexEngine engine = QAConfig.getEngin();
            engine.setView(videoView);

            tc.Run(null);
            AlertDialog.Builder builder = new AlertDialog.Builder(this);

            builder.setMessage(tc.getMessage() )
                    .setNeutralButton("OK", new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            if( tc.Ok() ) {
                                QAConfig.getEngin().stop();
                                dialog.cancel();
                            }else{
                                mDlgPromotionVideo.setMessage(tc.getMessage());
                                tc.Run(null);
                            }
                        }
                    }).setNegativeButton("NOK", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    if( tc.Nok() ) {
                        QAConfig.getEngin().stop();
                        dialog.cancel();
                    }else{
                        mDlgPromotionVideo.setMessage(tc.getMessage());
                        tc.Run(null);
                    }
                }
            }).setView(li);
            mDlgPromotionVideo = builder.show();

        }

    }


}
