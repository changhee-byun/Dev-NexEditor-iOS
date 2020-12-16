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
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;

import com.nexstreaming.editordemo.R;
import com.nexstreaming.editordemo.fragment.kmSTColorPallet2Fragment;
import com.nexstreaming.editordemo.fragment.kmSTColorPalletFragment;

import java.util.ArrayList;

public class kmTexteffectEdit4Activity extends ActionBarActivity implements
        kmSTColorPalletFragment.OnSTColorPalletSelectedListener,
        kmSTColorPallet2Fragment.OnSTColorPalletSelectedListener {

    private static final String TAG = "KM_TexteffectEdit";

    private Toolbar mToolbar;
    private EditText mETextEditing;
    private EditText mETextEditing2;
    private ArrayList<Integer> mColorList;
    private ArrayList<String> mtextList;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_km_texteffect_edit3);

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

        mETextEditing2 = (EditText) findViewById(R.id.texteffect_inputtext2);
        if(textList.get(1).startsWith("#Hint:", 0)) {
            String Hint = textList.get(1).substring(6);
            mETextEditing2.setHintTextColor(Color.parseColor("#474747"));
            mETextEditing2.setHint(Hint);
        } else {
            mETextEditing2.setText(textList.get(1));
            mETextEditing2.setSelection(mETextEditing2.getText().length());
        }

        mColorList = getIntent().getIntegerArrayListExtra("colorPallet");

        ArrayList<String> titlePalletList = getIntent().getStringArrayListExtra("titlePallet");

        Fragment colorPalletFragment = new kmSTColorPalletFragment();

        Bundle bundle = new Bundle();
        bundle.putString("From", "Form_a");
        bundle.putString("titlePallet", titlePalletList.get(0));
        bundle.putInt("colorPallet", mColorList.get(0).intValue());
        colorPalletFragment.setArguments(bundle);

        Log.d(TAG, "1st titlePallet=" + titlePalletList.get(0) + " colorPallet=" + mColorList.get(0).intValue());

        FragmentTransaction fragmentTransaction = getFragmentManager().beginTransaction();
        fragmentTransaction.replace(R.id.fragment_container, colorPalletFragment);
        fragmentTransaction.commit();

        Fragment colorPalletFragment2 = new kmSTColorPallet2Fragment();

        Bundle bundle2 = new Bundle();
        bundle2.putString("From", "Form_b");
        bundle2.putString("titlePallet", titlePalletList.get(1));
        bundle2.putInt("colorPallet", mColorList.get(1).intValue());
        colorPalletFragment2.setArguments(bundle2);

        Log.d(TAG, "2nd titlePallet=" + titlePalletList.get(1) + " colorPallet=" + mColorList.get(1).intValue());

        FragmentTransaction fragmentTransaction2 = getFragmentManager().beginTransaction();
        fragmentTransaction2.replace(R.id.fragment_container2, colorPalletFragment2);
        fragmentTransaction2.commit();
    }

    void hideSoftKeyboard(EditText input) {
        input.setInputType(0);
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(input.getWindowToken(), 0);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(mColorList != null) {
            mColorList.clear();
        }
        if(mtextList != null) {
            mtextList.clear();
        }
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
                mtextList.add(mETextEditing2.getText().toString());

                hideSoftKeyboard(mETextEditing);

                Intent intent = new Intent(getBaseContext(), kmEditPreviewActivity.class);
                intent.putStringArrayListExtra("TextEffectInputText", mtextList);
                intent.putIntegerArrayListExtra("TextEffectColor", mColorList);
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

    @Override
    public void onStColorPalletSelected(String from, int colorpallet) {
        Log.d(TAG, "from=" + from + " colorpallet=" + colorpallet);
        if(from.equals("Form_a")) {
            mColorList.set(0, Integer.valueOf(colorpallet));
        } else {
            mColorList.set(1, Integer.valueOf(colorpallet));
        }
    }

    @Override
    public void onStSwitchSelected(String from, boolean checked) {

    }
}
