/******************************************************************************
 * File Name        : ExceptionTestActivity.java
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
import android.os.Bundle;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;

import java.util.ArrayList;


public class ExceptionTestActivity extends Activity {
    private static final String TAG="Exception";
    private ArrayList<String> m_listfilepath;
    private StringBuilder mExceptionMessageBuilder;
    private TextView mTextViewExceptionMessage;
    private int mExceptionCount = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_exception_test);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mExceptionMessageBuilder = new StringBuilder();
        mTextViewExceptionMessage = (TextView) findViewById(R.id.textview_exception_test_exception_message);

        nexProject project1, project2;

        m_listfilepath = new ArrayList<String>();

        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");

        nexClip clip = nexClip.getSupportedClip(m_listfilepath.get(0));
        if( clip == null ){
            Toast.makeText(getApplicationContext(), "No SupportedClip.", Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        try {
            clip.getClipEffect();
        }catch (Exception e){
            appendExceptionMessage(e, e.getMessage());
        }

        try {
            clip.getTransitionEffect();
        }catch (Exception e){
            appendExceptionMessage(e, e.getMessage());
        }

        try {
            clip.getVideoClipEdit();
        }catch (Exception e){
            appendExceptionMessage(e, e.getMessage());
        }

        project1 = new nexProject();
        project1.add(clip);

        try {
            clip.getVideoClipEdit().setTrim(0,400);
        }catch (Exception e){
            appendExceptionMessage(e, e.getMessage());
        }

        try {
            clip.getVideoClipEdit().setTrim(0,0);
        }catch (Exception e){
            appendExceptionMessage(e, e.getMessage());
        }

        try {
            clip.getVideoClipEdit().setTrim(-1,clip.getTotalTime()+1);
        }catch (Exception e){
            appendExceptionMessage(e, e.getMessage());
        }

        try {
            clip.getVideoClipEdit().setTrim(0,900);
            clip.getVideoClipEdit().setSpeedControl(200);
            project1.updateProject();
        }catch (Exception e){
            appendExceptionMessage(e, e.getMessage());
        }

        try {
            clip.getClipEffect().setEffectShowTime(1000, 0);
        }catch (Exception e){
            appendExceptionMessage(e, e.getMessage());
        }

        try {
            clip.getClipEffect().setEffect("com.nexstreaming.unkownid");
        }catch (Exception e){
            appendExceptionMessage(e, e.getMessage());
        }

        try {
            clip.getTransitionEffect().setTransitionEffect("com.nexstreaming.unkownid");
        }catch (Exception e){
            appendExceptionMessage(e, e.getMessage());
        }

        project2 = new nexProject();
        try {
            project2.add(clip);
        }catch (Exception e){
            appendExceptionMessage(e, e.getMessage());
        }

        //project1.allClear(true);

        project1.remove(clip);

        project2.add(clip);


        mTextViewExceptionMessage.setText(mExceptionMessageBuilder.toString());
//        Log.e(TAG, "Test END");
    }


    /**---------------------------------------------------------------------------------
     *                               Private Function                                  *
     ---------------------------------------------------------------------------------**/
    private void appendExceptionMessage(Exception e, String message) {

        mExceptionCount++;
        mExceptionMessageBuilder.append("Exception " + mExceptionCount + ".");
        mExceptionMessageBuilder.append("\n");
        mExceptionMessageBuilder.append(e.getClass().toString());
        mExceptionMessageBuilder.append("\n");
        mExceptionMessageBuilder.append(message);
        mExceptionMessageBuilder.append("\n\n");
    }
}
