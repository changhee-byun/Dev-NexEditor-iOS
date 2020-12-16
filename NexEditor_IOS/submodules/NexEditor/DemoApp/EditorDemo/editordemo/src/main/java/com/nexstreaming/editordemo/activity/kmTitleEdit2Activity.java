package com.nexstreaming.editordemo.activity;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import com.nexstreaming.nexeditorsdk.nexEffectLibrary;
import com.nexstreaming.nexeditorsdk.nexTheme;
import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.utility.dataParcel;
import com.nexstreaming.editordemo.utility.utilityThemeIcon;

public class kmTitleEdit2Activity extends Activity {

    private ImageView mIViewThemeIcon;
    private TextView mTViewTheme;

    private EditText mETextOpening;
    private EditText mETextEnding;

    private dataParcel mDataParcel;
    private utilityThemeIcon mThemeIcon;

    private nexTheme mTheme;
    private String mFromActivity;

    public static Activity titledit2Act;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        titledit2Act = this;

        mDataParcel = getIntent().getParcelableExtra("parcelData");
//        mFromActivity = getIntent().getStringExtra("fromActivity");
//        if(mFromActivity != null) {
//            mTheme = nexEffectLibrary.getEffectLibrary(this).findThemeById(mDataParcel.getNameId());
//            mThemeIcon = new utilityThemeIcon(getBaseContext(), mTheme);
//        } else {
//            mTheme = nexEffectLibrary.getEffectLibrary(this).findThemeById("com.nexstreaming.kinemaster.basic");
//            mThemeIcon = new utilityThemeIcon(getBaseContext(), mTheme);
//        }
        mTheme = nexEffectLibrary.getEffectLibrary(this).findThemeById(mDataParcel.getNameId());
        mThemeIcon = new utilityThemeIcon(getBaseContext(), mTheme);

        setContentView(R.layout.activity_km_title_edit2);

        mIViewThemeIcon = (ImageView) findViewById(R.id.title_theme_icon);
        mThemeIcon.loadBitmap(0, null, mIViewThemeIcon);

        mTViewTheme = (TextView) findViewById(R.id.title_theme_id);
        mTViewTheme.setText(mTheme.getName(getBaseContext()));

        mETextOpening = (EditText) findViewById(R.id.title_opening);
        mETextOpening.setText(mDataParcel.getOpeningTitle());
        if(getIntent().getIntExtra("from_edit", 0) == 0) {
            mETextOpening.setTextColor(Color.parseColor("#ffffff"));
            mETextOpening.setSelection(mETextOpening.getText().length());
            mETextOpening.requestFocus();
        } else {
            mETextOpening.setTextColor(Color.parseColor("#474747"));
        }
        mETextOpening.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if (hasFocus) {
                    mETextOpening.setTextColor(Color.parseColor("#ffffff"));
                } else {
                    mETextOpening.setTextColor(Color.parseColor("#474747"));
                }
            }
        });
        mETextEnding = (EditText) findViewById(R.id.title_ending);
        mETextEnding.setText(mDataParcel.getEndingTitle());
        if(getIntent().getIntExtra("from_edit", 0) == 1) {
            mETextEnding.setTextColor(Color.parseColor("#ffffff"));
            mETextEnding.setSelection(mETextEnding.getText().length());
            mETextEnding.requestFocus();
        } else {
            mETextEnding.setTextColor(Color.parseColor("#474747"));
        }
        mETextEnding.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if(actionId == EditorInfo.IME_ACTION_DONE) {
//                    if(mFromActivity != null) {
//                        Intent intent = new Intent(getBaseContext(), kmThemePreviewActivity.class);
//                        intent.putExtra("openingTitle", mETextOpening.getText().toString());
//                        intent.putExtra("endingTitle", mETextEnding.getText().toString());
//                        setResult(1, intent);
//                        finish();
//                    } else {
//                        Intent intent = new Intent(getBaseContext(), kmThemePreview2Activity.class);
//                        intent.putExtra("openingTitle", mETextOpening.getText().toString());
//                        intent.putExtra("endingTitle", mETextEnding.getText().toString());
//                        setResult(1, intent);
//                        finish();
//                    }
                    Intent intent = new Intent(getBaseContext(), kmThemePreviewActivity.class);
                    intent.putExtra("openingTitle", mETextOpening.getText().toString());
                    intent.putExtra("endingTitle", mETextEnding.getText().toString());
                    setResult(1, intent);
                    finish();

                    return true;
                }
                return false;
            }
        });
        mETextEnding.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if (hasFocus) {
                    mETextEnding.setTextColor(Color.parseColor("#ffffff"));
                } else {
                    mETextEnding.setTextColor(Color.parseColor("#474747"));
                }
            }
        });
    }
}
