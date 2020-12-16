package com.nexstreaming.app.common.util;

import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;


public class Profiler {
	
	private final String mTag;
	private final String mLogTag;
	private long mStartTime; 
	private long mPrevTime; 
	private int mPass;
	private final boolean mEnabled;
	
	public Profiler( String tag, boolean enabled ) {
		mTag = tag;
		mEnabled = enabled;
		mLogTag = "NexProfile_" + mTag;
		mStartTime = System.nanoTime();
		mPrevTime = mStartTime;
		mPass = 0;
	}
	
	public void reset() {
		mStartTime = System.nanoTime();
		mPrevTime = mStartTime;
		mPass++;
	}
	
	public void checkpoint( int num ) {
		if( !mEnabled ) {
			return;
		}
		long t = System.nanoTime();
		long elapsed = t-mPrevTime;
		long total = t-mStartTime;
		mPrevTime = t;
		if(LL.V) Log.v(mLogTag, "[" + mPass + "] Profile Checkpoint " + num + ": elapsed=" + (elapsed/1000000) + "." + ((elapsed%1000000)/100000) + " total=" + (total/1000000) + "." + ((total%1000000)/100000));
	}
	
}
