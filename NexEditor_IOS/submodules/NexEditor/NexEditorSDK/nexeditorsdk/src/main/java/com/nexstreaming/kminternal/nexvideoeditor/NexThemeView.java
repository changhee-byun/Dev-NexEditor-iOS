
package com.nexstreaming.kminternal.nexvideoeditor;


import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import com.nexstreaming.kminternal.kinemaster.config.LL;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;

public class NexThemeView extends TextureView implements TextureView.SurfaceTextureListener {
	
    private static String LOG_TAG = "VideoEditor_ThemeView";
    //private static final boolean DEBUG = false;
    private int	mCurrentTime;
    private NexEditor mEditor = null;
    private static float mAspectRatio = 1280.0f/720.0f;
    private RenderType mRenderType = RenderType.None;
    private boolean mHadNativeRenderSinceClear = false;
    private Queue<OnClearCompletionListener> mClearListenerList = new LinkedList<OnClearCompletionListener>();
    private Handler mHandler = new Handler();
    private boolean mBlackOut = false;
    private boolean isAvailable = false;
    private int m_width = 0;
    private int m_height = 0;
    private CaptureListener m_captureListener = null;
    private SurfaceTexture actualSurfaceTexture;
    private Surface actualSurface;
    private List<Runnable> postOnPrepareSurfaceRunnables = new ArrayList<>();
    private NexThemeViewEventListener nexThemeViewCallback;

    public static final int kEventType_Available = 1;
    public static final int kEventType_SizeChanged = 2;
    public static final int kEventType_Destroyed = 3;
    public interface NexThemeViewEventListener {
        void onEventNotify(int eventType, Object object, int param1, int param2, int param3 );
    }


    protected void setNotify(NexThemeViewEventListener callback) {
        nexThemeViewCallback = callback;
    }

    private void updateActualSurface(SurfaceTexture surface) {
        if (surface != actualSurfaceTexture) {
            actualSurfaceTexture = surface;
            if (actualSurfaceTexture != null) {
                actualSurface = new Surface(actualSurfaceTexture);
            } else {
                actualSurface = null;
            }
        }

    }

    private String diagString() {
        return "[0x" + Integer.toHexString(System.identityHashCode(this)) + " " + getWidth() + "x" + getHeight() + "] ";
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        updateActualSurface(surface);
        m_width = width;
        m_height = height;
        Log.d(LOG_TAG, "onSurfaceTextureAvailable " + diagString() + (mEditor == null ? "(editor is null)" : "(editor is SET)"));
        if (mEditor != null) {
            mEditor.prepareSurfaceForView(actualSurface);
            mEditor.notifySurfaceChanged();
            for (Runnable r : postOnPrepareSurfaceRunnables) {
                post(r);
            }
            postOnPrepareSurfaceRunnables.clear();
        }
        if (nexThemeViewCallback != null) {
            nexThemeViewCallback.onEventNotify(kEventType_Available,null,width, height,0);
        }
        isAvailable = true;
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        updateActualSurface(surface);
        m_width = width;
        m_height = height;

        Log.d(LOG_TAG, "onSurfaceTextureSizeChanged " + diagString() + (mEditor == null ? "(editor is null)" : "(editor is SET)"));
        if (mEditor != null) {
            mEditor.prepareSurfaceForView(actualSurface);
            mEditor.notifySurfaceChanged();
        }
        if (nexThemeViewCallback != null) {
            nexThemeViewCallback.onEventNotify(kEventType_SizeChanged,null,width, height,0);
        }
    }

