package com.nexstreaming.nexeditorsdk.exception;


/**
 * Created by jeongwook.yoon on 2018-03-12.
 */

public class ExpiredTimeException extends Exception {
    private static final long serialVersionUID = 1L;

    public ExpiredTimeException() {
        super("Asset has expired.");
    }

    public ExpiredTimeException(String assetid) {
        super(assetid+" has expired.");
    }
}
