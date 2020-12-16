/******************************************************************************
 * File Name        : ProjectEditItemAdapter.java
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

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;

import java.lang.ref.WeakReference;

/**
 * Created by jeongwook.yoon on 2015-04-22.
 */
public class ProjectEditItemAdapter extends BaseAdapter {
    private nexProject mProject;
    private boolean mActiveButton;

    public ProjectEditItemAdapter( nexProject project, boolean activebutton) {
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
    public View getView(int position, View convertView, final ViewGroup parent) {
        final int pos = position;
        final Context context = parent.getContext();


        if ( convertView == null ) {
            LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflater.inflate(R.layout.linear_project_edit_item, parent, false);
        }

        if( mProject.getClip(pos, true).getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
            if( mProject.getClip(pos, true).getMainThumbnail(240f, context.getResources().getDisplayMetrics().density) == null ) {
                new AsyncThumbnail((ImageView) convertView.findViewById(R.id.imageview_project_edit_item),pos);
            }else{
                ImageView iv = (ImageView) convertView.findViewById(R.id.imageview_project_edit_item);
                iv.setImageBitmap(mProject.getClip(pos, true).getMainThumbnail(240f, context.getResources().getDisplayMetrics().density));
            }
        }else {
            ImageView iv = (ImageView) convertView.findViewById(R.id.imageview_project_edit_item);
            iv.setImageBitmap(mProject.getClip(pos, true).getMainThumbnail(240f, context.getResources().getDisplayMetrics().density));
        }

        if(mActiveButton == true) {
            Button btn2 = (Button) convertView.findViewById(R.id.button_project_edit_item_clip_play);
            btn2.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    ProjectEditActivity act = (ProjectEditActivity) context;
                    act.playClip(pos);
                }
            });

            Button btn1 = (Button) convertView.findViewById(R.id.button_project_edit_item_clip_change);
            btn1.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    ProjectEditActivity act = (ProjectEditActivity) context;
                    final boolean isPlaying = act.isEnginePlaying();

                    if(isPlaying == true) {
                        Toast.makeText(context, "retry after playing... ", Toast.LENGTH_SHORT).show();
                        return ;
                    }

                    CharSequence menu[] = new CharSequence[]{"change clip effect", "change transition effect","rotate", "up", "down", "delete"};

                    AlertDialog.Builder builder = new AlertDialog.Builder(context);
                    builder.setTitle("change clip setting");
                    builder.setItems(menu, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            switch (which) {
                                case 0:
                                    ProjectEditActivity c = (ProjectEditActivity) context;
                                    c.getClipeffect(pos);
                                    break;
                                case 1:
                                    ProjectEditActivity d = (ProjectEditActivity) context;
                                    d.getTransition(pos);
                                    break;

                                case 2:
                                    int degree = mProject.getClip(pos, true).getRotateDegree() + 90;
                                    Toast.makeText(context, "Rotate(" + degree + ") : " + mProject.getClip(pos, true).getPath(), Toast.LENGTH_SHORT).show();
                                    mProject.getClip(pos, true).setRotateDegree(degree);
                                    ApiDemosConfig.getApplicationInstance().getEngine().updateProject();
//                                    int seekTime = mProject.getClip(pos, true).getProjectStartTime() + (mProject.getClip(pos, true).getProjectEndTime() - mProject.getClip(pos, true).getProjectStartTime()) / 2;
//                                    ApiDemosConfig.getApplicationInstance().getEngin().seek(seekTime);
                                    break;
                                case 3:
                                    if (pos > 0) {
                                        Toast.makeText(context, "Move UP : " + mProject.getClip(pos, true).getPath(), Toast.LENGTH_SHORT).show();
                                        mProject.move(pos - 1, mProject.getClip(pos, true));
                                        notifyDataSetChanged();
                                    }
                                    break;
                                case 4:
                                    if (pos < (getCount() - 1)) {
                                        Toast.makeText(context, "Move Down : " + mProject.getClip(pos, true).getPath(), Toast.LENGTH_SHORT).show();
                                        mProject.move(pos + 1, mProject.getClip(pos, true));
                                        notifyDataSetChanged();
                                    }
                                    break;
                                case 5:
                                    Toast.makeText(context, "Delete : " + mProject.getClip(pos, true).getPath(), Toast.LENGTH_SHORT).show();
                                    mProject.remove(mProject.getClip(pos, true));
                                    notifyDataSetChanged();
                                    break;
                                default:
                                    break;
                            }
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
                        if (imageView != null && mProject.getTotalClipCount(true) > 0 && mProject.getClip(mPos, true)!=null) {
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
