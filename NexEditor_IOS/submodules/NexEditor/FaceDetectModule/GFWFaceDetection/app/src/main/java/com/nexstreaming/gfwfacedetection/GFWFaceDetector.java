package com.nexstreaming.gfwfacedetection;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.PointF;
import android.graphics.RectF;
import android.util.Log;

/**
 * Created by alex.kang on 2017-07-13.
 */

public class GFWFaceDetector {
    private static final String LOG_TAG = "GFWFaceDetector";
    Bitmap bm;
    android.media.FaceDetector.Face[] faces = null;
    private int numFaces;

    public boolean init(String path, Context context) {
        Log.d(LOG_TAG, "init GFWFaceDetector");
        this.bm = NexImageLoader.loadBitmap(path, 512, 512, 512 * 512 * 4, Bitmap.Config.RGB_565).getBitmap();
        if(faces == null) {
            Log.d(LOG_TAG,"Create new GFWFaceDetector handle");
            faces = new android.media.FaceDetector.Face[32];
        }
        return true;
    }

    public boolean detect(RectF rect) {
        if(null == bm)
            return false;
        Log.d(LOG_TAG, "detect GFWFaceDetector");
        if(bm.getWidth() % 2 == 1){
            bm = Bitmap.createScaledBitmap(bm, bm.getWidth() + 1, bm.getHeight(), false);
        }
        if( bm==null )
            return false;

        android.media.FaceDetector fd = new android.media.FaceDetector(bm.getWidth(), bm.getHeight(), 32);
        int numFaces = fd.findFaces(bm, faces);
        this.numFaces = 0;

        RectF bounds = new RectF();
        PointF eyes = new PointF();
        float width = bm.getWidth();
        float height = bm.getHeight();
        float faceMinWidth = width / 30;
        float faceMinHeight = height / 30;
        for (int i = 0; i < numFaces; i++) {
            if (faces[i].confidence() < 0.4)
                continue;

            faces[i].getMidPoint(eyes);
            float d = faces[i].eyesDistance();
            if (d < faceMinWidth || d < faceMinHeight)
                continue;
            this.numFaces++;
            bounds.set((eyes.x - d / 2) / width, (eyes.y - d / 3) / height, (eyes.x + d / 2) / width, (eyes.y + d / 3 * 2) / height);
            rect.union(bounds);
        }
        bm.recycle();

        if (!rect.isEmpty()) {
            float overflowWidth = 0.6f - rect.width();
            float overflowHeight = 0.6f - rect.height();
            if (overflowWidth > 0) {
                rect.left -= overflowWidth / 2;
                rect.right += overflowWidth / 2;
            }
            if (overflowHeight > 0) {
                rect.top -= overflowHeight / 2;
                rect.bottom += overflowHeight / 2;
            }
            return true;
        }
        Log.d(LOG_TAG, "can not find faceinfos");
        return false;
    }

    public boolean deinit() {
        Log.d(LOG_TAG, "deinit GFWFaceDetector");
        return false;
    }
}
