/******************************************************************************
 * File Name        : nexSDKException.java
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
 * Created by jeongwook.yoon on 2015-03-24.
 */
public class nexSDKException extends RuntimeException{
    private static final long serialVersionUID = 1L;

    public nexSDKException() {
    }

    public nexSDKException(String detailMessage) {
        super(detailMessage);
    }

    public nexSDKException(Throwable throwable) {
        super(throwable);
    }

    public nexSDKException(String detailMessage, Throwable throwable) {
        super(detailMessage, throwable);
    }

}
