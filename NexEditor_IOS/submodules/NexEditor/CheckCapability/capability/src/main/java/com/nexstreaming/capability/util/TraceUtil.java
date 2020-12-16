package com.nexstreaming.capability.util;

import android.annotation.TargetApi;
import android.os.Trace;

/**
 * Created by Eric on 2015-11-09.
 */
public class TraceUtil {

    private TraceUtil() {
    }

    static final boolean TRACE_ENABLED = true;

    /**
     * Writes a trace message to indicate that a given section of code has begun.
     *
     * @see android.os.Trace#beginSection(String)
     */
    public static void beginSection(String sectionName) {
        if (TRACE_ENABLED && Util.SDK_INIT >= 18) {
            beginSectionV18(sectionName);
        }
    }

    /**
     * Writes a trace message to indicate that a given section of code has ended.
     *
     * @see android.os.Trace#endSection()
     */
    public static void endSection() {
        if (TRACE_ENABLED && Util.SDK_INIT >= 18) {
            endSectionV18();
        }
    }

    @TargetApi(18)
    private static void beginSectionV18(String sectionName) {
        Trace.beginSection(sectionName);
    }

    @TargetApi(18)
    private static void endSectionV18() {
        Trace.endSection();
    }
}
