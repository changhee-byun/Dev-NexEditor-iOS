package com.nexstreaming.capability.tasks;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.opengl.GLES20;
import android.os.AsyncTask;
import android.os.SystemClock;
import android.view.Surface;

import com.nexstreaming.capability.util.CBLog;
import com.nexstreaming.capability.util.InputSurface;
import com.nexstreaming.capability.util.Util;

import java.io.IOException;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Created by Eric on 2015-10-07.
 */
public class EncoderExecutorTask {

    final String TAG = "EncoderTask";

    public interface EncoderListener {
        void onEncoderListener(String message);
    }

    EncoderListener listener;

    ecInputThread encoderInputThread;
    ecOutputThread encoderOutputThread;

    InputSurface surface;
    AtomicReference<Surface> surfaceReference = new AtomicReference<Surface>();

    int width;
    int height;
    int taskId;

    public EncoderExecutorTask(int taskId, int width, int height, EncoderListener listener) {
        this.taskId = taskId;
        this.width = width;
        this.height = height;
        this.listener = listener;

        encoderInputThread = new ecInputThread();
        encoderOutputThread = new ecOutputThread();
    }

    public AsyncTask.Status getThreadStatus() {
        if(encoderInputThread != null) {
            return encoderInputThread.getStatus();
        } else {
            return AsyncTask.Status.PENDING;
        }
    }

    public void cancelThread(boolean mayInterruptIfRunning) {
        CBLog.d(taskId + "/Encoder]cancelEncoderTask");
        if(encoderInputThread != null) {
            encoderInputThread.cancel(mayInterruptIfRunning);
        }
    }

    public void releaseEncoder() {
        if(surface != null) {
            surface.release();
            surface = null;
        }
        try {
            if(encoder != null) {
                if(Util.SDK_INIT > 20) {
                    if(flag_codecError) {
                        encoder.reset();
                        encoder.stop();
                    } else {
                        encoder.stop();
                    }
                } else {
                    encoder.stop();
                }
            }
        } finally {
            try {
                if(encoder != null) {
                    encoder.release();
                }
            } finally {
                CBLog.d(taskId + "/Encoder]releaseEncoder, encoder=" + encoder);
                encoder = null;
            }
        }
    }

    MediaCodecInfo getCodecInfo(String mimeType) {
        if(Util.SDK_INIT > 20) {
            MediaCodecList codecList = new MediaCodecList(MediaCodecList.REGULAR_CODECS);
            for(MediaCodecInfo info : codecList.getCodecInfos()) {

                if(!info.isEncoder()) {
                    continue;
                }

                String[] types = info.getSupportedTypes();
                for (int j = 0; j < types.length; j++) {
                    if (types[j].equalsIgnoreCase(mimeType)) {
                        return info;
                    }
                }
            }
        } else {
            int numCodec = MediaCodecList.getCodecCount();
            for(int i=0; i<numCodec; i++) {
                MediaCodecInfo info = MediaCodecList.getCodecInfoAt(i);

                if(!info.isEncoder()) {
                    continue;
                }

                String[] types = info.getSupportedTypes();
                for (int j = 0; j < types.length; j++) {
                    if (types[j].equalsIgnoreCase(mimeType)) {
                        return info;
                    }
                }
            }
        }
        return  null;
    }

    public boolean createEncoder() {
        CBLog.d(taskId + "/Encoder]createEncoder");

        MediaFormat videoFormat = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, width, height);
        videoFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        videoFormat.setInteger(MediaFormat.KEY_BIT_RATE, 3000000);
        videoFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
        videoFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2);
//        videoFormat.setInteger(MediaFormat.KEY_BITRATE_MODE, MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_VBR);
//        videoFormat.setInteger(MediaFormat.KEY_LEVEL, MediaCodecInfo.CodecProfileLevel.AVCLevel52);
//        videoFormat.setInteger(MediaFormat.KEY_PROFILE, MediaCodecInfo.CodecProfileLevel.AVCProfileHigh);

        MediaCodecInfo codecInfo = getCodecInfo(MediaFormat.MIMETYPE_VIDEO_AVC);
        if(codecInfo == null) {
            CBLog.d(taskId + "/Encoder] \"fail getCodecInfo<video/avc>\"");
            return false;
        }

