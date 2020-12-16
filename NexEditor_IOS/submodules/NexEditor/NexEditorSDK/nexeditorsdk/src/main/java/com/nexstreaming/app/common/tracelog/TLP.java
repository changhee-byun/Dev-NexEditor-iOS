package com.nexstreaming.app.common.tracelog;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.preference.PreferenceManager;
import android.util.Log;

import com.google.gson_nex.Gson;
import com.google.gson_nex.JsonSyntaxException;
import com.nexstreaming.app.common.task.ResultTask;
import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.LL;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URL;
import java.util.WeakHashMap;

abstract class TLP<TREQUEST,TRESPONSE extends TLP.BaseResponse> {
	
	private static final String LOG_TAG = "TLP";
	private final Class<TRESPONSE> responseClass;
	private boolean mConsolidateRequests = false;
	private boolean mCanCache = false;
	private Context mContext = null;
	private TRESPONSE mCachedResponse = null;
	private long mCacheExpiration = -1;
	private long mCacheRefresh = -1;
	private long mLastUpdate = -1;
	private boolean mCheckedForCachedResponse = false;
	ResultTask<TRESPONSE> mExistingTask = null;
	public interface BaseResponse {
		public int getResult();
	}
	
	TLP( Class<TRESPONSE> responseClass) {
		this.responseClass = responseClass;
	}
	
	private static class TaskParams<T> {
		URI[] servers;
		T request;
		private TaskParams(URI[] servers, T request) {
			this.servers = servers;
			this.request = request;
		}
	}
	
	public static class TaskIOError extends Task.SimpleTaskError {
		public TaskIOError(Exception e, String message) {
			super(e, message);
		}
		@Override
		public String toString() {
			if( getException()!=null ) {
				return "<TaskIOError: " + getException().getClass().getName() + ">";

			} else {
				return "<TaskIOError: " + getMessage() + ">";
			}
		}
	}

	public static class TaskParseError extends Task.SimpleTaskError {
		public TaskParseError(Exception e, String message) {
			super(e, message);
		}
		@Override
		public String toString() {
			if( getException()!=null ) {
				return "<TaskParseError: " + getException().getClass().getName() + ">";

			} else {
				return "<TaskParseError: " + getMessage() + ">";
			}
		}
	}

