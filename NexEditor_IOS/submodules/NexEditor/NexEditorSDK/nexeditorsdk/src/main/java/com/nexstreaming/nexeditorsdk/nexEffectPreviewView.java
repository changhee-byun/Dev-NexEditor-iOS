/******************************************************************************
 * File Name        : nexEffectPreviewView.java
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

import com.nexstreaming.app.common.nexasset.preview.AssetPreviewView;

/**
 * This class defines and enables the preview of an effect before it is actually applied to a clip. 
 *     <b>Layout XML</b>
 *     {@code   <com.nexstreaming.nexeditorsdk.nexEffectPreviewView
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:id="@+id/effectPreview"
        android:layout_gravity="center_horizontal" />
      }


      <b>class sample code</b>
      {@code     nexEffectPreviewView previewView = (nexEffectPreviewView)findViewById(R.id.XXXXX);
           previewView.setEffectTime(3000);
           String[] aStrings = new String[2];
           aStrings[0] = "1st Text Track";
           aStrings[1] = "2nd Text Track";
           m_previewView.setEffectOptions( nexEffect.getTitleOptions(effect ID ,aStrings));
           previewView.setEffect( effect ID );
       }
 * @since version 1.0.0
 */
//public class nexEffectPreviewView extends EffectPreviewView {
public class nexEffectPreviewView extends AssetPreviewView {


    nexEffectPreviewView(Context context) {
        super(context);
        super.setAspectRatio(nexApplicationConfig.getAspectRatio());
    }

    public nexEffectPreviewView(Context context, AttributeSet attrs) {
        super(context, attrs);
        super.setAspectRatio(nexApplicationConfig.getAspectRatio());
    }

    /**
     * This method sets the ID of an effect.
     *
     * <p>Example code :</p>
     *      {@code    private nexEffectPreviewView m_previewView;
                m_previewView.setEffect(mSelectedEffectId);
            }
     * @param effect_id The ID of an effect, as a <tt>String</tt>.
     * 
     * @since version 1.0.0
     * @see nexEffectLibrary#getClipEffects()
     * @see nexEffectLibrary#getTransitionEffects()
     */
    @Override
    public void setEffect(String effect_id) {
        super.setEffect(effect_id);
    }

    /**
     * This method sets the duration of an effect.
     *
     * <p>Example code :</p>
     *      {@code    private nexEffectPreviewView m_previewView;
                m_previewView.setEffectTime(3000);
            }
     * @param effectTime The duration of an effect as an <tt>integer</tt>, in <tt>msec</tt> (milliseconds).
     * @since version 1.0.0
     */
    @Override
    public void setEffectTime(int effectTime) {
        super.setEffectTime(effectTime);
    }

    /**
     * This method prints text on <tt>nexEffectPreviewView</tt>, by applying the encoded text.
     * Text can be encoded into the options format with the method {@link com.nexstreaming.nexeditorsdk.nexEffect#getTitleOptions getTitleOptions}(nexEffectOptions).
     * <p>Example code :</p>
     *      {@code String[] aStrings = new String[2];
     *      aStrings[0] = "1st Text Track";
     *      aStrings[1] = "2nd Text Track";
     *      m_previewView.setEffectOptions( nexEffect.getTitleOptions(effect ID ,aStrings));
     *      }
     * @param effectOptions The text encoded into the options format.
     * @since version 1.0.0
     * @see {@link com.nexstreaming.nexeditorsdk.nexEffect#getTitleOptions getTitleOptions}(nexEffectOptions)
    */
    @Override
    public void setEffectOptions(String effectOptions) {
        super.setEffectOptions(effectOptions);
    }
}
