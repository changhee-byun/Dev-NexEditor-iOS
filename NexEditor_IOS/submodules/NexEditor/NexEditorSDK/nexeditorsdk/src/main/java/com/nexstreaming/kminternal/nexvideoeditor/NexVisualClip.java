package com.nexstreaming.kminternal.nexvideoeditor;
import java.util.Arrays;
import java.util.Vector;

/**
 * Provides information on clip.  This is returned by {@link NexEditor#getClipInfo}. See that
 * method for details.
 * 
 * @author Nextreaming Corporation
 *
 */

public final class NexVisualClip
{
	public NexVisualClip()
	{
		mClipID			= 0;
		mClipType		= NexEditorType.NEXCLIP_TYPE_NONE;

		mTotalAudioTime		= 0;
		mTotalVideoTime		= 0;

		mTotalTime		= 0;
		mStartTime		= 0;
		mEndTime		= 0;
		mStartTrimTime	= 0;
		mEndTrimTime	= 0;
		mWidth			= 0;
		mHeight			= 0;
		mExistVideo		= 0;
		mExistAudio		= 0;
		mTitleStyle		= NexEditorType.NEXTITLE_STYLE_NONE;
		mTitleStartTime	= 0;
		mTitleEndTime	= 0;
		mStartRect		= new NexRectangle(0, 0, 0, 0); 
		mEndRect		= new NexRectangle(0, 0, 0, 0);
		mDestRect		= new NexRectangle(0, 0, 0, 0);
		mAudioOnOff		= 0;
		mClipVolume		= 0;
		mBGMVolume		= 0;
		mRotateState	= 0;

		mBrightness		= 0;
		mContrast		= 0;
		mSaturation		= 0;
		mTintcolor		= 0;
        mHue            = 0;
		mCustomLUT_A 	= 0;
		mCustomLUT_B 	= 0;
		mCustomLUT_Power= 0;
		mLUT			= 0;
		mVignette		= 0;

		mSpeedControl	= 0;
		mKeepPitch		= 1;

		mVoiceChanger 	= 0;
		mCompressor		= 0;
		mPitchFactor		= 0;
		mMusicEffector		= 0;
		mProcessorStrength	= -1;
		mBassStrength		= -1;
		mPanLeft		= -111;
		mPanRight		= -111;

		mEffectDuration	= 0;
		mEffectOffset 	= 0;
		mEffectOverlap	= 0;
		
		mVolumeEnvelopeLevel = null;
		mVolumeEnvelopeTime = null;
		
		mStartMatrix = null;
		mEndMatrix = null;

		mSlowMotion = 0;
		mMotionTracked = 0;
		mAudioClipVec = new Vector<NexAudioClip>();

        mFreezeDuration = 0;
        mIframePlay = 0;
	}
	
	public void setStartRect(int iLeft, int iTop, int iRight, int iBottom)
	{
		mStartRect.setRect(iLeft, iTop, iRight, iBottom);
	}
	
	public void setEndRect(int iLeft, int iTop, int iRight, int iBottom)
	{
		mEndRect.setRect(iLeft, iTop, iRight, iBottom);
	}

	public void setDestRect(int iLeft, int iTop, int iRight, int iBottom)
	{
		mDestRect.setRect(iLeft, iTop, iRight, iBottom);
	}
	
	public int addAudioClip(NexAudioClip clip)
	{
		mAudioClipVec.add(clip);
		return 0;
	}
	
	public void deleteAudioClip(int iClipID)
	{
		for(int i = 0; i < mAudioClipVec.size(); i++ )
		{
			if( mAudioClipVec.get(i).mClipID == iClipID )
			{
				mAudioClipVec.remove(i);
			}
		}
	}	
	
	public void clearAudioClips()
	{
		mAudioClipVec.clear();
	}	
	
	public int mClipID;
	public int mClipType;			// NexEditorType.NEXCLIP_TYPE_????
	public int mTotalAudioTime;
	public int mTotalVideoTime;
	public int mTotalTime;
	public int mStartTime;
	public int mEndTime;
	public int mStartTrimTime;
	public int mEndTrimTime;
	public int mWidth;
	public int mHeight;
	public int mExistVideo;		//?
	public int mExistAudio;		//?

	public String mTitle;
	public int mTitleStyle;
	public int mTitleStartTime;
	public int mTitleEndTime;
	public int mAudioOnOff;
	public int mClipVolume;			// 0 - 200
	public int mBGMVolume;			// 0 - 200
	public int mRotateState;

	public int mBrightness;
	public int mContrast;
	public int mSaturation;
	public int mHue;
	public int mTintcolor;
	public int mCustomLUT_A;
	public int mCustomLUT_B;
	public int mCustomLUT_Power;
	public int mLUT;
	public int mVignette;

	public int mSpeedControl;			/// 25 ~ 200
	public int mVoiceChanger;			// 0 ~ 4
	public int mKeepPitch;				// 0 or 1

