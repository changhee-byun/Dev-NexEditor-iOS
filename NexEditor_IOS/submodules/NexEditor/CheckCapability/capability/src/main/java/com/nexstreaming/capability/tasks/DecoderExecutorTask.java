package com.nexstreaming.capability.tasks;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.os.SystemClock;
import android.view.Surface;

import com.nexstreaming.capability.util.CBLog;
import com.nexstreaming.capability.util.Util;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by Eric on 2015-10-07.
 */
public class DecoderExecutorTask {

    final String TAG = "DecoderTask";

    public interface DecoderListener {
        void onDecoderListener(String message);
    }

    DecoderListener listener;

    String filePath;
    int taskId;

    boolean codecStartFail = false;

    public DecoderExecutorTask(int taskId, String filePath, DecoderListener listener) {
        this.taskId = taskId;
        this.filePath = filePath;
        this.listener = listener;

        flag_cancelThread = false;
        flag_eos = false;
        codecStartFail = false;
    }

    volatile boolean flag_cancelThread;
    volatile boolean flag_eos;

    public void cancelThread() {
        CBLog.d(taskId + "/Decoder]cancelThread");
        flag_cancelThread = true;
    }

    public void releaseDecoder() {
        try {
            if ( decoder != null && !codecStartFail ) {
                if ( Util.SDK_INIT > 20 ) {
                    if ( flag_codecError ) {
                        decoder.reset();
                        decoder.stop();
                    } else {
                        decoder.stop();
                    }
                } else {
                    decoder.stop();
                }
            } else {
                codecStartFail = false;
            }
        } catch ( IllegalStateException e ) {
            e.printStackTrace();
        } finally {
            if ( decoder != null ) {
                decoder.release();
            }
            CBLog.d(taskId + "/Decoder]releaseDecoder, decoder=" + decoder);
            decoder = null;
        }
    }

    long source_duration;
    Surface surface;

    // 0: no, 1: real time, 2: available
    int testLevel = 0;

    public boolean createDecoder(Surface surface, int testLevel) {
        CBLog.d(taskId + "/Decoder]createDecoder testLevel: " + testLevel);
        this.testLevel = testLevel;
        this.surface  = surface;
        try {
            extractor = new MediaExtractor();
            extractor.setDataSource(filePath);
            for ( int i = 0; i < extractor.getTrackCount(); i++ ) {
                MediaFormat tmp_format = extractor.getTrackFormat(i);
                String mime = tmp_format.getString(MediaFormat.KEY_MIME);
                if ( mime.startsWith("video/") ) {
                    source_duration = tmp_format.getLong(MediaFormat.KEY_DURATION);
                    format = tmp_format;
                    extractor.selectTrack(i);
                    extractor.seekTo(0, MediaExtractor.SEEK_TO_CLOSEST_SYNC);
                    break;
                }
            }
        } catch ( IOException e ) {
            e.printStackTrace();
        }

        try {
            elapsedBasedTime = SystemClock.elapsedRealtime();
            long codecInitializingTimestamp = SystemClock.elapsedRealtime();
            // Override configureCodec to provide the surface.
            decoder = MediaCodec.createDecoderByType(format.getString(MediaFormat.KEY_MIME));
            decoder.configure(format, this.surface, null, 0);
            decoder.start();
            long codecInitializedTimestamp = SystemClock.elapsedRealtime();
//            Log.d(TAG, taskId + "/Decoder]initialized time: " + (codecInitializedTimestamp - codecInitializingTimestamp));
            CBLog.d(taskId + "/Decoder]created decoder id: " + decoder);
        } catch ( IOException |IllegalArgumentException |IllegalStateException | NullPointerException e ) {
            CBLog.d(taskId + "/Decoder]createDecoder / codecError=" + e.getMessage());
            codecStartFail = true;
            return false;
        }
        return true;
    }

    public boolean createDecoderEx(Surface surface, int width, int height) {
        CBLog.d(taskId + "/Decoder]createDecoderEx");
        this.surface  = surface;

        MediaFormat videoFormat;
        videoFormat= MediaFormat.createVideoFormat("video/avc"/*h.264*/, width, height);
        videoFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        videoFormat.setInteger(MediaFormat.KEY_BIT_RATE, 5000000);
        videoFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 30);

