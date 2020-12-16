package com.nexstreaming.nexeditorsdkapis.qatest;

import android.content.Context;
import android.os.Environment;

import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Calendar;

/**
 * Created by jeongwook.yoon on 2016-05-03.
 */
public class QAConfig {
    private  static final String sContentPath = File.separator+"qatest"+File.separator;
    private static Context sContext;
    private static TestCase sPlayTest;

    public static String sdcardRootPath(){
        return Environment.getExternalStorageDirectory().getAbsolutePath()+sContentPath;
    }

    public static String getContentPath(String contentFileName){
        return sdcardRootPath() + contentFileName;
    }

    public static String sdcardTempPath(){
        return sdcardRootPath() + "temp"+File.separator;
    }

    public static String sdcardReportPath(){
        return sdcardRootPath() + "report"+File.separator;
    }

    public static File getTempFile(  String filename ){
        File tempDir = new File(QAConfig.sdcardTempPath());
        tempDir.mkdirs();
        Calendar calendar = Calendar.getInstance();
        java.util.Date date = calendar.getTime();
        String tictime = (new SimpleDateFormat("yyMMdd_HHmmss").format(date));
        File tempFile = new File(tempDir, tictime+filename );
        return tempFile;
    }

    public static nexEngine getEngin(){
        return ApiDemosConfig.getApplicationInstance().getEngine();
    }

    public static Context getContext(){
        return sContext;
    }

    public static void setContext(Context c){
        sContext =c;
    }

    public static void setPlayTestcase(TestCase tc){
        sPlayTest = tc;
    }

    public static TestCase getPlayTestcase(){
        return sPlayTest;
    }

}
