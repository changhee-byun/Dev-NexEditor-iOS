package com.nexstreaming.assetstore.test1;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.net.LocalSocket;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

import com.nexstreaming.nexwebview.ClientHelper;
import com.nexstreaming.nexwebview.ConfigGlobal;
import com.nexstreaming.nexwebview.localProtocol.*;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.SecureRandom;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;

public class AssetStoreService extends Service {

    private final static String TAG = "AssetStoreService";
    private final static String ASSET_STORE_ADDR = "com.nexstreaming.appstore";
    final static int REQUEST_COMMUNICATION_KEY = 1;
    final static int REQUEST_SESSIONID = 2;
    final static int REQUEST_ENCAES_KEY = 3;

    final static short REQUEST_START = 10;
    final static short REQUEST_SEC = 20;
    final static short REQUEST_END = 30;


    //private static byte[] commkey;
    private static int sessionId;
    private String sVendor;
    private static String commkey;
    private static nexCipher cipher = new nexCipher();//nexCipher.getInstance();
    private static Context context;
    ServiceThread thread;
    final static  int CHECK_SERVER_TIME = 3600000;

    public AssetStoreService() {
        Log.d(TAG, "AssetStoreService Start");
    }

    static nexServer.onAcceptCallback serverProcess = new nexServer.onAcceptCallback() {
        @Override
        public void onProcess(LocalSocket sock, nexProtocol.Header hdr) throws IOException, NoSuchPaddingException, NoSuchAlgorithmException, IllegalBlockSizeException, BadPaddingException, NoSuchProviderException, InvalidKeyException {
            byte[] message;
            switch(hdr.command){
                    //get public key from SDK
                case REQUEST_COMMUNICATION_KEY:
                    Log.d(TAG,"REQUEST_COMMUNICATION_KEY");
                    if( hdr.messageSize > 0 ){
                        byte[] encCommkey;
                        message = new byte[hdr.messageSize];
                        nexProtocol.receive(sock,message,message.length);
                        commkey = cipher.makeKeyString();

                        encCommkey = cipher.RSAEncrypt(commkey.getBytes(), message);
                        sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, encCommkey.length));
                        sock.getOutputStream().write(encCommkey);
                    }
                    break;
                case REQUEST_SESSIONID:
                    Log.d(TAG,"REQUEST_SESSIONID");
                    if( hdr.messageSize > 0 ){
                        message = new byte[hdr.messageSize];
                        nexProtocol.receive(sock,message,message.length);
                        String temp = "tttteeeeesssssstttttttt";
                        sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                        sock.getOutputStream().write(temp.getBytes());
                    }
                    break;
                case REQUEST_ENCAES_KEY:
                    Log.d(TAG,"REQUEST_ENCAES_KEY");
                    if( hdr.messageSize > 0 ){
                        message = new byte[hdr.messageSize];
                        nexProtocol.receive(sock,message,message.length);
                        //decrypt AES key using RSA private key
                        String tempPath = new String(message);
                        String subDir = null;
                        String orgfile= null;
                        int filenameStart = tempPath.lastIndexOf('/');
                        if( filenameStart < 0 ){
                            filenameStart = 0;
                            orgfile = tempPath;
                        }else{
                            filenameStart += 1;
                            subDir =  tempPath.substring(0, filenameStart);
                            orgfile= tempPath.substring(filenameStart);
                        }
                        String encAESkeyfile = "."+orgfile;
                        String name_=null;
                        //cho
                        if(encAESkeyfile.endsWith(".zip")){
                            int end = encAESkeyfile.lastIndexOf(".");
                             name_ = encAESkeyfile.substring(0, end);
                        }
                        File encAESkey=null;
                        if(filenameStart==0) {
                            encAESkey = new File(Environment.getExternalStorageDirectory() + "/nexassets/", name_);
                        }else{
                            encAESkey = new File(Environment.getExternalStorageDirectory() + "/nexassets/"+subDir+File.separator, name_);
                        }
                        int size = (int) encAESkey.length();
                        byte[] mEncAESkey = new byte[size];
                        try {
                            BufferedInputStream buf = new BufferedInputStream(new FileInputStream(encAESkey));
                            buf.read(mEncAESkey, 0, mEncAESkey.length);
                            buf.close();
                        } catch (FileNotFoundException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                            String temp = "error";
                            sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                            sock.getOutputStream().write(temp.getBytes());
                        } catch (IOException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                            String temp = "error";
                            sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                            sock.getOutputStream().write(temp.getBytes());
                        }

                        String filename2 = "RSAprivkey";
                        File privKey = new File(context.getFilesDir(), filename2);

                        int size2 = (int) privKey.length();
                        byte[] mPrivKey = new byte[size2];
                        try {
                            BufferedInputStream buf = new BufferedInputStream(new FileInputStream(privKey));
                            buf.read(mPrivKey, 0, mPrivKey.length);
                            buf.close();
                        } catch (FileNotFoundException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                            String temp = "error";
                            sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                            sock.getOutputStream().write(temp.getBytes());
                        } catch (IOException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                            String temp = "error";
                            sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                            sock.getOutputStream().write(temp.getBytes());
                        }
                        byte[] decAESkey = cipher.RSADecrypt(mEncAESkey, mPrivKey);

                        //encrypt AES key using comm key
                        InputStream inputStream = new ByteArrayInputStream(decAESkey);
                        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
                        try {

                            //String AESkeyfilename = "."+new String(message);
                            //File AESkey = new File(Environment.getExternalStorageDirectory()+"/nexassets/", AESkeyfilename);
                            //OutputStream outputStream = null;
                            //outputStream = new FileOutputStream(AESkey);
                            cipher.encrypt(inputStream, byteArrayOutputStream, commkey.getBytes());
                            /*if(AESkey.exists()) {
                                int size3 = (int) AESkey.length();
                                byte[] mAESKey = new byte[size3];
                                try {
                                    BufferedInputStream buf = new BufferedInputStream(new FileInputStream(AESkey));
                                    buf.read(mAESKey, 0, mAESKey.length);
                                    buf.close();
                                    Log.d(TAG, "mAESKey is " + new String(mAESKey));
                                } catch (FileNotFoundException e) {
                                    // TODO Auto-generated catch block
                                    e.printStackTrace();
                                    String temp = "error";
                                    sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                                    sock.getOutputStream().write(temp.getBytes());
                                } catch (IOException e) {
                                    // TODO Auto-generated catch block
                                    e.printStackTrace();
                                    String temp = "error";
                                    sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                                    sock.getOutputStream().write(temp.getBytes());
                                }

                            }*/

                        } catch (Exception e) {
                            e.printStackTrace();
                            String temp = "error";
                            sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                            sock.getOutputStream().write(temp.getBytes());
                        }

                        //sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, 0));
                        //TODO : decrypt AES key use private key and encrypt AES key use comm key
                        sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, byteArrayOutputStream.size()));
                        sock.getOutputStream().write(byteArrayOutputStream.toByteArray());
                    }
                    break;
                default:
                    Log.d(TAG,"no defined command="+hdr.command);
                    String temp = "error";
                    sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                    sock.getOutputStream().write(temp.getBytes());
                    break;
            }
        }
    };

    static nexServer.onAcceptCallback serverProcess2 = new nexServer.onAcceptCallback() {
        @Override
        public void onProcess(LocalSocket sock, nexProtocol.Header hdr) throws IOException, NoSuchPaddingException, NoSuchAlgorithmException, IllegalBlockSizeException, BadPaddingException, NoSuchProviderException, InvalidKeyException {
            byte[] message;
            switch(hdr.command){
                //get public key from SDK
                case REQUEST_START:
                    Log.d(TAG,"REQUEST_Start");

                    if( sessionId != 0 ){
                        //TODO : error
                    }

                    if( hdr.messageSize > 0 ){
                        byte[] encCommkey;
                        message = new byte[hdr.messageSize];
                        nexProtocol.receive(sock,message,message.length);

                        SecureRandom rand = new SecureRandom();
                        int sessionID = rand.nextInt();
                        Log.d(TAG,"sessionID="+sessionID);

                        commkey = cipher.makeKeyString();
                        encCommkey = cipher.RSAEncrypt(commkey.getBytes(), message);

                        sock.getOutputStream().write(nexProtocol.makeResponseHeader((short)hdr.command,0,encCommkey.length+4));
                        ByteBuffer buff = ByteBuffer.allocate(4);
                        Log.d(TAG,"res sessionID="+(sessionID^hdr.randsum));
                        buff.putInt(sessionID ^ hdr.randsum);
                        sock.getOutputStream().write(buff.array());
                        Log.d(TAG, "res encCommkey=" + encCommkey.length);
                        sock.getOutputStream().write(encCommkey);
                        sessionId = sessionID;
                    }
                    break;
                case REQUEST_SEC:
                    Log.d(TAG,"REQUEST_SESSIONID");

                    if( hdr.randsum != sessionId ){
                        Log.d(TAG,"Invalid SSID");
                        sendErrorResponse(sock, hdr.command, nexProtoErrorCode.InvalidSSID, "Invalid SSID");
                        return;
                    }

                    if( hdr.messageSize > 0 ){
                        message = new byte[hdr.messageSize];
                        nexProtocol.receive(sock,message,message.length);

                        String tempPath = new String(message);
                        String subDir = null;
                        String orgfile= null;
                        int filenameStart = tempPath.lastIndexOf('/');
                        if( filenameStart < 0 ){
                            filenameStart = 0;
                            orgfile = tempPath;
                        }else{
                            filenameStart += 1;
                            subDir =  tempPath.substring(0, filenameStart);
                            orgfile= tempPath.substring(filenameStart);
                        }
                        String encAESkeyfile = "."+orgfile;
                        String name_=null;

                        if(encAESkeyfile.endsWith(".zip")){
                            int end = encAESkeyfile.lastIndexOf(".");
                            name_ = encAESkeyfile.substring(0, end);
                        }
                        File encAESkey=null;
                        if(filenameStart==0) {
                            encAESkey = new File(Environment.getExternalStorageDirectory() + "/nexassets/", name_);
                        }else{
                            encAESkey = new File(Environment.getExternalStorageDirectory() + "/nexassets/"+subDir+File.separator, name_);
                        }
                        int size = (int) encAESkey.length();
                        byte[] mEncAESkey = new byte[size];

                        try {
                            BufferedInputStream buf = new BufferedInputStream(new FileInputStream(encAESkey));
                            buf.read(mEncAESkey, 0, mEncAESkey.length);
                            buf.close();
                        } catch (FileNotFoundException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                            String temp = "error";
                            sendErrorResponse(sock, hdr.command, nexProtoErrorCode.PackageNotFound, temp);
                        } catch (IOException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                            String temp = "error";
                            sendErrorResponse(sock,hdr.command, nexProtoErrorCode.IOException, temp);
                        }

                        String filename2 = "RSAprivkey";
                        File privKey = new File(context.getFilesDir(), filename2);

                        int size2 = (int) privKey.length();
                        byte[] mPrivKey = new byte[size2];
                        try {
                            BufferedInputStream buf = new BufferedInputStream(new FileInputStream(privKey));
                            buf.read(mPrivKey, 0, mPrivKey.length);
                            buf.close();
                        } catch (FileNotFoundException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                            String temp = "error";
                            sendErrorResponse(sock, hdr.command, nexProtoErrorCode.PackageNotFound, temp);
                        } catch (IOException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                            String temp = "error";
                            sendErrorResponse(sock, hdr.command, nexProtoErrorCode.IOException, temp);
                        }
                        byte[] decAESkey = cipher.RSADecrypt(mEncAESkey, mPrivKey);

                        InputStream inputStream = new ByteArrayInputStream(decAESkey);
                        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
                        try {

                            //String AESkeyfilename = "."+new String(message);
                            //File AESkey = new File(Environment.getExternalStorageDirectory()+"/nexassets/", AESkeyfilename);
                            //OutputStream outputStream = null;
                            //outputStream = new FileOutputStream(AESkey);
                            cipher.encrypt(inputStream, byteArrayOutputStream, commkey.getBytes());
                            /*if(AESkey.exists()) {
                                int size3 = (int) AESkey.length();
                                byte[] mAESKey = new byte[size3];
                                try {
                                    BufferedInputStream buf = new BufferedInputStream(new FileInputStream(AESkey));
                                    buf.read(mAESKey, 0, mAESKey.length);
                                    buf.close();
                                    Log.d(TAG, "mAESKey is " + new String(mAESKey));
                                } catch (FileNotFoundException e) {
                                    // TODO Auto-generated catch block
                                    e.printStackTrace();
                                    String temp = "error";
                                    sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                                    sock.getOutputStream().write(temp.getBytes());
                                } catch (IOException e) {
                                    // TODO Auto-generated catch block
                                    e.printStackTrace();
                                    String temp = "error";
                                    sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                                    sock.getOutputStream().write(temp.getBytes());
                                }

                            }*/

                        } catch (Exception e) {
                            e.printStackTrace();
                            String temp = "error";
                            //sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, temp.getBytes().length));
                            sendErrorResponse(sock, hdr.command, nexProtoErrorCode.EncFail, temp);
                        }

                        //sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, 0));
                        //TODO : decrypt AES key use private key and encrypt AES key use comm key
                        //sock.getOutputStream().write(nexProtocol.makeSenderHeader(hdr.command, byteArrayOutputStream.size()));
                        sock.getOutputStream().write(nexProtocol.makeResponseHeader((short) hdr.command, 0, byteArrayOutputStream.size()));
                        sock.getOutputStream().write(byteArrayOutputStream.toByteArray());
                    }
                    break;
                case REQUEST_END:
                    Log.d(TAG,"REQUEST_ENCAES_KEY");
                    if( hdr.randsum != sessionId ){
                        Log.d(TAG,"Invalid SSID");
                        sendErrorResponse(sock, hdr.command, nexProtoErrorCode.InvalidSSID, "Invalid SSID");
                        return;
                    }
                    sessionId = 0;
                    sendErrorResponse(sock, hdr.command, nexProtoErrorCode.OK, "Ok");
                    break;
                default:
                    Log.d(TAG, "no defined command=" + hdr.command);
                    String temp = "error";
                    sendErrorResponse(sock,hdr.command, nexProtoErrorCode.InvalidCMD, temp);
                    //sock.getOutputStream().write(nexProtocol.makeResponseHeader((short)hdr.command, nexProtoErrorCode.InvalidCMD.getValue(),temp.getBytes().length));
                    //sock.getOutputStream().write(temp.getBytes());
                    break;
            }
        }
    };

    static nexServer mServer = new nexServer(ASSET_STORE_ADDR,serverProcess2);

    private static void sendErrorResponse(LocalSocket sock, int command, nexProtoErrorCode error , String errorMessage) throws IOException{
        sock.getOutputStream().write(nexProtocol.makeResponseHeader((short)command, error.getValue(),errorMessage.getBytes().length));
        sock.getOutputStream().write(errorMessage.getBytes());
    }


    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        //throw new UnsupportedOperationException("Not yet implemented");
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String vendor = intent.getStringExtra(ConfigGlobal.kAssetStoreVendor);
        if( vendor == null && sVendor == null){
            sVendor = "NexStreaming";
        }

        if(  vendor != null ){
            sVendor = vendor;
        }

        mServer.connect();

        this.context = getApplicationContext();

        if(sVendor == null)
            sVendor ="NexStreaming";

        myServiceHandler handler = new myServiceHandler();
        thread = new ServiceThread(handler,context , sVendor);
        thread.start();
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        mServer.disconnect();
        thread.stopForever();
        thread = null;
        super.onDestroy();
    }

    //check featured list on looping
    public class ServiceThread extends Thread{
        Handler handler;
        Context context;
        String sVendor;
        boolean isRun = true;

        public ServiceThread(Handler handler ,Context context, String vendor){

            this.handler = handler;
            this.context = context;
            this.sVendor = vendor;
        }

        public void stopForever(){
            synchronized (this) {
                this.isRun = false;
            }
        }

        public void run(){

            while(isRun){
                boolean isDownload = false;
                //check featuredlist folder
//                isDownload = ResourceDn.getFeaturedList();
                ClientHelper FeaturedList = new ClientHelper("1234",sVendor);
                FeaturedList.login();
                FeaturedList.getNewAssetList("1", ConfigGlobal.getAssetStoreRootDirectory().getAbsolutePath());
                Log.d(TAG,"download service thread");
                if(isDownload){
                    handler.sendEmptyMessage(0);
                }else {
                    handler.sendEmptyMessage(2);
                }
                try{
                    Thread.sleep(CHECK_SERVER_TIME);
                }catch (Exception e) {}
            }
        }
    }
    class myServiceHandler extends Handler {
        @Override
        public void handleMessage(android.os.Message msg) {
            //download start when get message what = 2
            Log.d(TAG, "service is running:"+msg.what);
            if(msg.what == 2){
                //download getfeaturedlist
            }

        }
    };
}
