package com.nexstreaming.nexeditorsdkapis.modules;

import android.graphics.Bitmap;
import android.os.Environment;
import android.util.Log;

import com.nexstreaming.anigifsdk.nexAniGifSDK;
import com.nexstreaming.nexeditorsdk.module.UserField;
import com.nexstreaming.nexeditorsdk.module.nexExternalExportProvider;
import com.nexstreaming.nexeditorsdk.nexExportFormat;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class AniGifExport implements nexExternalExportProvider {
    final static private String Tag = "AniGifExport";
    nexAniGifSDK aniGifSDK;
    FileOutputStream output;
    String outPath;
    int width;
    int height;
    int count;
    final static boolean useBufferPool = false;
    final static boolean dumpRGBA = false;
    final static boolean dumpJpeg = false;
    @Override
    public String name() {
        return "nexAniGif";
    }

    @Override
    public String uuid() {
        return "4190aa40-cb48-11e7-abc4-cec278b6b50a";
    }

    @Override
    public String description() {
        return "save format is animation gif";
    }

    @Override
    public String auth() {
        return "NexStreaming";
    }

    @Override
    public String format() {
        return "gif";
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
    public boolean OnPrepare(nexExportFormat format) {

        width = format.getInteger(nexExportFormat.TAG_FORMAT_WIDTH);
        height = format.getInteger(nexExportFormat.TAG_FORMAT_HEIGHT);
        int intervalTime = format.getInteger(nexExportFormat.TAG_FORMAT_INTERVAL_TIME);
        outPath = format.getString(nexExportFormat.TAG_FORMAT_PATH);
        int quality = (100 - format.getInteger(nexExportFormat.TAG_FORMAT_QUALITY))/10;
        Log.d(Tag,"OnPrepare("+outPath+", "+width+", "+height+", "+intervalTime+", "+quality+")");
        Log.d(Tag,"nexAniGifSDK version ="+nexAniGifSDK.versionCode());
        if( !useBufferPool ) {
            try {
                output = new FileOutputStream(outPath);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                return false;
            }
        }
        aniGifSDK = nexAniGifSDK.createInstance();
        aniGifSDK.setWidth(width);
        aniGifSDK.setHeight(height);
        aniGifSDK.setDelayMs(intervalTime);
        aniGifSDK.setCpuCoreCount(Runtime.getRuntime().availableProcessors());
        if( quality == 0 )
            aniGifSDK.setMode(nexAniGifSDK.Mode.Speed);
        else{
            aniGifSDK.setMode(nexAniGifSDK.Mode.Quality);
            aniGifSDK.setSampleFactor(quality);
        }
        aniGifSDK.setup();

        if( useBufferPool ) {
            aniGifSDK.useBufferPool(outPath);
        }
        count = 0;

        return true;
    }

    @Override
    public boolean OnPushData(int format, byte[] data) {
        Log.d(Tag,"OnPushData("+data.length+", "+format+")");
        if( dumpJpeg ) {
            Bitmap bm = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            bm.copyPixelsFromBuffer(ByteBuffer.wrap(data));

            String bmpPath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "KM" + File.separator + "test" + count + ".jpg";
            File tempFile = new File(bmpPath);
            try {
                tempFile.createNewFile();
                FileOutputStream out = new FileOutputStream(tempFile);
                bm.compress(Bitmap.CompressFormat.JPEG, 90, out);
                out.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        if( dumpRGBA ) {
            String dumpPath = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "KM" + File.separator + "test_" + width + "X" + height + "_" + count + ".dump";
            File tempFile = new File(dumpPath);
            try {
                tempFile.createNewFile();
                FileOutputStream out = new FileOutputStream(tempFile);
                out.write(data);
                out.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        count++;

        int size = aniGifSDK.pushRGBAFrame(data);
        if( size == 0 ){
            Log.d(Tag,"OnPushData need next frame");
            return true;
        }else if( size < 0 ){
            return false;
        }

        if( !useBufferPool ) {
            byte[] gifFrame = aniGifSDK.popEncodedFrame();
            if (gifFrame.length == 0) {
                return false;
            }
            try {
                Log.d(Tag, "OnPushData output size=" + gifFrame.length + "[" + gifFrame[0] + "]" + "[" + gifFrame[1] + "]" + "[" + gifFrame[2] + "]" + "[" + gifFrame[3] + "]");
                output.write(gifFrame, 0, gifFrame.length);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return true;
    }

    @Override
    public int OnLastProcess() {
        Log.d(Tag,"OnLastProcess()");
        byte[] gifFrame =aniGifSDK.sendEoS();
        if( !useBufferPool ) {
            try {
                if (gifFrame.length > 0) {
                    output.write(gifFrame, 0, gifFrame.length);
                }
                output.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return 0;
    }

    @Override
    public boolean OnEnd(int error) {
        Log.d(Tag,"OnEnd("+error+")");
        aniGifSDK.destroy();
        aniGifSDK = null;
        output = null;

        if( error != 0 ){
            File f = new File(outPath);
            if( f.isFile() ){
                f.delete();
            }
        }
        return true;
    }
}
