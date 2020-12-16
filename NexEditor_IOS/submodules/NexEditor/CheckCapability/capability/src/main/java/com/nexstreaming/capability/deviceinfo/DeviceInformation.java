package com.nexstreaming.capability.deviceinfo;

import android.os.Build;

import com.nexstreaming.capability.util.CBLog;

import java.lang.reflect.Method;

public class DeviceInformation{

	private String AliasName;
	private String ModelName;
	private String Manufacture;
	private String Chipset;
	private String OSVersion;

	public void init()
	{
		ModelName = Build.MODEL;
		Manufacture = Build.MANUFACTURER;
		Chipset = searchChipset();
		OSVersion = Integer.toString(android.os.Build.VERSION.SDK_INT);

		CBLog.d("##ModelName: " + ModelName);
		CBLog.d("##Manufacture: " + Manufacture);
		CBLog.d("##Chipset: " + Chipset);
		CBLog.d("##OSVersion: " + OSVersion);
	}

	public String getAliasName()
	{
		return AliasName;
	}

	public String getModelName()
	{
		return ModelName;
	}

	public String getManufacture()
	{
		return Manufacture;
	}

	public String getChipset()
	{
		return Chipset;
	}

	public String getOSVersion()
	{
		return OSVersion;
	}

	private String searchChipset()
	{
		String chipSet = "";
		String temp = "";

		try {
			Class<?> c = Class.forName("android.os.SystemProperties");
			Method get = c.getMethod("get", String.class);
			chipSet = (String)get.invoke(c, "ro.board.platform");

			if (chipSet == null || chipSet.length() <= 0) {
				temp = (String)get.invoke(c, "ro.mediatek.platform");
				chipSet = (temp != null && temp.length() > 0) ? ", " + temp : "";
			}
		}
		catch (Exception ignored) {
		}
		return chipSet;
	}
}
