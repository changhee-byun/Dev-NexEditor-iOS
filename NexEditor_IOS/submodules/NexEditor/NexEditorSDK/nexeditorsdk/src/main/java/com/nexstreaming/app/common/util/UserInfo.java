package com.nexstreaming.app.common.util;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Calendar;
import java.util.Locale;
import java.util.TimeZone;
import java.util.UUID;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;

public class UserInfo {

	private final static String LOG_TAG = "[UserInfo]";
	static public final String KEY_APPID = "nex_tl_app_id";
	
	private UserInfo() {
		// Private empty constructor to prevent making instances of this class
	}
	
	// TODO: Unused function? --Matthew
	public static String getUserInfo( Context ctx) {
		String userInfo = getDeviceId(ctx) + getPhoneNum(ctx) + getSubscriberId(ctx);
		return userInfo;
	}

	public static String getAccountInfo( Context ctx ) {
		// TODO: Memory leak! Account manager keeps the context!  --MATTHEW
		AccountManager accountmanager = AccountManager.get(ctx
				.getApplicationContext());
		// TODO: Security issue.  This method requires the caller to hold the permission GET_ACCOUNTS. --MATTHEW
		Account[] accountlist = accountmanager.getAccountsByType("com.google");
		if (accountlist.length > 0) {
			for (int i = 0; i < accountlist.length; i++) {
				if(LL.D) Log.d(LOG_TAG, "Account : " + i + "--> name : "
						+ accountlist[i].name + " type : "
						+ accountlist[i].type);
			}

			return accountlist[0].name;
		} else {
			Account[] accountlist2 = accountmanager.getAccounts();
			if (accountlist2.length > 0) {
				return accountlist2[0].name;
			}
		}

		return null;
	}
	
