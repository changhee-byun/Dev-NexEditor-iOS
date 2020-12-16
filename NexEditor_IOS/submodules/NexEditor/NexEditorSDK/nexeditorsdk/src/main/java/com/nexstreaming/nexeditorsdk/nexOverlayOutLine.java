package com.nexstreaming.nexeditorsdk;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PointF;
import android.graphics.Rect;

import com.nexstreaming.kminternal.nexvideoeditor.LayerRenderer;

/**
 *
 *
 * @since 2.0.14
 */

public class nexOverlayOutLine {
    private static String TAG="nexOverlayOutLine";

    Bitmap solidBlackBitmap;
    Bitmap solidWhiteBitmap;
    boolean solidOutline;
    boolean showAnchorLine;
    int marchingAnts_dashSize = 10;
    int marchingAnts_width = 4;
    Bitmap[] outLineIcon;
    int spaceTopPixel = 0;
    int spaceLeftPixel = 0;
    int spaceRightPixel = 0;
    int spaceBottomPixel = 0;
    int blackColor = Color.BLACK;
    int whiteColor = Color.WHITE;

    private nexOverlayOutLine(){
        Canvas c;

        solidOutline = false;
        showAnchorLine = false;

        solidBlackBitmap = Bitmap.createBitmap(16,16, Bitmap.Config.ARGB_8888);
        c = new Canvas(solidBlackBitmap);
        c.drawColor(blackColor);

        solidWhiteBitmap = Bitmap.createBitmap(16,16, Bitmap.Config.ARGB_8888);
        c = new Canvas(solidWhiteBitmap);
        c.drawColor(whiteColor);

        outLineIcon = new Bitmap[4];
    }

    static nexOverlayOutLine builder(){
        return new nexOverlayOutLine();
    }

    /**
     *
     * @return instance of this
     * @since 2.0.14
     */
    public nexOverlayOutLine reset(){
        Canvas c;
        marchingAnts_dashSize = 10;
        marchingAnts_width = 4;
        spaceTopPixel = 0;
        spaceLeftPixel = 0;
        spaceRightPixel = 0;
        spaceBottomPixel = 0;
        solidOutline = false;
        showAnchorLine = false;

        if( blackColor != Color.BLACK ) {
            blackColor = Color.BLACK;
            solidBlackBitmap = Bitmap.createBitmap(16, 16, Bitmap.Config.ARGB_8888);
            c = new Canvas(solidBlackBitmap);
            c.drawColor(blackColor);
        }

        if( whiteColor != Color.WHITE ) {
            whiteColor = Color.WHITE;
            solidWhiteBitmap = Bitmap.createBitmap(16, 16, Bitmap.Config.ARGB_8888);
            c = new Canvas(solidWhiteBitmap);
            c.drawColor(whiteColor);
        }

        outLineIcon = new Bitmap[4];
        return this;
    }

    /**
     *
     * @return
     * @since 2.0.14
     */
    public int getLineColor(){
        return whiteColor;
    }

    /**
     *
     * @param colorARGB
     * @return instance of this
     * @since 2.0.14
     */
    public nexOverlayOutLine setLineColor(int colorARGB){
        whiteColor = (0xff000000 | colorARGB);
        blackColor = (0xff000000|(colorARGB ^ 0xffffffff));
        Canvas c;

        solidBlackBitmap = Bitmap.createBitmap(16,16, Bitmap.Config.ARGB_8888);
        c = new Canvas(solidBlackBitmap);
        c.drawColor(blackColor);

        solidWhiteBitmap = Bitmap.createBitmap(16,16, Bitmap.Config.ARGB_8888);
        c = new Canvas(solidWhiteBitmap);
        c.drawColor(whiteColor);

        return this;
    }

    /**
     *
     * @return
     * @since 2.0.14
     */
    public boolean isSolidOutline() {
        return solidOutline;
    }

    /**
     *
     * @param solidOutlen
     * @return instance of this
     * @since 2.0.14
     */
    public nexOverlayOutLine setSolidOutline(boolean solidOutlen) {
        this.solidOutline = solidOutlen;
        return this;
    }

    /**
     *
     * @return
     * @since 2.0.14
     */
    public boolean isShowAnchorLine() {
        return showAnchorLine;
    }

    /**
     *
     * @param showAnchorLine
     * @return instance of this
     * @since 2.0.14
     */
    public nexOverlayOutLine showAnchorLine(boolean showAnchorLine) {
        this.showAnchorLine = showAnchorLine;
        return this;
    }

    /**
     *
     * @return
     * @since 2.0.14
     */
    public int getMarchingAntsDashSize() {
        return marchingAnts_dashSize;
    }

    /**
     *
     * @param marchingAnts_dashSize
     * @return instance of this
     * @since 2.0.14
     */
    public nexOverlayOutLine setMarchingAntsDashSize(int marchingAnts_dashSize) {
        this.marchingAnts_dashSize = marchingAnts_dashSize;
        return this;
    }

