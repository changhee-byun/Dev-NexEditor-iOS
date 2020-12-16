package com.nexstreaming.nexwebview;

import android.os.AsyncTask;
import android.util.Log;

import com.nexstreaming.nexwebview.json.AssetStoreAPIData;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

/**
 * Created by jeongwook.yoon on 2016-11-15.
 */



public class ClientHelper {
    private static final String TAG="ClientHelper";
    private static final String Ver="v2";
    private final String id;
    private String packageName;
    private String access_token;
    private long expires_time = 0;
    private final String vendor;
    private VendorList.vendorDefine vendorDefine;
    private Client client;
    private Client.AuthorizeRequest authReq;
    private Client.StandardRequest stdReq;

    private static Executor GET_CATEGORY_EXECUTOR = Executors.newSingleThreadExecutor();
    private List<Category> categoryList= new ArrayList<>();

    private Map<String,AssetInfo> cacheAssetInfoMap = new HashMap<>();


    void makeAuthorizeRequest(){
        if( authReq == null) {
            authReq = new Client.AuthorizeRequest();
            authReq.app_uuid = id + vendorDefine.app_name;
            authReq.app_name = vendorDefine.app_name;
            authReq.app_version = "23";
            authReq.app_ucode = vendorDefine.app_ucode;
            authReq.client_id = id;
            authReq.application = vendorDefine.app_name;
            authReq.market_id = vendorDefine.market_id;
            authReq.scope = "1";
            authReq.client_info = null;
            authReq.edition = null;
        }else{
            authReq.client_id = id;
        }
    }

    void makeStandardRequest(){
        if( stdReq == null) {
            stdReq = new Client.StandardRequest();
            stdReq.edition = "TEST";
            stdReq.language = "en";
            stdReq.application = vendorDefine.app_name;
            stdReq.access_token = access_token;
        }else{
            stdReq.access_token = access_token;
        }
    }

    public ClientHelper(String id, String vendor){
        this.id = id;
        this.vendor =vendor;
        vendorDefine = VendorList.getVendorDefine(vendor);
        client = new Client(vendorDefine.base_url);
        makeAuthorizeRequest();
    }

    public boolean isLogin(){
        if ( access_token == null ){
            return false;
        }
        if( System.currentTimeMillis() >= expires_time ){
            return false;
        }
        return true;
    }

    public int login(){
        AssetStoreAPIData.AuthorizeOrRefreshResponse ret = client.callToServer(AssetStoreAPIData.AuthorizeOrRefreshResponse.class,authReq,Client.method_post,Ver,"oauth","authorize",null );
        if( ret == null){
            return client.getLastError();
        }

        access_token = ret.access_token;
        expires_time = System.currentTimeMillis()+ ret.expires_in;
        Log.d(TAG,"login access_token="+access_token);
        makeStandardRequest();
        return 0;
    }

    public int logout(){
        AssetStoreAPIData.RefreshOrDeleteAuthRequest ret = client.callToServer(AssetStoreAPIData.RefreshOrDeleteAuthRequest.class,stdReq,Client.method_post,Ver,"oauth","delauth",null );
        access_token = null;

        if( ret == null){
            return client.getLastError();
        }
        Log.d(TAG,"logout ok!");
        return 0;
    }

    public List<Category> getCategoryList(){
        return categoryList;
    }

    public int getCategoryInfos(){
        AssetStoreAPIData.GetCategoryInfoResponse list = client.callToServer(AssetStoreAPIData.GetCategoryInfoResponse.class,stdReq,Client.method_post,Ver,"category","info",null );

        if( list == null){
            return client.getLastError();
        }
        categoryList.clear();
        for( AssetStoreAPIData.CategoryInfo info  : list.objList ){
            Category c = new Category(info.category_idx,info.category_aliasName, info.category_url);
            Log.d(TAG,"getCategoryInfos ="+c.toString());
            categoryList.add(c);

        }
        return 0;
    }

    public AssetInfo getAssetInfo(String AssetIdx){

        AssetInfo ainfo = cacheAssetInfoMap.get(AssetIdx);
        if( ainfo != null ){
            return ainfo;
        }

        AssetStoreAPIData.AssetInfo info = client.callToServer(AssetStoreAPIData.AssetInfo.class,stdReq,Client.method_post,Ver,"asset","info",AssetIdx );
        AssetInfo ret = new AssetInfo();
        ret.idx = info.idx;
        ret.packageURL = info.asset_filepath;
        ret.thumbnailURL = info.thumbnail_path;
        cacheAssetInfoMap.put(AssetIdx,ret);
        Log.d(TAG,"getAssetInfo ="+ret.toString());
        return ret;
    }

