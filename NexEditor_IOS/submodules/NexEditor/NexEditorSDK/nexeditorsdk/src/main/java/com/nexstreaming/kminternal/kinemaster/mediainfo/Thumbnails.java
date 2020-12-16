package com.nexstreaming.kminternal.kinemaster.mediainfo;

import android.graphics.Bitmap;

public interface Thumbnails {
	int getWidth(int rotation);
	int getHeight(int rotation);
	float getWidthForHeight( int rotation, float height );
	Bitmap getThumbnailBitmap( int rotation, int time, boolean fliph, boolean flipv );
	Bitmap getThumbnailBitmapNoCache( int rotation, int time, boolean fliph, boolean flipv );
	int closestActualThumbTime( int time );
	int[] getThumbnailTimeLine();
	
}
