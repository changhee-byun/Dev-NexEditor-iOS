package com.nexstreaming.nexwebview;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Environment;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.webkit.WebSettings;
import android.webkit.WebView;

import com.nexstreaming.nexwebview.localProtocol.nexCipher;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;


public class nexWebview {
    private static final String TAG = "nexWebview";

    private WebView webView = null;

    private static HashMap<String, Integer> assetList;
    private static HashMap<String, DownloadTask> taskList;

    private DownloadTask mtask;

    ArrayList<HashMap<String, String>> categoryList;


    private int mimeType;
    private String storeDomainURL;
    private String callingPackage;
    private boolean isCalled = false;
    nexCipher cipher;

    String pubkeyfilename = "RSApubkey";

    String privkeyfilename = "RSAprivkey";

    public static File pubkeyfile;
    public static File privkeyfile;

    private static Context context = null;

    private ClientHelper ApiClient;

    private static String externalStorage = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "nexassets";
    AssetCallback assetCallback;

    String sAssetID;

    public nexWebview(  Context context) {
        this.context = context;
    }

    public void downProgress(final String assetId, final ClientHelper ApiClient) {
        Log.d(TAG, "downProgress");
        String AssetSavePath = null;

        if (callingPackage != null)
            AssetSavePath = externalStorage+ File.separator+ callingPackage;
        else AssetSavePath = externalStorage;

        ApiClient.getAssetInfosAsync(assetId, AssetSavePath, new Runnable() {
            @Override
            public void run() {
                AssetInfo ai = ApiClient.getAssetInfo(assetId);
                String saveFileName = null;

                if (callingPackage != null)
                    saveFileName = callingPackage + File.separator + new String(assetId);
                else saveFileName = new String(assetId);

                mtask = new DownloadTask(assetCallback, context);
                mtask.setAssetId(assetId);
                mtask.setPackageName(callingPackage);
                mtask.setMode("NORMAL");
                mtask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, ai.packageURL(), saveFileName + ".zip", "NORMAL");
                setTaskList(assetId,mtask);
            }
        });
    }

    public void init(String callingPackage, WebView webView, ClientHelper ApiClient, String AssetIDx, int mimeType) {
        this.callingPackage = callingPackage;
        this.webView = webView;
        if(this.callingPackage!=null) {
            isCalled = true;
        }
        this.ApiClient = ApiClient;
        categoryList = new ArrayList<>();
        assetList = new HashMap<>();
        taskList = new HashMap<>();
        sAssetID = AssetIDx;
        this.mimeType = mimeType;
        if(checkNetwork()<0){
            return;
        }
        webViewInit();
    }
    public void setCallback(AssetCallback callback){
        this.assetCallback = callback;
    }

    private String getMainPageUrl(){
        String categoryUrl = null;
        for( Category cat : ApiClient.getCategoryList() ) {

            if (cat.getAliasName().compareToIgnoreCase("Template") == 0) {
                categoryUrl = cat.getCategoryUrl();
                if( (mimeType & ConfigGlobal.AssetStoreMimeType_Template) == ConfigGlobal.AssetStoreMimeType_Template ){
                    break;
                }
            } else if (cat.getAliasName().compareToIgnoreCase("Effect") == 0) {
                if( (mimeType & ConfigGlobal.AssetStoreMimeType_Effect) == ConfigGlobal.AssetStoreMimeType_Effect ){
                    categoryUrl = cat.getCategoryUrl();
                    break;
                }

            } else if (cat.getAliasName().compareToIgnoreCase("ClipGraphics") == 0) {
                if( (mimeType & ConfigGlobal.AssetStoreMimeType_Background) == ConfigGlobal.AssetStoreMimeType_Background ){
                    categoryUrl = cat.getCategoryUrl();
                    break;
                }

            } else if (cat.getAliasName().compareToIgnoreCase("Transition") == 0) {
                if( (mimeType & ConfigGlobal.AssetStoreMimeType_Transition) == ConfigGlobal.AssetStoreMimeType_Transition ){
                    categoryUrl = cat.getCategoryUrl();
                    break;
                }

            } else if (cat.getAliasName().compareToIgnoreCase("Font") == 0) {
                if( (mimeType & ConfigGlobal.AssetStoreMimeType_Font) == ConfigGlobal.AssetStoreMimeType_Font ){
                    categoryUrl = cat.getCategoryUrl();
                    break;
                }

            } else if (cat.getAliasName().compareToIgnoreCase("Overlay") == 0) {
                if( (mimeType & ConfigGlobal.AssetStoreMimeType_Overlay) == ConfigGlobal.AssetStoreMimeType_Overlay ){
                    categoryUrl = cat.getCategoryUrl();
                    break;
                }

            } else if (cat.getAliasName().compareToIgnoreCase("Audio") == 0) {
                if( (mimeType & ConfigGlobal.AssetStoreMimeType_Audio) == ConfigGlobal.AssetStoreMimeType_Audio ){
                    categoryUrl = cat.getCategoryUrl();
                    break;
                }
            } else if (cat.getAliasName().compareToIgnoreCase("Text Effects") == 0) {
                if( (mimeType & ConfigGlobal.AssetStoreMimeType_TitleTemplate) == ConfigGlobal.AssetStoreMimeType_TitleTemplate ){
                    categoryUrl = cat.getCategoryUrl();
                    break;
                }
            }
        }
        return categoryUrl;
    }

    private int webViewInit(){
        initGenKey();

        if( callingPackage == null ){
            File file = new File(context.getFilesDir(),".lastCall");
            if( file.isFile() ) {
                try {
                    InputStream os = new FileInputStream(file);
                    byte[] data = new byte[1024];
                    int n = os.read(data);
                    callingPackage = new String(data,0,n);
                    os.close();
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }else{
            File file = new File(context.getFilesDir(),".lastCall");
            try {
                OutputStream os = new FileOutputStream(file);
                os.write(callingPackage.getBytes());
                os.close();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        ApiClient.getCategoryInfosAsync(new Runnable() {
            @Override
            public void run() {
                if (sAssetID != null){
                    storeDomainURL = ApiClient.getWebViewUrl() + "/asset/view/"+sAssetID;
                }else{
                    String categoryUrl = getMainPageUrl();
                    storeDomainURL = ApiClient.getWebViewUrl() + categoryUrl;
                }
                Log.d(TAG,"storeDomainURL="+storeDomainURL);
                webViewConnet(webView, storeDomainURL);
                nexAssetDirectoryLoad();
            }
        });
        return 0;
    }

    public void nexAssetDirectoryLoad(){
        Log.d(TAG, "nexAssetDirectoryLoad");

        File f;
        if( callingPackage != null ){
            f = new File(externalStorage+File.separator+callingPackage);
        }else{
            f = new File(externalStorage);
        }

        if(!f.exists())
            f.mkdirs();

        assetList.clear();

        File[] files = f.listFiles();
        for (File inFile : files) {
            String path_ = inFile.getAbsolutePath();
            if(path_.endsWith(".zip")){
                int start = path_.lastIndexOf("/");
                int end = path_.lastIndexOf(".");
                String name_ = path_.substring(start+1, end);
                Log.d(TAG, "getSatus Id Search ="+name_);
                assetList.put(name_,100);
            }
        }
        f= null;

    }
    public void onDestroy() {
        if(assetList != null) {
            assetList.clear();
        }

        if(taskList != null){
            cancelAllDownloadTask();
            taskList.clear();
        }
    }

    void webViewConnet(WebView webview, String webview_url) {
        Log.d(TAG, "webViewConnet");
        Map<String, String> extraHeader = new HashMap<String, String>();
        if(checkNetwork()<0){
            return;
        }
        webview.getSettings().setJavaScriptEnabled(true);

        extraHeader.put("application", ApiClient.getPackageName());
        extraHeader.put("edition", "TEST");
        extraHeader.put("language", "en");
        extraHeader.put("subscription", "free");
        extraHeader.put("permission", "free");
        extraHeader.put("app_version", "free");
        extraHeader.put("env", "prod");
        extraHeader.put("token", ApiClient.getSessionID());

        webview.loadUrl(webview_url, extraHeader);

        webview.getSettings().setLoadWithOverviewMode(true);
        webview.getSettings().setSupportZoom(true);
        webview.getSettings().setBuiltInZoomControls(true);
        webview.getSettings().setDisplayZoomControls(false);
        webview.getSettings().setLayoutAlgorithm(WebSettings.LayoutAlgorithm.SINGLE_COLUMN);
        webview.getSettings().setCacheMode(WebSettings.LOAD_NO_CACHE);
        webview.setHorizontalScrollBarEnabled(false);
        webview.getSettings().setLoadWithOverviewMode(true);
        webview.getSettings().setUseWideViewPort(true);

        return;
    }

    private void initGenKey() {
        Log.d(TAG, "initGenKey");
        pubkeyfile = new File(context.getFilesDir(), pubkeyfilename);
        privkeyfile = new File(context.getFilesDir(), privkeyfilename);

        cipher = new nexCipher();

        if (!pubkeyfile.exists()) {
            cipher.RSAGenKey();
            //save RSA key to data
            FileOutputStream outputStream;
            try {
                outputStream = context.openFileOutput(pubkeyfilename, Context.MODE_PRIVATE);
                outputStream.write(cipher.getRSApubKey());
                outputStream.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
            FileOutputStream outputStream2;
            try {
                outputStream2 = context.openFileOutput(privkeyfilename, Context.MODE_PRIVATE);
                outputStream2.write(cipher.getRSAprivKey());
                outputStream2.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
    public String makeUUID() {
        Log.d(TAG, "makeUUID");
        final TelephonyManager tm = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);

        final String tmDevice, tmSerial, androidId;
        tmDevice = "" + tm.getDeviceId();
        tmSerial = "" + tm.getSimSerialNumber();
        androidId = "" + Settings.Secure.getString(context.getContentResolver(), Settings.Secure.ANDROID_ID);

        UUID deviceUuid = new UUID(androidId.hashCode(), ((long) tmDevice.hashCode() << 32) | tmSerial.hashCode());
        String deviceId = deviceUuid.toString();
        return deviceId;
    }
    public static void setAssetList(String assetid, int value) {
        if (assetList != null) {
            assetList.put(assetid, value);
        }
    }
    public static int getAssetList(String assetid) {
        if (assetList != null && assetList.containsKey(assetid)) {
            return assetList.get(assetid);
        }
        return -1;
    }

    public static void setTaskList(String assetid, DownloadTask mtask) {
        if (taskList != null) {
            taskList.put(assetid, mtask);
        }
    }
    public static HashMap<String, DownloadTask> getTaskAllList() {
        return taskList;
    }
    public static DownloadTask getTaskList(String assetid) {
        if (taskList != null && taskList.containsKey(assetid)) {
            return taskList.get(assetid);
        }
        return null;
    }

    //TODO : make refreshauth, delAuth, getAssetInfo, getFeaturedAssetInfo, getNewAssetList, getAssetLangStr, getCategoryList
    public static int getConnectivityStatus(Context context) {
        ConnectivityManager cm = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);

        NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
        if (null != activeNetwork) {
            if (activeNetwork.getType() == ConnectivityManager.TYPE_WIFI && activeNetwork.isConnectedOrConnecting())
                return 1;

            if (activeNetwork.getType() == ConnectivityManager.TYPE_MOBILE && activeNetwork.isConnectedOrConnecting())
                return 0;
        }
        return -1;
    }

    public int checkNetwork() {
        int networkstate = getConnectivityStatus(context);
        if (networkstate < 0)
        {
            return -1;
        }
        return networkstate;
    }
    public void cancelDownload(String assetId){
        Log.d(TAG, "cancel Download:"+assetId);
        DownloadTask temptask = taskList.get(assetId);
        temptask.cancel(true);
        assetList.remove(assetId);
        taskList.remove(assetId);

    }
    private void cancelAllDownloadTask(){
        Log.d(TAG, "cancel Download All task");
        if(taskList.size()>0){
            for(String key : taskList.keySet()){
                DownloadTask temptask = taskList.get(key);
                temptask.cancel(true);
            }
        }
    }
}
