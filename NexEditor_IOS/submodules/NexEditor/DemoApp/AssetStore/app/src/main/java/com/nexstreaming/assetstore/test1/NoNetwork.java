package com.nexstreaming.assetstore.test1;

import android.app.Activity;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.view.View;
import android.widget.Button;

/**
 * Created by alex.kang on 2016-10-27.
 */

public class NoNetwork extends Activity{
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_no_network);
        Button btn_refresh = (Button)findViewById(R.id.refresh);
        btn_refresh.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(getConnectivityStatus(getApplicationContext())>0){
//                    Intent intent = new Intent(getBaseContext(), webviewtest.class);
//                    //intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP|Intent.FLAG_ACTIVITY_SINGLE_TOP);
//                    setResult(RESULT_OK, intent);
                    finish();
                }
            }
        });
    }

    @Override
    public void onBackPressed() {
        ActivityCompat.finishAffinity(this);
        System.runFinalizersOnExit(true);
        System.exit(0);
    }
    public static int getConnectivityStatus(Context context) {
        ConnectivityManager cm = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);

        NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
        if (null != activeNetwork) {
            if(activeNetwork.getType() == ConnectivityManager.TYPE_WIFI && activeNetwork.isConnectedOrConnecting())
                return 1;

            if(activeNetwork.getType() == ConnectivityManager.TYPE_MOBILE && activeNetwork.isConnectedOrConnecting())
                return 0;
        }
        return -1;
    }
}
