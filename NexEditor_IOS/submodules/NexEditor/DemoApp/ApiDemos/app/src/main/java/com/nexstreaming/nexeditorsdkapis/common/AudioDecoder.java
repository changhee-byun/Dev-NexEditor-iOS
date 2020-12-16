package com.nexstreaming.nexeditorsdkapis.common;

import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Created by jeongwook.yoon on 2018-04-24.
 */

public class AudioDecoder {
    private final static String TAG = "AudioDecoder";
    private MediaExtractor mExtractor;
    private MediaCodec mDecoder;
    private static final int TIMEOUT_US = 1000;
    private boolean eosReceived;
    private int mSampleRate = 0;
    private int mChannelCount = 0;
    private long mTotalDurationMs = 0;

    private String mMimeType;
    private OnReceivedDataCallback mCallback;
    private int state = kState_Of_Idle;
    private static final ExecutorService sInstallThreadExcutor = Executors.newSingleThreadExecutor();
    public final static int kState_Of_Idle = 0;
    public final static int kState_Of_SetDataSourced = 1;
    public final static int kState_Of_Prepared = 2;
    public final static int kState_Of_Decoding = 3;

    public interface OnReceivedDataCallback{
        boolean onReceivedData( byte[] rawData);
        void onEos(AudioDecoder ad);
        void onCancel(AudioDecoder ad);
    }

    public int getSampleRate() {
        return mSampleRate;
    }

    public int getChannelCount() {
        return mChannelCount;
    }

    public long getTotalDurationMs() {
        return mTotalDurationMs;
    }

    public String getMimeType() {
        return mMimeType;
    }

    public int getMaxSize(){
        return mSampleRate * 2 * mChannelCount * (int)(mTotalDurationMs/1000 +1);
    }

    public void setDataSource(String path){
        mExtractor = new MediaExtractor();
        try {
            mExtractor.setDataSource(path);
            state = kState_Of_SetDataSourced;
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void setOnReceivedDataCallback(OnReceivedDataCallback callback){
        mCallback = callback;
    }

    public boolean prepare(){
        if( state != kState_Of_SetDataSourced ){
            return false;
        }

        eosReceived = false;

        for (int i = 0; i < mExtractor.getTrackCount(); i++) {
            MediaFormat format = mExtractor.getTrackFormat(i);
            mMimeType = format.getString(MediaFormat.KEY_MIME);
            mTotalDurationMs = format.getLong(MediaFormat.KEY_DURATION)/1000 +1;
            if (mMimeType.startsWith("audio/")) {
                mExtractor.selectTrack(i);

                Log.d(TAG, "format : " + format);
                mSampleRate = format.getInteger(MediaFormat.KEY_SAMPLE_RATE);
                mChannelCount = format.getInteger(MediaFormat.KEY_CHANNEL_COUNT);

                try {
                    mDecoder = MediaCodec.createDecoderByType(mMimeType);
                } catch (IOException e) {
                    e.printStackTrace();
                }
                mDecoder.configure(format, null, null, 0);
                break;
            }
        }

        if (mDecoder == null) {
            Log.e(TAG, "Can't find audio info!");
            return false;
        }
        state = kState_Of_Prepared;
        return true;
    }

    public void release(){
        if( mDecoder != null ) {
            mDecoder.release();
            mDecoder = null;
        }

        if( mExtractor != null ) {
            mExtractor.release();
            mExtractor = null;
        }
    }

    private  final class AudioDecProcess extends AsyncTask<AudioDecoder,Void,Integer>{
        private  int num;
        private AudioDecoder ad;
        private OnReceivedDataCallback callback;
        AudioDecProcess(int num, AudioDecoder ad, OnReceivedDataCallback callback){
            this.num = num;
            this.ad = ad;
            this.callback = callback;
        }

        @Override
        protected Integer doInBackground(AudioDecoder... params) {
            int rval = Decoder();
            return rval;
        }

        @Override
        protected void onPostExecute(Integer integer) {
            if( integer.intValue() == 1) {
                if (callback != null) {
                    callback.onCancel(ad);
                }
            }else{
                if (callback != null) {
                    callback.onEos(ad);
                }
            }

            super.onPostExecute(integer);
        }


    }

    public void decode(){
        if( state != kState_Of_Prepared ){
            return;
        }
        state = kState_Of_Decoding;
        mDecoder.start();
        new AsyncTask<AudioDecoder,Void,Integer>(){
            AudioDecoder target;
            @Override
            protected Integer doInBackground(AudioDecoder... params) {
                target = params[0];
                int rval = Decoder();
                return rval;
            }

            @Override
            protected void onPostExecute(Integer integer) {
                if( integer.intValue() == 1) {
                    if (mCallback != null) {
                        mCallback.onCancel(target);
                    }
                }else{
                    if (mCallback != null) {
                        mCallback.onEos(target);
                    }
                }

                super.onPostExecute(integer);
            }
        }.executeOnExecutor(sInstallThreadExcutor,this);
    }

    private int Decoder() {
        ByteBuffer[] inputBuffers = mDecoder.getInputBuffers();
        ByteBuffer[] outputBuffers = mDecoder.getOutputBuffers();
        mExtractor.seekTo(0,MediaExtractor.SEEK_TO_PREVIOUS_SYNC);
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        boolean cancel = false;
        while (!eosReceived) {
            int inIndex = mDecoder.dequeueInputBuffer(TIMEOUT_US);
            if (inIndex >= 0) {
                ByteBuffer buffer = inputBuffers[inIndex];
                int sampleSize = mExtractor.readSampleData(buffer, 0);
                if (sampleSize < 0) {
                    Log.d(TAG, "InputBuffer BUFFER_FLAG_END_OF_STREAM");
                    mDecoder.queueInputBuffer(inIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                    eosReceived = true;
                } else {
                    mDecoder.queueInputBuffer(inIndex, 0, sampleSize, mExtractor.getSampleTime(), 0);
                    mExtractor.advance();
                }

                int outIndex = mDecoder.dequeueOutputBuffer(info, TIMEOUT_US);
                switch (outIndex) {
                    case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                        Log.d(TAG, "INFO_OUTPUT_BUFFERS_CHANGED");
                        outputBuffers = mDecoder.getOutputBuffers();
                        break;

                    case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                        MediaFormat format = mDecoder.getOutputFormat();
                        Log.d(TAG, "New format " + format);

                        break;
                    case MediaCodec.INFO_TRY_AGAIN_LATER:
                        Log.d(TAG, "dequeueOutputBuffer timed out!");
                        break;

                    default:
                        ByteBuffer outBuffer = outputBuffers[outIndex];

                        final byte[] chunk = new byte[info.size];
                        outBuffer.get(chunk);
                        outBuffer.clear();

                        if( mCallback != null ) {
                            cancel = mCallback.onReceivedData(chunk);
                        }
                        mDecoder.releaseOutputBuffer(outIndex, false);
                        break;
                }

                // All decoded frames have been rendered, we can stop playing now
                if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    Log.d(TAG, "OutputBuffer BUFFER_FLAG_END_OF_STREAM");
                    break;
                }
            }
        }

        if( cancel ){
            mDecoder.flush();
        }

        mDecoder.stop();
        if( cancel ) {
            return 1;
        }
        state = kState_Of_Prepared;

        return 0;
    }
}
