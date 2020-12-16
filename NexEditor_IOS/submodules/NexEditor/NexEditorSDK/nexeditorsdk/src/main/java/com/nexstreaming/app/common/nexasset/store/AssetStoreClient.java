package com.nexstreaming.app.common.nexasset.store;

import android.app.ActivityManager;
import android.os.AsyncTask;
import android.os.Environment;
import android.util.Log;

import com.nexstreaming.app.common.localprotocol.nexCipher;
import com.nexstreaming.app.common.localprotocol.nexProtoErrorCode;
import com.nexstreaming.app.common.localprotocol.nexProtocol;
import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.SecureRandom;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;

/**
 * Created by jeongwook.yoon on 2016-10-18.
 */
public class AssetStoreClient {
    final static private String TAG = "AssetStoreClient";
    final static public String none = "nonedata";
    static private nexCipher cipher;
    private static Object m_configLock = new Object();

    public static void makeConfig() {
        Log.d(TAG,"makeConfig in");
        synchronized (m_configLock) {
            if (cipher == null) {
                cipher = new nexCipher();
                cipher.fastRSAGenKey();
            }
        }
        Log.d(TAG,"makeConfig out");
    }

    static int getDataSync2(String[] list, String[] rets, String packageName){
        if( cipher == null ){
            return -1;
        }
        byte[] commkey= null;


        SecureRandom rand = new SecureRandom();
        int seed = rand.nextInt();
        nexProtocol.HDRnData start = AssetStoreSock.Start(cipher.getRSApubKey(),seed );
        if(start == null){
            Log.d(TAG, "start is null");
            return -1;
        }

        if( start.errorCode != nexProtoErrorCode.OK.getValue() ){
            Log.d(TAG, "start is error="+start.errorCode);
            return -1;
        }

        final ByteBuffer startData = ByteBuffer.wrap(start.data);

        int newSSID = startData.getInt() ^ seed;
        Log.d(TAG, "newSSID=" + newSSID);

        byte[] encCommkey = new byte[start.data.length-4];
        startData.get(encCommkey);
        Log.d(TAG, "encCommkey=" + encCommkey.length);
        try {
            commkey = cipher.fastRSADecrypt(encCommkey);
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

        for( int i = 0 ; i < list.length ; i++ ) {

            nexProtocol.HDRnData sec = AssetStoreSock.requestSECUseCommKey(newSSID,packageName+File.separator+list[i]);

            if( sec == null ){
                rets[i] = none;
                continue;
            }

            if( sec.errorCode != nexProtoErrorCode.OK.getValue() ){
                Log.d(TAG, "enckey is fail!");
                return -1;
            }

            try {
                byte[] AESkey = nexCipher.decrypt(sec.data,commkey);
                rets[i] = new String(AESkey);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return 0;
    }


}
