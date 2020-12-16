/******************************************************************************
 * File Name        : nexOverlayInfoTitle.java
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

import android.graphics.RectF;

/**
 * @brief This interface provides an easy way to set property of overlay title.
 * @since 2.0.15
 */
public interface nexOverlayInfoTitle {

    /**
     * This method get rectangle of overlay title info.
     * @return Rectangle of overlay title.
     * @since 2.0.15
     */
    public RectF getRect();


    /**
     * This method get title of overlay title info.
     * @param locale This parameter use to get default title.
     * @return String of current title info. This function will return last inputted title, if you set the title otherwise it will be return default title.
     * @since 2.0.15
     */
    public String getTitle(String locale);

    /**
     * This method set title of overlay title info.
     * @param text New title.
     * @return True is success or not.
     * @since 2.0.15
     */
    public boolean setTitle(String text);

    /**
     * This method get font id of current title info.
     * @return Returns the last set font id.
     * @since 2.0.15
     */
    public String getTitleFont();

    /**
     * This method set title font.
     * This method do not support yet
     * @param id Id of font.
     * @return True is success or not.
     * @since 2.0.15
     */
    public boolean setTitleFont(String id);

    /**
     * This method get title fill color of current title info.
     * @return Returns the last set title fill color.
     * @since 2.0.15
     */
    public int getTitleFillColor();

    /**
     * This method set title color.
     * This method do not support yet
     * @param color color(argb) of title.
     * @return True is success or not.
     * @since 2.0.15
     */
    public boolean setTitleFillColor(int color);

    /**
     * This method get title stroke color of current title info.
     * @return Returns the last set title stroke color.
     * @since 2.0.15
     */
    public int getTitleStrokeColor();

    /**
     * This method set stroke color of title.
     * This method do not support yet
     * @param color color(argb) of stroke.
     * @return True is success or not.
     * @since 2.0.15
     */
    public boolean setTitleStrokeColor(int color);

    /**
     * This method get title drop shadow color of current title info.
     * @return Returns the last set title drop shadow color.
     * @since 2.0.15
     */
    public int getTitleDropShadowColor();

    /**
     * This method set drop shadow color of title.
     * This method do not support yet
     * @param color Color(argb) of drop shadow.
     * @return True is success or not.
     * @since 2.0.15
     */
    public boolean setTitleDropShadowColor(int color);

    /**
     * This method get max title input length.
     * @return Max length of current title info.
     * @since 2.0.15
     */
    public int getTitleMaxLength();

    /**
     * This method get to recommend title input length.
     * @return Recommended length of current title info.
     * @since 2.0.15
     */
    public int getTitleRecommendLength();

    /**
     * This method get max line of input title.
     * @return Max line count.
     * @since 2.0.15
     */
    public int getTitleMaxLines();
}
