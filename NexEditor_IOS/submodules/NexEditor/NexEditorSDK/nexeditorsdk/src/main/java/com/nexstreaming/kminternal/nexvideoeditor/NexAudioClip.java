package com.nexstreaming.kminternal.nexvideoeditor;

import com.nexstreaming.kminternal.nexvideoeditor.NexEditorType;

import java.util.Arrays;

/**
 * Provides information on audio clip.
 * method for details.
 * 
 * @author Nextreaming Corporation
 *
 */

public final class NexAudioClip
{
	public NexAudioClip()
	{
		mClipID			= 0;
		mVisualClipID		= 0;
		mClipType		= NexEditorType.NEXCLIP_TYPE_AUDIO;
		mTotalTime		= 0;
		mStartTime		= 0;
		mEndTime		= 0;
		mStartTrimTime		= 0;
		mEndTrimTime		= 0;
		mAudioOnOff		= 0;
		mAutoEnvelop		= 0;
		mClipVolume		= 0;
		mVolumeEnvelopeLevel	= null;
		mVolumeEnvelopeTime	= null;
		mVoiceChanger		= 0;
		mCompressor		= 0;
		mPitchFactor		= 0;
		mMusicEffector		= 0;
		mProcessorStrength 	= -1;
		mBassStrength		= -1;
		mPanLeft		= -111;
		mPanRight		= -111;
		mSpeedControl = 100;
	}
	
	public NexAudioClip(	int iClipID, 
							int iVisualClipID,
							int iClipType,
							int iTotalTime,
							int iStartTime,
							int iEndTime,
							int iStartTrimTime,
							int iEndTrimTime,
							int iAudioOnOff,
							int iClipVolume,
							int iVoiceChanger)
	{
		mClipID			= iClipID;
		mVisualClipID		= iVisualClipID;
		mClipType		= iClipType;
		mTotalTime		= iTotalTime;
		mStartTime		= iStartTime;
		mEndTime		= iEndTime;
		mStartTrimTime		= iStartTrimTime;
		mEndTrimTime		= iEndTrimTime;
		mAudioOnOff		= iAudioOnOff;
		mClipVolume		= iClipVolume;
		mVoiceChanger		= iVoiceChanger;
		mCompressor		= 0;
		mPitchFactor		= 0;
		mMusicEffector		= 0;
		mProcessorStrength	= -1;
		mBassStrength		= -1;
		mPanLeft		= -111;
		mPanRight		= -111;
		mSpeedControl = 100;
	}

    public NexAudioClip(	int iClipID,
                            int iVisualClipID,
                            int iClipType,
                            int iTotalTime,
                            int iStartTime,
                            int iEndTime,
                            int iStartTrimTime,
                            int iEndTrimTime,
                            int iAudioOnOff,
                            int iAutoEnvelop,
                            int iClipVolume,
                            int iVoiceChanger)
	{
		mClipID			= iClipID;
		mVisualClipID		= iVisualClipID;
		mClipType		= iClipType;
		mTotalTime		= iTotalTime;
		mStartTime		= iStartTime;
		mEndTime		= iEndTime;
		mStartTrimTime		= iStartTrimTime;
		mEndTrimTime		= iEndTrimTime;
		mAudioOnOff		= iAudioOnOff;
		mAutoEnvelop		= iAutoEnvelop;
		mClipVolume		= iClipVolume;
		mVoiceChanger		= iVoiceChanger;
		mCompressor		= 0;
		mPitchFactor		= 0;
		mMusicEffector		= 0;
		mProcessorStrength	= -1;
		mBassStrength		= -1;
		mPanLeft		= -111;
		mPanRight		= -111;
		mSpeedControl = 100;
	}

	public int mClipID;
	public int mVisualClipID;
	public int mClipType;
	public int mTotalTime;
	public int mStartTime;
	public int mEndTime;
	public int mStartTrimTime;
	public int mEndTrimTime;
	public String mClipPath;
	public int mAudioOnOff;
	public int mAutoEnvelop;
	public int mClipVolume;
	public int[] mVolumeEnvelopeLevel;
	public int[] mVolumeEnvelopeTime;
	public int mVoiceChanger;