//        CBLog.d("getComplexityRange : " + codecInfo.getCapabilitiesForType(MediaFormat.MIMETYPE_VIDEO_AVC).getEncoderCapabilities().getComplexityRange());
//        CBLog.d("isBitrateModeSupported(BITRATE_MODE_VBR) : "
//                + codecInfo.getCapabilitiesForType(MediaFormat.MIMETYPE_VIDEO_AVC).getEncoderCapabilities().isBitrateModeSupported(EncoderCapabilities.BITRATE_MODE_VBR));
//        CBLog.d("isBitrateModeSupported(BITRATE_MODE_CBR) : "
//                + codecInfo.getCapabilitiesForType(MediaFormat.MIMETYPE_VIDEO_AVC).getEncoderCapabilities().isBitrateModeSupported(EncoderCapabilities.BITRATE_MODE_CBR));
//        CBLog.d("isBitrateModeSupported(BITRATE_MODE_CQ) : "
//                + codecInfo.getCapabilitiesForType(MediaFormat.MIMETYPE_VIDEO_AVC).getEncoderCapabilities().isBitrateModeSupported(EncoderCapabilities.BITRATE_MODE_CQ));

        try {

            long codecInitializingTimestamp = SystemClock.elapsedRealtime();
            encoder = MediaCodec.createByCodecName(codecInfo.getName());
            encoder.configure(videoFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            surfaceReference.set(encoder.createInputSurface());
            encoder.start();
            long codecInitializedTimestamp = SystemClock.elapsedRealtime();
//            Log.d(TAG, taskId + "/Encoder]initialized time: " + (codecInitializedTimestamp - codecInitializingTimestamp));
            CBLog.d(taskId + "/Encoder]created encoder id: " + encoder);
        } catch (IOException |IllegalArgumentException |IllegalStateException e) {
            encoder = null;
            if(listener != null) {
                listener.onEncoderListener(taskId + "/Encoder]createEncoder / codecError= " + e.getMessage());
            }
            encoderInputThread = null;
            encoderOutputThread = null;
            return false;
        }
        return true;
    }

    public void executeThread() {
        encoderInputThread.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        encoderOutputThread.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();

    MediaCodec encoder;

    /**
     * Generates a frame of data using GL commands.
     */
    private void generateSurfaceFrame(int frameIndex, int width, int hegith) {
        int startX, startY;

        final int TEST_R0 = 0;                   // RGB equivalent of {0,0,0}
        final int TEST_G0 = 136;
        final int TEST_B0 = 0;
        final int TEST_R1 = 236;                 // RGB equivalent of {120,160,200}
        final int TEST_G1 = 50;
        final int TEST_B1 = 186;

        frameIndex %= 8;
        if (frameIndex < 4) {
            // (0,0) is bottom-left in GL
            startX = frameIndex * (width / 4);
            startY = hegith / 2;
        } else {
            startX = (7 - frameIndex) * (width / 4);
            startY = 0;
        }

        GLES20.glDisable(GLES20.GL_SCISSOR_TEST);
        GLES20.glClearColor(TEST_R0 / 255.0f, TEST_G0 / 255.0f, TEST_B0 / 255.0f, 1.0f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glEnable(GLES20.GL_SCISSOR_TEST);
        GLES20.glScissor(startX, startY, width / 4, hegith / 2);
        GLES20.glClearColor(TEST_R1 / 255.0f, TEST_G1 / 255.0f, TEST_B1 / 255.0f, 1.0f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
    }

    class ecInputThread extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... params) {
            if(encoder == null) {
                return null;
            }

            long videoPTS = 0l;
            int videoFrameCount = 0;

            /* all job should be done in the same thread.
            **/
            surface = new InputSurface(surfaceReference.get());

            for(;;) {
                if(flag_codecError) {
                    CBLog.d(taskId + "/Encoder]ecInputThread / flag_codecError(true)");
                    flag_inputTheadEnd = true;
                    return null;
                }

                if (isCancelled()) {
                    CBLog.d(taskId + "/Encoder]ecInputThread / cancel");
                    flag_inputTheadEnd = true;
                    encoder.signalEndOfInputStream();
                    return null;
                }

                surface.makeCurrent();
                generateSurfaceFrame(videoFrameCount, surface.getWidth(), surface.getHeight());
                surface.setPresentationTime(videoPTS*1000);
                surface.swapBuffers();

                videoFrameCount++;
                videoPTS+=33000;
            }
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            //
            flag_inputTheadEnd = true;
        }
    }

    boolean flag_inputTheadEnd = false;
    boolean flag_codecError = false;

    class ecOutputThread extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... params) {
            if(encoder == null) {
                CBLog.d(taskId + "/Encoder]ecOutputThread / encoder null");
                return null;
            }

            for(;;) {
                try {
                    int outputBufferIndex = encoder.dequeueOutputBuffer(bufferInfo, -1);
                    if(outputBufferIndex < 0) {
                        if(outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                            CBLog.d(taskId + "/Encoder]ecOutputThread / outputVideoFormat = " + encoder.getOutputFormat());
                        } else if(outputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                            CBLog.d(taskId + "/Encoder]ecOutputThread / ReTry");
                        }
                        continue;
                    } else {
                        if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                            // The codec will continue to return output buffers
                            // until it eventually signals the end of the output stream by specifying the same end-of-stream flag in the MediaCodec.BufferInfo set in dequeueOutputBuffer
                            // or returned via onOutputBufferAvailable.
                            CBLog.d(taskId + "/Encoder]ecOutputThread / eos");
                            return null;
                        }
                        if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                            CBLog.d(taskId + "/Encoder]ecOutputThread / codec config buffer");
                            encoder.releaseOutputBuffer(outputBufferIndex, false);
                            continue;
                        }

                        encoder.releaseOutputBuffer(outputBufferIndex, false);
                    }
                } catch(IllegalStateException |MediaCodec.CryptoException e) {
                    CBLog.d(taskId + "/Encoder]ecOutputThread / codecError=" + e.getMessage());
                    flag_codecError = true;
                    long timeout = System.currentTimeMillis();
                    while(true) {
                        if(flag_inputTheadEnd) {
                            break;
                        }
                        CBLog.d("wait Input Thread End");
                        try {
                            Thread.sleep(10);
                            if(System.currentTimeMillis() - timeout > 10000){
                                break;
                            }
                        } catch (InterruptedException e1) {
                            e1.printStackTrace();
                        }
                    }
                    return null;
                }
            }
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if(surface != null/*for not executing repeatedly*/ && encoder != null) {
                releaseEncoder();
                if(listener != null) {
                    listener.onEncoderListener(taskId + "/Encoder] release: " + (flag_codecError == true ? "Error" : "NoError"));
                }
            }
        }
    }
}