    public void postOnPrepareSurface( Runnable runnable ) {
        postOnPrepareSurfaceRunnables.add(runnable);
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        Log.d(LOG_TAG, "onSurfaceTextureDestroyed " + diagString() + (mEditor==null?"(editor is null)":"(editor is SET)") );
        if(mEditor != null) {
            mEditor.prepareSurfaceForView( null );
        }
        actualSurfaceTexture = null;
        actualSurface = null;
        if (nexThemeViewCallback != null) {
            nexThemeViewCallback.onEventNotify(kEventType_Destroyed,null,0,0,0);
        }
        isAvailable = false;
        return true;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {

    }
//	RenderListener m_renderListener = null;
    
	public static abstract class RenderListener {
		abstract void onStartRender();
		abstract void onFinishRender();
	}
	
	public static abstract class CaptureListener {
		public abstract void onCapture( Bitmap bm );
	}
	
	public void capture( CaptureListener listener ) {
		m_captureListener = listener;
//		requestRender();
	}
	
	public void setBlackOut( boolean blackOut ) {
		mBlackOut = blackOut;
	}
    
    private enum RenderType {
    	None, Native, Clear
    }
    
    public static abstract class OnClearCompletionListener {
    	public abstract void onClearComplete();
    }

    public NexThemeView(Context context) {
        super(context);
        init(false, 0, 1);
    }

    public NexThemeView(Context context, AttributeSet attrs) {
		super(context, attrs);
        init(false, 0, 0);
	}

	public NexThemeView(Context context, boolean translucent, int depth, int stencil) {
        super(context);
        init(translucent, depth, stencil);
    }

    void linkToEditor(NexEditor editor) {
        if (LL.D)
            Log.d(LOG_TAG, "linkToEditor " + diagString() + " editor=" + (editor == null ? "NULL" : "not-null"));
        mEditor = editor;
        if (mEditor != null && actualSurface != null) {
            mEditor.prepareSurfaceForView(actualSurface);
        }
//		m_renderListener = listener;
    }

    public static void setAspectRatio(float aspectRatio) {
        mAspectRatio = aspectRatio;
    }

    public static float getAspectRatio() {
        return mAspectRatio;
    }

    /*@Override
	public void onPause() {
    	Log.d(TAG,"onPause()");
		super.onPause();
	}

	@Override
	public void onResume() {
    	Log.d(TAG,"onResume()");
		super.onResume();
	}
*/
	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int widthMode = View.MeasureSpec.getMode(widthMeasureSpec);
        int heightMode = View.MeasureSpec.getMode(heightMeasureSpec);

    	int width = View.MeasureSpec.getSize(widthMeasureSpec);
    	int height = View.MeasureSpec.getSize(heightMeasureSpec);

        int measuredHeight = 0;
        int measuredWidth = 0;
        if( widthMode == View.MeasureSpec.EXACTLY && heightMode == View.MeasureSpec.EXACTLY ) {
            measuredHeight = height;
            measuredWidth = width;
        }
        else if( widthMode == View.MeasureSpec.EXACTLY ) {
            measuredHeight = (int) Math.min( height, width / mAspectRatio );
            measuredWidth = (int) (measuredHeight * mAspectRatio);

    	}
        else if( heightMode == View.MeasureSpec.EXACTLY) {
            measuredWidth = (int) Math.min( width, height * mAspectRatio );
            measuredHeight = (int) (measuredWidth / mAspectRatio);
        }
        else {
            if ( width > height * mAspectRatio ) {
                measuredHeight = height;
                measuredWidth = (int)( measuredHeight * mAspectRatio );
            } else {
                measuredWidth = width;
                measuredHeight = (int) (measuredWidth / mAspectRatio);
            }
        }
    	setMeasuredDimension(measuredWidth, measuredHeight);

        Log.d(LOG_TAG, String.format("onMeasure(%X %X %d %d %f)", widthMeasureSpec, heightMeasureSpec, width, height, mAspectRatio));
	}

    public boolean isSurfaceAvailable() {
        return isAvailable;
    }



