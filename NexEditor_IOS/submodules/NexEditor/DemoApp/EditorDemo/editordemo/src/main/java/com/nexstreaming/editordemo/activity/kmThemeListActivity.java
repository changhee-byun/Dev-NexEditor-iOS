package com.nexstreaming.editordemo.activity;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
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
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexTheme;
import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.utility.dataParcel;
import com.nexstreaming.editordemo.utility.utilityThemeIcon;

import java.util.ArrayList;

public class kmThemeListActivity extends ActionBarActivity {

    private ListView mList;
    private Toolbar mToolbar;

    private ArrayList<nexTheme> mThemes;
    private Bitmap mDefaultIcon = null;

    private dataParcel mDataParcel;
    private utilityThemeIcon mThemeIcon;

    public static Activity themelistAct;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        themelistAct = this;

        if(getIntent().getIntExtra("from_preview", 0) == 1) {
            if(kmThemePreviewActivity.previewAct != null) {
                kmThemePreviewActivity.previewAct.finish();
            } else if(kmTemplateActivity.previewAct != null) {
                kmTemplateActivity.previewAct.finish();
            }
        }
        else if(getIntent().getIntExtra("from_titledit", 0) == 1) kmTitleEditActivity.titleditAct.finish();

        mDataParcel = getIntent().getParcelableExtra("parcelData");

        if(mThemes != null && mThemes.size() != 0) mThemes.clear();

        mThemes = new ArrayList<>();
        mThemeIcon = new utilityThemeIcon(getBaseContext(), mThemes);

        for(nexTheme theme: nexEffectLibrary.getEffectLibrary(this).getThemesEx()) {
            if(theme.getId().contains("basic")) {
                mThemes.add(0, theme);
                continue;
            }
            mThemes.add(theme);
        }

        mDefaultIcon = BitmapFactory.decodeResource(getResources(), R.mipmap.ic_list_avatar_40dp);

        setContentView(R.layout.activity_km_theme_list);

        mToolbar = (Toolbar) findViewById(R.id.themelist_toolbar);
        setSupportActionBar(mToolbar);

        ActionBar actionBar = getSupportActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setHomeAsUpIndicator(getResources().getDrawable(R.mipmap.ic_arrow_back_white_24dp_noshadow));
        actionBar.setTitle("Theme List");

        mList = (ListView) findViewById(R.id.themelist);

        mList.setAdapter(new ThemeListAdapter());
        mList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

                dataParcel parcel = dataParcel.getDataParcel(((nexTheme)mList.getItemAtPosition(position)).getId(), mDataParcel.getPath(), null);

                Intent intent = new Intent(getBaseContext(), kmTitleEditActivity.class);
                intent.putExtra("parcelData", parcel);
                startActivity(intent);
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        mDefaultIcon.recycle();
        mThemes.clear();
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

    private class ThemeListAdapter extends BaseAdapter {

        private LayoutInflater inflater=null;

        public ThemeListAdapter() {
            inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public int getCount() {
            return mThemes.size();
        }

        @Override
        public Object getItem(int position) {
            return mThemes.get(position);
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View vi=convertView;
            if(convertView==null)
                vi = inflater.inflate(R.layout.listitem_view, null);

            ImageView imageView = (ImageView) vi.findViewById(R.id.list_icon);
            mThemeIcon.loadBitmap(position, mDefaultIcon, imageView);

            TextView title = (TextView) vi.findViewById(R.id.list_text);
            title.setText(mThemes.get(position).getName(getBaseContext()));

            return vi;
        }
    }
}

