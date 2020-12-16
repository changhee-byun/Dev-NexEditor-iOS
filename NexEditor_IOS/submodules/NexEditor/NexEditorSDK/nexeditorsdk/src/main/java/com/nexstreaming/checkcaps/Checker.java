package com.nexstreaming.checkcaps;

import android.annotation.TargetApi;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.util.Log;
import android.view.Surface;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.AbstractMap;
import java.util.HashMap;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Created by sunghyun.eric.yoo on 2015-08-04.
 */

public class Checker {

    public static final int RET_VALUE_SUCCESS = 0;
    private static String LOGTAG = "CapChecker";
    private static final String AVC_MIME_TYPE = "video/avc"; // H.264
    private static final int TIMEOUT_USEC = 1000;
    private static final int INTERNAL_COMMAND_ENCODER_AVAILABILITY = 1;


    private boolean mInfo2log = false;
    //private TextView mInfoView = null;
    private InputSurface mInputSurface = null;
    private CheckerEventListener mListener;
    private MediaCodec mVideoEncoder = null;
    private MediaCodec.BufferInfo mVideoEncoderOutputBufferInfo = null;
    private MediaFormat mOutputVideoFormat = null;
    private ByteBuffer[] mVideoEncoderOutputBuffers = null;

    public interface CheckerEventListener {
        void onCheckerCapsInfoAvailable(Checker checker, String msg);
        void onCheckerCapsResult(Checker checker, int nResult);
    }

    public void setCheckerEventListener(CheckerEventListener listener) {
        mListener = listener;
    }

