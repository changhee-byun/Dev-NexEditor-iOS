/******************************************************************************
 * File Name        : AudioEnvelopAdapter.java
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

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdkapis.R;

import java.util.ArrayList;

/**
 * Created by jeongwook.yoon on 2015-04-22.
 */
public class AudioEnvelopAdapter extends BaseAdapter {
    private Context mContext;
    private ArrayList<String> mTime;
    private ArrayList<String> mVol;
    private TextView time;
    private TextView volume;
    public AudioEnvelopAdapter(Context c, ArrayList<String> time, ArrayList<String> vol) {
        mContext = c;
        mTime = time;
        mVol = vol;
    }

    public AudioEnvelopAdapter(Context c) {
        mContext = c;
        mTime = new ArrayList<String>();
        mVol = new ArrayList<String>();
    }

    public int getCount() {
        return mTime.size();
    }

    public Object getItem(int position) {
        return null;
    }

    public long getItemId(int position) {
        return 0;
    }

    // create a new ImageView for each item referenced by the Adapter
    public View getView(int position, View convertView, ViewGroup parent) {
        EnvelopViewHolder viewholder;

        if (convertView == null) { // if it not recycled, initialize some attributes
            LayoutInflater li = (LayoutInflater) mContext.getSystemService( Context.LAYOUT_INFLATER_SERVICE );            
            convertView = li.inflate(R.layout.audio_envelop_adapter, null);
            time = (TextView) convertView.findViewById(R.id.edittext_audio_envelop_env_time);
            volume = (TextView) convertView.findViewById(R.id.edittext_audio_envelop_env_vol);
            viewholder = new EnvelopViewHolder();
            viewholder.time = time;
            viewholder.volume = volume;
            convertView.setTag(viewholder);
        }
        else{
        	viewholder = (EnvelopViewHolder) convertView.getTag();
            time    = viewholder.time;
            volume  = viewholder.volume;
        }
        if(mTime.size() !=0 && mVol.size()!=0) {
            time.setText(mTime.get(position));
            volume.setText(mVol.get(position));
        }

        return convertView;
    }
    public class EnvelopViewHolder
    {
      public TextView time;
      public TextView volume;
    }
}