	private static String makeMD5(String deviceId)
	{
		String MD5 = ""; 
		try{
			MessageDigest md = MessageDigest.getInstance("MD5"); 
			md.update(deviceId.getBytes()); 
			byte byteData[] = md.digest();
			StringBuffer sb = new StringBuffer(); 
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

	public static String getDeviceId( Context ctx ) {
		
		
		String deviceUUID = ""; 
		int osVersionCode = getOsVersionCode(); 
		if(osVersionCode >= android.os.Build.VERSION_CODES.GINGERBREAD)
		{
			deviceUUID = android.provider.Settings.Secure.getString(ctx.getContentResolver(), android.provider.Settings.Secure.ANDROID_ID); 
		}
		else
		{
			// TODO: Is this correct to be random?  --Matthew
			deviceUUID = UUID.randomUUID().toString(); 
		}
		
		return deviceUUID; 
	}
	
	
	public static String getAppUuid( Context ctx ) {
		
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(ctx);
		String userId = prefs.getString(KEY_APPID, null);
		if (userId != null) {
			return userId;
		}
		
		String deviceId = null;
		String deviceUUID = ""; 
		String packageName = ""; 
		int osVersionCode = getOsVersionCode(); 
		
		if(osVersionCode >= android.os.Build.VERSION_CODES.GINGERBREAD)
		{
			deviceUUID = android.provider.Settings.Secure.getString(ctx.getContentResolver(), android.provider.Settings.Secure.ANDROID_ID); 
		}
		else
		{
			deviceUUID = UUID.randomUUID().toString(); 
		}
		
		packageName = getAppPackageName(ctx); 
		deviceId = makeMD5(deviceUUID + packageName); 
		
		if(deviceId != null)
		{
			if(LL.D) Log.d(LOG_TAG, "deviceId length : " + deviceId.length());
			if(LL.D) Log.d(LOG_TAG, "deviceId : " + deviceId); 
			prefs.edit().putString(KEY_APPID, deviceId).commit();
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
	
	public static String getPhoneNum( Context ctx ) {
		// TODO:  Requires permission READ_PHONE_STATE --Matthew
		TelephonyManager telephonytmanager = (TelephonyManager) ctx
				.getSystemService(Context.TELEPHONY_SERVICE);
		String phoneNum = telephonytmanager.getLine1Number();
		if(LL.D) Log.d(LOG_TAG, "Phone No. : " + phoneNum);

		return phoneNum;
	}

	public static String getSubscriberId( Context ctx ) {
		TelephonyManager telephonytmanager = (TelephonyManager) ctx
				.getSystemService(Context.TELEPHONY_SERVICE);
		String subscriberId = telephonytmanager.getSubscriberId();
		if(LL.D) Log.d(LOG_TAG, "Subscriber : " + subscriberId);

		return subscriberId;
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

	public static String getAppVersionCode( Context ctx ) {

		String versionCode = "";

		try {
			PackageInfo packageInfo = ctx.getPackageManager().getPackageInfo(
					ctx.getPackageName(), 0);
			versionCode = String.valueOf(packageInfo.versionCode);
		} catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		// setTitle(mTitle);
		// R:56 G:5 B:95
		// setTitleColor(0xFF38055F);

		if(LL.E) Log.e(LOG_TAG, "Version Information ===== ");
		if(LL.E) Log.e(LOG_TAG, "version code : " + versionCode);

		return versionCode;
	}

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

	public static long getCreationDate() {
		return Long.valueOf(System.currentTimeMillis());
	}

	public static long getLocalTime() {
		return Long.valueOf(System.currentTimeMillis());
	}

	public static String getTimeZone( Context ctx ) {
		TimeZone tz;
		try {
			Configuration userConfig = new Configuration();
			Settings.System.getConfiguration(ctx.getContentResolver(),
					userConfig);
			Calendar calendar = Calendar.getInstance(userConfig.locale);
			tz = calendar.getTimeZone();
		} catch (Exception e) {
			tz = TimeZone.getDefault();
		}

		String strTimeZone = tz.getDisplayName();
		if(LL.D) Log.d(LOG_TAG, "TimeZone = " + strTimeZone);

		return strTimeZone;
	}

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

	public static String getOsType() {
		return "Android";
	}

	public static String getOsVersion() {
		return android.os.Build.VERSION.RELEASE;
	}
	
	public static String getOsBuildNum() {
		return android.os.Build.DISPLAY;
	}

	public static int getOsVersionCode()
	{
		return android.os.Build.VERSION.SDK_INT; 
	}
	
	public static String getDeviceModel() {
		return android.os.Build.MODEL;
	}

	public static String getDeviceKernelVersion() {
		if(LL.D) Log.d(LOG_TAG, "kernel:" + System.getProperty("os.version"));
		return System.getProperty("os.version");
	}

	public static String getDeviceMaker() {
		return android.os.Build.MANUFACTURER;
	}
	
	public static String getBoardPlatform() {
		String board_plat = NexEditor.getSysProperty("ro.board.platform");
		if( board_plat!=null && board_plat.trim().equalsIgnoreCase("msm8960")) {
			String product_board = NexEditor.getSysProperty("ro.product.board");
			if( product_board!=null && product_board.trim().equalsIgnoreCase("8x30")) {
				board_plat = "MSM8x30n";
			}
		}
		// for universal7420 galaxy s6 device.
		if( board_plat!=null && board_plat.equalsIgnoreCase("exynos5") ) {
			String product_board = NexEditor.getSysProperty("ro.product.board");
			if( product_board!=null && product_board.equalsIgnoreCase("universal7420") )
			{
				board_plat = product_board;
			}
		}

		if( board_plat==null || board_plat.trim().length()<1 ) {
			board_plat = NexEditor.getSysProperty("ro.mediatek.platform");
		}
		if( board_plat==null || board_plat.trim().length()<1 ) {
			board_plat = NexEditor.getSysProperty("ro.hardware");
		}
		if( board_plat==null || board_plat.trim().length()<1 ) {
			board_plat = "unknown";
		}
		
		return board_plat;
	}
	
	public static String getNetworkType() {
		/*int netType = NexNetworkUtil.checkNetworkType(mCtx);
		if (netType == NexNetworkUtil.NETWORK_3G) {
			return "3G";
		} else if (netType == NexNetworkUtil.NETWORK_LTE) {
			return "LTE";
		} else if (netType == NexNetworkUtil.NETWORK_NONE) {
			return "NONE";
		} else if (netType == NexNetworkUtil.NETWORK_WIFI) {
			return "wifi";
		}*/

		return null;
	}

	public static String getCountry( Context ctx ) {
		Locale locale = ctx.getResources().getConfiguration().locale;
		String country = locale.getCountry().toUpperCase(Locale.US);

		return country;
	}

	public static String getCountryFromSim( Context ctx ) {

		String country = null;

		TelephonyManager tm = (TelephonyManager) ctx
				.getSystemService(Context.TELEPHONY_SERVICE);
		country = tm.getSimCountryIso();

		if (country != null && country.length() > 0) {
			if(LL.D) Log.d(LOG_TAG, "simCountry:" + country);
			return country.toUpperCase(Locale.US);
		} else {
			country = tm.getNetworkCountryIso();
			if (country != null && country.length() > 0) {
				if(LL.D) Log.d(LOG_TAG, "netCountry:" + country);
				return country.toUpperCase(Locale.US);
			} else {
				country = getCountry(ctx);
				if (country != null && country.length() > 0) {
					return country.toUpperCase(Locale.US);
				}
			}
		}

		return null;
	}
	
	
}
