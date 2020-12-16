/******************************************************************************
 * File Name        : nexCrop.java
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

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.PointF;
import android.graphics.Rect;
import android.graphics.RectF;
import android.media.FaceDetector;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.nexvideoeditor.nexEditorUtils;
import com.nexstreaming.kminternal.kinemaster.utils.facedetect.FaceInfo;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;
import com.nexstreaming.kminternal.nexvideoeditor.NexRectangle;

import java.io.File;
import java.util.ArrayList;
import java.util.List;


/**
 * This class defines how the NexEditor&trade;&nbsp;SDK crops and actually displays video and image clips in a project.
 * 
 * When the start and end positions of a "crop" have been designated on a clip, the relevant clip will play
 * from the start position of the crop to the end position of the crop.  For images in a project, this creates a Ken Burns effect.
 *  
 * The start and end of a crop are defined in terms of the original size of the clip.
 * The resolution of the cropped clip will be increased or decreased appropriately when the project is previewed or exported.

 * <p>Example code :</p>
 int wid = mEngine.getProject().getClip(0, true).getCrop().getWidth();
 int hei = mEngine.getProject().getClip(0, true).getCrop().getHeight();
 int rot = mEngine.getProject().getClip(0, true).getCrop().getRotate();

 if( rot == 90 || rot == 270 ) {
    mRatio = 360f / (float) wid;
    int tmp = wid;
    wid = hei;
    hei = tmp;
 }else{
    mRatio = 360f / (float) hei;
 }

 Rect startRect = new Rect();
 mEngine.getProject().getClip(0, true).getCrop().getStartPosition(startRect);

 startRect.left = (int) (startRect.left * mRatio);
 startRect.top = (int) (startRect.top * mRatio);
 startRect.right = (int) (startRect.right * mRatio);
 startRect.bottom = (int) (startRect.bottom * mRatio);

 Rect endRect = new Rect();
 mEngine.getProject().getClip(0, true).getCrop().getEndPosition(endRect);

 endRect.left = (int) (endRect.left * mRatio);
 endRect.top = (int) (endRect.top * mRatio);
 endRect.right = (int) (endRect.right * mRatio);
 endRect.bottom = (int) (endRect.bottom * mRatio);
 * @since version 1.1.0
 */
public final class nexCrop implements Cloneable {
//    nexClip mClip;

    final static String TAG = "nexCrop";
    int m_startPositionLeft = 0;
    int m_startPositionBottom = 0;
    int m_startPositionRight = 0;
    int m_startPositionTop = 0;
    int m_endPositionLeft = 0;
    int m_endPositionBottom = 0;
    int m_endPositionRight = 0;
    int m_endPositionTop = 0;

    int m_rotatedStartPositionLeft = 0;
    int m_rotatedStartPositionBottom = 0;
    int m_rotatedStartPositionRight = 0;
    int m_rotatedStartPositionTop = 0;
    int m_rotatedEndPositionLeft = 0;
    int m_rotatedEndPositionBottom = 0;
    int m_rotatedEndPositionRight = 0;
    int m_rotatedEndPositionTop = 0;

    int m_facePositionLeft = 0;
    int m_facePositionTop = 0;
    int m_facePositionRight = 0;
    int m_facePositionBottom = 0;
    int m_rotatedFacePositionLeft = 0;
    int m_rotatedFacePositionTop = 0;
    int m_rotatedFacePositionRight = 0;
    int m_rotatedFacePositionBottom = 0;
    private boolean m_startLock = false;
    private boolean m_endLock = false;

    private float[] m_startMatrix;
    private float[] m_endMatrix;

    int m_rotation = 0;
    private int m_width = 0;
    private int m_height = 0;
    final private String m_mediaPath;

    private float m_faceBounds_left;
    private float m_faceBounds_top;
    private float m_faceBounds_right;
    private float m_faceBounds_bottom;
    private boolean m_faceBounds_set;


    private boolean isSolid() {
        return m_mediaPath!=null && (m_mediaPath.startsWith("@solid:") && m_mediaPath.endsWith(".jpg"));
    }

    protected static nexCrop clone(nexCrop src) {
        //Log.d("clone", "clone work./nexCrop");
        nexCrop object = null;
        try {
            object = (nexCrop)src.clone();
        } catch (CloneNotSupportedException e) {
            e.printStackTrace();
        }
        return object;
    }

    /**
     * The value of the "Abstract" dimension used when cropping.
     * 
     * @since version 1.1.0
     */
    public static final int ABSTRACT_DIMENSION = 100000;

    nexCrop(String path, int width, int height, int degree, int duration){
        m_mediaPath = path;
        m_width = width;
        m_height = height;
        m_rotation = degree;
        clip_druation = duration;
        randomizeStartEndPosition(false, CropMode.PAN_RAND);
    }

    void setRotate(int degree ){
        if( degree != m_rotation ) {
            m_rotation = degree;
            if (m_rotation == 90 || m_rotation == 270) {
                if (m_rotatedStartPositionLeft == 0 && m_rotatedStartPositionBottom == 0 && m_rotatedStartPositionRight == 0 && m_rotatedStartPositionTop == 0) {
                    randomizeStartEndPosition(false, CropMode.PAN_RAND);
                }
            } else {
                if (m_startPositionLeft == 0 && m_startPositionBottom == 0 && m_startPositionRight == 0 && m_startPositionTop == 0) {
                    randomizeStartEndPosition(false, CropMode.PAN_RAND);
                }
            }
        }
    }

    /**
     * This method gets the degrees of rotation on a <tt>nexClip</tt> instance of a rotated clip where a crop will be performed.
     * 
     * This reflects the same value that is set by <tt> setRotateDegree(int)</tt> in <tt>nexClip</tt>.
     * 
     * <p>Example code:</p>
     *     {@code
                int rot = mEngine.getProject().getClip(0, true).getCrop().getRotate();
            }
     * @return  The degrees of rotation of the clip, where 0 is no rotation and for example 180 would mean the clip is upside down.
     * 
     * @see nexClip#getRotateDegree()
     * @since version 1.1.0
     */
    public int getRotate( ){
        return m_rotation;
    }

    /**
     * This method gets the width of the original clip, before a crop.
     * 
     * <p>Example code:</p>
     *     {@code
                int wid = mEngine.getProject().getClip(0, true).getCrop().getWidth();
            }
     * @return  The original width of the clip, as an <tt>integer</tt>.
     * 
     * @see #getHeight()
     * @see nexClip#getWidth()
     * @since version 1.1.0
     */
    public int getWidth() {
        return m_width;
    }

