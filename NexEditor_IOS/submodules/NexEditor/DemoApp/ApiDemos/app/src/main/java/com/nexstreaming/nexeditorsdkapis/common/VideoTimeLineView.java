package com.nexstreaming.nexeditorsdkapis.common;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.AsyncTask;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.util.SparseArray;
import android.view.MotionEvent;
import android.view.View;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Created by jeongwook.yoon on 2017-07-05.
 */

public class VideoTimeLineView extends View {
    private final static String TAG="VideoTimeLineView";
    private Paint paint;
    private Paint paint2;
    private boolean pressedLeft;
    private boolean pressedRight;
    private boolean pressedSeek;
    private float progressSeek;
    private float pressDx;
    private final static int kshowDurationMs = 2000;
    private final static int kCollectDurationMs = 2000;
    private VideoTimeLineViewChangeListener delegate;
    private AsyncTask<reloadFramesParams, Integer, Integer> currentTask;
    private static final Object sync = new Object();
    private int frameTimeOffset;
    private int frameWidth;
    private int frameHeight;
    private int framesToLoad;
    private int framesToDisplay;
    private float density = 1;

    private int loadedFrameMinTime = 0;
    private int loadedFrameMaxTime = 0;

    private int maxTime = 100;
    private int currentTime = 0;
    private int leftTime = 0;

    private List<Bitmap> mainThumbnails = new ArrayList<>();
    private SparseArray<Bitmap> detailThumbnails = new SparseArray<>();
    private VideoTimeLineViewCallbacks callbacks;

    private int viewMode = kViewMode_Full;

    public final static int kViewMode_Full = 0;
    public final static int kViewMode_Detail = 1;

    private final static int kProgressType_SeekBar = 0;
    private boolean forceUpdate = true;

    private float lastTouchDownX = 0;
    private boolean isRunLongPressCheck = false;
    private static final ExecutorService sReloadThreadExcutor = Executors.newSingleThreadExecutor();

    private boolean ignoreTouchMoveEvent = false;

    private Runnable mLongPress = new Runnable() {
        @Override
        public void run() {
            Log.d(TAG,"mLongPress check="+isRunLongPressCheck);
            if( isRunLongPressCheck ) {

                if (delegate != null) {
                    delegate.onLongHoldTouch(kProgressType_SeekBar, progressSeek, currentTime);
                }
                isRunLongPressCheck = false;
            }
        }
    };

    private Runnable mSidePress = new Runnable() {
        @Override
        public void run() {

            if( pressedLeft ){
                leftTime -= 30;
                if( leftTime <= 0 ){
                    leftTime = 0;
                    currentTime = 0;
                    Log.d(TAG,"mSidePress left leftTime="+leftTime);
                    invalidate();
                    if (delegate != null) {
                        delegate.onProgressChanged(kProgressType_SeekBar,progressSeek,currentTime,true);
                    }
                    getFrame(true);
                    return;
                }
                currentTime = leftTime;
                invalidate();
                if (delegate != null) {
                    delegate.onProgressChanged(kProgressType_SeekBar,progressSeek,currentTime,true);
                }
                getFrame(true);
                Log.d(TAG,"mSidePress left leftTime="+leftTime);
            }else if(pressedRight ){
                leftTime += 30;
                if( leftTime+kshowDurationMs >= maxTime){
                    leftTime = maxTime - kshowDurationMs;
                    currentTime = maxTime;
                    invalidate();
                    Log.d(TAG,"mSidePress right leftTime="+leftTime);
                    if (delegate != null) {
                        delegate.onProgressChanged(kProgressType_SeekBar,progressSeek,currentTime,true);
                    }
                    getFrame(false);
                    return;
                }
                currentTime = leftTime + kshowDurationMs;
                invalidate();
                if (delegate != null) {
                    delegate.onProgressChanged(kProgressType_SeekBar,progressSeek,currentTime,true);
                }
                getFrame(false);
                Log.d(TAG,"mSidePress right leftTime="+leftTime);
            }else{
                return;
            }
            postDelayed(mSidePress,50);
        }
    };

