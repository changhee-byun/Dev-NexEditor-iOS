package com.nexstreaming.editordemo.activity;

import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;

import com.nexstreaming.editordemo.R;

import java.util.ArrayList;

public class kmTexteffectEdit2Activity extends ActionBarActivity {

    private static final String TAG = "KM_TexteffectEdit";

    private Toolbar mToolbar;
    private EditText mETextEditing;
    private ArrayList<String> mtextList;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_km_texteffect_edit);

        mToolbar = (Toolbar) findViewById(R.id.title_edit_toolbar);
        setSupportActionBar(mToolbar);

        android.support.v7.app.ActionBar actionBar = getSupportActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setHomeAsUpIndicator(getResources().getDrawable(R.mipmap.ic_arrow_back_white_24dp_noshadow));
        actionBar.setTitle("Text input");

        mtextList = new ArrayList<>();
        ArrayList<String> textList = getIntent().getStringArrayListExtra("titleEffect");

        mETextEditing = (EditText) findViewById(R.id.texteffect_inputtext);
        if(textList.get(0).startsWith("#Hint:", 0)) {
            String Hint = textList.get(0).substring(6);
            mETextEditing.setHintTextColor(Color.parseColor("#474747"));
            mETextEditing.setHint(Hint);
        } else {
            mETextEditing.setText(textList.get(0));
            mETextEditing.setSelection(mETextEditing.getText().length());
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
                mtextList.add(mETextEditing.getText().toString());

                hideSoftKeyboard(mETextEditing);

                Intent intent = new Intent(getBaseContext(), kmEditPreviewActivity.class);
                intent.putStringArrayListExtra("TextEffectInputText", mtextList);
                setResult(1000, intent);
                finish();
                return true;
            }
            case android.R.id.home:
            {
                Intent intent = new Intent(getBaseContext(), kmEditPreviewActivity.class);
                setResult(1000, intent);
                finish();
                return true;
            }
        }
        return super.onOptionsItemSelected(item);
    }
}
