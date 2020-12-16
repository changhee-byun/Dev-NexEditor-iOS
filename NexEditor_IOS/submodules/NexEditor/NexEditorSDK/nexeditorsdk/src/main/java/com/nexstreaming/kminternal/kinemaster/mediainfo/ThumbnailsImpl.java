package com.nexstreaming.kminternal.kinemaster.mediainfo;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;

/**
 * Created by jeongwook.yoon on 2016-12-01.
 */

public class ThumbnailsImpl implements Thumbnails {

    private final Bitmap mBitmap;
    private final int mWidth;
    private final int mHeight;
    private final int mThumbnailCount;
    private final int[] mThumbnailTime;
//	private Bitmap[] m_thumbnailBitmap;
//	private int[] m_rotation;

    ThumbnailsImpl( Bitmap bm, int width, int height, int[] thumbnailTime ) {
        mBitmap = bm;
        mWidth = width;
        mHeight = height;
        mThumbnailTime = thumbnailTime;
        if(bm == null) {
            throw new RuntimeException("bm is null");
        } else if (thumbnailTime == null){
            throw new RuntimeException("thumbnailTime is null");
        }
        mThumbnailCount = Math.min(bm.getWidth()/mWidth, thumbnailTime.length);
//		m_thumbnailBitmap = new Bitmap[mThumbnailCount];
//		m_rotation = new int[mThumbnailCount];
    }

    public int getWidth(int rotation) {
        return (rotation==90 || rotation==270)?mHeight:mWidth;
    }

    public int getHeight(int rotation) {
        return (rotation==90 || rotation==270)?mWidth:mHeight;
    }
    public float getWidthForHeight( int rotation, float height ) {
        if( rotation==90 || rotation==270 ) {
            return (float)height/(float)mWidth*(float)mHeight;
        } else {
            return (float)height/(float)mHeight*(float)mWidth;
        }
    }

    public Bitmap getThumbnailBitmap( int rotation, int time, boolean fliph, boolean flipv ) {
        return getThumbnailBitmapInternal(rotation, time, fliph, flipv);
    }

    public Bitmap getThumbnailBitmapNoCache( int rotation, int time, boolean fliph, boolean flipv ) {
        return getThumbnailBitmapInternal(rotation, time, fliph, flipv);
    }

    public int closestActualThumbTime( int time ) {
        int mindiff = 0;
        int usetime = 0;
        for( int i=0; i<mThumbnailCount; i++ ) {
            int diff = Math.abs(mThumbnailTime[i]-time);
            if( i==0 || diff<mindiff ) {
                mindiff=diff;
                usetime=mThumbnailTime[i];
            }
        }
        return usetime;
    }

    public int[] getThumbnailTimeLine(){
        return mThumbnailTime;
    }

    private Bitmap getThumbnailBitmapInternal( int rotation, int time, boolean fliph, boolean flipv ) {

        int mindiff = 0;
        int useidx = 0;
        for( int i=0; i<mThumbnailCount; i++ ) {
            int diff = Math.abs(mThumbnailTime[i]-time);
            if( i==0 || diff<mindiff ) {
                mindiff=diff;
                useidx=i;
            }
        }

//		if( cache && m_thumbnailBitmap[useidx] != null && m_rotation[useidx]==rotation )
//			return m_thumbnailBitmap[useidx];

        Rect src = new Rect(useidx*mWidth,0,useidx*mWidth+mWidth,mHeight);

        if( rotation==180 ) {
            Bitmap bm = Bitmap.createBitmap(mWidth, mHeight, Bitmap.Config.RGB_565);
            Canvas canvas = new Canvas(bm);
            if(fliph && flipv) canvas.scale(-1, -1, bm.getWidth()/2, bm.getHeight()/2);
            else if(fliph) canvas.scale(-1, 1, bm.getWidth()/2, bm.getHeight()/2);
            else if(flipv) canvas.scale(1, -1, bm.getWidth()/2, bm.getHeight()/2);
            canvas.drawBitmap(mBitmap, src, new Rect(0,0,mWidth,mHeight), null);
            return bm;
        } else if( rotation==0 ) {
            Bitmap bm = Bitmap.createBitmap(mWidth, mHeight, Bitmap.Config.RGB_565);
            Canvas canvas = new Canvas(bm);
            if(fliph && flipv) canvas.scale(-1, -1, bm.getWidth()/2, bm.getHeight()/2);
            else if(fliph) canvas.scale(-1, 1, bm.getWidth()/2, bm.getHeight()/2);
            else if(flipv) canvas.scale(1, -1, bm.getWidth()/2, bm.getHeight()/2);
            canvas.rotate(180,mWidth/2,mHeight/2);
            canvas.drawBitmap(mBitmap, src, new Rect(0,0,mWidth,mHeight), null);
            return bm;
        } else if( rotation==90 ) {
            Bitmap bm = Bitmap.createBitmap(mHeight, mWidth, Bitmap.Config.RGB_565);
            Canvas canvas = new Canvas(bm);
            if(fliph && flipv) canvas.scale(-1, -1, bm.getWidth()/2, bm.getHeight()/2);
            else if(fliph) canvas.scale(1, -1, bm.getWidth()/2, bm.getHeight()/2);
            else if(flipv) canvas.scale(-1, 1, bm.getWidth()/2, bm.getHeight()/2);
            canvas.rotate(90,0,0);
            canvas.drawBitmap(mBitmap, src, new Rect(0,-mHeight,mWidth,0), null);
            return bm;
        } else if( rotation==270 ) {
            Bitmap bm = Bitmap.createBitmap(mHeight, mWidth, Bitmap.Config.RGB_565);
            Canvas canvas = new Canvas(bm);
            if(fliph && flipv) canvas.scale(-1, -1, bm.getWidth()/2, bm.getHeight()/2);
            else if(fliph) canvas.scale(1, -1, bm.getWidth()/2, bm.getHeight()/2);
            else if(flipv) canvas.scale(-1, 1, bm.getWidth()/2, bm.getHeight()/2);
            canvas.rotate(270,0,0);
            canvas.drawBitmap(mBitmap, src, new Rect(-mWidth,0,0,mHeight), null);
            return bm;
        }

        return null;
    }
}