	public int mCompressor;
	public int mPitchFactor;
	public int mMusicEffector;
	public int mProcessorStrength;
	public int mBassStrength;
	public int mPanLeft;
	public int mPanRight;
	public int mSpeedControl;			/// 25 ~ 200
	public String mEnhancedAudioFilter;
	public String mEqualizer;

	@Override
	public boolean equals(Object o) {
		if (this == o) return true;
		if (o == null || getClass() != o.getClass()) return false;

		NexAudioClip that = (NexAudioClip) o;

		if (mClipID != that.mClipID) return false;
		if (mVisualClipID != that.mVisualClipID) return false;
		if (mClipType != that.mClipType) return false;
		if (mTotalTime != that.mTotalTime) return false;
		if (mStartTime != that.mStartTime) return false;
		if (mEndTime != that.mEndTime) return false;
		if (mStartTrimTime != that.mStartTrimTime) return false;
		if (mEndTrimTime != that.mEndTrimTime) return false;
		if (mAudioOnOff != that.mAudioOnOff) return false;
		if (mAutoEnvelop != that.mAutoEnvelop) return false;
		if (mClipVolume != that.mClipVolume) return false;
		if (mVoiceChanger != that.mVoiceChanger) return false;
		if (mCompressor != that.mCompressor) return false;
		if (mPitchFactor != that.mPitchFactor) return false;
		if (mMusicEffector != that.mMusicEffector) return false;
		if (mProcessorStrength != that.mProcessorStrength) return false;
		if (mBassStrength != that.mBassStrength) return false;
		if (mPanLeft != that.mPanLeft) return false;
		if (mPanRight != that.mPanRight) return false;
		if (mSpeedControl != that.mSpeedControl) return false;
		if (!mClipPath.equals(that.mClipPath)) return false;
		if (!Arrays.equals(mVolumeEnvelopeLevel, that.mVolumeEnvelopeLevel)) return false;
		if (!Arrays.equals(mVolumeEnvelopeTime, that.mVolumeEnvelopeTime)) return false;
		if (mEqualizer != null ? !mEqualizer.equals(that.mEqualizer) : that.mEqualizer != null) return false;
		return !(mEnhancedAudioFilter != null ? !mEnhancedAudioFilter.equals(that.mEnhancedAudioFilter) : that.mEnhancedAudioFilter != null);

	}

	@Override
	public int hashCode() {
		int result = mClipID;
		result = 31 * result + mVisualClipID;
		result = 31 * result + mClipType;
		result = 31 * result + mTotalTime;
		result = 31 * result + mStartTime;
		result = 31 * result + mEndTime;
		result = 31 * result + mStartTrimTime;
		result = 31 * result + mEndTrimTime;
		result = 31 * result + mClipPath.hashCode();
		result = 31 * result + mAudioOnOff;
		result = 31 * result + mAutoEnvelop;
		result = 31 * result + mClipVolume;
		result = 31 * result + (mVolumeEnvelopeLevel != null ? Arrays.hashCode(mVolumeEnvelopeLevel) : 0);
		result = 31 * result + (mVolumeEnvelopeTime != null ? Arrays.hashCode(mVolumeEnvelopeTime) : 0);
		result = 31 * result + mVoiceChanger;
		result = 31 * result + mCompressor;
		result = 31 * result + mPitchFactor;
		result = 31 * result + mMusicEffector;
		result = 31 * result + mProcessorStrength;
		result = 31 * result + mBassStrength;
		result = 31 * result + mPanLeft;
		result = 31 * result + mPanRight;
		result = 31 * result + mSpeedControl;
		result = 31 * result + (mEqualizer != null ? mEqualizer.hashCode() : 0);
		result = 31 * result + (mEnhancedAudioFilter != null ? mEnhancedAudioFilter.hashCode() : 0);
		return result;
	}
}

