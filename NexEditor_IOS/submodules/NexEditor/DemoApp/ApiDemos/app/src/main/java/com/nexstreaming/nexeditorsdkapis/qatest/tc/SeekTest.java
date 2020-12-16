package com.nexstreaming.nexeditorsdkapis.qatest.tc;

import android.widget.VideoView;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.qatest.QAConfig;
import com.nexstreaming.nexeditorsdkapis.qatest.TestCase;
import com.nexstreaming.nexeditorsdkapis.qatest.VideoViewTestCase;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

/**
 * Created by jeongwook.yoon on 2016-05-09.
 */
public class SeekTest extends TestCase {
    private final String video1 = "video1.mp4";
    private int totalTime;
    private nexEngine engine;
    private nexEngineListener mEditorListener;
    private int inputSeekTime;
    private int lastSeekTimeDone;
    private boolean mStop = false;

    public class EditorPlayTest extends VideoViewTestCase {
        private final int seekTime;
        public EditorPlayTest(TestCase tc, String message, int lockNum, int seekTime ){
            super(tc,QAConfig.getContentPath(video1),message,lockNum);
            this.seekTime =seekTime;
        }

        @Override
        public boolean Nok() {
            getTestCase().resultNOk("EditorPlayTest(" + getMessage() + ")");
            getTestCase().unLock(getLockNum());
            return true;
        }

        @Override
        public void Run(VideoView vv) {
            engine.updateProject();
            engine.seek(seekTime);
        }
    }

    @Override
    protected boolean setUp(){
        engine = QAConfig.getEngin();

        nexClip clip1 = nexClip.getSupportedClip(QAConfig.getContentPath(video1));
        if( clip1 == null ){
            resultNoMoreTest(video1+" file not found or not supported file");
            engine = null;
            return false;
        }

        engine.setProject(new nexProject());
        mEditorListener = new nexEngineListener() {
            @Override
            public void onStateChange(int i, int i1) {

            }

            @Override
            public void onTimeChange(int i) {

            }

            @Override
            public void onSetTimeDone(int i) {
                lastSeekTimeDone = i;
                //unLock(1);
            }

            @Override
            public void onSetTimeFail(int i) {

            }

            @Override
            public void onSetTimeIgnored() {

            }

            @Override
            public void onEncodingDone(boolean b, int i) {

            }

            @Override
            public void onPlayEnd() {

            }

            @Override
            public void onPlayFail(int i, int i1) {

            }

            @Override
            public void onPlayStart() {

            }

            @Override
            public void onClipInfoDone() {

            }

            @Override
            public void onSeekStateChanged(boolean b) {

            }

            @Override
            public void onEncodingProgress(int i) {

            }

            @Override
            public void onCheckDirectExport(int i) {

            }

            @Override
            public void onProgressThumbnailCaching(int i, int i1) {

            }

            @Override
            public void onFastPreviewStartDone(int i, int i1, int i2) {

            }

            @Override
            public void onFastPreviewStopDone(int i) {

            }

            @Override
            public void onFastPreviewTimeDone(int i) {

            }

            @Override
            public void onPreviewPeakMeter(int iCts, int iLeftValue, int iRightValue) {

            }
        };
        engine.setEventHandler(mEditorListener);
        engine.getProject().add(clip1);

        return true;
    }

    @Override
    protected void preTest() {
        //inputSeekTime = 3000;
        //engine.seek(inputSeekTime);
        unLock(1);
    }

    @Override
    protected boolean postTest(int lockNum) {
        /*
        if(mStop){
            resultNOk("test cancel!");
            return false;
        }

        if( inputSeekTime != lastSeekTimeDone ){
            resultNotEquals("seel time invalid!",inputSeekTime,lastSeekTimeDone);
        }
        resultOk("test complete!");
        return false;
        */
        callEditorPlay(new EditorPlayTest(this,"seek 3sec",1,3000));
        return false;
    }

    @Override
    protected void cancel() {
        mStop = true;
    }
}