        try {
//            elapsedBasedTime = SystemClock.elapsedRealtime();
//            long codecInitializingTimestamp = SystemClock.elapsedRealtime();
            // Override configureCodec to provide the surface.
            decoder = MediaCodec.createDecoderByType("video/avc");

            CBLog.d("decoder : " + decoder.toString() + " / " + decoder.getName() + " / " + decoder);
            decoder.configure(videoFormat, this.surface, null, 0);
            decoder.start();
//            long codecInitializedTimestamp = SystemClock.elapsedRealtime();
//            Log.d(TAG, taskId + "/Decoder]initialized time: " + (codecInitializedTimestamp - codecInitializingTimestamp));
            CBLog.d(taskId + "/Decoder]created decoder id: " + decoder);
        } catch ( IOException |IllegalArgumentException |IllegalStateException | NullPointerException e ) {
            CBLog.d(taskId + "/Decoder]createDecoderEx / codecError=" + e.getMessage());
            codecStartFail = true;
            return false;
        }
        return true;
    }

    Thread inputThread;
    Thread outputThread;

    public void startThread() {
        inputThread = new Thread(decoderInputThread);
        outputThread = new Thread(decoderOutputThread);

        inputThread.start();
        outputThread.start();
    }

    MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();

    MediaExtractor extractor;
    MediaCodec decoder;
    MediaFormat format;

    synchronized void feedInputBuffer(ByteBuffer inputBuffer, int inputBufferId)
    throws IllegalStateException, MediaCodec.CryptoException {
        inputBuffer.clear();
        // fill inputBuffer with valid data
        int sampleSize = extractor.readSampleData(inputBuffer, 0);
        if ( sampleSize > 0 ) {
            long pts = extractor.getSampleTime();
            decoder.queueInputBuffer(inputBufferId, 0, sampleSize, pts, 0);
            extractor.advance();
        } else {
            CBLog.d(taskId + "/Decoder]input buffer(idx): ("
                    + inputBufferId + ","
                    + " eos" + "))");
            flag_eos = true;
            decoder.queueInputBuffer(inputBufferId, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
        }
    }

    synchronized void feedInputBuffer(ByteBuffer[] inputBuffer, int inputBufferId)
    throws IllegalStateException, MediaCodec.CryptoException {
        inputBuffer[inputBufferId].clear();
        // fill inputBuffer with valid data
        int sampleSize = extractor.readSampleData(inputBuffer[inputBufferId], 0);
        if ( sampleSize > 0 ) {
            long pts = extractor.getSampleTime();
            decoder.queueInputBuffer(inputBufferId, 0, sampleSize, pts, 0);
            extractor.advance();
        } else {
            CBLog.d(taskId + "/Decoder]input buffer(idx): ("
                    + inputBufferId + ","
                    + " eos" + "))");
            flag_eos = true;
            decoder.queueInputBuffer(inputBufferId, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
        }
    }

    volatile boolean isCompleteFeed = false;

    Runnable decoderInputThread = new Runnable() {
        @Override
        public void run() {
            try {
                while ( !flag_cancelThread
                        && !flag_eos ) {
                    int inputBufferId = decoder.dequeueInputBuffer(-1);
                    if ( decoder == null // -1's meaning: wait indefinitely * for the availability of an input buffer
                         || flag_codecError ) {
                        CBLog.d(taskId + "/Decoder]decoderInputThread / it's made decoder null or it's made flag_codecError true");
                        return;
                    }

                    isCompleteFeed = false;
                    if ( inputBufferId >= 0 ) {
                        if ( Util.SDK_INIT > 20 ) {
                            ByteBuffer inputBuffer = decoder.getInputBuffer(inputBufferId);
                            if ( inputBuffer != null ) {
                                feedInputBuffer(inputBuffer, inputBufferId);
                            } else {
                                CBLog.d(taskId + "/Decoder]inputBuffer null");
                            }
                        } else {
                            ByteBuffer[] inputBuffer = decoder.getInputBuffers();
                            if ( inputBuffer != null && inputBuffer.length != 0 ) {
                                feedInputBuffer(inputBuffer, inputBufferId);
                            } else {
                                CBLog.d(taskId + "/Decoder]inputBuffer null or length is 0");
                            }
                        }
                        isCompleteFeed = true;
                    } else {
                        CBLog.d(taskId + "/Decoder]decoderInputThread / ReTry");
                        isCompleteFeed = true;
                        continue;
                    }
                }
                if ( flag_cancelThread ) {
                    int inputBufferId = decoder.dequeueInputBuffer(-1);
                    decoder.queueInputBuffer(inputBufferId, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                }
            } catch ( IllegalStateException e ) {
                CBLog.d(taskId + "/Decoder]decoderInputThread / codecError=" + e.getMessage());
                if ( !isCompleteFeed ) isCompleteFeed = true;
                return;
            }
//            CBLog.d(taskId + "/Decoder]decoderInputThread / (flag_cancelThread, flag_eos, flag_codecError)=" + flag_cancelThread + "," + flag_eos + "," + flag_codecError);
        }
    };

    //
    long elapsedBasedTime = 0l;

    boolean isInsideThreshold = true;
    volatile boolean flag_codecError = false;

    Runnable decoderOutputThread = new Runnable() {
        @Override
        public void run() {
            try {
                long timeout = System.currentTimeMillis();
                CBLog.d(taskId + "/Decoder]decoderOutputThread / Set timeout:"+timeout);
                while ( !flag_cancelThread && !flag_eos ) {
                    int outputBufferIndex = decoder.dequeueOutputBuffer(bufferInfo, 5000000);
                    if ( outputBufferIndex < 0 ) {
                        if ( outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED ) {
                            CBLog.d(taskId + "/Decoder]decoderOutputThread / outputVideoFormat = " + decoder.getOutputFormat());
                        } else if ( outputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER ) {
                            CBLog.d(taskId + "/Decoder]decoderOutputThread / ReTry");
                        }
                        CBLog.d(taskId + "/Decoder]decoderOutputThread / elapsed time:"+(System.currentTimeMillis()-timeout));
                        if ( System.currentTimeMillis() - timeout > 20000 ) {
                            flag_codecError=true;
                            break;
                        }
                        continue;
                    } else {
                        // The codec will continue to return output buffers
                        // until it eventually signals the end of the output stream by specifying the same end-of-stream flag in the MediaCodec.BufferInfo set in dequeueOutputBuffer
                        // or returned via onOutputBufferAvailable.
                        if ( (bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0 ) {
                            CBLog.d(taskId + "/Decoder]decoderOutputThread / eos, elapsed time="
                                    + (SystemClock.elapsedRealtime() - elapsedBasedTime) + " source duration:" + source_duration / 1000);

                            if ( testLevel == 1 ) {
                                if ( (SystemClock.elapsedRealtime() - elapsedBasedTime) > (source_duration/1000) ) {
                                    isInsideThreshold = false;
                                } else {
                                    isInsideThreshold = true;
                                }
                            } else {
                                isInsideThreshold = true;
                            }
                            break;
                        }
// debug //
//                        long pts = bufferInfo.presentationTimeUs;
                        long pts = bufferInfo.presentationTimeUs / 1000;
//                        int size = bufferInfo.size;
//                        int offset = bufferInfo.offset;
//                        int flags = bufferInfo.flags;

                        long playtime = (SystemClock.elapsedRealtime() - elapsedBasedTime);

//                        NexLog.d(taskId + "/Decoder]decoderOutputThread / (idx,size,offset,flags,pts)=("
//                                + outputBufferIndex + ", "
//                                + size + ", "
//                                + offset + ", "
//                                + flags + ", "
//                                + pts / 1000 + ")");

                        if ( (testLevel == 1) && pts > playtime && (pts < source_duration / 1000) ) {
                            long sleeptime = pts - playtime;
                            try {
                                Thread.sleep(sleeptime);
                            } catch ( InterruptedException e ) {
                                e.printStackTrace();
                            }
                        }

                        if ( (bufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0 ) {
                            CBLog.d(taskId + "/Decoder]decoderOutputThread / codec config buffer");
                            decoder.releaseOutputBuffer(outputBufferIndex, false);
                            continue;
                        }
                        decoder.releaseOutputBuffer(outputBufferIndex, true);
                    }
                }
            } catch ( IllegalStateException e ) {
                CBLog.d(taskId + "/Decoder]decoderOutputThread / codecError=" + e.getMessage());
                flag_codecError = true;
                while ( !isCompleteFeed ) {
                    try {
                        CBLog.d("decoderOutputThread Sleep");
                        Thread.sleep(10);
                    } catch ( InterruptedException ie ) {
                        ie.printStackTrace();
                    }
                }
            } finally {
                releaseDecoder();

                if ( extractor != null ) {
                    extractor.release();
                    extractor = null;
                }

                if ( listener != null ) {
                    if ( flag_codecError ) {
                        listener.onDecoderListener(taskId + "/Decoder]codecError in decoderOutputThread");
                    } else {
                        listener.onDecoderListener(taskId + "/Decoder] "+(isInsideThreshold==true?"OK":"NOK"));
                    }
                }

                if ( flag_eos ) flag_eos = false;
                if ( flag_cancelThread ) flag_cancelThread = false;
                if ( flag_codecError ) flag_codecError = false;

                inputThread = null;
                outputThread = null;
            }
        }
    };
}
