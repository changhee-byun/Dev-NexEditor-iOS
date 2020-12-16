package com.nexstreaming.kminternal.kinemaster.mediainfo;

import android.graphics.Bitmap;


public interface ThumbnailCallbackBitmap extends ThumbnailCallback {
    void processThumbnail( Bitmap bitmap, int index, int totalCount, int timestamp );
}
