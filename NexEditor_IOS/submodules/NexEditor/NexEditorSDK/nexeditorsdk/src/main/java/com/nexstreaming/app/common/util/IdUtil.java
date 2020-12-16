package com.nexstreaming.app.common.util;

import android.os.Build;
import android.view.View;

import java.util.concurrent.atomic.AtomicInteger;

public class IdUtil {
    private IdUtil(){} // Prevent instantiation

    private static final AtomicInteger sNextGeneratedId = new AtomicInteger(0x00A00000);

    public static int generateRuntimeResourceId() {
        if(Build.VERSION.SDK_INT > Build.VERSION_CODES.JELLY_BEAN_MR1 ) {
            return View.generateViewId();
        } else {
            return generateViewId();
        }
    }

    private static int generateViewId() {
        for (;;) {
            final int result = sNextGeneratedId.get();
            // aapt-generated IDs have the high byte nonzero; clamp to the range under that.
            int newValue = result + 1;
            if (newValue > 0x00FFFFFF) newValue = 1; // Roll over to 1, not 0.
            if (sNextGeneratedId.compareAndSet(result, newValue)) {
                return result;
            }
        }
    }


}
