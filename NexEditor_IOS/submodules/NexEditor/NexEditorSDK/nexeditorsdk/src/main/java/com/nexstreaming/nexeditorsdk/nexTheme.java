/******************************************************************************
 * File Name        : nexTheme.java
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
import android.graphics.Bitmap;
//import android.support.v7.internal.view.menu.BaseMenuPresenter;


/**
 *  This class contains effects, for basic video editing, sorted in themes.
 * <p>Example code: </p>
 * <b>class sample code</b>
 * {@code nexEffectLibrary fxlib = nexEffectLibrary.getEffectLibrary(this);
    nexTheme[] allThemes = fxlib.getThemes();
    for( nexTheme t: allThemes ) {
        Log.i(LOG_TAG,"Name =" + t.getName(getBaseContext()));
    }}
 * @since version 1.0.0
 */
@Deprecated
public final class nexTheme {
    private final static String none ="none";

    nexTheme() {

    }

    /**
     *  This method gets the ID of a theme.
     *
     * <p>Example code :</p>
     * {@code id.setText(mAllThemes[index].getId());}
     * @return Theme ID as a <tt> String</tt>. 
     * @since version 1.0.0
     */
    @Deprecated
    public String getId() {
        return none;
    }

    /**
     * This method gets the name of a theme.
     *
     * <p>Example code :</p>
     * {@code name.setText(mAllThemes[index].getName(mContext));}
     * @param context The Android context.
     * @return A theme name as a <tt>String</tt>.
     * @since version 1.0.0
     */
    @Deprecated
    public String getName(Context context) {
        return none;
    }

    /**
     * This method gets the description of a theme.
     *
     * <p>Example code :</p>
     * {@code desc.setText(mAllThemes[index].getDesc(getApplicationContext()));}
     * @param context The Android context.
     * @return Description of the theme as a <tt>string</tt>.
     * @since version 1.0.0
     */
    @Deprecated
    public String getDesc(Context context) {
        return none;
    }
/*
    public Bitmap getIconSync(){
        return mTheme.getThumbnailSync();
    }
*/
    /**
     *  This method gets the icon of a theme, as a bitmap.
     *
     * <p>Example code :</p>
     * {@code if( mAllThemes[index].getIconSyncEx() != null ){
            iv.setVisibility(View.VISIBLE);
            iv.setImageBitmap(mAllThemes[index].getIconSyncEx());
        }}
     * @return Bitmap of the theme icon.
     * @since version 1.0.0
     */
    @Deprecated
    public Bitmap getIconSyncEx(){
        return null;
    }

    /**
     * This method gets the transition effect ID of a theme.  
     *
     * <p>Example code :</p>
     * {@code trid.setText(mAllThemes[index].getTransitionId());}
     * @return The ID of the transition effect. 
     * @since version 1.3.3
     */
    @Deprecated
    public String getTransitionId() {
        return none;
    }

    /**
     * This method gets the opening effect ID of a theme. 
     *
     * The effect ID retrieved with this method can be used to make a separate clip effect. 
     *
     * <p>Example code :</p>
     * {@code opentitle.setText(mAllThemes[index].getOpeningTitleEffectId());}
     * @return The ID of an opening effect. 
     * @since version 1.3.3
     */
    @Deprecated
    public String getOpeningTitleEffectId() {
        return none;
    }

    /**
     * This method gets the ending effect ID of a theme. 
     *
     * The effect ID retrieved with this method can be used to make a separate clip effect. 
     *
     * <p>Example code :</p>
     * {@code endtitle.setText(mAllThemes[index].getEndingTitleEffectId());}
     * @return The ID of an ending effect. 
     * @since version 1.3.3
     */
    @Deprecated
    public String getEndingTitleEffectId() {
        return none;
    }

    /**
     * This method gets the middle effect ID of a theme. 
     * Middle effect allows to set titles to the clip. 
     * The effect ID retrieved with this method can be used to make a separate clip effect. 
     *
     * <p>Example code :</p>
     * {@code middletitle.setText(mAllThemes[index].getMiddleTitleEffectId());}
     * @return The ID of a title effect. 
     * @since version 1.3.3
     */
    @Deprecated
    public String getMiddleTitleEffectId() {

        return none;
    }

    /**
     * This method gets the accent effect ID of a theme. 
     * The effect ID retrieved with this method can be used to make a separate clip effect. 
     *
     * <p>Example code :</p>
     * {@code accenteffectid.setText(mAllThemes[index].getAccentEffectId());}
     * @return The ID of an accent effect.
     * @since version 1.3.3
     */
    @Deprecated
    public String getAccentEffectId() {
        return none;
    }


    /**
     * This method checks whether an ID is a valid theme ID.
     *
     * <p>Example code :</p>
     * {@code if( nexTheme.isValidThemeId(ThemeId) ) {
            mThemeId = ThemeId;
        }}
     * @param Id The theme ID to check, as a <tt> String</tt>. 
     * @return <tt>TRUE</tt> if the theme ID is usable for <tt>nexEffectLibrary</tt>, otherwise <tt>FALSE</tt>.
     * @since version 1.0.0
     */
    @Deprecated
    public static boolean isValidThemeId( String Id ) {
        return false;
    }
}
