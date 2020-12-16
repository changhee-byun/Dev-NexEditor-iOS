package com.nexstreaming.nexeditorsdk.module;

import android.content.Context;
import android.graphics.RectF;

/**
 *
 * @since 2.0.0
 */
public interface nexFaceDetectionMethod {
    public boolean init(String path, Context context);
    public boolean detect(RectF rect);
    public boolean deinit();

}
