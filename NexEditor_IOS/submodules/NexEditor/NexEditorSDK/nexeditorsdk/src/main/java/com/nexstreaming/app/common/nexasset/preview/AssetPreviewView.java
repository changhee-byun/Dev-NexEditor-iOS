package com.nexstreaming.app.common.nexasset.preview;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemCategory;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemType;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.config.NexEditorDeviceProfile;
import com.nexstreaming.kminternal.nexvideoeditor.EffectResourceLoader;
import com.nexstreaming.kminternal.nexvideoeditor.NexImageLoader;
import com.nexstreaming.kminternal.nexvideoeditor.NexImageLoader.OverlayPathResolver;
import com.nexstreaming.kminternal.nexvideoeditor.NexThemeRenderer;

import java.io.IOException;
import java.util.Collections;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

public class AssetPreviewView extends GLSurfaceView {

    private static String TAG = "NexThemePreviewView";
    private float mAspectRatio = 1280.0f/720.0f;
    private boolean m_showOnRender = false;
    private Handler m_handler = new Handler();
    private long m_startTime;
    private long m_frameStart;
    private long m_frameEnd;
    private ItemInfo m_effect;
    private ItemInfo m_setEffect;
    private boolean m_swapPlaceholders = false;
    private String m_effectOptions = null;
    private String m_setEffectOptions = null;
    public boolean m_done = false;
    private Object m_renderLock = new Object();
    private long m_effectTime = 2000;
    private long m_setEffectTime = -1;
    private long m_pauseTime = 1000;
    private int m_absTime = 0;
    private boolean m_isClipEffect;
    private boolean m_isRenderItem;

    private AssetPackageManager m_assetPackageManager;

    private NexThemeRenderer m_nexThemeRenderer = null;
    private OverlayPathResolver m_overlayPathResolver = null;

    private static long miliTime() {
        return System.nanoTime()/1000000;
    }

    public AssetPreviewView(Context context) {
        super(context);
        init(false, 0, 1);
    }

