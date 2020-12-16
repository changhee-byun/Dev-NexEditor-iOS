/******************************************************************************
 * File Name        : ThumbnailCacnelTestActivity.java
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

package com.nexstreaming.nexeditorsdkapis.thumbnail;

import android.app.Activity;
import android.content.ClipData;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Handler;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.ConvertFilepathUtil;

import java.util.ArrayList;


public class ThumbnailCacnelTestActivity extends Activity {
    private static final String TAG="ThumbnailCacnelTest";

    private ArrayList<String> mListFilePath;
    private nexClip mClip;


    private Button mButtonAdd;
    private Button mButtonTestStart;

    private SeekBar mSeekBarCancelBefore;
    private SeekBar mSeekBarCancelAfter;
    private SeekBar mSeekBarCancelTime;

    private TextView mTextViewResult;

    private Switch mNoCache;
    private Switch mYonly;

    private int mCancelBeforeCount = 0;
    private int mCancelAfterCount = 0;
    private int mCancelTime = 0;
    private int mEndTime = 0;
    private int mInterval = 2000;

    private StringBuilder mResult;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_thumbnail_cacnel_test);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mListFilePath = new ArrayList<String>();

        mButtonAdd = (Button)findViewById(R.id.button_thumnail_cancel_add);
        mButtonAdd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //Log.d(TAG, "mButtonAdd click");
                Intent intent = new Intent( );
                intent.setType("video/*");
                intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
                intent.setAction(Intent.ACTION_GET_CONTENT);

                startActivityForResult(intent, 100);
            }
        });

        mNoCache = (Switch)findViewById(R.id.switch_nocache);
        mYonly = (Switch)findViewById(R.id.switch_yonly);

        mButtonTestStart = (Button)findViewById(R.id.button_thumbnail_cancel_teststart);
        mButtonTestStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "mButtonTestStart Click!");
                Log.d(TAG, "mCancelBeforeCount: " + mCancelBeforeCount);
                Log.d(TAG, "mCancelAfterCount: " + mCancelAfterCount);
                Log.d(TAG, "mCancelTime: " + mCancelTime);
                updateTextViewResult("");

                mClip = new nexClip(mListFilePath.get(0));
                mResult = new StringBuilder();
                mEndTime = 0;
                final boolean noCache = mNoCache.isChecked();

                Log.i(TAG, "file="+mClip.getPath());

                if( mClip.getClipType() != nexClip.kCLIP_TYPE_VIDEO  ){
                    Toast.makeText(getApplicationContext(), "No Video Clips.", Toast.LENGTH_LONG).show();
                    finish();
                    return;
                }

                // start get thumbnail for beforecount
                for (int i = 0; i < mCancelBeforeCount; i++) {
                    final int idx = i;
                    mEndTime += mInterval;

                    if( mYonly.isChecked() ){
                        mClip.getVideoClipIDR2YOnlyThumbnails(160,90,0,0,50,new nexClip.OnGetVideoClipIDR2YOnlyThumbnailsListener() {
                            @Override
                            public void onGetVideoClipIDR2YOnlyThumbnailsResult(int event, byte[] data, int index, int totalCount, int timestamp) {
                                if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {
                                    mResult.append("Before Y Only kEvent_Ok" + idx + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before Y Only  kEvent_Ok" + idx);
                                } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Completed) {
                                    mResult.append("Before Y Only  kEvent_Completed" + idx + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before Y Only  kEvent_Completed" + idx);
                                } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Fail) {
                                    mResult.append("Before Y Only  kEvent_Fail" + idx + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before Y Only  kEvent_Fail" + idx);
                                } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_systemError) {
                                    mResult.append("Before Y Only kEvent_systemError" + idx + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before Y Only kEvent_systemError" + idx);
                                } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_UserCancel) {
                                    mResult.append("Before Y Only kEvent_UserCancel" + idx + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before Y Only kEvent_UserCancel" + idx);
                                } else {
                                    mResult.append("Before Y Only default event=" + event + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before Y Only default event=" + event);
                                }

                            }
                        });
                    }else {
                        mClip.getVideoClipDetailThumbnails(160, 90, 0, 0, 50, 0, noCache, null, new nexClip.OnGetVideoClipDetailThumbnailsListener() {
                            @Override
                            public void onGetDetailThumbnailResult(int event, Bitmap bm, int index, int totalCount, int timestamp) {
                                if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {
                                    mResult.append("Before kEvent_Ok" + idx + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before kEvent_Ok" + idx);
                                } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Completed) {
                                    mResult.append("Before kEvent_Completed" + idx + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before kEvent_Completed" + idx);
                                } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Fail) {
                                    mResult.append("Before kEvent_Fail" + idx + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before kEvent_Fail" + idx);
                                } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_systemError) {
                                    mResult.append("Before kEvent_systemError" + idx + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before kEvent_systemError" + idx);
                                } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_UserCancel) {
                                    mResult.append("Before kEvent_UserCancel" + idx + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before kEvent_UserCancel" + idx);
                                } else {
                                    mResult.append("Before default event=" + event + "\n");
                                    updateTextViewResult(mResult.toString());

                                    Log.d(TAG, "Before default event=" + event);
                                }
                            }
                        });
                    }
                }


                // after delaytime, start cancel thumbnail
                Handler handler = new Handler();
                handler.postDelayed(new Runnable() {
                    public void run() {
                        Log.i(TAG, "after delay time( " + mCancelTime + ") cancelThumbnails() start");
                        boolean brt = nexClip.cancelThumbnails();
                        Log.i(TAG, "after delay time( " + mCancelTime + ") cancelThumbnails() end = " + brt);
                        Log.d(TAG, "endTime: " + mEndTime);

                        // start get thumbnail for aftercount
                        for (int i = 0; i < mCancelAfterCount; i++) {
                            final int idx = i;
                            mEndTime += mInterval;
                            if( mYonly.isChecked() ) {
                                mClip.getVideoClipIDR2YOnlyThumbnails(160,90,0,0,50,new nexClip.OnGetVideoClipIDR2YOnlyThumbnailsListener() {
                                    @Override
                                    public void onGetVideoClipIDR2YOnlyThumbnailsResult(int event, byte[] data, int index, int totalCount, int timestamp) {
                                        if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {
                                            mResult.append("After Y only kEvent_Ok" + idx + "\n");
                                            updateTextViewResult(mResult.toString());

                                            Log.d(TAG, "After Y only kEvent_Ok" + idx);
                                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Completed) {
                                            mResult.append("After Y only kEvent_Completed" + idx + "\n");
                                            updateTextViewResult(mResult.toString());

                                            Log.d(TAG, "After Y only kEvent_Completed" + idx);
                                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Fail) {
                                            mResult.append("After Y only kEvent_Fail" + idx + "\n");
                                            updateTextViewResult(mResult.toString());

                                            Log.d(TAG, "After Y only kEvent_Fail" + idx);
                                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_systemError) {
                                            mResult.append("After Y only kEvent_systemError" + idx + "\n");
                                            updateTextViewResult(mResult.toString());

                                            Log.d(TAG, "After Y only kEvent_systemError" + idx);
                                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_UserCancel) {
                                            mResult.append("After Y only kEvent_UserCancel" + idx + "\n");
                                            updateTextViewResult(mResult.toString());
                                            Log.d(TAG, "After Y only kEvent_UserCancel" + idx);
                                        } else {
                                            mResult.append("After Y only default event=" + event + "\n");
                                            updateTextViewResult(mResult.toString());

                                            Log.d(TAG, "After default event=" + event);
                                        }
                                    }
                                });
                            }else {
                                mClip.getVideoClipDetailThumbnails(160, 90, 0, 0, 50, 0, noCache, null, new nexClip.OnGetVideoClipDetailThumbnailsListener() {
                                    @Override
                                    public void onGetDetailThumbnailResult(int event, Bitmap bm, int index, int totalCount, int timestamp) {
                                        if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {
                                            mResult.append("After kEvent_Ok" + idx + "\n");
                                            updateTextViewResult(mResult.toString());

                                            Log.d(TAG, "After kEvent_Ok" + idx);
                                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Completed) {
                                            mResult.append("After kEvent_Completed" + idx + "\n");
                                            updateTextViewResult(mResult.toString());

                                            Log.d(TAG, "After kEvent_Completed" + idx);
                                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Fail) {
                                            mResult.append("After kEvent_Fail" + idx + "\n");
                                            updateTextViewResult(mResult.toString());

                                            Log.d(TAG, "After kEvent_Fail" + idx);
                                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_systemError) {
                                            mResult.append("After kEvent_systemError" + idx + "\n");
                                            updateTextViewResult(mResult.toString());

                                            Log.d(TAG, "After kEvent_systemError" + idx);
                                        } else if (event ==nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_UserCancel) {
                                            mResult.append("After kEvent_UserCancel" + idx + "\n");
                                            updateTextViewResult(mResult.toString());
                                            Log.d(TAG, "After kEvent_UserCancel" + idx);
                                        } else {
                                            mResult.append("After default event=" + event + "\n");
                                            updateTextViewResult(mResult.toString());

                                            Log.d(TAG, "After default event=" + event);
                                        }
                                    }
                                });
                            }
                        }
                    }
                }, mCancelTime);
            }
        });

        mButtonTestStart.setEnabled(false);

        mSeekBarCancelBefore = (SeekBar) findViewById(R.id.seekbar_thumnail_cancel_before);
        mSeekBarCancelBefore.setMax(4);
        mSeekBarCancelBefore.setProgress(0);
        mSeekBarCancelBefore.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                //Log.d(TAG, "mSeekBarCancelBefore $ onProgressChanged $ value: " + value);
                ((TextView) findViewById(R.id.textview_thumbnail_cancel_before_value)).setText("" + value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                //Log.d(TAG, "mSeekBarCancelBefore $ onStopTrackingTouch $ value: " + value);
                ((TextView) findViewById(R.id.textview_thumbnail_cancel_before_value)).setText("" + value);
                mCancelBeforeCount = value;
            }
        });

        mSeekBarCancelAfter = (SeekBar) findViewById(R.id.seekbar_thumnail_cancel_after);
        mSeekBarCancelAfter.setMax(4);
        mSeekBarCancelAfter.setProgress(0);
        mSeekBarCancelAfter.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                //Log.d(TAG, "mSeekBarCancelAfter $ onProgressChanged $ value: " + value);
                ((TextView) findViewById(R.id.textview_thumbnail_cancel_after_value)).setText("" + value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                //Log.d(TAG, "mSeekBarCancelAfter $ onStopTrackingTouch $ value: " + value);
                ((TextView) findViewById(R.id.textview_thumbnail_cancel_after_value)).setText("" + value);
                mCancelAfterCount = value;
            }
        });

        mSeekBarCancelTime = (SeekBar) findViewById(R.id.seekbar_thumnail_cancel_time);
        mSeekBarCancelTime.setMax(2000);
        mSeekBarCancelTime.setProgress(0);
        mSeekBarCancelTime.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            int value = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                value = progress;
                //Log.d(TAG, "mSeekBarCancelTime $ onProgressChanged $ value: " + value);
                ((TextView) findViewById(R.id.textview_thumbnail_cancel_time_value)).setText("" + value);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                //Log.d(TAG, "mSeekBarCancelTime $ onStopTrackingTouch $ value: " + value);
                ((TextView) findViewById(R.id.textview_thumbnail_cancel_time_value)).setText("" + value);
                mCancelTime = value; // ms
            }
        });

        mTextViewResult = (TextView)findViewById(R.id.textview_thumbnail_cancel_result);

    }

    void updateTextViewResult(final String str)
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTextViewResult.setText(str);
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode == 100 && resultCode == Activity.RESULT_OK) {
            Log.d(TAG, "onActivityResult $ START");
            ClipData clipData = data.getClipData();
            String filepath = null;
            mListFilePath.clear();
            if(clipData == null) {
                filepath = ConvertFilepathUtil.getPath(this, data.getData());
                Log.d(TAG, "onActivityResult $ (clipData == null) filepath: " + filepath);
                mListFilePath.add(filepath);
            } else {

                ClipData.Item item;
                Uri uri = null;
                for(int i=0; i<clipData.getItemCount(); i++) {
                    item = clipData.getItemAt(i);
                    uri = item.getUri();
                    filepath = ConvertFilepathUtil.getPath(this, uri);
                    Log.d(TAG, "onActivityResult $ filepath: " + filepath);
                    mListFilePath.add(filepath);
                }
            }
            mButtonTestStart.setEnabled(true);
        }
    }
}