    /**
     *
     * @return
     * @since 2.0.14
     */
    public int getMarchingAntsWidth() {
        return marchingAnts_width;
    }

    /**
     *
     * @param marchingAnts_width
     * @return instance of this
     * @since 2.0.14
     */
    public nexOverlayOutLine setMarchingAntsWidth(int marchingAnts_width) {
        this.marchingAnts_width = marchingAnts_width;
        return this;
    }

    /**
     *
     * @param pos
     * @return
     * @since 2.0.14
     */
    public Bitmap getOutLineIcon(int pos) {
        if( pos < 0 || pos > 3)
            return null;
        return outLineIcon[pos];
    }

    /**
     *
     * @return
     * @since 2.0.14
     */
    public int getSpaceUpperSize() {
        return spaceTopPixel;
    }

    /**
     *
     * @param spaceTopPixel
     * @return instance of this
     * @since 2.0.14
     */
    public nexOverlayOutLine setSpaceUpperSize(int spaceTopPixel) {
        this.spaceTopPixel = spaceTopPixel;
        return this;
    }

    /**
     *
     * @return
     * @since 2.0.14
     */
    public int getSpaceLeftSize() {
        return spaceLeftPixel;
    }

    /**
     *
     * @param spaceLeftPixel
     * @return instance of this
     * @since 2.0.14
     */
    public nexOverlayOutLine setSpaceLeftSize(int spaceLeftPixel) {
        this.spaceLeftPixel = spaceLeftPixel;
        return this;
    }

    /**
     *
     * @return
     * @since 2.0.14
     */
    public int getSpaceRightSize() {
        return spaceRightPixel;
    }

    /**
     *
     * @param spaceRightPixel
     * @return instance of this
     * @since 2.0.14
     */
    public nexOverlayOutLine setSpaceRightSize(int spaceRightPixel) {
        this.spaceRightPixel = spaceRightPixel;
        return this;
    }

    /**
     *
     * @return
     * @since 2.0.14
     */
    public int getSpaceUnderSize() {
        return spaceBottomPixel;
    }

    /**
     *
     * @param spaceBottomPixel
     * @return instance of this
     * @since 2.0.14
     */
    public nexOverlayOutLine setSpaceUnderSize(int spaceBottomPixel) {
        this.spaceBottomPixel = spaceBottomPixel;
        return this;
    }

    /**
     *
     * @param context
     * @param pos
     * @param resID
     * @return instance of this
     * @since 2.0.14
     */
    public nexOverlayOutLine setOutLineIcon(Context context, int pos , int resID ){
        if( outLineIcon != null ){
            if(pos <= 0 || pos > 4 ){
                return this;
            }
            if( resID == 0 ){
                outLineIcon[pos - 1] = null;
            }else {
                outLineIcon[pos - 1] = BitmapFactory.decodeResource(context.getResources(), resID);
            }
        }
        return this;
    }

    static private double getAngle(PointF start, PointF end) {
        double dy = end.y-start.y;
        double dx = end.x-start.x;
        double angle = Math.atan(dy/dx) * (180.0/Math.PI);

        if(dx < 0.0) {
            angle += 180.0;
        } else {
            if(dy<0.0) angle += 360.0;
        }
        return angle;
    }

