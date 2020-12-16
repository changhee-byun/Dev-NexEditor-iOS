package com.nexstreaming.nexeditorsdkapis.qatest.tc;

import android.media.MediaMetadataRetriever;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdkapis.qatest.QAConfig;
import com.nexstreaming.nexeditorsdkapis.qatest.TestCase;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.List;


/**
 * Created by jeongwook.yoon on 2016-05-03.
 */
public class ClipInfoTest extends TestCase {
    private final List <String> contentPaths = new ArrayList<>();
    private boolean mStop = false;

    public ClipInfoTest(String subTag, String contentName){

        setSubTag(subTag);
        contentPaths.add(contentName);
    }

    public ClipInfoTest( ){
        File dir = new File(QAConfig.sdcardRootPath()+"clipinfo");
        if( dir.isDirectory() ) {
            String[] files = dir.list();
            for( String file : files) {
                contentPaths.add("clipinfo"+File.separator+file);
            }
        }
    }

    @Override
    protected void preTest() {

    }

    @Override
    protected boolean postTest(int lockNum) {
        return false;
    }

    @Override
    protected void runTest() {

        for( String file : contentPaths ) {
            if( mStop ){
                resultNOk(" test cancel.");
                return;
            }
            String contentPath = QAConfig.getContentPath(file);
            int rotate = 0;
            int width = 0;
            int height = 0;
            int duration = 0;
            try {
                MediaMetadataRetriever retriever = new MediaMetadataRetriever();
                retriever.setDataSource(contentPath);
                String rotation = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION);
                String w = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH);
                String h = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT);
                String d = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
                if (rotation != null) {
                    rotate = Integer.parseInt(rotation);
                }

                if (w != null) {
                    width = Integer.parseInt(w);
                }

                if (h != null) {
                    height = Integer.parseInt(h);
                }

                if (d != null) {
                    duration = Integer.parseInt(d);
                }

            }catch (IllegalArgumentException e){
                resultNoMoreTest(file+" Media Scanner not found!=" + contentPath);
                return;
            }

            nexClip clip = nexClip.getSupportedClip(contentPath,false);
            if (clip == null) {
                resultNoMoreTest(file+" not supported =" + contentPath);
                return;
            }

            int clipRotate = 360 - clip.getRotateInMeta();
            if(clipRotate == 360)
                clipRotate = 0;
            if (rotate != clipRotate) {
                resultNotEquals(file + " rotate invalid!", rotate, clipRotate);
            }

            if (width != clip.getWidth()) {
                resultNotEquals(file+" width invalid!", width, clip.getWidth());
            }

            if (height != clip.getHeight()) {
                resultNotEquals(file+" height invalid!", height, clip.getHeight());
            }
            int dur = clip.getTotalTime();
            if( ( (duration - 100) < dur) && ( (duration + 100) > dur) ){
                ;
            }else{
                dur = clip.getAudioDuration();
                if( ( (duration - 100) < dur) && ( (duration + 100) > dur) ){

                }else {
                    resultNotEquals(file + " duration invalid!", duration, dur);
                }
            }
            resultOk(file+" test complete.");
        }
    }

    @Override
    protected void cancel() {
        mStop = true;
    }

}
