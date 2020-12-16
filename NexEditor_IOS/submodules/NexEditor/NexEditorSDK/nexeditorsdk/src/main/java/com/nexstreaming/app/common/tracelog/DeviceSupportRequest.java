package com.nexstreaming.app.common.tracelog;

import android.content.Context;
import android.os.Build;

import com.nexstreaming.app.common.util.UserInfo;

public class DeviceSupportRequest {

	public int version;				// API version (10000 by default)
	public String app_uuid;			// device_uuid+package_name
	public String app_name;
	public String app_version;
	public String package_name;		// APP package name
	public String app_ucode;		// APP secret code
	public String os;
	public String os_version;
	
	 
	public String build_device;		// Build.DEVICE
	public String build_model;		// Build.MODEL
	public String board_platform;	// ro.board.platform (or if null, Build.HARDWARE)
    public String manufacturer;		// Build.MANUFACTURER
    public int os_api_level;		// Build.VERSION.SDK_INT

	public DeviceSupportRequest(Context context, String appSecretCode) {
		version = 10000;
		app_uuid = UserInfo.getAppUuid(context);
		app_name = UserInfo.getAppName(context);
		app_version = UserInfo.getAppVersionName(context);
		package_name = UserInfo.getAppPackageName(context);
		app_ucode = appSecretCode;
		
		build_device = Build.DEVICE;
		build_model = Build.MODEL;
		manufacturer = Build.MANUFACTURER;
		os_api_level = Build.VERSION.SDK_INT;
		board_platform = UserInfo.getBoardPlatform();/*NexEditor.getSysProperty("ro.board.platform");
		if( board_platform==null || board_platform.trim().length()<1 ) {
			board_platform = Build.HARDWARE;
		}*/
	}

}
