package com.nexstreaming.editordemo.activity;

import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.nexstreaming.editordemo.R;

import java.util.ArrayList;

public class kmTexteffectEdit5Activity extends ActionBarActivity {

    private static final String TAG = "KM_TexteffectEdit";

    private Toolbar mToolbar;
    private EditText mETextEdit_1st;
    private EditText mETextEdit_2nd;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_km_texteffect_edit4);

        mToolbar = (Toolbar) findViewById(R.id.title_edit_toolbar);
        setSupportActionBar(mToolbar);

        android.support.v7.app.ActionBar actionBar = getSupportActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setHomeAsUpIndicator(getResources().getDrawable(R.mipmap.ic_arrow_back_white_24dp_noshadow));
        actionBar.setTitle("Text Animation");

        String text_1st = getIntent().getStringExtra("titleText_1st");
        String text_2nd = getIntent().getStringExtra("titleText_2nd");
        Boolean visible_2ndlayout = getIntent().getBooleanExtra("visible_2nd", false);

        if(!visible_2ndlayout) {
            LinearLayout layout = (LinearLayout) findViewById(R.id.inputlayout_2nd);
            layout.setVisibility(View.GONE);

            mETextEdit_1st = (EditText) findViewById(R.id.inputtext_1st);
            mETextEdit_1st.setImeOptions(EditorInfo.IME_ACTION_DONE);
            if(text_1st.startsWith("#Hint:", 0)) {
                String Hint = text_1st.substring(6);
                mETextEdit_1st.setHintTextColor(Color.parseColor("#474747"));
                mETextEdit_1st.setHint(Hint);
            } else {
                mETextEdit_1st.setText(text_1st);
                mETextEdit_1st.setSelection(mETextEdit_1st.getText().length());
            }
            mETextEdit_1st.setOnEditorActionListener(new TextView.OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                    if (actionId == EditorInfo.IME_ACTION_DONE) {
                        hideSoftKeyboard(mETextEdit_1st);
                        Intent intent = new Intent(getBaseContext(), kmEditPreviewActivity.class);
                        intent.putExtra("titleText_1st", mETextEdit_1st.getText().toString());
                        setResult(1000, intent);
                        finish();
                        return true;
                    }
                    return false;
                }
            });
        } else {
            mETextEdit_1st = (EditText) findViewById(R.id.inputtext_1st);
            if(text_1st.startsWith("#Hint:", 0)) {
                String Hint = text_1st.substring(6);
                mETextEdit_1st.setHintTextColor(Color.parseColor("#474747"));
                mETextEdit_1st.setHint(Hint);
            } else {
                mETextEdit_1st.setText(text_1st);
                mETextEdit_1st.setSelection(mETextEdit_1st.getText().length());
            }
            mETextEdit_2nd = (EditText) findViewById(R.id.inputtext_2nd);
            if(text_2nd.startsWith("#Hint:", 0)) {
                String Hint = text_2nd.substring(6);
                mETextEdit_2nd.setHintTextColor(Color.parseColor("#474747"));
                mETextEdit_2nd.setHint(Hint);
            } else {
                mETextEdit_2nd.setText(text_2nd);
                mETextEdit_2nd.setSelection(mETextEdit_2nd.getText().length());
            }
            mETextEdit_2nd.setOnEditorActionListener(new TextView.OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                    if(actionId == EditorInfo.IME_ACTION_DONE) {
                        hideSoftKeyboard(mETextEdit_1st);
                        hideSoftKeyboard(mETextEdit_2nd);
                        Intent intent = new Intent(getBaseContext(), kmEditPreviewActivity.class);
                        intent.putExtra("titleText_1st", mETextEdit_1st.getText().toString());
                        intent.putExtra("titleText_2nd", mETextEdit_2nd.getText().toString());
                        setResult(1000, intent);
                        finish();
                        return true;
                    }
                    return false;
                }
            });
        }
    }

    void hideSoftKeyboard(EditText input) {
        input.setInputType(0);
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(input.getWindowToken(), 0);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu_km_texteffect_edit, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case R.id.action_check:
            {
                hideSoftKeyboard(mETextEdit_1st);
                if(mETextEdit_2nd != null)
                    hideSoftKeyboard(mETextEdit_2nd);
                Intent intent = new Intent(getBaseContext(), kmEditPreviewActivity.class);
                intent.putExtra("titleText_1st", mETextEdit_1st.getText().toString());
                if(mETextEdit_2nd != null)
                    intent.putExtra("titleText_2nd", mETextEdit_2nd.getText().toString());
                setResult(1000, intent);
                finish();
                return true;
            }
            case android.R.id.home:
            {
                Intent intent = new Intent(getBaseContext(), kmEditPreviewActivity.class);
                setResult(1018, intent);
                finish();
                return true;
            }
        }
        return super.onOptionsItemSelected(item);
    }
}
