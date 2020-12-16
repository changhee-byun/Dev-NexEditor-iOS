package com.nexstreaming.editorsimple;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Environment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;
import java.util.ArrayList;

/**
 * Created by alex.kang on 2016-11-11.
 */
public class CustomViewAdapter extends BaseAdapter {

    private static final String TAG = "CustomViewAdapter";

    Context con;
    LayoutInflater flater;
    ArrayList<EditorActivity.ListViewItem> list;

    public CustomViewAdapter(Context con, ArrayList<EditorActivity.ListViewItem> list) {
        this.con = con;
        this.list = list;
        flater = (LayoutInflater) con.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public int getCount() {
        return list.size();
    }

    @Override
    public Object getItem(int position) {
        return list.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final int pos = position;

        if (convertView == null) {
            int res = R.layout.listview_item;
            convertView = flater.inflate(res, parent, false);
        }
        final View f_view = convertView;

        TextView txtTitle = (TextView) convertView.findViewById(R.id.textView1);
        ImageView imgView = (ImageView) convertView.findViewById(R.id.imageView1);
        TextView txtRatio = (TextView) convertView.findViewById(R.id.textView);
        if(list.get(pos).getAssetName()!=null) {
            txtTitle.setText(list.get(pos).getAssetName());
        }else{
            txtTitle.setText("null");
        }

        if(list.get(pos).getIcon()!=null) {
            imgView.setImageBitmap(list.get(pos).getIcon());
        }else{
            int w = 320;
            int h = 240;

            Bitmap.Config conf = Bitmap.Config.ARGB_8888; // see other conf types
            Bitmap bmp = Bitmap.createBitmap(w, h, conf); // this creates a MUTABLE bitmap

            imgView.setImageBitmap(bmp);
        }

        if( list.get(pos).getAspectRatio() != 0 ) {

            if (list.get(pos).getAspectRatio() == 9f / 16) {
                txtRatio.setText("9v16");
            } else if (list.get(pos).getAspectRatio() == 2) {
                txtRatio.setText("2v1");
            } else if (list.get(pos).getAspectRatio() == 0.5) {
                txtRatio.setText("1v2");
            } else if (list.get(pos).getAspectRatio() == 1) {
                txtRatio.setText("1v1");
            } else {
                txtRatio.setText("16v9");
            }
        }else{
            txtRatio.setVisibility(View.GONE);
        }
        return convertView;
    }
}