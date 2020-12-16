package com.nexstreaming.app.common.localprotocol;

import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;

/**
 * Created by jeongwook.yoon on 2016-10-31.
 */
public enum nexProtoErrorCode {
    OK(0),

    InvalidHDR(101),
    InvalidRQ(102),
    InvalidRS(103),
    InvalidSSID(104),
    Unrecognized(800);

    private final int mValue;

    nexProtoErrorCode(int value){
        mValue = value;
    }

    public int getValue() {
        return mValue;
    }

    static nexProtoErrorCode fromValue( int value ) {
        for( nexProtoErrorCode v : nexProtoErrorCode.values() ) {
            if( v.getValue()==value )
                return v;
        }
        return nexProtoErrorCode.Unrecognized;
    }

}
