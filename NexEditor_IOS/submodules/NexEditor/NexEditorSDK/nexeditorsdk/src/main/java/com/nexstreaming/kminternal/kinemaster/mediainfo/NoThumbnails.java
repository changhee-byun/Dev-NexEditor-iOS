package com.nexstreaming.kminternal.kinemaster.mediainfo;

import android.graphics.Bitmap;

/**
 * Created by jeongwook.yoon on 2016-12-01.
 */

class NoThumbnails implements Thumbnails{
    private static NoThumbnails instance;

    public static Thumbnails getInstance() {
        if( instance==null ) instance = new NoThumbnails();
        return instance;
    }

    private NoThumbnails(){}

    @Override
    public int getWidth(int rotation) {
        return 0;
    }

    @Override
    public int getHeight(int rotation) {
        return 0;
    }

    @Override
    public float getWidthForHeight(int rotation, float height) {
        return 0;
    }

    @Override
    public Bitmap getThumbnailBitmap(int rotation, int time, boolean fliph, boolean flipv) {
        return null;
    }

    @Override
    public Bitmap getThumbnailBitmapNoCache(int rotation, int time, boolean fliph, boolean flipv) {
        return null;
    }

    @Override
    public int closestActualThumbTime(int time) {
        return 0;
    }

    @Override
    public int[] getThumbnailTimeLine(){
        return null;
    }
}
