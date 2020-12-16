/******************************************************************************
 * File Name        : EffectListActivity.java
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
import android.os.Bundle;
import android.view.WindowManager;
import android.widget.ListView;

import com.nexstreaming.nexeditorsdkapis.R;


public class EffectListActivity extends Activity {
    private static final String TAG = "EffectListAcitivty";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_effect_list);
        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        final ListView ThemeListView = (ListView) findViewById(R.id.listView_effect_list);
        ThemeListView.setAdapter(new EffectListAdapter(getApplicationContext()));
    }
}
