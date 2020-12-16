/******************************************************************************
 * File Name        : ClipInfoActivity.java
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

package com.nexstreaming.nexeditorsdkapis.edit;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdkapis.R;

import java.util.ArrayList;


public class ClipInfoActivity extends Activity {
    private static final String TAG = "ClipInfoActivity";
    private ArrayList<String> mListFilePath;
    private nexClip mClip;
    private GridView mGridview;
    private ImageView mImageview;
    private TextView mTextViewClitype;
    private TextView mTextViewResolution;
    private TextView mTextViewFps;
    private TextView mTextViewIsAudioAndVideo;
    private TextView mTextViewTotaltime;
    private TextView mTextViewSupportedresult;
    private int mVideoBitrate;
    private int mAudioBitrate;
    private ImageView mImageViewAudioPCM;

    private ArrayList<Bitmap> mBitmaplist = null;
    private ArrayList<String> mTextlist = null;

//    private ImageView mIV;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_clip_info);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mGridview = (GridView) findViewById(R.id.gridview_thumbnail_info);
        mImageview = (ImageView) findViewById(R.id.imageView_clip_info);
        mImageViewAudioPCM = (ImageView)findViewById(R.id.imageview_audio_pcm);
        mListFilePath = new ArrayList<String>();
        mBitmaplist = new ArrayList<Bitmap>();
        mTextlist = new ArrayList<String>();

        Intent intent = getIntent();
        mListFilePath = intent.getStringArrayListExtra("filelist");

        if(mListFilePath.size() == 0) {
            //error
        }else{
            mClip = new nexClip(mListFilePath.get(0));

            if( mClip.getClipType() == nexClip.kCLIP_TYPE_VIDEO ) {
                mClip.loadVideoClipThumbnails(new nexClip.OnLoadVideoClipThumbnailListener(){
                    @Override
                    public void onLoadThumbnailResult(int event) {
                        if( event == 0 ) {
                            int timeLine[] = mClip.getVideoClipTimeLineOfThumbnail();

                            mBitmaplist.add(mClip.getMainThumbnail(240f, getBaseContext().getResources().getDisplayMetrics().density));
                            mTextlist.add("Movie");
                            mImageview.setImageBitmap(mBitmaplist.get(0));
                            for( int i = 0 ; i < timeLine.length ; i++){
                                mBitmaplist.add(mClip.getVideoClipTimeLineThumbnail(0, timeLine[i], false, false));
                                mTextlist.add(String.format("%d(s)", timeLine[i] / 1000));
                            }
                            mGridview.setAdapter(new ClipThumbnailAdapter(ClipInfoActivity.this, mTextlist, mBitmaplist));
                        }
                    }
                });
            } else {
                mBitmaplist.add(mClip.getMainThumbnail(240f, getBaseContext().getResources().getDisplayMetrics().density));
                mTextlist.add("Photo");
                mImageview.setImageBitmap(mBitmaplist.get(0));
                mGridview.setAdapter(new ClipThumbnailAdapter(ClipInfoActivity.this, mTextlist, mBitmaplist));
                //mIV.setImageBitmap(bm);
            }

            mClip.getAudioPcmLevels(new nexClip.OnGetAudioPcmLevelsResultListener(){

                @Override
                public void onGetAudioPcmLevelsResult(byte[] pcmlevles) {
                    makePCMBitmap(pcmlevles,1);
                }
            });

            //clip type text
            mTextViewClitype = (TextView)findViewById(R.id.textview_clip_info_type);
            if(mClip.getClipType() == nexClip.kCLIP_TYPE_VIDEO) {
                mTextViewClitype.setText("Clip Type : Video");
            } else if(mClip.getClipType() == nexClip.kCLIP_TYPE_AUDIO) {
                mTextViewClitype.setText("Clip Type : Audio");
            } else if(mClip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                mTextViewClitype.setText("Clip Type : Image");
            } else {
                mTextViewClitype.setText("Clip Type : None");
            }

            //clip resolution text
            mTextViewResolution = (TextView)findViewById(R.id.textview_clip_info_resolution);
            mTextViewResolution.setText("Width=" + mClip.getWidth() + ", Height=" + mClip.getHeight());

            //clip exist audio/video
            mTextViewIsAudioAndVideo = (TextView)findViewById(R.id.textview_clip_info_is_audio_video);
            if(mClip.hasAudio() && mClip.hasVideo())
                mTextViewIsAudioAndVideo.setText("Clip exist Audio and Video");
            else if(mClip.hasAudio() && !mClip.hasVideo())
                mTextViewIsAudioAndVideo.setText("Clip exist Audio only");
            else if(mClip.hasVideo() && !mClip.hasAudio())
                mTextViewIsAudioAndVideo.setText("Clip exist Video only");
            else
                mTextViewIsAudioAndVideo.setText("Clip does not exist Video and Audio");

            mTextViewFps = (TextView)findViewById(R.id.textview_clip_info_fps);
            mTextViewFps.setText("Clip FPS : " + mClip.getFramesPerSecond());

            //clip total time
            mTextViewTotaltime = (TextView)findViewById(R.id.textview_clip_info_total_time);
            mTextViewTotaltime.setText("Total time : " + mClip.getTotalTime());

            mVideoBitrate = mClip.getVideoBitrate();
            mAudioBitrate = mClip.getAudioBitrate();

            Log.d("ClipInfo", "Video bitrate ="+mVideoBitrate + "Audio bitrate ="+mAudioBitrate);

            //clip supported result
            mTextViewSupportedresult = (TextView)findViewById(R.id.textview_clip_info_supported_result);
            switch (mClip.getSupportedResult())
            {
                case 0 : //mClip.kCLIP_Supported :
                    mTextViewSupportedresult.setText("Clip is supported");
                    break;
                case 1 : //mClip.kClip_Supported_Unknown :
                    mTextViewSupportedresult.setText("Clip is unknown");
                    break;
                case 2 : //mClip.kClip_NotSupported_AudioCodec :
                    mTextViewSupportedresult.setText("Clip is not supported audio codec");
                    break;
                case 3 : //mClip.kClip_NotSupported_AudioProfile :
                    mTextViewSupportedresult.setText("Clip is not supported audio profile");
                    break;
                case 4 : //mClip.kClip_NotSupported_Container :
                    mTextViewSupportedresult.setText("Clip is not supported container");
                    break;
                case 5 : //mClip.kClip_NotSupported_ResolutionTooHigh :
                    mTextViewSupportedresult.setText("Clip is not supported resolution(high)");
                    break;
                case 6 : //mClip.kClip_NotSupported_DurationTooShort :
                    mTextViewSupportedresult.setText("Clip is not supported duration");
                    break;
                case 7 : //mClip.kClip_NotSupported_ResolutionTooLow :
                    mTextViewSupportedresult.setText("Clip is not supported resolution(low)");
                    break;
                case 8 : //mClip.kClip_NotSupported_VideoProfile :
                    mTextViewSupportedresult.setText("Clip is not supported video profile");
                    break;
                case 9 : //mClip.kClip_NotSupported_VideoCodec :
                    mTextViewSupportedresult.setText("Clip is not supported video codec");
                    break;
                case 10 : //mClip.kClip_NotSupported_VideoFPS :
                    mTextViewSupportedresult.setText("Clip is not supported video FPS");
                    break;
                case 11 : //mClip.kClip_NotSupported_VideoLevel :
                    mTextViewSupportedresult.setText("Clip is not supported video level");
                    break;
                case 12 : //mClip.kClip_NotSupported :
                    mTextViewSupportedresult.setText("Clip is not supported");
                    break;
                case 13 : //mClip.kClip_Supported_NeedResolutionTranscoding :
                    mTextViewSupportedresult.setText("Clip is need resolution transcoding");
                    break;
                case 14 : //mClip.kClip_Supported_NeedFPSTranscoding :
                    mTextViewSupportedresult.setText("Clip is need FPS transcoding");
                    break;
                default:
                    mTextViewSupportedresult.setText("Clip is not supported");
                    break;
            }
        }

    }

    private void makePCMBitmap(byte[] pcmData, int drawMode) {
        if (pcmData == null) {
            return ;
        }

        Paint p = new Paint();

        int originalDuration = mClip.getTotalTime();
        int width = Math.min(originalDuration / 15,8192);
        int height = 128;
        int level = -1;
        int prevLevel = -1;

        if(width <= 0 || height <= 0)
            return ;

        Log.d(TAG,"Make PCM thumbnail: " + width + "x" + height);

        Bitmap bm = Bitmap.createBitmap(width, height, Bitmap.Config.ALPHA_8);
        Canvas canvas = new Canvas(bm);
        p.setAntiAlias(true);

        Path path = new Path();
        if( drawMode == 0 ) {
            int prevIndex = -1;
            path.moveTo(-50, height + 1);

            for (int x = 0; x < width; x += 3) {
                int index = (int) ((long) x * (long) pcmData.length / (long) width);

                if (pcmData.length <= 0)
                    return;

                if (index != prevIndex) {
                    level = 0;
                    for (int i = prevIndex + 1; i <= index; i++) {
                        level += ((int) pcmData[index]) & 0xFF;
                    }
                    level /= Math.max(1, (index - prevIndex));
                    prevIndex = index;
                }

                if (level != prevLevel) {
                    path.lineTo(x, height - (level * height / 255));
                    prevLevel = level;
                }
            }

            path.lineTo(width - 1, height - (level * height / 255));
            path.lineTo(width + 50, height + 1);
            path.close();
            canvas.drawPath(path, p);
        }else {

            float[] points = new float[pcmData.length*4];
            for (int i = 0; i < pcmData.length - 1; i++) {
                points[i * 4] = width * i / (pcmData.length - 1);
                points[i * 4 + 1] = height / 2
                        + ((byte) (pcmData[i] + 128)) * (height / 2) / 128;
                points[i * 4 + 2] = width * (i + 1) / (pcmData.length - 1);
                points[i * 4 + 3] = height / 2
                        + ((byte) (pcmData[i + 1] + 128)) * (height / 2) / 128;
            }
            canvas.drawLines(points, p);
        }
        mImageViewAudioPCM.setImageBitmap(bm);
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");

        if(mBitmaplist != null) {
            mBitmaplist.clear();
            mTextlist.clear();
        }
        super.onDestroy();
    }
}