    public int getStartTime(){
        return leftTime;
    }

    public int getEndTime(){
       if( viewMode == kViewMode_Detail ){
            int time = leftTime + kshowDurationMs;
            if (time > maxTime) {
                time = maxTime;
            }
            return time;
       }
       return  maxTime;
    }

    public void setTotalPlayTime(int value){
        if( maxTime != value) {
            maxTime = value;
            clearFrames();
        }
    }

    public void setViewMode(int mode){
        if( viewMode !=  mode ){

            if( mode == kViewMode_Full){
                leftTime = 0;
                progressSeek = (float) currentTime/(float)maxTime;
            }else if( mode == kViewMode_Detail){

                leftTime = currentTime -kshowDurationMs/2;
                if( leftTime < 0 ){
                    leftTime = 0;
                }else if( leftTime + kshowDurationMs > maxTime ){
                    leftTime = maxTime - kshowDurationMs;
                    if( leftTime < 0 ){
                        leftTime = 0;
                    }
                }
                progressSeek = (float)(currentTime - leftTime)/ (float)kshowDurationMs;
            }else{
                return;
            }

            viewMode = mode;
            Log.d(TAG,"setViewMode="+mode);
            clearFrames();
            //reloadFrames(currentTime,0);
        }
    }

    public void setCurrentPlayTime(int value){
        if( viewMode == kViewMode_Full){

            currentTime = value;
            if( currentTime < 0 ){
                currentTime = 0;
            }else if( currentTime > maxTime ){
                currentTime = maxTime;
            }
            progressSeek = (float) currentTime/ (float) maxTime;
            invalidate();
        }
    }

    public interface VideoTimeLineViewCallbacks{
        int waitTimeMsToLoading();
        Bitmap onGetDetailThumbnail(int Time );
        Bitmap onGetMainThumbnail(int Time );
    }

    public interface VideoTimeLineViewChangeListener {
        void onProgressChanged(int type, float progress, int timeMs, boolean fromUser);
        void onStartTrackingTouch(int type, float progress, int timeMs);
        void onStopTrackingTouch(int type, float progress, int timeMs);
        void onLongHoldTouch(int type, float progress, int timeMs);
    }

    public VideoTimeLineView(Context context) {
        super(context);
        init(context);
    }

