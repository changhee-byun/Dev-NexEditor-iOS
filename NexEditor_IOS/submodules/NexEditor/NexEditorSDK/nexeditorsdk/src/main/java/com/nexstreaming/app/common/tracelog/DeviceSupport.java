package com.nexstreaming.app.common.tracelog;

import android.content.Context;
import android.util.Log;

import com.nexstreaming.app.common.task.Task.TaskError;
import com.nexstreaming.kminternal.kinemaster.config.KMConfigFile;
import com.nexstreaming.kminternal.kinemaster.config.LL;

public class DeviceSupport extends TLP<DeviceSupportRequest, DeviceSupportResponse> {


	private static final String	LOG_TAG	= "TLP_DeviceSupport";
	private static DeviceSupport instance;
	public static DeviceSupport getInstance(Context ctx) {
		if( instance==null ) {
			instance = new DeviceSupport();
			instance.setConsoldateRequests(true);
			if( KMConfigFile.getInstance().getDeviceSupportServerCache(true)) {
				instance.setCanCache(true,ctx);
			}
		}
		return instance;
	}

	private DeviceSupport() {
		super(DeviceSupportResponse.class);
	}

	@Override
	protected boolean isValidResponse(ResponseCode code) {
		switch(code) {
			case SUCCESS:
			case NO_MATCH:
				return true;
			default:
				return false;
		}
	}
	
	@Override
	protected void onResponseAvailable(DeviceSupportResponse result) {
		super.onResponseAvailable(result);
		if( LL.D ) Log.d(LOG_TAG,"onResponseAvailable : " + result.toString());
		cacheResponse(result, System.currentTimeMillis() + (result.next*86400000), Long.MAX_VALUE);
	}

	@Override
	protected void onCachedResponseAvailable(DeviceSupportResponse result) {
		super.onCachedResponseAvailable(result);
		if( LL.D ) Log.d(LOG_TAG,"onCachedResponseAvailable : " + result.toString());
	}
	
	@Override
	protected void onFailure(TaskError failureReason, DeviceSupportResponse result) {
		super.onFailure(failureReason, result);
		if( LL.D ) Log.d(LOG_TAG,"onFailure : " + failureReason + " result=" + String.valueOf(result));
	}
	
}
