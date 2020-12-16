package com.nexstreaming.app.assetlibrary.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.preference.PreferenceManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.nexstreaming.app.assetlibrary.LL;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Locale;

public class UserInfo {

	private final static String LOG_TAG = "[UserInfo]";
	static public final String KEY_APPID = "nex_tl_app_id";
	
	private UserInfo() {
		// Private empty constructor to prevent making instances of this class
	}
	
	private static String makeMD5(String deviceId)
	{
		String MD5 = ""; 
		try{
			MessageDigest md = MessageDigest.getInstance("MD5"); 
			md.update(deviceId.getBytes()); 
			byte byteData[] = md.digest();
			StringBuilder sb = new StringBuilder();
			for(int i = 0 ; i < byteData.length ; i++){
				sb.append(Integer.toString((byteData[i]&0xff) + 0x100, 16).substring(1));
			}
			MD5 = sb.toString();
			
		}catch(NoSuchAlgorithmException e){
			e.printStackTrace(); 
			MD5 = null; 
		}
		return MD5;		
	}

	public static String getAppUuid( Context ctx ) {
		
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(ctx);
		String userId = prefs.getString(KEY_APPID, null);
		if (userId != null) {
			return userId;
		}
		
		String deviceUUID;

		deviceUUID = android.provider.Settings.Secure.getString(ctx.getContentResolver(), android.provider.Settings.Secure.ANDROID_ID);
		String deviceId = makeMD5(deviceUUID +  getAppPackageName(ctx));
		
		if(deviceId != null)
		{
			if(LL.D) Log.d(LOG_TAG, "deviceId length : " + deviceId.length());
			if(LL.D) Log.d(LOG_TAG, "deviceId : " + deviceId); 
			prefs.edit().putString(KEY_APPID, deviceId).apply();
		}
		return deviceId; 
	}

	public static String getPhoneType( Context ctx )
	{
		String phoneType = "0"; 
		
		TelephonyManager telephonymanager = (TelephonyManager) ctx
				.getSystemService(Context.TELEPHONY_SERVICE);
		
		int type = telephonymanager.getPhoneType(); 
		// TODO: Upwards compatibility issue -- other phone types could be added (better to check != NONE???) --MATTHEW
		if(type == TelephonyManager.PHONE_TYPE_CDMA || type == TelephonyManager.PHONE_TYPE_GSM)
		{
			phoneType = "0"; 
		}
		else
		{
			phoneType = "1";
		}
		
		return phoneType; 
	}
	
	public static String getAppName( Context ctx ) {
		PackageManager pm = ctx.getPackageManager();
		ApplicationInfo ai;
		try {
		    ai = pm.getApplicationInfo( ctx.getPackageName(), 0);
		} catch (final NameNotFoundException e) {
		    ai = null;
		}
		String applicationName = (String) (ai != null ? pm.getApplicationLabel(ai) : "(unknown)");
		if(LL.D) Log.d(LOG_TAG, "appName : " + applicationName);
		return applicationName; 
	}

	public static String getAppPackageName( Context ctx ) {
		String packageName = "";

		try {
			PackageInfo packageInfo = ctx.getPackageManager().getPackageInfo(
					ctx.getPackageName(), 0);
			packageName = packageInfo.packageName;
		} catch (NameNotFoundException e) {
			e.printStackTrace();
		}

		if(LL.D) Log.d(LOG_TAG, "package : " + packageName);
		return packageName;
	}

//	public static String getAppVersionCode( Context ctx ) {
//
//		String versionCode = "";
//
//		try {
//			PackageInfo packageInfo = ctx.getPackageManager().getPackageInfo(
//					ctx.getPackageName(), 0);
//			versionCode = String.valueOf(packageInfo.versionCode);
//		} catch (NameNotFoundException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
//
//		// setTitle(mTitle);
//		// R:56 G:5 B:95
//		// setTitleColor(0xFF38055F);
//
//		if(LL.E) Log.e(LOG_TAG, "Version Information ===== ");
//		if(LL.E) Log.e(LOG_TAG, "version code : " + versionCode);
//
//		return versionCode;
//	}

	public static String getAppVersionName( Context ctx ) {

		String versionName = "";

		try {
			PackageInfo packageInfo = ctx.getPackageManager().getPackageInfo(
					ctx.getPackageName(), 0);
			versionName = packageInfo.versionName;
		} catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		// setTitle(mTitle);
		// R:56 G:5 B:95
		// setTitleColor(0xFF38055F);

		if(LL.E) Log.e(LOG_TAG, "Version Information ===== ");
		if(LL.E) Log.e(LOG_TAG, "version name : " + versionName);

		return versionName;
	}

//	public static long getCreationDate() {
//		return Long.valueOf(System.currentTimeMillis());
//	}
//
//	public static long getLocalTime() {
//		return Long.valueOf(System.currentTimeMillis());
//	}
//
//	public static String getTimeZone( Context ctx ) {
//		TimeZone tz;
//		try {
//			Configuration userConfig = new Configuration();
//			Settings.System.getConfiguration(ctx.getContentResolver(),
//					userConfig);
//			Calendar calendar = Calendar.getInstance(userConfig.locale);
//			tz = calendar.getTimeZone();
//		} catch (Exception e) {
//			tz = TimeZone.getDefault();
//		}
//
//		String strTimeZone = tz.getDisplayName();
//		if(LL.D) Log.d(LOG_TAG, "TimeZone = " + strTimeZone);
//
//		return strTimeZone;
//	}

	public static String getLocale() {

		Locale locale = Locale.getDefault();
		String lang = locale.getLanguage().toLowerCase(Locale.US);
		String country = locale.getCountry().toLowerCase(Locale.US);
		String variant = locale.getVariant().toLowerCase(Locale.US);
		String useLocale = "";
		if( variant.isEmpty() && country.isEmpty() ) {
			useLocale = lang;
		} else if( variant.isEmpty() ) {
			useLocale = lang + "-" + country;
		} else if( country.isEmpty() ) {
			useLocale = lang + "-" + variant;
		} else {
			useLocale = lang + "-" + country + "-" + variant;
		}
		
		return useLocale;

		//return Locale.getDefault().getLanguage();
	}

//	public static String getOsType() {
//		return "Android";
//	}

	public static String getOsVersion() {
		return android.os.Build.VERSION.RELEASE;
	}
	
	public static String getOsBuildNum() {
		return android.os.Build.DISPLAY;
	}

//	public static int getOsVersionCode()
//	{
//		return android.os.Build.VERSION.SDK_INT;
//	}
	
	public static String getDeviceModel() {
		return android.os.Build.MODEL;
	}

//	public static String getDeviceKernelVersion() {
//		if(LL.D) Log.d(LOG_TAG, "kernel:" + System.getProperty("os.version"));
//		return System.getProperty("os.version");
//	}

	public static String getDeviceMaker() {
		return android.os.Build.MANUFACTURER;
	}

}
