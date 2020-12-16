package com.nexstreaming.app.common.util;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;

public class NexNetworkUtil {

	public static final int NETWORK_NONE = -1;
	public static final int NETWORK_WIFI = 0;
	public static final int NETWORK_3G = 1;
	public static final int NETWORK_LTE = 2;

	public static int checkNetworkType(Context context) {
		int ret = NETWORK_NONE;
		ConnectivityManager cm = (ConnectivityManager) context
				.getSystemService(Context.CONNECTIVITY_SERVICE);

		TelephonyManager tm = (TelephonyManager) context
				.getSystemService(Context.TELEPHONY_SERVICE);

		NetworkInfo activeNetwork = cm.getActiveNetworkInfo();

		if (activeNetwork != null && activeNetwork.isAvailable() == true) {

			switch (activeNetwork.getType()) {
			case (ConnectivityManager.TYPE_WIFI):
				ret = NETWORK_WIFI;
				break;
			case (ConnectivityManager.TYPE_MOBILE): {
				switch (tm.getNetworkType()) {
				case (TelephonyManager.NETWORK_TYPE_LTE | TelephonyManager.NETWORK_TYPE_HSPAP):
					ret = NETWORK_LTE;
					break;
				/*
				 * case (TelephonyManager.NETWORK_TYPE_EDGE |
				 * TelephonyManager.NETWORK_TYPE_GPRS): PrefetchCacheSize /= 2;
				 * break;
				 */
				case (TelephonyManager.NETWORK_TYPE_UNKNOWN):
					ret = NETWORK_NONE;
					break;

				default:
					ret = NETWORK_3G;
					break;
				}
				break;
			}
			default:
				break;
			}
		}

		if(LL.D) Log.d("CheckNetwork", "Type:" + ret);
		return ret;
	}
}
