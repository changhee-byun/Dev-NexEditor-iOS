/******************************************************************************
 * File Name        : AudioVoiceChangerItemAdapter.java
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

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ImageView;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;

import java.lang.ref.WeakReference;

/**
 * Created by jeongwook.yoon on 2015-04-22.
 */
public class AudioVoiceChangerItemAdapter extends BaseAdapter {
    private nexProject mProject;
    private boolean mActiveButton;

    public AudioVoiceChangerItemAdapter(nexProject project, boolean activebutton) {
        mProject = project;
        mActiveButton = activebutton;
    }

    @Override
    public int getCount() {
        return mProject.getTotalClipCount(true);
    }

    @Override
    public Object getItem(int position) {
        return mProject.getClip(position, true);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(final int position, View convertView, final ViewGroup parent) {
        final int pos = position;
        final Context context = parent.getContext();

        if ( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(R.layout.voice_changer_item, parent, false);
        }


        if( mProject.getClip(pos, true).getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
            if( mProject.getClip(pos, true).getMainThumbnail(240f, context.getResources().getDisplayMetrics().density) == null ) {
                new AsyncThumbnail((ImageView) convertView.findViewById(R.id.imageview_audio_voice_changer_item),pos);
            }else{
                ImageView iv = (ImageView) convertView.findViewById(R.id.imageview_audio_voice_changer_item);
                iv.setImageBitmap(mProject.getClip(pos, true).getMainThumbnail(240f, context.getResources().getDisplayMetrics().density));
            }
        }else {
            ImageView iv = (ImageView) convertView.findViewById(R.id.imageview_audio_voice_changer_item);
            iv.setImageBitmap(mProject.getClip(pos, true).getMainThumbnail(240f, context.getResources().getDisplayMetrics().density));
        }

        if(mActiveButton == true) {
            Button btn1 = (Button) convertView.findViewById(R.id.button_audio_voice_changer_item_speed);
            btn1.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    final CharSequence menu[] = new CharSequence[]{"12", "25", "50", "75", "100", "125","150","175","200","400"};

                    AlertDialog.Builder builder = new AlertDialog.Builder(context);
                    builder.setTitle("change clip speed");
                    builder.setItems(menu, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            mProject.getClip(pos,true).getVideoClipEdit().setSpeedControl(Integer.parseInt(menu[which].toString()));
                        }
                    });
                    builder.show();
                }
            });
            Button btn2 = (Button) convertView.findViewById(R.id.button_audio_voice_changer_item_voice);
            btn2.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    final CharSequence menu2[] = new CharSequence[]{"0", "1", "2", "3", "4"};

                    AlertDialog.Builder builder = new AlertDialog.Builder(context);
                    builder.setTitle("change clip speed");
                    builder.setItems(menu2, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            mProject.getClip(pos,true).setVoiceChangerFactor(Integer.parseInt(menu2[which].toString()));
                        }
                    });
                    builder.show();
                }
            });
        }
        convertView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {

            }
        });

        return convertView;
    }

    class AsyncThumbnail{
        private final WeakReference<ImageView> imageViewReference;
        private final int mPos;

        public AsyncThumbnail(ImageView imageView, int pos ){
            imageViewReference = new WeakReference<ImageView>(imageView);
            mPos = pos;
            mProject.getClip(mPos, true).loadVideoClipThumbnails(new nexClip.OnLoadVideoClipThumbnailListener() {
                @Override
                public void onLoadThumbnailResult(int event) {
                    if (event == 0) {
                        final ImageView imageView = imageViewReference.get() ;
                        if (imageView != null) {
                            imageView.setImageBitmap(mProject.getClip(mPos, true).getMainThumbnail(240f, 0f ));
                        }
                    } else {
                        ;
                    }
                }
            });
        }
    }
}
