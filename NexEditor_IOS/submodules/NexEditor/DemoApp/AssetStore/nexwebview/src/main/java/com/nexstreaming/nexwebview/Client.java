package com.nexstreaming.nexwebview;

import android.util.Log;

import com.google.gson.Gson;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.NameValuePair;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.Field;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by jeongwook.yoon on 2016-11-15.
 */

public class Client {
    private String baseUrl;
    private Gson gson = new Gson();
    private int lastErrorCode;

    public static final int method_post = 1;

    public static final int ErrorCode_NORMAL = 501;
    public static final int ErrorCode_APPAUTH_FAILED = 502;
    public static final int ErrorCode_INVAILD_LICENSE = 503;
    public static final int ErrorCode_INVAILD_SCOPE = 504;
    public static final int ErrorCode_TOKEN_EXPIRE = 505;
    public static final int ErrorCode_INVALID_APIVERSION = 506;
    public static final int ErrorCode_INVALID_REQUEST = 507;
    public static final int ErrorCode_INVALID_PARAMETERS = 508;
    public static final int ErrorCode_NOT_FOUND_LIST = 59;
    public static final int ErrorCode_SOMETHING_WRONG = 510;

    public static class StandardRequest {
        public String application;
        public String edition;
        public String language;
        public String access_token;
    }

    public static class AuthorizeRequest {
        public String app_uuid;
        public String app_name;
        public String app_version;
        public String app_ucode;                // KineMaster:  NDBhNDU1NGJjYjU5MzRlZThmOTU4YmYxMmYzZTU3MmZmOTM3NTZjZTpHb29nbGU
        public String client_id;
        public String application;
        public String market_id;
        public String scope;
        public String client_info;
        public String edition;
    }

    public Client(String baseUrl){
        this.baseUrl =baseUrl;
    }

    private Map<String, String> toMap(Object obj) {
        Field[] fields = obj.getClass().getFields();
        Map<String, String> result = new HashMap<String, String>();

        for ( int i = 0; i < fields.length; ++i ) {
            try {
                //Log.d("error",fields[i].getType().getName()+" "+fields[i].getName());

                if(fields[i].getType().getName().compareTo("java.lang.String") == 0 ){
                    String val = (String)fields[i].get(obj);
                    //Log.d("error"," field "+fields[i].getName()+"="+val);
                    if( val != null ) {
                        result.put(fields[i].getName(), val );
                    }
                }
            } catch (IllegalArgumentException e) {
                e.printStackTrace();
                return null;
            } catch (IllegalAccessException e) {
                e.printStackTrace();
                return null;
            }
        }
        return result;
    }

    public <T> T  callToServer(Class<T> classOfResult ,Object reqBody, int type ,String version, String controller, String function, String element){
        return callToServer( classOfResult , reqBody,  type , version,  controller,  function,  element, null );
    }

    public <T> T  callToServer(Class<T> classOfResult ,Object reqBody, int type ,String version, String controller, String function, String element, String savePath ){

        InputStream is = null;

        ArrayList<NameValuePair> post = new ArrayList<NameValuePair>();

        Map<String, String> reqMap = toMap(reqBody);
        for(String key : reqMap.keySet() ){
            post.add(new BasicNameValuePair(key, reqMap.get(key)));
        }
        String url = baseUrl+"/"+version+"/"+controller+"/"+function;

        if( element != null ){
            url += "/"+element;
        }

        Log.d("error","callToServer url="+url);
        try {
            // defaultHttpClient
            /*
            if(checkNetwork(getApplicationContext())<0){
                return null;
            }
            */
            DefaultHttpClient httpClient = new DefaultHttpClient();
            HttpPost httpPost = new HttpPost(url);

            HttpParams httpParams = httpClient.getParams();
            HttpConnectionParams.setConnectionTimeout(httpParams, 5000);
            HttpConnectionParams.setSoTimeout(httpParams, 5000);
            UrlEncodedFormEntity entity = new UrlEncodedFormEntity(post, "UTF-8");
            httpPost.setEntity(entity);

            HttpResponse httpResponse = httpClient.execute(httpPost);
            int statuscode = httpResponse.getStatusLine().getStatusCode();
            lastErrorCode = statuscode;
            Log.d("error","lastErrorCode="+lastErrorCode);
            if(statuscode != 200 ){
                return null;
            }
            HttpEntity httpEntity = httpResponse.getEntity();
            is = httpEntity.getContent();

        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        try {
            BufferedReader reader = new BufferedReader(new InputStreamReader(
                    is, "iso-8859-1"), 8);
            StringBuilder sb = new StringBuilder();
            String line = null;
            while ((line = reader.readLine()) != null) {
                sb.append(line + "\n");
            }
            is.close();
            Log.d("error","json="+sb.toString());
            if( savePath != null ){
                File jsonFile = new File(savePath);
                try {
                    OutputStream os = new FileOutputStream(jsonFile);
                    try {
                        os.write(sb.toString().getBytes());
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    try {
                        os.close();
                    } catch (IOException e) {
                    }
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }
            }
            return gson.fromJson(sb.toString(),classOfResult);

        } catch (Exception e) {
            Log.e("Buffer Error", "Error converting result " + e.toString());
        }
        return null;
    }

    public int getLastError(){
        return lastErrorCode;
    }

    public void saveToJson(Object obj, String path, String fileName ){
        String json = gson.toJson(obj);
        File jsonFile = new File(path,fileName);
        try {
            OutputStream os = new FileOutputStream(jsonFile);
            try {
                os.write(json.getBytes());
            } catch (IOException e) {
                e.printStackTrace();
            }
            try {
                os.close();
            } catch (IOException e) {
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();

        }
    }

    public boolean downloadFile(String httpUrl, String destFile ){
        int count;

        try {
            URL url = new URL(httpUrl);
            URLConnection conection = url.openConnection();
            conection.setConnectTimeout(5000);
            conection.setReadTimeout(5000);
            conection.connect();

            InputStream input = new BufferedInputStream(url.openStream(),8192);

            OutputStream output;
            File file=null;
            file = new File(destFile);
            output = new FileOutputStream(file);

            byte data[] = new byte[1024];
            long total = 0;

            //boolean isOnline = true;
            while ((count = input.read(data)) != -1) {
                total += count;
                output.write(data, 0, count);
            }
            output.flush();
            output.close();
            input.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

}
