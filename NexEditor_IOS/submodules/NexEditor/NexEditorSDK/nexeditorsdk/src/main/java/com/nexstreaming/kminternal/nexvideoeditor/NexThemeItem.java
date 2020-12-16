package com.nexstreaming.kminternal.nexvideoeditor;

/**
 * Provides information on theme item. 
 * 
 * @author Nextreaming Corporation
 *
 */
public final class NexThemeItem
{
	public NexThemeItem(String strEffectID, String strEffectName)
	{
		mEffectID	= strEffectID;
		mEffectName	= strEffectName;
		mEffectOffset	= 0;
		mEffectOverlap	= 0;
	}
	public NexThemeItem(String strEffectID, String strEffectName, int iEffectOffset, int iEffectOverlap)
	{
		mEffectID		= strEffectID;
		mEffectName		= strEffectName;
		mEffectOffset	= iEffectOffset;
		mEffectOverlap	= iEffectOverlap;
	}	
	
	public String mEffectID;
	public String mEffectName;
	
	public int mEffectOffset;
	public int mEffectOverlap;
}
