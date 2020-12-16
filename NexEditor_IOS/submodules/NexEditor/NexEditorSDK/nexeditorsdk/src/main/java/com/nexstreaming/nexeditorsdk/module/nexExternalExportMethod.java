package com.nexstreaming.nexeditorsdk.module;

import android.content.Context;

import com.nexstreaming.nexeditorsdk.nexExportFormat;


/**
 *
 * @since 2.0.0
 */
public interface nexExternalExportMethod {

    /**
     *
     * @param format
     * @return
     * @since 2.0.0
     */
    public boolean OnPrepare( nexExportFormat format);

    /**
     *
     * @param format
     * @param data
     * @return
     * @since 2.0.0
     */
    public boolean OnPushData(int format, byte[] data  );

    /**
     *
     * @return
     */
    public int OnLastProcess( );

    /**
     *
     * @param error
     * @return
     * @since 2.0.0
     */
    public boolean OnEnd( int error );
}
