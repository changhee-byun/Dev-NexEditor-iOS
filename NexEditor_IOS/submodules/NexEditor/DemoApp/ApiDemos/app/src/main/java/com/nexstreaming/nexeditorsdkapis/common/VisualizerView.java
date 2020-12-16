/******************************************************************************
 * File Name        : VisualizerView.java
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

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.View;
/**
 * Created by jeongwook.yoon on 2016-05-19.
 */
public class VisualizerView extends View{
    private byte[] mBytes;
    private float[] mPoints;
    private Rect mRect = new Rect();
    private Paint mForePaint = new Paint();

    public VisualizerView(Context context) {
        super(context);
        init();
    }

    public VisualizerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public VisualizerView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        mBytes = null;
        mForePaint.setStrokeWidth(1f);
        mForePaint.setAntiAlias(true);
        mForePaint.setColor(Color.rgb(0, 128, 255));
    }

    public void updateVisualizer(byte[] bytes) {
        mBytes = bytes;
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (mBytes == null) {
            return;
        }
        if (mPoints == null || mPoints.length < mBytes.length * 4) {
            mPoints = new float[mBytes.length * 4];
        }
        mRect.set(0, 0, getWidth(), getHeight());
        for (int i = 0; i < mBytes.length - 1; i++) {
            mPoints[i * 4] = mRect.width() * i / (mBytes.length - 1);
            mPoints[i * 4 + 1] = mRect.height() / 2
                    + ((byte) (mBytes[i] + 128)) * (mRect.height() / 2) / 128;
            mPoints[i * 4 + 2] = mRect.width() * (i + 1) / (mBytes.length - 1);
            mPoints[i * 4 + 3] = mRect.height() / 2
                    + ((byte) (mBytes[i + 1] + 128)) * (mRect.height() / 2)
                    / 128;
        }
        canvas.drawLines(mPoints, mForePaint);
    }
}
