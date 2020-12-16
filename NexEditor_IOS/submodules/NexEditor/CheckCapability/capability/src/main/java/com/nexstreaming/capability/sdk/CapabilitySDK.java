package com.nexstreaming.capability.sdk;

import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.Surface;

import com.nexstreaming.capability.test.CodecCapabilityManager;
import com.nexstreaming.capability.util.CBLog;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;

/**
 * NexEditor&tm; Capability Check SDK
 *
 * Before executing NexEditor&tm;, the NexEditor&tm; Capability Check SDK counts the number of Encoders and Decoders(Codec) usable on the device. (It only needs to be executed once on the device.)
 * When Capability SDK is run, a folder named 'CodecCapacity'will be created in the Environment.getExternalStorageDirectory(),
 * and a Json CapabilityResult.txt file in the folder as a result.
 */
public class CapabilitySDK {
    final String TAG = "Capability";

    //
    Context context;
    Activity activity;

    // Parameters
    Boolean isSearchAvailableCodec = false;
    int threadCount = 0;

    // 0: no, 1: real time, 2: available
    int currentTestLevel = 0;

    //
//    CodecCapabilityTest codecCapabilityTest;
    CodecCapabilityManager codecCapabilityManager;
    Thread destroySurfaceThread;

    //
    CapabilityListener capabilityListenerlistener;

    /**
     * A CapabilityListener interface.
     * When the Capability SDK is run, a completion event is passed to the listener.
     * @param in Capability check result value.
     */
    public interface CapabilityListener {
        void onCapabilityListener(InputStream in);
    }

    ArrayList<SurfaceTexture> surfaceTexturesList = new ArrayList<>();
    ArrayList<Surface> surfacesList = new ArrayList<>();

    /**
     * A Capability SDK Constructor.
     *
     * <p>Example code:</p>
     * <pre>
     *     <b>class sample code</b>
     *     {@code
            mCapabilitySDK = new CapabilitySDK(context, activity, this);
            }
     * </pre>
     *
     * @param context context
     * @param activity activity
     * @param listener CapabilityListener
     */
    public CapabilitySDK(Context context, Activity activity, CapabilityListener listener)
    {
        this.context = context;
        this.activity = activity;
        this.capabilityListenerlistener = listener;

        init();

        for ( int loopId=0; loopId<CodecCapabilityManager.MAX_CODEC_COUNT ; loopId++ ) {
            surfaceTexturesList.add(loopId, null);
            surfacesList.add(loopId, null);
        }
    }

    private void init()
    {
        codecCapabilityManager = new CodecCapabilityManager(context);
        codecCapabilityManager.setCodecCapabilityTestListener(codecCapabilityManagerlistener);
    }

    CodecCapabilityManager.CodecCapabilityManagerListener codecCapabilityManagerlistener = new CodecCapabilityManager.CodecCapabilityManagerListener()
    {
        @Override
        public void onCodecCapabilityListener(String request_message, final int requestThreadCount, final InputStream in)
        {
            CBLog.d("onCodecCapabilityListener> request_message:" + request_message);

            threadCount = requestThreadCount;

            if ( request_message.equals("generateSurface") ) {
                CBLog.d("onCodecCapabilityListener> requestThreadCount:" + requestThreadCount);

                try {
                    if ( destroySurfaceThread != null ) {
                        if ( destroySurfaceThread.getState() == Thread.State.NEW ) {
                            destroySurfaceThread.start();
                        }
                        destroySurfaceThread.join();
                        destroySurfaceThread = null;
                    }
                } catch ( InterruptedException e ) {
                    e.printStackTrace();
                }

                new Thread(new Runnable()
                {
                    @Override
                    public void run() {
                        CBLog.d("currentTestLevel : " + currentTestLevel);
                        generateSurface(requestThreadCount);
                        codecCapabilityManager.capabilityTestStart(surfacesList, currentTestLevel);
                    }
                }).start();
            } else if ( request_message.equals("destroySurface") ) {
                CBLog.d("onCodecCapabilityListener> destroySurface requestThreadCount:" + requestThreadCount);
                destroySurfaceThread = new Thread(new Runnable() {
                    @Override
                    public void run() {
                      destroySurface(requestThreadCount);
                    }
                });
            }

            else if ( request_message.equals("searchRealTimeTest") ) {
                currentTestLevel = 1;
            } else if ( request_message.equals("searchAvailableCodecTest") ) {
                currentTestLevel = 2;
//                isSearchAvailableCodec = true;
//                destroySurfaceThread.start();
            } else if ( request_message.equals("generateResult") ) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        activity.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                if ( capabilityListenerlistener != null ) {
                                    capabilityListenerlistener.onCapabilityListener(in);
                                    //codecCapabilityTest.releaseResources();
                                }
                            }
                        });
                    }
                }).start();

            }
        }
    };

    /**
     * This method starts the Capability test.
     * @return \c TRUE if the test is successful or \c FALSE if not.
     */
    public boolean startCapabilityTest(int encoderSet, int decoderSet, int maxCodecCount) {
        boolean result;
        currentTestLevel = 1;

        result = codecCapabilityManager.startCodecCapabilityTest(encoderSet, decoderSet, maxCodecCount);
        if ( !result ) {
            codecCapabilityManager.stopCodecCapabilityTest();
            codecCapabilityManager.releaseResources();
        }
        return result;
    }

    /**
     * @brief This method stops the Capability test.
     */
    public void stopCapabilityTest() {
        isSearchAvailableCodec = false;
        codecCapabilityManager.stopCodecCapabilityTest();
        codecCapabilityManager.releaseResources();
    }

    /**
     * @brief This method gets the NexEditor&tm; Capability SDK Version information.
     * @return Capability SDK Version String
     */
    public String getSDKVersionInfo()
    {
        return codecCapabilityManager.getSDKVersion();
    }

    private void releaseResources() {
        if ( surfaceTexturesList != null ) {
            surfaceTexturesList.clear();
            surfaceTexturesList = null;
        }

        if ( surfacesList != null ) {
            surfacesList.clear();
            surfacesList = null;
        }
    }

    private void destroySurface(int threadCount) {
        if ( surfaceTexturesList == null ) return;

        int loopId;
        for ( loopId=0; loopId<threadCount; loopId++ ) {
            surfacesList.get(loopId).release();
            surfaceTexturesList.get(loopId).release();

            surfaceTexturesList.set(loopId, null);
            surfacesList.set(loopId, null);
        }

//        Log.d(TAG, "destroySurface.");
        destroySurfaceThread = null;

//        if(isSearchAvailableCodec) {
//            isSearchAvailableCodec = false;
//            CodecCapabilityManager.searchAvailableCodecCount();
//        }
    }

    private void generateSurface(int threadCount) {
        if ( surfaceTexturesList == null ) return;

        int loopId;
        for ( loopId=0; loopId<threadCount; loopId++ ) {
            SurfaceTexture surfaceTexture = new SurfaceTexture(loopId);
            Surface surface = new Surface(surfaceTexture);

            surfaceTexturesList.set(loopId, surfaceTexture);
            surfacesList.set(loopId, surface);
        }
    }

}