/******************************************************************************
 * File Name        : ClipIsNotVideoException.java
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
 *
 */
public class ClipIsNotVideoException extends nexSDKException {
    private static final long serialVersionUID = 1L;

    public ClipIsNotVideoException() {
        super("This clip is not a video clip.");
    }

}
