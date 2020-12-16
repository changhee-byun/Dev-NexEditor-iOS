package com.nexstreaming.nexeditorsdkapis.etc;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Movie;
import android.os.SystemClock;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.webkit.WebView;

import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.GifWebView;

import java.util.ArrayList;

public class GifViewActivity extends Activity {
    private ArrayList<String> m_listfilepath;
    private final static String Tag = "GifViewActivity";
    private GifWebView view;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_gif_view);
        m_listfilepath = new ArrayList<String>();

        Intent intent = getIntent();
        m_listfilepath = intent.getStringArrayListExtra("filelist");
        view = (GifWebView)findViewById(R.id.webView_gif);

        Log.d(Tag,"gif file path="+m_listfilepath.get(0));

        view.setGifAssetPath("file://" + m_listfilepath.get(0));
        view.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                view.reload();
            }
        });
    }

}
