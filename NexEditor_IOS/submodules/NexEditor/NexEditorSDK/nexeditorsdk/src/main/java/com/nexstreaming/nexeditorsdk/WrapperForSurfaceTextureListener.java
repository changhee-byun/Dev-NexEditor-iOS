/******************************************************************************
 * File Name        : WrapperForSurfaceTextureListener.java
 * Description      :
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

package com.nexstreaming.nexeditorsdk;

import android.graphics.SurfaceTexture;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.view.Surface;

import java.security.InvalidParameterException;
import java.util.Objects;
import java.util.concurrent.Semaphore;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.TimeUnit;

/**
 * Created by sunghyun.eric.yoo on 2015-07-22.
 * Exclude this class from proguard's list.
 * This class is used from native code.
 *
 * Updated 2015-08-03 by Matthew Feinberg.
 * Changed architecture for correct thread synchronization.
 */

public class WrapperForSurfaceTextureListener implements SurfaceTexture.OnFrameAvailableListener
{
    private static final String LOG_TAG = "WrapperForSTL";
    private Semaphore mFrameAvailableSemaphore = new Semaphore(0);
    private final int mInstanceNum;
    private static int sIntanceNum = 0;
    private SurfaceTexture mConnectedSurfaceTexture = null;
    private static Handler sHandler = null;
    private static HandlerThread sHandlerThread = null;

    @SuppressWarnings("unused") // called from native
    public static SurfaceTexture makeSurfaceTexture(final int texName) {
        final SynchronousQueue<SurfaceTexture> queue = new SynchronousQueue<>();
        if( sHandler==null || sHandlerThread == null ) {
            sHandlerThread = new HandlerThread("surfaceTextureFactory",-2);
            sHandlerThread.start();
            sHandler = new Handler(sHandlerThread.getLooper());
        }
        sHandler.post(new Runnable() {
            @Override
            public void run() {
                SurfaceTexture surfaceTexture = new SurfaceTexture(texName);
                boolean success = false;
                while(true) {
                    try {
                        success = queue.offer(surfaceTexture, 1000, TimeUnit.MILLISECONDS);
                        break;
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
                if( !success ) {
                    surfaceTexture.release();
                    Log.w( LOG_TAG, "Surface texture abandoned");
                }
//                queue.add(new SurfaceTexture(texName));
            }
        });
        SurfaceTexture result = null;
        while(result==null) {
            try {
                result = queue.take();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        return result;
    }

    public WrapperForSurfaceTextureListener(int a)
    {
        mInstanceNum = ++sIntanceNum;
        Log.d(LOG_TAG, "[W:" + mInstanceNum + "] WrapperForSurfaceTextureListener Constructor. a=" + a);
    }

    @SuppressWarnings("unused") // called from native
    public void connectListener(final SurfaceTexture st)
    {
        if( mConnectedSurfaceTexture!=null ) {
            throw new IllegalStateException();
        }
        Log.d(LOG_TAG, "[W:" + mInstanceNum + "] WrapperForSurfaceTextureListener connectListener.");
        mFrameAvailableSemaphore.drainPermits();
        if(Build.VERSION.SDK_INT>=Build.VERSION_CODES.LOLLIPOP) {
            st.setOnFrameAvailableListener(this, sHandler);
        } else {
            sHandler.post(new Runnable() {
                @Override
                public void run() {
                    st.setOnFrameAvailableListener(WrapperForSurfaceTextureListener.this);
                }
            });
        }
        mConnectedSurfaceTexture = st;
    }

    @SuppressWarnings("unused") // called from native
    public void disconnectListener(SurfaceTexture st)
    {
        if( st!=mConnectedSurfaceTexture ) {
            throw new InvalidParameterException();
        }
        Log.d(LOG_TAG, "[W:" + mInstanceNum + "] WrapperForSurfaceTextureListener disconnectListener.");
        st.setOnFrameAvailableListener(null);
        mConnectedSurfaceTexture = null;
    }

    @SuppressWarnings("unused") // called from native
    public int waitFrameAvailable(int timeOutInMS)
    {
        if( mConnectedSurfaceTexture==null ) {
            throw new IllegalStateException();
        }

        timeOutInMS = timeOutInMS < 0 ? 2500/*10000*/ : timeOutInMS;

        //Log.d(LOG_TAG, "[W:" + mInstanceNum + "] waitFrameAvailable : " + timeOutInMS);
        long st = System.nanoTime();
        boolean acquiredSeamphore = false;
        boolean interrupted = false;
        while(true) {
            try {
                acquiredSeamphore = mFrameAvailableSemaphore.tryAcquire(timeOutInMS, TimeUnit.MILLISECONDS);
                break;
            } catch (InterruptedException e) {
                interrupted = true;
                Thread.currentThread().interrupt();
            }
        }
//        mFrameAvailableSemaphore.acquireUninterruptibly();
        boolean timeout = !acquiredSeamphore;
        long elapsed = System.nanoTime() - st;
        if( timeout ) {
            Log.w(LOG_TAG, "[W:" + mInstanceNum + "] waitFrameAvailable : (elapsed=" + elapsed + ") timeout=" + timeout + " interrupted=" + interrupted);
        } else {
            //Log.d(LOG_TAG, "[W:" + mInstanceNum + "] waitFrameAvailable : (elapsed=" + elapsed + ") timeout=" + timeout + " interrupted=" + interrupted);
        }

        return (timeout?0x4:0x0) | (interrupted?0x8:0x0);

    }

    @Override
    public void onFrameAvailable(SurfaceTexture st)
    {
        //Log.d(LOG_TAG, "[W:" + mInstanceNum + "] new frame available." + st);
        if( st != mConnectedSurfaceTexture ) {
            Log.w(LOG_TAG, "[W:" + mInstanceNum + "] WARNING - Frame available to wrong listener : " + st + " != " + String.valueOf(mConnectedSurfaceTexture));
            return;
        }
        mFrameAvailableSemaphore.release();
    }
}
