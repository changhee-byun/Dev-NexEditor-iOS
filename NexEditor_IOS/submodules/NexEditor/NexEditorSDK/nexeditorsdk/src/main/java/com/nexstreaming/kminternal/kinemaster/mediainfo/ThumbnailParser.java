package com.nexstreaming.kminternal.kinemaster.mediainfo;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.util.Log;

import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.kminternal.kinemaster.config.LL;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

class ThumbnailParser {

    private static final String LOG_TAG = "ThumbnailParser";

    static Task.TaskError parseFromRawFile(File file, int maxThumbnailCount, ThumbnailCallback thumbnailCallback ){
        int format, width, height;
        long start;
        int sizeEach;
        int thumbCount;

        if( !file.exists() ) {
            return ThumbnailError.RawFileNotFound;
        }

        long thumbFileSize = file.length();

        if( thumbFileSize < 8 ) {
            return ThumbnailError.RawFileTooSmall;
        }

        try {
            RandomAccessFile randomAccessFile = new RandomAccessFile(file, "r");
            start = randomAccessFile.getFilePointer();
            format = randomAccessFile.readInt();
            width = randomAccessFile.readInt();
            height = randomAccessFile.readInt();

            int tempformat = swapEndian(format);
            int tempwidth = swapEndian(width);
            int tempheight = swapEndian(height);

            sizeEach = tempwidth * tempheight * tempformat / 8;
            thumbCount = (int)Math.min(maxThumbnailCount,(thumbFileSize-8)/(sizeEach+4));
            if( thumbCount < 1 ) {
                return ThumbnailError.NoThumbailsFound;
            }

            byte[] buffer = new byte[sizeEach];
            ByteBuffer bufferWrapper = ByteBuffer.wrap(buffer);

            ArrayList<TimeOrder> list = new ArrayList<TimeOrder>();
            int lastTime = 0;
            boolean needSort = false;
            for(int i=0; i<thumbCount; i++){
                int time = randomAccessFile.readInt();
                int temptime = swapEndian(time);

                if( lastTime > temptime ){
                    needSort = true;
                    if(LL.D) Log.d(LOG_TAG,"thumbnail needSort");
                }
                lastTime = temptime;
                long offset = randomAccessFile.getFilePointer();
                TimeOrder order = new TimeOrder(temptime, time, offset);
                list.add(order);
                int size = randomAccessFile.read(buffer);
            }
            randomAccessFile.seek(start);
            Collections.sort(list, new TimeCompare());
            if(LL.D) Log.d(LOG_TAG,"Sort thumbnail time stamp");
            int index =0;
            for(TimeOrder temp : list){
                randomAccessFile.seek(temp.getOrder());
                randomAccessFile.read(buffer);
                parseFromStream(buffer,temp.getTime(),tempwidth ,tempheight,tempformat,index ,thumbCount,thumbnailCallback);
                index++;
            }
            randomAccessFile.close();
            list.clear();
        }catch (IOException e){
            return Task.makeTaskError(e);
        }
        return null;
    }

