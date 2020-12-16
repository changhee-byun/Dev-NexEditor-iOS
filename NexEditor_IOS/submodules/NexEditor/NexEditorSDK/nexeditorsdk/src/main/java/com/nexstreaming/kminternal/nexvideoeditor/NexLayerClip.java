package com.nexstreaming.kminternal.nexvideoeditor;
import java.util.Vector;

/**
 * Provides information on clip.  This is returned by {@link NexEditor#getClipInfo}. See that
 * method for details.
 * 
 * @author Nextreaming Corporation
 *
 */

public final class NexLayerClip
{
	public NexLayerClip()
	{
		mID				= 0;
		mType			= NexEditorType.NEXCLIP_TYPE_NONE;

		mTotalTime		= 0;
		mStartTime		= 0;
		mEndTime		= 0;
		mStartTrimTime	= 0;
		mEndTrimTime	= 0;
		mWidth			= 0;
		mHeight			= 0;
		mExistVideo		= 0;

		mRotateState	= 0;

		mBrightness		= 0;
		mContrast		= 0;
		mSaturation		= 0;
		mHue			= 0;
		mTintcolor		= 0;

		mSpeedControl	= 0;
	}
	
	public int mID;
	public int mType;			// NexEditorType.NEXCLIP_TYPE_????
	
	public int mTotalTime;
	public int mStartTime;
	public int mEndTime;
	
	public int mStartTrimTime;
	public int mEndTrimTime;
	
	public int mWidth;
	public int mHeight;
	
	public int mExistVideo;		//?

	public int mBrightness;
	public int mContrast;
	public int mSaturation;
	public int mHue;
	public int mTintcolor;
	
	public int mRotateState;
	
	public int mSpeedControl;			/// 25 ~ 200
	
	public String mDisplayOptions;

	public String mClipPath;
}

