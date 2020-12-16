package com.nexstreaming.nexwebview;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Environment;
import android.util.Log;

import com.nexstreaming.nexwebview.localProtocol.nexCipher;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;

/**
 * Created by alex.kang on 2016-10-23.
 */
public class DownloadTask extends AsyncTask<String, String, String> {
    private static final String TAG = "Download Task";
    private  String externalStorage = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "nexassets";
    private String receiveCallingPackage;
    nexCipher cipher = new nexCipher();
    byte[] encAESkey;
    String assetId;
    String taskMode;

    public AssetCallback delegate;
    public Context context;
    public DownloadTask( AssetCallback delegate, Context context) {
        this.context = context;
        this.delegate = delegate;

    }

    public void setAssetId(String id){

        assetId = id;
    }

    public void setMode(String mode){

        taskMode = mode;
    }

    public void setPackageName(String name){

        receiveCallingPackage = name;
    }


    void cancelDeleteFile(){

        File f;
        if( receiveCallingPackage != null ){
            f = new File(externalStorage+File.separator+receiveCallingPackage);
        }else{
            f = new File(externalStorage);
        }

        File[] files = f.listFiles();

        for (File inFile : files) {
            String path_ = inFile.getAbsolutePath();
            int count = path_.lastIndexOf(assetId);
            Log.d(TAG,"cancelDeleteFile out ="+path_+","+count);
            if(count >0 ) {
                Log.d(TAG,"cancelDeleteFile ="+path_);
                inFile.delete();
            }
        }

    }

    @Override
    protected void onCancelled() {
        Log.d(TAG,"onCancelled");
        super.onCancelled();
        if(taskMode!=null&&taskMode.equals("NORMAL"))
            cancelDeleteFile();

    }

    /**
     * Before starting background thread Show Progress Bar Dialog
     * */
    @Override
    protected void onPreExecute() {
        super.onPreExecute();
        Log.d(TAG,"DownloadFileFromURL");

    }

