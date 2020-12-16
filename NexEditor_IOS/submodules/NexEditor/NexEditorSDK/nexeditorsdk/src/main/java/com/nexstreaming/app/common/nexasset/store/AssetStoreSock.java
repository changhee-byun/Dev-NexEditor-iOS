package com.nexstreaming.app.common.nexasset.store;

import com.nexstreaming.app.common.localprotocol.nexClient;
import com.nexstreaming.app.common.localprotocol.nexProtocol;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.security.SecureRandom;

/**
 * Created by jeongwook.yoon on 2016-10-18.
 */
public class AssetStoreSock {
    private final static String ASSET_STORE_ADDR = "com.nexstreaming.appstore";
    final static private String TAG = "AssetStoreSock";
    final static int REQUEST_COMMUNICATION_KEY = 1;
    final static int REQUEST_SESSIONID = 2;
    final static int REQUEST_ENCAES_KEY = 3;
    final static short REQUEST_START = 10;
    final static short REQUEST_SEC = 20;
    final static short REQUEST_END = 30;

    public static nexProtocol.HDRnData Start(byte[] pubKey, int randSSID){
        nexClient client = new nexClient(ASSET_STORE_ADDR);
        nexProtocol.HDRnData data = null;
        if( client.connect() ){
            try {
                client.sendReqMessage(REQUEST_START, randSSID, pubKey);
                data = client.receive(REQUEST_START, randSSID, false);
            } catch (IOException e) {
                e.printStackTrace();
            }
            client.disconnect();
        }
        return data;
    }

    public static nexProtocol.HDRnData requestSECUseCommKey(int SSID, String path){
        nexClient client = new nexClient(ASSET_STORE_ADDR);
        nexProtocol.HDRnData data = null;
        if( client.connect() ){
            try {
                client.sendReqMessage(REQUEST_SEC,SSID, path.getBytes());
                data = client.receive(REQUEST_SEC,SSID,false);
            } catch (IOException e) {
                e.printStackTrace();
            }
            client.disconnect();
        }
        return data;
    }

    public static int End(int SSID){
        nexClient client = new nexClient(ASSET_STORE_ADDR);
        nexProtocol.HDRnData data = null;
        if( client.connect() ){
            try {
                client.sendReqMessage(REQUEST_END, SSID, "bye".getBytes());
                data = client.receive(REQUEST_END, SSID, false);
            } catch (IOException e) {
                e.printStackTrace();
            }
            client.disconnect();
        }
        if( data == null ){
            return -1;
        }
        return data.errorCode;
    }

}
