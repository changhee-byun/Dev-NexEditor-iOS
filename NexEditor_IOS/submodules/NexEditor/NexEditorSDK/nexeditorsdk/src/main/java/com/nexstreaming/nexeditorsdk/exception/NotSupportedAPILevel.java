/******************************************************************************
 * File Name        : NotSupportedAPILevel.java
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

import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;

/**
 * Created by jeongwook.yoon on 2015-12-10.
 */
public class NotSupportedAPILevel extends nexSDKException  {
    private static final long serialVersionUID = 1L;

    public NotSupportedAPILevel() {
        super("Not Supported API Level("+ EditorGlobal.ApiLevel+")");
    }
}
