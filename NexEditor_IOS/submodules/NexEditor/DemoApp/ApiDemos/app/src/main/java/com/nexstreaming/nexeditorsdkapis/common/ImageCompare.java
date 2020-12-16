/******************************************************************************
 * File Name        : ImageCompare.java
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

import android.graphics.Bitmap;

import com.nexstreaming.nexeditorsdkapis.qatest.AutoTestActivity;

import java.nio.ByteBuffer;

/**
 * Created by songkyebeom on 10/04/2017.
 */

public class ImageCompare {

    public static class CompareValue {
        public float aveDiff = 0f;
        public float diffPercent = 0f;

        public double mse = 0f;
        public double snr = 0f;
        public double max_psnr = 0f;
        public double psnr = 0f;

        public void reset() {
            aveDiff = 0f;
            diffPercent = 0f;
            mse = 0f;
            snr = 0f;
            max_psnr = 0f;
            psnr = 0f;
        }
    };

    public double log10(double x) {
        return Math.log(x)/Math.log(10);
    }

    private byte[] loadBitmapByteArray(Bitmap bitmap) {
        int bytes = bitmap.getByteCount();
        ByteBuffer buffer = ByteBuffer.allocate(bytes);
        bitmap.copyPixelsToBuffer(buffer);
        byte[] array = buffer.array();
        return array;
    }

    public void compareBitmap(Bitmap bitmap1, Bitmap bitmap2, CompareValue result) {
        if (bitmap1.getWidth() != bitmap2.getWidth() || bitmap1.getHeight() != bitmap2.getHeight()) {
            throw new RuntimeException("images were of diffrent size");
        }

        byte[] first = loadBitmapByteArray(bitmap1);
        byte[] second = loadBitmapByteArray(bitmap2);
        int loopCount = first.length;
        int diffCount = 0;
        long diffSum = 0;
        for (int i = 0; i < loopCount; i++) {
            int v1 = 0xFF & first[i];
            int v2 = 0xFF & second[i];
            int error = Math.abs(v1 - v2);
            if (error > 0) {
                diffCount++;
                diffSum += error;
            }
        }
        result.diffPercent = ((float) diffCount) / first.length;
        result.aveDiff = ((float) diffSum) / first.length;
    }

    public void compareBitmapWithPSNR(Bitmap bitmap1, Bitmap bitmap2, CompareValue result) {
        if (bitmap1.getWidth() != bitmap2.getWidth() || bitmap1.getHeight() != bitmap2.getHeight()) {
            throw new RuntimeException("images were of diffrent size");
        }

        double  peak, signal, noise, mse;

        byte[] first = loadBitmapByteArray(bitmap1);
        byte[] second = loadBitmapByteArray(bitmap2);
        int width = bitmap1.getWidth();
        int height = bitmap1.getHeight();

        signal = noise = peak = 0;
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                int idx = i*width+j;
                signal += first[idx] * first[idx];
                noise += (first[idx] - second[idx]) * (first[idx] - second[idx]);
                if (peak < first[idx])
                    peak = first[idx];
            }
        }

        result.mse = noise/(width*height);
        result.snr = 10*log10(signal/noise);
        result.max_psnr = (10*log10(255*255/result.mse));
        result.psnr = 10*log10((peak*peak)/result.mse);
    }
}
