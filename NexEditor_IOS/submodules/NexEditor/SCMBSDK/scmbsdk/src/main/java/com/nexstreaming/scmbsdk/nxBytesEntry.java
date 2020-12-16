package com.nexstreaming.scmbsdk;

import android.graphics.Bitmap;

/**
 * Created by jeongwook.yoon on 2018-08-09.
 */

class nxBytesEntry {
    protected byte[] mData;

    protected nxBytesEntry(){

    }

    public nxBytesEntry(byte[] data){
        mData = data;
    }

    byte[] getData(){
        return mData;
    }

    void setData(byte[] data){
        mData = data;
    }
}