    /**
     * Downloading file in background thread
     * */
    @Override
    protected String doInBackground(String... f_url) {
        int count;
        String orgfile= null;
        try {
            URL url = new URL(f_url[0]);
            URLConnection conection = url.openConnection();
            conection.setConnectTimeout(5000);
            conection.setReadTimeout(5000);
            conection.connect();

            int lenghtOfFile = conection.getContentLength();

            InputStream input = new BufferedInputStream(url.openStream(),8192);

            String subDir = null;
            int filenameStart = f_url[1].lastIndexOf('/');
            if( filenameStart < 0 ){
                filenameStart = 0;
                if(f_url[2].equals("NORMAL")) {
                    orgfile= f_url[1]+".temp";
                }else orgfile= f_url[1];
            }else{
                filenameStart += 1;
                subDir =  f_url[1].substring(0, filenameStart-1);
                if(f_url[2].equals("NORMAL")) {
                    orgfile= f_url[1].substring(filenameStart)+".temp";
                }else orgfile= f_url[1].substring(filenameStart);
            }

            OutputStream output;
            File file=null;
            if( subDir == null ) {
                file = new File(Environment.getExternalStorageDirectory() + "/nexassets/", orgfile);
                output = new FileOutputStream(file);
            }else{
                file = new File(Environment.getExternalStorageDirectory() + "/nexassets/"+subDir+"/", orgfile);
                output = new FileOutputStream(file);
            }

            byte data[] = new byte[1024];

            long total = 0;
            int end = f_url[1].lastIndexOf(".");
            String name_=null;

            if(end>0)
                 name_ = f_url[1].substring(filenameStart, end);

            boolean isOnline = true;
            while ((count = input.read(data)) != -1) {
                if(!isOnline()){
                    isOnline = false;
                    break;
                }
                total += count;
                if(f_url[2].equals("NORMAL")) {
                    publishProgress("" + (long) ((total * 100L) / lenghtOfFile), name_);

                }
                output.write(data, 0, count);
            }
            output.flush();
            output.close();
            input.close();
            if(!isOnline){
                file.delete();
                if (delegate!=null){
                    delegate.onFail(name_);
                }
                //return null;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        if(f_url[2].equals("NORMAL")) {
            return f_url[1];
        }else {
            return null;
        }
    }

    /**
     * Updating progress bar
     * */
    protected void onProgressUpdate(String... progress) {
        nexWebview.setAssetList(progress[1], Integer.parseInt(progress[0]));
        //assetList.put(progress[1], Integer.parseInt(progress[0]));
        //pDialog.setProgress(Integer.parseInt(progress[0]));
    }

    /**
     * After completing background task Dismiss the progress dialog
     * **/
    @Override
    protected void onPostExecute(String file_name) {
        if(file_name == null) {
            return;
        }
        //rename temp file
        Log.d(TAG,"DownloadTask postExcute");

        String subDir = null;
        String orgfile= null;
        int filenameStart = file_name.lastIndexOf('/');
        if( filenameStart < 0 ){
            filenameStart = 0;
            orgfile = file_name;
        }else{
            filenameStart += 1;
            subDir =  file_name.substring(0, filenameStart);
            orgfile= file_name.substring(filenameStart);
        }
        File tmpfile = null;
        File file = null;
        if(filenameStart==0) {
            tmpfile = new File(Environment.getExternalStorageDirectory() + "/nexassets/" + orgfile + ".temp");
            file = new File(Environment.getExternalStorageDirectory() + "/nexassets/" + orgfile);
            if (tmpfile.exists()) {
                tmpfile.renameTo(file);
            }
        }else {
            tmpfile = new File(Environment.getExternalStorageDirectory() + "/nexassets/" +subDir+File.separator+ orgfile + ".temp");
            file = new File(Environment.getExternalStorageDirectory() + "/nexassets/" +subDir+File.separator+ orgfile);
            if (tmpfile.exists()) {
                tmpfile.renameTo(file);
            }
        }

        //gen AES key
        byte[] AESkey;
        AESkey = cipher.makeKeyString().getBytes();

        //encrypt AES key using RSA public key

        int size = (int) nexWebview.pubkeyfile.length();
        byte[] bytes = new byte[size];
        try {
            BufferedInputStream buf = new BufferedInputStream(new FileInputStream(nexWebview.pubkeyfile));
            buf.read(bytes, 0, bytes.length);
            buf.close();
            encAESkey = cipher.RSAEncrypt(AESkey, bytes);
        } catch (IOException e) {
            e.printStackTrace();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        } catch (NoSuchProviderException e) {
            e.printStackTrace();
        } catch (NoSuchPaddingException e) {
            e.printStackTrace();
        } catch (InvalidKeyException e) {
            e.printStackTrace();
        } catch (IllegalBlockSizeException e) {
            e.printStackTrace();
        } catch (BadPaddingException e) {
            e.printStackTrace();
        }

        //save encrypted AES key with file
        int end = orgfile.lastIndexOf(".zip");
        String name_ = orgfile.substring(0, end);
        String encAESkeyfilename = "."+name_;

        //File encAESkeyfile = new File(Environment.getExternalStorageDirectory()+"/nexassets/", encAESkeyfilename);
        File encAESkeyfile = null;
        if(filenameStart==0) {
            encAESkeyfile = new File(Environment.getExternalStorageDirectory() + "/nexassets/" + encAESkeyfilename);
        }else{
            encAESkeyfile = new File(Environment.getExternalStorageDirectory() + "/nexassets/" +subDir+File.separator+ encAESkeyfilename);
        }

        int count;
        InputStream input = new ByteArrayInputStream(encAESkey);

        OutputStream output = null;
        try {
            output = new FileOutputStream(encAESkeyfile.getPath());

            byte data[] = new byte[1024];

            long total = 0;
            while ((count = input.read(data)) != -1) {
                total += count;
                output.write(data, 0, count);
            }
            output.flush();
            output.close();
            input.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        //encrypte asset file using AES key
        try {
            if(file.exists()) {
                InputStream inputStream = new FileInputStream(file);
                String encfile = orgfile+".enc";
                File file3=null;
                if(filenameStart==0) {
                    file3 = new File(Environment.getExternalStorageDirectory() + "/nexassets/" + encfile);
                }else{
                    file3 = new File(Environment.getExternalStorageDirectory() + "/nexassets/" +subDir+File.separator+ encfile);
                }
                OutputStream outputStream = new FileOutputStream(file3);

                cipher.encrypt(inputStream, outputStream, AESkey);
                file.delete();
                file3.renameTo(file);
                if (delegate!=null) delegate.onTaskDone(name_);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    public static int getConnectivityStatus(Context context) {
        ConnectivityManager cm = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);

        NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
        if (null != activeNetwork) {
            if (activeNetwork.getType() == ConnectivityManager.TYPE_WIFI)
                return 1;

            if (activeNetwork.getType() == ConnectivityManager.TYPE_MOBILE)
                return 0;
        }
        return -1;
    }

    public int checkNetwork(Context context) {
        int networkstate = getConnectivityStatus(context);
        if (networkstate < 0)
        {
            return -1;
        }
        return networkstate;
    }
    public boolean isOnline() {
        ConnectivityManager cm =
                (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo netInfo = cm.getActiveNetworkInfo();
        return netInfo != null && netInfo.isConnectedOrConnecting();
    }
}