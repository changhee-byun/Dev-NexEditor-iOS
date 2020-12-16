package com.nexstreaming.nexeditorsdk.exception;

/**
 * Created by jeongwook.yoon on 2018-05-15.
 */

public class NoSuchObjectException extends Exception {
    private static final long serialVersionUID = 1L;

    public NoSuchObjectException() {
        super("Object not found.");
    }

    public NoSuchObjectException(String assetid) {
        super(assetid+" not found.");
    }
}
