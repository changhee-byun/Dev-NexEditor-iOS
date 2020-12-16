/******************************************************************************
 * File Name        : ColorEffectListActivity.java
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

package com.nexstreaming.nexeditorsdkapis.effect;
import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Bundle;
import android.view.WindowManager;
import android.widget.ListView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.Constants;

public class ColorEffectListActivity extends Activity {
    private int mEffectType;
    private ListView mEffectListView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_color_effect_list);

        Intent intent = getIntent();
        mEffectType = intent.getIntExtra(Constants.EFFECT_TYPE, Constants.EFFECT_TYPE_COLOR);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mEffectListView = (ListView) findViewById(R.id.listView_color_effect_list);
        if( mEffectType == Constants.EFFECT_TYPE_COLOR ) {
            // set screen timeout to never

            Bitmap bm = BitmapFactory.decodeResource(getResources(), R.raw.cesample);
            if (bm != null) {
                mEffectListView.setAdapter(new ColorEffectListAdapter(getApplicationContext(), bm));
            } else {
                Toast.makeText(getApplicationContext(), "bitmap is null", Toast.LENGTH_SHORT).show();
            }
        }else if( mEffectType == Constants.EFFECT_TYPE_STICKER ){
            mEffectListView.setAdapter(new StickerListAdapter(getApplicationContext()));
        }else if( mEffectType == Constants.EFFECT_TYPE_FONT ){
            mEffectListView.setBackgroundColor(Color.BLACK);
            mEffectListView.setAdapter(new FontListAdapter(getApplicationContext()));
        }
    }
}