	public ResultTask<TRESPONSE> execute( URI[] servers, TREQUEST request ) {
		ResultTask<TRESPONSE> cachedResultTask = null;
		boolean refresh = false;
		if( mCanCache ) {
			TRESPONSE cachedResponse = getCachedResponse();
			long cacheExpiration = getCachedResponseExpiration();
			long cacheRefresh = getCachedResponseRefresh();
			long now = System.currentTimeMillis();
			// TODO: cacheRefresh
			if( cachedResponse!=null && cacheExpiration > 0 && now < cacheExpiration ) {
				onCachedResponseAvailable(cachedResponse);
				refresh = true;
				cachedResultTask =  ResultTask.completedResultTask(cachedResponse);
				if( cacheRefresh>0 && now < cacheRefresh ) {
					return cachedResultTask;
				}
			}
		}
		if( mConsolidateRequests ) {
			if( mExistingTask!=null && mExistingTask.isRunning() )
				return mExistingTask;
		}
		final ResultTask<TRESPONSE> task = refresh?null:new ResultTask<TRESPONSE>();
		if( mConsolidateRequests ) {
			mExistingTask = task;
		}
		new AsyncTask<TaskParams<?>, Void, TRESPONSE>(){
			
			Task.TaskError failureReason = null;
			URI sourceUri = null;
			
			@Override
			protected TRESPONSE doInBackground(TaskParams<?>... params) {
				
				TRESPONSE response = null;
				Gson gson = new Gson();
				String request_json_text = gson.toJson(params[0].request);
				if( LL.I ) Log.i(LOG_TAG,"Send request : " + request_json_text);
				byte[] request_json;
				try {
					request_json = request_json_text.getBytes("UTF-8");
				} catch (UnsupportedEncodingException e) {
					throw new RuntimeException(e); // Android always supports UTF-8 encoding; should never get this error.
				}
				for( URI server: params[0].servers ) {
					URL serverURL;
					try {
						serverURL = server.toURL();
					} catch (MalformedURLException e) {
						if( LL.I ) Log.i(LOG_TAG,"Bad URL; skipping : " + server,e);
						continue;
					}
					if( LL.D ) Log.d(LOG_TAG,"Trying : " + server);
					try {
						HttpURLConnection connection = (HttpURLConnection)(serverURL.openConnection());
						try {
							
							connection.setRequestProperty("Content-Type", "application/json");
							connection.setDoOutput(true);
							OutputStream out = connection.getOutputStream();
							out.write(request_json);
							out.close();
							
							InputStream is = new BufferedInputStream(connection.getInputStream());
							StringBuilder readdata = new StringBuilder();
							byte[] buffer = new byte[8192];
							int len;
							while( (len=is.read(buffer)) != -1 ) {
								readdata.append( new String(buffer,0,len,"UTF-8") );
							}
							String data = readdata.toString();
							if( LL.D ) Log.d(LOG_TAG,"Response : " + data);
							response = gson.fromJson(data,responseClass);
							sourceUri = server;
							
						} finally {
							connection.disconnect();
						}
					} catch (IOException e) {
						if( LL.I ) Log.i(LOG_TAG,"Failed reading data from server : " + server,e);
						failureReason = new TaskIOError(e, "Failed reading data from server");
					} catch (JsonSyntaxException e) {
						if( LL.I ) Log.i(LOG_TAG,"Failed parsing data from server : " + server,e);
						failureReason = new TaskParseError(e,"Failed parsing data from server");
					}
					
				}
				
				return response;
			}
			
			protected void onPostExecute(TRESPONSE result) {
				
				if( result==null) {
					onFailure(failureReason, null);
					if( task!=null )
						task.sendFailure(failureReason);
					return;
				}
				int resultCode = result.getResult();
				ResponseCode responseCode = ResponseCode.fromValue(resultCode);
				
				if( !isValidResponse(responseCode) ) {
					onFailure(responseCode, result);
					if( task!=null )
						task.sendFailure(responseCode);
				} else {
					onResponseAvailable(result);
					String originalSrc = "?";
					if( sourceUri!=null ) {
						originalSrc = sourceUri.getHost();
					}
					if( mCachedResponse==result ) {
						sResponseInfo.put(result, new TLPResponseInfo(false,mCacheExpiration,mCacheRefresh,System.currentTimeMillis(),originalSrc));
					} else {
						sResponseInfo.put(result, new TLPResponseInfo(false,-1,-1,System.currentTimeMillis(),originalSrc));
					}
					if( task!=null )
						task.sendResult(result);
				}
			};
			
			
		}.executeOnExecutor(EditorGlobal.NETWORK_OPERATION_THREAD_POOL, new TaskParams<TREQUEST>(servers,request));
		
		return refresh?cachedResultTask:task;
	}

	protected abstract boolean isValidResponse( ResponseCode code );
	
	public void setCanCache( boolean canCache, Context ctx ) {
		if( ctx==null ) {
			canCache = false;
			return;
		}
		mContext = ctx.getApplicationContext();
		mCanCache = canCache;
		checkForCachedResponse();
	}
	
	public void clearCachedResponse() {
		if( !mCanCache  )
			return;
		if( mContext==null )
			return;
		
		mCachedResponse = null;
		mCacheExpiration = -1;
		mCheckedForCachedResponse = false;
		
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
		prefs.edit().remove(getPrefDataName()).remove(getPrefExpirationName()).remove(getPrefRefreshName()).apply();
		
	}
	
	public TRESPONSE getCachedResponse() {
		if( !mCanCache  )
			return null;
		if( mContext==null )
			return null;
		checkForCachedResponse();
		return mCachedResponse;
	}
	
	private static WeakHashMap<Object, TLPResponseInfo> sResponseInfo = new WeakHashMap<Object, TLPResponseInfo>();
	
