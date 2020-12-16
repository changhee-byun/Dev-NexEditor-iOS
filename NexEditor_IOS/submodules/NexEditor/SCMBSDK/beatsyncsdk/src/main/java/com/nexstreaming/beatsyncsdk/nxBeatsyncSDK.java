package com.nexstreaming.beatsyncsdk;


import android.os.AsyncTask;

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Created by jeongwook.yoon on 2018-09-14.
 */

public class nxBeatsyncSDK {
    private int handle = -1;
    private int [][] result = new int[3][];
    private int error = 0;

    private int channel = 2;
    private int samplingRate = 441000;
    private int bitPerSample = 16;

    public static final int Beat = 0;
    public static final int Transition = 1;
    public static final int Effect = 2;

    private static final ExecutorService sInstallThreadExcutor = Executors.newSingleThreadExecutor();
    private native int createHandle(int channel, int sampling_rate, int bits_per_sample);
    private native int destroyHandle(int handle);
    private native int pushPCM(int handle,int flags,  byte [] pcmBlock);
    private native int processFind(int handle);
    private native int[] popResult(int handle,int cmd);

    public int getChannel() {
        return channel;
    }

    public int getSamplingRate() {
        return samplingRate;
    }

    public int getBitPerSample() {
        return bitPerSample;
    }

    public interface OnCompleteListener{
        void onComplete( int errorNo);
        void onProgress(int percent);
        void onCancel();
    }

    private nxBeatsyncSDK(int channel, int sampling_rate, int bits_per_sample){
        handle = createHandle( channel,  sampling_rate,  bits_per_sample);
    }

    public static nxBeatsyncSDK createInstance(int channel, int sampling_rate, int bits_per_sample){
        nxBeatsyncSDK ins = new nxBeatsyncSDK(channel,sampling_rate,bits_per_sample);
        if( ins.handle < 0 ){
            return null;
        }
        ins.channel = channel;
        ins.samplingRate = sampling_rate;
        ins.bitPerSample = bits_per_sample;
        return ins;
    }

    public void destroyInstance(){
        destroyHandle(handle);
        handle = -1;
    }

    public int pushData(byte[] pcmblock) {
        error = pushPCM(handle,0, pcmblock);
        return error;
    }

    public int pushEos(){
        error = pushPCM(handle,1, new  byte[0]);
        return error;
    }

    public int process(){
        processFind(handle);
        for( int i = 0 ; i < 3 ; i++){
            int[] res = popResult(handle,i);

            if( res == null ){
                error = -1;
                continue;
            }

            result[i] = new int[res.length];
            for(int j = 0 ; j < res.length ; j++ ){
                result[i][j] = res[j];
            }
        }
        return 0;
    }

    public int processAsync(final OnCompleteListener listener){
        new AsyncTask<nxBeatsyncSDK,Void,Integer>(){
            nxBeatsyncSDK target;
            @Override
            protected Integer doInBackground(nxBeatsyncSDK... params) {
                target = params[0];
                return processFind(handle);
            }

            @Override
            protected void onPostExecute(Integer integer) {
                for( int i = 0 ; i < 3 ; i++){
                    int[] res = popResult(handle,i);

                    if( res == null ){
                        error = -1;
                        continue;
                    }

                    result[i] = new int[res.length];
                    for(int j = 0 ; j < res.length ; j++ ){
                        result[i][j] = res[j];
                    }
                }
                if (listener != null) {
                    listener.onComplete(0);
                }
                super.onPostExecute(integer);
            }
        }.executeOnExecutor(sInstallThreadExcutor,this);
        return 0;
    }


    public int[] getResultSampleUnit(int cmd){
        return  result[cmd];
    }

    public int sampleToTimeUnit(int samplesCount){
        //return (int)((((float)samplesCount / (float)channel) * 1000f) / samplingRate);
        return  (int)(((float)samplesCount/(float)samplingRate)*1000f);
    }

    public int getLastErrorNumber(){
        return error;
    }

    public static short bytes2short(byte[] data, int position){
        if( data.length <= position ){
            return 0;
        }
        return ByteBuffer.wrap(new byte[]{data[position], data[position+1]}).getShort();
    }

    static {
        System.loadLibrary("nxBeatSyncjni");
    }
}
