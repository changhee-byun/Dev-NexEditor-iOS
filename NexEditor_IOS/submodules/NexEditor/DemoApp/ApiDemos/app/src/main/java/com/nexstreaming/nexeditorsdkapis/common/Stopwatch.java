/******************************************************************************
 * File Name        : Stopwatch.java
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

package com.nexstreaming.nexeditorsdkapis.common;

public class Stopwatch {

    private long startTime;
    private long accum;
    private boolean running;

    public void start() {
        if(!running) {
            running = true;
            startTime = System.nanoTime();
        }
    }

    public void stop() {
        if(running) {
            running = false;
            accum += System.nanoTime()-startTime;
        }
    }

    public void reset() {
        running = false;
        accum = 0;
    }

    public boolean isRunning() {
        return running;
    }

    public long elapsedNanos() {
        if(running) {
            return accum + (System.nanoTime()-startTime);
        } else {
            return accum;
        }
    }

    public long elapsedMillis() {
        if(running) {
            return (accum + (System.nanoTime()-startTime))/1000000;
        } else {
            return accum/1000000;
        }
    }

    @Override
    public String toString() {
        long elapsed = elapsedNanos();
        return String.format("%1$,.3f ms", (double) elapsed / 1000000.0);
    }
}
