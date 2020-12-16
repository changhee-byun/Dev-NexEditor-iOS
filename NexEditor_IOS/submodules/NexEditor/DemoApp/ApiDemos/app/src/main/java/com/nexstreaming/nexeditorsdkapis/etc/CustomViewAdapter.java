/******************************************************************************
 * File Name        : CustomViewAdapter.java
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

package com.nexstreaming.nexeditorsdkapis.etc;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexAssetPackageManager;
import com.nexstreaming.nexeditorsdk.nexFont;
import com.nexstreaming.nexeditorsdkapis.R;

import java.util.ArrayList;
import java.util.Locale;

/**
 * Created by jeongwook.yoon on 2017-04-04.
 */

public class CustomViewAdapter extends BaseAdapter {

    private static final String TAG = "CustomViewAdapter";
    private static final  String [] aspects ={"16v9","9v16","1v2","2v1","1v1","3v4","4v3"};
    Context con;
    LayoutInflater flater;
    //ArrayList<EditorSampleTemplateActivity.ListViewItem> list;
    ArrayList<String> idsList;
    private LruCache bitmapCache= new LruCache<Object, Bitmap>(1024*1024);

    public CustomViewAdapter(Context con, ArrayList<String> list) {
        this.con = con;
        this.idsList = list;
        flater = (LayoutInflater) con.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public int getCount() {
        return idsList.size();
    }

    @Override
    public Object getItem(int position) {
        return idsList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final int pos = position;
        final String id = idsList.get(pos);
        if (convertView == null) {
            int res = R.layout.custom_view_adapter;
            convertView = flater.inflate(res, parent, false);
        }
        final View f_view = convertView;

        TextView txtTitle = (TextView) convertView.findViewById(R.id.textView1);
        ImageView imgView = (ImageView) convertView.findViewById(R.id.imageView1);
        TextView txtRatio = (TextView) convertView.findViewById(R.id.textView);

        if( id.compareTo("none") == 0 ){
            imgView.setImageBitmap(Bitmap.createBitmap(250, 250, Bitmap.Config.ARGB_8888));
            txtTitle.setText("None");
            txtRatio.setText("default");
            return convertView;
        }else if( id.startsWith("builtin.font.") || id.startsWith("system.") ){
            //font
            nexFont font = nexFont.getFont(id);
            if( font != null ){
                imgView.setImageBitmap(font.getSampleImage(con));
                txtTitle.setText(id);
                txtRatio.setText(font.isSystemFont()?"system":"builtin");
            }
            return convertView;
        }else {
            nexAssetPackageManager.Item item = nexAssetPackageManager.getAssetPackageManager(con).getInstalledAssetItemById(id);
            if (item == null) {
                Log.d(TAG, "not installed ? =" + id);
                return convertView;
            }

            Bitmap icon = (Bitmap) bitmapCache.get(id);
            if (icon == null) {
                icon = item.thumbnail();
                if (icon == null) {
                    icon = item.icon();
                }

                if (icon == null) {
                    icon = Bitmap.createBitmap(250, 250, Bitmap.Config.ARGB_8888); // this creates a MUTABLE bitmap
                }
                bitmapCache.put(id, icon);
            }

            if (icon == null && item.category() == nexAssetPackageManager.Category.font) {
                nexFont font = nexFont.getFont(id);
                if (font != null) {
                    imgView.setImageBitmap(font.getSampleImage(con));
                }
            } else {
                imgView.setImageBitmap(icon);
            }

            if (item.packageInfo() != null) {
                txtTitle.setText(item.packageInfo().assetName(null));
            } else {
                txtTitle.setText(item.name(null));
            }
            StringBuffer sb = new StringBuffer();

            nexAssetPackageManager.PreAssetCategoryAlias assetCategoryAlias = nexAssetPackageManager.getPreAssetCategoryAliasFromItem(item);
            if (assetCategoryAlias == nexAssetPackageManager.PreAssetCategoryAlias.Template ) {
                sb.append("template");
            }else if( assetCategoryAlias == nexAssetPackageManager.PreAssetCategoryAlias.Audio ){
                sb.append("audio");
            }else if( assetCategoryAlias == nexAssetPackageManager.PreAssetCategoryAlias.Overlay ){
                sb.append("overlay sticker");
            }else if( assetCategoryAlias == nexAssetPackageManager.PreAssetCategoryAlias.TextEffect ){
                sb.append("overlay template");
            }else if( assetCategoryAlias == nexAssetPackageManager.PreAssetCategoryAlias.Collage){
                sb.append("collage");
            }else if( assetCategoryAlias == nexAssetPackageManager.PreAssetCategoryAlias.StaticCollage){
                sb.append("static collage");
            }else if( assetCategoryAlias == nexAssetPackageManager.PreAssetCategoryAlias.DynamicCollage){
                sb.append("dynamic collage");
            }else if( assetCategoryAlias == nexAssetPackageManager.PreAssetCategoryAlias.BeatTemplate){
                sb.append("beat template");
            }

            sb.append(",");
            sb.append(""+item.packageInfo().assetIdx());
            sb.append(",");
            sb.append(item.id());
            sb.append(",");
            boolean findAspect = false;

            for (String aspect : aspects) {
                if (id.contains(aspect)) {
                    sb.append(aspect);
                    findAspect = true;
                    break;
                }
            }

            if (!findAspect) {
                if (item.category() == nexAssetPackageManager.Category.template ) {
                    sb.append("16v9");
                } else {
                    sb.append("All");
                }
            }
            sb.append(",");

            if (item.packageInfo().installedType() == nexAssetPackageManager.AssetInstallType.APP_ASSETS) {
                sb.append("Preload");
            } else if (item.packageInfo().installedType() == nexAssetPackageManager.AssetInstallType.SHARE) {
                sb.append("Plugin");
            } else if (item.packageInfo().installedType() == nexAssetPackageManager.AssetInstallType.STORE) {
                sb.append("Store");
            }
            sb.append(",");
            sb.append("version:"+item.packageInfo().getAssetVersionCode());
            sb.append(",");

            String expire = "OK";
            long expire_remain = item.packageInfo().expireRemain(); //list.get(pos).getExpireTime();
            if (expire_remain > 0) {
                expire_remain = item.packageInfo().installedTime() + item.packageInfo().expireRemain();
                if (expire_remain - System.currentTimeMillis() < 0) {
                    expire = "Expire";
                } else {
                    expire = fmtExpire(expire_remain - System.currentTimeMillis());
                }
            }
            sb.append(expire);
            txtRatio.setText(sb);

            return convertView;
        }
    }


    private static String fmtExpire( long ms ) {
        if( ms >= 86400000 ) {
            return String.format(Locale.US,"%02d days remain",
                    ms / 86400000);
        } else if( ms >= 3600000 ) {
            return String.format(Locale.US,"%02d hour remain",
                    ms/3600000);
        } else  {
            return String.format(Locale.US, "%02d.%01d remain",
                    ms/60000,
                    (ms%1000)/100);
        }
    }
}