	public int mEffectDuration;
	public int mEffectOffset;
	public int mEffectOverlap;
	public String mClipEffectID;
	public String mTitleEffectID;
	public String mFilterID;
	
	public NexRectangle mStartRect;
	public NexRectangle mEndRect;
	public NexRectangle mDestRect;
	
	public float[] mStartMatrix;
	public float[] mEndMatrix;
	
	public String mClipPath;
	public String mThumbnailPath;

	public int[] mVolumeEnvelopeLevel;
	public int[] mVolumeEnvelopeTime;

	public String mEnhancedAudioFilter;
	public String mEqualizer;

	public int mCompressor;
	public int mPitchFactor;
	public int mMusicEffector;
	public int mProcessorStrength;
	public int mBassStrength;
	public int mManualVolumeControl;
	public int mPanLeft;
	public int mPanRight;
	public int mSlowMotion; //0 , 1
	public int mMotionTracked; // 0, 1
	Vector<NexAudioClip> mAudioClipVec;

	public int mFreezeDuration;
    public int mIframePlay; //0 , 1
    
	@Override
	public boolean equals(Object o) {
		if (this == o) return true;
		if (o == null || getClass() != o.getClass()) return false;

		NexVisualClip that = (NexVisualClip) o;

		if (mClipID != that.mClipID) return false;
		if (mClipType != that.mClipType) return false;
		if (mTotalAudioTime != that.mTotalAudioTime) return false;
		if (mTotalVideoTime != that.mTotalVideoTime) return false;
		if (mTotalTime != that.mTotalTime) return false;
		if (mStartTime != that.mStartTime) return false;
		if (mEndTime != that.mEndTime) return false;
		if (mStartTrimTime != that.mStartTrimTime) return false;
		if (mEndTrimTime != that.mEndTrimTime) return false;
		if (mWidth != that.mWidth) return false;
		if (mHeight != that.mHeight) return false;
		if (mExistVideo != that.mExistVideo) return false;
		if (mExistAudio != that.mExistAudio) return false;
		if (mTitleStyle != that.mTitleStyle) return false;
		if (mTitleStartTime != that.mTitleStartTime) return false;
		if (mTitleEndTime != that.mTitleEndTime) return false;
		if (mAudioOnOff != that.mAudioOnOff) return false;
		if (mClipVolume != that.mClipVolume) return false;
		if (mBGMVolume != that.mBGMVolume) return false;
		if (mRotateState != that.mRotateState) return false;
		if (mBrightness != that.mBrightness) return false;
		if (mContrast != that.mContrast) return false;
		if (mSaturation != that.mSaturation) return false;
		if (mHue != that.mHue) return false;
		if (mTintcolor != that.mTintcolor) return false;
		if (mLUT != that.mLUT) return false;
		if (mCustomLUT_A != that.mCustomLUT_A) return false;
		if (mCustomLUT_B != that.mCustomLUT_B) return false;
		if (mCustomLUT_Power != that.mCustomLUT_Power) return false;
		if (mVignette != that.mVignette) return false;
		if (mSpeedControl != that.mSpeedControl) return false;
		if (mKeepPitch != that.mKeepPitch) return false;
		if (mVoiceChanger != that.mVoiceChanger) return false;
		if (mEffectDuration != that.mEffectDuration) return false;
		if (mEffectOffset != that.mEffectOffset) return false;
		if (mEffectOverlap != that.mEffectOverlap) return false;
		if (mCompressor != that.mCompressor) return false;
		if (mPitchFactor != that.mPitchFactor) return false;
		if (mMusicEffector != that.mMusicEffector) return false;
		if (mProcessorStrength != that.mProcessorStrength) return false;
		if (mBassStrength != that.mBassStrength) return false;
		if (mManualVolumeControl != that.mManualVolumeControl) return false;
		if (mPanLeft != that.mPanLeft) return false;
		if (mPanRight != that.mPanRight) return false;
		if (mSlowMotion != that.mSlowMotion) return false;
		if (mMotionTracked != that.mMotionTracked) return false;

		if (mTitle != null ? !mTitle.equals(that.mTitle) : that.mTitle != null) return false;
		if (mClipEffectID != null ? !mClipEffectID.equals(that.mClipEffectID) : that.mClipEffectID != null)
			return false;
		if (mTitleEffectID != null ? !mTitleEffectID.equals(that.mTitleEffectID) : that.mTitleEffectID != null)
			return false;
		if (mFilterID != null ? !mFilterID.equals(that.mFilterID) : that.mFilterID != null)
			return false;

		if (mStartRect != null ? !mStartRect.equals(that.mStartRect) : that.mStartRect != null)
			return false;
		if (mEndRect != null ? !mEndRect.equals(that.mEndRect) : that.mEndRect != null)
			return false;
		if (mDestRect != null ? !mDestRect.equals(that.mDestRect) : that.mDestRect != null)
			return false;

		if (!Arrays.equals(mStartMatrix, that.mStartMatrix)) return false;
		if (!Arrays.equals(mEndMatrix, that.mEndMatrix)) return false;
		if (!mClipPath.equals(that.mClipPath)) return false;
		if (mThumbnailPath != null ? !mThumbnailPath.equals(that.mThumbnailPath) : that.mThumbnailPath != null)
			return false;
		if (!Arrays.equals(mVolumeEnvelopeLevel, that.mVolumeEnvelopeLevel)) return false;
		if (!Arrays.equals(mVolumeEnvelopeTime, that.mVolumeEnvelopeTime)) return false;

        if( mFreezeDuration != that.mFreezeDuration ) return false;
        if (mIframePlay != that.mIframePlay) return false;

		if (mEnhancedAudioFilter != null ? !mEnhancedAudioFilter.equals(that.mEnhancedAudioFilter) : that.mEnhancedAudioFilter != null)
			return false;

		if (mEqualizer != null ? !mEqualizer.equals(that.mEqualizer) : that.mEqualizer != null)
			return false;

		return !(mAudioClipVec != null ? !mAudioClipVec.equals(that.mAudioClipVec) : that.mAudioClipVec != null);
	}

