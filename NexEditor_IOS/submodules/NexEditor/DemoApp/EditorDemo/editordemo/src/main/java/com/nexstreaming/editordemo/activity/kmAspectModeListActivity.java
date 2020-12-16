package com.nexstreaming.editordemo.activity;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.nexstreaming.editordemo.kmsdk.kmEngine;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.utility.dataParcel;

import java.util.ArrayList;

public class kmAspectModeListActivity extends ActionBarActivity {

    private ListView mList;
    private Toolbar mToolbar;

    private dataParcel mDataParcel;

    public static Activity templatelistAct;

    private ArrayList<String> list_aspectmode = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        templatelistAct = this;

        if(getIntent().getIntExtra("from_preview", 0) == 1) kmTemplateActivity.previewAct.finish();

        mDataParcel = getIntent().getParcelableExtra("parcelData");

        list_aspectmode.add("16:9 Aspect Mode");
        list_aspectmode.add("9:16 Aspect Mode");
        list_aspectmode.add("1:1 Aspect Mode");

        setContentView(R.layout.activity_km_aspect_list);

        mToolbar = (Toolbar) findViewById(R.id.aspectlist_toolbar);
        setSupportActionBar(mToolbar);

        ActionBar actionBar = getSupportActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setHomeAsUpIndicator(getResources().getDrawable(R.mipmap.ic_arrow_back_white_24dp_noshadow));
        actionBar.setTitle("Aspect List");

        mList = (ListView) findViewById(R.id.aspectlist);

        mList.setAdapter(new AspectListAdapter());
        mList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

                kmEngine.releaseEngine(); //

                if(position == 0) {
                    // 16:9 Aspect Mode
                    nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_16v9);
                } else if(position == 1) {
                    // 9:16 Aspect Mode
                    nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_9v16);
                } else {
                    // 1:1 Aspect Mode
                    nexApplicationConfig.setAspectMode(nexApplicationConfig.kAspectRatio_Mode_1v1);
                }

                kmEngine.createEngine(); //

                dataParcel parcel = dataParcel.getDataParcel(null, mDataParcel.getPath(), null);

                Intent intent = new Intent(getBaseContext(), kmTemplateActivity.class);
                intent.putExtra("parcelData", parcel);
                intent.putExtra("aspectMode", position/*0->"16:9", 1->"9:16", 2->"1:1"*/);
                startActivity(intent);
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if(list_aspectmode != null) {
            list_aspectmode.clear();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private class AspectListAdapter extends BaseAdapter {

        private LayoutInflater inflater=null;

        public AspectListAdapter() {
            inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return list_aspectmode.size();
        }

        @Override
        public Object getItem(int position) {
            return null;
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View vi=convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem5_view, null);

            TextView title = (TextView) vi.findViewById(R.id.list_text);
            title.setText(list_aspectmode.get(position));

            return vi;
        }
    }
}