    public VideoTimeLineView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public VideoTimeLineView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context);
    }

    public void init(Context context){
        density = context.getResources().getDisplayMetrics().density;
        Log.d( TAG,"density="+ density);
        if( density < 1 ){
            density = 1;
        }
        paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setColor(0xffffffff);
        paint2 = new Paint();
        paint2.setColor(0xff00ffff);

    }

    public void setVideoTimeLineViewCallbacks(VideoTimeLineViewCallbacks callbacks){
        this.callbacks = callbacks;
        Log.d( TAG,"setVideoTimeLineViewCallbacks()");
    }

    public void setIgnoreTouchMoveEvent(boolean on){
        ignoreTouchMoveEvent = on;
        Log.d( TAG,"setIgnoreTouchMoveEvent()="+on);
    }

    private int dp(float value) {
        if (value == 0) {
            return 0;
        }
        return (int) Math.ceil(density * value);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event == null) {
            return false;
        }
        float x = event.getX();
        float y = event.getY();

        int width = getMeasuredWidth()-dp(36);
        //int startX = (int) (width * progressLeft) + dp(16);
        //int endX = (int) (width * progressRight) + dp(16);

        int startX = (int) (width * progressSeek) + dp(16);
        int endX = width+ dp(16);
        int additionWidth = dp(12);
        //Log.d( TAG,"onTouchEvent event="+event.getAction());
        if (event.getAction() == MotionEvent.ACTION_DOWN) {

            if (startX - additionWidth <= x && x <= startX + additionWidth && y >= 0 && y <= getMeasuredHeight()) {
                pressedSeek = true;
                pressDx = (int) (x - startX);
                lastTouchDownX = x;
                getParent().requestDisallowInterceptTouchEvent(true);
                invalidate();
                Log.d( TAG,"MotionEvent.ACTION_DOWN pressDx="+pressDx);
                if (delegate != null) {
                    delegate.onStartTrackingTouch(kProgressType_SeekBar,progressSeek,currentTime);
                }
                isRunLongPressCheck = true;
                postDelayed(mLongPress,1500);
                return true;
            }

                /*
            } else if (endX - additionWidth <= x && x <= endX + additionWidth && y >= 0 && y <= getMeasuredHeight()) {
                pressedRight = true;
                pressDx = (int) (x - endX);
                getParent().requestDisallowInterceptTouchEvent(true);
                invalidate();
                return true;
            }*/
        } else if (event.getAction() == MotionEvent.ACTION_UP || event.getAction() == MotionEvent.ACTION_CANCEL) {
            if (pressedSeek) {
                pressedSeek = false;
                pressedLeft = false;
                pressedRight = false;
                removeCallbacks(mSidePress);
                Log.d( TAG,"MotionEvent.ACTION_UP pressedSeek="+pressedSeek);
                if (delegate != null) {
                    delegate.onStopTrackingTouch(kProgressType_SeekBar,progressSeek,currentTime);
                }
                if( isRunLongPressCheck ){
                    isRunLongPressCheck = false;
                    removeCallbacks(mLongPress);
                }
                return true;
            }
            /*
            else if (pressedRight) {
                pressedRight = false;
                return true;
            }
            */
        } else if (event.getAction() == MotionEvent.ACTION_MOVE) {
            if( ignoreTouchMoveEvent ){
                Log.d( TAG,"MotionEvent.ACTION_MOVE ignore.");
                return true;
            }
            if (pressedSeek) {
                startX = (int) (x - pressDx);
                if( isRunLongPressCheck ) {
                    if ((x - additionWidth) < lastTouchDownX && lastTouchDownX < (x + additionWidth)) {
                        Log.d(TAG,"MotionEvent.ACTION_MOVE long press checking.");
                        return true;
                    }else{
                        isRunLongPressCheck = false;
                        removeCallbacks(mLongPress);
                        Log.d(TAG,"MotionEvent.ACTION_MOVE long press cancel.");
                        return true;
                    }
                }

                if (startX < dp(16)) {
                    startX = dp(16);
                    //Press left
                } else if (startX > endX) {
                    startX = endX;
                    //Press right
                }
                progressSeek = (float) (startX - dp(16)) / (float) width;
                if( viewMode == kViewMode_Full){
                    currentTime = (int) (maxTime * progressSeek);
                    if (delegate != null) {
                        delegate.onProgressChanged(kProgressType_SeekBar,progressSeek,currentTime,true);
                    }
                    invalidate();
                    Log.d( TAG,"MotionEvent.ACTION_MOVE main x="+x+", progressSeek="+progressSeek+", currentTime="+currentTime);
                    return true;
                }

                if( progressSeek == 0 ){
                    if( !pressedLeft ) {
                        if( pressedRight ){
                            pressedRight = false;
                            removeCallbacks(mSidePress);
                        }
                        pressedLeft = true;
                        postDelayed(mSidePress, 1000);
                        Log.d( TAG,"MotionEvent.ACTION_MOVE left, currentTime="+currentTime);
                    }
                    return true;
                }else if( progressSeek == 1 ){
                    if( !pressedRight ) {
                        if( pressedLeft ){
                            pressedLeft = false;
                            removeCallbacks(mSidePress);
                        }
                        pressedRight = true;
                        postDelayed(mSidePress, 1000);
                        Log.d( TAG,"MotionEvent.ACTION_MOVE right, currentTime="+currentTime);
                    }
                    return true;
                }else{
                    pressedLeft = false;
                    pressedRight = false;
                    Log.d( TAG,"MotionEvent.ACTION_MOVE cancel, currentTime="+currentTime);
                    removeCallbacks(mSidePress);
                }
                currentTime = leftTime+ (int) (kshowDurationMs * progressSeek);

                if (delegate != null) {
                    delegate.onProgressChanged(kProgressType_SeekBar,progressSeek,currentTime,true);
                }
                invalidate();
                Log.d( TAG,"MotionEvent.ACTION_MOVE detail x="+x+", progressSeek="+progressSeek+", currentTime="+currentTime);
                return true;
            }
        }
        return false;
    }

    public void setVideoTimeLineViewChangeListener(VideoTimeLineViewChangeListener delegate) {
        this.delegate = delegate;
    }

    private class reloadFramesParams{
        int currrentTime;
        int mode; //0 - init , 1 - get one

        public reloadFramesParams(int currrentTime, int mode) {
            this.currrentTime = currrentTime;
            this.mode = mode;
        }
    }

    private void getFrame( boolean left){
        int time = 0;
        if( left ){
            time = loadedFrameMinTime - frameTimeOffset;
            if( time < 0 ){
                time = 0;
            }
            reloadFrames(time,2);
        }else{
            time = loadedFrameMaxTime + frameTimeOffset;
            reloadFrames(time,1);
        }
    }

    private int getCollectStartTime(int curTime){
        int lowCollectTime = curTime - kCollectDurationMs;
        if( lowCollectTime < 0 ){
            return 0;
        }

        int count = lowCollectTime  / frameTimeOffset;
        return  frameTimeOffset * count;
    }

    private int getCollectEndTime(int curTime){
        int highCollectTime = curTime + kCollectDurationMs;
        if( highCollectTime > maxTime ){
            return maxTime;
        }

        if( highCollectTime < kCollectDurationMs *2 ){
            highCollectTime = kCollectDurationMs *2 - frameTimeOffset;
        }

        int count = highCollectTime  / frameTimeOffset;
        return  frameTimeOffset * count;
    }

    private void reloadFrames(final int currentTime , final int mode ) {
        if( callbacks == null ){
            return;
        }

        if( viewMode == kViewMode_Full && mainThumbnails.size() > 0 ){
            invalidate();
            return;
        }

        int delayMs = callbacks.waitTimeMsToLoading();
        if( delayMs > 0 ){
            Log.d(TAG,"reloadFrames delay="+delayMs);
            postDelayed(new Runnable() {
                @Override
                public void run() {
                    reloadFrames(currentTime, mode);
                }
            },delayMs);
            return;
        }

        if ( mode == 0 ) {
            frameHeight = dp(40);
            framesToDisplay = (getMeasuredWidth() - dp(16)) / frameHeight;

            frameWidth = (int) Math.ceil((float) (getMeasuredWidth() - dp(16)) / (float) framesToDisplay);
            frameTimeOffset =  kshowDurationMs / framesToDisplay;
            framesToLoad = framesToDisplay*2;
            //loadedFrameMinTime = params.startTime;
        }
        reloadFramesParams param = new reloadFramesParams(currentTime,mode);
        Log.d(TAG,"reloadFrames currentTask start.");
        currentTask = new AsyncTask<reloadFramesParams, Integer, Integer>() {
            private int currrentTime = 0;
            private int mode = 0;
            @Override
            protected Integer doInBackground(reloadFramesParams... objects) {
                currrentTime = objects[0].currrentTime;
                mode = objects[0].mode;
                Bitmap bitmap = null;
                Log.d(TAG,"reloadFrames() doInBackground");
                if (isCancelled()) {
                    Log.d(TAG, "reloadFrames() isCancelled() mode=" + mode);
                    return 0;
                }

                if( viewMode == kViewMode_Full ){
                    int offsetTime = maxTime/framesToDisplay;
                    for (int time = 0; time < maxTime; time += offsetTime) {
                        bitmap = callbacks.onGetMainThumbnail(time);
                        if (bitmap != null) {
                            Bitmap result = Bitmap.createBitmap(frameWidth, frameHeight, bitmap.getConfig());
                            Canvas canvas = new Canvas(result);
                            float scaleX = (float) frameWidth / (float) bitmap.getWidth();
                            float scaleY = (float) frameHeight / (float) bitmap.getHeight();
                            float scale = scaleX > scaleY ? scaleX : scaleY;
                            int w = (int) (bitmap.getWidth() * scale);
                            int h = (int) (bitmap.getHeight() * scale);
                            Rect srcRect = new Rect(0, 0, bitmap.getWidth(), bitmap.getHeight());
                            Rect destRect = new Rect((frameWidth - w) / 2, (frameHeight - h) / 2, w, h);
                            canvas.drawBitmap(bitmap, srcRect, destRect, null);
                            bitmap.recycle();
                            bitmap = result;
                            Log.d(TAG, "reloadFrames() put main bitmap time=" + time);
                            mainThumbnails.add(bitmap);
                        }
                    }
                }else if( viewMode == kViewMode_Detail) {
                    if( mode == 1 ) {
                        if( detailThumbnails.get(currrentTime) != null ){
                            return mode;
                        }
                        bitmap = callbacks.onGetDetailThumbnail(currrentTime);
                        if (bitmap != null) {
                            Bitmap result = Bitmap.createBitmap(frameWidth, frameHeight, bitmap.getConfig());
                            Canvas canvas = new Canvas(result);
                            float scaleX = (float) frameWidth / (float) bitmap.getWidth();
                            float scaleY = (float) frameHeight / (float) bitmap.getHeight();
                            float scale = scaleX > scaleY ? scaleX : scaleY;
                            int w = (int) (bitmap.getWidth() * scale);
                            int h = (int) (bitmap.getHeight() * scale);
                            Rect srcRect = new Rect(0, 0, bitmap.getWidth(), bitmap.getHeight());
                            Rect destRect = new Rect((frameWidth - w) / 2, (frameHeight - h) / 2, w, h);
                            canvas.drawBitmap(bitmap, srcRect, destRect, null);
                            bitmap.recycle();
                            bitmap = result;
                            Log.d(TAG, "reloadFrames() put detail right bitmap time=" + currrentTime);

                            if( leftTime > 2000 ){
                                if ( (leftTime - 1000) > loadedFrameMinTime) {
                                    detailThumbnails.remove(loadedFrameMinTime);
                                    Log.d(TAG, "reloadFrames() put detail right remove time=" + loadedFrameMinTime);
                                    loadedFrameMinTime += frameTimeOffset;
                                }
                            }

                            detailThumbnails.put(currrentTime, bitmap);
                            loadedFrameMaxTime = currrentTime;
                        }
                    }else if(mode == 2){
                        if( detailThumbnails.get(currrentTime) != null ){
                            return mode;
                        }

                        bitmap = callbacks.onGetDetailThumbnail(currrentTime);
                        if (bitmap != null) {
                            Bitmap result = Bitmap.createBitmap(frameWidth, frameHeight, bitmap.getConfig());
                            Canvas canvas = new Canvas(result);
                            float scaleX = (float) frameWidth / (float) bitmap.getWidth();
                            float scaleY = (float) frameHeight / (float) bitmap.getHeight();
                            float scale = scaleX > scaleY ? scaleX : scaleY;
                            int w = (int) (bitmap.getWidth() * scale);
                            int h = (int) (bitmap.getHeight() * scale);
                            Rect srcRect = new Rect(0, 0, bitmap.getWidth(), bitmap.getHeight());
                            Rect destRect = new Rect((frameWidth - w) / 2, (frameHeight - h) / 2, w, h);
                            canvas.drawBitmap(bitmap, srcRect, destRect, null);
                            bitmap.recycle();
                            bitmap = result;
                            Log.d(TAG, "reloadFrames() put detail left bitmap time=" + currrentTime);
                            if( (leftTime + kshowDurationMs + 1000) < loadedFrameMaxTime ) {
                                detailThumbnails.remove(loadedFrameMaxTime);
                                Log.d(TAG, "reloadFrames() put detail left remove time=" + loadedFrameMinTime);
                                loadedFrameMaxTime -= frameTimeOffset;
                            }

                            detailThumbnails.put(currrentTime, bitmap);

                            loadedFrameMinTime = currrentTime;
                        }
                    }else {
                        int startTime = 0;
                        int endTime = maxTime;
                        int lowCollectTime = currrentTime - kCollectDurationMs;
                        if( lowCollectTime > 0 ){
                            int count = lowCollectTime  / frameTimeOffset;
                            startTime =  frameTimeOffset * count;
                        }

                        int highCollectTime = currrentTime + kCollectDurationMs;
                        if( highCollectTime < maxTime ){
                            if( highCollectTime < kCollectDurationMs *2 ){
                                highCollectTime = kCollectDurationMs *2 - frameTimeOffset;
                            }
                            int count = highCollectTime  / frameTimeOffset;
                            endTime = frameTimeOffset * count;
                        }
                        boolean get1stThumbnail = false;
                        for (int time = startTime; time < endTime; time += frameTimeOffset) {
                            bitmap = callbacks.onGetDetailThumbnail(time);
                            if (bitmap != null) {
                                Bitmap result = Bitmap.createBitmap(frameWidth, frameHeight, bitmap.getConfig());
                                Canvas canvas = new Canvas(result);
                                float scaleX = (float) frameWidth / (float) bitmap.getWidth();
                                float scaleY = (float) frameHeight / (float) bitmap.getHeight();
                                float scale = scaleX > scaleY ? scaleX : scaleY;
                                int w = (int) (bitmap.getWidth() * scale);
                                int h = (int) (bitmap.getHeight() * scale);
                                Rect srcRect = new Rect(0, 0, bitmap.getWidth(), bitmap.getHeight());
                                Rect destRect = new Rect((frameWidth - w) / 2, (frameHeight - h) / 2, w, h);
                                canvas.drawBitmap(bitmap, srcRect, destRect, null);
                                bitmap.recycle();
                                bitmap = result;
                                Log.d(TAG, "reloadFrames() put detail bitmap time=" + time);
                                detailThumbnails.put(time, bitmap);
                                if( !get1stThumbnail ){
                                    get1stThumbnail = true;
                                    loadedFrameMinTime = time;
                                }
                                loadedFrameMaxTime = time;
                            }
                        }
                    }
                }
                return mode;
            }

            @Override
            protected void onPostExecute(Integer integer) {
                super.onPostExecute(integer);
                if( mode == 0 ) {
                    invalidate();
                }
            }
        };
        currentTask.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, param, null, null);
    }

    public void destroy() {
        if (currentTask != null) {
            currentTask.cancel(true);
            currentTask = null;
        }

        for (int i = 0; i < mainThumbnails.size(); i++) {
            Bitmap bitmap = mainThumbnails.get(i);
            if (bitmap != null) {
                bitmap.recycle();
            }
        }
        mainThumbnails.clear();

        for(int i = 0 ; i < detailThumbnails.size() ; i++  ){
            Bitmap bitmap = detailThumbnails.valueAt(i);
            if (bitmap != null) {
                bitmap.recycle();
            }
        }
        detailThumbnails.clear();
    }

    public void clearFrames() {
        if (currentTask != null) {
            currentTask.cancel(true);
            currentTask = null;
        }

        forceUpdate = true;

        if( viewMode == kViewMode_Full) {
            for (int i = 0; i < mainThumbnails.size(); i++) {
                Bitmap bitmap = mainThumbnails.get(i);
                if (bitmap != null) {
                    bitmap.recycle();
                }
            }
            mainThumbnails.clear();
        }else if( viewMode == kViewMode_Detail) {

            for (int i = 0; i < detailThumbnails.size(); i++) {
                Bitmap bitmap = detailThumbnails.valueAt(i);
                if (bitmap != null) {
                    bitmap.recycle();
                }
            }
            detailThumbnails.clear();
        }
        invalidate();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int width = View.MeasureSpec.getSize(widthMeasureSpec);
        int height = View.MeasureSpec.getSize(heightMeasureSpec);
        if (View.MeasureSpec.getMode(widthMeasureSpec) != View.MeasureSpec.EXACTLY) {
            height = dp(46);
        }
        setMeasuredDimension(width, height);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        int width = getMeasuredWidth() - dp(36);
        int startX = (int) (width * progressSeek) + dp(16);

        if( callbacks == null ){
            super.onDraw(canvas);
            return;
        }
        Log.d(TAG,"onDraw() forceUpdate="+forceUpdate+", viewMode="+viewMode);
        canvas.save();
        canvas.drawRect(0,0,getMeasuredWidth(),dp(46), paint2);
        canvas.clipRect(dp(16), 0, width + dp(20), dp(44));

        if (forceUpdate ) {
            forceUpdate = false;
            reloadFrames(currentTime,0);
        } else {

            int startTime = leftTime - frameTimeOffset;
            if (startTime < 0)
                startTime = 0;

            int endTime = leftTime + kshowDurationMs + frameTimeOffset;
            if (endTime > maxTime) {
                endTime = maxTime;
            }

            if (viewMode == kViewMode_Full) {
                int offset = 0;
                for (int i = 0; i < mainThumbnails.size(); i++) {
                    Bitmap bitmap = mainThumbnails.get(i);
                    if (bitmap != null) {
                        canvas.drawBitmap(bitmap, dp(16)+ offset * frameWidth, dp(2), null);
                    }
                    offset++;
                }
            } else if (viewMode == kViewMode_Detail) {

                Log.d(TAG, "onDraw() thumbnails size=" + detailThumbnails.size());
                if( detailThumbnails.size() > 0 ) {
                    ArrayList<Integer> keyTimes = new ArrayList<>();
                    for (int a = 0; a < detailThumbnails.size(); a++) {
                        if (detailThumbnails.keyAt(a) >= startTime && detailThumbnails.keyAt(a) <= endTime) {
                            keyTimes.add(detailThumbnails.keyAt(a));
                        }
                    }

                    int offset = 0;
                    float startPos =  dp(16);
                    boolean calcStartPos = false;
                    Collections.sort(keyTimes);
                    for (int keytime : keyTimes) {
                        if (!calcStartPos) {
                            calcStartPos = true;
                            if (leftTime > keytime) {
                                float diff = leftTime - keytime;
                                startPos = ((float) frameWidth * diff / (float) frameTimeOffset) * -1;
                                startPos += dp(16);
                            }
                        }
                        Log.d(TAG, "onDraw() bitmap time=" + keytime + ", offset=" + offset + ",leftTime=" + leftTime + ",startPos=" + startPos);
                        Bitmap bitmap = detailThumbnails.get(keytime);
                        if (bitmap != null) {
                            canvas.drawBitmap(bitmap, startPos + offset * frameWidth, dp(2), null);
                        }
                        offset++;
                    }
                }
            }
        }
        canvas.drawRect(startX, 0, startX + dp(2), dp(44), paint);
        canvas.restore();
        canvas.drawCircle(startX, getMeasuredHeight() / 2, dp(7), paint);
    }
}
