package com.nexstreaming.kminternal.kinemaster.mediainfo;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.app.common.task.Task;

import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.AsyncTask;
import android.util.Log;

abstract class ThumbnailConversionTask extends AsyncTask<Integer, Integer, Task.TaskError> {

	private static final String LOG_TAG = "KMMediaInfo_ThumbConv";

	private static final int MAX_THUMBNAIL_COUNT = 50;
	private static final int THUMBNAIL_SMALL_WIDTH = 160;
	private static final int THUMBNAIL_SMALL_HEIGHT = 90;
	private static final int THUMBNAIL_LARGE_WIDTH = 640;
	private static final int THUMBNAIL_LARGE_HEIGHT = 360;

	private File mThumbOutputFile;
    private File mLargeStartThumbOutputFile;
    private File mLargeEndThumbOutputFile;
	private File mRawThumbInputFile;
	private long mThumbFileSize;
//	private int mThumbFormat;
//	private int mThumbWidth;
//	private int mThumbHeight;
//	private int mSizeEach;
//	private int mThumbCount;
	private Bitmap mSmallThumbBitmap;
    private Bitmap mLargeStartThumbBitmap;
    private Bitmap mLargeEndThumbBitmap;
	private int[] mThumbTime;
	
	public ThumbnailConversionTask( File rawThumbInput, File smallThumbOutput, File largeStartThumbOutput, File largeEndThumbOutput ) {
		mThumbOutputFile = smallThumbOutput;
		mRawThumbInputFile = rawThumbInput;
        mLargeStartThumbOutputFile = largeStartThumbOutput;
        mLargeEndThumbOutputFile = largeEndThumbOutput;
	}
	
	@Override
	protected Task.TaskError doInBackground(Integer... params) {
		if( !mRawThumbInputFile.exists() ) {
			if(LL.D) Log.d(LOG_TAG,"doInBackground : raw thumbnail file not found");
			return ThumbnailError.RawFileNotFound;
		}
		
		mThumbFileSize = mRawThumbInputFile.length();
		if(LL.D) Log.d(LOG_TAG,"doInBackground : mThumbFileSize=" + mThumbFileSize);
		
		if( mThumbFileSize < 8 ) {
			if(LL.D) Log.d(LOG_TAG,"doInBackground : raw thumbnail file too small");
			return ThumbnailError.RawFileTooSmall;
		}
		
		try {
			Task.TaskError error = openAndProcessRawFile();
			if( error!=null ) {
				if(LL.D) Log.d(LOG_TAG,"doInBackground : raw thumbnail file parse error");
				return error;
			}
			writeOutputFiles();
		} catch (IOException e) {
			if(LL.D) Log.d(LOG_TAG,"doInBackground : EXCEPTION", e);
			return Task.makeTaskError(e);
		}
		if(LL.D) Log.d(LOG_TAG,"doInBackground : out");

		return null;
	}
	