	@Override
	public int hashCode() {
		int result = mClipID;
		result = 31 * result + mClipType;
		result = 31 * result + mTotalAudioTime;
		result = 31 * result + mTotalVideoTime;
		result = 31 * result + mTotalTime;
		result = 31 * result + mStartTime;
		result = 31 * result + mEndTime;
		result = 31 * result + mStartTrimTime;
		result = 31 * result + mEndTrimTime;
		result = 31 * result + mWidth;
		result = 31 * result + mHeight;
		result = 31 * result + mExistVideo;
		result = 31 * result + mExistAudio;
		result = 31 * result + (mTitle != null ? mTitle.hashCode() : 0);
		result = 31 * result + mTitleStyle;
		result = 31 * result + mTitleStartTime;
		result = 31 * result + mTitleEndTime;
		result = 31 * result + mAudioOnOff;
		result = 31 * result + mClipVolume;
		result = 31 * result + mBGMVolume;
		result = 31 * result + mRotateState;
		result = 31 * result + mBrightness;
		result = 31 * result + mContrast;
		result = 31 * result + mSaturation;
		result = 31 * result + mHue;
		result = 31 * result + mTintcolor;
		result = 31 * result + mLUT;
		result = 31 * result + mCustomLUT_A;
		result = 31 * result + mCustomLUT_B;
		result = 31 * result + mCustomLUT_Power;
		result = 31 * result + mVignette;
		result = 31 * result + mSpeedControl;
		result = 31 * result + mKeepPitch;
		result = 31 * result + mVoiceChanger;
		result = 31 * result + mEffectDuration;
		result = 31 * result + mEffectOffset;
		result = 31 * result + mEffectOverlap;
		result = 31 * result + (mClipEffectID != null ? mClipEffectID.hashCode() : 0);
		result = 31 * result + (mTitleEffectID != null ? mTitleEffectID.hashCode() : 0);
		result = 31 * result + (mFilterID != null ? mFilterID.hashCode() : 0);
		result = 31 * result + (mStartRect != null ? mStartRect.hashCode() : 0);
		result = 31 * result + (mEndRect != null ? mEndRect.hashCode() : 0);
		result = 31 * result + (mDestRect != null ? mDestRect.hashCode() : 0);
		result = 31 * result + (mStartMatrix != null ? Arrays.hashCode(mStartMatrix) : 0);
		result = 31 * result + (mEndMatrix != null ? Arrays.hashCode(mEndMatrix) : 0);
		result = 31 * result + mClipPath.hashCode();
		result = 31 * result + (mThumbnailPath != null ? mThumbnailPath.hashCode() : 0);
		result = 31 * result + (mVolumeEnvelopeLevel != null ? Arrays.hashCode(mVolumeEnvelopeLevel) : 0);
		result = 31 * result + (mVolumeEnvelopeTime != null ? Arrays.hashCode(mVolumeEnvelopeTime) : 0);
		result = 31 * result + (mEnhancedAudioFilter != null ? mEnhancedAudioFilter.hashCode() : 0);
		result = 31 * result + (mEqualizer != null ? mEqualizer.hashCode() : 0);
		result = 31 * result + mCompressor;
		result = 31 * result + mPitchFactor;
		result = 31 * result + mMusicEffector;
		result = 31 * result + mProcessorStrength;
		result = 31 * result + mBassStrength;
		result = 31 * result + mManualVolumeControl;
		result = 31 * result + mPanLeft;
		result = 31 * result + mPanRight;
		result = 31 * result + mSlowMotion;
		result = 31 * result + mMotionTracked;
		result = 31 * result + (mAudioClipVec != null ? mAudioClipVec.hashCode() : 0);
        result = 31 * result + mFreezeDuration;
        result = 31 * result + mIframePlay;

		return result;
	}
}

