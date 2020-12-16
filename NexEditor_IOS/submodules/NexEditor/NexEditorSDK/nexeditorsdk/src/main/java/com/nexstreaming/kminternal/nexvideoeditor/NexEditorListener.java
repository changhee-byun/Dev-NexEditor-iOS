package com.nexstreaming.kminternal.nexvideoeditor;

public interface NexEditorListener {

	void onStateChange(NexEditor.PlayState oldState, NexEditor.PlayState newState);
	
	void onTimeChange(int currentTime);
		
	void onSetTimeDone(int currentTime);
	
	void onSetTimeFail(NexEditor.ErrorCode err);
	
	void onSetTimeIgnored();
	
	void onEncodingDone(NexEditor.ErrorCode result);
	
	void onPlayEnd();
	
	void onPlayFail(NexEditor.ErrorCode err, int iClipID);
	
	void onPlayStart();
	
	void onClipInfoDone();
	
	void onSeekStateChanged(boolean isSeeking);

	void onCheckDirectExport(NexEditor.ErrorCode err, int result);

	void onProgressThumbnailCaching(int progress, int maxValue);

	void onReverseProgress(int progress, int maxValue);

	void onReverseDone(NexEditor.ErrorCode result);

	void onFastPreviewStartDone(NexEditor.ErrorCode result, int iStartTime, int iEndTime);

	void onFastPreviewStopDone(NexEditor.ErrorCode result);

	void onFastPreviewTimeDone(NexEditor.ErrorCode result);

	void onPreviewPeakMeter(int iCts, int iLeftValue, int iRightValue);
}
