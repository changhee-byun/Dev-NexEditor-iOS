/******************************************************************************
 * File Name        : ColorEffectListAdapter.java
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
import android.graphics.ColorMatrixColorFilter;
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexColorEffect;
import com.nexstreaming.nexeditorsdkapis.R;

/**
 * Created by jeongwook.yoon on 2016-01-26.
 */
public class ColorEffectListAdapter extends BaseAdapter {
    private Context mContext;
    private Bitmap mSource;
    private LruCache<Integer,Bitmap> mLutCache;// = new LruCache<>(100);

    public ColorEffectListAdapter(Context context , Bitmap source ){
        mContext = context;
        mSource = source;
    }

    public void deInt(){
        mLutCache.evictAll();
    }

    @Override
    public int getCount() {
        return nexColorEffect.getPresetList().size();
    }

    @Override
    public Object getItem(int position) {
        return nexColorEffect.getPresetList().get(position);
    }

    @Override
    public long getItemId(int position) {
        return nexColorEffect.getPresetList().get(position).getLUTId();
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent)
    {
        ViewHolder holder;
        LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        convertView = inflater.inflate(R.layout.color_effect_list_item, parent, false);
        holder = new ViewHolder();
        holder.iv = (ImageView)convertView.findViewById(R.id.imageView_color_effect_apply);
        holder.name = (TextView)convertView.findViewById(R.id.textView_color_effect_name);
        convertView.setTag(holder);

        nexColorEffect effect = nexColorEffect.getPresetList().get(position);
        int lut_id = effect.getLUTId();

        Bitmap bm = null;

        if(  lut_id == 0 )
        {
            holder.iv.setColorFilter(new ColorMatrixColorFilter(effect.getColorMatrix()));
            bm = mSource;
        }
        else
        {
            if( mLutCache == null )
                mLutCache = new LruCache<>(100);

            bm = mLutCache.get(lut_id);
            if( bm == null )
            {
                bm = nexColorEffect.applyColorEffectOnBitmap(mSource, effect);
                if( bm != null )
                {
                    mLutCache.put(lut_id, bm);
                }
            }

        }
        holder.iv.setImageBitmap(bm);
        holder.name.setText(effect.getPresetName());
        holder.name.setTextColor(Color.WHITE);

        return convertView;
    }

    class ViewHolder
    {
        ImageView iv;
        TextView name;
    }
}
