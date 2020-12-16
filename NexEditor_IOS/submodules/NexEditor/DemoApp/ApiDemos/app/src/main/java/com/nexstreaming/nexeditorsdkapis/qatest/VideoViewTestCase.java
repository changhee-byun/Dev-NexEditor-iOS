package com.nexstreaming.nexeditorsdkapis.qatest;

import android.widget.VideoView;

/**
 * Created by jeongwook.yoon on 2016-05-11.
 */
public class VideoViewTestCase {
    private final TestCase tc;
    private final String message;
    private final String contentPath;
    private final int lockNum;
    public VideoViewTestCase(TestCase tc, String contentPath,String message, int lockNum ){
        this.tc = tc;
        this.message = message;
        this.lockNum = lockNum;
        this.contentPath = contentPath;
    }
    public final String getMessage(){
        return message;
    }

    public boolean Ok(){
        tc.unLock(lockNum);
        return true;
    }
    public boolean Nok(){
        tc.resultNOk("VideoView("+message+")");
        tc.unLock(lockNum);
        return true;
    }
    public void Run(VideoView vv){
        vv.setVideoPath(contentPath);
        vv.start();
    }

    protected TestCase getTestCase(){
        return tc;
    }

    protected int getLockNum(){
        return lockNum;
    }

}
