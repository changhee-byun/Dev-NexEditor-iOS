package com.nexstreaming.nexeditorsdkapis.qatest;

import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Modifier;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

/**
 * Created by jeongwook.yoon on 2016-05-03.
 */
public class TestCaseManager extends Thread{
    static BlockingQueue<TestCase> sbqueue = new ArrayBlockingQueue<TestCase>(3);;
    private static OnStateChangeListener stateChanger;
    private int currentPosition;
    private int selectTestIndex = -1;
    private static final Class sTestCaseList[] = TestCaseList.sTestCaseList;
    public static class TestCaseResult{
        public String mName;
        public TestCase.State mState;
        public int mErrorCount;
        public String mLogFilePath;
    }

    private boolean bStop = false;
    private boolean bRunning = false;
    private TestCase currentTestCase;

    static OnVideoViewPlayListener callVideoView;
    static OnEditorPlayListener callEditorPlay;

    private static TestCaseResult report[];

    public static abstract class OnStateChangeListener {
        public abstract void onStateChange( int index , TestCaseResult result );
        public abstract void onComplete( );
    }

    public static abstract class OnVideoViewPlayListener {
        public abstract void onPlay( final VideoViewTestCase tc );
    }

    public static abstract class OnEditorPlayListener {
        public abstract void onPlay( final VideoViewTestCase tc );
    }

    public TestCaseManager(){

    }

    public static final int getTotalCount(){
        return sTestCaseList.length;
    }

    public boolean setSingleTest(int index){
        if( sTestCaseList.length >=  index ){
            return false;
        }
        selectTestIndex = index;
        return true;
    }

    public static String[] getTestNames(){
        String str[] = new String[sTestCaseList.length];
        int i = 0;
        for( Class test : sTestCaseList ){
            str[i] = test.getName().substring(test.getName().lastIndexOf('.')+1);
            i++;
        }
        return str;
    }

    public static TestCaseResult[] getReport(){
        if(report == null ) {
            report = new TestCaseResult[sTestCaseList.length];
            for (int i = 0; i < sTestCaseList.length; i++) {
                report[i] = new TestCaseResult();
                report[i].mName = sTestCaseList[i].getName().substring(sTestCaseList[i].getName().lastIndexOf('.') + 1);
                report[i].mState = TestCase.State.Ready;
                report[i].mErrorCount = 0;
            }
        }
        return report;
    }

    public static void setStateChangeListener(final OnStateChangeListener listener){
        stateChanger = listener;
    }

    public int getCurrentPosition(){
        return currentPosition;
    }

    public TestCaseResult getCurrentReport(){
        return report[currentPosition];
    }

    static public byte[] getReportLog(int index){
        String path = report[index].mLogFilePath;
        if( path != null){
            File log = new File(path);
            try {
                FileInputStream fis = new FileInputStream(log);
                int readcount = (int)log.length();
                byte[] buffer = new byte[readcount];
                fis.read(buffer);
                fis.close();
                return buffer;
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    public void run(int index){
        Class test = sTestCaseList[index];
        currentPosition = index;
        Constructor constructor = null;

        try {
            constructor = test.getConstructor();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }

        if( constructor != null ){
            try {
                TestCase testinst = (TestCase)constructor.newInstance();

                if( !testinst.isSubTag() ){
                    testinst.setSubTag(test.getName().substring(test.getName().lastIndexOf('.')+1));
                }

                if( bStop ){
                    return;
                }

                currentTestCase = testinst;

                report[currentPosition].mLogFilePath = testinst.openLogFile(currentPosition);
                testinst.prepareRun();
                report[currentPosition].mState = testinst.getState();

                if( stateChanger != null )
                    stateChanger.onStateChange(currentPosition, report[currentPosition] );
                if( testinst.setUp() ) {
                    testinst.runTest();
                }else{

                }

                testinst.end();

                try {
                    TestCase waitTest = (TestCase)sbqueue.take();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                testinst.closeLogFile();
                report[currentPosition].mState = testinst.getState();
                report[currentPosition].mErrorCount = testinst.getErrorCount();
                if (stateChanger != null)
                    stateChanger.onStateChange(currentPosition, report[currentPosition] );

            } catch (InstantiationException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void run(){
        bRunning = true;
        if( selectTestIndex == -1 ) {

            currentPosition = 0;
            for (int i = 0; i < sTestCaseList.length; i++) {
                run(i);
                if( bStop )
                    break;
            }

            if (stateChanger != null)
                stateChanger.onComplete();
        }else{
            run(selectTestIndex);
            selectTestIndex = -1;
        }
        bRunning = false;
    }

    static public void setOnVideoViewPlayListener(OnVideoViewPlayListener listener){
        callVideoView = listener;
    }

    static public void setOnEditorPlayListener(OnEditorPlayListener listener){
        callEditorPlay = listener;
    }


    public boolean isRunning(){
        return bRunning;
    }

    public void cancel(){
        bStop = true;
        if( currentTestCase != null)
            currentTestCase.cancel();
    }
}
