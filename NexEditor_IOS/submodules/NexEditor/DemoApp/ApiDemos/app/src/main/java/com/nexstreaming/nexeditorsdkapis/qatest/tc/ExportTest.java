package com.nexstreaming.nexeditorsdkapis.qatest.tc;

import android.content.Intent;
import android.media.MediaScannerConnection;
import android.os.Environment;
import android.widget.VideoView;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.qatest.QAConfig;
import com.nexstreaming.nexeditorsdkapis.qatest.TestCase;
import com.nexstreaming.nexeditorsdkapis.qatest.VideoViewActivity;
import com.nexstreaming.nexeditorsdkapis.qatest.VideoViewTestCase;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

/**
 * Created by jeongwook.yoon on 2016-05-04.
 */
public class ExportTest extends TestCase {
    private final String Image1 = "image1.png";
    private final String video1 = "video1.mp4";
    private int totalTime;
    private nexEngine engine;
    private nexEngineListener mEditorListener;
    private File tempFile;

    private int exportWidth =1280;
    private int exportHeight =720;

    private boolean mStop = false;

    public ExportTest(){
        setSubTag("ExportTest");
    }

    @Override
    protected boolean setUp(){
        engine = QAConfig.getEngin();

        nexClip clip1 = nexClip.getSupportedClip(QAConfig.getContentPath(Image1));

        if( clip1 == null ){
            resultNoMoreTest(Image1+" file not found or not supported file");
            engine = null;
            return false;
        }

        nexClip clip2 = nexClip.getSupportedClip(QAConfig.getContentPath(video1));
        if( clip2 == null ){
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

            }

            @Override
            public void onSetTimeFail(int i) {

            }

            @Override
            public void onSetTimeIgnored() {

            }

            @Override
            public void onEncodingDone(boolean b, int i) {
                if( b ){
                    resultNOk("export fail! errno is "+i);
                }
                unLock(1);
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
                if( mStop ){
                    engine.stop();
                    unLock(1);
                }
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
        engine.getProject().add(clip2);
        engine.getProject().getClip(0, true).setImageClipDuration(3000);
        engine.getProject().getClip(1, true).getVideoClipEdit().setTrim(0, 3000);

        return true;
    }

    @Override
    protected void preTest() {
        engine.updateProject();
        totalTime = engine.getProject().getTotalTime();
        tempFile = QAConfig.getTempFile("test1.mp4");
        engine.export(tempFile.getAbsolutePath(), exportWidth, exportHeight, engine.getProject().getClip(1, true).getVideoBitrate(), Long.MAX_VALUE, 0);
    }

    @Override
    protected boolean postTest(int lockNum) {
        if( mStop ){
            resultNOk("test cancel!");
            return true;
        }
        nexClip clip = nexClip.getSupportedClip(tempFile.getAbsolutePath(),false);
        int duration = clip.getTotalTime();
        if( totalTime != duration ){
            resultNotEquals("export duration invalid!",totalTime,duration);
        }

        if( clip.getWidth() != exportWidth ){
            resultNotEquals("export width invalid!",exportWidth,clip.getWidth());
        }

        if( clip.getHeight() != exportHeight ){
            resultNotEquals("export height invalid!",exportHeight,clip.getHeight());
        }

        callVideoView(new VideoViewTestCase(this,clip.getPath(),"image 3 sec + view 3 sec",1));

        tempFile.delete();
        resultOk("test complete!");

        return false;
    }

    @Override
    protected void cancel() {
        mStop = true;
    }

}
