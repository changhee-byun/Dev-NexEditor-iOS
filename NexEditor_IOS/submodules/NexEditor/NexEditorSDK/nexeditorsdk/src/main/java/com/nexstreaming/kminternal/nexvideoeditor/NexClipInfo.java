package com.nexstreaming.kminternal.nexvideoeditor;

/**
 * Provides information on clip.  This is returned by {@link NexEditor#getClipInfo}. See that
 * method for details.
 * 
 * @author Nextreaming Corporation
 *
 */
public final class NexClipInfo
{
	/** If the media include video, this is 1; otherwise it is 0. */
	public int mExistVideo;

	/** If the media include audio, this is 1; otherwise it is 0. */
	public int mExistAudio;

	/** Width of the video, in pixels */
	public int mVideoWidth;

	/** Height of the video, in pixels */
	public int mVideoHeight;

	/** Width of the video, in pixels */
	public int mDisplayVideoWidth;

	/** Height of the video, in pixels */
	public int mDisplayVideoHeight;
	
	/** Duration of the clip, in second */
	public int mAudioDuration;	
	
	/** Duration of the clip, in second */
	public int mVideoDuration;	
	
	/** Seek point count of the clip */
	public int mSeekPointCount;	

	/** Seek point count of the clip */
	public int [] mSeekTable;	

	/** Frame per second of the clip */
	public int mFPS;
	public float mfFPS;

	/** Video codec(H264) profile of the clip
	 *  Unknown : 0
	 *  Baseline : 66
	 *  Main : 77
	 *  Extended : 88
	 *  High : 100
	 *  High10 : 110
	 *  High422 : 122
	 *  High444 : 244 */
	public int mVideoH264Profile;

    /** Video Codec Type of Clip */
    public int mVideoCodecType;

    /** Video codec(H264) Level of the clip */
	public int mVideoH264Level;

    /** Video codec(H264) ScanType of the clip */
	public int mVideoH264Interlaced;
	
	/** Video Bitrateof the clip */
	public int mVideoBitRate;

    /** Video Orientation the clip */
    public int mVideoOrientation;

	/** Audio SampleRate the clip */
	public int mAudioSampleRate;

	/** Audio channels the clip */
	public int mAudioChannels;

	/** Audio Bitrateof the clip */
	public int mAudioBitRate;

    /** Audio Codec Type of Clip */
    public int mAudioCodecType;

    /** File path of thumbnail, if available thumbnail. */
	public String mThumbnailPath;

	public int mVideoRenderType; //yoon

	public byte[] mVideoUUID;

	public int mVideoEditBoxTime;
	public int mAudioEditBoxTime;

	public int mVideoHDRType;
}
