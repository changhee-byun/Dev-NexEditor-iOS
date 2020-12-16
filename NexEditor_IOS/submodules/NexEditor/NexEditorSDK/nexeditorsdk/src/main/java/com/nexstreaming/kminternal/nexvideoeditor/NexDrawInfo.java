package com.nexstreaming.kminternal.nexvideoeditor;
import java.util.Vector;

/**
 * Provides information on clip.  This is returned by {@link NexEditor#getClipInfo}. See that
 * method for details.
 * 
 * @author Nextreaming Corporation
 *
 */

public final class NexDrawInfo
{
	public NexDrawInfo()
	{
        mID             = 0;
        mTrackID        = 0;

        mEffectID       = null;
        mTitle          = null;

        mStartTime      = 0;
        mEndTime        = 0;

        mRotateState    = 0;
        mUserRotateState = 0;
        mTranslateX = 0;
        mTranslateY = 0;
        mBrightness     = 0;
        mContrast       = 0;
        mSaturation     = 0;
        mHue            = 0;
        mTintcolor      = 0;
        mLUT            = 0;
        mCustomLUT_A    = 0;
        mCustomLUT_B    = 0;
        mCustomLUT_Power= 0;

        mStartRect      = new NexRectangle(0, 0, 0, 0);
        mEndRect        = new NexRectangle(0, 0, 0, 0);
        mFaceRect        = new NexRectangle(0, 0, 0, 0);
	}

    public int			mID;
    public int			mTrackID;
    public int          mSubEffectID;

    public String       mEffectID;
    public String       mTitle;
    public int          mIsTransition;

    public int			mStartTime;
    public int			mEndTime;

    public int			mRotateState;
    public int          mUserRotateState;
    public int          mTranslateX;
    public int          mTranslateY;
    public int			mBrightness;
    public int			mContrast;
    public int			mSaturation;
    public int          mHue;
    public int			mTintcolor;
    public int			mLUT;
    public int          mCustomLUT_A;
    public int          mCustomLUT_B;
    public int          mCustomLUT_Power;

    public NexRectangle    mStartRect;
    public NexRectangle    mEndRect;
    public NexRectangle    mFaceRect;
}

