/******************************************************************************
 * File Name        : EffectPreviewListActivity.java
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

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexTheme;
import com.nexstreaming.nexeditorsdk.nexTransitionEffect;
import com.nexstreaming.nexeditorsdkapis.common.Constants;
import com.nexstreaming.nexeditorsdkapis.R;

import java.util.ArrayList;


public class EffectPreviewListActivity extends ListActivity {
    final private String TAG = "EffectPreviewList";
    private ArrayList<String> mEffectPreviewList;
    private ArrayList<String> mListFilePath;
    private int mEffectType;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_effect_preview_list);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        Intent intent = getIntent();

        mEffectType = intent.getIntExtra(Constants.EFFECT_TYPE, 0);
        mEffectPreviewList = new ArrayList<String>();

        mListFilePath = new ArrayList<String>();
        mListFilePath = intent.getStringArrayListExtra("filelist");

        nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(getApplicationContext());
        if (mEffectType == Constants.EFFECT_TYPE_THEME) {
            nexTheme[] allThemes = fxlib.getThemes();
            for (nexTheme t : allThemes) {
                mEffectPreviewList.add(t.getId());
                Log.i(TAG, "id=" + t.getId() + ", name=" + t.getName(getBaseContext()));
            }
        } else if (mEffectType == Constants.EFFECT_TYPE_RANDOM_TRANSITION) {
            mEffectPreviewList.add("random");
            nexTransitionEffect[] allTransition = fxlib.getTransitionEffects();
            for (nexTransitionEffect te : allTransition) {
                mEffectPreviewList.add(te.getId());
                Log.i(TAG, "id=" + te.getId() + ", name=" + te.getName(getBaseContext()));
            }
        }

        setListAdapter(new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_list_item_1, mEffectPreviewList));
    }

    @Override
    public void onListItemClick(ListView parent,View v, int position, long ID){
        super.onListItemClick(parent, v, position, ID);
        if(mEffectType == Constants.EFFECT_TYPE_THEME) {
            Intent intent = new Intent(this, EffectPreviewListTestActivity.class);
            intent.putStringArrayListExtra("filelist", mListFilePath);
            intent.putExtra(Constants.THEME_ID, mEffectPreviewList.get(position));
            intent.putExtra(Constants.TRANSITION_ID, "none");
            startActivity(intent);
        } else if(mEffectType == Constants.EFFECT_TYPE_RANDOM_TRANSITION) {
            Intent intent = new Intent(this, EffectPreviewListTestActivity.class);
            intent.putStringArrayListExtra("filelist", mListFilePath);
            intent.putExtra(Constants.THEME_ID, "com.nexstreaming.kinemaster.basic");
            intent.putExtra(Constants.TRANSITION_ID, mEffectPreviewList.get(position));
            startActivity(intent);
        }
    }
}
