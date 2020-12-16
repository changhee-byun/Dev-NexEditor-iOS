/******************************************************************************
 * File Name        : nexEngineView.java
 * Description      :
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

package com.nexstreaming.nexeditorsdk;

import android.content.Context;
import android.util.AttributeSet;

import com.nexstreaming.kminternal.nexvideoeditor.NexThemeView;

/**
 *  This class previews projects if there is any project in <tt>nexEngine</tt>.
 * <p>Example code:</p>
       <b> Layout XML </b>
             {@code <com.nexstreaming.nexeditorsdk.nexEngineView
                 android:id="@+id/videoView"
                 android:layout_width="wrap_content"
                 android:layout_height="match_parent"
                 android:layout_gravity="center"/>}

       <b> Class sample code </b>
            {@code nexEngineView  m_editorView = (nexEngineView)findViewById(R.id.XXXXX);
                m_editorView.setBlackOut(true);
                mEngine.setView(m_editorView); }
 * @since version 1.0.0
 */
public class nexEngineView extends NexThemeView implements NexThemeView.NexThemeViewEventListener{
    private NexViewListener nexThemeViewListener;

    @Override
    public void onEventNotify(int eventType, Object object, int param1, int param2, int param3) {
        if( nexThemeViewListener != null ) {
            switch (eventType) {
                case NexThemeView.kEventType_Available:
                    nexThemeViewListener.onEngineViewAvailable(param1,param2);
                    break;

                case NexThemeView.kEventType_SizeChanged:
                    nexThemeViewListener.onEngineViewSizeChanged(param1,param2);
                    break;
                case NexThemeView.kEventType_Destroyed:
                    nexThemeViewListener.onEngineViewDestroyed();
                    break;
            }
        }
    }

    public interface  NexViewListener{
        void onEngineViewAvailable( int width, int height);
        void onEngineViewSizeChanged( int width, int height);
        void onEngineViewDestroyed();
    }
    /**
     * Constructor for <tt>nexEngineView</tt>.
     * @param context The application context. 
     * @since version 1.1.0
     */
    public nexEngineView(Context context) {
        super(context);
        super.setAspectRatio(nexApplicationConfig.getAspectRatioInScreenMode());
        super.setNotify(this);
    }
 
    /**
     * Constructor for <tt>nexEngineView</tt>.
     * @param context The application context. 
     * @param attrs The set of attributes. 
     * @since version 1.1.0
     */
    public nexEngineView(Context context, AttributeSet attrs) {
        super(context, attrs);
        super.setAspectRatio(nexApplicationConfig.getAspectRatioInScreenMode());
        super.setNotify(this);
    }

    /** 
     * This method black outs the screen to clear out the surface for the next content.
     *
     * When the application is reopened, the screen might still be showing an image from the previous content. 
     * To prevent this, this method should be used before or after playing a content. 
     * 
     * @param blackOut Set to <tt>TRUE</tt> to enable black out, set to <tt>FALSE</tt> to disable black out. 
     * @since version 1.0.0
     * @deprecated For internal use only. Please do not use.
     */
    @Override
    @Deprecated
    public void setBlackOut(boolean blackOut) {
        super.setBlackOut(blackOut);
    }

    /**
     *
     * @param listener
     * @since 1.7.58
     */
    public void setListener(NexViewListener listener){
        nexThemeViewListener = listener;
        if( isSurfaceAvailable() ){
            nexThemeViewListener.onEngineViewAvailable(getWidth(),getHeight());
        }
    }
}