    /**
     * This method gets the height of the original clip, before a crop.
     * 
     * <p>Example code:</p>
     *     {@code
                int hei = mEngine.getProject().getClip(0, true).getCrop().getHeight();
            }
     * @return  The original height of the clip, as an <tt>integer</tt>.
     * 
     * @see #getWidth()
     * @see nexClip#getHeight()
     * @since version 1.1.0
     */
    public int getHeight() {
        return m_height;
    }

    private Matrix matrixForRect( Rect rc ) {
        return matrixForRect(rc.left, rc.top, rc.right, rc.bottom);
    }

    private Matrix matrixForRect( float left, float top, float right, float bottom ) {
        Matrix m = new Matrix();
        m.postTranslate( -left, -top);
        m.postScale(EditorGlobal.PROJECT_ASPECT_RATIO/(right-left), 1/(bottom-top));
        return m;
    }

    /**
     * This enumeration defines the possible <tt> CropMode</tt> options when a clip is cropped.
     * 
     * These options are possible values for the <tt> cropMode</tt> parameter of the <tt>randomizeStartEndPosition()</tt> method.
     * 
     * The possible modes for a crop are:
     *  <ul>
     *  <li><b> FIT:</b> The largest dimension of the original clip will be fit to the project display, without any distortion of the
     *       original dimensions (and the rest of the display will display black).
     *  <li><b> FILL:</b> The clip will be cropped to fill the project display, and the original may be distorted.
     *  <li><b> PAN_RAND:</b> The start and end positions of the crop will be randomly selected.
     *  <li><b> PAN_FACE:</b> Not currently available.
     *  </ul>
     * 
     * @see #randomizeStartEndPosition(boolean, com.nexstreaming.nexeditorsdk.nexCrop.CropMode)
     * @since version 1.1.0
     * 
     */
    public enum CropMode {
        FIT, FILL, PAN_RAND, PAN_FACE, NONE, PANORAMA ;
    }