    public AssetPreviewView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(false, 0, 1);
    }

    public AssetPreviewView(Context context, boolean translucent, int depth, int stencil) {
        super(context);
        init(translucent, depth, 1);
    }

    public void setAspectRatio( float aspectRatio ) {
        mAspectRatio = aspectRatio;
    }

    public float getAspectRatio() {
        return mAspectRatio;
    }

    public void setOverlayPathResolver( OverlayPathResolver overlayPathResolver ) {
        m_overlayPathResolver = overlayPathResolver;
    }

    public void setEffectOptions(String effectOptions) {
        if( effectOptions!=null && m_effectOptions !=null && effectOptions.equals(m_effectOptions) )
            return;
        m_effectOptions = effectOptions;
    }

    public String getEffectOptions() {
        return m_effectOptions;
    }

    public void deinitRenderer() {

    }

    public void setEffect(final String effect_id) {
        setEffect(AssetPackageManager.getInstance(getContext()).getInstalledItemById(effect_id));
    }

    public void setEffect(ItemInfo effect) {

        if( effect==m_effect )
            return;

        m_startTime = miliTime();
        //m_isClipEffect = (effect instanceof ClipEffect);
        m_isClipEffect = ((effect.getCategory() == ItemCategory.effect) ? true : false);
        m_isRenderItem = (effect.getType() == ItemType.renderitem);
        m_effect = effect;
    }

    public void setEffectTime( int effectTime ) {
        if( effectTime==m_effectTime )
            return;
        m_startTime = miliTime();
        m_effectTime = effectTime;
    }

    public void setPauseTime( int pauseTime ) {
        m_pauseTime = pauseTime;
    }

    public int getPauseTime() {
        return (int)m_pauseTime;
    }

    public void stepForward( int amount ) {
        m_absTime += amount;
        if( m_absTime < 0 )
            m_absTime = 0;
        if( m_absTime > 60 )
            m_absTime = 60;
        m_effectTime = 0;
    }

    public void stepBackward( int amount ) {
        m_absTime -= amount;
        if( m_absTime < 0 )
            m_absTime = 0;
        if( m_absTime > 60 )
            m_absTime = 60;
        m_effectTime = 0;
    }

    public void showOnRender() {
        m_showOnRender = true;
    }

    @Override
    protected void onDetachedFromWindow() {
        destroyRenderer();
        super.onDetachedFromWindow();
    }

    private void destroyRenderer() {
        synchronized(m_renderLock) {
            if( m_nexThemeRenderer!=null ) {
                m_nexThemeRenderer.deinit(true);
                m_nexThemeRenderer = null;
            }
        }
    }

    private void makeRenderer() {
        if( m_nexThemeRenderer==null && !m_done) {
            m_nexThemeRenderer = new NexThemeRenderer();
            EffectResourceLoader effectResourceLoader = AssetPackageManager.getInstance(getContext()).getResourceLoader();
            m_nexThemeRenderer.init(new NexImageLoader(getContext().getResources(), effectResourceLoader, m_overlayPathResolver, 1440, 810, 1500000  ));
            if( m_swapPlaceholders && !m_isClipEffect )
                m_nexThemeRenderer.setPlaceholders("placeholder2.jpg", "placeholder1.jpg");
            else
                m_nexThemeRenderer.setPlaceholders("placeholder1.jpg", "placeholder2.jpg");
        }
    }

    @Override
    protected void onAttachedToWindow() {
        makeRenderer();
        super.onAttachedToWindow();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int width = MeasureSpec.getSize(widthMeasureSpec);
        int height = MeasureSpec.getSize(heightMeasureSpec);
        if( mAspectRatio>0 ) {
            if( height*mAspectRatio > width ) {
                height = (int)(width/mAspectRatio);
            } else {
                width = (int)(height*mAspectRatio);
            }
        }
        setMeasuredDimension(width, height);
    }

    private void init(boolean translucent, int depth, int stencil) {

        m_assetPackageManager = AssetPackageManager.getInstance(getContext());

        setEGLContextClientVersion(2);//Fix for JB. 130108. SYLee

        setZOrderOnTop(true);
        this.getHolder().setFormat(PixelFormat.RGBA_8888);

        final NexEditorDeviceProfile devprof = NexEditorDeviceProfile.getDeviceProfile();

        setEGLConfigChooser(new EGLConfigChooser() {
            public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {

                final int EGL_OPENGL_ES2_BIT = 4;
                final int[] attrib_list_a =
                        {
                                EGL10.EGL_RENDERABLE_TYPE,       	EGL_OPENGL_ES2_BIT,
                                EGL10.EGL_RED_SIZE,               	8,
                                EGL10.EGL_GREEN_SIZE,             	8,
                                EGL10.EGL_BLUE_SIZE,              	8,
                                EGL10.EGL_ALPHA_SIZE,             	8,
                                EGL10.EGL_STENCIL_SIZE,           	1,
                                EGL10.EGL_SAMPLE_BUFFERS,			devprof.getGLMultisample()?1:0,
                                EGL10.EGL_SAMPLES,					devprof.getGLMultisample()?2:0,
                                EGL10.EGL_DEPTH_SIZE,             	devprof.getGLDepthBufferBits(),
                                EGL10.EGL_NONE
                        };
                final int[] attrib_list_b =
                        {
                                EGL10.EGL_RENDERABLE_TYPE,       	EGL_OPENGL_ES2_BIT,
                                EGL10.EGL_RED_SIZE,               	8,
                                EGL10.EGL_GREEN_SIZE,             	8,
                                EGL10.EGL_BLUE_SIZE,              	8,
                                EGL10.EGL_ALPHA_SIZE,             	8,
                                EGL10.EGL_STENCIL_SIZE,           	8,
                                EGL10.EGL_NONE

                        };

                int[] attrib_list = attrib_list_a;

                int[] num_config = new int[1];
                egl.eglChooseConfig(display, attrib_list, null, 0, num_config);
                if( num_config[0]<1 ) {
                    attrib_list = attrib_list_b;
                    egl.eglChooseConfig(display, attrib_list, null, 0, num_config);
                }
                EGLConfig[] configs = new EGLConfig[num_config[0]];
                egl.eglChooseConfig(display, attrib_list, configs, configs.length, num_config);

                return configs[0];
            }
        });
        
        /* Set the renderer responsible for frame rendering */
        Renderer render = new Renderer();
        setRenderer(render);
        setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        if(LL.D) Log.d(TAG, "GL View Created " + render);

        m_startTime = miliTime();

    }

    public void suspendRendering() {
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void resumeRendering() {
        setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }

    public int getRenderingMode(){
        return getRenderMode();
    }

    private static final long FRAME_RATE = 30;
    private static final long FRAME_TIME = 1000/FRAME_RATE;
    private float mColor = 0;
    private class Renderer implements GLSurfaceView.Renderer
    {
        public void onDrawFrame(GL10 gl) {

            m_frameEnd = miliTime();

            if( m_frameStart==0 ) {
                m_frameStart = m_frameEnd;
            }

            long elapsed = m_frameEnd - m_frameStart;

//			Log.d("EffectPreviewView","onDrawFrame m_frameEnd=" + m_frameEnd + " m_frameStart=" + m_frameStart + " elapsed=" + elapsed);

            long sleepTime = FRAME_TIME-elapsed-5;
            if( sleepTime > 5 ) {
                try {
                    Thread.sleep(sleepTime);
                } catch (InterruptedException e) {
                    if(LL.D) Log.d("NexThemePreviewView","Preview sleep INTERRUPTED");
                }
            }
            m_frameStart += FRAME_TIME;

            mColor += 0.005f;
            if( mColor > 1.0f )
                mColor = 0.0f;
            gl.glClearColor(1.0f, 0.2f, mColor, 1.0f);
            gl.glClear(GL10.GL_DEPTH_BUFFER_BIT | GL10.GL_COLOR_BUFFER_BIT);

            //gl.glEnable(GL10.GL_MULTISAMPLE);

            boolean swap;
//			float progress;
            elapsed = -1;

            if( m_effectTime == 0 ) {
                swap = false;
//				progress = ((float)m_absTime)/60.0f;
            } else {
                long cycleTime = m_pauseTime + m_effectTime;
                elapsed = (miliTime() - m_startTime) % (cycleTime*2);
                elapsed -= elapsed%33;
                swap = ((elapsed>cycleTime)&&!m_isClipEffect);
//				progress = (float)Math.min(elapsed%cycleTime,m_effectTime)/(float)m_effectTime;
            }

            boolean didSwap = false;
            if( swap != m_swapPlaceholders ) {
                didSwap = true;
                m_swapPlaceholders = swap;
                if( m_swapPlaceholders )
                    m_nexThemeRenderer.setPlaceholders("placeholder2.jpg", "placeholder1.jpg");
                else
                    m_nexThemeRenderer.setPlaceholders("placeholder1.jpg", "placeholder2.jpg");
            }

            synchronized(m_renderLock) {
                if( m_nexThemeRenderer != null ) {
                    boolean bNeedLoadEffect = false;
                    boolean bNeedSetEffect = false;
                    if( m_effectOptions != m_setEffectOptions ) {
                        m_setEffectOptions = m_effectOptions;
                        bNeedSetEffect = true;
                    }
                    if( m_effect!=null && m_effect!=m_setEffect ) {
                        m_setEffect = m_effect;
                        bNeedLoadEffect = true;
                    }
                    if( m_setEffectTime != m_effectTime ) {
                        m_setEffectTime = m_effectTime;
                        bNeedSetEffect = true;
                    }
//					Log.d("EffectPreviewView","bNeedSetEffect=" + bNeedSetEffect + " m_effectTime=" + m_effectTime);

                    if( m_assetPackageManager !=null && (bNeedSetEffect || bNeedLoadEffect)) {
                        m_nexThemeRenderer.clearClipEffect();
                        m_nexThemeRenderer.clearTransitionEffect();
                        if( bNeedLoadEffect ) {
                            String effectData = null;
                            try {
                                if( m_isRenderItem){
                                    effectData = m_assetPackageManager.renderItemDataForEffects(Collections.singletonList(m_effect.getId()) );
                                }else {
                                    effectData = m_assetPackageManager.themeDataForEffects(Collections.singletonList(m_effect.getId()), false);
                                }

                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                            //Log.d("EffectPreviewView"," effectData="+effectData);
                            Log.d("EffectPreviewView"," m_isRenderItem="+m_isRenderItem+",m_isClipEffect="+m_isClipEffect);
                            if( effectData!=null ) {
                                if( m_isRenderItem){
                                    m_nexThemeRenderer.clearRenderItems();
                                    m_nexThemeRenderer.loadRenderItem(m_effect.getId(), effectData);
                                }else {
                                    m_nexThemeRenderer.loadThemeData(effectData);
                                }
                            }
                        }
                        if( m_effect!=null ) {
                            if( m_isClipEffect ) {
                                m_nexThemeRenderer.setClipEffect(m_effect.getId(), m_effectOptions, 1, 3, 0, (int)m_effectTime+100, 50, (int)m_effectTime+50);
//								Log.d("EffectPreviewView","setClipEffect : " + 0 + "," + ((int)m_effectTime+100) + "," + 50 + "," + ((int)m_effectTime-50) );
                            } else {
                                m_nexThemeRenderer.setTransitionEffect(m_effect.getId(), m_effectOptions, 1, 3, 0, (int)m_effectTime);
//								Log.d("EffectPreviewView","setTransitionEffect : " + 0 + ", " + m_effectTime);
                            }
                        }
                    }
//					Log.d("EffectPreviewView","setCTS:" + ((int)Math.min(elapsed%(m_pauseTime + m_effectTime),m_effectTime-1)+(m_isClipEffect?50:0)));
                    m_nexThemeRenderer.setCTS((int)Math.min(elapsed%(m_pauseTime + m_effectTime),m_effectTime-1)+(m_isClipEffect?50:0));
                    m_nexThemeRenderer.render();
//					if( m_isClipEffect && elapsed>=0 ) {
//						m_nexThemeRenderer.renderAtTime((int)Math.min(elapsed%(m_pauseTime + m_effectTime),m_effectTime), (int)m_effectTime);
//					} else {
//						m_nexThemeRenderer.render(progress);
//					}
                }
            }
            if( m_showOnRender ) {
                m_showOnRender = false;
                m_handler.post(new Runnable() {
                    public void run() {
                        setVisibility(View.VISIBLE);
                    }
                });
            }

            if( didSwap ) {
                long load_time = (miliTime()-m_frameEnd);
//				Log.d("EffectPreviewView","Swap Placeholders; load_time=" + load_time);
                m_startTime += load_time;
                m_frameStart += load_time;
            }

        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            if(LL.D) Log.d(TAG, "onSurfaceChanged");
            gl.glViewport(0, 0, width, height);
            if( m_nexThemeRenderer != null )
                m_nexThemeRenderer.surfaceChange(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            if(LL.D) Log.d(TAG, "onSurfaceCreated");
        }
    }

}