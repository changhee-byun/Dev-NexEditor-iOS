/******************************************************************************
 * File Name        : EffectListAdapter.java
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
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;
import com.nexstreaming.nexeditorsdk.nexClipEffect;
import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexTransitionEffect;
import com.nexstreaming.nexeditorsdkapis.R;

import java.util.List;

/**
 * Created by jeongwook.yoon on 2015-06-16.
 */
public class EffectListAdapter extends BaseAdapter {
    private nexEffectLibrary mfxLib;
    private Context mContext;
    private List<nexAssetPackageManager.Item> mTemplates;
    private nexClipEffect[] mClipEffects;
    private nexTransitionEffect[] mTransitionEffects;
    private LruCache<Integer,Bitmap> LutCache = new LruCache<>(20);

    public EffectListAdapter(Context context){
        mfxLib = nexEffectLibrary.getEffectLibrary(context);
        mContext = context;
        mTemplates = nexAssetPackageManager.getAssetPackageManager(mContext).getInstalledAssetItems(nexAssetPackageManager.Category.template);
        mClipEffects = mfxLib.getClipEffects();
        mTransitionEffects = mfxLib.getTransitionEffects();
    }

    private Bitmap getTemplateIcon(int index){
        Bitmap bmp = LutCache.get(index);
        if( bmp == null ) {
            Bitmap org = mTemplates.get(index).icon();
            if( org != null ) {
                float scaleFactor = (float) 150 / (float) org.getWidth();
                int adjustedHeight = (int) (scaleFactor * org.getHeight());

                bmp = Bitmap.createScaledBitmap(org, 150, adjustedHeight, true);
                if (bmp != null) {
                    LutCache.put(index, bmp);
                }
            }
        }
        return bmp;
    }

    @Override
    public int getCount() {
        int total = 0;
        if( mTemplates != null ){
            total += mTemplates.size();
        }

        if( mClipEffects != null ){
            total += mClipEffects.length;
        }

        if( mTransitionEffects != null ){
            total += mTransitionEffects.length;
        }

        return total+3;
    }

    @Override
    public Object getItem(int position) {
        return null;
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if( convertView == null ){
            LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(R.layout.effec_list_item, parent, false);
        }

        ImageView iv = (ImageView)convertView.findViewById(R.id.imageView_effect_list);
        iv.setVisibility(View.GONE);
        TextView name = (TextView)convertView.findViewById(R.id.textView_name);
        TextView id = (TextView)convertView.findViewById(R.id.textView_id);
        TextView title = (TextView)convertView.findViewById(R.id.textView_effect_list_title);
        title.setVisibility(View.GONE);
        if( position == 0  ) {
            title.setText("Templates");
            title.setVisibility(View.VISIBLE);
            name.setVisibility(View.GONE);
            id.setVisibility(View.GONE);
        }else if( position == mTemplates.size() + 1 ) {
            title.setText("Clip Effects");
            title.setVisibility(View.VISIBLE);
            name.setVisibility(View.GONE);
            id.setVisibility(View.GONE);
        }else if( position == mTemplates.size() + mClipEffects.length + 2 ){
            title.setText("Transition Effects");
            title.setVisibility(View.VISIBLE);
            name.setVisibility(View.GONE);
            id.setVisibility(View.GONE);
        }else if(position > 0 && position < mTemplates.size()+1 ){
            int index = position-1;
            name.setText(mTemplates.get(index).name("en"));
            id.setText(mTemplates.get(index).id());
            id.setVisibility(View.VISIBLE);
            name.setVisibility(View.VISIBLE);
            Bitmap bmp =  getTemplateIcon(index);
            if( bmp != null ){
                iv.setVisibility(View.VISIBLE);
                iv.setImageBitmap(bmp);
            }
        }else if(  position >= mTemplates.size()+2 && position < (mClipEffects.length + mTemplates.size()+3) ){
            int index = position-(mTemplates.size()+2);
            name.setText(mClipEffects[index].getName(mContext)+":"+mClipEffects[index].getCategoryTitle(mContext));
            id.setText(mClipEffects[index].getId());
            id.setVisibility(View.VISIBLE);
            name.setVisibility(View.VISIBLE);
            if( mClipEffects[index].getIcon() != null ){
                iv.setVisibility(View.VISIBLE);
                iv.setImageBitmap(mClipEffects[index].getIcon());
            }
        }else if(  position >= (mTemplates.size() + mClipEffects.length+3) && position < (mTransitionEffects.length+mTemplates.size() + mClipEffects.length+4) ){
            int index = position - (mTemplates.size() + mClipEffects.length+3);
            name.setText(mTransitionEffects[index].getName(mContext)+":"+mTransitionEffects[index].getCategoryTitle(mContext));
            id.setText(mTransitionEffects[index].getId());
            id.setVisibility(View.VISIBLE);
            name.setVisibility(View.VISIBLE);
            if( mTransitionEffects[index].getIcon() != null ){
                iv.setVisibility(View.VISIBLE);
                iv.setImageBitmap(mTransitionEffects[index].getIcon());
            }
        }

        return convertView;
    }
}