    public void shrinkToAspect( Rect rc, float aspect ) {
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

    public void growToAspect( Rect rc, float aspect ) {
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

    /**
     * This method randomizes the start and end crops of a clip, based on the chosen crop mode.
     * 
     * By default, the NexEditor&trade;&nbsp;SDK randomly selects the start and end positions of crops on clips added, as if the
     * <tt>cropMode</tt> parameter is set to <tt>PAN_RAND</tt>.
     * 
     * @param faceDetection  Crop to sections where faces are detected in the clip (Not currently available but planned for future versions).
     * @param cropMode  The crop mode to use when randomly setting the start and end of a crop.  This will be one of the following CropMode options: cropMode
     *                  <ul>
     *                   <li><b> FIT:</b> The largest dimension of the original clip will be fit to the project display, without any distortion of the
     *                     original dimensions (and the rest of the display will display black).
     *                   <li><b> FILL:</b> The clip will be cropped to fill the project display, and the original may be distorted.
     *                   <li><b> PAN_RAND:</b> The start and end positions of the crop will be randomly selected.
     *                   <li><b> PAN_FACE:</b> Not currently available.
     *                  </ul>
     * <p>Example code :</p>
     *  {@code mEngine.getProject().getClip(0, true).getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FIT) }
     * @see com.nexstreaming.nexeditorsdk.nexCrop.CropMode
     * @since version 1.1.0
     */
    public void randomizeStartEndPosition( boolean faceDetection, CropMode cropMode ) {
        Rect startPos = new Rect();
        Rect endPos = new Rect();
        getStartPosition(startPos);
        getEndPosition(endPos);
        int width, height;
        if( m_rotation==90 || m_rotation==270 ) {
            width = getHeight();
            height = getWidth();
        } else {
            width = getWidth();
            height = getHeight();
        }

        if( width < 1 || height < 1 )
            return;

        if( isSolid() ) {
            Rect cropBounds = new Rect(2,2,width-2,height-2);
            setStartPosition(cropBounds);
            setEndPosition(cropBounds);
            return;
        }


        if( cropMode==CropMode.FILL ) {
            Rect fullbounds = new Rect(0, 0, width, height);
            //shrinkToAspect(fullbounds, 16.0f / 9.0f);
            shrinkToAspect(fullbounds, nexApplicationConfig.getAspectRatio());
            setStartPosition(fullbounds);
            setEndPosition(fullbounds);
            return;
        } else if( cropMode==CropMode.FIT ) {
            Rect fullbounds = new Rect(0, 0, width, height);
            //growToAspect(fullbounds, 16.0f / 9.0f);
            growToAspect(fullbounds, nexApplicationConfig.getAspectRatio());
            setStartPosition(fullbounds);
            setEndPosition(fullbounds);
            return;
        } else if( cropMode == CropMode.PANORAMA){
            float contentRatio = width / height;
            if(  contentRatio < nexApplicationConfig.getAspectRatio() ){
                int newHeight = (int)((float)width / nexApplicationConfig.getAspectRatio());
                Rect startBound = new Rect(0, 0, width,newHeight );
                Rect endBound = new Rect(0 , height- newHeight, width, height );
                if( Math.random() < 0.5 ) {
                    setStartPosition(startBound);
                    setEndPosition(endBound);
                } else {
                    setStartPosition(endBound);
                    setEndPosition(startBound);
                }
                return;
            }else if( contentRatio > nexApplicationConfig.getAspectRatio() ){
                int newWidth = (int)((float)height * nexApplicationConfig.getAspectRatio());
                Rect startBound = new Rect(0, 0, newWidth, height);
                Rect endBound = new Rect(width - newWidth, 0 , width, height );
                if( Math.random() < 0.5 ) {
                    setStartPosition(startBound);
                    setEndPosition(endBound);
                } else {
                    setStartPosition(endBound);
                    setEndPosition(startBound);
                }
                return;

            }else{
                Rect fullbounds = new Rect(0, 0, width, height);
                //shrinkToAspect(fullbounds, 16.0f / 9.0f);
                shrinkToAspect(fullbounds, nexApplicationConfig.getAspectRatio());
                setStartPosition(fullbounds);
                setEndPosition(fullbounds);
                return;
            }
        }

        Rect zoombounds = new Rect();
//        if( faceDetection && cropMode==CropMode.PAN_FACE )
//            getFaceBounds(zoombounds);

        if( zoombounds.isEmpty() ) {
            zoombounds.set(0, 0, width*3/4, height*3/4);
            zoombounds.offset((int)(width*Math.random()/4), (int)(height*Math.random()/4));
        } else {
            int addSpace;
            addSpace = (width/4)-zoombounds.width();
            if( addSpace>=2 ) {
                zoombounds.left -= addSpace/2;
                zoombounds.right += addSpace/2;
            }
            addSpace = (height/4)-zoombounds.height();
            if( addSpace>=2 ) {
                zoombounds.top -= addSpace/2;
                zoombounds.bottom += addSpace/2;
            }
            //growToAspect(zoombounds, 16.0f / 9.0f);
            growToAspect(zoombounds, nexApplicationConfig.getAspectRatio());
            if(!zoombounds.intersect(0, 0, width, height)) {
                zoombounds.set(0, 0, width*2/3, height*2/3);
                zoombounds.offset((int)(width*Math.random()/3), (int)(height*Math.random()/3));
            }
        }
        //shrinkToAspect(zoombounds, 16.0f / 9.0f);
        shrinkToAspect(zoombounds, nexApplicationConfig.getAspectRatio());
        Rect fullbounds = new Rect(0, 0, width, height);
        int speed = getFaceDetectSpeed();
        applyCropSpeed(fullbounds, zoombounds, width, height, speed, getClipDuration());

        shrinkToAspect(fullbounds, nexApplicationConfig.getAspectRatio());
        shrinkToAspect(zoombounds, nexApplicationConfig.getAspectRatio());

        if( !zoombounds.isEmpty() ) {
            if( Math.random() < 0.5 ) {
                setStartPosition(fullbounds);
                setEndPosition(zoombounds);
            } else {
                setStartPosition(zoombounds);
                setEndPosition(fullbounds);
            }
        }

        Math.random();
    }

    public int faceStartEndPosition(int duration){
        Rect zoombounds = new Rect();

        Bitmap bm = loadImageForFaceRecognition();
        if( bm==null ) {
            if( LL.D ) Log.d(TAG,"faceStartEndPosition fail!");
            return 0;
        }
        FaceDetector.Face[] faces = new FaceDetector.Face[32];
        FaceDetector fd = new FaceDetector(bm.getWidth(), bm.getHeight(), 32);
        int numFaces = fd.findFaces(bm, faces);

        if( numFaces  == 0 ){
            if( LL.D ) Log.d(TAG,"faceStartEndPosition no face");
            return 0;
        }

        //NexRectangle[] faceInfos = new NexRectangle[numFaces];
        List<RectF> faceDetects = new ArrayList<>();
        //RectF bounds = new RectF();
        PointF eyes = new PointF();
        float width = getWidth();
        float height = getHeight();
        float faceMinWidth = width / 30;
        float faceMinHeight = height / 30;
        for( int i=0; i<numFaces; i++ ) {

            if( LL.D ) Log.d(TAG,"face : " + i + " c=" + faces[i].confidence() + " e=" + faces[i].eyesDistance() + " faceMinWidth=" + faceMinWidth + " faceMinHeight=" + faceMinHeight);
            if( faces[i].confidence() < 0.4 )
                continue;

            faces[i].getMidPoint(eyes);
            float d = faces[i].eyesDistance();
//            if( d < faceMinWidth || d < faceMinHeight )
//                continue;
            RectF bounds = new RectF();
            bounds.set((eyes.x - d/2)/width, (eyes.y-d/3)/height, (eyes.x+d/2)/width, (eyes.y+d/3*2)/height);
            faceDetects.add(bounds);
/*
            //--->
            if( LL.D ) Log.d(TAG,"face bounds : " + bounds);
            float overflowWidth = 0.6f - bounds.width();
            float overflowHeight = 0.6f - bounds.height();
            if( overflowWidth > 0 ) {
                bounds.left -= overflowWidth/2;
                bounds.right += overflowWidth/2; 
            }
            if( overflowHeight > 0 ) {
                bounds.top -= overflowHeight/2;
                bounds.bottom += overflowHeight/2;
            }
            if( LL.D ) Log.d(TAG,"face bounds (final) : " + bounds);
            bounds.left = Math.max(0,bounds.left);
            bounds.top = Math.max(0,bounds.top);
            bounds.right = Math.min(bounds.right,1);
            bounds.bottom = Math.min(bounds.bottom,1);
*/
            //----->



            //faceInfos[i] = new NexRectangle((int)(bounds.left*ABSTRACT_DIMENSION),(int)(bounds.top*ABSTRACT_DIMENSION),(int)(bounds.right*ABSTRACT_DIMENSION),(int)(bounds.bottom*ABSTRACT_DIMENSION) );
            //if( LL.D ) Log.d(TAG,"loadImageForFaceRecognition face=("+faceInfos[i].mLeft+","+faceInfos[i].mTop+","+faceInfos[i].mRight+","+faceInfos[i].mBottom+")");
        }
        bm.recycle();

        numFaces = faceDetects.size();

        if( numFaces  == 0 ){
            if( LL.D ) Log.d(TAG,"faceStartEndPosition no face");
            return 0;
        }

        NexRectangle[] faceInfos = new NexRectangle[numFaces];
        for( int i = 0 ; i < numFaces ; i++  ){
            faceInfos[i] = new NexRectangle((int)(faceDetects.get(i).left*ABSTRACT_DIMENSION),(int)(faceDetects.get(i).top*ABSTRACT_DIMENSION),(int)(faceDetects.get(i).right*ABSTRACT_DIMENSION),(int)(faceDetects.get(i).bottom*ABSTRACT_DIMENSION) );
        }

        NexRectangle[] result = new NexRectangle[2];
        result[0] = new NexRectangle(0,0,0,0);
        result[1] = new NexRectangle(0,0,0,0);
        int[] aspect = nexApplicationConfig.getAspectProfile().aspect2ReduceFraction();
        nexEditorUtils.getKenBurnsRects(m_width,m_height,faceInfos,aspect[0],aspect[1],duration,result);
        Rect rect = new Rect(result[0].mLeft,result[0].mTop,result[0].mRight,result[0].mBottom);
        setStartPosition(rect);
        rect.set(result[1].mLeft,result[1].mTop,result[1].mRight,result[1].mBottom);
        setEndPosition(rect);
        return numFaces;
    }

    public void randomizeStartEndPosition(boolean faceDetection, CropMode cropMode, float ratio ) {
        Rect startPos = new Rect();
        Rect endPos = new Rect();
        getStartPosition(startPos);
        getEndPosition(endPos);
        int width, height;
        if( m_rotation==90 || m_rotation==270 ) {
            width = getHeight();
            height = getWidth();
        } else {
            width = getWidth();
            height = getHeight();
        }

        if( width < 1 || height < 1 )
            return;

        if( isSolid() ) {
            Rect cropBounds = new Rect(2,2,width-2,height-2);
            setStartPosition(cropBounds);
            setEndPosition(cropBounds);
            return;
        }


        if( cropMode==CropMode.FILL ) {
            Rect fullbounds = new Rect(0, 0, width, height);
            //shrinkToAspect(fullbounds, 16.0f / 9.0f);
            shrinkToAspect(fullbounds, ratio);
            setStartPosition(fullbounds);
            setEndPosition(fullbounds);
            return;
        } else if( cropMode==CropMode.FIT ) {
            Rect fullbounds = new Rect(0, 0, width, height);
            //growToAspect(fullbounds, 16.0f / 9.0f);
            growToAspect(fullbounds, ratio);
            setStartPosition(fullbounds);
            setEndPosition(fullbounds);
            return;
        } else if( cropMode == CropMode.PANORAMA){
            float contentRatio = width / height;
            if(  contentRatio < ratio ){
                int newHeight = (int)((float)width / ratio);
                Rect startBound = new Rect(0, 0, width,newHeight );
                Rect endBound = new Rect(0 , height- newHeight, width, height );
                if( Math.random() < 0.5 ) {
                    setStartPosition(startBound);
                    setEndPosition(endBound);
                } else {
                    setStartPosition(endBound);
                    setEndPosition(startBound);
                }
                return;
            }else if( contentRatio > ratio ){
                int newWidth = (int)((float)height * ratio);
                Rect startBound = new Rect(0, 0, newWidth, height);
                Rect endBound = new Rect(width - newWidth, 0 , width, height );
                if( Math.random() < 0.5 ) {
                    setStartPosition(startBound);
                    setEndPosition(endBound);
                } else {
                    setStartPosition(endBound);
                    setEndPosition(startBound);
                }
                return;

            }else{
                Rect fullbounds = new Rect(0, 0, width, height);
                //shrinkToAspect(fullbounds, 16.0f / 9.0f);
                shrinkToAspect(fullbounds, ratio);
                setStartPosition(fullbounds);
                setEndPosition(fullbounds);
                return;
            }
        }

        Rect zoombounds = new Rect();

        if( zoombounds.isEmpty() ) {
            zoombounds.set(0, 0, width*3/4, height*3/4);
            zoombounds.offset((int)(width*Math.random()/4), (int)(height*Math.random()/4));
        } else {
            int addSpace;
            addSpace = (width/4)-zoombounds.width();
            if( addSpace>=2 ) {
                zoombounds.left -= addSpace/2;
                zoombounds.right += addSpace/2;
            }
            addSpace = (height/4)-zoombounds.height();
            if( addSpace>=2 ) {
                zoombounds.top -= addSpace/2;
                zoombounds.bottom += addSpace/2;
            }
            //growToAspect(zoombounds, 16.0f / 9.0f);
            growToAspect(zoombounds, ratio);
            if(!zoombounds.intersect(0, 0, width, height)) {
                zoombounds.set(0, 0, width*2/3, height*2/3);
                zoombounds.offset((int)(width*Math.random()/3), (int)(height*Math.random()/3));
            }
        }
        //shrinkToAspect(zoombounds, 16.0f / 9.0f);
        shrinkToAspect(zoombounds, ratio);

        Rect fullbounds = new Rect(0, 0, width, height);
        //shrinkToAspect(fullbounds, 16.0f / 9.0f);
        int speed = getFaceDetectSpeed();
        applyCropSpeed(fullbounds, zoombounds, width, height, speed, clip_druation);

        shrinkToAspect(fullbounds, nexApplicationConfig.getAspectRatio());
        shrinkToAspect(zoombounds, nexApplicationConfig.getAspectRatio());

        if( !zoombounds.isEmpty() ) {
            if( Math.random() < 0.5 ) {
                setStartPosition(fullbounds);
                setEndPosition(zoombounds);
            } else {
                setStartPosition(zoombounds);
                setEndPosition(fullbounds);
            }
        }

        Math.random();
    }

    /**
     * This method gets the starting crop position to be displayed on a clip, as a rectangle, <tt> Rect</tt>.
     * 
     * <p>Example code:</p>
     *     {@code
                mEngine.getProject().getClip(0, true).getCrop().getStartPosition(startRect);
            }
     * @param rect The starting crop position to be shown, as a rectangle.
     * 
     * @see #setStartPosition(android.graphics.Rect)
     * @see #setEndPosition(android.graphics.Rect)
     * @see #getEndPosition(android.graphics.Rect)
     * @since version 1.1.0
     */
    public void getStartPosition( Rect rect ) {
        if( m_rotation==90 || m_rotation==270 ) {
            rect.bottom = m_rotatedStartPositionBottom * m_width / ABSTRACT_DIMENSION;
            rect.left	= m_rotatedStartPositionLeft * m_height / ABSTRACT_DIMENSION;
            rect.right	= m_rotatedStartPositionRight * m_height / ABSTRACT_DIMENSION;
            rect.top	= m_rotatedStartPositionTop * m_width / ABSTRACT_DIMENSION;
        } else {
            rect.bottom = m_startPositionBottom * m_height / ABSTRACT_DIMENSION;
            rect.left	= m_startPositionLeft * m_width / ABSTRACT_DIMENSION;
            rect.right	= m_startPositionRight * m_width / ABSTRACT_DIMENSION;
            rect.top	= m_startPositionTop * m_height / ABSTRACT_DIMENSION;
        }
//		rectFromAbs(rect);
    }

    /**
     * This method sets the starting position of a crop on a clip.
     * 
     * The original size of the clip is used as the frame of reference for the crop.
     *
     * @note  If, for example, the cropped section of the original should be displayed at a ratio of 16:9, then
     * the crop coordinates should also be set to a ratio of 16:9.
     * 
     * <p>Example code:</p>
     *     {@code
                rect.left = (int) (rect.left / mRatio);
                rect.top = (int) (rect.top / mRatio);
                rect.right = (int) (rect.right / mRatio);
                rect.bottom = (int) (rect.bottom / mRatio);
                mEngine.getProject().getClip(0, true).getCrop().setStartPosition(rect);
            }
     * @param rect  The start position to set for the crop on a clip, as a rectangle.
     * 
     * @see #setEndPosition(android.graphics.Rect)
     * @see #getStartPosition(android.graphics.Rect)
     * @see #getEndPosition(android.graphics.Rect)
     * @since version 1.1.0
     */
    public void setStartPosition( Rect rect ) {
        if( m_rotation==90 || m_rotation==270 ) {
            m_rotatedStartPositionBottom	= rect.bottom * ABSTRACT_DIMENSION / m_width;
            m_rotatedStartPositionLeft 		= rect.left * ABSTRACT_DIMENSION / m_height;
            m_rotatedStartPositionRight		= rect.right * ABSTRACT_DIMENSION / m_height;
            m_rotatedStartPositionTop		= rect.top * ABSTRACT_DIMENSION / m_width;
        } else {
            m_startPositionBottom	= rect.bottom * ABSTRACT_DIMENSION / m_height;
            m_startPositionLeft 	= rect.left * ABSTRACT_DIMENSION / m_width;
            m_startPositionRight	= rect.right * ABSTRACT_DIMENSION / m_width;
            m_startPositionTop		= rect.top * ABSTRACT_DIMENSION / m_height;
        }
        if( m_startMatrix==null )
            m_startMatrix = new float[9];
        matrixForRect(rect).setValues(m_startMatrix);
    }

    /**
     * Brief in progress...
     * This method crops a clip in {@value #ABSTRACT_DIMENSION} ratio.
     * @param rect
     * @since 1.7.1
     */
    public void setStartPositionRaw( Rect rect ) {
        if( m_rotation==90 || m_rotation==270 ) {
            m_rotatedStartPositionBottom	= rect.bottom;
            m_rotatedStartPositionLeft 		= rect.left;
            m_rotatedStartPositionRight		= rect.right;
            m_rotatedStartPositionTop		= rect.top;
        } else {
            m_startPositionBottom	= rect.bottom;
            m_startPositionLeft 	= rect.left;
            m_startPositionRight	= rect.right;
            m_startPositionTop		= rect.top;
        }
    }

    /**
     * Brief in progress...
     * This method crops a clip in {@value #ABSTRACT_DIMENSION} ratio.
     * @param rect
     * @since 1.7.1
     */
    public void setEndPositionRaw( Rect rect ) {
        if( m_rotation==90 || m_rotation==270 ) {
            m_rotatedEndPositionBottom	= rect.bottom;
            m_rotatedEndPositionLeft 	= rect.left;
            m_rotatedEndPositionRight	= rect.right;
            m_rotatedEndPositionTop	= rect.top;
        } else {
            m_endPositionBottom	= rect.bottom;
            m_endPositionLeft 	= rect.left;
            m_endPositionRight	= rect.right;
            m_endPositionTop	= rect.top;
        }
    }

    /**
     * This method gets the end crop position to be displayed on a clip, as a rectangle, <tt> rect</tt>.
     * 
     * <p>Example code:</p>
     *     {@code
                mEngine.getProject().getClip(0, true).getCrop().getEndPosition(endRect);
            }
     * @param rect  The end position for the crop on a clip, as a rectangle.
     * 
     * @see #setStartPosition(android.graphics.Rect)
     * @see #setEndPosition(android.graphics.Rect)
     * @see #getStartPosition(android.graphics.Rect)
     * @since version 1.1.0
     */
    public void getEndPosition( Rect rect ) {
        if( m_rotation==90 || m_rotation==270 ) {
            rect.bottom = m_rotatedEndPositionBottom * m_width / ABSTRACT_DIMENSION;
            rect.left	= m_rotatedEndPositionLeft * m_height / ABSTRACT_DIMENSION;
            rect.right	= m_rotatedEndPositionRight * m_height / ABSTRACT_DIMENSION;
            rect.top	= m_rotatedEndPositionTop * m_width / ABSTRACT_DIMENSION;
        } else {
            rect.bottom = m_endPositionBottom * m_height / ABSTRACT_DIMENSION;
            rect.left	= m_endPositionLeft * m_width / ABSTRACT_DIMENSION;
            rect.right	= m_endPositionRight * m_width / ABSTRACT_DIMENSION;
            rect.top	= m_endPositionTop * m_height / ABSTRACT_DIMENSION;
        }
    }

    /**
     * This method sets the end position of a crop on a clip.
     * 
     * The original size of the clip is used as the frame of reference for the crop.
     *
     * <p>Example code:</p>
     *     {@code     rect.left = (int) (rect.left / mRatio);
                rect.top = (int) (rect.top / mRatio);
                rect.right = (int) (rect.right / mRatio);
                rect.bottom = (int) (rect.bottom / mRatio);
                mEngine.getProject().getClip(0, true).getCrop().setStartPosition(rect);
            }
     * @note  If, for example, the cropped section of the original should be displayed at a ratio of 16:9, then
     * the crop coordinates should also be set to a ratio of 16:9.
     * 
     * @param rect  The end position to set for the crop on a clip, as a rectangle.
     * 
     * @see #setStartPosition(android.graphics.Rect)
     * @see #getStartPosition(android.graphics.Rect)
     * @see #getEndPosition(android.graphics.Rect)
     * @since version 1.1.0
     */
    public void setEndPosition( Rect rect ) {
        if( m_rotation==90 || m_rotation==270 ) {
            m_rotatedEndPositionBottom	= rect.bottom * ABSTRACT_DIMENSION / m_width;
            m_rotatedEndPositionLeft 	= rect.left * ABSTRACT_DIMENSION / m_height;
            m_rotatedEndPositionRight	= rect.right * ABSTRACT_DIMENSION / m_height;
            m_rotatedEndPositionTop	= rect.top * ABSTRACT_DIMENSION / m_width;
        } else {
            m_endPositionBottom	= rect.bottom * ABSTRACT_DIMENSION / m_height;
            m_endPositionLeft 	= rect.left * ABSTRACT_DIMENSION / m_width;
            m_endPositionRight	= rect.right * ABSTRACT_DIMENSION / m_width;
            m_endPositionTop	= rect.top * ABSTRACT_DIMENSION / m_height;
        }
        if( m_endMatrix==null )
            m_endMatrix = new float[9];
        matrixForRect(rect).setValues(m_endMatrix);
    }

    public void setFacePosition(Rect rect){

        if( m_rotation==90 || m_rotation==270 ) {
            m_rotatedFacePositionBottom  = rect.bottom * ABSTRACT_DIMENSION / m_width;
            m_rotatedFacePositionLeft    = rect.left * ABSTRACT_DIMENSION / m_height;
            m_rotatedFacePositionRight   = rect.right * ABSTRACT_DIMENSION / m_height;
            m_rotatedFacePositionTop     = rect.top * ABSTRACT_DIMENSION / m_width;
        } else {
            m_facePositionBottom = rect.bottom * ABSTRACT_DIMENSION / m_height;
            m_facePositionLeft   = rect.left * ABSTRACT_DIMENSION / m_width;
            m_facePositionRight  = rect.right * ABSTRACT_DIMENSION / m_width;
            m_facePositionTop    = rect.top * ABSTRACT_DIMENSION / m_height;
        }
    }

    public void getFacePositionRaw(Rect rect){

        if(m_rotation == 90 || m_rotation == 270){
            rect.left = m_rotatedFacePositionLeft;
            rect.top = m_rotatedFacePositionTop;
            rect.right = m_rotatedFacePositionRight;
            rect.bottom = m_rotatedFacePositionBottom;
        }
        else{
            rect.left = m_facePositionLeft;
            rect.top = m_facePositionTop;
            rect.right = m_facePositionRight;
            rect.bottom = m_facePositionBottom;
        }
    }

    /**
     * This method gets the start position of a crop on a clip (as a rectangle), converted into a range value that's internally used by the NexEditor&trade; SDK.
     * use to {@link com.nexstreaming.nexeditorsdk.nexEngine#fastPreviewCrop(android.graphics.Rect)}
     *
     * <p>Example code:</p>
     *     {@code     Rect raw = new Rect();
                mEngine.getProject().getClip(0, true).getCrop().getStartPositionRaw(raw);
            }
     * @param rect   The position of the rectangle, converted into the NexEditor&trade; SDK range value ( 0 ~ 100000 ).
     * @see #getEndPositionRaw(android.graphics.Rect)
     * @see com.nexstreaming.nexeditorsdk.nexEngine#fastPreviewCrop(android.graphics.Rect)
     * @since version 1.3.43
     */
    public void getStartPositionRaw( Rect rect ) {
        if( m_rotation==90 || m_rotation==270 ) {
            rect.bottom = m_rotatedStartPositionBottom;
            rect.left	= m_rotatedStartPositionLeft;
            rect.right	= m_rotatedStartPositionRight;
            rect.top	= m_rotatedStartPositionTop;
        } else {
            rect.bottom = m_startPositionBottom;
            rect.left	= m_startPositionLeft;
            rect.right	= m_startPositionRight;
            rect.top	= m_startPositionTop;
        }
//		rectFromAbs(rect);
    }

    /**
     * This method gets the end position of a crop on a clip (as a rectangle), converted into a range value that's internally used by the NexEditor&trade; SDK.
     * use to {@link com.nexstreaming.nexeditorsdk.nexEngine#fastPreviewCrop(android.graphics.Rect)}
     *
     * <p>Example code:</p>
     *     {@code     Rect raw = new Rect();
                mEngine.getProject().getClip(0, true).getCrop().getEndPositionRaw(raw);
            }
     * @param rect   The position of the rectangle, converted into the NexEditor&trade; SDK range value ( 0 ~ 100000 ).
     * @see #getStartPositionRaw(android.graphics.Rect)
     * @see com.nexstreaming.nexeditorsdk.nexEngine#fastPreviewCrop(android.graphics.Rect)
     * @since version 1.3.43
     */
    public void getEndPositionRaw( Rect rect ) {
        if( m_rotation==90 || m_rotation==270 ) {
            rect.bottom = m_rotatedEndPositionBottom;
            rect.left	= m_rotatedEndPositionLeft;
            rect.right	= m_rotatedEndPositionRight;
            rect.top	= m_rotatedEndPositionTop;
        } else {
            rect.bottom = m_endPositionBottom;
            rect.left	= m_endPositionLeft;
            rect.right	= m_endPositionRight;
            rect.top	= m_endPositionTop;
        }
    }

    /**
     * Initializes the start and end positions of the crop.
     * Switches to Full Screen mode.
     * @since 1.5.42
     */
    public void resetStartEndPosition(){
        m_startPositionLeft = 0;
        m_startPositionBottom = ABSTRACT_DIMENSION;
        m_startPositionRight = ABSTRACT_DIMENSION;
        m_startPositionTop = 0;
        m_endPositionLeft = 0;
        m_endPositionBottom = ABSTRACT_DIMENSION;
        m_endPositionRight = ABSTRACT_DIMENSION;
        m_endPositionTop = 0;

        m_rotatedStartPositionLeft = 0;
        m_rotatedStartPositionBottom = ABSTRACT_DIMENSION;
        m_rotatedStartPositionRight = ABSTRACT_DIMENSION;
        m_rotatedStartPositionTop = 0;
        m_rotatedEndPositionLeft = 0;
        m_rotatedEndPositionBottom = ABSTRACT_DIMENSION;
        m_rotatedEndPositionRight = ABSTRACT_DIMENSION;
        m_rotatedEndPositionTop = 0;

    }

    void fitStartEndPosition(int outputWidth, int outputHeight){
        int width = getWidth();
        int height = getHeight();

        if(  m_rotation==90 || m_rotation==270 ) {
            width = getHeight();
            height = getWidth();
        }
        Rect fullbounds = new Rect(0, 0, width, height);
        //growToAspect(fullbounds, 16.0f / 9.0f);
        growToAspect(fullbounds, ((float)outputWidth/(float)outputHeight));
        setStartPosition(fullbounds);
        setEndPosition(fullbounds);
    }
    
    private static Rect getFitRwaPosition(int srcWidth, int srcHeight, float outWidth, float outHeight, boolean highQuality){
        Rect fullbounds = new Rect(0, 0, srcWidth, srcHeight);
        float outAspect =  outWidth / outHeight;

        float originalAspect = ((float)fullbounds.width()) / (float) (fullbounds.height());
        if( originalAspect < outAspect ) {
            // Original too tall
            int newWidth = (int)(((float)fullbounds.height())*outAspect);
            int ctrx = fullbounds.centerX();
            fullbounds.left = ctrx - newWidth/2;
            fullbounds.right = fullbounds.left + newWidth;
        } else {
            // Original too wide
            int newHeight = (int)(((float)fullbounds.width())/outAspect);
            int ctry = fullbounds.centerY();
            fullbounds.top = ctry - newHeight/2;
            fullbounds.bottom = fullbounds.top + newHeight;
        }

        int bottom	= fullbounds.bottom * ABSTRACT_DIMENSION / srcHeight;
        int left 		= fullbounds.left * ABSTRACT_DIMENSION / srcWidth;
        int right		= fullbounds.right * ABSTRACT_DIMENSION / srcWidth;
        int top		= fullbounds.top * ABSTRACT_DIMENSION / srcHeight ;

        fullbounds.set(left, top, right, bottom);
        if( top == 0 ) {
            Log.d(TAG, "getFitRwaPosition() raw=" + fullbounds.toString());
            return fullbounds;
        }
        if( highQuality ) {
            enhancedCrop(fullbounds,  srcWidth,  srcHeight,  outWidth,  outHeight);
        }
        return fullbounds;
    }

    static void enhancedCrop(Rect rawRect, int srcWidth, int srcHeight, float outWidth, float outHeight){

        int left = rawRect.left;
        int top	 = rawRect.top;
        int right	= rawRect.right;
        int bottom	= rawRect.bottom;

        if( rawRect.top == 0 ){
            return ;
        }

        float h = bottom - top;
        float elem = (h / (float) outHeight) / (1f / (float)srcHeight * (float) ABSTRACT_DIMENSION);
        int pixel_height = (int) ((float) Math.abs(top) / (h / (float) outHeight));
        int delta = (int) (pixel_height / 10f);
        int end = pixel_height + delta;
        int peek = pixel_height - delta;
        float quality = 1;
        for (int i = pixel_height - delta; i < end; i++) {
            float q = (float) Math.min( Math.ceil(elem * i) - (elem * i), (elem * i) - Math.floor(elem * i));
            //Log.d("nexCrop","getFitRwaPosition() peek="+i+", result="+q+", elem="+elem);
            if (quality > q) {
                quality = q;
                peek = i;
            }
        }

        int newTop = (int) Math.round(h / outHeight * peek) *-1;
        int newBottom = (int) (h + newTop);
        rawRect.set(left, newTop, right, newBottom);
        Log.d(TAG,"enhancedCrop() highQuality="+peek+", q="+rawRect.toString());
    }

    /**
     * This method detects any face from the image item. 
     * 
     * <p>Example code:</p>
     *     {@code
                mEngine.getProject().getClip(0, true).getCrop().getFaceBounds(raw);
            }
     * @param bounds Detected face area on the image item, as <tt>Rect</tt>.
     * 
     * @since version 1.3.27
     */
    public void getFaceBounds(Rect bounds , Context context) {
/*
        FaceInfo info = FaceInfo.getFaceInfo(m_mediaPath);
        if ( info == null ) {
            Log.d(TAG, "Second face detection with use filter");
            info = new FaceInfo(new File(m_mediaPath), true, context);
        }
        if( info!=null ) {
            RectF sbounds = new RectF();
            info.getBounds(sbounds);
            float width = getWidth();
            float height = getHeight();
            bounds.set( (int)(sbounds.left*width), (int)(sbounds.top*height), (int)(sbounds.right*width), (int)(sbounds.bottom*height) );
            return;
       }
       */
        calculateFaceBounds();
        if( !m_faceBounds_set ) {
            bounds.setEmpty();
            return;
        }
        float width = getWidth();
        float height = getHeight();
        bounds.set( (int)(m_faceBounds_left*width), (int)(m_faceBounds_top*height), (int)(m_faceBounds_right*width), (int)(m_faceBounds_bottom*height) );
    }

    private void calculateFaceBounds() {

        if( m_faceBounds_set )
            return;

        Bitmap bm = loadImageForFaceRecognition();
        if( bm==null )
            return;

        FaceDetector.Face[] faces = new FaceDetector.Face[32];
        FaceDetector fd = new FaceDetector(bm.getWidth(), bm.getHeight(), 32);
        int numFaces = fd.findFaces(bm, faces);
        RectF faceBounds = new RectF();
        RectF bounds = new RectF();
        PointF eyes = new PointF();
        float width = getWidth();
        float height = getHeight();
        float faceMinWidth = width / 30;
        float faceMinHeight = height / 30;
        for( int i=0; i<numFaces; i++ ) {

            if( LL.D ) Log.d(TAG,"face : " + i + " c=" + faces[i].confidence() + " e=" + faces[i].eyesDistance() + " faceMinWidth=" + faceMinWidth + " faceMinHeight=" + faceMinHeight);
            if( faces[i].confidence() < 0.4 )
                continue;

            faces[i].getMidPoint(eyes);
            float d = faces[i].eyesDistance();
            if( d < faceMinWidth || d < faceMinHeight )
                continue;
            bounds.set((eyes.x - d/2)/width, (eyes.y-d/3)/height, (eyes.x+d/2)/width, (eyes.y+d/3*2)/height);
            faceBounds.union(bounds);
        }
        bm.recycle();

        if( LL.D ) Log.d(TAG,"face bounds : " + faceBounds);

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

        if( LL.D ) Log.d(TAG,"face bounds (final) : " + faceBounds);

        m_faceBounds_set = true;
        m_faceBounds_left = Math.max(0,faceBounds.left);
        m_faceBounds_top = Math.max(0,faceBounds.top);
        m_faceBounds_right = Math.min(faceBounds.right,1);
        m_faceBounds_bottom = Math.min(faceBounds.bottom,1);
    }

    private Bitmap loadImageForFaceRecognition() {

//        if( isPreset() )
//            return null;

        BitmapFactory.Options opt = new BitmapFactory.Options();
        opt.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(m_mediaPath, opt);
        opt.inJustDecodeBounds = false;
        opt.inPreferredConfig = Bitmap.Config.RGB_565;

        if( opt.outWidth < 1 || opt.outHeight < 1 )
            return null;

        int thumbWidth = 720;
        int thumbHeight = opt.outHeight*720/opt.outWidth;


        // TODO: Handle image orientation (maybe load the image using NexImage)
        opt.inSampleSize = 1;
        while( opt.outHeight/opt.inSampleSize > thumbHeight*1.5 || opt.outWidth/opt.inSampleSize > thumbWidth*1.5 ) {
            opt.inSampleSize*=2;
        }
        if(LL.D) Log.d(TAG, "loadImageForFaceRecognition:   bounds decoded : width,height=" + opt.outWidth + "," + opt.outHeight + "; target w,h=" + thumbWidth + "," + thumbHeight + "; sampleSize=" + opt.inSampleSize);
        Bitmap bm = BitmapFactory.decodeFile(m_mediaPath, opt);

        if( bm==null ) {
            return null;
        }

        if( bm.getConfig() != Bitmap.Config.RGB_565 ) {
            Bitmap bm2 = Bitmap.createBitmap(bm.getWidth(), bm.getHeight(), Bitmap.Config.RGB_565);
            Canvas cvs = new Canvas(bm2);
            cvs.drawBitmap(bm, 0, 0, null);
            bm.recycle();
            bm = bm2;
        }

        return bm;
    }


    nexSaveDataFormat.nexCropOf getSaveData(){
        nexSaveDataFormat.nexCropOf data = new nexSaveDataFormat.nexCropOf();
        data.m_startPositionLeft = m_startPositionLeft;
        data.m_startPositionBottom = m_startPositionBottom;
        data.m_startPositionRight = m_startPositionRight;
        data.m_startPositionTop = m_startPositionTop;
        data.m_endPositionLeft = m_endPositionLeft;
        data.m_endPositionBottom = m_endPositionBottom;
        data.m_endPositionRight = m_endPositionRight;
        data.m_endPositionTop = m_endPositionTop;

        data.m_rotatedStartPositionLeft = m_rotatedStartPositionLeft;
        data.m_rotatedStartPositionBottom = m_rotatedStartPositionBottom;
        data.m_rotatedStartPositionRight = m_rotatedStartPositionRight;
        data.m_rotatedStartPositionTop = m_rotatedStartPositionTop;
        data.m_rotatedEndPositionLeft = m_rotatedEndPositionLeft;
        data.m_rotatedEndPositionBottom = m_rotatedEndPositionBottom;
        data.m_rotatedEndPositionRight = m_rotatedEndPositionRight;
        data.m_rotatedEndPositionTop = m_rotatedEndPositionTop;

        data.m_facePositionLeft = m_facePositionLeft;
        data.m_facePositionTop = m_facePositionTop;
        data.m_facePositionRight = m_facePositionRight;
        data.m_facePositionBottom = m_facePositionBottom;
        data.m_rotatedFacePositionLeft = m_rotatedFacePositionLeft;
        data.m_rotatedFacePositionTop = m_rotatedFacePositionTop;
        data.m_rotatedFacePositionRight = m_rotatedFacePositionRight;
        data.m_rotatedFacePositionBottom = m_rotatedFacePositionBottom;

        //private float[] m_startMatrix;
        //private float[] m_endMatrix;

        data.m_rotation = m_rotation;
        data.m_width = m_width;
        data.m_height = m_height;
        //private String m_mediaPath; --> internal set

        data.m_faceBounds_left = m_faceBounds_left;
        data.m_faceBounds_top = m_faceBounds_top;
        data.m_faceBounds_right = m_faceBounds_right;
        data.m_faceBounds_bottom = m_faceBounds_bottom;
        data.m_faceBounds_set = m_faceBounds_set;

        return data;
    }

    nexCrop(String path ,nexSaveDataFormat.nexCropOf data){
        m_startPositionLeft = data.m_startPositionLeft;
        m_startPositionBottom = data.m_startPositionBottom;
        m_startPositionRight = data.m_startPositionRight;
        m_startPositionTop = data.m_startPositionTop;
        m_endPositionLeft = data.m_endPositionLeft;
        m_endPositionBottom = data.m_endPositionBottom;
        m_endPositionRight = data.m_endPositionRight;
        m_endPositionTop = data.m_endPositionTop;

        m_rotatedStartPositionLeft = data.m_rotatedStartPositionLeft;
        m_rotatedStartPositionBottom = data.m_rotatedStartPositionBottom;
        m_rotatedStartPositionRight = data.m_rotatedStartPositionRight;
        m_rotatedStartPositionTop = data.m_rotatedStartPositionTop;
        m_rotatedEndPositionLeft = data.m_rotatedEndPositionLeft;
        m_rotatedEndPositionBottom = data.m_rotatedEndPositionBottom;
        m_rotatedEndPositionRight = data.m_rotatedEndPositionRight;
        m_rotatedEndPositionTop = data.m_rotatedEndPositionTop;

        m_facePositionLeft = data.m_facePositionLeft;
        m_facePositionTop = data.m_facePositionTop;
        m_facePositionRight = data.m_facePositionRight;
        m_facePositionBottom = data.m_facePositionBottom;
        m_rotatedFacePositionLeft = data.m_rotatedFacePositionLeft;
        m_rotatedFacePositionTop = data.m_rotatedFacePositionTop;
        m_rotatedFacePositionRight = data.m_rotatedFacePositionRight;
        m_rotatedFacePositionBottom = data.m_rotatedFacePositionBottom;

        //private float[] m_startMatrix;
        //private float[] m_endMatrix;

        m_rotation = data.m_rotation;
        m_width = data.m_width;
        m_height = data.m_height;
        m_mediaPath = path;

        m_faceBounds_left = data.m_faceBounds_left;
        m_faceBounds_top = data.m_faceBounds_top;
        m_faceBounds_right = data.m_faceBounds_right;
        m_faceBounds_bottom = data.m_faceBounds_bottom;
        m_faceBounds_set = data.m_faceBounds_set;

    }
    private int facedetect_speed = 101;
    public void setFaceDetectSpeed(int spd){
        facedetect_speed = spd;
    }
    public int getFaceDetectSpeed(){
        return facedetect_speed;
    }

    private int clip_druation = 0;
    public void setClipDuration(int duration){
        clip_druation = duration;
    }
    public int getClipDuration(){
        return clip_druation;
    }

    public void applyCropSpeed(Rect StartRect, Rect EndRect, int width, int height, int speed, int totalDuration){
        if(!getEndPosionLock()) {
            if (EndRect.intersect(0, 0, width, height) == false) {
                EndRect.set(0, 0, width * 2 / 3, height * 2 / 3);
                EndRect.offset((int) (width * Math.random() / 3), (int) (height * Math.random() / 3));
            }
        }

        int distance = (int)(speed*((float)totalDuration*0.5/1000));
        if(speed < 101) {
            StartRect.set(EndRect.left - width*distance/1000, EndRect.top - height*distance/1000, EndRect.right + width*distance/1000, EndRect.bottom + height*distance/1000*3);
            //tmpFull.set(tmpStart.left - distance, tmpStart.top - distance, tmpStart.right + distance, tmpStart.bottom + distance);
        }else {
            StartRect.set(EndRect.left - (width/4), EndRect.top - (height/4), EndRect.right + (width/4), EndRect.bottom + (height/4));
        }
        if( StartRect.intersect(0, 0, width, height) == false) {
            StartRect.set(0, 0, width*3/4, height*3/4);
            StartRect.offset((int)(width*Math.random()/4), (int)(height*Math.random()/4));
        }
    }
    public void setStartPositionLock(boolean lock){
        m_startLock = lock;
    }
    public boolean getStartPosionLock(){
        return m_startLock;
    }
    public void setEndPositionLock(boolean lock){
        m_endLock = lock;
    }
    public boolean getEndPosionLock(){
        return m_endLock;
    }
}
