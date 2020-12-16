/******************************************************************************
 * File Name        : ThumbnailSeektabActivity.java
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
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.GridView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.edit.ClipThumbnailAdapter;

import java.util.ArrayList;
import java.util.Collections;


public class ThumbnailSeektabActivity extends Activity {
    public static final String TAG = "ThumbnailSeekTab";

    private ArrayList<String> mListFilePath;
    private int mWidth = 160;
    private int mHeight = 90;
    private int mAngle = 0;

    private int mSeekTab[];
    private ArrayList<Integer> mSelectedSeekTab;

    private Button mButtonGet;
    private Button mButtonClear;
    private TextView mTextViewSelectedSeekTab;

    private ListView mListViewSeekTab;
    private ArrayAdapter<String> mListAdapterSeekTab;

    private GridView mGridView;
    private ClipThumbnailAdapter mClipThumbnailAdapter;

    private ArrayList<Bitmap> mBmList = null;
    private ArrayList<String> mTextList = null;

    private nexClip mClip;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_thumbnail_seektab);

        // set screen timeout to never
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mListFilePath = getIntent().getStringArrayListExtra("filelist");
        mSelectedSeekTab = new ArrayList<Integer>();

        mClip = new nexClip(mListFilePath.get(0));
        mSeekTab = mClip.getSeekPointsSync();

        // UI initialize
        mGridView = (GridView) findViewById(R.id.gridview_thumbnail_seektab);
        mTextViewSelectedSeekTab = (TextView) findViewById(R.id.textview_thumbnail_seektab_selected_seektab);

        mClipThumbnailAdapter = new ClipThumbnailAdapter(ThumbnailSeektabActivity.this);
        mClipThumbnailAdapter.setParams(mTextList, mBmList);
        mGridView.setAdapter(mClipThumbnailAdapter);

        mListViewSeekTab = (ListView) findViewById(R.id.listview_thumbnail_seektab);
        mListAdapterSeekTab = new ArrayAdapter<String>(getApplicationContext(), R.layout.simple_list_item_1);
        mListViewSeekTab.setAdapter(mListAdapterSeekTab);
        for(int i = 0; i < mSeekTab.length; i++) {
            mListAdapterSeekTab.add(mSeekTab[i] + "");
        }

        mListViewSeekTab.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Integer itemValue = new Integer(Integer.parseInt(mListAdapterSeekTab.getItem(position)));
                if(mSelectedSeekTab.contains(itemValue)) {
                    Toast.makeText(getApplicationContext(), "postion(" + itemValue + ") is alreay selected", Toast.LENGTH_SHORT).show();
                    return ;
                }

                mSelectedSeekTab.add(itemValue);

                Collections.sort(mSelectedSeekTab);

                String selectedSeekTab = "selected: ";
                for(int i = 0; i < mSelectedSeekTab.size(); i++) {
                    selectedSeekTab += " " + mSelectedSeekTab.get(i);
                }

                mTextViewSelectedSeekTab.setText(selectedSeekTab);
            }
        });

        mButtonClear = (Button) findViewById(R.id.button_thumbnail_seektab_clear);
        mButtonClear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedSeekTab.clear();
                mTextViewSelectedSeekTab.setText("selected: ");

                if( mBmList != null )
                    mBmList.clear();

                if( mTextList != null )
                    mTextList.clear();

                if( mClipThumbnailAdapter != null ) {
                    mClipThumbnailAdapter.setParams(mTextList, mBmList);
                    mClipThumbnailAdapter.notifyDataSetChanged();
                }
            }
        });

        mButtonGet = (Button) findViewById(R.id.button_thumbnail_seektab_get);
        mButtonGet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(mSelectedSeekTab.size() == 0) {
                    Toast.makeText(getApplicationContext(), "no Selected. select now !!", Toast.LENGTH_SHORT).show();
                    return ;
                }

                if( mBmList == null )
                    mBmList = new ArrayList<Bitmap>();

                if( mTextList == null )
                    mTextList = new ArrayList<String>();

                mBmList.clear();
                mTextList.clear();

                if(mClipThumbnailAdapter != null) {
                    mClipThumbnailAdapter.setParams(mTextList, mBmList);
                    mClipThumbnailAdapter.notifyDataSetChanged();
                }

                int selectedSeektab[] = new int[mSelectedSeekTab.size()];
                for(int i = 0; i < selectedSeektab.length; i++) {
                    selectedSeektab[i] = mSelectedSeekTab.get(i).intValue();
                    Log.d(TAG, "seek" + i + ": " + selectedSeektab[i]);
                }

                // getThumbnail
                mClip.getVideoClipDetailThumbnails(160, 90, 0, 0, selectedSeektab.length, 0, false, selectedSeektab, new nexClip.OnGetVideoClipDetailThumbnailsListener() {
                    @Override
                    public void onGetDetailThumbnailResult(int event, Bitmap bm, int index, int totalCount, int timestamp) {
                        if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {
                            mBmList.add(Bitmap.createBitmap(bm));
                            mTextList.add(String.format("%d", timestamp + 1));
                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Completed) {
                            // mClipThumbnailAdapter = new ClipThumbnailAdapter(ThumbnailSeektabActivity.this);
                            mClipThumbnailAdapter.setParams(mTextList, mBmList);
                            mClipThumbnailAdapter.notifyDataSetChanged();
                            // mGridView.setAdapter(mClipThumbnailAdapter);
                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Fail) {
                            Toast.makeText(getApplicationContext(), "getVideoClipDetailThumbnails fail!", Toast.LENGTH_SHORT).show();
                        } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_systemError) {
                            Toast.makeText(getApplicationContext(), "system error", Toast.LENGTH_LONG).show();
                        }
                    }
                });
            }
        });

    }
}
