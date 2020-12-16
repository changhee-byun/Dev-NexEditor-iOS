package com.nexstreaming.assetstore.test1;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.net.http.SslError;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.View;
import android.webkit.JavascriptInterface;
import android.webkit.SslErrorHandler;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;

import com.nexstreaming.nexwebview.AssetCallback;
import com.nexstreaming.nexwebview.ClientHelper;
import com.nexstreaming.nexwebview.ConfigGlobal;
import com.nexstreaming.nexwebview.nexWebview;


public class webviewtest extends FragmentActivity{
    private static final String TAG = "webviewtest";
    private static final int B_ACTIVITY = 9002;
    private static final int NONETWORK_ACTIVITY = 9003;

    private WebView webView = null;
    private int detailPageOn = 0;

    private nexWebview nexwebview;

    private Button btn_back;
    private Button btn_set;
    private String callingPackage;
    private boolean isCalled = false;

    AssetCallback assetCallback;

    String sAssetID;
    boolean bMultiselect;
    String sVendor;
    int iMimeType;

    private ClientHelper ApiClient;

    private class AndroidBridge {

        @JavascriptInterface
        public int getStatus(String assetId) {
            return nexWebview.getAssetList(assetId);
        }

        @JavascriptInterface
        public boolean download(String assetId) {
            Log.d(TAG,"download");
            nexwebview.downProgress(assetId, ApiClient);
            nexWebview.setAssetList(assetId, 0);
            return true;
        }

        @JavascriptInterface
        public boolean remove(String assetId) {
            Log.d(TAG,"remove");
            return false;
        }

        @JavascriptInterface
        public boolean dlCancel(String assetId){
            Log.d(TAG,"dlCancel= "+assetId );
            nexwebview.cancelDownload(assetId);
            return true;
        }
    }

    @SuppressLint("JavascriptInterface")
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.webviewtest);
        Intent intent = getIntent();
        sAssetID = intent.getStringExtra(ConfigGlobal.kAssetStoreAssetID);
        bMultiselect = intent.getBooleanExtra(ConfigGlobal.kAssetStoreMultiSelect, false);
        sVendor = intent.getStringExtra(ConfigGlobal.kAssetStoreVendor);
        iMimeType = intent.getIntExtra(ConfigGlobal.kAssetStoreMimeType, 0);
        nexwebview = new nexWebview(getApplicationContext());
        Log.d(TAG,"Vendor Name ="+sVendor);
        if( sVendor == null ){
            sVendor = "NexStreaming";
        }

        callingPackage = getCallingPackage();
        if(callingPackage!=null) {
            isCalled = true;
        }
        Log.d(TAG,"callingPackage="+callingPackage);
        Log.d(TAG, "Version : v0.9.3");

        ApiClient = new ClientHelper("1234",sVendor);

        btn_back = (Button) findViewById(R.id.btn1);
        btn_set = (Button) findViewById(R.id.btn2);

        btn_set.setEnabled(false);
        btn_set.setAlpha(0);

        btn_back.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                if (detailPageOn == 1) {
                    webView.goBack();
                    detailPageOn = 0;
                }
            }
        });

        btn_set.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
               // getFeaturedList();
               // Intent intent = new Intent(webviewtest.this, setting_menu.class);
               // intent.putExtra("requestPageCode", callingPackage);
               // startActivityForResult(intent, B_ACTIVITY);

            }
        });
        assetCallback = new AssetCallback() {
            @Override
            public void onTaskDone(String assetID) {
                if (isCalled) {
                    if(bMultiselect&& nexWebview.getTaskAllList().size()>1){
                        nexWebview.getTaskAllList().remove(assetID);
                        return;
                    }
                    webView.clearHistory();
                    webView.clearCache(true);
                    webView.clearView();
                    webView.destroy();
                    Intent intent = new Intent();
                    intent.putExtra("test", "data");
                    setResult(RESULT_OK, intent);
                    finish();
                }
            }

            @Override
            public void onFail(String assetId) {
                Log.d(TAG, "Download fail asset:"+assetId);
                nexwebview.cancelDownload(assetId);
                //checkNetwork(getApplicationContext());
            }
        };
        if(nexwebview.checkNetwork()<0){
            Intent intent2 = new Intent(this, NoNetwork.class);
            intent2.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
            startActivityForResult(intent2, NONETWORK_ACTIVITY);
            return;
        }
        webView = (WebView) findViewById(R.id.webview);
        webView.getSettings().setJavaScriptEnabled(true);
        webView.addJavascriptInterface(new AndroidBridge(), "AssetStoreClient");
        nexwebview.init(callingPackage, webView, ApiClient, sAssetID,iMimeType);
        nexwebview.setCallback(assetCallback);
        webView.setWebViewClient(new WebViewClient() {
            @Override
            public void onPageFinished(WebView view, String url) {
                super.onPageFinished(view, url);
            }

            @Override
            public void onReceivedSslError(WebView view, SslErrorHandler handler, SslError error) {
                handler.proceed();
            }

            @Override
            public boolean shouldOverrideUrlLoading(WebView view, String url) {
                view.loadUrl(url);
                return true;
            }
        });
    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if(requestCode == B_ACTIVITY || requestCode == NONETWORK_ACTIVITY){
            nexwebview.nexAssetDirectoryLoad();
            if(webView!=null) {
                webView.clearHistory();
                webView.clearCache(true);
                webView.clearView();
                webView.reload();
            }else{
                webView = (WebView)findViewById(R.id.webview);
                nexwebview.init(callingPackage, webView, ApiClient,sAssetID,iMimeType);
                nexwebview.setCallback(assetCallback);
            }
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        nexwebview.onDestroy();
    }

    @Override
    public void onBackPressed() {
        if (webView.canGoBack()) {
            detailPageOn = 0;
            webView.goBack();
        }else{
            webView.clearHistory();
            webView.clearCache(true);
            webView.clearView();
            webView.destroy();
            super.onBackPressed();
        }
    }



    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onStop() {
        super.onStop();
    }
}
