package com.nexstreaming.app.common.util;

import android.support.annotation.Nullable;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;

import java.io.Closeable;
import java.io.IOException;

public class CloseUtil {

    private static final String LOG_TAG = "CloseUtil";

    private CloseUtil(){} // Prevent instantiation

    /**
     * Closes a closable object, ignoring any exceptions.
     *
     * This is useful for the 'finally' block managing a resource, because any exceptions thrown
     * in the finally block would overwrite any pending exception, thus making debugging more
     * difficult.
     *
     * For example:
     *
     *      InputStream in = new FileInputStream(...);
     *      try {
     *          // do some work
     *      } finally {
     *          CloseUtils.closeSilently(in);  // Exceptions thrown here won't overwrite exceptions thrown in the 'try' block
     *      }
     *
     * @param closeable   The object to close; can be null, in which case no action is taken.
     */
    public static void closeSilently(@Nullable Closeable closeable) {
        if( closeable!=null ) {
            try {
                closeable.close();
            } catch (IOException e) {
                if( LL.W ) Log.w(LOG_TAG,"Error closing resource",e);
            }
        }
    }
}
