/******************************************************************************
 * File Name        : OverlaySlideShowActivity.java
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

package com.nexstreaming.nexeditorsdkapis.overlay;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexAnimate;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexOverlayImage;
import com.nexstreaming.nexeditorsdk.nexOverlayItem;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;

import java.util.ArrayList;


public class OverlaySlideShowActivity extends Activity {
    private static final String LOG_TAG = "OverlaySlideShow";
    private nexEngineListener mEditorListener = null;
    private ArrayList<String> m_listfilepath;
    private nexEngine mEngine;
    private nexEngineView m_editorView;
    private nexProject mProject;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if(!nexApplicationConfig.isSupportedApi(nexApplicationConfig.APILevel.OverlayImageLimited)) {
            Toast.makeText(getApplicationContext(),"API Level is not supported!",Toast.LENGTH_SHORT).show();
            finish();
            return;
        }


                setContentView(R.layout.activity_overlay_slide_show);

                // set screen timeout to never
                getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

                mProject = new nexProject();
                final int editorViewWidth = 1280;
                final int editorViewHeight = 720;

                final Intent intent = getIntent();
                m_listfilepath = intent.getStringArrayListExtra("filelist");

                for( String file : m_listfilepath )
                {
                    Log.d(LOG_TAG, "Selected file = " + file);
                }

                if (m_listfilepath.size() < 6) {
                    Toast.makeText(getApplicationContext(), "select 3 pictures", Toast.LENGTH_LONG).show();

                    int pictureCount = m_listfilepath.size();
                    while (pictureCount < 6) {
                        m_listfilepath.add(m_listfilepath.get(0));
                        pictureCount++;
                    }
                }


                //mProject.add(new nexClip(m_listfilepath.get(0)));
                mProject.add(nexClip.getSolidClip(0xff00ffff));


                mProject.getClip(0, true).setImageClipDuration(15000);

                mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
                mEngine.setEventHandler(mEditorListener); // null
                m_editorView = (nexEngineView) findViewById(R.id.engineview_overlay_slide_show);
                mEngine.setView(m_editorView);
                mEngine.setProject(mProject);


                // overlayItem1 create and add to project
                final int olWidth = editorViewWidth / 2;
                final int olHeight = editorViewHeight / 2;
                int olX = olWidth / 2;
                int olY = -(olHeight / 2);

                nexOverlayItem overlayItem1 = new nexOverlayItem(makeKmOverlayImage("image1", m_listfilepath.get(0), olWidth, olHeight), olX, olY, 0, 6000);
                overlayItem1.addAnimate(nexAnimate.getMove(0, 3000, new nexAnimate.MoveTrackingPath() {
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if (coordinate == nexAnimate.kCoordinateX) {
                            return 0;
                        } else if (coordinate == nexAnimate.kCoordinateY) {
                            return olHeight * timeRatio;
                        }
                        return 0;
                    }
                }));
                mProject.addOverlay(overlayItem1);

                // overlayItem2 create and add to project
                final int ol2Width = editorViewWidth / 2;
                final int ol2Height = editorViewHeight / 2;
                int ol2X = -ol2Width / 2;
                int ol2Y = ol2Height + ol2Height / 2;
                nexOverlayItem overlayItem2 = new nexOverlayItem(makeKmOverlayImage("image2", m_listfilepath.get(1), ol2Width, ol2Height), ol2X, ol2Y, 0, 6000);
                overlayItem2.addAnimate(nexAnimate.getMove(0, 3000, new nexAnimate.MoveTrackingPath() {
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if (coordinate == nexAnimate.kCoordinateX) {
                            return ol2Width * timeRatio;
                        } else if (coordinate ==nexAnimate.kCoordinateY) {
                            return 0;
                        }
                        return 0;
                    }
                }));
                mProject.addOverlay(overlayItem2);


                // overlayItem3 create and add to project
                final int ol3Width = editorViewWidth / 2;
                final int ol3Height = editorViewHeight;
                int ol3X = editorViewWidth /*+ ol3Width / 2*/;
                int ol3Y = ol3Height / 2;

                nexOverlayItem overlayItem3 = new nexOverlayItem(makeKmOverlayImage("image3", m_listfilepath.get(2), ol3Width, ol3Height), ol3X, ol3Y, 0, 6000);
                overlayItem3.addAnimate(nexAnimate.getMove(0, 3000, new nexAnimate.MoveTrackingPath() {
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if (coordinate == nexAnimate.kCoordinateX) {
                            return -ol3Width / 2 * timeRatio;
                        } else if (coordinate == nexAnimate.kCoordinateY) {
                            return 0;
                        }
                        return 0;
                    }
                }));
                overlayItem3.setScale(0, 1);
                overlayItem3.addAnimate(nexAnimate.getScale(0, 3000, 1, 1));
                mProject.addOverlay(overlayItem3);

                // overlayItem4 create and add to project
                final int ol4Width = editorViewWidth / 3;
                final int ol4Height = editorViewHeight;
                int ol4X = editorViewWidth + ol4Width / 2;
                int ol4Y = ol4Height / 2;
                nexOverlayItem overlayItem4 = new nexOverlayItem(makeKmOverlayImage("image4", m_listfilepath.get(3), ol4Width, ol4Height), ol4X, ol4Y, 6000, 15000);
                overlayItem4.addAnimate(nexAnimate.getMove(0, 2000, new nexAnimate.MoveTrackingPath() {
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if (coordinate == nexAnimate.kCoordinateX) {
                            return -editorViewWidth * timeRatio;
                        } else if (coordinate == nexAnimate.kCoordinateY) {
                            return 0;
                        }
                        return 0;
                    }
                }));
                mProject.addOverlay(overlayItem4);


                // overlayItem5 create and add to project
                final int ol5Width = editorViewWidth / 3;
                final int ol5Height = editorViewHeight;
                int ol5X = editorViewWidth + ol5Width / 2;
                int ol5Y = ol5Height / 2;
                nexOverlayItem overlayItem5 = new nexOverlayItem(makeKmOverlayImage("image5", m_listfilepath.get(4), ol5Width, ol5Height), ol5X, ol5Y, 6000, 15000);
                overlayItem5.addAnimate(nexAnimate.getMove(2000, 2000, new nexAnimate.MoveTrackingPath() {
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if (coordinate == nexAnimate.kCoordinateX) {
                            return (-ol5Width * 2) * timeRatio;
                        } else if (coordinate ==nexAnimate.kCoordinateY) {
                            return 0;
                        }
                        return 0;
                    }
                }));
                mProject.addOverlay(overlayItem5);


                // overlayItem6 create and add to project
                final int ol6Width = editorViewWidth / 3;
                final int ol6Height = editorViewHeight;
                int ol6X = editorViewWidth + ol6Width / 2;
                int ol6Y = ol6Height / 2;
                nexOverlayItem overlayItem6 = new nexOverlayItem(makeKmOverlayImage("image6", m_listfilepath.get(5), ol6Width, ol6Height), ol6X, ol6Y, 6000, 15000);
                overlayItem6.addAnimate(nexAnimate.getMove(4000, 2000, new nexAnimate.MoveTrackingPath() {
                    @Override
                    public float getTranslatePosition(int coordinate, float timeRatio) {
                        if (coordinate == nexAnimate.kCoordinateX) {
                            return -ol5Width * timeRatio;
                        } else if (coordinate == nexAnimate.kCoordinateY) {
                            return 0;
                        }
                        return 0;
                    }
                }));
                mProject.addOverlay(overlayItem6);

                mEngine.updateProject();

                final Button btnPlay = (Button) findViewById(R.id.button_slideshow_playstop);
                btnPlay.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        mEngine.play();
                    }
                });
    }

    nexOverlayImage makeKmOverlayImage(String imageId, String bitmapPath, int width, int height)
    {
        nexOverlayImage image = new nexOverlayImage(imageId, bitmapPath);
        image.setCrop(width, height);
        return image;
    }


    @Override
    protected void onStart() {
        super.onStart();
        Log.d(LOG_TAG, "onStart");
        ApiDemosConfig.getApplicationInstance().getEngine();
    }
    
    @Override
    protected void onStop() {
        mEngine.stop();
        mEngine.clearTrackCache();
        super.onStop();
    }

    @Override
    protected void onDestroy() {

        Log.d(LOG_TAG, "onDestroy");

        ApiDemosConfig.getApplicationInstance().releaseEngine();
        super.onDestroy();
    }
}
