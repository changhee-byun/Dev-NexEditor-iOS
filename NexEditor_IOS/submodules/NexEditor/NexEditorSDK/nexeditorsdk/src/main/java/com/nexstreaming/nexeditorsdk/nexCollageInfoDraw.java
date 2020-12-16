/******************************************************************************
 * File Name        : nexCollage.java
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

/**
 * @brief This interface provides an easy way to set or get property of collage draw for clip.
 * @since 2.0.0
 */
public interface nexCollageInfoDraw {
    final public static int kRotate_doing = 0;
    final public static int kRotate_start = 1;
    final public static int kRotate_end = 2;
    /**
     * This method get tag id of draw info.
     * @return Tag id.
     * @since 2.0.0
     */
    public int getTagID();

    /**
     * This method move position of clip.
     * @param xMov x value for moving.
     * @param yMov y value for moving.
     * @param screenWidth Width of SDK engine view.
     * @param screenHeight Height of SDK engine view.
     * @return True is success or not.
     * @since 2.0.0
     */
    public boolean setTranslate(int xMov, int yMov, int screenWidth, int screenHeight);

    /**
     * This method get current rotate of clip.
     * @return Rotate value of clip.
     * @since 2.0.0
     */
    public int getRotate();

    /**
     * This method set rotate of clip.
     * @param state Rotate state flag, 1: start rotate, 2: end rotate, 0: rotating state.
     * @param angle Rotate value.
     * @return True is success or not.
     * @since 2.0.0
     */
    public boolean setRotate(int state, int angle);

    /**
     * This method set flip of clip.
     * @param direction Flip value, 0: flip clear, 1: horizontal flip, 2: vertical flip.
     * @return True is success or not.
     * @since 2.0.0
     */
    public boolean setFlip(int direction);

    /**
     * This method set scale of clip.
     * @param scale Scale value.
     * @return True is success or not.
     * @since 2.0.0
     */
    public boolean setScale(float scale);

    /**
     * This method get current setted lut ID.
     * @return lut ID String or null.
     * @since 2.0.0
     */
    public String getLut();

    /**
     * This method change lut of draw information.
     * @param lut Id of new lut.
     *            Clear lut if set the "none" or null.
     *            Set collage default lut if set the "default".
     * @return True is success or not.
     * @since 2.0.0
     */
    public boolean setLut(String lut);

    /**
     * This method get visible status.
     * @return True is visible or not.
     * @since 2.0.0
     */
    public boolean getVisible();

    /**
     * This method set visible status.
     * @param visible Visible or not.
     * @return True is success or not.
     * @since 2.0.0
     */
    public boolean setVisible(boolean visible);

    /**
     * This method change source of draw information.
     * @param clip Instan ce of new clip.
     * @return True is success or not.
     * @since 2.0.0
     */
    public boolean changeSource(nexClip clip);

    /**
     * This method get instance of nexClip with linked current drawinfo.
     * @return nexClip instance.
     * @since 2.0.0
     */
    public nexClip getBindSource();

    /**
     * This method set audio volume of linked nexClip if video clip.
     * @param volume new volume, volume ranges are 0 to 200
     * @return true if linked source is video or false.
     * @since 2.0.0
     */
    public boolean setAudioVolume(int volume);

}