    private static int GetEncoderCount(String mimeType) {
        int numCodecs = MediaCodecList.getCodecCount();
        int numReturnCount = 0;

        for (int i = 0; i < numCodecs; i++) {
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);

            if (!codecInfo.isEncoder()) {
                continue;
            }

            String[] types = codecInfo.getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equalsIgnoreCase(mimeType)) {
                    numReturnCount++;
                    break;
                }
            }
        }

        return numReturnCount;
    }

    private static MediaCodecInfo[] GetEncoderInfos(String mimeType) {
        MediaCodecInfo[] CodecInfos4Ret = null;
        int nEncoderCount = GetEncoderCount(mimeType);

        CodecInfos4Ret = new MediaCodecInfo[nEncoderCount];
        if ( 0 == nEncoderCount ) {
            return CodecInfos4Ret;
        }

        int numCodecs = MediaCodecList.getCodecCount();
        int nCount = 0;

        for (int i = 0; i < numCodecs; i++) {
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);

            //Log.d(LOGTAG,i+" th name:"+codecInfo.getName()+"encoder:"+codecInfo.isEncoder());
            if (!codecInfo.isEncoder()) {
                continue;
            }

            String[] types = codecInfo.getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equalsIgnoreCase(mimeType)) {
/*
                    CodecCapabilities codecCapabilities = codecInfo.getCapabilitiesForType(mimeType);
                    MediaCodecInfo.EncoderCapabilities encoderCapabilities = codecCapabilities.getEncoderCapabilities();
                    boolean isCBR =  encoderCapabilities.isBitrateModeSupported(MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CBR);
                    boolean iVBR =  encoderCapabilities.isBitrateModeSupported(MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_VBR);
                    boolean iCQ =  encoderCapabilities.isBitrateModeSupported(MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_CQ);

                    Range<Integer> complexityRange = encoderCapabilities.getComplexityRange(); // KEY_COMPLEXITY
*/
                    CodecInfos4Ret[nCount++] = codecInfo;
                    break;
                }
            }
        }
        return CodecInfos4Ret;
    }

    @TargetApi(21)
    private MediaCodec createVideoEncoder(
            MediaCodecInfo codecInfo,
            MediaFormat format,
            AtomicReference<Surface> surfaceReference) {
        MediaCodec encoder = null;
        boolean ErrorOccurred = false;
        try {
            encoder = MediaCodec.createByCodecName(codecInfo.getName());
        } catch (IOException e) {
            e.printStackTrace();
            ErrorOccurred = true;
        }

        if ( ErrorOccurred )
            return null;

        try {
            encoder.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        } catch (MediaCodec.CodecException e) {
            // over Api21,
            // Insufficient resource
            ErrorOccurred = true;
        } catch (IllegalArgumentException|IllegalStateException e) {
            e.printStackTrace();
            ErrorOccurred = true;
        }

        if ( ErrorOccurred )
            return null;

        // Must be called before start() is.
        surfaceReference.set(encoder.createInputSurface());
        encoder.start();
        return encoder;
    }

    public Checker(boolean isInfo2log)
    {
        mInfo2log = isInfo2log;
        //mInfoView = infoView;
    }

    private static class CheckerRunningThread extends Thread {
        private Checker mChecker = null;
        private AbstractMap<String,Object> mParameters = null;

        private CheckerRunningThread(Checker checker, AbstractMap<String,Object> attributes) {
            mChecker = checker;
            mParameters = attributes;
        }

        @Override
        public void run() {

            if ( null == mParameters )
                return;

            int cmd = (int)mParameters.get("command");

            switch( cmd )
            {
                case INTERNAL_COMMAND_ENCODER_AVAILABILITY :
                    int width = (int)mParameters.get("width");
                    int height = (int)mParameters.get("height");
                    int nRet = mChecker._CheckEncoderAvailability(width, height);
                    mChecker.mListener.onCheckerCapsResult(mChecker, nRet);
                    break;
            }
            //mParameters.get
            //try {

            //    mChecker.Test1();
            //} catch (Throwable th) {
            //    mThrowable = th;
            //}
        }

        public static void runChecker(Checker checker, AbstractMap<String,Object> attributes) {//throws Throwable {
            CheckerRunningThread thread = new CheckerRunningThread(checker, attributes );
            thread.start();
            //Thread th = new Thread(wrapper, "codec test");
            //th.start();
        }
    }

    public void CheckEncoderAvailability(int width, int height) {
        AbstractMap<String,Object> att = new HashMap<String,Object>();

        att.put("command",INTERNAL_COMMAND_ENCODER_AVAILABILITY);
        att.put("width",width);
        att.put("height",height);
        CheckerRunningThread.runChecker(this,att);
    }

    public void CheckEncodingOnly() {
        //CheckerRunningThread.runChecker(this);
    }

    @TargetApi(18)
    private int _CheckEncoderAvailability(int width, int height) {
        MediaCodecInfo[] codecInfos = GetEncoderInfos(AVC_MIME_TYPE);
        if (null != mListener) {
            mListener.onCheckerCapsInfoAvailable(this, "The count of '" + AVC_MIME_TYPE + "' Encoder : " + codecInfos.length);
            for (int i = 0; i < codecInfos.length; i++) {
                mListener.onCheckerCapsInfoAvailable(this, i + " th encoder's name is '" + codecInfos[i].getName() + "'");
            }
        }

        MediaFormat outputVideoFormat = MediaFormat.createVideoFormat(AVC_MIME_TYPE, width, height);
        outputVideoFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        outputVideoFormat.setInteger(MediaFormat.KEY_BIT_RATE, 3000000); // 3Mbps
        outputVideoFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 30); //30fps
        outputVideoFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2); //every 2 secs

        if (null != mListener) {
            mListener.onCheckerCapsInfoAvailable(this, "Video Format of Encoder : " + outputVideoFormat);
        }

        AtomicReference<Surface> inputSurfaceFromEncoderReference = new AtomicReference<Surface>();
        mVideoEncoder = createVideoEncoder(codecInfos[0], outputVideoFormat, inputSurfaceFromEncoderReference);

        if (mVideoEncoder != null) {
            mInputSurface = new InputSurface(inputSurfaceFromEncoderReference.get());
            mInputSurface.makeCurrent();

            mVideoEncoder.stop();
            mVideoEncoder.release();
            mVideoEncoder = null;

            mInputSurface.release();
            mInputSurface = null;
            return 0;
        }
        else
            return -1;
    }

    @TargetApi(18)
    public void Test1(int width, int height)
    {
        MediaCodecInfo[] codecInfos = GetEncoderInfos(AVC_MIME_TYPE);
        if ( null != mListener )
        {
            mListener.onCheckerCapsInfoAvailable(this, "The count of '" + AVC_MIME_TYPE + "' Encoder : " + codecInfos.length);
            for ( int i = 0 ; i < codecInfos.length ; i++ )
            {
                mListener.onCheckerCapsInfoAvailable(this, i + " th encoder's name is '" + codecInfos[i].getName() + "'");
            }
        }

        MediaFormat outputVideoFormat = MediaFormat.createVideoFormat(AVC_MIME_TYPE, width, height);
        outputVideoFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        outputVideoFormat.setInteger(MediaFormat.KEY_BIT_RATE, 3000000); // 3Mbps
        outputVideoFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 30); //30fps
        outputVideoFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2); //every 2 secs

        if ( null != mListener )
        {
            mListener.onCheckerCapsInfoAvailable(this, "Video Format of Encoder : " + outputVideoFormat);
        }

        AtomicReference<Surface> inputSurfaceFromEncoderReference = new AtomicReference<Surface>();
        mVideoEncoder = createVideoEncoder(codecInfos[0],outputVideoFormat,inputSurfaceFromEncoderReference);

        if (mVideoEncoder != null) {
            mVideoEncoder.stop();
            mVideoEncoder.release();
            mVideoEncoder = null;
            Log.e("er","createSuccess1");
        }
        else
        {
            Log.e("er","createError1");
        }

        mVideoEncoder = createVideoEncoder(codecInfos[0],outputVideoFormat,inputSurfaceFromEncoderReference);
        if (mVideoEncoder != null) {
            mVideoEncoder.stop();
            mVideoEncoder.release();
            mVideoEncoder = null;
            Log.e("er","createSuccess1");
        }
        else
        {
            Log.e("er","createError2");
        }

        if ( true )
            return;

        mInputSurface = new InputSurface(inputSurfaceFromEncoderReference.get());
        mInputSurface.makeCurrent();
        /* all job should be done in the same thread.*/
