/******************************************************************************
 * File Name        : InvalidRangeException.java
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

package com.nexstreaming.nexeditorsdk.exception;

/**
 * Created by jeongwook.yoon on 2015-05-26.
 */
public class InvalidRangeException extends nexSDKException {
    private static final long serialVersionUID = 1L;

    public InvalidRangeException(int start, int end) {

        super("end is low then start ("+start+" >= "+end+")");
    }

    public InvalidRangeException(int duration) {

        super("duration is zero or minus. ("+duration+")");
    }

    public InvalidRangeException(int min, int max, int current ) {

        super("value is not range. ("+min+" < "+ current+" < "+max+")");
    }

    public InvalidRangeException(int limit, int value, boolean msUinit ) {
        super("value is low then limit. (" + value + " < " + limit + ")");
    }


}
