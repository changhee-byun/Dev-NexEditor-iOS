package com.nexstreaming.nexeditorsdkapis.qatest;

import android.util.Log;

import com.nexstreaming.nexeditorsdkapis.qatest.tc.ExportTest;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

/**
 * Created by jeongwook.yoon on 2016-05-03.
 */
public abstract class TestCase {
    private final String TAG = "TestCase";
    private String subTag="noTag";
    private FileOutputStream reportFileStream;

    public enum State{
        Ready,
        Run,
        Complete,
        Error,
        Skip,
    };

    private State state = State.Ready;
    private int errorCount = 0;
    private int noMoreTestCount = 0;

    private static BlockingQueue<Integer> queue = new ArrayBlockingQueue<Integer>(3);

    private boolean bUseTestBlocking = false;

    public class TestBlocking extends Thread {

        private BlockingQueue<Integer> queue;

        public TestBlocking(BlockingQueue queue) {
            this.queue = queue;
        }

        @Override
        public void run() {
            try {
                while(true) {
                    preTest();
                    Integer i = queue.take();
                    if (!postTest(i)) {
                        break;
                    }
                }
                setFinish();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    public void callVideoView( VideoViewTestCase vtc){
        try {
            if( TestCaseManager.callVideoView != null ) {
                TestCaseManager.callVideoView.onPlay(vtc);
                queue.take();
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void callEditorPlay( VideoViewTestCase vtc){
        try {
            if( TestCaseManager.callEditorPlay != null ) {
                TestCaseManager.callEditorPlay.onPlay(vtc);
                queue.take();
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }


    String openLogFile(int index){
        File dir = new File(QAConfig.sdcardReportPath());
        dir.mkdirs();
        File logFile = new File(QAConfig.sdcardReportPath()+"["+index+"]"+subTag);
        try {
            reportFileStream = new FileOutputStream(logFile);
            return logFile.getAbsolutePath();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return null;
    }

    void closeLogFile(){
        if( reportFileStream != null ) {
            try {
                reportFileStream.close();
                reportFileStream = null;
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void writeLogFile(byte[] file_content){
        if( reportFileStream != null ) {
            try {
                reportFileStream.write(file_content);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public void unLock(int lockNum){
        try {
            queue.put(lockNum);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void printLog(String message){
        if( subTag == null ){
            subTag="noTag";
        }
        Log.d(TAG, "[" + subTag + "]" + message);
        String log = message+'\n';
        writeLogFile(log.getBytes());
    }

    final void prepareRun(){
        bUseTestBlocking = false;
        state = State.Run;
        errorCount = 0;
        noMoreTestCount = 0;
    }

    protected boolean setUp(){
        return true;
    }

    protected void end(){
        if( !bUseTestBlocking ) {
            setFinish();
        }
    }

    abstract protected void preTest();

    abstract protected boolean postTest(int lockNum);

    abstract protected void cancel();

    void setState(State s){
        state = s;
    }

    State getState(){
        return state;
    }

    public int getErrorCount(){
        return  errorCount;
    }

    protected void runTest(){
        bUseTestBlocking = true;
        TestBlocking test = new TestBlocking(queue);
        test.start();
    }

    public final void setSubTag(String Tag){
        subTag = Tag;
    }

    public final void setFinish(){
        if( noMoreTestCount > 0 ){
            state = State.Error;
        }else{
            state = State.Complete;
        }
        try {
            TestCaseManager.sbqueue.put(this);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public final boolean isSubTag(){
        if( subTag == null ){
            return false;
        }
        if( subTag.compareTo("noTag")==0 ){
            return false;
        }
        return true;
    }


    public final void resultOk(String message){
        printLog("OK:"+message);
    }

    public final void resultNOk(String message){
        printLog("NOK:"+message);
        errorCount++;
    }

    public final void resultNotEquals(String message,int expected, int actual ){
        printLog("NotEquals:"+message+", value expected="+expected+", value actual="+actual);
        errorCount++;
    }

    public final void resultNoMoreTest(String message){
        printLog("NoMoreTest:"+message);
        noMoreTestCount++;
    }

    public final void resultSkipTest(String message){
        printLog("SkipTest:"+message);
        state = State.Skip;
    }

}
