package com.nexstreaming.editordemo.activity;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexTheme;
import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.utility.dataParcel;
import com.nexstreaming.editordemo.utility.utilityThemeIcon;

import java.util.ArrayList;

public class kmTitleEditActivity extends ActionBarActivity {

    private ImageView mIViewThemeIcon;
    private TextView mTViewTheme;

    private EditText mETextOpening;
    private EditText mETextEnding;
    private Toolbar mToolbar;

    private dataParcel mDataParcel;
    private utilityThemeIcon mThemeIcon;

    private nexTheme mTheme;

    private String mFromActivity;
    private String mTemplateType;

    public static Activity titleditAct;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if(kmThemeListActivity.themelistAct != null) {
            kmThemeListActivity.themelistAct.finish();
        }
        if(kmAspectModeListActivity.templatelistAct != null) {
            kmAspectModeListActivity.templatelistAct.finish();
        }
        titleditAct = this;

        mDataParcel = getIntent().getParcelableExtra("parcelData");
        mFromActivity = getIntent().getStringExtra("fromActivity");
        if(mFromActivity == null) {
            mTheme = nexEffectLibrary.getEffectLibrary(this).findThemeById(mDataParcel.getNameId());
            mThemeIcon = new utilityThemeIcon(getBaseContext(), mTheme);
        } else {
            mTheme = nexEffectLibrary.getEffectLibrary(this).findThemeById("com.nexstreaming.kinemaster.basic");
            mThemeIcon = new utilityThemeIcon(getBaseContext(), mTheme);
            mTemplateType = getIntent().getStringExtra("templateType");
        }

        setContentView(R.layout.activity_km_title_edit);

        mToolbar = (Toolbar) findViewById(R.id.title_edit_toolbar);
        setSupportActionBar(mToolbar);

        android.support.v7.app.ActionBar actionBar = getSupportActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setHomeAsUpIndicator(getResources().getDrawable(R.mipmap.ic_arrow_back_white_24dp_noshadow));
        actionBar.setTitle("Theme Title Input");

        mIViewThemeIcon = (ImageView) findViewById(R.id.title_theme_icon);
        mThemeIcon.loadBitmap(0, null, mIViewThemeIcon);

        mTViewTheme = (TextView) findViewById(R.id.title_theme_id);
        mTViewTheme.setText(mTheme.getName(getBaseContext()));

        mETextOpening = (EditText) findViewById(R.id.title_opening);
        mETextOpening.setHintTextColor(Color.parseColor("#474747"));
        mETextOpening.setHint("Input Text");
        mETextEnding = (EditText) findViewById(R.id.title_ending);
        mETextEnding.setHintTextColor(Color.parseColor("#474747"));
        mETextEnding.setHint("Input Text");
        mETextEnding.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if(actionId == EditorInfo.IME_ACTION_DONE) {
                    ArrayList<String> titlelist = new ArrayList<>();
                    titlelist.add(mETextOpening.getText().toString());
                    titlelist.add(mETextEnding.getText().toString());
                    dataParcel parcel = dataParcel.getDataParcel(mDataParcel.getNameId(), mDataParcel.getPath(), titlelist);

                    if(mFromActivity == null) {
                        Intent intent = new Intent(getBaseContext(), kmThemePreviewActivity.class);
                        intent.putExtra("parcelData", parcel);
                        startActivity(intent);
                    } else {
                        Intent intent = new Intent(getBaseContext(), kmTemplateActivity.class);
                        intent.putExtra("parcelData", parcel);
                        intent.putExtra("templateType", mTemplateType);
                        startActivity(intent);
                    }
                    return true;
                }
                return false;
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu_km_title_edit, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case R.id.action_preview:
            {
                ArrayList<String> titlelist = new ArrayList<>();
                titlelist.add(mETextOpening.getText().toString());
                titlelist.add(mETextEnding.getText().toString());
                dataParcel parcel = dataParcel.getDataParcel(mDataParcel.getNameId(), mDataParcel.getPath(), titlelist);

                if(mFromActivity == null) {
                    Intent intent = new Intent(getBaseContext(), kmThemePreviewActivity.class);
                    intent.putExtra("parcelData", parcel);
                    startActivity(intent);
                } else {
                    Intent intent = new Intent(getBaseContext(), kmTemplateActivity.class);
                    intent.putExtra("parcelData", parcel);
                    intent.putExtra("templateType", mTemplateType);
                    startActivity(intent);
                }
                return true;
            }
            case android.R.id.home:
            {
                dataParcel Theme = dataParcel.getDataParcel(null, mDataParcel.getPath(), null);

                if(mFromActivity == null) {
                    Intent intent = new Intent(getBaseContext(), kmThemeListActivity.class);
                    intent.putExtra("from_titledit", 1);
                    intent.putExtra("parcelData", Theme);
                    startActivity(intent);
                } else {
                    Intent intent = new Intent(getBaseContext(), kmAspectModeListActivity.class);
                    intent.putExtra("from_titledit", 1);
                    intent.putExtra("parcelData", Theme);
                    startActivity(intent);
                }
                return true;
            }
        }
        return super.onOptionsItemSelected(item);
    }
}
