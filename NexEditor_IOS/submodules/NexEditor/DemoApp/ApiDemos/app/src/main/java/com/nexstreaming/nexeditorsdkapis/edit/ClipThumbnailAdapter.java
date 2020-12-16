/******************************************************************************
 * File Name        : ClipThumbnailAdapter.java
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

package com.nexstreaming.nexeditorsdkapis.edit;

import android.content.Context;
import android.graphics.Bitmap;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdkapis.R;

import java.util.ArrayList;

/**
 * Created by jeongwook.yoon on 2015-04-22.
 */
public class ClipThumbnailAdapter extends BaseAdapter {
    private Context mContext;
    private ArrayList<String> mTexts;
    private ArrayList<Bitmap> mThumb;

    public ClipThumbnailAdapter(Context c, ArrayList<String> text , ArrayList<Bitmap> Thumb ) {
        mContext = c;
        mTexts = text;
        mThumb = Thumb;
    }

    public ClipThumbnailAdapter(Context c) {
        mContext = c;
    }

    public void setParams(ArrayList<String> text , ArrayList<Bitmap> Thumb) {
        mTexts = text;
        mThumb = Thumb;
    }

    public int getCount() {
        if(mThumb != null) {
            return mThumb.size();
        }
        return 0;
    }

    public Object getItem(int position) {
        return null;
    }

    public long getItemId(int position) {
        return 0;
    }

    // create a new ImageView for each item referenced by the Adapter
    public View getView(int position, View convertView, ViewGroup parent) {
    	PersonViewHolder viewholder;    	

        if (convertView == null) { // if it not recycled, initialize some attributes
            LayoutInflater li = (LayoutInflater) mContext.getSystemService( Context.LAYOUT_INFLATER_SERVICE );            
            convertView = li.inflate(R.layout.linear_clip_thumbnails, null);
            viewholder = new PersonViewHolder();
            viewholder.timestamp = (TextView)convertView.findViewById(R.id.textview_clipthumb_time);
            viewholder.timestamp.setText(mTexts.get(position));
            viewholder.thumb = (ImageView)convertView.findViewById(R.id.imageview_clipthumb_image);
//iv.setLayoutParams(new GridView.LayoutParams(85, 85));
//iv.setScaleType(ImageView.ScaleType.CENTER_CROP);
            viewholder.thumb.setPadding(8, 8, 8, 8);
            if(viewholder.thumb.getDrawable()!=null) {
                viewholder.thumb.setImageBitmap(null);
            }
            if(mThumb.get(position)!=null) {
                viewholder.thumb.setImageBitmap(mThumb.get(position));
            }
            convertView.setTag(viewholder);
        }
        else{
        	viewholder = (PersonViewHolder) convertView.getTag();
        }
        
        viewholder.timestamp.setText(mTexts.get(position));
        viewholder.thumb.setPadding(8, 8, 8, 8);
        if(viewholder.thumb.getDrawable()!=null) {
            viewholder.thumb.setImageBitmap(null);
        }
        if(mThumb.get(position)!=null) {
            viewholder.thumb.setImageBitmap(mThumb.get(position));
        }
        return convertView;
    }
    public class PersonViewHolder
    {
      public ImageView thumb;
      public TextView timestamp;      
    }
}
