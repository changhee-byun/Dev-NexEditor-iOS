package com.nexstreaming.nexeditorsdkapis.qatest;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaMetadataRetriever;
import android.os.Bundle;
import android.os.Debug;
import android.os.Environment;
import android.util.Log;
import android.view.WindowManager;

import com.nexstreaming.nexeditorsdk.exception.ExpiredTimeException;
import com.nexstreaming.nexeditorsdk.nexApplicationConfig;
import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexEngine;
import com.nexstreaming.nexeditorsdk.nexEngineListener;
import com.nexstreaming.nexeditorsdk.nexEngineView;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.nexeditorsdk.nexTemplateManager;
import com.nexstreaming.nexeditorsdkapis.ApiDemosConfig;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.common.Stopwatch;

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

//import android.support.v7.app.ActionBarActivity;

public class AutoTestActivity extends Activity {
    private final String LOG_TAG = "AutoTestActivity";

    private nexEngineView mView;
    private nexEngine mEngine;

    private ImageCompare imageCompare = new ImageCompare();
    private CompareValue compareValue = new CompareValue();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_auto_test);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        final Intent intent = getIntent();
        if( intent.getExtras() == null )
            return;

        nexApplicationConfig.waitForLoading(this, new Runnable() {
            @Override
            public void run() {
            }
        });

        new Thread(new Runnable() {
            public void run() {
                String test_action = intent.getStringExtra("test_action");
                String[] actions = test_action.split(",");
                final String test_root = intent.getStringExtra("test_root");
                final String result_root = intent.getStringExtra("result_root");

                // createFlagFile(Environment.getExternalStorageDirectory().getAbsoluteFile() + path);
                String test_path;
                String result_path;
                for( String action : actions ) {

                    switch (action) {
                        case "thumbnail":
                            test_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + test_root + "thumbnail/";
                            result_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + result_root + "thumbnail/";

                            Log.e(LOG_TAG, "Thumbnail auto test with : " + test_path);
                            test_thumbnail(test_path, result_path);
                            break;
                        case "direct":
                            test_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + test_root + "direct/";

                            Log.e(LOG_TAG, "Direct auto test with : " + test_path);
                            // test_direct(test_path);
                            break;
                        case "template":
                            test_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + test_root + "template/";
                            result_path = Environment.getExternalStorageDirectory().getAbsoluteFile() + result_root + "template/";

                            Log.e(LOG_TAG, "Template auto test with : " + test_path);
                            test_template(test_path, result_path);
                            break;
                    }
                }
                deleteFlagFile(Environment.getExternalStorageDirectory().getAbsoluteFile() + test_root);
            }
        }).start();
    }

    void test_thumbnail(final String test_root, final String result_root) {

        final StringBuilder sb = new StringBuilder();
        final BlockingQueue<String> blockingQueue = new ArrayBlockingQueue<String>(1);
        final Stopwatch elapsed = new Stopwatch();

        File dir = new File(test_root);
        if( dir.isDirectory() == false ) {
            Log.e(LOG_TAG, "Thumbnail auto test failed with wrong test path : " + test_root);
            return;
        }

        File dir_result = new File(result_root);
        if( dir_result.exists() == false ) {
            dir_result.mkdirs();
        }

        sb.append("Thumbnail auto test start\n");
        sb.append("\n-----------------------------------------------------------------\n");

        for(File f : dir.listFiles() ) {
            if( f.isDirectory() && f.getName().equals("content")) {
                Log.e(LOG_TAG, "Thumbnail auto test start");

                ApiDemosConfig.getApplicationInstance().initApp();
                mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
                // mEngine.setView(mView);
                mEngine.set360VideoForceNormalView();

                sb.append( checkMemoryStatus());
                sb.append("\n-----------------------------------------------------------------\n");

                for(File tf : f.listFiles() ) {
                    if( tf.isDirectory() ) continue;

                    final String testFile = tf.getName();
                    nexClip clip = nexClip.getSupportedClip(tf.getAbsolutePath());
                    if( clip == null ) {
                        sb.append("Test clip: " + tf.getAbsolutePath() + " do not support\n");
                        sb.append("\n-----------------------------------------------------------------\n");
                        continue;
                    }
                    sb.append("Test clip: " + tf.getAbsolutePath() + " is checking\n");

                    elapsed.reset();
                    elapsed.start();

                    int width, height, startTime, endTime, count, angle;
                    boolean noCache = true;

                    width = 320;
                    height = 240;
                    startTime = 0;
                    endTime = clip.getTotalTime();
                    count = 10;
                    angle = 0;

                    clip.getVideoClipDetailThumbnails(width, height, startTime, endTime, count, angle, noCache, null, new nexClip.OnGetVideoClipDetailThumbnailsListener() {

                        @Override
                        public void onGetDetailThumbnailResult(int event, Bitmap bm, int index, int totalCount, int timestamp) {
                            if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {

                                elapsed.stop();
                                String result_path = result_root + testFile + "_" + timestamp + "_" + index + "_" + totalCount + ".png";
                                String expect_path = test_root + "expect/" + testFile + "_" + timestamp + "_" + index + "_" + totalCount + ".png";

                                String checked = "(not checked)";
                                Bitmap expect = BitmapFactory.decodeFile(expect_path);

                                if( expect != null ) {
                                    compareValue.reset();
                                    imageCompare.compareBitmapWithPSNR(bm, expect, compareValue);
                                    if( compareValue.diffPercent < 5f) {
                                        checked = "(pass)";
                                    }
                                    else {
                                        checked = "(not pass)";
                                        savebitmap(bm, result_path);
                                    }
                                    expect = null;
                                }
                                else {
                                    savebitmap(bm, result_path);
                                }

                                sb.append("Thumbnail saved" + checked +" : " + result_path + " diffPercent(" +compareValue.diffPercent+") aveDiff(" + compareValue.aveDiff + ")");
                                sb.append(" mse(" +compareValue.mse+") snr(" + compareValue.snr + ")");
                                sb.append(" psnr(" + compareValue.psnr + "/" + compareValue.max_psnr + ")\n");

                                elapsed.start();
                                elapsed.start();

                            } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Completed) {
                                try {
                                    blockingQueue.put("Complated!");
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Fail) {
                                try {
                                    blockingQueue.put("fail!");
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_systemError) {
                                try {
                                    blockingQueue.put("system error!");
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            }
                        }
                    });

                    try {
                        String result = (String)blockingQueue.take();
                        elapsed.stop();
                        sb.append("Test clip checking End with " + result + " (Elapsed time : " + elapsed.toString() + ")\n");
                        sb.append("\n-----------------------------------------------------------------\n");
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }

                    int [] seekPointTable = clip.getSeekPointsSync();

                    if( seekPointTable != null ) {

                        int [] requestTable = new int[seekPointTable.length];

                        int gap = seekPointTable.length / 15;
                        gap = gap == 0 ? 1 : gap;

                        int dstIdx = 0;
                        for(int i = 0; i < 15 && i < seekPointTable.length; i += gap ) {
                            requestTable[dstIdx] = seekPointTable[i];
                            dstIdx++;
                        }

                        elapsed.reset();
                        elapsed.start();

                        clip.getVideoClipDetailThumbnails(width, height, 0, 0, dstIdx, angle, noCache, requestTable, new nexClip.OnGetVideoClipDetailThumbnailsListener() {

                            @Override
                            public void onGetDetailThumbnailResult(int event, Bitmap bm, int index, int totalCount, int timestamp) {
                                if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Ok) {

                                    elapsed.stop();
                                    String result_path = result_root + testFile + "_seektable_" + timestamp + "_" + index + "_" + totalCount + ".png";
                                    String expect_path = test_root + "expect/" + testFile + "_seektable_" + timestamp + "_" + index + "_" + totalCount + ".png";

                                    String checked = "(not checked)";
                                    Bitmap expect = BitmapFactory.decodeFile(expect_path);
                                    if( expect != null ) {
                                        compareValue.reset();
                                        imageCompare.compareBitmap(bm, expect, compareValue);
                                        if( compareValue.diffPercent < 5f) {
                                            checked = "(pass)";
                                        }
                                        else {
                                            checked = "(not pass)";
                                            savebitmap(bm, result_path);
                                        }
                                        expect = null;
                                    }
                                    else {
                                        savebitmap(bm, result_path);
                                    }

                                    sb.append("Thumbnail saved" + checked +" : " + result_path + " diffPercent(" +compareValue.diffPercent+") aveDiff(" + compareValue.aveDiff+ ")\n");
                                    elapsed.start();
                                } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Completed) {
                                    try {
                                        blockingQueue.put("Complated!");
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }
                                } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_Fail) {
                                    try {
                                        blockingQueue.put("fail!");
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }
                                } else if (event == nexClip.OnGetVideoClipDetailThumbnailsListener.kEvent_systemError) {
                                    try {
                                        blockingQueue.put("system error!");
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }
                                }
                            }
                        });
                        try {
                            String result = (String)blockingQueue.take();
                            elapsed.stop();
                            sb.append("Test clip checking(Seek table) End with " + result + " (Elapsed time : " + elapsed.toString() + ")\n");
                            sb.append("\n-----------------------------------------------------------------\n");
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                }

                ApiDemosConfig.getApplicationInstance().releaseAPP();

                System.gc();
                sb.append( checkMemoryStatus());
                sb.append("\n-----------------------------------------------------------------\n");

                String thumb_test_path = result_root + "/Thumbnail_test_log.txt";
                savelog(sb.toString(), thumb_test_path);
                mEngine = null;
            }
        }
    }

    void test_direct(String path) {

        final StringBuilder sb = new StringBuilder();
        final BlockingQueue<String> blockingQueue = new ArrayBlockingQueue<String>(1);
        final Stopwatch elapsed = new Stopwatch();

        final String test_path = path;
        File dir = new File(test_path);
        if( dir.isDirectory() == false ) {
            Log.e(LOG_TAG, "Direct auto test failed with wrong test path : " + path);
            return;
        }

        final String test_path_result = test_path + File.separator + "result";
        File dir_result = new File(test_path_result);
        if( dir_result.exists() == false ) {
            dir_result.mkdirs();
        }

        sb.append("Direct auto test start\n");
        sb.append("\n-----------------------------------------------------------------\n");

        for(File f : dir.listFiles() ) {
            if( f.isDirectory() && f.getName().equals("content")) {
                Log.e(LOG_TAG, "Direct auto test start");

                ApiDemosConfig.getApplicationInstance().initApp();
                mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
                // mEngine.setView(mView);
                mEngine.set360VideoForceNormalView();

                sb.append( checkMemoryStatus());
                sb.append("\n-----------------------------------------------------------------\n");

                for(File tf : f.listFiles() ) {
                    if( tf.isDirectory() ) continue;

                    final String testFile = tf.getName();
                    nexClip clip = nexClip.getSupportedClip(tf.getAbsolutePath());
                    if( clip == null ) {
                        sb.append("Test clip: " + tf.getAbsolutePath() + " do not support\n");
                        sb.append("\n-----------------------------------------------------------------\n");
                        continue;
                    }
                    sb.append("Test clip: " + tf.getAbsolutePath() + " is testing\n");

                    elapsed.reset();
                    elapsed.start();



                    try {
                        String result = (String)blockingQueue.take();
                        elapsed.stop();
                        sb.append("Test clip checking End with " + result + " (Elapsed time : " + elapsed.toString() + ")\n");
                        sb.append("\n-----------------------------------------------------------------\n");
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                ApiDemosConfig.getApplicationInstance().releaseAPP();

                System.gc();
                sb.append( checkMemoryStatus());
                sb.append("\n-----------------------------------------------------------------\n");

                String thumb_test_path = test_path + "/result/Thumbnail_test_log.txt";
                savelog(sb.toString(), thumb_test_path);
                mEngine = null;
            }
        }
    }

    void test_template(final String test_path, final String result_path) {

        final StringBuilder sb = new StringBuilder();
        final BlockingQueue<String> blockingQueue = new ArrayBlockingQueue<String>(1);
        final Stopwatch elapsed = new Stopwatch();

        File dir = new File(test_path);
        if( dir.isDirectory() == false ) {
            Log.e(LOG_TAG, "Template auto test failed with wrong test path : " + test_path);
            return;
        }

        File dir_result = new File(result_path);
        if( dir_result.exists() == false ) {
            dir_result.mkdirs();
        }

        sb.append("Template auto test start\n");
        sb.append("\n-----------------------------------------------------------------\n");

        ApiDemosConfig.getApplicationInstance().initApp();
        mEngine = ApiDemosConfig.getApplicationInstance().getEngine();
        // mEngine.setView(mView);
        mEngine.set360VideoForceNormalView();
        mEngine.setEventHandler(new nexEngineListener() {
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
                sb.append("Template export done with " + b +"\n");

                if(b) {
                    try {
                        blockingQueue.put("Export error with " + i);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                } else {
                    try {
                        blockingQueue.put("Export complate!");
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
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
            public void onPreviewPeakMeter(int i, int i1, int i2) {

            }
        });

        nexTemplateManager templateManager = nexTemplateManager.getTemplateManager(getApplicationContext(), getApplicationContext());
        if( templateManager == null ) {
            sb.append("Template test failed because template manager can't create\n");
            sb.append("\n-----------------------------------------------------------------\n");
            nexApplicationConfig.releaseApp();
            return;
        }

        if( templateManager.getTemplates() == null ) {
            sb.append("Template test failed because template count is 0\n");
            sb.append("\n-----------------------------------------------------------------\n");
            nexApplicationConfig.releaseApp();
            return;
        }

        List<nexTemplateManager.Template> templates = new ArrayList<>();

        templateManager.loadTemplate();
        for (nexTemplateManager.Template template : templateManager.getTemplates()) {
            if (template.aspect() == 16 / 9.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_16v9) {
                templates.add(template);
            } else if (template.aspect() == 9 / 16.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_9v16) {
                templates.add(template);
            } else if (template.aspect() == 1.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v1) {
                templates.add(template);
            } else if (template.aspect() == 2.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_2v1) {
                templates.add(template);
            } else if (template.aspect() == 1 / 2.0f && nexApplicationConfig.getAspectRatioMode() == nexApplicationConfig.kAspectRatio_Mode_1v2) {
                templates.add(template);
            }

            Log.d(LOG_TAG, String.format("Template Activity load templates(%s) (%f)", template.name("en"), template.aspect()));
        }

        if( templates.size() <= 0 ) {
            sb.append("Template test failed because no available template\n");
            sb.append("\n-----------------------------------------------------------------\n");
            nexApplicationConfig.releaseApp();
            return;
        }

        for(File f : dir.listFiles() ) {
            if( f.isDirectory() == false || f.getName().equals("content") == false ) continue;

            Log.e(LOG_TAG, "Template auto test start :" + f.getName());

            sb.append( checkMemoryStatus());
            sb.append("\n-----------------------------------------------------------------\n");

            for(File tf : f.listFiles() ) {
                if( tf.getName().startsWith("project") && tf.isDirectory() ) {
                    nexProject project = new nexProject();;

                    for( File sourceFile : tf.listFiles() ) {
                        if( sourceFile.isDirectory() ) continue;

                        nexClip clip = nexClip.getSupportedClip(sourceFile.getAbsolutePath());
                        if( clip != null ) {
                            updateRotate(clip);
                            project.add(clip);
                        }
                    }

                    if( project.getTotalClipCount(true) <= 0 ) {
                        sb.append("Template test failed with " + tf.getName() + " test set\n");
                        sb.append("\n-----------------------------------------------------------------\n");
                        continue;
                    }

                    for(nexTemplateManager.Template template : templates ) {

                        Stopwatch watch = new Stopwatch();
                        watch.start();

                        nexProject cloneProject = nexProject.clone(project);

                        try {
                            if (templateManager.applyTemplateToProjectById(cloneProject, template.id()) == false) {
                                sb.append("Template apply failed with " + template.name("en") + "\n");
                                sb.append("\n-----------------------------------------------------------------\n");
                                continue;
                            }
                        } catch (ExpiredTimeException e) {
                            e.printStackTrace();
                            sb.append("Template apply failed with " + template.name("en") + "\n");
                            sb.append("\n-----------------------------------------------------------------\n");
                            continue;
                        }

                        mEngine.setProject(cloneProject);
                        watch.stop();

                        sb.append("Template apply time : " + watch.toString() + ")\n");

                        String exportPath = result_path + template.name("en") + "_1280x720.mp4";

                        elapsed.reset();
                        elapsed.start();

                        int iRet = mEngine.exportNoException(exportPath, 1280, 720, 4 * 1024 * 1024, Long.MAX_VALUE, 0, 44100, 0x01, 0x800, 3000, nexEngine.ExportCodec_AVC);
                        if (iRet == 0) {
                        } else if (iRet == -1) {
                            sb.append("Template already export with " + template.name("en") + "\n");
                            try {
                                blockingQueue.put("Already export!");
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        } else if (iRet == -2) {
                            sb.append("Template export failed with broken effect " + template.name("en") + "\n");
                            try {
                                blockingQueue.put("Export error with broken effect!");
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        } else {
                            sb.append("Template export error " + template.name("en") + "\n");
                            try {
                                blockingQueue.put("Export error!");
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }

                        try {
                            String result = (String) blockingQueue.take();
                            elapsed.stop();
                            sb.append(template.name("en") + "export test end with " + result + " (Elapsed time : " + elapsed.toString() + ")\n");
                            sb.append("\n-----------------------------------------------------------------\n");
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                }
            }
        }

        ApiDemosConfig.getApplicationInstance().releaseAPP();

        System.gc();
        sb.append( checkMemoryStatus());
        sb.append("\n-----------------------------------------------------------------\n");

        String test_log = result_path + "template_test_log.txt";
        savelog(sb.toString(), test_log);
        mEngine = null;
    }

    private Bitmap openbitmap(String filename) {
        Bitmap b = BitmapFactory.decodeFile(filename);
        return b;
    }


    private File savebitmap(Bitmap bitmap, String filename) {
        OutputStream outStream = null;

        File file = new File(filename);
        if (file.exists()) {
            file.delete();
            file = new File(filename);
        }
        try {
            // make a new bitmap from your file
            outStream = new FileOutputStream(file);
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, outStream);
            outStream.flush();
            outStream.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        // Log.e("file", "" + file);
        return file;

    }

    private File savelog(String log, String filename) {
        File file = new File(filename);
        if (file.exists()) {
            file.delete();
            file = new File(filename);
        }
        try {
            // make a new bitmap from your file

            OutputStreamWriter fos = new OutputStreamWriter(new FileOutputStream(file));
            fos.write(log);
            fos.flush();
            fos.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        // Log.e("file", "" + file);
        return file;

    }

    private String checkMemoryStatus() {
        StringBuilder sb = new StringBuilder();

        long totalMemory = Runtime.getRuntime().totalMemory();
        long useMemory = Runtime.getRuntime().totalMemory() - Runtime.getRuntime().freeMemory();
        long maxMemory = Runtime.getRuntime().maxMemory();
        long nativeMemory = Debug.getNativeHeapAllocatedSize();

        sb.append("VM Total Memory \t: " + totalMemory + "\n");
        sb.append("VM Use Memory \t: " + useMemory + "\n");
        sb.append("VM Max Memory \t: " + maxMemory + "\n");
        sb.append("Native Memory \t: " + nativeMemory + "\n");

        return sb.toString();
    }

    private void createFlagFile(String path) {
        File file = new File(path, "now_testing");
        if (file.exists()) {
            file.delete();
            file = new File(path, "now_testing");
        }
        try {
            // make a new bitmap from your file

            OutputStreamWriter fos = new OutputStreamWriter(new FileOutputStream(file));
            fos.write("now_testing");
            fos.flush();
            fos.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        file = null;
    }

    private void deleteFlagFile(String path) {
        File file = new File(path, "now_testing");
        if (file.exists()) {
            file.delete();
        }
        file = null;
    }

    private void updateRotate(nexClip clip) {
        if( clip.getClipType()== nexClip.kCLIP_TYPE_VIDEO ) {
            try {
                int rotate = clip.getRotateDegree();
                MediaMetadataRetriever retriever = new MediaMetadataRetriever();
                retriever.setDataSource(clip.getPath());
                String rotation = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION);

                if (rotation != null) {
                    rotate = Integer.parseInt(rotation);
                }

                clip.setRotateDegree(360 - rotate);

            } catch (RuntimeException e) {
            }
        }
    }

    public class CompareValue {
        float aveDiff = 0f;
        float diffPercent = 0f;

        double mse = 0f;
        double snr = 0f;
        double max_psnr = 0f;
        double psnr = 0f;

        void reset() {
            aveDiff = 0f;
            diffPercent = 0f;
            mse = 0f;
            snr = 0f;
            max_psnr = 0f;
            psnr = 0f;
        }
    }

    public class ImageCompare {
        public double log10(double x) {
            return Math.log(x)/Math.log(10);
        }

        private byte[] loadBitmapByteArray(Bitmap bitmap) {
            int bytes = bitmap.getByteCount();
            ByteBuffer buffer = ByteBuffer.allocate(bytes);
            bitmap.copyPixelsToBuffer(buffer);
            byte[] array = buffer.array();
            return array;
        }

        public void compareBitmap(Bitmap bitmap1, Bitmap bitmap2, CompareValue result) {
            if (bitmap1.getWidth() != bitmap2.getWidth() || bitmap1.getHeight() != bitmap2.getHeight()) {
                throw new RuntimeException("images were of diffrent size");
            }

            byte[] first = loadBitmapByteArray(bitmap1);
            byte[] second = loadBitmapByteArray(bitmap2);
            int loopCount = first.length;
            int diffCount = 0;
            long diffSum = 0;
            for (int i = 0; i < loopCount; i++) {
                int v1 = 0xFF & first[i];
                int v2 = 0xFF & second[i];
                int error = Math.abs(v1 - v2);
                if (error > 0) {
                    diffCount++;
                    diffSum += error;
                }
            }
            result.diffPercent = ((float) diffCount) / first.length;
            result.aveDiff = ((float) diffSum) / first.length;
        }

        public void compareBitmapWithPSNR(Bitmap bitmap1, Bitmap bitmap2, CompareValue result) {
            if (bitmap1.getWidth() != bitmap2.getWidth() || bitmap1.getHeight() != bitmap2.getHeight()) {
                throw new RuntimeException("images were of diffrent size");
            }

            double  peak, signal, noise, mse;

            byte[] first = loadBitmapByteArray(bitmap1);
            byte[] second = loadBitmapByteArray(bitmap2);
            int width = bitmap1.getWidth();
            int height = bitmap1.getHeight();

            signal = noise = peak = 0;
            for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                    int idx = i*width+j;
                    signal += first[idx] * first[idx];
                    noise += (first[idx] - second[idx]) * (first[idx] - second[idx]);
                    if (peak < first[idx])
                        peak = first[idx];
                }
            }

            result.mse = noise/(width*height);
            result.snr = 10*log10(signal/noise);
            result.max_psnr = (10*log10(255*255/result.mse));
            result.psnr = 10*log10((peak*peak)/result.mse);
        }
    }
}