    static ThumbnailError  parseFromStream(byte[] buffer, int  time, int thumbWidth, int thumbHeight, int thumbFormat, int index , int maxThumbnailCount, ThumbnailCallback thumbnailCallback ) throws IOException {

        if( thumbnailCallback==null )
            return ThumbnailError.ParameterError;

        boolean rawMode = (thumbnailCallback instanceof ThumbnailCallbackRaw);
        ThumbnailCallbackBitmap thumbnailCallbackBitmap;
        ThumbnailCallbackRaw thumbnailCallbackRaw;
        if( rawMode ) {
            thumbnailCallbackBitmap = null;
            thumbnailCallbackRaw = (ThumbnailCallbackRaw)thumbnailCallback;
        } else {
            thumbnailCallbackBitmap = (ThumbnailCallbackBitmap)thumbnailCallback;
            thumbnailCallbackRaw = null;
        }

        if(LL.D) Log.d(LOG_TAG, "processRawFile: w/h/time=" + thumbWidth + "/" + thumbHeight + "/"+ time +", format=" + thumbFormat);
        Bitmap bm = null;
        boolean flipThumbnails = false;
        if( thumbFormat == 32 ) {
            if( !rawMode )
                bm = Bitmap.createBitmap(thumbWidth, thumbHeight, Bitmap.Config.ARGB_8888);
            flipThumbnails = true;
        } else if( thumbFormat == 16 ) {
            if( !rawMode )
                bm = Bitmap.createBitmap(thumbWidth, thumbHeight, Bitmap.Config.RGB_565);
            flipThumbnails = true;
        } else if( thumbFormat == 8 ) {
            if( !rawMode )
                bm = Bitmap.createBitmap(thumbWidth, thumbHeight, Bitmap.Config.ARGB_8888);
            flipThumbnails = true;
        } else {
            return ThumbnailError.UnknownFormat;
        }
        Bitmap flipbm = null;
        Canvas flipCanvas = null;
        if( flipThumbnails && !rawMode ) {
            flipbm = Bitmap.createBitmap(thumbWidth,thumbHeight,bm.getConfig());
            flipCanvas = new Canvas(flipbm);
            flipCanvas.scale(1,-1);
        }

        ByteBuffer bufferWrapper = ByteBuffer.wrap(buffer);
        if(LL.D) Log.d(LOG_TAG,"processRawFile : thumbCount=" + maxThumbnailCount);

        if( rawMode ) {
            thumbnailCallbackRaw.processThumbnail(buffer, index, maxThumbnailCount, time);
        } else {
            bm.copyPixelsFromBuffer(bufferWrapper);
            Bitmap resultBitmap;
            if( flipThumbnails ) {
                flipCanvas.drawBitmap(bm, 0, -thumbHeight, null);
                resultBitmap = flipbm;
            } else {
                resultBitmap = bm;
            }
            thumbnailCallbackBitmap.processThumbnail(resultBitmap, index, maxThumbnailCount, time);
        }
        return null;
    }

