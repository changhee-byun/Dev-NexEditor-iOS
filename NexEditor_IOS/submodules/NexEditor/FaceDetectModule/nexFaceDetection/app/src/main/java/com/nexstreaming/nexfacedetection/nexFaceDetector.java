package com.nexstreaming.nexfacedetection;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.RectF;
import android.util.Log;

import com.nexstreaming.nexvision.NexFaceDetector;
import com.nexstreaming.nexvision.NexVisionResult;

import java.util.List;

/**
 * Created by alex.kang on 2017-07-13.
 */

public class nexFaceDetector {
    private static final String LOG_TAG = "nexFaceDetector";
    Bitmap bm = null;
    NexFaceDetector FaceDet =  null;
    List<NexVisionResult> results = null;

    public static final int NEX_FACE_DETECTOR_FRONT = 0;
    public static final int NEX_FACE_DETECTOR_FRONT_LEFT_RIGHT = 1;
    public static final int NEX_FACE_DETECTOR_ALL = 2;

    public boolean init(String path, Context context) {
        Log.d(LOG_TAG, "init NexDetector");
        this.bm = NexImageLoader.loadBitmap(path, 800, 800, 800 * 800 * 4, Bitmap.Config.ARGB_8888).getBitmap();
        if(FaceDet == null) {
            Log.d(LOG_TAG,"Create new nexFaceDetector handle");
            FaceDet = new NexFaceDetector();
            return true;
        }
        return true;
    }

    public boolean detect(RectF rect) {
        if(null == bm)
            return false;
        Log.d(LOG_TAG, "detect NexDetector");
        if(bm == null){
            Log.d(LOG_TAG, "bitmap is null");
            return false;
        }
        float width = bm.getWidth();
        float height = bm.getHeight();
        if (null != FaceDet) {
            results = FaceDet.detect(bm);
            if (results == null || results.size() == 0) {
                Log.d(LOG_TAG, "NexDetector can not find Face detector result");
            }
        }


        if (results != null) {
            for (final NexVisionResult ret : results) {
                RectF bounds = new RectF();
                bounds.left = ret.getLeft() / width;
                bounds.top = ret.getTop() / height;
                bounds.right = ret.getRight() / width;
                bounds.bottom = ret.getBottom() / height;

                rect.union(bounds);
            }
            return true;
        }
        Log.d(LOG_TAG, "can not find faceinfos");
        return false;
    }

    public boolean deinit() {
        Log.d(LOG_TAG, "deinit NexDetector");
        if(FaceDet != null) {
            FaceDet.release();
        }
        return false;
    }
}
