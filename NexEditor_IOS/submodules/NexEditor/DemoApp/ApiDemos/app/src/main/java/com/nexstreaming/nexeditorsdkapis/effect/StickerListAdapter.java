/******************************************************************************
 * File Name        : StickerListAdapter.java
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
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexOverlayPreset;
import com.nexstreaming.nexeditorsdkapis.R;

/**
 * Created by jeongwook.yoon on 2016-11-13.
 */

public class StickerListAdapter extends BaseAdapter {
    private Context mContext;
    private static LruCache<Integer,Bitmap> sLutCache = new LruCache<>(100);
    private static String[] sIds;

    StickerListAdapter(Context context){
        Init(context);
    }

    public void Init(Context context ){
        mContext = context;
        sIds = nexOverlayPreset.getOverlayPreset(mContext).getIDs();
    }

    public static void deInit(){
        sLutCache.evictAll();
    }

    @Override
    public int getCount() {
        if( sIds == null )
            return 0;
        return sIds.length;
    }

    @Override
    public Object getItem(int position) {
        return sIds[position];
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        StickerListAdapter.ViewHolder holder;

        LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        convertView = inflater.inflate(R.layout.color_effect_list_item, parent, false);
        holder = new StickerListAdapter.ViewHolder();
        holder.iv = (ImageView)convertView.findViewById(R.id.imageView_color_effect_apply);
        holder.name = (TextView)convertView.findViewById(R.id.textView_color_effect_name);
        convertView.setTag(holder);

        Bitmap bm = sLutCache.get(position);
        if( bm == null ) {
            bm = nexOverlayPreset.getOverlayPreset().getIcon(sIds[position],300,300);
            if( bm != null )
            {
                sLutCache.put(position, bm);
            }
        }

        holder.iv.setImageBitmap(bm);
        holder.name.setText(sIds[position]);
        holder.name.setTextColor(Color.WHITE);

        return convertView;
    }

    class ViewHolder
    {
        ImageView iv;
        TextView name;
    }
}
