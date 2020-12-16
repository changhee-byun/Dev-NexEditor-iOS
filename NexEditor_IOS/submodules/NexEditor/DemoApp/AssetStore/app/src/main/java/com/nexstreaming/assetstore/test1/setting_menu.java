package com.nexstreaming.assetstore.test1;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.webkit.WebView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import java.io.File;
import java.util.ArrayList;

import static com.nexstreaming.assetstore.test1.setting_menu.adapter;


public class setting_menu extends Activity {
    private static int mainMenuCount = 0;
    private Button btn_back;
    private WebView webView;
    private static final String TAG = "settingMenu";
    static CustomListViewAdapter adapter = null;
    private String receiveCallingPackage =null;
    ArrayList<ListViewItem> list=null;


    @Override
    public void onStart() {
        super.onStart();

    }

    @Override
    public void onStop() {
        super.onStop();

        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if(list!=null){
            list.clear();
            list = null;
            Log.d(TAG, "onDestroy list clear");
        }

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.setting_menu);
        // set screen timeout to never

        Intent intent = getIntent();
        receiveCallingPackage = intent.getStringExtra("requestPageCode");
        //Log.d(TAG,"receiveCallingPackage ="+receiveCallingPackage);
        //  ArrayList<WebView> ser = ( ArrayList<WebView>)intent.getSerializableExtra("LIST");

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        btn_back =(Button) findViewById(R.id.btn1);

        btn_back.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });

        list = new ArrayList<>();
        adapter = new CustomListViewAdapter(this,list,receiveCallingPackage);
        adapter.loadItemsFromDB(list);

        ListView listview = (ListView) findViewById(R.id.listview1) ;
        listview.setAdapter(adapter) ;


    }

}

class ListViewItem{
    private static final String TAG = "ListViewItem";
    private Bitmap bitmapImage;
    private String assetId;
    private String assetName;

    void setBitmap(Bitmap image){

        bitmapImage = image;
    }

    Bitmap getBitmap(){

        return bitmapImage;
    }

    void setAssetId(String id){
        assetId = id;
    }

    String getAssetId(){
        return assetId;
    }

    void setAssetName(String name){

        assetName = name;
    }
    String getAssetName(){
        return assetName;
    }

}

class CustomListViewAdapter extends BaseAdapter {

    private static final String TAG = "CustomListViewAdapter";
    private  String externalStorage = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "nexassets";
    private String receiveCallingPackage;
    ArrayList<ListViewItem> list;
    Context con;
    LayoutInflater flater;


    public CustomListViewAdapter(Context con , ArrayList<ListViewItem> list , String receiveCallingPackage) {
        this.list = list;
        this.con = con;
        this.receiveCallingPackage = receiveCallingPackage;
        flater = (LayoutInflater)con.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

    }

    boolean loadItemsFromDB(ArrayList<ListViewItem> list ){

        File f;
        if( receiveCallingPackage != null ){
            f = new File(externalStorage+File.separator+receiveCallingPackage);
        }else{
            f = new File(externalStorage);
        }

        if(!f.exists())
            f.mkdir();

        ListViewItem item = null;

        if(list == null)
            list = new ArrayList<ListViewItem>();

        File[] files = f.listFiles();
        for (File inFile : files) {
            String path_ = inFile.getAbsolutePath();

            if(path_.endsWith(".jpg")){
                item = new ListViewItem();
                int start = path_.lastIndexOf("/");
                int end = path_.lastIndexOf("_");
                String assetId = path_.substring(start+1, end);

                Log.d(TAG,"CustomListViewAdapter = "+assetId);
                int start2 = path_.lastIndexOf("_");
                int end2 = path_.lastIndexOf(".");
                String assetName = path_.substring(start2+1, end2);
                Log.d(TAG,"CustomListViewAdapter 1 = "+assetName);
                item.setAssetId(assetId);
                item.setAssetName(assetName);
                item.setBitmap(BitmapFactory.decodeFile(path_));
                list.add(item);
            }
        }
        for (File inFile : files) {
            String path_ = inFile.getAbsolutePath();
            if(path_.endsWith(".temp")){
                int start = path_.lastIndexOf("/");
                int end = path_.lastIndexOf(".");
                String assetId = path_.substring(start+1, end-4);

                for(int i =0; i<list.size(); i++){
                    if(list.get(i).getAssetId().equals(assetId)){
                        list.remove(i);
                        break;
                    }
                }
            }
        }

        f=null;

        return true;

    }

    boolean clearItemsFromDB(ArrayList<ListViewItem> list ) {

        if(list!=null){
            list.clear();
        }

        return true;

    };

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

        Log.d("cho", "getView = " +position);
        if(convertView == null) {
            int res = R.layout.listview_item;
            convertView = flater.inflate(res , parent , false);

        }
        final View f_view = convertView;

        TextView txtTitle = (TextView)convertView.findViewById(R.id.textView1);
        ImageView imgView = (ImageView)convertView.findViewById(R.id.imageView1);
        ListViewItem dto = list.get(position);
        txtTitle.setText(dto.getAssetName());
        imgView.setImageBitmap(dto.getBitmap());


        Button button1 = (Button) convertView.findViewById(R.id.button1);
        button1.setOnClickListener(new Button.OnClickListener() {

            public void onClick(View v) {
                ListViewItem dto = list.get(pos);

                File f;
                if( receiveCallingPackage != null ){
                    f = new File(externalStorage+File.separator+receiveCallingPackage);
                }else{
                    f = new File(externalStorage);
                }

                File[] files = f.listFiles();
                for (File inFile : files) {
                    String path_ = inFile.getAbsolutePath();
                    int count = path_.lastIndexOf(dto.getAssetId());
                    Log.d(TAG,"cancelDeleteFile out ="+path_+","+count);
                    if(count >0 ) {
                        inFile.delete();
                    }
                }
                if(adapter!=null) {
                    adapter.clearItemsFromDB(list);
                    adapter.loadItemsFromDB(list);
                    adapter.notifyDataSetChanged();
                }

            }

        });

        return convertView;
    }
}


