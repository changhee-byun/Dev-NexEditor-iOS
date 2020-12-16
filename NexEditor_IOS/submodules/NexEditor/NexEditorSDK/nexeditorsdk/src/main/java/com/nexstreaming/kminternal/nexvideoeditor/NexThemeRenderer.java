package com.nexstreaming.kminternal.nexvideoeditor;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.LL;

import android.content.Context;
import android.os.Build;
import android.util.Log;
import android.view.Surface;

// TODO: Temporarily public to work with kmpackage (move to kmpackage namespace eventually, is better, but need to update jni side too)
public class NexThemeRenderer {
	
	private long m_instanceHandle = 0;
	
	public native void init( NexImageLoader bitmapManager );
    public native void surfaceChange( int width, int height );
    public native void prepareSurface( Surface surface );
	public native void setCTS( int cts );
	public native void render();
	public native void deinit(boolean isDetachedContext);
	public native int clearRenderItems();
	public native int loadRenderItem(String strEffectID, String strEffectData);
	public native void loadThemes( String themeSetBody );
	public native void setTransitionEffect( String effectId, String effectOptions, int clipIndex, int totalClipCount, int effectStartTime, int effectEndTime );
	public native void setClipEffect( String effectId, String effectOptions, int clipIndex, int totalClipCount, int clipStartTime, int clipEndTime, int effectStartTime, int effectEndTime );
	public native void clearTransitionEffect();
	public native void clearClipEffect();
	public native void setPlaceholders(String primary, String secondary);
    public native void recycleRenderer( );
    public native void clearSurface( int tag );
    public native int releaseContext( boolean swapBuffers );
    public native int aquireContext( );
	public native int setForceRTT(int val);

    public void loadThemeData( String themeData ) {
    	loadThemes( themeData );
    }
    
    void loadThemeSet( Context context, int resource ) {
    	
    	String themeData = null;
    	
    	try {
	    	InputStream s = context.getResources().openRawResource(resource);
	    	byte[] b = new byte[s.available()];
	    	s.read(b);
	    	themeData = new String(b);
    	} catch( IOException e ) {
    		themeData = "";
    	}
    	loadThemes( themeData );
    }
	
	void setThemeInstanceHandle( long handle ) {
		m_instanceHandle = handle;
	}
	
	long getThemeInstanceHandle( ) {
		return m_instanceHandle;
	}
	
    static {
    	try {
    		
    		/*
    		switch( Build.VERSION.SDK_INT ) {
    		case 8:			// FROYO
        		if(LL.E) Log.e("NexThemeRenderer", "[NexThemeRenderer.java] No binaries for API : " + Build.VERSION.SDK_INT + " (FROYO)");
    			break;
    		case 9:			// GB
    		case 10:		// GB-MR1
	    		System.loadLibrary("nexcal_oc_dec_gb"); 
	    		System.loadLibrary("nexcalbody_iomxenc_gb");
	    		System.loadLibrary("nexvideoeditor_gb");
    			break;
    		case 11:		// HONEYCOMB
    		case 12:		// HONEYCOMB-MR1
    		case 13:		// HONEYCOMB-MR2
        		if(LL.E) Log.e("NexThemeRenderer", "[NexThemeRenderer.java] No binaries for API : " + Build.VERSION.SDK_INT + " (HONEYCOMB)");
    			break;
    		case 14:		// ICS
    		case 15:		// ICS-MR1
	    		System.loadLibrary("nexcal_oc_dec_ics");
	    		System.loadLibrary("nexcalbody_iomxenc_ics");
	    		System.loadLibrary("nexvideoeditor_ics");
    			break;
    		default:
        		if(LL.E) Log.e("NexThemeRenderer", "[NexEditor.java] No binaries for API : " + Build.VERSION.SDK_INT);
    			break;
    		}
    		*/
    		
    		System.loadLibrary("stlport_shared");
    		// System.loadLibrary("nexralbody_audio");
            /*
            if( new File("/system/lib/","libnexeditorsdk.so").exists() ) {
                if(LL.D) Log.d("ThemeRenderer","libnexeditorsdk.so libs found in: /system/lib/" );
                System.load("/system/lib/libnexeditorsdk.so");
            } else {
                if(LL.D) Log.d("ThemeRenderer","libnexeditorsdk.so libs NOT FOUND in: /system/lib/");
                System.loadLibrary("nexeditorsdk");
            }
            */
            System.loadLibrary("nexeditorsdk"+ EditorGlobal.SO_FREFIX);
    	} catch ( UnsatisfiedLinkError exc ) {
    		if(LL.E) Log.e("NexThemeRenderer", "[NexThemeRenderer.java] nexeditor load failed : " + exc);
    		;;
    	}
    } 
}
