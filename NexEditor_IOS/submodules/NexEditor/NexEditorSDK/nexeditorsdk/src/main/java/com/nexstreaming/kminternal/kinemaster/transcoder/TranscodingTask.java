package com.nexstreaming.kminternal.kinemaster.transcoder;

import java.io.File;

import android.os.SystemClock;
import android.util.Log;

import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.config.NexExportProfile;

public class TranscodingTask extends Task {

	private static final String LOG_TAG = "TranscodingTask";

	private final File sourcePath;
	private final File destinationPath;
	private final File tempPath;
	private final NexExportProfile exportProfile;
	
	TranscodingTask( File sourcePath, File tempPath,  File destinationPath, NexExportProfile exportProfile ) {
		if( LL.D ) Log.d(LOG_TAG,"constructor : " + this + " sourcePath=" + sourcePath + " destinationPath=" + destinationPath);
		this.sourcePath = sourcePath;
		this.destinationPath = destinationPath;
		this.exportProfile = exportProfile;
		this.tempPath = tempPath;
	}
	
	public File getSourcePath() {
		return sourcePath;
	}

	public File getDestinationPath() {
		return destinationPath;
	}

	public File getTempPath() {
		return tempPath;
	}

	public NexExportProfile getExportProfile() {
		return exportProfile;
	}
	
	public void cancel() {
		Transcoder.cancelTask(this);
	}
	
	long transcodingStartTime;
	long transcodingEndTime;
	boolean beforeTranscoding = true;
	boolean doingTranscoding = false;
	
	void signalTranscodingStart() {
		if( LL.D ) Log.d(LOG_TAG,"signalTranscodingStart");
		if( !beforeTranscoding ) {
			throw new IllegalStateException();
		}
		beforeTranscoding = false;
		transcodingStartTime = SystemClock.uptimeMillis();
		doingTranscoding = true;
	}
	
	void signalTranscodingRetry() {
		if( LL.D ) Log.d(LOG_TAG,"signalTranscodingStart");
		beforeTranscoding = true;
		transcodingStartTime = 0;
		doingTranscoding = false;
	}
	
	
	void signalTranscodingDone() {
		if( LL.D ) Log.d(LOG_TAG,"signalTranscodingDone");
		if( !doingTranscoding ) {
			if( LL.W ) Log.w(LOG_TAG,"signalTranscodingDone called, but not transcoding");
			return;
//			throw new IllegalStateException();
		}
		transcodingEndTime = SystemClock.uptimeMillis();
		doingTranscoding = false;
	}
	
	public long getElapsedTime() {
		long elapsed;
		if( doingTranscoding ) {
			elapsed = SystemClock.uptimeMillis() - transcodingStartTime;
		} else {
			elapsed = transcodingEndTime - transcodingStartTime;
		}
		if( LL.D ) Log.d(LOG_TAG,"getElapsedTime returning " + elapsed);
		return elapsed;
	}
	
	public boolean isTranscoding() {
		return doingTranscoding;
	}
	
	/**
	 * Gets the remaining time
	 * @return	If the remaining time is unknown, returns -1. 
	 * 			If transcoding has completed or failed, returns 0.
	 * 			If transcoding is in progress and the remaining time has been calculated,
	 * 				returns the remaining time in milliseconds.
	 */
	public long getRemainingTime() {
		long elapsed = getElapsedTime();
		if( LL.D ) Log.d(LOG_TAG,"getRemainingTime beforeTranscoding=" + beforeTranscoding + " didSignalEvent(Task.Event.PROGRESS)=" + didSignalEvent(Task.Event.PROGRESS) + " elapsed=" + elapsed);
		int progress = getProgress();
		if( beforeTranscoding || !isProgressAvailable() || progress<1 || (elapsed < 12000 && progress<getMaxProgress()/2) ) {
			if( LL.D ) Log.d(LOG_TAG,"getRemainingTime INDETERMINATE");
			return -1;
		} else if( !doingTranscoding ) {
			if( LL.D ) Log.d(LOG_TAG,"getRemainingTime DONE");
			return 0;
		}
		int maxProgress = getMaxProgress();
		long remaining = elapsed * (maxProgress-progress) / progress;
		if( LL.D ) Log.d(LOG_TAG,"getRemainingTime returning : " + remaining);
		return remaining;
	}

}
