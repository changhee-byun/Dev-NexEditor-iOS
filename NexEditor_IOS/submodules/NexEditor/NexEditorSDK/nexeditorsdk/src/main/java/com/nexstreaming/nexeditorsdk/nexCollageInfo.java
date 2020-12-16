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

import android.graphics.PointF;
import android.graphics.RectF;

import java.util.List;

/**
 * @brief This interface provides an easy way to get property of collage.
 * @since 2.0.0
 */
public interface nexCollageInfo {
    /**
     * This method gets the id of collage sub information.
     * @return ID.
     * @since 2.0.0
     */
    public String getId();

    /**
     * This method gets the position of collage sub information at edit time.
     * @return Position rectangle.
     *          Each position of rectangle was scaled from ratio of collage.
     *          Range of position is from 0 to 1.
     * @since 2.0.0
     */
    public RectF getRectangle();

    /**
     * This method gets the positions of collage sub information at edit time.
     * @return Position list of polygon.
     *          Each position was scaled from ratio of collage.
     *          Range of position is from 0 to 1.
     * @since 2.0.0
     */
    public List<PointF> getPositions();
}

