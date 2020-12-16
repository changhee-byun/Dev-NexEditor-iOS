package com.nexstreaming.nexeditorsdkapis.modules;

import android.content.Context;
import android.graphics.RectF;
import android.support.annotation.NonNull;

import com.nexstreaming.gfwfacedetection.GFWFaceDetector;
import com.nexstreaming.nexeditorsdk.module.UserField;
import com.nexstreaming.nexeditorsdk.module.nexFaceDetectionProvider;

/**
 * Created by jeongwook.yoon on 2017-11-20.
 */

public class FaceDetecorGWF implements nexFaceDetectionProvider {
    private GFWFaceDetector GWFFD = null;

    @NonNull
    @Override
    public String name() {
        return "GFWFaceDetector";
    }

    @NonNull
    @Override
    public String uuid() {
        return "31717431-8d11-41e5-9dc1-aaaaafda0dca";
    }

    @NonNull
    @Override
    public String description() {
        return "GFW FaceDetector";
    }

    @NonNull
    @Override
    public String auth() {
        return "google";
    }

    @NonNull
    @Override
    public String format() {
        return "fd";
    }

    @Override
    public int version() {
        return 1;
    }

    @Override
    public UserField[] userFields() {
        return new UserField[0];
    }

    @Override
    public boolean init(String path, Context context) {
        if( GWFFD == null )
            GWFFD = new GFWFaceDetector();

        return GWFFD.init(path, context);
    }

    @Override
    public boolean detect(RectF rect) {
        boolean success = GWFFD.detect(rect);
        if (!success) {
            return false;
        }
        return true;
    }

    @Override
    public boolean deinit() {
        if(GWFFD == null)
            return false;
        return GWFFD.deinit();
    }
}