	private void init(boolean translucent, int depth, int stencil) {
    	  	
        /* By default, GLSurfaceView() creates a RGB_565 opaque surface.
         * If we want a translucent one, we should change the surface's
         * format here, using PixelFormat.TRANSLUCENT for GL Surfaces
         * is interpreted as any 32-bit surface with alpha by SurfaceFlinger.
         */
        /*if (translucent) {
            this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        }*/

        /* Setup the context factory for 2.0 rendering.
         * See ContextFactory class definition below
         */
        //setEGLContextFactory(new ContextFactory());

        /* We need to choose an EGLConfig that matches the format of
         * our surface exactly. This is going to be done in our
         * custom config chooser. See ConfigChooser class definition
         * below.
         */
        /*setEGLConfigChooser( translucent ?
                             new ConfigChooser(8, 8, 8, 8, depth, stencil) :
                             new ConfigChooser(5, 6, 5, 0, depth, stencil) );*/
        
        //this.getHolder().setFormat(PixelFormat.RGBX_8888); 

        setSurfaceTextureListener(this);

//		getHolder().addCallback(new SurfaceHolder.Callback() {
//
//			public void surfaceDestroyed(SurfaceHolder holder)
//			{
////				Log.d(LOG_TAG, "SurfaceHolder: surfaceDestroyed " + (mEditor==null?"(editor is null)":"(editor is SET)") );
////				if(mEditor != null) {
////					mEditor.prepareSurface( null );
////				}
//			}
//
//			public void surfaceCreated(SurfaceHolder holder)
//			{
////				Log.d(LOG_TAG, "SurfaceHolder: surfaceCreated " + (mEditor==null?"(editor is null)":"(editor is SET)") );
////				if(mEditor != null) {
////					mEditor.prepareSurface( holder.getSurface() );
////				}
//			}
//
//			public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
//			{
////				Log.d(LOG_TAG, "SurfaceHolder: surfaceChanged " + (mEditor==null?"(editor is null)":"(editor is SET)") );
////				if(mEditor != null) {
////					mEditor.prepareSurface( holder.getSurface() );
////					mEditor.notifySurfaceChanged();
////				}
//			}
//		});
		
        //setEGLConfigChooser(8, 8, 8, 8, depth, stencil);
        

        /* Set the renderer responsible for frame rendering */
        /*Renderer render = new Renderer();
        setRenderer(render);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        Log.d(TAG, "GL View Created " + render); */
    }

    /*private static class ContextFactory implements GLSurfaceView.EGLContextFactory {
        private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
        public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
            if(LL.W) Log.w(TAG, "creating OpenGL ES 2.0 context");
            checkEglError("Before eglCreateContext", egl);
            int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
            EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
            checkEglError("After eglCreateContext", egl);
            return context;
        }

        public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
            if(LL.W) Log.w(TAG, "destroying OpenGL ES 2.0 context");
            egl.eglDestroyContext(display, context);
        }
    }*/

    /*private static void checkEglError(String prompt, EGL10 egl) {
        int error;
        while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS) {
            if(LL.E) Log.e(TAG, String.format("%s: EGL error: 0x%x", prompt, error));
        }
    }*/
    
