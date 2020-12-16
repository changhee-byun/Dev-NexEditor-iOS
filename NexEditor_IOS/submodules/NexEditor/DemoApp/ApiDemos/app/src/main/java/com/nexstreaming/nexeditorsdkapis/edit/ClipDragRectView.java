/******************************************************************************
 * File Name        : ClipDragRectView.java
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

package com.nexstreaming.nexeditorsdkapis.edit;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.text.TextPaint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.FrameLayout;

import com.nexstreaming.nexeditorsdk.nexApplicationConfig;

/**
 * Created by jeongwook.yoon on 2015-05-26.
 */
public class ClipDragRectView extends View {
    private Paint mRectPaint;

    private boolean mInit = false;
    private int mStartX = 0;
    private int mStartY = 0;
    private int mEndX = 0;
    private int mEndY = 0;
    private boolean mDrawRect = false;
    private TextPaint mTextPaint = null;

    private int mWidth=640;
    private int mHeight=360;
    private float  mAspectRatio = 1280.0f/720.0f;
    private OnUpCallback mCallback = null;

    public void resizeDimension(int wid , int hei)
    {
        mWidth = wid;
        mHeight = hei;

        FrameLayout.LayoutParams lp = null;
        lp = new FrameLayout.LayoutParams(mWidth, mHeight);
        this.setLayoutParams(lp);
    }

    public interface OnUpCallback {
        void onRectFinished(Rect rect);
    }

    public ClipDragRectView(final Context context) {
        super(context);
        init();
    }

    public ClipDragRectView(final Context context, final AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public ClipDragRectView(final Context context, final AttributeSet attrs, final int defStyle) {
        super(context, attrs, defStyle);
        init();
    }

    /**
     * Sets callback for up
     *
     * @param callback {@link OnUpCallback}
     */
    public void setOnUpCallback(OnUpCallback callback) {
        mCallback = callback;
    }

    /**
     * Inits internal data
     */
    private void init() {
        mRectPaint = new Paint();
        mRectPaint.setColor(getContext().getResources().getColor(android.R.color.holo_green_light));
        mRectPaint.setStyle(Paint.Style.STROKE);
        mRectPaint.setStrokeWidth(5); // TODO: should take from resources

        mTextPaint = new TextPaint();
        mTextPaint.setColor(getContext().getResources().getColor(android.R.color.holo_green_light));
        mTextPaint.setTextSize(20);
        mInit = true;
    }

    public void initRect(Rect rect){
        mStartX = rect.left;
        mStartY = rect.top;
        mEndX = rect.right;
        mEndY = rect.bottom;
        mDrawRect = true;
        if( mInit ){
            invalidate();
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        //super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        setMeasuredDimension(mWidth, mHeight);
    }

    @Override
    public boolean onTouchEvent(final MotionEvent event) {

        // TODO: be aware of multi-touches
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                mDrawRect = false;
                mStartX = (int) event.getX();
                mStartY = (int) event.getY();
                invalidate();
                break;

            case MotionEvent.ACTION_MOVE:
                final int x = (int) event.getX();
                final int y = (int) event.getY();

                if (!mDrawRect || Math.abs(x - mEndX) > 5 || Math.abs(y - mEndY) > 5) {
                    mEndX = x;
                    if(nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v1 )
                        mEndY = mStartY + (mEndX - mStartX);
                    else
                        mEndY = mStartY + (mEndX - mStartX)*9/16;
                    invalidate();
                }

                mDrawRect = true;
                break;

            case MotionEvent.ACTION_UP:
                if (mCallback != null) {
                    mCallback.onRectFinished(new Rect(Math.min(mStartX, mEndX), Math.min(mStartY, mEndY),
                            Math.max(mEndX, mStartX), Math.max(mEndY, mStartY)));
                }
                invalidate();
                break;

            default:
                break;
        }

        return true;
    }

    @Override
    protected void onDraw(final Canvas canvas) {
        super.onDraw(canvas);

        if (mDrawRect) {
            //canvas.drawBitmap();
            canvas.drawRect(Math.min(mStartX, mEndX), Math.min(mStartY, mEndY),
                    Math.max(mEndX, mStartX), Math.max(mEndY, mStartY), mRectPaint);
            canvas.drawText("  (" + Math.abs(mStartX - mEndX) + ", " + Math.abs(mStartY - mEndY) + ")",
                    Math.max(mEndX, mStartX), Math.max(mEndY, mStartY), mTextPaint);
        }
    }
}