	private Task.TaskError processRawFile(InputStream input) throws IOException  {

		return ThumbnailParser.parseFromStream(input, mThumbFileSize, MAX_THUMBNAIL_COUNT, new ThumbnailCallbackBitmap() {

			int outputHeight;
			int outputWidth;
			Bitmap outputBitmap;
			Canvas canvas;
			Rect dst;
			Paint paint;

			@Override
			public void processThumbnail(Bitmap bm, int i, int totalCount, int timestamp) {

				if (i == 0) {
					outputHeight = THUMBNAIL_SMALL_HEIGHT;
					outputWidth = THUMBNAIL_SMALL_WIDTH * totalCount;
					outputBitmap = Bitmap.createBitmap(outputWidth, outputHeight, Bitmap.Config.RGB_565);
					mSmallThumbBitmap = outputBitmap;
					canvas = new Canvas(outputBitmap);
					dst = new Rect(0, 0, THUMBNAIL_SMALL_WIDTH, THUMBNAIL_SMALL_HEIGHT);
					paint = new Paint();
					paint.setFilterBitmap(true);
					mThumbTime = new int[totalCount];
					if(LL.D) Log.d(LOG_TAG,"processRawFile : totalCount=" + totalCount);
				}

				mThumbTime[i] = timestamp;

				if( bm==null ) {
					dst.offset(THUMBNAIL_SMALL_WIDTH, 0);
					return;
				}

				if( i==0 ) {
					if( LL.D ) Log.d(LOG_TAG,"Make large thumnail at i==0");
					Bitmap largeBitmap = Bitmap.createBitmap(THUMBNAIL_LARGE_WIDTH, THUMBNAIL_LARGE_HEIGHT, Bitmap.Config.RGB_565);
					Canvas largeCanvas = new Canvas(largeBitmap);
					largeCanvas.drawBitmap(bm, null, new Rect(0,0,THUMBNAIL_LARGE_WIDTH,THUMBNAIL_LARGE_HEIGHT), paint);
					mLargeStartThumbBitmap = largeBitmap;
				} else if( i==totalCount-1 ) {
					if( LL.D ) Log.d(LOG_TAG,"Make large end thumnail at i==" + i );
					Bitmap largeBitmap = Bitmap.createBitmap(THUMBNAIL_LARGE_WIDTH, THUMBNAIL_LARGE_HEIGHT, Bitmap.Config.RGB_565);
					Canvas largeCanvas = new Canvas(largeBitmap);
					largeCanvas.drawBitmap(bm, null, new Rect(0,0,THUMBNAIL_LARGE_WIDTH,THUMBNAIL_LARGE_HEIGHT), paint);
					mLargeEndThumbBitmap = largeBitmap;
				}
				canvas.save();
				canvas.scale(-1, -1, THUMBNAIL_SMALL_WIDTH/2, THUMBNAIL_SMALL_HEIGHT/2);
				canvas.drawBitmap(bm, null, dst, paint);
				canvas.restore();
				canvas.translate(THUMBNAIL_SMALL_WIDTH, 0);
//				canvas.drawBitmap(bm, null, dst, paint);
//				canvas.translate(THUMBNAIL_SMALL_WIDTH, 0);
			}
		});

	}
	
	private Task.TaskError openAndProcessRawFile() throws IOException {
		InputStream input = new BufferedInputStream(new FileInputStream(mRawThumbInputFile));
		try {
			return processRawFile(input);
		} finally {
			input.close();
		}
	}

	@Override
	protected final void onPostExecute(Task.TaskError error) {
		if( error==null ) {
			onSuccess();
		} else {
			onError(error);
		}
	};

	abstract protected void onSuccess();

	abstract protected void onError(Task.TaskError error);
	
	private void writeOutputFiles() throws IOException {
        writeBitmapToFile(null, mLargeStartThumbBitmap, mLargeStartThumbOutputFile);
        writeBitmapToFile(null, mLargeEndThumbBitmap==null?mLargeStartThumbBitmap:mLargeEndThumbBitmap, mLargeEndThumbOutputFile);
		writeBitmapToFile(mThumbTime, mSmallThumbBitmap, mThumbOutputFile);
	}
	
	private void writeBitmapToFile( int[] timeIndex, Bitmap bm, File file ) throws IOException {
		if(LL.D) Log.d(LOG_TAG,"writeBitmapToFile(" + file + ") : " + (timeIndex==null?"no index":"width index"));
		DataOutputStream output = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(file)));
		try {
			if( timeIndex != null ) {
				output.writeInt(THUMBNAIL_SMALL_WIDTH);
				output.writeInt(THUMBNAIL_SMALL_HEIGHT);
				output.writeInt(timeIndex.length);
				for( int i=0; i<timeIndex.length; i++ ) {
					output.writeInt(timeIndex[i]);
				}
			}
			bm.compress(CompressFormat.JPEG, 70, output);
		} finally {
			output.close();
			file.setReadable(true);
		}
	}
	
//	private static int swapEndian( int value ) {
//		return 	((value&0xFF000000)>>>24)
//			|	((value&0x00FF0000)>>>8)
//			|	((value&0x0000FF00)<< 8)
//			|	((value&0x000000FF)<< 24);
//	}

}