    /*
    @Override
	public void requestRender() {
        if(LL.E) Log.e(TAG, "requestRender()");
		super.requestRender();
	}
*/
	public void requestDraw(int uiCurrentTime)
    {
        if(LL.E) Log.e(LOG_TAG, "requestDraw(" + uiCurrentTime + ")");
    	mRenderType = RenderType.Native;
    	mCurrentTime = uiCurrentTime;
    	//requestRender();
    }
/*
    public void clear()
    {
        if(LL.E) Log.e(TAG, "clear() mRenderType=" + mRenderType.name());
    	if( mRenderType!=RenderType.Native )
    		mRenderType = RenderType.Clear;
    	//requestRender();
    }

    public void clear(OnClearCompletionListener onCompletionListener)
    {
        if(LL.E) Log.e(TAG, "clear(listener) mRenderType=" + mRenderType.name());
    	if( onCompletionListener!=null )
    		mClearListenerList.add(onCompletionListener);
    	if( mRenderType!=RenderType.Native )
    		mRenderType = RenderType.Clear;
    	//requestRender();
    }

    */
    /*
    private static class ConfigChooser implements GLSurfaceView.EGLConfigChooser {

        public ConfigChooser(int r, int g, int b, int a, int depth, int stencil) {
            mRedSize = r;
            mGreenSize = g;
            mBlueSize = b;
            mAlphaSize = a;
            mDepthSize = depth;
            mStencilSize = stencil;
        }

        private static int EGL_OPENGL_ES2_BIT = 4;
        private static int[] s_configAttribs2 =
        {
            EGL10.EGL_RED_SIZE, 4,
            EGL10.EGL_GREEN_SIZE, 4,
            EGL10.EGL_BLUE_SIZE, 4,
            EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL10.EGL_NONE
        };
        
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {

            // Get the number of minimally matching EGL configurations
            int[] num_config = new int[1];
            egl.eglChooseConfig(display, s_configAttribs2, null, 0, num_config);

            int numConfigs = num_config[0];

            if (numConfigs <= 0) {
                throw new IllegalArgumentException("No configs match configSpec");
            }

            // Allocate then read the array of minimally matching EGL configs
            EGLConfig[] configs = new EGLConfig[numConfigs];
            egl.eglChooseConfig(display, s_configAttribs2, configs, numConfigs, num_config);

            if (DEBUG) {
                 printConfigs(egl, display, configs);
            }
            // Now return the "best" one
            return chooseConfig(egl, display, configs);
        }

        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display, EGLConfig[] configs) 
        {
        	
            for(EGLConfig config : configs) {
            	
                int d = findConfigAttrib(egl, display, config, EGL10.EGL_DEPTH_SIZE, 0);
                int s = findConfigAttrib(egl, display, config, EGL10.EGL_STENCIL_SIZE, 0);

                // We need at least mDepthSize and mStencilSize bits
                if (d < mDepthSize || s < mStencilSize)
                    continue;

                // We want an *exact* match for red/green/blue/alpha
                int r = findConfigAttrib(egl, display, config, EGL10.EGL_RED_SIZE, 0);
                int g = findConfigAttrib(egl, display, config, EGL10.EGL_GREEN_SIZE, 0);
                int b = findConfigAttrib(egl, display, config, EGL10.EGL_BLUE_SIZE, 0);
                int a = findConfigAttrib(egl, display, config, EGL10.EGL_ALPHA_SIZE, 0);

                if (r == mRedSize && g == mGreenSize && b == mBlueSize && a == mAlphaSize)
                    return config;
                
            }
            
            return null;
        }

        private int findConfigAttrib(EGL10 egl, EGLDisplay display,
                EGLConfig config, int attribute, int defaultValue) 
        {

            if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) {
                return mValue[0];
            }
            
            return defaultValue;
        }

        private void printConfigs(EGL10 egl, EGLDisplay display, EGLConfig[] configs) 
        {
            int numConfigs = configs.length;
            if(LL.W) Log.w(TAG, String.format("%d configurations", numConfigs));
            for (int i = 0; i < numConfigs; i++) {
                if(LL.W) Log.w(TAG, String.format("Configuration %d:\n", i));
                printConfig(egl, display, configs[i]);
            }
        }

        private void printConfig(EGL10 egl, EGLDisplay display, EGLConfig config) 
        {
            int[] attributes = {
                    EGL10.EGL_BUFFER_SIZE,
                    EGL10.EGL_ALPHA_SIZE,
                    EGL10.EGL_BLUE_SIZE,
                    EGL10.EGL_GREEN_SIZE,
                    EGL10.EGL_RED_SIZE,
                    EGL10.EGL_DEPTH_SIZE,
                    EGL10.EGL_STENCIL_SIZE,
                    EGL10.EGL_CONFIG_CAVEAT,
                    EGL10.EGL_CONFIG_ID,
                    EGL10.EGL_LEVEL,
                    EGL10.EGL_MAX_PBUFFER_HEIGHT,
                    EGL10.EGL_MAX_PBUFFER_PIXELS,
                    EGL10.EGL_MAX_PBUFFER_WIDTH,
                    EGL10.EGL_NATIVE_RENDERABLE,
                    EGL10.EGL_NATIVE_VISUAL_ID,
                    EGL10.EGL_NATIVE_VISUAL_TYPE,
                    0x3030, // EGL10.EGL_PRESERVED_RESOURCES,
                    EGL10.EGL_SAMPLES,
                    EGL10.EGL_SAMPLE_BUFFERS,
                    EGL10.EGL_SURFACE_TYPE,
                    EGL10.EGL_TRANSPARENT_TYPE,
                    EGL10.EGL_TRANSPARENT_RED_VALUE,
                    EGL10.EGL_TRANSPARENT_GREEN_VALUE,
                    EGL10.EGL_TRANSPARENT_BLUE_VALUE,
                    0x3039, // EGL10.EGL_BIND_TO_TEXTURE_RGB,
                    0x303A, // EGL10.EGL_BIND_TO_TEXTURE_RGBA,
                    0x303B, // EGL10.EGL_MIN_SWAP_INTERVAL,
                    0x303C, // EGL10.EGL_MAX_SWAP_INTERVAL,
                    EGL10.EGL_LUMINANCE_SIZE,
                    EGL10.EGL_ALPHA_MASK_SIZE,
                    EGL10.EGL_COLOR_BUFFER_TYPE,
                    EGL10.EGL_RENDERABLE_TYPE,
                    0x3042 // EGL10.EGL_CONFORMANT
            };
            String[] names = {
                    "EGL_BUFFER_SIZE",
                    "EGL_ALPHA_SIZE",
                    "EGL_BLUE_SIZE",
                    "EGL_GREEN_SIZE",
                    "EGL_RED_SIZE",
                    "EGL_DEPTH_SIZE",
                    "EGL_STENCIL_SIZE",
                    "EGL_CONFIG_CAVEAT",
                    "EGL_CONFIG_ID",
                    "EGL_LEVEL",
                    "EGL_MAX_PBUFFER_HEIGHT",
                    "EGL_MAX_PBUFFER_PIXELS",
                    "EGL_MAX_PBUFFER_WIDTH",
                    "EGL_NATIVE_RENDERABLE",
                    "EGL_NATIVE_VISUAL_ID",
                    "EGL_NATIVE_VISUAL_TYPE",
                    "EGL_PRESERVED_RESOURCES",
                    "EGL_SAMPLES",
                    "EGL_SAMPLE_BUFFERS",
                    "EGL_SURFACE_TYPE",
                    "EGL_TRANSPARENT_TYPE",
                    "EGL_TRANSPARENT_RED_VALUE",
                    "EGL_TRANSPARENT_GREEN_VALUE",
                    "EGL_TRANSPARENT_BLUE_VALUE",
                    "EGL_BIND_TO_TEXTURE_RGB",
                    "EGL_BIND_TO_TEXTURE_RGBA",
                    "EGL_MIN_SWAP_INTERVAL",
                    "EGL_MAX_SWAP_INTERVAL",
                    "EGL_LUMINANCE_SIZE",
                    "EGL_ALPHA_MASK_SIZE",
                    "EGL_COLOR_BUFFER_TYPE",
                    "EGL_RENDERABLE_TYPE",
                    "EGL_CONFORMANT"
            };
            int[] value = new int[1];
            for (int i = 0; i < attributes.length; i++) {
                int attribute = attributes[i];
                String name = names[i];
                if ( egl.eglGetConfigAttrib(display, config, attribute, value)) {
                    if(LL.W) Log.w(TAG, String.format("  %s: %d\n", name, value[0]));
                } else {
                    // if(LL.W) Log.w(TAG, String.format("  %s: failed\n", name));
                    while (egl.eglGetError() != EGL10.EGL_SUCCESS);
                }
            }
        }
        

        // Subclasses can adjust these values:
        protected int mRedSize;
        protected int mGreenSize;
        protected int mBlueSize;
        protected int mAlphaSize;
        protected int mDepthSize;
        protected int mStencilSize;
        private int[] mValue = new int[1];
    }
    */
	/*private class Renderer implements GLSurfaceView.Renderer 
    {
		//int m_phase = 0;
		public void onDrawFrame(GL10 gl) {
			
			if(m_renderListener!=null)
				m_renderListener.onStartRender();
			
			RenderType renderType = mRenderType;
			mRenderType = RenderType.None;
			Log.d(TAG, "onDrawFrame START for mCurrentTime=" + mCurrentTime + "; RenderType=" + renderType.name() + "; mHadNativeRenderSinceClear=" + mHadNativeRenderSinceClear );
			boolean bRenderedFrame = false;
			switch( renderType ) {
			case Clear:
				mHadNativeRenderSinceClear = false;
				break;
			case Native:
				if( mEditor != null ) {
					mEditor.nativeVideoRender(mCurrentTime);
					bRenderedFrame = true;
					mHadNativeRenderSinceClear = true;
				}
				break;
			case None:
				// For call render calback after did not request
				if( mEditor != null && mHadNativeRenderSinceClear ) {
					mEditor.nativeVideoRender(-1);
					bRenderedFrame = true;
				}
				break;
			}
			
			
			if( !bRenderedFrame || mBlackOut ) {
				// If we didn't render something, the default behavior
				// to clear the view to black
				
				gl.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				gl.glClear(GL10.GL_COLOR_BUFFER_BIT);
			}
			
			if( !mClearListenerList.isEmpty() ) {
				mHandler.post(new Runnable() {
					public void run() {
						OnClearCompletionListener listener;
						while( (listener=mClearListenerList.poll())!=null ) {
							listener.onClearComplete();
						}
					}
				});
			}
			
			Log.d(TAG, "onDrawFrame END for mCurrentTime=" + mCurrentTime + "; bRenderedFrame=" + bRenderedFrame);
			if(m_renderListener!=null) {
				m_renderListener.onFinishRender();
			}
			
			/*m_phase = (m_phase+5)%100;
			gl.glClearColor(0.0f, ((float)m_phase)/100f, 1.0f, 1.0f);
			gl.glClear(GL10.GL_COLOR_BUFFER_BIT);* /
			
			if( m_captureListener!=null ) { 
				
				int width = m_width; //nextHighestPowerOfTwo(m_width)/2;
				int height = m_height; //nextHighestPowerOfTwo(m_height)/2;
				
				Buffer pixels = ByteBuffer.allocate(width*height*4);
				
				Log.d(TAG, "capture : w,h=" + m_width + "," + m_height + " -> " + width + "," + height + "  hasArray=" + pixels.hasArray()  );

				if( pixels.hasArray() ) {
					
					//gl.glPixelStorei(GL10.GL_PACK_ALIGNMENT, 1);
					gl.glReadPixels(0, 0, width, height, GL10.GL_RGBA, GL10.GL_UNSIGNED_BYTE, pixels);
					
					byte[] pixelData = (byte[])pixels.array();
					byte[] rowData = new byte[width*4];
					for( int i=0; i<height/2; i++ ) {
						System.arraycopy(pixelData, width*i*4, rowData, 0, width*4);
						System.arraycopy(pixelData, width*(height-1-i)*4, pixelData, width*i*4, width*4);
						System.arraycopy(rowData, 0, pixelData, width*(height-1-i)*4, width*4);
					}
					
					Bitmap bm = Bitmap.createBitmap(width, height, Config.ARGB_8888);
					bm.copyPixelsFromBuffer(pixels);
					m_captureListener.onCapture(bm);
					m_captureListener = null;
				}
			}
			
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
        	m_width = width;
        	m_height = height;
			Log.d(TAG, "onSurfaceChanged - START (width=" + width + "; height=" + height + ")" );
			if( mEditor != null )
				mEditor.changeSurface(width, height);
			Log.d(TAG, "onSurfaceChanged - END");
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			Log.d(TAG, "onSurfaceCreated - START");
			if( mEditor != null )
				mEditor.prepareSurface();
			Log.d(TAG, "onSurfaceCreated - END");
        }
    }
	*/
	/*private static int nextHighestPowerOfTwo( int v ) {
		v--;
		v |= v >>> 1;
		v |= v >>> 2;
		v |= v >>> 4;
		v |= v >>> 8;
		v |= v >>> 16;
		v++;
		return v;
	}*/

}
