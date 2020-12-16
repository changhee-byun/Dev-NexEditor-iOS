/******************************************************************************
 * File Name        : FontListAdapter.java
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

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexFont;
import com.nexstreaming.nexeditorsdkapis.R;

import java.util.List;

/**
 * Created by jeongwook.yoon on 2016-11-13.
 */

public class FontListAdapter extends BaseAdapter {
    private  Context mContext;
    private static List<nexFont> fonts;

    FontListAdapter(Context context){
        Init(context);
    }

    public void Init(Context context ){
        mContext = context;
        fonts = nexFont.getPresetList();
    }

    @Override
    public int getCount() {
        if( fonts == null )
            return 0;
        return fonts.size();
    }

    @Override
    public Object getItem(int position) {
        return fonts.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        FontListAdapter.ViewHolder holder;

        LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        convertView = inflater.inflate(R.layout.color_effect_list_item, parent, false);
        holder = new FontListAdapter.ViewHolder();
        holder.iv = (ImageView)convertView.findViewById(R.id.imageView_color_effect_apply);
        holder.name = (TextView)convertView.findViewById(R.id.textView_color_effect_name);
        convertView.setTag(holder);



        Bitmap bm = fonts.get(position).getSampleImage(mContext);


        holder.iv.setImageBitmap(bm);
        holder.name.setText(fonts.get(position).getId());
        holder.name.setTextColor(Color.WHITE);

        return convertView;
    }

    class ViewHolder
    {
        ImageView iv;
        TextView name;
    }
}
