package com.nexstreaming.nexeditorsdkapis.modules;

import android.content.Context;
import android.graphics.RectF;

import com.nexstreaming.nexeditorsdk.module.UserField;
import com.nexstreaming.nexeditorsdk.module.nexFaceDetectionProvider;
import com.vivo.szfacedetection.SZFaceDetector;

/**
 * Created by jeongwook.yoon on 2017-11-20.
 */

public class FaceDetectorSZ implements nexFaceDetectionProvider {
    private static SZFaceDetector SzFD = null;
    private static Thread init_thread = null;

    public static boolean createInstance(final Context context) {

        if( SzFD == null ) {
            SzFD = new SZFaceDetector();
        }

        init_thread = new Thread(new Runnable(){

            //@Override
            public void run(){
                SzFD.init(context);
            }
        });
        init_thread.start();

        return true;
    }

    @Override
    public String name() {
        return "FaceDetectorSz";
    }

    @Override
    public String uuid() {
        return "b3fa3b66-419c-4053-95c2-5087a013495e";
    }

    @Override
    public String description() {
        return "Sz Dace Detector";
    }

    @Override
    public String auth() {
        return "vivo";
    }

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
        if( SzFD == null ) {
            SzFD = new SZFaceDetector();
        }

        if(init_thread != null){

            try{
                init_thread.join();
            }
            catch(Exception e){
                e.printStackTrace();
            }

            init_thread = null;
        }

        return SzFD.init(path, context);
    }

    @Override
    public boolean detect(RectF rect) {
        boolean success = SzFD.detect(rect);
        if (!success) {
            return false;
        }
        return true;
    }

    @Override
    public boolean deinit() {
        if(SzFD == null)
            return false;
        return SzFD.deinit();
    }
}
