package com.nexstreaming.kminternal.kinemaster.utils.facedetect;

import android.graphics.Bitmap;
import android.graphics.PointF;
import android.graphics.Rect;
import android.graphics.RectF;
import android.media.FaceDetector;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;

/**
 * Created by jeongwook.yoon on 2017-02-22.
 */

public class FaceFind {

    public int findFaces(Bitmap srcRGB565, RectF[] retFaces){
        if( srcRGB565 == null ){
            return 0;
        }

        if( retFaces == null ){
            return 0;
        }

        int num = retFaces.length;

        FaceDetector.Face[] faces = new FaceDetector.Face[num];
        FaceDetector fd = new FaceDetector(srcRGB565.getWidth(), srcRGB565.getHeight(), num);
        int numFaces = fd.findFaces(srcRGB565, faces);
        int count = 0;

        RectF bounds = new RectF();
        PointF eyes = new PointF();
        float width = srcRGB565.getWidth();
        float height = srcRGB565.getHeight();
        float faceMinWidth = width / 30;
        float faceMinHeight = height / 30;

        for( int i=0; i<numFaces; i++ ) {

            //Log.d(LOG_TAG,"face : " + i + " c=" + faces[i].confidence() + " e=" + faces[i].eyesDistance() + " faceMinWidth=" + faceMinWidth + " faceMinHeight=" + faceMinHeight);
            if( faces[i].confidence() < 0.4 )
                continue;

            faces[i].getMidPoint(eyes);
            float d = faces[i].eyesDistance();
            if( d < faceMinWidth || d < faceMinHeight )
                continue;

            bounds.set((eyes.x - d/2)/width, (eyes.y-d/3)/height, (eyes.x+d/2)/width, (eyes.y+d/3*2)/height);
            retFaces[count] = new RectF(bounds);
            count++;
        }
        return count;
    }

    public RectF getUnionRectF(RectF[] faces, int numFace){
        RectF faceBounds = new RectF();

        for( int i = 0 ; i <  numFace ; i++ ){
            faceBounds.union(faces[i]);
        }

        if( !faceBounds.isEmpty() ) {
            float overflowWidth = 0.6f - faceBounds.width();
            float overflowHeight = 0.6f - faceBounds.height();
            if( overflowWidth > 0 ) {
                faceBounds.left -= overflowWidth/2;
                faceBounds.right += overflowWidth/2;
            }
            if( overflowHeight > 0 ) {
                faceBounds.top -= overflowHeight/2;
                faceBounds.bottom += overflowHeight/2;
            }
        }
        return faceBounds;
    }

    public Rect screenRect(RectF faceUnion, int imageWidth, int imageHeight, float aspect ){
        int addSpace;
        Rect ret = new Rect();
        ret.set((int)(faceUnion.left*imageWidth), (int)(faceUnion.top*imageHeight), (int)(faceUnion.right*imageWidth), (int)(faceUnion.bottom*imageHeight));
        addSpace = (imageWidth/4)-ret.width();
        if( addSpace>=2 ) {
            ret.left -= addSpace/2;
            ret.right += addSpace/2;
        }
        addSpace = (imageHeight/4)-ret.height();
        if( addSpace>=2 ) {
            ret.top -= addSpace/2;
            ret.bottom += addSpace/2;
        }
        //growToAspect(zoombounds, 16.0f / 9.0f);
        growToAspect(ret, aspect);
        if(!ret.intersect(0, 0, imageWidth, imageHeight)) {
            ret.set(0, 0, imageWidth*2/3, imageHeight*2/3);
            ret.offset((int)(imageWidth*Math.random()/3), (int)(imageHeight*Math.random()/3));
        }

        shrinkToAspect(ret, aspect);
        return ret;
    }

    private void shrinkToAspect( Rect rc, float aspect ) {
        float originalAspect = ((float)rc.width()) / (float)(rc.height());
        if( originalAspect < aspect ) {
            // Original too tall
            int newHeight = (int)(((float)rc.width())/aspect);
            int ctry = rc.centerY();
            rc.top = ctry - newHeight/2;
            rc.bottom = rc.top + newHeight;
        } else {
            // Original too wide
            int newWidth = (int)(((float)rc.height())*aspect);
            int ctrx = rc.centerX();
            rc.left = ctrx - newWidth/2;
            rc.right = rc.left + newWidth;
        }
    }

    private void growToAspect( Rect rc, float aspect ) {
        float originalAspect = ((float)rc.width()) / (float)(rc.height());
        if( originalAspect < aspect ) {
            // Original too tall
            int newWidth = (int)(((float)rc.height())*aspect);
            int ctrx = rc.centerX();
            rc.left = ctrx - newWidth/2;
            rc.right = rc.left + newWidth;
        } else {
            // Original too wide
            int newHeight = (int)(((float)rc.width())/aspect);
            int ctry = rc.centerY();
            rc.top = ctry - newHeight/2;
            rc.bottom = rc.top + newHeight;
        }
    }

}
