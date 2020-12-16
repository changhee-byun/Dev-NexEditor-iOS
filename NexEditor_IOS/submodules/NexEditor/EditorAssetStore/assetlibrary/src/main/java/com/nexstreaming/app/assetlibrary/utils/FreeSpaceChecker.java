package com.nexstreaming.app.assetlibrary.utils;

import android.os.StatFs;
import android.util.Log;

import com.nexstreaming.app.assetlibrary.LL;

import java.io.File;

public final class FreeSpaceChecker {
	
	private static final String LOG_TAG = "FreeSpaceChecker";

	private FreeSpaceChecker() {
		
	}
	
	/**
	 * 
	 * @param file path of the device root
	 * @param fileSize byte
	 * @return
	 */
	public static boolean checkReserveDeviceSpace(File path, long fileSize) {
		long freeSpace = getFreeSpace(path);
		
		if(LL.I) Log.i(LOG_TAG, "device free volume : " + freeSpace);
		if(LL.I) Log.i(LOG_TAG, "extraSpaceSize : " + fileSize);
		
		return freeSpace > fileSize;
	}
	
	public static boolean checkReserveDeviceSpace(File path, long fileSize, long extraSpaceSize) {
		return checkReserveDeviceSpace(path, fileSize + extraSpaceSize);
	}
	
	public static long getFreeSpace(File path) {
		
		while( path!=null && !path.exists() )
			path = path.getParentFile();
		if( path==null ) {
			return Long.MAX_VALUE;
		}
		try {
			StatFs stat = new StatFs(path.getAbsolutePath());
			return  (((long)stat.getAvailableBlocks())-1) * ((long)stat.getBlockSize());
		} catch(IllegalArgumentException e) {
			if(LL.E) Log.e(LOG_TAG, "KM-1618 : ", e);
			return Long.MAX_VALUE;
		}
	}
}