    static ThumbnailError parseFromStream(InputStream input, long thumbFileSize, int maxThumbnailCount, ThumbnailCallback thumbnailCallback ) throws IOException {

        int thumbFormat;
        int thumbWidth;
        int thumbHeight;
        int sizeEach;
        int thumbCount;
        if( thumbnailCallback==null )
            return ThumbnailError.ParameterError;

        DataInputStream dataIn = new DataInputStream(input);
        thumbFormat = dataIn.readInt();
        thumbWidth  = dataIn.readInt();
        thumbHeight = dataIn.readInt();
        boolean rawMode = (thumbnailCallback instanceof ThumbnailCallbackRaw);
        ThumbnailCallbackBitmap thumbnailCallbackBitmap;
        ThumbnailCallbackRaw thumbnailCallbackRaw;
        if( rawMode ) {
            thumbnailCallbackBitmap = null;
            thumbnailCallbackRaw = (ThumbnailCallbackRaw)thumbnailCallback;
        } else {
            thumbnailCallbackBitmap = (ThumbnailCallbackBitmap)thumbnailCallback;
            thumbnailCallbackRaw = null;
        }

        boolean needSwapEndian = false;
        if( (thumbWidth & 0xFFFF0000)!=0 || (thumbHeight & 0xFFFF0000)!=0 ) {
            thumbFormat = swapEndian(thumbFormat);
            thumbWidth = swapEndian(thumbWidth);
            thumbHeight = swapEndian(thumbHeight);
            needSwapEndian = true;
        }
        if(LL.D) Log.d(LOG_TAG, "processRawFile: w/h/swap=" + thumbWidth + "/" + thumbHeight + "/" + needSwapEndian + " format=" + thumbFormat);
        Bitmap bm = null;
        boolean flipThumbnails = false;
        if( thumbFormat == 32 ) {
            if( !rawMode )
                bm = Bitmap.createBitmap(thumbWidth, thumbHeight, Bitmap.Config.ARGB_8888);
            flipThumbnails = true;
        } else if( thumbFormat == 16 ) {
            if( !rawMode )
                bm = Bitmap.createBitmap(thumbWidth, thumbHeight, Bitmap.Config.RGB_565);
            flipThumbnails = true;
        } else if( thumbFormat == 8 ) {
            if( !rawMode )
                bm = Bitmap.createBitmap(thumbWidth, thumbHeight, Bitmap.Config.ARGB_8888);
            flipThumbnails = true;
        } else {
            return ThumbnailError.UnknownFormat;
        }
        Bitmap flipbm = null;
        Canvas flipCanvas = null;
        if( flipThumbnails && !rawMode ) {
            flipbm = Bitmap.createBitmap(thumbWidth,thumbHeight,bm.getConfig());
            flipCanvas = new Canvas(flipbm);
            flipCanvas.scale(1,-1);
        }

        sizeEach = thumbWidth * thumbHeight * thumbFormat / 8;
        thumbCount = (int)Math.min(maxThumbnailCount,(thumbFileSize-8)/(sizeEach+4));
        if( thumbCount < 1 ) {
            return ThumbnailError.NoThumbailsFound;
        }

        byte[] buffer = new byte[sizeEach];
        ByteBuffer bufferWrapper = ByteBuffer.wrap(buffer);
        if(LL.D) Log.d(LOG_TAG,"processRawFile : thumbCount=" + thumbCount);

        for( int i=0; i<thumbCount; i++ ) {
            if(LL.D) Log.d(LOG_TAG,"processRawFile : i=" + i);

            int time = dataIn.readInt();
            if( needSwapEndian ) {
                time = swapEndian(time);
            }
            if(LL.D) Log.d(LOG_TAG,"processRawFile : time=" + time);

            int size = dataIn.read(buffer);
            if( size < sizeEach-1 ) {
                if( rawMode ) {
                    thumbnailCallbackRaw.processThumbnail(null, i, thumbCount, time);
                } else {
                    thumbnailCallbackBitmap.processThumbnail(null, i, thumbCount, time);
                }
                continue;
            }
            if( rawMode ) {
                thumbnailCallbackRaw.processThumbnail(buffer,i,thumbCount,time);
                bufferWrapper.rewind();
                continue;
            }
            bm.copyPixelsFromBuffer(bufferWrapper);
            bufferWrapper.rewind();
            Bitmap resultBitmap;
            if( flipThumbnails ) {
                flipCanvas.drawBitmap(bm, 0, -thumbHeight, null);
                resultBitmap = flipbm;
            } else {
                resultBitmap = bm;
            }
            thumbnailCallbackBitmap.processThumbnail(resultBitmap, i,thumbCount,time);
        }
        return null;
    }

    private static int swapEndian( int value ) {
        return 	((value&0xFF000000)>>>24)
                |	((value&0x00FF0000)>>>8)
                |	((value&0x0000FF00)<< 8)
                |	((value&0x000000FF)<< 24);
    }
    public static class TimeOrder{
        private int time;
        private int orgTime;
        private long offset;

        TimeOrder(int mTime, int mOrgTime, long mOffset){
            time = mTime;
            orgTime = mOrgTime;
            offset = mOffset;
        }
        public int getTime(){
            return time;
        }
        public int getOrgTime(){
            return orgTime;
        }
        public long getOrder(){
            return offset;
        }

    }

    static class TimeCompare implements Comparator<TimeOrder> {
        @Override
        public int compare(TimeOrder lhs, TimeOrder rhs) {
            return lhs.getTime()<rhs.getTime()?-1:lhs.getTime()>rhs.getTime()?1:0;
        }
    }

}
