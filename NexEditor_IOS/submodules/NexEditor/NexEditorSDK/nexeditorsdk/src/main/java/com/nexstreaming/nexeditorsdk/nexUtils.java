/******************************************************************************
 * File Name        : nexUtils.java
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

import com.nexstreaming.app.common.util.SimplexNoise;

/**
 * NexEditor&trade;&nbsp; provides methods that are difficult to implement in the application. 
 */
public final class nexUtils {
    /**
     * Use this method to make an animation that moves to random coordinates in {@link nexAnimate}.
     * @param xin The x coordinate.
     * @param yin The y coordinate.
     * @param zin The z coordinate or the time value.
     * @return The value to which the noise has been applied. 
     */
    public static double noise(double xin, double yin, double zin){
        return SimplexNoise.noise(xin,yin,zin);
    }
}
