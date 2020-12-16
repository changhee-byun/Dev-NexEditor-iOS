package com.nexstreaming.kminternal.kinemaster.utils.facedetect;

import android.content.Context;
import android.graphics.RectF;
import android.os.AsyncTask;
import android.util.Log;
import android.util.LruCache;

import com.nexstreaming.app.common.task.ResultTask;
import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.nexeditorsdk.module.nexFaceDetectionProvider;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

public class FaceInfo {

    private static final String LOG_TAG = "FaceInfo";
    private static LruCache<String,FaceInfo> faceInfoCache = new LruCache<>(100);
    private static Executor concurrentExecutor = Executors.newFixedThreadPool(4);
    private static Map<String,WeakReference<ResultTask<FaceInfo>>> taskCache = new HashMap<>();

    private float left, top, right, bottom;

    private static Thread faceModule_init_thread = null;
    public static nexFaceDetectionProvider faceModule;
    public void getBounds(RectF bounds) {
        bounds.set(left,top,right,bottom);
    }
    public void setBounds(RectF bounds) {
        left = Math.max(0, bounds.left);
        top = Math.max(0, bounds.top);
        right = Math.min(bounds.right, 1);
        bottom = Math.min(bounds.bottom, 1);
    }

    private void faceDetectProc(File path, boolean usefilter, Context context){
        long time = System.currentTimeMillis();
        if( path==null )
            return;
        if(faceModule != null){

            if(faceModule_init_thread != null){

                try{

                    faceModule_init_thread.join();
                }
                catch(Exception e){

                    e.printStackTrace();
                }

                faceModule_init_thread = null;
            }
            if(faceModule != null){
                 faceModule.init(path.getAbsolutePath(), context);

                 if(faceModule != null){
                    RectF rectF = new RectF();
                     boolean success = faceModule.detect(rectF);
                     if(success) {
                        setBounds(rectF);
                    }
                } else {
                    Log.d(LOG_TAG, "faceDetectProc:skip detect(faceModule is null)");
                }
            } else{
                Log.d(LOG_TAG, "faceDetectProc:skip init(faceModule is null)");
            }
            //faceModule.deinit();
            Log.d(LOG_TAG, "FaceDetector elapsed time:(" + (System.currentTimeMillis()-time) + ")");
        }else{
            Log.d(LOG_TAG, "Can not find FaceDetector module");
        }
    }

    public FaceInfo(File path, boolean usefilter, Context context) {

        faceDetectProc(path, usefilter, context);
    }

    public FaceInfo(File path, int big_flag, Context context) {

        faceDetectProc(path, true, context);
    }

    public static nexFaceDetectionProvider getFaceModule() {
        return faceModule;
    }

    public static void setFaceModule(nexFaceDetectionProvider ifaceModule) {
        if(faceModule != null){
            faceModule.deinit();
        }
        faceModule = ifaceModule;
        FaceInfo.clearFaceInfo();
    }

    public static FaceInfo getFaceInfo( String path ) {
        return faceInfoCache.get(path);
    }

    public static void putFaceInfo(String path, FaceInfo face) {

        faceInfoCache.put(path, face);
    }
    public static boolean clearFaceInfo() {
        Log.d(LOG_TAG, "clearFaceInfo");
        faceInfoCache.evictAll();
        return true;
    }
    public static ResultTask<FaceInfo> getFaceInfoAsync( final String path ) {
        FaceInfo faceInfo = faceInfoCache.get(path);
        if( faceInfo!=null ) {
            return ResultTask.completedResultTask(faceInfo);
        }
        WeakReference<ResultTask<FaceInfo>> existingTaskRef = taskCache.get(path);
        if( existingTaskRef!=null ) {
            ResultTask<FaceInfo> existingTask = existingTaskRef.get();
            if( existingTask.isRunning() ) {
                return existingTask;
            }
        }
        final ResultTask<FaceInfo> resultTask = new ResultTask<>();
        taskCache.put(path,new WeakReference<ResultTask<FaceInfo>>(resultTask));
        new AsyncTask<String,Void,FaceInfo>(){

            Task.TaskError taskError = null;

            @Override
            protected FaceInfo doInBackground(String... params) {
                File file = new File(params[0]);
                //return new FaceInfo(file, true);
                return null;
            }

            @Override
            protected void onPostExecute(FaceInfo faceInfo) {
                taskCache.remove(path);
                if( faceInfo==null ) {
                    resultTask.sendFailure(taskError);
                } else {
                    faceInfoCache.put(path,faceInfo);
                    resultTask.sendResult(faceInfo);
                    resultTask.signalEvent(Task.Event.COMPLETE, Task.Event.SUCCESS);
                }
            }
        }.executeOnExecutor(concurrentExecutor,path);
        return resultTask;
    }

}