    public void getAssetInfo(String AssetIdx, String rootPath){
        AssetStoreAPIData.AssetInfo info = client.callToServer(AssetStoreAPIData.AssetInfo.class,stdReq,Client.method_post,Ver,"asset","info",AssetIdx );
        if( info == null){
            int error = client.getLastError();
            return ;
        }

        File saveDir = new File(rootPath);
        if( !saveDir.isDirectory() ){
            saveDir.mkdirs();
        }
        client.saveToJson(info, saveDir.getAbsolutePath() ,""+AssetIdx+".json" );

        AssetInfo ainfo = cacheAssetInfoMap.get(AssetIdx);
        if( ainfo != null ){
            ainfo.packageURL = info.asset_filepath;
            ainfo.thumbnailURL = info.thumbnail_path;
        }else{
            ainfo = new AssetInfo();
            ainfo.idx = info.idx;
            ainfo.packageURL = info.asset_filepath;
            ainfo.thumbnailURL = info.thumbnail_path;
            cacheAssetInfoMap.put(AssetIdx,ainfo);
            Log.d(TAG,"getAssetInfo ="+ainfo.toString());
        }

        String thumbnailUrl = info.thumbnail_path_s;
        if( thumbnailUrl == null ){
            thumbnailUrl = info.thumbnail_path;
        }

        client.downloadFile(thumbnailUrl,saveDir.getAbsolutePath()+File.separator+""+info.idx+".jpg");
    }

    public void getNewAssetList(String arg, String rootPath){
        AssetStoreAPIData.GetNewAssetList list = client.callToServer(AssetStoreAPIData.GetNewAssetList.class,stdReq,Client.method_post,Ver,"featured","list",arg );

        if( list == null){
            int error = client.getLastError();
            return ;
        }

        File saveDir = new File(rootPath+File.separator+vendor);
        if( !saveDir.isDirectory() ){
            saveDir.mkdirs();
        }

        client.saveToJson(list, saveDir.getAbsolutePath() ,"1.json" );

        for(AssetStoreAPIData.AssetInfo info :  list.objList ){
            Log.d(TAG,"getNewAssetList idx="+info.idx+", thumbnail="+info.thumbnail_path_s);
            String thumbnailUrl = info.thumbnail_path_s;
            if( thumbnailUrl == null ){
                thumbnailUrl = info.thumbnail_path;
            }
            client.downloadFile(thumbnailUrl,saveDir.getAbsolutePath()+File.separator+info.idx);
        }
    }

    public void getNewAssetListAsync(final String arg, final String rootPath){
        new Thread() {
            @Override
            public void run() {
                if( !isLogin() ){
                    login();
                }
                //getCategoryInfos();
                getNewAssetList(arg,rootPath);
                //logout();
            }
        }.start();
    }

    public void getCategoryInfosAsync(final Runnable listener){
        new AsyncTask<Void,Void, Void>() {

            @Override
            protected Void doInBackground(Void... params) {
                if (!isLogin()) {
                    login();
                }
                getCategoryInfos();
                return null;
            }

            @Override
            protected void onPostExecute(Void aVoid) {
                if(listener != null) {
                    listener.run();
                }
            }
        }.executeOnExecutor(GET_CATEGORY_EXECUTOR);
    }

    public void getAssetInfosAsync(String idx, String rootPath, final Runnable listener){
        new AsyncTask<String,Void, Void>() {

            @Override
            protected Void doInBackground(String... params) {
                if (!isLogin()) {
                    login();
                }
                getAssetInfo(params[0], params[1]);
                return null;
            }

            @Override
            protected void onPostExecute(Void aVoid) {
                if(listener != null) {
                    listener.run();
                }
            }
        }.executeOnExecutor(GET_CATEGORY_EXECUTOR,idx,rootPath);
    }

    public String getWebViewUrl(){
        return vendorDefine.webview_url;
    }

    public String getPackageName(){
        return vendorDefine.app_name;
    }

    public String getSessionID(){
        return access_token;
    }
}