    void renderOutLine(nexOverlayItem item, LayerRenderer layerRenderer){
        if( solidBlackBitmap!=null && solidWhiteBitmap != null ) {

            nexOverlayItem.BoundInfo info = item.getBoundInfo(item.mTime);
            if( showAnchorLine  ) {
                PointF start = new PointF(item.anchorPointX, item.anchorPointY);
                PointF end = new PointF(info.getTranslateX(), info.getTranslateY());
                float angle = (float) getAngle(start, end);

                float length = (float) Math.sqrt((Math.pow(end.y - start.y, 2) + Math.pow(end.x - start.y, 2)));
                layerRenderer.save();
                layerRenderer.rotate(angle, item.anchorPointX, item.anchorPointY);
                if( solidOutline ) {
                    layerRenderer.drawBitmap(solidWhiteBitmap, item.anchorPointX, item.anchorPointY, item.anchorPointX + length, item.anchorPointY + marchingAnts_width);
                }else{
                    layerRenderer.drawBitmap(solidBlackBitmap, item.anchorPointX, item.anchorPointY, item.anchorPointX + length, item.anchorPointY + marchingAnts_width);
                    float phase = marchingAnts_dashSize * 2f;
                    for (float x = item.anchorPointX - marchingAnts_dashSize * 2; x < (item.anchorPointX + length); x += marchingAnts_dashSize * 2) {
                        float s = Math.max(item.anchorPointX, x + phase);
                        float e = Math.min((item.anchorPointX + length), s + marchingAnts_dashSize);
                        if (e < item.anchorPointX || s > (item.anchorPointX + length))
                            continue;
                        layerRenderer.drawBitmap(solidWhiteBitmap, s, item.anchorPointY, e, item.anchorPointY + marchingAnts_width);
                    }
                }
                layerRenderer.restore();
            }
/*
            layerRenderer.save();

            layerRenderer.translate(info.x, info.y);
            //layerRenderer.scale(info.scaleX, info.scaleY, item.mX, item.mY);
            layerRenderer.scale(info.scaleX, info.scaleY);
            //layerRenderer.rotate(info.angle, item.mX, item.mY);
            layerRenderer.rotate(info.angle,0,0);
*/
            layerRenderer.save();
/*
            layerRenderer.scale(info.scaleX, info.scaleY, info.x, info.y);
            layerRenderer.rotate(info.angle, info.x, info.y);
            layerRenderer.translate(info.x, info.y);
*/
            layerRenderer.translate(info.getTranslateX(), info.getTranslateY());
            layerRenderer.scale(info.getScaleX(), info.getScaleY());
//            layerRenderer.rotateAroundAxis(info.angleX,1,0,0);
//            layerRenderer.rotateAroundAxis(info.angleY,0,1,0);
            layerRenderer.rotateAroundAxis(info.getAngle(),0,0,1);
            layerRenderer.save();
            layerRenderer.scale(1f / info.getScaleX(), 1f / info.getScaleY());
/*
            float w = info.getWidth() * info.scaleX;
            float h = info.getHeight() * info.scaleY;


            float left =  -w/2 ;
            float right = left + w ;
            float top =   -h/2 ;
            float bottom = top + h ;
            float mid = top + (bottom-top)/2;
*/
            Rect rc = new Rect();
            info.getDrawBound(rc);

            float left =  (rc.left - spaceLeftPixel) *info.getScaleX() ;
            float right = (rc.right + spaceRightPixel) *info.getScaleX() ;
            float top =   (rc.top -  spaceTopPixel) * info.getScaleY() ;
            float bottom = (rc.bottom + spaceBottomPixel) * info.getScaleY() ;

            if( solidOutline ) {
                layerRenderer.drawBitmap(solidWhiteBitmap, left, top, left + marchingAnts_width, bottom);
                layerRenderer.drawBitmap(solidWhiteBitmap, right - marchingAnts_width, top, right, bottom);
                layerRenderer.drawBitmap(solidWhiteBitmap, left, top, right, top + marchingAnts_width);
                layerRenderer.drawBitmap(solidWhiteBitmap, left, bottom - marchingAnts_width, right, bottom);
            }else{
                layerRenderer.drawBitmap(solidBlackBitmap, left, top, left + marchingAnts_width, bottom);
                layerRenderer.drawBitmap(solidBlackBitmap, right - marchingAnts_width, top, right, bottom);
                layerRenderer.drawBitmap(solidBlackBitmap, left, top, right, top + marchingAnts_width);
                layerRenderer.drawBitmap(solidBlackBitmap, left, bottom - marchingAnts_width, right, bottom);

                float phase = marchingAnts_dashSize * 2f;

                for (float x = left - marchingAnts_dashSize * 2; x < right; x += marchingAnts_dashSize * 2) {
                    float s = Math.max(left, x + phase);
                    float e = Math.min(right, s + marchingAnts_dashSize);
                    if (e < left || s > right)
                        continue;
                    layerRenderer.drawBitmap(solidWhiteBitmap, s, top, e, top + marchingAnts_width);
                    layerRenderer.drawBitmap(solidWhiteBitmap, s, bottom - marchingAnts_width, e, bottom);
                }
                for (float y = top - marchingAnts_dashSize * 2; y < bottom; y += marchingAnts_dashSize * 2) {
                    float s = Math.max(top, y + phase);
                    float e = Math.min(bottom, s + marchingAnts_dashSize);
                    if (e < top || s > bottom)
                        continue;
                    layerRenderer.drawBitmap(solidWhiteBitmap, left, s, left + marchingAnts_width, e);
                    layerRenderer.drawBitmap(solidWhiteBitmap, right - marchingAnts_width, s, right, e);
                }
            }

            layerRenderer.restore();

            for( int index = 0 ; index < 4 ; index++) {
                if( outLineIcon[index] != null ){
                    int pos = index+1;
                    float x=0;
                    float y=0 ;
                    switch (pos){
                        case nexOverlayItem.kOutLine_Pos_LeftTop:
                            x = left;
                            y = top;
                            break;
                        case nexOverlayItem.kOutLine_Pos_RightTop:
                            x = right;
                            y = top;
                            break;
                        case nexOverlayItem.kOutLine_Pos_LeftBottom:
                            x = left;
                            y = bottom;
                            break;
                        case nexOverlayItem.kOutLine_Pos_RightBottom:
                            x = right;
                            y = bottom;
                            break;
                    }
                    layerRenderer.save();
                    layerRenderer.scale(1f /info.getScaleX(), 1f / info.getScaleY());
                    layerRenderer.drawBitmap(outLineIcon[index], x, y);
                    layerRenderer.restore();
                }
            }
            layerRenderer.restore();
        }
    }
}
