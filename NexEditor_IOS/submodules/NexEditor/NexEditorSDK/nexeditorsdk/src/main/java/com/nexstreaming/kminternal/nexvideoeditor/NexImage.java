package com.nexstreaming.kminternal.nexvideoeditor;

import android.graphics.Bitmap;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;

public class NexImage {
	private final Bitmap mBitmap;
	private final int mWidth;
	private final int mHeight;
	private final int mLoadedType;
	private static final String LOG_TAG = "NexImage";

	public NexImage( Bitmap bitmap, int width, int height, int loadtype ) {
		if( LL.D ) Log.d(LOG_TAG,"new NexImage(" + bitmap + "," + width + "," + height+ "," + loadtype +")");
		mBitmap = bitmap;
		mWidth = width;
		mHeight = height;
		mLoadedType = loadtype;
	}
	public NexImage( Bitmap bitmap, int width, int height ) {
		if( LL.D ) Log.d(LOG_TAG,"new NexImage(" + bitmap + "," + width + "," + height +")");
		mBitmap = bitmap;
		mWidth = width;
		mHeight = height;
		mLoadedType = 1;
	}
    public int getWidth()
    {
    	return mWidth;
    }
    public int getHeight()
    {
    	return mHeight;
    }
	public int getLoadedType()
	{
		return mLoadedType;
	}
    public void getPixels(int[] pixels)
    {
    	if( mBitmap == null ) return;
    	try {
    		mBitmap.getPixels(pixels, 0, mWidth, 0, 0, mWidth, mHeight);
    	} catch ( ArrayIndexOutOfBoundsException e ) {
    		throw new ArrayIndexOutOfBoundsException("w=" + mWidth + " h=" + mHeight + " bm=" + mBitmap.getWidth() + "x" + mBitmap.getHeight() + " pixels=" + (pixels==null?"null":pixels.length));
    	}
        /*if(!m_premult) 
        {
        	for( int i=0; i<pixels.length; i++ ) 
        	{
        		int a = (pixels[i]>>24) & 0xFF;
        		if( a==0 ) {
        			pixels[i] = 0x00000000;
        		} else {
	        		int b = pixels[i] & 0xFF; 
	        		int g = (pixels[i]>>8) & 0xFF; 
	        		int r = (pixels[i]>>16) & 0xFF; 
	        		r = (r * a) / 255;
	        		g = (g * a) / 255;
	        		b = (b * a) / 255;
	        		pixels[i] = (a<<24)|(r<<16)|(g<<8)|b;
        		}
        	}
        }*/
    }

    public void getPixels(int[] pixels, int offset, int stride, int x, int y, int width, int height)
    {
    	if( mBitmap == null ) return;
    	if( y + height > mBitmap.getHeight() ) {
    		// Work-around for MANTIS 1858.  This should prevent the crash,
    		// but the original cause is still known.  The exception that's
    		// occuring is:
    		//     java.lang.IllegalArgumentException: y + height must be <= bitmap.height()
    		// By returning in this case, we prevent the exception.  The log has been
    		// added to try to identify the original cause.
    		//  --Matthew 2012-04-12
    		if(LL.D) Log.d(LOG_TAG,"getPixels() WARNING: y + height exceeds bitmap height!!; offset=" + offset + 
    				"; stride=" + stride + "; x,y=" + x + "," + y + 
    				"; width,height=" + width + "," + height + 
    				"; mWidth,mHeight=" + mWidth + "," + mHeight +
    				"; pixels.length=" + pixels.length + 
    				"; mBitmap {width=" + mBitmap.getWidth() + 
    				"; height=" + mBitmap.getHeight() + 
    				"}" );
    		return;
    	} else if( x + width > mBitmap.getWidth() ) {
    		// Work-around for MANTIS 1911.  This should prevent the crash,
    		// but the original cause is still known.  The exception that's
    		// occuring is:
    		//     java.lang.IllegalArgumentException: x + width must be <= bitmap.width()
    		// By returning in this case, we prevent the exception.  The log has been
    		// added to try to identify the original cause.
    		//  --Matthew 2012-04-23
    		if(LL.D) Log.d(LOG_TAG,"getPixels() WARNING: y + height exceeds bitmap height!!; offset=" + offset + 
    				"; stride=" + stride + "; x,y=" + x + "," + y + 
    				"; width,height=" + width + "," + height + 
    				"; mWidth,mHeight=" + mWidth + "," + mHeight +
    				"; pixels.length=" + pixels.length + 
    				"; mBitmap {width=" + mBitmap.getWidth() + 
    				"; height=" + mBitmap.getHeight() + 
    				"}" );
    		return;
    	}
        mBitmap.getPixels(pixels, offset, stride, x, y, width, height);
    }
    
    public Bitmap getBitmap()
    {
    	return mBitmap;
    }

    public void recycle()
    {
        mBitmap.recycle();
        Log.d(LOG_TAG,"recycle Bitmap from native" );
    }

}
