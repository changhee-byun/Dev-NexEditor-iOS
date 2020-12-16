package com.nexstreaming.kminternal.nexvideoeditor;

import java.util.Vector;

import com.nexstreaming.kminternal.kinemaster.config.LL;

import android.util.Log;

/**
 * Provides information on effect manager. 
 * 
 * @author Nextreaming Corporation
 *
 */


public final class NexThemeItemManager
{
	public NexThemeItemManager()
	{
		mEffectVec = new Vector<NexThemeItem>();
	}
	public int addItem(String strEffectID, String strEffectName)
	{
		if(LL.D) Log.d("NexThemeItemManager","T=" + this + "; strEffectID=" + strEffectID + "; strEffectName=" + strEffectName);
		NexThemeItem effect = new NexThemeItem(strEffectID, strEffectName);
		mEffectVec.add(effect);
		return 0;
	}
	
	
	public int addItem2(int iEffectOffset, int iEffectOverlap, String strEffectID, String strEffectName)
	{
		if(LL.D) Log.d("NexThemeItemManager", String.format("[ThemeItemManager.java] ID(%s) Name(%s) Offset(%d) Overlap(%d)", strEffectID, strEffectName, iEffectOffset, iEffectOverlap));
		NexThemeItem effect = new NexThemeItem(strEffectID, strEffectName, iEffectOffset, iEffectOverlap);
		mEffectVec.add(effect);
		return 0;
	}	
	
	public int deleteItem(String strID)
	{
		if( mEffectVec.size() < 0 )
		{
			return -1;
		}
		
		for(int i = 0; i < mEffectVec.size(); i++ )
		{
			if( mEffectVec.elementAt(i).mEffectID.equals(strID) )
			{
				mEffectVec.remove(i);
			}
		}
		return 0;
	}
	
	public void clearItems()
	{
		mEffectVec.clear();
	}	
	
	public Vector<NexThemeItem> mEffectVec;
}
