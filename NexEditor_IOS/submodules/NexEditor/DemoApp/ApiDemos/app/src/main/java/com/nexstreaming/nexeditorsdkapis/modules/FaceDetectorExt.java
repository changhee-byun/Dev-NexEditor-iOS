package com.nexstreaming.nexeditorsdkapis.modules;

import android.content.Context;
import android.graphics.RectF;

import com.nexstreaming.nexeditorsdk.module.UserField;
import com.nexstreaming.nexeditorsdk.module.nexFaceDetectionProvider;
import com.nexstreaming.nexfacedetection.nexFaceDetector;

/**
 * Created by jeongwook.yoon on 2017-11-20.
 */

public class FaceDetectorExt implements nexFaceDetectionProvider {
    private nexFaceDetector nexFD = null;

    @Override
    public String name() {
        return "FaceDetectorExt";
    }

    @Override
    public String uuid() {
        return "6a460d22-cd87-11e7-abc4-cec278b6b50a";
    }

    @Override
    public String description() {
        return "nex face detector";
    }

    @Override
    public String auth() {
        return "Nexstreaming";
    }

    @Override
    public String format() {
        return "fd";
    }

    @Override
    public int version() {
        return 0;
    }

    @Override
    public UserField[] userFields() {
        return new UserField[0];
    }

    @Override
    public boolean init(String path, Context context) {
        if( nexFD == null ) {
            nexFD = new nexFaceDetector();
        }
        nexFD.init(path, context);

        return true;
    }

    @Override
    public boolean detect(RectF rect) {
        nexFD.detect(rect);
        return true;
    }

    @Override
    public boolean deinit() {
        if(nexFD == null)
            return false;
        nexFD.deinit();
        return false;
    }
}
