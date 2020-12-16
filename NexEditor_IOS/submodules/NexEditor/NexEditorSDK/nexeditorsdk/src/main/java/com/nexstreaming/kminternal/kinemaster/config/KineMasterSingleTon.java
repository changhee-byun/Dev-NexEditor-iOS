package com.nexstreaming.kminternal.kinemaster.config;

import android.content.Context;
import android.util.Log;


import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;
import com.nexstreaming.kminternal.nexvideoeditor.NexImageLoader;

import java.io.File;

/**
 * Created by jeongwook.yoon on 2015-02-06.
 */
public class KineMasterSingleTon {
    private static final String LOG_TAG = "KineMasterSingleTon";
    private static KineMasterSingleTon instance;
    Context mContext;
    public static KineMasterSingleTon getApplicationInstance() {
        if( instance==null ) {
            if( LL.E ) Log.e(LOG_TAG,"getApplicationInstance : Returning NULL!");
        }
        return instance;
    }


    public Context getApplicationContext(){
        return mContext;
    }

    public KineMasterSingleTon(Context context) {
        mContext = context;

        instance = this;
    }

    private void createEditor( ) {

        if( m_editor!=null )
            return;

        try {

            if( LL.D ) Log.d(LOG_TAG, "Editor Instance Created");
            NexImageLoader.OverlayPathResolver overlayResolver = new NexImageLoader.OverlayPathResolver() {
                @Override
                public String resolveOverlayPath(String overlayPath) {
                    return new File(EditorGlobal.getOverlaysDirectory(),overlayPath).getAbsolutePath();
                }
            };

            NexEditorDeviceProfile dprof = NexEditorDeviceProfile.getDeviceProfile();
            int[] props = {
                    NexEditor.PROP_DEPTH_BUFFER_BITS,	dprof.getGLDepthBufferBits(),
                    NexEditor.PROP_MULTISAMPLE,			dprof.getGLMultisample()?1:0,
                    NexEditor.PROP_ENGINE_LOG_LEVEL, dprof.getNativeLogLevel(),
                    0};
            m_editor = new NexEditor(mContext, null, EditorGlobal.getEffectiveModel(), EditorGlobal.getUserData(), overlayResolver, props);

//            m_editor.setTheme(EditorGlobal.DEFAULT_THEME_ID,false);

            m_editor.createProject();

        } catch (UnsatisfiedLinkError e) {
            if( LL.E ) Log.e(LOG_TAG, "UnsatisfiedLinkError!!!");
            m_linkException = e;
        } catch (NexEditor.EditorInitException e) {
            if( LL.E ) Log.e(LOG_TAG, "EditorInitException!!!");
            m_initException = e;
        }

    }

    private  NexEditor m_editor;
    private   NexEditor.EditorInitException m_initException;
    private   UnsatisfiedLinkError m_linkException;

    public   UnsatisfiedLinkError getLinkException() {
        return m_linkException;
    }

    public   NexEditor.EditorInitException getInitException() {
        return m_initException;
    }

    public   NexEditor getEditorNoAutoCreate() {
        return m_editor;
    }
    private Object getEditorLock = new Object();
    public  NexEditor getEditor() {
        synchronized (getEditorLock) {

            if (m_editor == null) {
                if (LL.D) Log.d(LOG_TAG, "getEditor : creating editor instance");
                createEditor();
            }

            if (m_editor == null) {
                if (LL.E) Log.e(LOG_TAG, "getEditor : editor instance is null");
            }
            return m_editor;
        }
    }

    public void releaseEditor(){
        if( m_editor != null ){
            if( LL.D ) Log.d(LOG_TAG,"releaseEditor : release editor instance");
            m_editor.closeProject();
            m_editor.destroy();
            m_editor = null;
        }
    }
}
