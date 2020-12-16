package com.nexstreaming.scmbsdk;

import android.util.Log;


/**
 * SDK error code define.
 * @see nxSCMBSDK#getLastErrorCode()
 */
public enum ErrorCode {
    /**
     * Error is None. OK.
     */
    None,

    /**
     * Argument was not valid. check null.
     */
    INVALID_Argument,

    /**
     * Using full handle. Max handle count is 16.
     */
    MAX_Handle,

    /**
     * handle was lost.
     */
    INVALID_Handle,

    /**
     * sdk instance was create fail
     */
    FailOfCreate,

    /**
     * sdk int fail
     */
    FailOfInit,

    /**
     * sdk write entry fail.
     */
    FailOfUpdate,

    /**
     * common error or not define error.
     */
    NotDefineError;

    static ErrorCode fromValue( int value ) {
        int index = value<0? (value*-1) : value;
        ErrorCode[] list = ErrorCode.values();
        if( index < 0 || index > list.length ){
            return NotDefineError;
        }
        return list[index];
    }
}