//mVideoEncoder.get
        mVideoEncoderOutputBuffers = mVideoEncoder.getOutputBuffers();

        long presentationTimeNs = 0;

        //if (VERBOSE) Log.d(LOG_TAG, "video encoder: notified of new frame");

        mVideoEncoderOutputBufferInfo = new MediaCodec.BufferInfo();
        boolean videoEncoderDone = false;

        int VideoEncoderInputCount = 0;
        int targetVideoFrameCount = 15;

        ByteBuffer[] InputBufferArray = mVideoEncoder.getInputBuffers();
        int cachedInputBufferCount = InputBufferArray.length;
        InputBufferArray = null;

        //ByteBuffer[] videoFrameBuffer;
        //videoFrameBuffer = new ByteBuffer[targetVideoFrameCount];
        //videoFrameBuffer[0] = ByteBuffer.allocate(10);

        mOutputVideoFormat = null;
//        else if ( i == 10 )
//        mVideoEncoder.signalEndOfInputStream();

        for ( int i = 0 ; false == videoEncoderDone ; i++ ) {

            if ( VideoEncoderInputCount < targetVideoFrameCount ) {
                presentationTimeNs = 33333333 * VideoEncoderInputCount;
                mInputSurface.setPresentationTime(presentationTimeNs);
                boolean swapResult = mInputSurface.swapBuffers();
                Log.d(LOGTAG, "swapResult : " + swapResult);
                VideoEncoderInputCount++;
            }
            else if ( VideoEncoderInputCount == targetVideoFrameCount )
            {
                Log.d(LOGTAG, "signalEnd");
                mVideoEncoder.signalEndOfInputStream();
                VideoEncoderInputCount++;
            }


            int encoderOutputBufferIndex = mVideoEncoder.dequeueOutputBuffer(mVideoEncoderOutputBufferInfo, TIMEOUT_USEC * 100);

            Log.d(LOGTAG,"encoderOutputBufferIndex : "+encoderOutputBufferIndex);

            switch (encoderOutputBufferIndex) {
                case MediaCodec.INFO_TRY_AGAIN_LATER:
                    break;
                case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                    mVideoEncoderOutputBuffers = mVideoEncoder.getOutputBuffers();
                    break;
                case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                    if ( null != mOutputVideoFormat )
                    {
                        Log.e(LOGTAG,"INFO_OUTPUT_FORMAT_CHANGED happens one more time.");
                    }
                    mOutputVideoFormat = mVideoEncoder.getOutputFormat();
                    Log.d(LOGTAG, "mOutputVideoFormat = " + mOutputVideoFormat);
                    break;
                default:
                    break;
            }

            if (encoderOutputBufferIndex < 0)
                continue;

            Log.d(LOGTAG, "video encoder: returned output buffer(idx,size,offset,flags,time): ("
                    + encoderOutputBufferIndex + ", "
                    + mVideoEncoderOutputBufferInfo.size + ", "
                    + mVideoEncoderOutputBufferInfo.offset + ", "
                    + mVideoEncoderOutputBufferInfo.flags + ", "
                    + mVideoEncoderOutputBufferInfo.presentationTimeUs + ")");

            if ((mVideoEncoderOutputBufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                Log.d(LOGTAG, "video encoder: codec config buffer");
                // Simply ignore codec config buffers.
                //videoFrameBuffer[0] = ByteBuffer.allocate(mVideoEncoderOutputBufferInfo.size);
                mVideoEncoder.releaseOutputBuffer(encoderOutputBufferIndex, false);
                continue;
            }

            if ((mVideoEncoderOutputBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                Log.d(LOGTAG, "video encoder: EOS");
                videoEncoderDone = true;
            }
            else if ( i == 10 )
                videoEncoderDone = true;

            mVideoEncoder.releaseOutputBuffer(encoderOutputBufferIndex, false);
        }
        int m =10;
        m++;
    }

    void Test2() {

    }
}
