package com.nexstreaming.app.assetstore.zte.ui;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.MenuItem;


/**
 * Created by ojin.kwon on 2016-11-17.
 */

public class BaseActivity extends AppCompatActivity {


    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()){
            case android.R.id.home :
                onSelectedHome();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    protected void onSelectedHome(){
        onBackPressed();
    }

    protected void onSelectedSetting(){
    }
}
