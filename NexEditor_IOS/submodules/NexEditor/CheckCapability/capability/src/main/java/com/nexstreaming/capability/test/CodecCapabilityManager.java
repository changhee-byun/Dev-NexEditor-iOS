package com.nexstreaming.capability.test;

import android.app.ActivityManager;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
import android.view.Surface;

import com.nexstreaming.capability.R;
import com.nexstreaming.capability.deviceinfo.CapabilityResult;
import com.nexstreaming.capability.tasks.DecoderExecutorTask;
import com.nexstreaming.capability.tasks.EncoderExecutorTask;
import com.nexstreaming.capability.util.CBLog;
import com.nexstreaming.capability.util.JSonWriterUtil;
import com.nexstreaming.capability.util.ResourceUtil;
import com.nexstreaming.capability.util.Util;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;

/**
 * Created by damian.lee on 2016-06-23.
 */
public class CodecCapabilityManager implements
EncoderExecutorTask.EncoderListener,
DecoderExecutorTask.DecoderListener {

    private final int SDK_VERSION_MAJOR = 1;
    private final int SDK_VERSION_MINOR = 1;
    private final int SDK_VERSION_PATCH = 5;

    private final int TEST_RUN_ENCODER = 0;
    private final int TEST_ONLY_DECODER = 1;

    private final int MESSAGE_TEST_AGAIN = 1;
    private final int MESSAGE_TEST_RESULT = 3;

    public static final int TEST_CONTENT_720P = 0;
    public static final int TEST_CONTENT_1080P = 1;
    public static final int TEST_CONTENT_3840P = 2;

    private int currentTestState = TEST_RUN_ENCODER;

    private int testContentType = TEST_CONTENT_1080P;

    public static final int MAX_CODEC_COUNT=64;
    private int maxCodecCount = MAX_CODEC_COUNT;

    private int requestThreadCount;
    private int responseThreadCount;
    private int decoderSuccessCount = 0;
    private int availableThreadCount;

    private boolean isEncoderError = false;
    private boolean isCodecError = false;
    private boolean isStopState = false;
    private boolean isProcessLowMemory = false;
    private boolean isFind = false;

    private int mTestLevel = -1;

    private int testFPS = 30;
    private int contentWidth = 1920;
    private int contentHeight = 1080;


    Context mContext;
    String mTestFilepath;

    ArrayList<DecoderExecutorTask> decoderExecutorTasks = new ArrayList<>();
    ArrayList<Surface> surfaceList;

    EncoderExecutorTask encoderExecutorTask;

    EncoderExecutorTask.EncoderListener encoderListener;
    DecoderExecutorTask.DecoderListener decoderListener;

    CodecCapabilityManagerListener codecCapabilityManagerListener;

    CapabilityResult capabilityResult;

    public interface CodecCapabilityManagerListener {
        void onCodecCapabilityListener(String request_message, int requestThreadCount, InputStream in);
    }

    public void setCodecCapabilityTestListener(CodecCapabilityManagerListener listener) {
        this.codecCapabilityManagerListener = listener;
    }

    public CodecCapabilityManager(Context context){

        mContext = context;

        encoderListener = this;
        decoderListener = this;

        setTestContents(TEST_CONTENT_1080P);

    }

    public boolean startCodecCapabilityTest(int encoderSet, int decoderSet, int codecCountSet) {

        if ( checkProcessLowMemory() ) {
            return false;
        }

        capabilityResult = new CapabilityResult();

        isProcessLowMemory = false;
        isCodecError = false;
        isStopState = false;

        responseThreadCount = 1;
        requestThreadCount = 1;
        currentTestState = TEST_RUN_ENCODER;
        testContentType = decoderSet;
        setTestContents(testContentType);

        maxCodecCount = getCreateCodecCount(testContentType);
        if ( maxCodecCount > codecCountSet) maxCodecCount = codecCountSet;
        if ( maxCodecCount > MAX_CODEC_COUNT ) maxCodecCount = MAX_CODEC_COUNT;

        CBLog.d("maxCodecCount["+testContentType+"]: " + maxCodecCount);

        if ( createEncoderTask(encoderSet) == true ) {
            runEncoderTask();
        }

        if ( codecCapabilityManagerListener != null ) {
            codecCapabilityManagerListener.onCodecCapabilityListener("generateSurface", requestThreadCount, null);
        }
        return true;
    }

    public void capabilityTestStart(ArrayList<Surface> surfaces, int testLevel)
    {
        decoderSuccessCount = 0;

        if ( surfaces == null ) {
            capabilityResult.setInTimeCount2TestList(currentTestState, availableThreadCount);

            if ( codecCapabilityManagerListener != null ) {
                codecCapabilityManagerListener.onCodecCapabilityListener("destroySurface", requestThreadCount, null);
                codecCapabilityManagerListener.onCodecCapabilityListener("searchAvailableCodecTest", -1, null);
            }
            return;
        }

        surfaceList = surfaces;
        mTestLevel = testLevel;

        isCodecError = false;
        isProcessLowMemory = false;

        if ( createDecoderTask(requestThreadCount, testLevel) ) {
            runDecoderTask(requestThreadCount);
        } else {
            if ( isFind ) {
                isFind = false;

                if ( mTestLevel == 1 ) {
                    capabilityResult.setInTimeCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);
                    capabilityResult.setDecoderCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);
                } else if ( mTestLevel == 2 ) {
                    capabilityResult.setDecoderCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);
                }
            } else {
                capabilityResult.setInTimeCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, 0);
            }

            if ( currentTestState == TEST_ONLY_DECODER ) {
                Message msg = Message.obtain();
                msg.what = MESSAGE_TEST_RESULT;
                codecCapabilityHandler.sendMessage(msg);
            } else {
                startNotEncoderTest();
            }
        }
    }

    public String getSDKVersion() {
        String SDKVersion = String.format("%d.%d.%d",SDK_VERSION_MAJOR,SDK_VERSION_MINOR,SDK_VERSION_PATCH);
        CBLog.d(SDKVersion);
        return SDKVersion;
    }

    @Override
    public void onDecoderListener(String message)
    {
        CBLog.d("onDecoderListener:" + message);

        if ( decoderExecutorTasks.size() == 0 ) {
            CBLog.d("resultValues is size=0, ignore all events");
            return;
        }

        if ( !isProcessLowMemory && message.contains("codecError") ) {
            CBLog.d("Codec Error");

            if ( isCodecError == false ) {
                isCodecError = true;
                cancelDecoderTask(requestThreadCount);
            }
        } else if ( !isCodecError && checkProcessLowMemory() ) {
            CBLog.d("Low Memory");

            isProcessLowMemory = true;
            CBLog.d("Low Memory, caused by " + !isCodecError + " isProcessLowMemory()=" + checkProcessLowMemory());
        }

        if ( isCodecError || isProcessLowMemory ) {      // response에 Error 가 포함되거나 메모리가 부족한 경우
            CBLog.d("onCodecCapabilityListener:requestThreadCount - " + requestThreadCount + ", responseThreadCount - "+responseThreadCount);
            if ( requestThreadCount == responseThreadCount ) {
                if ( codecCapabilityManagerListener != null ) {
                    CBLog.d("onCodecCapabilityListener:destroySurface - " + requestThreadCount);
                    codecCapabilityManagerListener.onCodecCapabilityListener("destroySurface", requestThreadCount, null);
                }

                if ( mTestLevel == 1 ) {
                    capabilityResult.setInTimeCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);
                    capabilityResult.setDecoderCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);
                } else if ( mTestLevel == 2 ) {
                    capabilityResult.setDecoderCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);
                }

                if ( currentTestState == TEST_ONLY_DECODER ) {
                    Message msg = Message.obtain();
                    msg.what = MESSAGE_TEST_RESULT;
                    codecCapabilityHandler.sendMessage(msg);
                } else {
                    startNotEncoderTest();
                }
            } else {
                responseThreadCount++;
            }
            return;
        }

        if ( !isCodecError && !isProcessLowMemory ) {
            if ( !message.contains("NOK") ) {
                decoderSuccessCount++;
                CBLog.d("decoderSuccessCount - " + decoderSuccessCount);
            }
        }

        CBLog.d("requestThreadCount : " + requestThreadCount + "/ responseThreadCount : " + responseThreadCount);


        if ( requestThreadCount == responseThreadCount ) {
            if ( codecCapabilityManagerListener != null ) {
                CBLog.d("onCodecCapabilityListener:destroySurface " + requestThreadCount);
                codecCapabilityManagerListener.onCodecCapabilityListener("destroySurface", requestThreadCount, null);
            }

            if ( requestThreadCount == decoderSuccessCount ) {
                CBLog.d("Request Count Increase Test");
                isFind = true;

                requestThreadCount += 1;

                // Update success count
                if ( mTestLevel == 1 ) {
                    capabilityResult.setInTimeCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);
                    capabilityResult.setDecoderCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);
                } else if ( mTestLevel == 2 ) {
                    capabilityResult.setDecoderCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);
                }

                // Switch test or force end.
                if ( currentTestState == TEST_RUN_ENCODER && (requestThreadCount > maxCodecCount - 1) ) {
                    startNotEncoderTest();
                } else if ( currentTestState == TEST_ONLY_DECODER && (requestThreadCount > maxCodecCount) ) {
                    Message msg = Message.obtain();
                    msg.what = MESSAGE_TEST_RESULT;
                    codecCapabilityHandler.sendMessage(msg);
                } else {
                    /* Test 환경 변수 조기화 */
                    responseThreadCount = 1;
                    decoderSuccessCount = 0;

                    saveTestResult();//Save intermediate result for displaying result when the app is crashed.

                    Message msg = Message.obtain();
                    msg.what = MESSAGE_TEST_AGAIN;
                    codecCapabilityHandler.sendMessage(msg);
                }
            } else {
                CBLog.d("Request Count : " + requestThreadCount + " / Success Count : " + decoderSuccessCount + " Realtime Test End");
                if ( mTestLevel == 1 ) {
                    if ( requestThreadCount > 1 ) {
                        capabilityResult.setInTimeCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);
                    }

                    mTestLevel = 2;
                    CBLog.d("Available Test Start");
                    codecCapabilityManagerListener.onCodecCapabilityListener("searchAvailableCodecTest", -1, null);


                    /* Test 환경 변수 조기화 */
                    requestThreadCount += 1;
                    decoderSuccessCount = 0;
                    responseThreadCount = 1;

                    if ( requestThreadCount > maxCodecCount ) {
                        capabilityResult.setDecoderCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);

                        Message msg = Message.obtain();
                        msg.what = MESSAGE_TEST_RESULT;
                        codecCapabilityHandler.sendMessage(msg);
                    } else {
                        Message msg = Message.obtain();
                        msg.what = MESSAGE_TEST_AGAIN;
                        codecCapabilityHandler.sendMessage(msg);
                    }
                } else if ( mTestLevel == 2 ) {
                    if ( requestThreadCount > 1 ) {
                        capabilityResult.setDecoderCount2TestList(/*TEST_RUN_ENCODER*/currentTestState, requestThreadCount - 1);
                    }

                    if ( currentTestState == TEST_ONLY_DECODER ) {
                        Message msg = Message.obtain();
                        msg.what = MESSAGE_TEST_RESULT;
                        codecCapabilityHandler.sendMessage(msg);
                    } else {
                        startNotEncoderTest();
                    }
                }
            }
        } else {
            responseThreadCount++;
        }
    }

    @Override
    public void onEncoderListener(String message)
    {
        CBLog.d("onEncoderListener:" + message);

        if ( message.contains("NoError") ) {
            isEncoderError = false;
            if ( isCodecError ) {
                CBLog.d("cancelEncoderTask because decoder error");
                cancelEncoderTask();
            }
//            Message msg = Message.obtain();
//            msg.what = MESSAGE_TEST_AGAIN;
//            codecCapabilityHandler.sendMessage(msg);
        } else {
            isEncoderError = true;
            cancelEncoderTask();
            isCodecError = true;
        }

    }

    private boolean createEncoderTask(int encoderSet)
    {
        switch ( encoderSet ) {
            case TEST_CONTENT_720P:
                encoderExecutorTask = new EncoderExecutorTask(1, 1280, 720, encoderListener);
                break;
            case TEST_CONTENT_3840P:
                encoderExecutorTask = new EncoderExecutorTask(1, 3840, 2160, encoderListener);
                break;
            case TEST_CONTENT_1080P:
            default:
                encoderExecutorTask = new EncoderExecutorTask(1, 1920, 1080, encoderListener);
                break;
        }

        if ( encoderExecutorTask.createEncoder() == false ) {
            encoderExecutorTask.releaseEncoder();
            return false;
        }
        return true;
    }

    boolean createDecoderTask(int threadCount, int testLevel)
    {
        boolean result = true;

        for ( int i=0; i<threadCount; i++ ) {
            if ( decoderExecutorTasks.get(i).createDecoder(surfaceList.get(i), testLevel) == false ) {
                result = false;

                for ( i=0; i<threadCount; i++ ) {
                    decoderExecutorTasks.get(i).releaseDecoder();
                }
                break;
            }
        }
        return result;
    }

    private void runEncoderTask()
    {
        encoderExecutorTask.executeThread();
    }

    void runDecoderTask(int threadCount)
    {
        for ( int i=0; i<threadCount; i++ ) {
            decoderExecutorTasks.get(i).startThread();
        }
    }

    /**
     *  stopCodecCapabilityTest
     */
    public void stopCodecCapabilityTest() {
        isStopState = true;

        if ( currentTestState == TEST_RUN_ENCODER ) {
            cancelEncoderTask();
            cancelDecoderTask(requestThreadCount);
        } else {
            cancelDecoderTask(requestThreadCount);
        }
    }

    /**
     *  releaseResources
     */
    public void releaseResources() {
        if ( capabilityResult != null ) {
            capabilityResult.releaseResource();
            capabilityResult = null;
        }
        decoderExecutorTasks.clear();
    }

    /**
     *  cancelDecoderTask
     */
    void cancelDecoderTask(int threadCount) {
        int loopId;
        for ( loopId=0; loopId<threadCount; loopId++ ) {
            decoderExecutorTasks.get(loopId).cancelThread();
        }
    }

    /**
     *  cancelEncoderTask
     */
    void cancelEncoderTask() {
        if ( encoderExecutorTask != null ) {
            if ( encoderExecutorTask.getThreadStatus() != AsyncTask.Status.PENDING ) {
                encoderExecutorTask.cancelThread(true);
            }
        }
    }

    void startNotEncoderTest()
    {
        cancelEncoderTask();
        try {
            Thread.sleep(500);
        } catch ( InterruptedException e ) {
        }

        System.gc();

        codecCapabilityManagerListener.onCodecCapabilityListener("searchRealTimeTest", -1, null);

        if ( currentTestState == TEST_RUN_ENCODER ) {
            currentTestState = TEST_ONLY_DECODER;
        }

        requestThreadCount = capabilityResult.getInTimeCount2TestList(TEST_RUN_ENCODER);

        if ( requestThreadCount < 1 )
            requestThreadCount = 1;

        responseThreadCount = 1;

        decoderSuccessCount = 0;

        isFind = false;
        isProcessLowMemory = false;
        isCodecError = false;
        isStopState = false;

        Message msg = Message.obtain();
        msg.what = MESSAGE_TEST_AGAIN;
        codecCapabilityHandler.sendMessage(msg);

//        codecCapabilityManagerListener.onCodecCapabilityListener("generateSurface", requestThreadCount, null);

    }

    Handler codecCapabilityHandler = new Handler() {

        @Override
        public void handleMessage(Message message) {
            switch ( message.what ) {
                case MESSAGE_TEST_AGAIN: {
                        if ( !isStopState ) {
                            if ( codecCapabilityManagerListener != null ) {
                                codecCapabilityManagerListener.onCodecCapabilityListener("generateSurface", requestThreadCount, null);
                            }
                        }
                        break;
                    }
                case MESSAGE_TEST_RESULT: {
                        int count1 = capabilityResult.getDecoderCount2TestList(TEST_RUN_ENCODER);
                        int count2 = capabilityResult.getDecoderCount2TestList(TEST_ONLY_DECODER);
                        int count3 = capabilityResult.getInTimeCount2TestList(TEST_RUN_ENCODER);
                        int count4 = capabilityResult.getInTimeCount2TestList(TEST_ONLY_DECODER);


                        int DecoderCount = ( count1 >  count4 ) ? count4 : count1;

                        capabilityResult.setMaxFPS(testFPS * DecoderCount);
                        capabilityResult.setHWCodecMemSize(contentWidth * contentHeight * DecoderCount);
                        capabilityResult.setMaxResolution(contentWidth * contentHeight * DecoderCount);

//                    boolean MPEG4VSupport = checkMPEG4VSupported();
//                    CBLog.d("MPEG4VSupport is : " + MPEG4VSupport);
//                    capabilityResult.setSupportMPEGV4(MPEG4VSupport);

                        CBLog.d("final result(count1, count2, count3, count4)=" + count1 + ", " + count2 + ", " + count3 + ", " + count4);

                        StringBuffer sb = new StringBuffer();
                        sb.append("{\n");
                        sb.append("\"value1_in_type1\"" + ": " + count1 + ",\n");
                        sb.append("\"value2_in_type1\"" + ": " + count2 + ",\n");
                        sb.append("\"value1_in_type2\"" + ": " + count3 + ",\n");
                        sb.append("\"value2_in_type2\"" + ": " + count4 + "\n");
                        sb.append("}");

                        CBLog.d("TEST Result : " + sb.toString());

                        saveTestResult();

                        if ( codecCapabilityManagerListener != null ) {
                            codecCapabilityManagerListener.onCodecCapabilityListener("generateResult", -1, new ByteArrayInputStream(sb.toString().getBytes()));
                        }
                    }
            }
        }
    };

    private int getCreateCodecCount(int type)
    {
        int createdCodecCount = 0;
        int availableCodecCount = 0;
        int countMax = maxCodecCount;

        ArrayList<SurfaceTexture> surfaceTextures = new ArrayList<>();
        ArrayList<Surface> surfaces = new ArrayList<>();

        for ( int i = 0; i < countMax/*maxThread*/; i++ ) {
            createdCodecCount++;
            surfaceTextures.add(new SurfaceTexture(i));
            surfaces.add(new Surface(surfaceTextures.get(i)));
            if ( !checkProcessLowMemory() && decoderExecutorTasks.get(i).createDecoderEx(surfaces.get(i), contentWidth, contentHeight) == true ) {
                //if(decoderExecutorTasks.get(i).createDecoderEx(surfaces.get(i), type) == true) {
                availableCodecCount ++;
            } else {
                break;
            }
        }

        for ( int i = 0; i < createdCodecCount; i++ ) {
            decoderExecutorTasks.get(i).releaseDecoder();
            surfaces.get(i).release();
            surfaceTextures.get(i).release();
        }

        surfaceTextures.clear();
        surfaceTextures = null;

        surfaces.clear();
        surfaces = null;

        return availableCodecCount;
    }


    boolean checkProcessLowMemory() {
//        ActivityManager.RunningAppProcessInfo runningAppProcessInfo = new ActivityManager.RunningAppProcessInfo();
//        ActivityManager.getMyMemoryState(runningAppProcessInfo);
//        /*
//         *   runningAppProcessInfo.lastTrimLevel
//         *   -----------------------------------
//         *   TRIM_MEMORY_RUNNING_CRITICAL(15) : the process is not an expendable background process,
//         *                                      but the device is running extremely low on memory and is about to not be able to keep any background processes running.
//         *                                      Your running process should free up as many non-critical resources as it can to allow that memory to be used elsewhere.
//         *   TRIM_MEMORY_RUNNING_LOW(10) :  the process is not an expendable background process, but the device is running low on memory.
//         *                                  Your running process should free up unneeded resources to allow that memory to be used elsewhere.
//         */
//        if(runningAppProcessInfo.lastTrimLevel == ComponentCallbacks2.TRIM_MEMORY_RUNNING_LOW
//                || runningAppProcessInfo.lastTrimLevel == ComponentCallbacks2.TRIM_MEMORY_RUNNING_CRITICAL) {
//
//
//            CBLog.d("pid:" + runningAppProcessInfo.pid
//                    + " uid:" + runningAppProcessInfo.uid
//                    + " lastTrimLevel:" + runningAppProcessInfo.lastTrimLevel
//                    + " importance:" + runningAppProcessInfo.importance
//                    + " importanceReasonCode:" + runningAppProcessInfo.importanceReasonCode);
//
//            return true;
//        }

        ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();
        ((ActivityManager)mContext.getSystemService(Context.ACTIVITY_SERVICE)).getMemoryInfo(memoryInfo);

        long threshold_value = 0;
        /*
         *  Exception Cases.
         */
        if ( Util.MODEL.contains("Sparkle") ) {
            threshold_value = 50;
        }
        CBLog.d("availMem:" + memoryInfo.availMem / (1024 * 1024)
                + " totalMem:" + memoryInfo.totalMem / (1024 * 1024)
                + " threshold:" + memoryInfo.threshold / (1024 * 1024));

        if ( (memoryInfo.availMem-memoryInfo.threshold)/(1024*1024) < threshold_value ) {


            return true;
        }

//        public void checkAppMemory(){
//            // Get app memory info
//            long available = Runtime.getRuntime().maxMemory();
//            long used = Runtime.getRuntime().totalMemory();
//
//            // Check for & and handle low memory state
//            float percentAvailable = 100f * (1f - ((float) used / available ));
//            if( percentAvailable <= LOW_MEMORY_THRESHOLD_PERCENT )
//                handleLowMemory();
//
//            // Repeat after a delay
//            memoryHandler_.postDelayed( new Runnable(){ public void run() {
//                checkAppMemory();
//            }}, (int)(CHECK_MEMORY_FREQ_SECONDS * 1000) );
//        }

        return false;
    }

    private boolean saveTestResult()
    {
        if ( !Util.checkExternalMemory() ) {
            CBLog.d("checkExternalMemory is false");
            return false;
        }

        JSonWriterUtil jSonWriterUtil = new JSonWriterUtil();
        OutputStream outputStream = jSonWriterUtil.getOutputStream2DefaultPath();

        if ( outputStream == null ) {
            CBLog.d("outputStream is null");
            return false;
        }

        try {
            jSonWriterUtil.writeJsonStream(outputStream, capabilityResult);
        } catch ( IOException e ) {
            e.printStackTrace();
            CBLog.d("writeJsonStream fail");
            return false;
        }

        return true;
    }

    private void setTestContents(int type)
    {
        int testContentResID;
        String fileName;

        if ( type == TEST_CONTENT_3840P ) {
            contentWidth = 3840;
            contentHeight = 2160;
            testContentResID = R.raw.file_4k_30fps_aoa_miniskirt;
            fileName = "file_4k_30fps_aoa_miniskirt.mp4";
        } else if ( type == TEST_CONTENT_1080P ) {
            contentWidth = 1920;
            contentHeight = 1024;
            testContentResID = R.raw.file_1080p_30fps_aoa_miniskirt;
            fileName = "file_1080p_30fps_aoa_miniskirt.mp4";
        } else {
            contentWidth = 1280;
            contentHeight = 720;
            testContentResID = R.raw.file_720p_30fps_aoa_miniskirt;
            fileName = "file_720p_30fps_aoa_miniskirt.mp4";
        }

        try {
            ResourceUtil.raw2file(mContext, testContentResID, fileName);
            mTestFilepath = mContext.getFilesDir().getAbsolutePath()+ File.separator+fileName;
            CBLog.d("file path = " + mTestFilepath);
        } catch ( Exception e ) {
            e.printStackTrace();
        }

        decoderExecutorTasks.clear();

        if ( decoderExecutorTasks.size() == 0 ) {
            for ( int i = 0; i < maxCodecCount; i++ ) {
                decoderExecutorTasks.add(i, new DecoderExecutorTask(i+1, mTestFilepath, decoderListener));
            }
        }
    }
}