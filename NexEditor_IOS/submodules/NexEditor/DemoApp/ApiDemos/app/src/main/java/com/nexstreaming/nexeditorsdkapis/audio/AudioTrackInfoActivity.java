/******************************************************************************
 * File Name        : AudioTrackInfoActivity.java
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

package com.nexstreaming.nexeditorsdkapis.audio;


import android.app.Activity;
import android.content.Intent;
import android.media.MediaMetadataRetriever;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.UtilityCode;

/**
 * Created by alex.kang on 2015-09-30.
 */
public class AudioTrackInfoActivity extends Activity {
    public static final String TAG = "AudoTrackInfo";
    private EditText editText_startTime;
    private EditText editText_endTime;
    private EditText editText_startTrim;
    private EditText editText_endTrim;
    private TextView trimrange;
    private String duration;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_track_info);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        Intent intent = getIntent();
        String filepath = intent.getStringExtra("filepath");
        editText_startTime = (EditText)findViewById(R.id.edittext_audio_track_info_starttime);
        editText_endTime = (EditText)findViewById(R.id.edittext_audio_track_info_endtime);
        editText_startTrim = (EditText)findViewById(R.id.edittext_audio_track_starttrim);
        editText_endTrim = (EditText)findViewById(R.id.editText_audio_track_info_endtrim);
        trimrange = (TextView)findViewById(R.id.textview_audio_track_info_trimrange);

        MediaMetadataRetriever metadataRetriever = new MediaMetadataRetriever();
        metadataRetriever.setDataSource(filepath);

        nexClip clip = new nexClip(filepath);
        duration = clip.getTotalTime() + "";
//        duration = metadataRetriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);

        trimrange.setText("~"+duration);

        Button btSetInfo = (Button)findViewById(R.id.button_audio_track_info_set);
        btSetInfo.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(editText_startTime.getText().toString().length() == 0 ||
                        editText_endTime.getText().toString().length() == 0 ||
                        editText_startTrim.getText().toString().length() == 0 ||
                        editText_endTrim.getText().toString().length() == 0) {
                    Toast.makeText(getApplicationContext(), "input edit box !!!", Toast.LENGTH_SHORT).show();
                    return ;
                }

                if(UtilityCode.isNumeric(editText_startTime.getText().toString()) == false ||
                        UtilityCode.isNumeric(editText_endTime.getText().toString()) == false ||
                        UtilityCode.isNumeric(editText_startTrim.getText().toString()) == false ||
                        UtilityCode.isNumeric(editText_endTrim.getText().toString()) == false)  {
                    Toast.makeText(getApplicationContext(), "your input is not nubmer. so input !!!", Toast.LENGTH_SHORT).show();
                    return ;
                }

                if(Integer.parseInt(editText_startTime.getText().toString()) >=  Integer.parseInt(editText_endTime.getText().toString()) ||
                        Integer.parseInt(editText_startTime.getText().toString()) < 0) {
                    Toast.makeText(getApplicationContext(), "startTime >= endTime or startTime < 0", Toast.LENGTH_SHORT).show();
                    return ;
                }

                if(Integer.parseInt(editText_startTrim.getText().toString()) >=  Integer.parseInt(editText_endTrim.getText().toString()) ||
                        Integer.parseInt(editText_startTrim.getText().toString()) < 0) {
                    Toast.makeText(getApplicationContext(), "startTrim >= endEnd or startTrim < 0", Toast.LENGTH_SHORT).show();
                    return ;
                }

                if(Integer.parseInt(editText_endTime.getText().toString()) >  Integer.parseInt(duration)) {
                    Toast.makeText(getApplicationContext(), "endTime > duration", Toast.LENGTH_SHORT).show();
                    return ;
                }

                if(Integer.parseInt(editText_endTrim.getText().toString()) >  Integer.parseInt(duration)) {
                    Toast.makeText(getApplicationContext(), "endTrim > duration", Toast.LENGTH_SHORT).show();
                    return ;
                }

                Intent RetEffectintent = new Intent();
                if("".equals(editText_startTime.getText().toString())) {
                    RetEffectintent.putExtra("StartTime", 0);
                }else{
                    RetEffectintent.putExtra("StartTime", Integer.parseInt(editText_startTime.getText().toString()));
                }
                if("".equals(editText_endTime.getText().toString())) {
                    RetEffectintent.putExtra("EndTime", Integer.parseInt(duration));
                }else{
                    RetEffectintent.putExtra("EndTime", Integer.parseInt(editText_endTime.getText().toString()));
                }
                if("".equals(editText_startTrim.getText().toString())) {
                    RetEffectintent.putExtra("StartTrim", 0);
                }else{
                    RetEffectintent.putExtra("StartTrim", Integer.parseInt(editText_startTrim.getText().toString()));
                }
                if("".equals(editText_endTrim.getText().toString())) {
                    RetEffectintent.putExtra("EndTrim", Integer.parseInt(duration));
                }else{
                    RetEffectintent.putExtra("EndTrim", Integer.parseInt(editText_endTrim.getText().toString()));
                }
                setResult(Activity.RESULT_OK, RetEffectintent);
                finish();
            }
        });
    }
}
