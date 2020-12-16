/******************************************************************************
 * File Name        : nexObserver.java
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


class nexObserver {

    protected boolean mClipTimeUpdated = true;
    protected boolean mNeedLoadList = true;
    protected int mEffectLoad2Engine; //0 - no loaded, 1 - play loaded, 2 - export loaded.

    nexObserver(){

    }

    void updateTimeLine(boolean onlyLoadlist){
        mNeedLoadList = true; //nexEngine.updateProject()
        if( !onlyLoadlist) {
            mClipTimeUpdated = true;//nexProject.updateProject()
        }
    }

    boolean setEffectLoad( int mode  ) {
        boolean rval;

        if (mEffectLoad2Engine == mode) {
            return false;
        }
        mEffectLoad2Engine = mode;
        return true;
    }
}