	public static class TLPResponseInfo {
		public final boolean fromCache;
		public final long cacheExpiration;
		public final long cacheRefresh;
		public final long dataReceived;
		public final String originalSrc;
		TLPResponseInfo(boolean fromCache,long cacheExpiration, long cacheRefresh, long dataReceived, String originalSrc ) {
			this.fromCache = fromCache;
			this.cacheExpiration = cacheExpiration;
			this.cacheRefresh = cacheRefresh;
			this.dataReceived = dataReceived;
			this.originalSrc = originalSrc;
		}
	}
	
	public static TLPResponseInfo getResponseInfo( Object object ) {
		return sResponseInfo.get(object);
	}
	
	private void checkForCachedResponse() {
		if( mCheckedForCachedResponse )
			return;
		if( !mCanCache  )
			return;
		if( mContext==null )
			return;
		
		mCheckedForCachedResponse = true;
		
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);

		String dataPref = prefs.getString(getPrefDataName(), "");
		long expirePref = prefs.getLong(getPrefExpirationName(), -1);
		long refreshPref = prefs.getLong(getPrefRefreshName(), -1);
		long lastUpdatePref = prefs.getLong(getPrefLastUpdateName(), -1);
		String originalSrcPref = prefs.getString(getPrefCacheSrcName(), "?");
		
		if( expirePref>0 && dataPref!=null && dataPref.length()>0 ) {
			Gson gson = new Gson();
			TRESPONSE response = null;
			try {
				response = gson.fromJson(dataPref, responseClass);
			} catch (JsonSyntaxException e) {
				return;
			}
			if( response!=null ) {
				sResponseInfo.put(response, new TLPResponseInfo(true, expirePref, refreshPref, lastUpdatePref, originalSrcPref));
				mCachedResponse = response;
				mCacheExpiration = expirePref;
				mCacheRefresh = refreshPref;
				mLastUpdate = lastUpdatePref;
			}
		}
		
	}
	
	public long getCachedResponseExpiration() {
		if( !mCanCache  )
			return -1;
		if( mContext==null )
			return -1;
		checkForCachedResponse();
		return mCacheExpiration;
	}
	
	public long getCachedResponseRefresh() {
		if( !mCanCache  )
			return -1;
		if( mContext==null )
			return -1;
		checkForCachedResponse();
		return mCacheRefresh;
	}
	
	public void cacheResponse(TRESPONSE response, long refresh, long expiration) {
		if( !mCanCache  )
			return;
		if( mContext==null )
			return;
		
		mCachedResponse = response;
		mCacheExpiration = expiration;
		mCacheRefresh = refresh;
		mCheckedForCachedResponse = true;
		String originalSrc = "";
		
		TLPResponseInfo rinfo = getResponseInfo(response);
		if( rinfo!=null ) {
			originalSrc = rinfo.originalSrc;
		}
		
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
		
		Gson gson = new Gson();
		String cache_data = gson.toJson(response);
		
		prefs.edit()
			.putString(getPrefDataName(), cache_data)
			.putLong(getPrefExpirationName(), expiration)
			.putLong(getPrefRefreshName(), refresh)
			.putLong(getPrefLastUpdateName(), System.currentTimeMillis())
			.putString(getPrefCacheSrcName(), originalSrc)
			.apply();
	}
	
	private String getPrefDataName() {
		return "tlp_cached_data_" + responseClass.getName();
	}
	
	private String getPrefExpirationName() {
		return "tlp_cached_exp_" + responseClass.getName();
	}
	
	private String getPrefRefreshName() {
		return "tlp_cached_refresh_" + responseClass.getName();
	}
	
	private String getPrefLastUpdateName() {
		return "tlp_cached_lastupdate_" + responseClass.getName();
	}
	
	private String getPrefCacheSrcName() {
		return "tlp_cached_cachesrc_" + responseClass.getName();
	}
	
	public void setConsoldateRequests(boolean consolidateRequests) {
		mConsolidateRequests = consolidateRequests;
	}
	
	protected void onResponseAvailable(TRESPONSE result) {
	}
	
	protected void onCachedResponseAvailable(TRESPONSE result) {
	}
	
	protected void onFailure(Task.TaskError failureReason, TRESPONSE result) {
	}
	
}
