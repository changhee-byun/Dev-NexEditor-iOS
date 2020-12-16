package com.nexstreaming.kminternal.nexvideoeditor;

import java.util.Vector;

/**
 * Provides information on clip.  This is returned by {@link NexEditor#getClipInfo}. See that
 * method for details.
 * 
 * @author Nextreaming Corporation
 *
 */

public final class NexClipManager
{
	NexClipManager()
	{
		mClipVec = new Vector<NexVisualClip>();
	}
	
	public int addClip(NexVisualClip item)
	{
		mClipVec.add(item);
		return 0;
	}
	public int deleteClip(int iClipID)
	{
		if( mClipVec.size() <= 0 ) return 0;
		
		for( int i = 0; i < mClipVec.size(); i++ )
		{
			if( mClipVec.get(i).mClipID == iClipID )
			{
				mClipVec.remove(i);
				return 0;
			}
			mClipVec.get(i).deleteAudioClip(iClipID);
		}
		return 0;
	}
	public void clearAllClip()
	{
		mClipVec.clear();
	}
	

	public Vector<NexVisualClip> mClipVec;
}

