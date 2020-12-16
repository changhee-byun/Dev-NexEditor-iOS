package com.nexstreaming.kminternal.kinemaster.mediainfo;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;
import java.io.Writer;
import java.lang.ref.WeakReference;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.util.Log;
import android.util.SparseArray;

import com.google.gson_nex.Gson;
import com.google.gson_nex.JsonIOException;
import com.google.gson_nex.JsonSyntaxException;
import com.nexstreaming.app.common.task.ResultTask;
import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.app.common.task.ResultTask.OnResultAvailableListener;
import com.nexstreaming.app.common.task.Task.Event;
import com.nexstreaming.app.common.task.Task.OnFailListener;
import com.nexstreaming.app.common.task.Task.OnTaskEventListener;
import com.nexstreaming.app.common.task.Task.TaskError;
import com.nexstreaming.app.common.util.FileType;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.ExclusionList;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.config.NexEditorDeviceProfile;
import com.nexstreaming.kminternal.nexvideoeditor.NexClipInfo;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor.ErrorCode;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor.OnEditorDestroyedListener;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor.OnGetClipInfoDoneListener;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor.OnIdleListener;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditorType;

/**
 * Provides information on media files supported by the KineMaster SDK, and
 * allows access to thumbnail, audio level and other details about supported
 * media files.<p>
 * 
 * To use this class, call the {@link #getInfo(String, boolean)} factory method. This
 * creates a new MediaInfo instance if necessary, or returns an existing one
 * if one is available.  MediaInfo instances are cached as weak references, so
 * as long as a reference is retained externally, the existing instance will be
 * returned for future request.<p>
 *
 */
public class MediaInfo {

	private static final String LOG_TAG = "MediaInfo";
	private static Map<String,WeakReference<MediaInfo>> s_cache = null;
	private static ExclusionList s_exclusionList;

	private String mMediaFileUrl; //yoon
	private int mMediaUrlType = -1;

	private static final int typeOfMediaUrl_file = 0;
	private static final int typeOfMediaUrl_asset = 1;

	private File mInfoFile;
	private File mSeekPointFile;
	private File mVideoThumbFile;
    private File mVideoLargeStartThumbFile;
    private File mVideoLargeEndThumbFile;
	private File mVideoRawThumbFile;
	private File mAudioPCMFile;
	private File mMediaInfoDir;
	private String mMediaPathHash;
	private NexEditor.ErrorCode mGetClipInfoResult = ErrorCode.NONE;
	private MediaInfoTask<Thumbnails,ThumbnailParams> mThumnbnailTask = null;
	private MediaInfoTask<PCMLevels,Void> mPCMLevelTask = null;
    private ResultTask<Bitmap> mLargeThumbTask = null;
    private ResultTask<Bitmap> mLargeEndThumbTask = null;
	private ResultTask<int[]> mSeekPointsTask = null;
	private AsyncTask<String, Integer, int[]> mSeekPointAsyncTask = null;
	private MediaDesc mMediaDesc;
	private static Executor THUMBNAIL_CONVERTER_EXECUTOR = Executors.newSingleThreadExecutor();
	private static Executor PARSER_EXECUTOR = Executors.newSingleThreadExecutor();

	private boolean mIsExcluded;
	private static File sCacheDir;
	private static File sThumbnailTempDir;

	private boolean mUseCache = true;

	/** if getThumbnail task 
	public boolean isBusy;
	/**
	 * Sets the directory to use for caching media info and generated
	 * thumbnails.<p>
	 * 
	 * Either this method or {@link #setCacheDirFromContext(Context)} must
	 * be called before {@link #getInfo(String) getInfo} can be used.<p>
	 * 
	 * This is useful if you want to expose the cache directory for testing
	 * purposes, but generally it is better to call {@link #setCacheDirFromContext(Context)}
	 * in production code, to use the cache associated with the Android application
	 * context (this hide cache files from the user and other apps to prevent tampering,
	 * and allows the user to clear the cache from system settings).<p> 
	 * 
	 * @param path	The path to use for the cache directory
	 */
	public static void setCacheDir( File path ) {
		sCacheDir = path;
	}

	private static int getUrlType(String path){
		if (path != null) {
			if (path.startsWith("nexasset://")) {
				return typeOfMediaUrl_asset;
			} else {
				return typeOfMediaUrl_file;
			}
		}
		return -1;
	}

	public static void setThumbnailTempDir( File path ) {
		sThumbnailTempDir = path;
	}

	public static void setCacheDirFromContext( Context ctx ) {
		sCacheDir = ctx.getApplicationContext().getCacheDir();
	}
	
	public static MediaInfo getInfo( String path ) {
		if( path==null ) {
			return null;
		}
		//return getInfo(new File(path),true);
		return getInfo(path,true);
	}

	public static MediaInfo getInfo( String path , boolean useCache) {

		if( path==null ) {
			return null;
		}

		if(LL.D) Log.d(LOG_TAG,"getInfo(" + path + ")");

		if( s_cache==null ) {
			if(LL.D) Log.d(LOG_TAG,"getInfo : init cache");
			s_cache = new HashMap<String,WeakReference<MediaInfo>>();
		}

		if( s_exclusionList == null) {
			if(LL.D) Log.d(LOG_TAG,"getInfo : init exclusion list");
			File mediaInfoDir = new File(sCacheDir,".km_mediainfo");
			s_exclusionList = ExclusionList.exclusionListBackedBy(new File(mediaInfoDir,"mediainfo_exclude.dat"));
		}

		String absPath = path;
		int type = getUrlType(path);
		if( type ==  typeOfMediaUrl_file ) {
			absPath = new File(path).getAbsolutePath();
		}

		if( useCache ) { //yoon

			WeakReference<MediaInfo> refCachedInfo = s_cache.get(absPath);
			if (refCachedInfo != null) {
				MediaInfo cachedInfo = refCachedInfo.get();
				if (cachedInfo != null) {
					if (LL.D) Log.d(LOG_TAG, "getInfo(" + path + ") using cache. duration="+cachedInfo.getDuration());
					return cachedInfo;
				}
			}
		}
		if( type ==  typeOfMediaUrl_asset ) {
			useCache = false;
		}
		MediaInfo mediaInfo = new MediaInfo(path,type,useCache);
		//yoon add
		if( mediaInfo.mGetClipInfoResult == NexEditor.ErrorCode.NO_INSTANCE_AVAILABLE ){
			return mediaInfo;
		}

		if (LL.D) Log.d(LOG_TAG, "getInfo(" + path + ") adding to cache duration="+mediaInfo.getDuration());
		s_cache.put(absPath, new WeakReference<MediaInfo>(mediaInfo));

		return mediaInfo;
	}

    public static void checkMissingFile(){
        if(s_cache!=null){
            List<String> toRemove = new ArrayList<>();
            for(Map.Entry<String,WeakReference<MediaInfo>> entry : s_cache.entrySet()){
                String key = entry.getKey();
                File path = key==null?null:new File(key);
                if(path!=null && !path.exists()){
                    toRemove.add(entry.getKey());
                }
            }
            for(String path: toRemove){
                s_cache.remove(path);
            }
        }
    }

    private void readImageInfo() {
		if( mMediaUrlType != typeOfMediaUrl_file ){
			mMediaDesc = new MediaDesc();
			mMediaDesc.version = MEDIADESC_VERSION;
			return;
		}

		File file = new File(mMediaFileUrl);
        BitmapFactory.Options opts = new BitmapFactory.Options();
        opts.inJustDecodeBounds = true;
		BitmapFactory.decodeFile(file.getAbsolutePath(),opts);
        mMediaDesc = new MediaDesc();
        mMediaDesc.version = MEDIADESC_VERSION;
		mMediaDesc.path = file.getAbsolutePath();
        mMediaDesc.hasAudio = false;
        mMediaDesc.hasVideo = false;
        mMediaDesc.hasImage = true;
        mMediaDesc.audioDuration = 0;
        mMediaDesc.videoDuration = 0;
		mMediaDesc.fileSize = file.length();
        mMediaDesc.videoWidth = opts.outWidth;
        mMediaDesc.videoHeight = opts.outHeight;
        mMediaDesc.seekPointCount = 0;
        mMediaDesc.framesPerSecond = 0;
	mMediaDesc.framesPerSecondFloat = 0;
        mMediaDesc.videoH264Profile = 0;
        mMediaDesc.videoH264Level = 0;
        mMediaDesc.VideoH264Interlaced = 0;
		mMediaDesc.videoRenderType = 0;
		mMediaDesc.videoCodecType = 0;
		mMediaDesc.audioCodecType = 0;
		mMediaDesc.videoHDRType = 0;
        mIsExcluded = false;
        mGetClipInfoResult = ErrorCode.NONE;
    }

	private long getLength(){
		if( mMediaUrlType == typeOfMediaUrl_file ){
			return new File(mMediaFileUrl).length();
		}else if( mMediaUrlType == typeOfMediaUrl_asset) {
			return Integer.parseInt(mMediaFileUrl.substring(mMediaFileUrl.lastIndexOf(':')+1) );
		}
		return 0;
	}

	private MediaInfo(String path, int urlType, boolean useCache) {

		mMediaFileUrl = path;
		mMediaUrlType = urlType;

		if( urlType == typeOfMediaUrl_file ) {
			FileType fileType = FileType.fromFile(path);

			if (fileType != null && fileType.isImage() /*EditorGlobal.isImage(path.getName())*/) {
				if( fileType.isSupportedFormat() ) {
					readImageInfo();
					return;
				}else{
					mIsExcluded = true;
					return;
				}
			}
		}

		File mediaInfoDir = new File(sCacheDir,".km_mediainfo");
		mediaInfoDir.mkdirs();
		String pathHash = String.format(Locale.US, "none_%08X", path.hashCode());
		if( urlType == typeOfMediaUrl_file ) {
			pathHash = makePathHash(new File(path));
		}else if( urlType == typeOfMediaUrl_asset) {
			pathHash = makePathHash(path);
		}
		if(LL.D) Log.d(LOG_TAG,"mediaInfoDir=" + mediaInfoDir);

		mInfoFile = new File(mediaInfoDir, pathHash + "_info.dat");
		mSeekPointFile = new File(mediaInfoDir, pathHash + "_seek.dat");
		mVideoThumbFile = new File(mediaInfoDir, pathHash + "_vthumb.dat");
		mVideoLargeStartThumbFile = new File(mediaInfoDir, pathHash + "_vthumb_large.dat");
		mVideoLargeEndThumbFile = new File(mediaInfoDir, pathHash + "_vthumb_large_end.dat");
		mVideoRawThumbFile = new File(mediaInfoDir, pathHash + "_vthumb_raw.dat");
		mAudioPCMFile = new File(mediaInfoDir, pathHash + "_pcm.dat");
		mMediaInfoDir = mediaInfoDir;
		mMediaPathHash = pathHash;
		mUseCache = useCache; //yoon

		Gson gson = null;
		if( useCache ) {
			if (mInfoFile.exists()) {
				if (LL.D)
					Log.d(LOG_TAG, "getInfo(" + path + ") info file exists -> attemptng to read");
				if (gson == null) gson = new Gson();
				MediaDesc mediaDesc = null;
				try {
					Reader reader = new BufferedReader(new FileReader(mInfoFile));
					try {
						mediaDesc = gson.fromJson(reader, MediaDesc.class);
					} finally {
						reader.close();
					}
				} catch (IOException e) {
					// These exceptions are caught and ignored intentionally;
					// if the file cannot be read, or if the file is damaged
					// or of an unrecognized version, we can just re-create
					// it from the original.
				} catch (JsonSyntaxException e) {
				} catch (JsonIOException e) {
				}
				if (mediaDesc != null && mediaDesc.version == MEDIADESC_VERSION) {
					if (LL.D) Log.d(LOG_TAG, "getInfo(" + path + ") info from disk cache");
					mMediaDesc = mediaDesc;
					return;
				}
			}
		}

		if( s_exclusionList!=null && s_exclusionList.isExcluded(pathHash) ) {
			if(LL.D) Log.d(LOG_TAG,"getInfo(" + path + ") skip due to exclusion");
			mIsExcluded = true;
			return;
		}

		NexClipInfo clipInfo = new NexClipInfo();

		NexEditor editor = getEditor();
		if( editor==null ) {
			mGetClipInfoResult = NexEditor.ErrorCode.NO_INSTANCE_AVAILABLE;
			if(LL.D) Log.d(LOG_TAG,"getInfo(" + path + ") failure:" + mGetClipInfoResult);
			return;
		}

		s_exclusionList.add(pathHash);
		if(LL.D) Log.d(LOG_TAG,"getInfo(" + path + ") call editor for info");
		mGetClipInfoResult = editor.getClipInfoSync(path, clipInfo, false, 0);
		if(LL.D) Log.d(LOG_TAG,"getInfo(" + path + ") returned from editor");
		s_exclusionList.remove(pathHash);

		mMediaDesc = new MediaDesc();

		if( mGetClipInfoResult != NexEditor.ErrorCode.NONE ) {
			if(LL.D) Log.d(LOG_TAG,"getInfo(" + path + ") failure:" + mGetClipInfoResult);
			return;
		}

		mMediaDesc.version = MEDIADESC_VERSION;
		mMediaDesc.path = path;
		mMediaDesc.hasAudio = (clipInfo.mExistAudio!=0);
		mMediaDesc.hasVideo = (clipInfo.mExistVideo!=0);
		mMediaDesc.hasImage = false;
		mMediaDesc.audioDuration = clipInfo.mAudioDuration;
		mMediaDesc.videoDuration = clipInfo.mVideoDuration;
		mMediaDesc.fileSize = getLength();
		mMediaDesc.videoWidth = clipInfo.mVideoWidth;
		mMediaDesc.videoHeight = clipInfo.mVideoHeight;
		mMediaDesc.displayVideoWidth = clipInfo.mDisplayVideoWidth;
		mMediaDesc.displayVideoHeight = clipInfo.mDisplayVideoHeight;
		mMediaDesc.seekPointCount = clipInfo.mSeekPointCount;
		mMediaDesc.framesPerSecond = clipInfo.mFPS;
		mMediaDesc.framesPerSecondFloat = clipInfo.mfFPS;
		mMediaDesc.videoH264Profile = clipInfo.mVideoH264Profile;
		mMediaDesc.videoH264Level = clipInfo.mVideoH264Level;
		mMediaDesc.VideoH264Interlaced = clipInfo.mVideoH264Interlaced;
		mMediaDesc.videoOrientation = clipInfo.mVideoOrientation;
		mMediaDesc.videoBitrate = clipInfo.mVideoBitRate;
		mMediaDesc.audioBitrate = clipInfo.mAudioBitRate;
		mMediaDesc.audioSamplingRate = clipInfo.mAudioSampleRate;
		mMediaDesc.audioChannels = clipInfo.mAudioChannels;
		mMediaDesc.videoRenderType = clipInfo.mVideoRenderType;
		mMediaDesc.videoCodecType = clipInfo.mVideoCodecType;
		mMediaDesc.audioCodecType = clipInfo.mAudioCodecType;
		mMediaDesc.videoHDRType = clipInfo.mVideoHDRType;

		if( useCache ) {
			if (gson == null) gson = new Gson();

			if (LL.D) Log.d(LOG_TAG, "getInfo(" + path + ") writing:" + mInfoFile);

			try {
				Writer writer = new BufferedWriter(new FileWriter(mInfoFile));
				try {
					gson.toJson(mMediaDesc, writer);
				} finally {
					writer.close();
					mInfoFile.setReadable(true);
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		if(LL.D) Log.d(LOG_TAG,"getInfo(" + path + ") out");
	}

	private static String makePathHash( String assetUrl ) {
		String cache = assetUrl.substring(11);
		cache.replace(':','_');
		return "nexasset_"+cache;
	}

    private static String makePathHash( File originalPath ) {
    	String name = originalPath.getName();
    	int pathHash = originalPath.getAbsolutePath().hashCode();
    	long modTime = originalPath.lastModified();
    	long fileSize = originalPath.length();
    	if( name.length() > 32 ) {
    		name = name.substring(0, 32);
    	}
    	int hashcode = (int)(((modTime * 212501089) + (fileSize * 194851861) + pathHash)%0xFFFFFFF);
    	return name + "_" + String.format(Locale.US, "%08X", hashcode);
    }

	private static class PCMLevelParams {
		public final String mediaUrl;
		public final File rawPcmLevelFile;
		public final int startTime;
		public final int endTime;
		public final int useCount;
		public final int skipCount;

		public PCMLevelParams(String mediaUrl, File rawPcmLevelFile, int startTime, int endTime, int useCount, int skipCount) {
			this.mediaUrl = mediaUrl;
			this.rawPcmLevelFile = rawPcmLevelFile;
			this.startTime = startTime;
			this.endTime = endTime;
			this.useCount = useCount;
			this.skipCount = skipCount;
		}
	}

	private static class ThumbnailParams {
		public final String mediaUrl;
		public final File rawThumbFile;
		public final int reqWidth;
		public final int reqHeight;
		public final int startTime;
		public final int endTime;
		public final int reqCount;
        public final int flags; //yoon
		public final int timeTable[]; //yoon

		protected ThumbnailParams(String mediaUrl, File rawThumbFile, int reqWidth, int reqHeight, int startTime, int endTime, int reqCount, int flags) {
			this.mediaUrl = mediaUrl;
			this.rawThumbFile = rawThumbFile;
			this.reqWidth = reqWidth;
			this.reqHeight = reqHeight;
			this.startTime = startTime;
			this.endTime = endTime;
			this.reqCount = reqCount;
            this.flags = flags;
			this.timeTable = null;
		}

		protected ThumbnailParams(String mediaUrl, File rawThumbFile, int reqWidth, int reqHeight, int[] timeTable, int flags) {
			this.mediaUrl = mediaUrl;
			this.rawThumbFile = rawThumbFile;
			this.reqWidth = reqWidth;
			this.reqHeight = reqHeight;
			this.startTime = 0;
			this.endTime = 0;
			this.reqCount = timeTable.length;
			this.flags = flags;
			this.timeTable = timeTable;
		}


	}

	private static Deque<MediaInfoTask<Thumbnails,ThumbnailParams>> sPendingThumbnailTasks = new ArrayDeque<>();

    private Task convertRawThumbsToJPEG() {
    	final Task task = new Task();
    	
		File mediaInfoDir = new File(sCacheDir,".km_mediainfo");
		mediaInfoDir.mkdirs();    	
    	
    	new ThumbnailConversionTask(mVideoRawThumbFile, mVideoThumbFile, mVideoLargeStartThumbFile, mVideoLargeEndThumbFile) {
    		
    		@Override
    		protected void onSuccess() {
   				mVideoRawThumbFile.delete();
   				task.signalEvent(Task.Event.SUCCESS, Task.Event.COMPLETE);
    		}
    		
    		@Override
    		protected void onError(TaskError error) {
   				mVideoRawThumbFile.delete();
				task.sendFailure(error);
//    			task.signalEvent(Task.Event.FAIL);
    		}

    	}.executeOnExecutor(THUMBNAIL_CONVERTER_EXECUTOR, 0);
    	
    	return task;
    }

//    private static int sThumbnailsRunTaskId = 0; //yoon
//    private static boolean sThumbnailCancel;
    public static boolean cancelAllGetThumbnails(){
        NexEditor editor = getEditor();
        if( editor==null ) {
            return false;
        }
//        sThumbnailCancel = true;

		if(LL.D) Log.d(LOG_TAG,"cancelAllGetThumbnails : Pending size="+sPendingThumbnailTasks.size());
		if(LL.D) Log.d(LOG_TAG,"cancelAllGetThumbnails : Active size="+sActiveTasks.size());

		for( int i = 0 ; i < sActiveTasks.size() ; i++ ){
			MediaInfoTask<Thumbnails, ThumbnailParams> task = (MediaInfoTask<Thumbnails, ThumbnailParams>) sActiveTasks.valueAt(i);
			task.retry = 0;
			task.cancel = true;
		}

		for(Iterator itr = sPendingThumbnailTasks.iterator();itr.hasNext();)  {
			MediaInfoTask<Thumbnails, ThumbnailParams> task = (MediaInfoTask<Thumbnails, ThumbnailParams>) itr.next();
			if(LL.D) Log.d(LOG_TAG,"cancelAllGetThumbnails : id="+task.getTaskId());
			task.retry = 0;
			task.cancel = true;
		}

		editor.getThumbStop(0);

        return true;
    }

	private static int sThumbnailDetailSerial = 0;

	public Task getDetailThumbnails(int width, int height, int startTime, int endTime, final int count, final int flags, final int [] arrayTimeTab , final ThumbnailCallback thumbnailCallback) {
		sThumbnailDetailSerial++;

		File dir = mMediaInfoDir;
		if( sThumbnailTempDir != null ){ //yoon
			dir = sThumbnailTempDir;
			if(LL.D) Log.d(LOG_TAG,"getDetailThumbnails::temp cache dir="+dir.getAbsolutePath());
		}

		if( !dir.exists() ){
			dir.mkdirs();
		}

		final File rawThumbFile = new File(dir, mMediaPathHash + "_detail_" + width + "_" + height + "_" + startTime + "_" + endTime + "_" + count + "_" + sThumbnailDetailSerial);

		ThumbnailParams params = null;
		if( arrayTimeTab != null ){
			params = new ThumbnailParams(mMediaFileUrl, rawThumbFile, width, height, arrayTimeTab, flags);
		}else {
			params = new ThumbnailParams(mMediaFileUrl , rawThumbFile, width, height, startTime, endTime, count, flags);
		}

		MediaInfoTask<Thumbnails,ThumbnailParams> detailThumbnailTask = new MediaInfoTask<Thumbnails,ThumbnailParams>(this, params, new OnEditorAsyncDoneListener<Thumbnails, ThumbnailParams>() {
			@Override
			public void onEditorAsyncDone(final MediaInfoTask<Thumbnails, ThumbnailParams> task, ErrorCode resultCode) {

				if (resultCode != NexEditor.ErrorCode.NONE) {

					if (task.cancel) {
						if (LL.D)
							Log.d(LOG_TAG, "getDetailThumbnails::onEditorAsyncDone : User Cancel ID=" + task.getTaskId());
						task.retry = 0;
						rawThumbFile.delete();
						task.sendFailure(/*ThumbnailError.UserCancel*/ ErrorCode.GETCLIPINFO_USER_CANCEL);
						startPendingThumbnailTask();
						return;
					}

					task.retry--;

					if( thumbnailCallback instanceof ThumbnailCallbackRaw ){
						task.retry = 0;
						Log.d(LOG_TAG, "getDetailThumbnails::Raw File. No RETRYING=" + resultCode + ", ID=" + task.getTaskId());
					}

					if (task.retry > 0) {
						if (LL.D)
							Log.d(LOG_TAG, "getDetailThumbnails::onEditorAsyncDone : RETRYING=" + resultCode + ", ID=" + task.getTaskId());
						rawThumbFile.delete();//yoon
						if (resultCode == ErrorCode.GETCLIPINFO_USER_CANCEL || task.cancel) {
							task.retry = 0;
							task.sendFailure(/*ThumbnailError.UserCancel*/ ErrorCode.GETCLIPINFO_USER_CANCEL);
							startPendingThumbnailTask();
						} else {
							sPendingThumbnailTasks.add(task);
							if ( resultCode != ErrorCode.INPROGRESS_GETCLIPINFO) {
								startPendingThumbnailTask();
							}
							/*
							if (sPendingThumbnailTasks.size() <= 1 && resultCode != ErrorCode.INPROGRESS_GETCLIPINFO) {
								startPendingThumbnailTask();
							}
							*/
						}
					} else {
						if (LL.D)
							Log.d(LOG_TAG, "getDetailThumbnails::onEditorAsyncDone : SEND FAILURE=" + resultCode + ", cancel=" + task.cancel);
						if (resultCode == ErrorCode.GETCLIPINFO_USER_CANCEL || task.cancel) {
							task.retry = 0;
							//startPendingThumbnailTask();
						}
						rawThumbFile.delete();
						task.sendFailure(resultCode);
						startPendingThumbnailTask();
					}
					return;
				}
				startPendingThumbnailTask();
				if ((task.getParams().flags & 0x40000) == 0x40000) {
					Log.d(LOG_TAG, "getDetailThumbnails::onEditorAsyncDone() no cache mode");
					task.signalEvent(Event.SUCCESS, Event.COMPLETE);
					task.sendResult(NoThumbnails.getInstance());
				} else {
					if (LL.D)
						Log.d(LOG_TAG, "getDetailThumbnails::onEditorAsyncDone(" + mMediaFileUrl + ") -> " + rawThumbFile);

					new AsyncTask<Void, Void, Void>() {

						TaskError error = null;

						@Override
						protected Void doInBackground(Void... params) {
							if(count ==0) {
								error = ThumbnailParser.parseFromRawFile(rawThumbFile, 50, thumbnailCallback);
							}else{
								error = ThumbnailParser.parseFromRawFile(rawThumbFile, count, thumbnailCallback);
							}
							rawThumbFile.delete();
							return null;
						}

						@Override
						protected void onPostExecute(Void aVoid) {
							if (error != null) {
								task.sendFailure(error);
							} else {
								task.signalEvent(Event.SUCCESS, Event.COMPLETE);
								task.sendResult(NoThumbnails.getInstance());
							}
						}
					}.executeOnExecutor(THUMBNAIL_CONVERTER_EXECUTOR);
				}
			}
		}, new OnThumbDoneListener() {
			@Override
			public void onThumbDone(int iMode, int iTime, int iWidth, int iHeight, int iSize, byte[] arrayData, int iCount, int iTotal) {
				if( iMode != 1 ){
					Log.d(LOG_TAG, "getDetailThumbnails::onThumbDone() not video. mode="+iMode);
					return ;
				}
				int format = (iSize / (iWidth * iHeight))*8;
				try {
					ThumbnailParser.parseFromStream(arrayData, iTime, iWidth, iHeight, format, iCount - 1, iTotal, thumbnailCallback);
				} catch (IOException e) {
					e.printStackTrace();
				}
				return ;
			}
		});

		NexEditor editor = getEditor();
		boolean startNow = editor!=null && sPendingThumbnailTasks.isEmpty();
		sPendingThumbnailTasks.add(detailThumbnailTask);
		if( startNow ) {
			startPendingThumbnailTask();
		}

		return detailThumbnailTask;
	}

    public ResultTask<Thumbnails> getThumbnails() {
        if( mVideoThumbFile==null && mVideoRawThumbFile==null ) {
            return ResultTask.failedResultTask(MediaInfoError.ThumbnailsNotAvailable);
        }
		if(LL.D)
			Log.d(LOG_TAG,"getThumbnails(" + mMediaFileUrl + ") sPendingThumbnailTasks=" + sPendingThumbnailTasks.size());
    	
    	if( mThumnbnailTask!=null && !mThumnbnailTask.didSignalEvent(Task.Event.FAIL) ) {
    		if(LL.D)
				Log.d(LOG_TAG,"getThumbnails(" + mMediaFileUrl + ") returning existing task");
    		return mThumnbnailTask; 
    	}

		ThumbnailParams params = new ThumbnailParams(mMediaFileUrl,mVideoRawThumbFile,640,360,0,getVideoDuration(),30,0);
    	
    	mThumnbnailTask = new MediaInfoTask<Thumbnails,ThumbnailParams>(this, params, new OnEditorAsyncDoneListener<Thumbnails, ThumbnailParams>() {
			@Override
			public void onEditorAsyncDone(final MediaInfoTask<Thumbnails, ThumbnailParams> task, ErrorCode resultCode) {
	    		if(LL.D)
					Log.d(LOG_TAG,"getThumbnails::onEditorAsyncDone(" + mMediaFileUrl + ") resultCode=" + resultCode + " retry=" + task.retry);
				if( resultCode != NexEditor.ErrorCode.NONE ) {
					task.retry--;
					if( task.retry > 0 ) {
			    		if(LL.D) Log.d(LOG_TAG,"getThumbnails::onEditorAsyncDone : RETRYING ID="+task.getTaskId());
						sPendingThumbnailTasks.add(task);
						// TODO: Start pending task after delay?  (if not, engine crashes?????)
						if( sPendingThumbnailTasks.size()<=1 && resultCode!=ErrorCode.INPROGRESS_GETCLIPINFO )
							startPendingThumbnailTask();
//						startPendingThumbnailTask();
					} else {
			    		// FOR Thumbnail not load issue.
			    		if(LL.D) Log.d(LOG_TAG,"getThumbnails::onEditorAsyncDone : SEND FAILURE");
						mThumnbnailTask.sendFailure(resultCode);
					}
					return;
				}
				
				startPendingThumbnailTask();
	    		if(LL.D)
					Log.d(LOG_TAG, "getThumbnails::onEditorAsyncDone(" + mMediaFileUrl + ") Start JPEG Conversion");
				convertRawThumbsToJPEG().onComplete(new OnTaskEventListener() {
					@Override
					public void onTaskEvent(Task t, Event e) {
						readBitmapFileWithTimeIndex(mVideoThumbFile).onResultAvailable(
								new OnResultAvailableListener<BitmapWithIndex>() {
							@Override
							public void onResultAvailable(
									ResultTask<BitmapWithIndex> task, Event event,
									BitmapWithIndex result) {
								if( result!=null ) {
									mThumnbnailTask.sendResult(new ThumbnailsImpl(result.bm,result.width,result.height,result.timeIndex));
								} else {
									mThumnbnailTask.sendFailure(null);
								}
							}
				    	});
					}
				}).onFailure( new OnFailListener() {
					
					@Override
					public void onFail(Task t, Event e, TaskError reason) {
						mThumnbnailTask.sendFailure(null);
					}
				});
			}
		});
    	
    	if( !EditorGlobal.SHOW_VIDEO_THUMBNAILS ) {
    		if(LL.D)
				Log.d(LOG_TAG,"getThumbnails(" + mMediaFileUrl + ") skipping because thumbnails disabled");
    		mThumnbnailTask.sendFailure(null);
        	return mThumnbnailTask;
    	}
    	
    	readBitmapFileWithTimeIndex(mVideoThumbFile).onResultAvailable(new OnResultAvailableListener<BitmapWithIndex>() {
			@Override
			public void onResultAvailable(ResultTask<BitmapWithIndex> task, Event event, BitmapWithIndex result) {
				if( result!=null && result.bm !=null && result.width > 0 && result.height > 0 && result.timeIndex!=null ) {
					mThumnbnailTask.sendResult(new ThumbnailsImpl(result.bm,result.width,result.height,result.timeIndex));
				} else {
		    		NexEditor editor = getEditor();
					boolean startNow = editor!=null && sPendingThumbnailTasks.isEmpty();
						sPendingThumbnailTasks.add(mThumnbnailTask);
					if( startNow ) {
						startPendingThumbnailTask();
					}
//				    	try {
//							editor.makeVideoThumbs(
//									mMediaFile,
//									mVideoRawThumbFile,
//									mThumnbnailTask.getTaskId());
//						} catch (IOException e) {
//							e.printStackTrace();
//							mThumnbnailTask.sendFailure(null);
//						}
				}

			}
		});
    	
		if(LL.D)
			Log.d(LOG_TAG,"getThumbnails(" + mMediaFileUrl + ") returning NEW task");
    	return mThumnbnailTask;
    }
    
    private ResultTask<Bitmap> readBitmapFile( File path ) {
    	final ResultTask<Bitmap> task = new ResultTask<Bitmap>();
    	
    	new AsyncTask<File, Integer, Bitmap>(){

			@Override
			protected Bitmap doInBackground(File... params) {
				return BitmapFactory.decodeFile(params[0].getAbsolutePath());
			}

			@Override
			protected void onPostExecute(Bitmap result) {
				task.setResult(result);
				task.signalEvent(Task.Event.RESULT_AVAILABLE, Task.Event.SUCCESS, Task.Event.COMPLETE);
			}
    		
    	}.executeOnExecutor(PARSER_EXECUTOR,path);
    	return task;
    }
    
    private static class BitmapWithIndex {
    	final int width;
    	final int height;
    	final int[] timeIndex;
    	final Bitmap bm;
    	BitmapWithIndex(int width, int height, int[] timeIndex, Bitmap bm ){
    		this.width = width;
    		this.height = height;
    		this.timeIndex = timeIndex;
    		this.bm = bm;
    	}
    }
    
    private ResultTask<BitmapWithIndex> readBitmapFileWithTimeIndex( File path ) {
    	final ResultTask<BitmapWithIndex> task = new ResultTask<BitmapWithIndex>();
    	
    	new AsyncTask<File, Integer, BitmapWithIndex>(){

			@Override
			protected BitmapWithIndex doInBackground(File... params) {
				
				try {
					return readFile(params[0]);
				} catch (IOException e) {
					e.printStackTrace();
					return null;
				}
//				return BitmapFactory.decodeFile(params[0].getAbsolutePath());
			}
			
			private BitmapWithIndex readFile(File file) throws IOException {
				DataInputStream input = new DataInputStream(new BufferedInputStream(new FileInputStream(file)));
				int width = input.readInt();
				int height = input.readInt();
				int entries = input.readInt();
				int[] index = new int[entries];
				for( int i = 0; i<entries; i++ ) {
					index[i] = input.readInt();
				}
				Bitmap bm = BitmapFactory.decodeStream(input);
				return new BitmapWithIndex(width, height, index, bm);
			}

			@Override
			protected void onPostExecute(BitmapWithIndex result) {
				task.setResult(result);
				task.signalEvent(Task.Event.RESULT_AVAILABLE, Task.Event.SUCCESS, Task.Event.COMPLETE);
			}
    		
    	}.executeOnExecutor(PARSER_EXECUTOR,path);
    	return task;
    }

	private ResultTask<PCMLevels> readPCMFile(File pcmFile) {
		final ResultTask<PCMLevels> task = new ResultTask<PCMLevels>();
		new AsyncTask<File, Integer, PCMLevels>() {

			@Override
			protected PCMLevels doInBackground(File... params) {
				File path = params[0];
				if( path.exists() ) {
					if( path.length() == 0 ){
						return null;
					}
					int pcmsize = (int)Math.min(path.length(),200*1024);
					try {
						byte[] buffer = new byte[pcmsize];
						InputStream reader = new FileInputStream(path);
						int readBytes = 0;
						try {
							readBytes = reader.read(buffer);
						} finally {
							reader.close();
							path.delete();
						}

						if( readBytes>=pcmsize ) {
							return new PCMLevels(buffer);
						}
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
				return null;
			}

			@Override
			protected void onPostExecute(PCMLevels result) {
				task.setResult(result);
				task.signalEvent(Task.Event.RESULT_AVAILABLE, Task.Event.SUCCESS, Task.Event.COMPLETE);
			}

		}.executeOnExecutor(PARSER_EXECUTOR,pcmFile);
		return task;
	}

    private ResultTask<PCMLevels> readPCMFile() {
    	final ResultTask<PCMLevels> task = new ResultTask<PCMLevels>();
    	new AsyncTask<MediaInfo, Integer, PCMLevels>() {
    		
			@Override
			protected PCMLevels doInBackground(MediaInfo... params) {
				MediaInfo mediaInfo = params[0];
				if( mediaInfo.mAudioPCMFile.exists() ) {
					int pcmsize = (int)Math.min(mediaInfo.mAudioPCMFile.length(),200*1024);
					try {
						byte[] buffer = new byte[pcmsize];
						InputStream reader = new FileInputStream(mAudioPCMFile);
						int readBytes = 0;
						try {
							readBytes = reader.read(buffer);
						} finally {
							reader.close();
						}
						if( readBytes>=pcmsize ) {
							return new PCMLevels(buffer);
						}
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
				return null;
			}
			
			@Override
			protected void onPostExecute(PCMLevels result) {
				task.setResult(result);
				task.signalEvent(Task.Event.RESULT_AVAILABLE, Task.Event.SUCCESS, Task.Event.COMPLETE);
			}
			
		}.executeOnExecutor(PARSER_EXECUTOR,this);
		return task;
    }

    private static Deque<MediaInfoTask<PCMLevels,Void>> sPendingPCMLevelTasks = new ArrayDeque<>();
	private static Deque<MediaInfoTask<PCMLevels,PCMLevelParams>> sPendingPCMLevelParcelTasks = new ArrayDeque<>();

    public static enum MediaInfoError implements TaskError {
        PCMLevelsNotAvailable, SeekPointsNotAvailable, ThumbnailsNotAvailable, LargeStartThumbnailNotAvailable, LargeEndThumbnailNotAvailable;

        @Override
        public Exception getException() {
            return null;
        }

        @Override
        public String getMessage() {
            return name();
        }

        @Override
        public String getLocalizedMessage(Context context) {
            return name();
        }
    }

    public ResultTask<PCMLevels> getPCMLevels( int startTime, int endTime, int useCount, int skipCount) {
		File tmpFile = null;
		try {
			tmpFile = File.createTempFile("pcmlevel","dum");
		} catch (IOException e) {
			return ResultTask.failedResultTask(MediaInfoError.PCMLevelsNotAvailable);
		}
		Log.d(LOG_TAG,"getPCMLevels start="+startTime+", endTime="+endTime+", useCount="+useCount+", skipCount="+skipCount+",tmpFile="+tmpFile.getAbsolutePath());
		PCMLevelParams param = new PCMLevelParams(mMediaFileUrl,tmpFile,startTime,endTime,useCount,skipCount);

		MediaInfoTask<PCMLevels,PCMLevelParams> PCMLevelParcelTask = new MediaInfoTask<PCMLevels,PCMLevelParams>(this, param, new OnEditorAsyncDoneListener<PCMLevels,PCMLevelParams>() {
			@Override
			public void onEditorAsyncDone(final MediaInfoTask<PCMLevels,PCMLevelParams> task, ErrorCode resultCode) {
				Log.d(LOG_TAG,"getPCMLevels onResultAvailable resultCode="+resultCode);
				if( resultCode==NexEditor.ErrorCode.INVALID_STATE ) {
					sPendingPCMLevelParcelTasks.add(task);
					return;
				}
				startPendingPCMLevelParcelTask();
				task.mediaInfo.readPCMFile(task.getParams().rawPcmLevelFile).onResultAvailable(new OnResultAvailableListener<PCMLevels>() {
					@Override
					public void onResultAvailable(ResultTask<PCMLevels> readFileTask, Event event, PCMLevels result) {
						if( result!=null ) {
							task.sendResult(result);
						} else {
							Log.d(LOG_TAG,"getPCMLevels onResultAvailable sendFailure!");
							task.sendFailure(null);
						}
					}
				});
			}
		});

		NexEditor editor = getEditor();
		if( editor==null || !sPendingPCMLevelParcelTasks.isEmpty() ) {
			sPendingPCMLevelParcelTasks.add(PCMLevelParcelTask);
		} else {
			editor.makeAudioPCMPartial(mMediaFileUrl, PCMLevelParcelTask.getParams().rawPcmLevelFile
					,PCMLevelParcelTask.getParams().startTime
					,PCMLevelParcelTask.getParams().endTime
					,PCMLevelParcelTask.getParams().useCount
					,PCMLevelParcelTask.getParams().skipCount ,PCMLevelParcelTask.getTaskId());
		}

		return PCMLevelParcelTask;
	}

    public ResultTask<PCMLevels> getPCMLevels() {

        if( mAudioPCMFile==null ) {
            return ResultTask.failedResultTask(MediaInfoError.PCMLevelsNotAvailable);
        }
    	
    	if( mPCMLevelTask!=null && !mPCMLevelTask.didSignalEvent(Task.Event.FAIL) ) {
    		return mPCMLevelTask;
    	}
    	
    	mPCMLevelTask = new MediaInfoTask<PCMLevels,Void>(this, null, new OnEditorAsyncDoneListener<PCMLevels,Void>() {
			@Override
			public void onEditorAsyncDone(final MediaInfoTask<PCMLevels,Void> task, ErrorCode resultCode) {
				if( resultCode==NexEditor.ErrorCode.INVALID_STATE ) {
					sPendingPCMLevelTasks.add(task);
					return;
				}
				startPendingPCMLevelTask();
				task.mediaInfo.readPCMFile().onResultAvailable(new OnResultAvailableListener<PCMLevels>() {
					@Override
					public void onResultAvailable(ResultTask<PCMLevels> readFileTask, Event event, PCMLevels result) {
						if( result!=null ) {
							task.sendResult(result);
						} else {
							task.sendFailure(null);
						}
					}
				});
			}
		});
    	
    	readPCMFile().onResultAvailable(new OnResultAvailableListener<PCMLevels>() {
			@Override
			public void onResultAvailable(ResultTask<PCMLevels> task, Event event, PCMLevels result) {
				if( result!=null ) {
					mPCMLevelTask.sendResult(result);
				} else {
		    		NexEditor editor = getEditor();
					if( editor==null || !sPendingPCMLevelTasks.isEmpty() ) {
						sPendingPCMLevelTasks.add(mPCMLevelTask);
					} else {
						editor.makeAudioPCM(mMediaFileUrl, mAudioPCMFile, mPCMLevelTask.getTaskId());
					}
				}
			}
		});
    	
    	return mPCMLevelTask;
    }
    
    public ResultTask<Bitmap> getLargeThumbnail() {
        if( mVideoLargeStartThumbFile==null ) {
            return ResultTask.failedResultTask(MediaInfoError.LargeStartThumbnailNotAvailable);
        }
    	if( mLargeThumbTask!=null ) {
    		return mLargeThumbTask;
    	}
    	
    	mLargeThumbTask = new ResultTask<Bitmap>();

    	readBitmapFile(mVideoLargeStartThumbFile).onResultAvailable(new OnResultAvailableListener<Bitmap>() {
			@Override
			public void onResultAvailable(ResultTask<Bitmap> task, Event event, Bitmap result) {
				if( result!=null ) {
					mLargeThumbTask.setResult(result);
					mLargeThumbTask.signalEvent(Task.Event.RESULT_AVAILABLE, Task.Event.SUCCESS, Task.Event.COMPLETE);
				} else {
					getThumbnails().onComplete(new OnTaskEventListener() {
						@Override
						public void onTaskEvent(Task t, Event e) {
					    	readBitmapFile(mVideoLargeStartThumbFile).onResultAvailable(new OnResultAvailableListener<Bitmap>() {
								@Override
								public void onResultAvailable(ResultTask<Bitmap> task, Event event, Bitmap result) {
									if( result!=null ) {
										mLargeThumbTask.setResult(result);
										mLargeThumbTask.signalEvent(
												Task.Event.RESULT_AVAILABLE, 
												Task.Event.SUCCESS, 
												Task.Event.COMPLETE);
									} else {
										mLargeThumbTask.signalEvent(Task.Event.FAIL);
									}
								}
					    	});
						}
					}).onFailure(new OnFailListener() {
                        @Override
                        public void onFail(Task t, Event e, TaskError failureReason) {
                            if(LL.D) Log.d(LOG_TAG, "getLargeThumbnail :: readBitmapFile :: onFail");
                            mLargeThumbTask.signalEvent(Task.Event.FAIL);
                        }
					});
				}
			}
		});
    	
    	return mLargeThumbTask;
    }

    public ResultTask<Bitmap> getLargeEndThumbnail() {
        if( mVideoLargeEndThumbFile==null ) {
            return ResultTask.failedResultTask(MediaInfoError.LargeEndThumbnailNotAvailable);
        }

        if( mLargeEndThumbTask!=null ) {
            return mLargeEndThumbTask;
        }

        mLargeEndThumbTask = new ResultTask<Bitmap>();

        readBitmapFile(mVideoLargeEndThumbFile).onResultAvailable(new OnResultAvailableListener<Bitmap>() {
            @Override
            public void onResultAvailable(ResultTask<Bitmap> task, Event event, Bitmap result) {
                if( result!=null ) {
                    mLargeEndThumbTask.setResult(result);
                    mLargeEndThumbTask.signalEvent(Task.Event.RESULT_AVAILABLE, Task.Event.SUCCESS, Task.Event.COMPLETE);
                } else {
                    getThumbnails().onComplete(new OnTaskEventListener() {
                        @Override
                        public void onTaskEvent(Task t, Event e) {
                            readBitmapFile(mVideoLargeEndThumbFile).onResultAvailable(new OnResultAvailableListener<Bitmap>() {
                                @Override
                                public void onResultAvailable(ResultTask<Bitmap> task, Event event, Bitmap result) {
                                    if( result!=null ) {
                                        mLargeEndThumbTask.setResult(result);
                                        mLargeEndThumbTask.signalEvent(
                                                Task.Event.RESULT_AVAILABLE,
                                                Task.Event.SUCCESS,
                                                Task.Event.COMPLETE);
                                    } else {
                                        mLargeEndThumbTask.signalEvent(Task.Event.FAIL);
                                    }
                                }
                            });
                        }
                    });
                }
            }
        });

        return mLargeEndThumbTask;
    }

    
//    public ResultTask<byte[]> getLargeThumbnailAsJpeg() {
//    	// TODO: Implement this
//    	return null;
//    }
    
    public int[] getSeekPointsSync() {
    	if( mSeekPointAsyncTask==null ) {
    		getSeekPoints();
    		if( mSeekPointAsyncTask==null ) {
    			throw new IllegalStateException();
    		}
    	}
    	try {
			return mSeekPointAsyncTask.get();
		} catch (InterruptedException e) {
			e.printStackTrace();
			return null;
		} catch (ExecutionException e) {
			e.printStackTrace();
			return null;
		}
    }

	public byte[] getVideoUUID( ){
		NexEditor editor = getEditor();
		if( editor==null ) {
			return null;
		}

		NexClipInfo clipInfo = new NexClipInfo();
		NexEditor.ErrorCode result = editor.getClipInfoSync(mMediaFileUrl, clipInfo, false, 0);
		if( result.isError() || clipInfo.mVideoUUID==null ) {
			if(LL.D)
				Log.d(LOG_TAG,"getVideoUUID(" + mMediaFileUrl + ") FAIL -> " + result);
			return null;
		}
		return clipInfo.mVideoUUID;
	}

	public NexEditor.ErrorCode ableToDecoding(boolean video, boolean audio ){
		NexEditor editor = getEditor();
		if( editor==null ) {
			return ErrorCode.GENERAL;
		}

		int flags = 0;
		if( video ){
			flags |= NexEditorType.GET_CLIPINFO_CHECK_VIDEO_DEC;
		}
		if( audio ){
			flags |= NexEditorType.GET_CLIPINFO_CHECK_AUDIO_DEC;
		}
		NexClipInfo clipInfo = new NexClipInfo();
		NexEditor.ErrorCode result = editor.getClipInfoSyncFlags(mMediaFileUrl, clipInfo, flags, 0);
		return result;
	}


    public ResultTask<int[]> getSeekPoints() {

        if( mSeekPointFile==null ) {
            return ResultTask.failedResultTask(MediaInfoError.SeekPointsNotAvailable);
        }

    	if( mSeekPointsTask!=null && !mSeekPointsTask.didSignalEvent(Task.Event.FAIL) ) {
    		return mSeekPointsTask;
    	}
		if(LL.D)
			Log.d(LOG_TAG,"getSeekPoints(" + mMediaFileUrl + ")");
    	
    	mSeekPointsTask = new ResultTask<int[]>();
    	
		mSeekPointAsyncTask = new AsyncTask<String, Integer, int[]>() {
    		
			@Override
			protected int[] doInBackground(String... params) {
				if(LL.D) Log.d(LOG_TAG,"getSeekPoints:doInBackground(" + params[0] + ")");
				try {
					return readFile(mSeekPointFile);
				} catch (IOException e) {
				}
				if(LL.D) Log.d(LOG_TAG,"getSeekPoints:doInBackground(" + params[0] + ") -> no cache available; making");
				
				NexEditor editor = getEditor();
				if( editor==null ) {
					return null;
				}

				NexClipInfo clipInfo = new NexClipInfo();
				NexEditor.ErrorCode result = editor.getClipInfoSync(params[0], clipInfo, true, 0);
				if( result.isError() || clipInfo.mSeekTable==null ) {
					if(LL.D)
						Log.d(LOG_TAG,"getSeekPoints(" + mMediaFileUrl + ") FAIL -> " + result);
					return null;
				}
				try {
					writeFile(mSeekPointFile,clipInfo.mSeekTable);
				} catch (IOException e) {
					if(LL.D)
						Log.d(LOG_TAG, "getSeekPoints(" + mMediaFileUrl + ") FAILED WRITING FILE");
					e.printStackTrace();
				}
				return clipInfo.mSeekTable;
			}
			
			private int[] readFile(File f) throws IOException {
				DataInputStream input = new DataInputStream(new FileInputStream(f));
				try {
					int size = (int)Math.min(f.length(),200*1024);
					int count = size/4;
					int[] buffer = new int[count];
					for( int i=0; i<count; i++ ) {
						buffer[i] = input.readInt();
					}
					if(LL.D) Log.d(LOG_TAG,"getSeekPoints():readFile : got " + count + " entries.");
					return buffer;
				} finally {
					input.close();
				}
			}
			
			private void writeFile(File f, int[] data) throws IOException {
				if(LL.D) Log.d(LOG_TAG,"getSeekPoints():writeFile(" + f + ")");
				DataOutputStream output = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(f)));
				try {
					for( int i=0; i<data.length; i++ ) {
						output.writeInt(data[i]);
					}
					if(LL.D)
						Log.d(LOG_TAG,"getSeekPoints(" + mMediaFileUrl + ") wrote " + data.length + " points");
				} finally {
					output.close();
					if(LL.D)
						Log.d(LOG_TAG,"getSeekPoints(" + mMediaFileUrl + ") wrote file: " + f);
					f.setReadable(true);
				}
			}
			
			@Override
			protected void onPostExecute(int[] result) {
				if( result==null ) {
					if(LL.D) Log.d(LOG_TAG,"onPostExecute : FAIL");
					mSeekPointsTask.signalEvent(Task.Event.FAIL);
				} else {
					if(LL.D) Log.d(LOG_TAG,"onPostExecute : SUCCESS " + result.length);
					mSeekPointsTask.setResult(result);
					mSeekPointsTask.signalEvent(Task.Event.RESULT_AVAILABLE, Task.Event.SUCCESS, Task.Event.COMPLETE);
				}
			}
		}.executeOnExecutor(PARSER_EXECUTOR,mMediaFileUrl);
    	return mSeekPointsTask;
    }
    
    public File getPath() {
		return new File(mMediaFileUrl);
    }

	public String getStringPath() {
		return mMediaFileUrl;
	}

    public long getFileSize() {
        if( mIsExcluded || mGetClipInfoResult.isError() ) {
            return 0;
        } else {
            return mMediaDesc.fileSize;
        }
    }
    
    public int getSeekPointCount() {
        if( mIsExcluded || mGetClipInfoResult.isError() ) {
            return 0;
        } else {
            return mMediaDesc.seekPointCount;
        }
    }
    
    public boolean isFPSSupported() {
        if( mGetClipInfoResult == ErrorCode.UNSUPPORT_VIDEO_FPS ) {
            return false;
        } else {
            return true;
        }
    }
    
    public boolean isFormatSupported() {
        // for Support MOV
        // if(isMov()) return false;

        if( mGetClipInfoResult == ErrorCode.UNSUPPORT_VIDEO_FPS ) {
            //return true;
            return true; // TODO: DENNIS, please change this to TRUE when transcoding is implemented based on FPS. --Matthew
        } else if( mGetClipInfoResult==ErrorCode.UNSUPPORT_MAX_RESOLUTION ) {
            NexEditorDeviceProfile profile = NexEditorDeviceProfile.getDeviceProfile();
            boolean swCodec = getCanUseSoftwareCodec();
            int maxSize = profile.getMaxImportSize(swCodec);
            if( getVideoHeight() * getVideoWidth() < maxSize * 110 / 100 ) {
                return false;
            }
            return true;
        }
        if( mIsExcluded || (mGetClipInfoResult.isError() ) )
        {
            return false;
        }
        return true;
    }
    
    private boolean isMov() {
    	String path = mMediaFileUrl.toLowerCase(Locale.US);
    	return (path.endsWith(".mov"));
    }
    
    public boolean isResolutionSupported() {
        NexEditorDeviceProfile profile = NexEditorDeviceProfile.getDeviceProfile();
        int maxSize = profile.getMaxResolution();
        if( mGetClipInfoResult == ErrorCode.UNSUPPORT_MAX_RESOLUTION ) {
            return false;
        } else if ( getVideoHeight() * getVideoWidth() > maxSize * 110 / 100 ) {
            return false;
        } else {
            return true;
        }
    }
    
    public boolean isSupported() {
    // for Support MOV
    // if(isMov()) return false
        if( mIsExcluded || mGetClipInfoResult.isError() )  {
            return false;
        }
        if( !isResolutionSupported() ) {
            return false;
        }
        return true;
    }
    
    public boolean isError() {
    	return ( mIsExcluded || mGetClipInfoResult.isError() );
    }

    public int getMediaSupportType() {
        if( mGetClipInfoResult.isError() ) {
            switch( mGetClipInfoResult ) {
                case UNSUPPORT_AUDIO_CODEC:			return -2 /*MediaSupportType.NotSupported_AudioCodec*/;
                case UNSUPPORT_AUDIO_PROFILE:		return -3/*MediaSupportType.NotSupported_AudioProfile*/;
                case UNSUPPORT_FORMAT:				return -4/*MediaSupportType.NotSupported_Container*/;
                case UNSUPPORT_MAX_RESOLUTION:		return -5/*MediaSupportType.NotSupported_ResolutionTooHigh*/;
                case UNSUPPORT_MIN_DURATION:		return -6/*MediaSupportType.NotSupported_DurationTooShort*/;
                case UNSUPPORT_MIN_RESOLUTION:		return -7/*MediaSupportType.NotSupported_ResolutionTooLow*/;
                case UNSUPPORT_VIDEIO_PROFILE:		return -8/*MediaSupportType.NotSupported_VideoProfile*/;
                case UNSUPPORT_VIDEO_CODEC:			return -9/*MediaSupportType.NotSupported_VideoCodec*/;
                case UNSUPPORT_VIDEO_FPS:			return -10/*MediaSupportType.NotSupported_VideoFPS*/;
                case UNSUPPORT_VIDEO_LEVEL:			return -11/*MediaSupportType.NotSupported_VideoLevel*/;
                default:		    				return -12 /*MediaSupportType.NotSupported*/;
            }
        } else {
            return isSupported()?0/*MediaSupportType.Supported*/:-1/*MediaSupportType.Unknown*/;
        }
    }

    public boolean isExcluded() {
    	return mIsExcluded;
    }
    
    public static boolean isMediaTaskBusy() {
		return sActiveTasks.size() == 0 ? false : true;
	}
    
    private static Task mWaitNotBusyTask = null;
    /**
     * Returns a task that is signalled when all media tasks
     * are complete.  Use this with caution:  The completion
     * could happen after the current activity finishes, so
     * weak references should be used to refer to the activity.
     * @return
     */
    public static Task waitForMediaTaskNotBusy() {
    	boolean busy = isMediaTaskBusy();
    	if( mWaitNotBusyTask==null || (mWaitNotBusyTask.isComplete()&&busy) ) {
    		mWaitNotBusyTask = new Task();
    	}
    	if( !busy ) {
    		mWaitNotBusyTask.signalEvent(Task.Event.COMPLETE);
    	}
    	
    	return mWaitNotBusyTask;
    }
    
    public boolean hasAudio() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return false;
		} else {
			return mMediaDesc.hasAudio;
		}
    }
    
    public boolean hasVideo() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return false;
		} else {
			return mMediaDesc.hasVideo;
		}
    }

    public boolean hasImage() {
        if( mIsExcluded || mGetClipInfoResult.isError() ) {
            return false;
        } else {
            return mMediaDesc.hasImage;
        }
    }

    public int getVideoWidth() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.videoWidth;
		}
    }
    
    public int getVideoHeight() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.videoHeight;
		}
    }

    public int getWidth() {
        if( mIsExcluded || mGetClipInfoResult.isError() ) {
            return 0;
        } else {
            return mMediaDesc.videoWidth;
        }
    }

    public int getHeight() {
        if( mIsExcluded || mGetClipInfoResult.isError() ) {
            return 0;
        } else {
            return mMediaDesc.videoHeight;
        }
    }

    public int getDuration() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			if( mMediaDesc.hasAudio )
				return mMediaDesc.audioDuration;
			else
				return mMediaDesc.videoDuration;
		}
    }

    public int getAudioDuration() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.audioDuration;
		}
    }

    public int getVideoDuration() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.videoDuration;
		}
    }

    public int getFPS() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.framesPerSecond;
		}
    }

    public float getFPSFloat() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.framesPerSecondFloat;
		}
    }

	public int getOrientation() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.videoOrientation;
		}
	}

    public int getVideoH264Profile() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.videoH264Profile;
		}
    }

    public int getVideoH264Level() {
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.videoH264Level;
		}
    }

    public int getVideoBitrate() {
        if( mIsExcluded || mGetClipInfoResult.isError() ) {
            return 0;
        } else {
            return mMediaDesc.videoBitrate;
        }
    }

    public int getAudioBitrate() {
        if( mIsExcluded || mGetClipInfoResult.isError() ) {
            return 0;
        } else {
            return mMediaDesc.audioBitrate;
        }
    }

    public int getAudioSamplingRate() {
        if( mIsExcluded || mGetClipInfoResult.isError() ) {
            return 0;
        } else {
            return mMediaDesc.audioSamplingRate;
        }
    }

    public int getAudioChannels() {
        if( mIsExcluded || mGetClipInfoResult.isError() ) {
            return 0;
        } else {
            return mMediaDesc.audioChannels;
        }
    }

	public int getVideoRenderType(){
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.videoRenderType;
		}
	}

	public int getVideoCodecType(){
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.videoCodecType;
		}
	}

	public int getAudioCodecType(){
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.audioCodecType;
		}
	}

	public int getVideoHDRType(){
		if( mIsExcluded || mGetClipInfoResult.isError() ) {
			return 0;
		} else {
			return mMediaDesc.videoHDRType;
		}
	}

	private static final int MEDIADESC_VERSION = 9;
    
	@SuppressWarnings("unused")
    private static class MediaDesc {
    	public int version;
		public String path;
    	public long fileSize;
    	public boolean hasAudio;
    	public boolean hasVideo;
        public boolean hasImage;
    	public int videoWidth;
    	public int videoHeight;
    	public int displayVideoWidth;
    	public int displayVideoHeight;
    	public int audioDuration;
    	public int videoDuration;
    	public int seekPointCount;
    	public int framesPerSecond;
    	public float framesPerSecondFloat;
    	public int videoH264Profile;
    	public int videoH264Level;
    	public int VideoH264Interlaced;
        public int videoBitrate;
        public int audioBitrate;
        public int audioSamplingRate;
        public int audioChannels;
		public int videoOrientation;
		public int videoRenderType; //yoon
		public int videoCodecType;
		public int audioCodecType;
		public int videoHDRType; //yoon
//		public String videoUUID;
    }
	
    private static SparseArray<MediaInfoTask<?,?>> sActiveTasks = new SparseArray<MediaInfoTask<?,?>>();
    
    private static interface OnEditorAsyncDoneListener<RESULT_TYPE,PARAM_TYPE> {
    	void onEditorAsyncDone( MediaInfoTask<RESULT_TYPE,PARAM_TYPE> task, ErrorCode resultCode );
    }

	//yoon
	private static interface OnThumbDoneListener {
		void onThumbDone( int iMode, int iTime, int iWidth, int iHeight, int iSize, byte[] arrayData, int iCount, int iTotal );
	}

    private static class MediaInfoTask<RESULT_TYPE,PARAM_TYPE> extends ResultTask<RESULT_TYPE> {
    	int retry = 3;  
		boolean cancel = false; //yoon
    	private final MediaInfo mediaInfo;
    	private final OnEditorAsyncDoneListener<RESULT_TYPE,PARAM_TYPE> onEditorAsyncDoneListener;
		private final OnThumbDoneListener onThumbDone; //yoon
		private final PARAM_TYPE params;
    	public MediaInfoTask(MediaInfo mediaInfo, PARAM_TYPE params, OnEditorAsyncDoneListener<RESULT_TYPE,PARAM_TYPE> listener) {
    		super();
    		this.onEditorAsyncDoneListener = listener;
			this.onThumbDone = null;
    		this.mediaInfo = mediaInfo;
			this.params = params;
    		if(LL.D) Log.d(LOG_TAG,"MediaInfoTask : add to active tasks; tag=" + getTaskId());
    		sActiveTasks.put(getTaskId(), this);
    	}
		//yoon
		public MediaInfoTask(MediaInfo mediaInfo, PARAM_TYPE params, OnEditorAsyncDoneListener<RESULT_TYPE,PARAM_TYPE> listener, OnThumbDoneListener thumb) {
			super();
			this.onEditorAsyncDoneListener = listener;
			this.onThumbDone = thumb;
			this.mediaInfo = mediaInfo;
			this.params = params;
			if(LL.D) Log.d(LOG_TAG,"MediaInfoTask : add to active tasks; thumb tag=" + getTaskId());
			sActiveTasks.put(getTaskId(), this);
		}

		private PARAM_TYPE getParams() {
			return params;
		}
    	@Override
    	public void sendResult( RESULT_TYPE result ) {
    		if(LL.D) Log.d(LOG_TAG,"MediaInfoTask::sendResult : remove from active tasks; tag=" + getTaskId());
    		if( sActiveTasks.get(getTaskId())!=this ) {
        		if(LL.D) Log.d(LOG_TAG,"MediaInfoTask::sendResult : NOT THIS; tag=" + getTaskId());
    			return;
    		}
    		sActiveTasks.remove(getTaskId());
    		if(LL.D) Log.d(LOG_TAG,"MediaInfoTask::sendResult : tag=" + getTaskId() + " sActiveTasks.size()=" + sActiveTasks.size());
    		if( sActiveTasks.size() == 0 && mWaitNotBusyTask!=null ) {
        		if(LL.D) Log.d(LOG_TAG,"MediaInfoTask::sendResult : tag=" + getTaskId() + " mWaitNotBusyTask.signalEvent(COMPLETE)");
    			mWaitNotBusyTask.signalEvent(Event.COMPLETE);
    		}
    		super.sendResult(result);
    	}
    	@Override
    	public void sendFailure( TaskError reason ) {
    		if(LL.D) Log.d(LOG_TAG,"MediaInfoTask::sendFailure : remove from active tasks; tag=" + getTaskId());
    		if( sActiveTasks.get(getTaskId())!=this ) {
        		if(LL.D) Log.d(LOG_TAG,"MediaInfoTask::sendFailure : NOT THIS; tag=" + getTaskId());
    			return;
    		}
    		super.sendFailure(reason);
//    		super.signalEvent(Task.Event.FAIL);
    		sActiveTasks.remove(getTaskId());
    		if(LL.D) Log.d(LOG_TAG,"MediaInfoTask::sendFailure : tag=" + getTaskId() + " sActiveTasks.size()=" + sActiveTasks.size());
    		if( sActiveTasks.size() == 0 && mWaitNotBusyTask!=null ) {
        		if(LL.D) Log.d(LOG_TAG,"MediaInfoTask::sendFailure : tag=" + getTaskId() + " mWaitNotBusyTask.signalEvent(COMPLETE)");
    			mWaitNotBusyTask.signalEvent(Event.COMPLETE);
    		}
    	}
    	private void notifyOnEditorAsyncDoneListener(ErrorCode resultCode) {
    		if( sActiveTasks.get(getTaskId())!=this ) {
    			return;
    		}
    		if(onEditorAsyncDoneListener!=null) {
    			onEditorAsyncDoneListener.onEditorAsyncDone(this, resultCode);
    		}
    	}
		//yoon
		private void notifyOnThumbDoneListener(int iMode, int iTime, int iWidth, int iHeight, int iSize, byte[] arrayData, int iCount, int iTotal) {
			if( sActiveTasks.get(getTaskId())!=this ) {
				return;
			}
			if(onThumbDone!=null) {
				onThumbDone.onThumbDone( iMode,  iTime,  iWidth,  iHeight,  iSize, arrayData,iCount,iTotal);
			}
		}

    }
    
    private static NexEditor getEditor() {
    	if( EditorGlobal.getEditor()==null ) {
			if(LL.D) Log.d(LOG_TAG,"getEditor : NULL EDITOR");
    		return null;
    	}
		if(LL.D) Log.d(LOG_TAG,"getEditor : setting sClipInfoDoneListener=" + sClipInfoDoneListener);
		EditorGlobal.getEditor().setOnGetClipInfoDoneListener(sClipInfoDoneListener);
		EditorGlobal.getEditor().setOnIdleListener(sIdleListener);
		EditorGlobal.getEditor().setOnEditorDestroyedListener(sOnEditorDestroyedListener);
		EditorGlobal.getEditor().setOnGetThumbDoneListener(sThumbDoneListener ); //yoon
    	return EditorGlobal.getEditor();
    }
    
    private static boolean sCheckedSWCodec = false;
    private static boolean sCanUseSWCodec = false;
    private static boolean getCanUseSoftwareCodec() {
    	if( !sCheckedSWCodec ) {
    		NexEditor editor = getEditor();
    		if( editor == null ) {
    			return false;
    		}
    		sCanUseSWCodec = editor.canUseSoftwareCodec();
    		sCheckedSWCodec = true;
    		if(LL.D) Log.d(LOG_TAG,"canUseSWCodec->" + sCanUseSWCodec);
    	}
		return sCanUseSWCodec;
    }

    private static void startPendingPCMLevelTask() {
    	if( !sPendingPCMLevelTasks.isEmpty() ) {
    		NexEditor editor = getEditor();
    		if( editor==null ) {
    			return;
    		}
			MediaInfoTask<PCMLevels,Void> nextTask = sPendingPCMLevelTasks.remove();
	    	editor.makeAudioPCM(
					nextTask.mediaInfo.mMediaFileUrl,
	    			nextTask.mediaInfo.mAudioPCMFile, 
	    			nextTask.getTaskId());
		}
    }

	private static void startPendingPCMLevelParcelTask() {
		Log.d(LOG_TAG,"startPendingPCMLevelParcelTask size="+sPendingPCMLevelParcelTasks.size());
		if( !sPendingPCMLevelParcelTasks.isEmpty() ) {
			NexEditor editor = getEditor();
			if( editor==null ) {
				return;
			}
			MediaInfoTask<PCMLevels,PCMLevelParams> nextTask = sPendingPCMLevelParcelTasks.remove();
			editor.makeAudioPCMPartial(
					nextTask.mediaInfo.mMediaFileUrl,
					nextTask.getParams().rawPcmLevelFile,
					nextTask.getParams().startTime,
					nextTask.getParams().endTime,
					nextTask.getParams().useCount,
					nextTask.getParams().skipCount,
					nextTask.getTaskId());
		}
	}


	private static Object lockStartPendingThumb = new Object();
    private static void startPendingThumbnailTask() {

        synchronized (lockStartPendingThumb) {
    		if(LL.D) Log.d(LOG_TAG,"startPendingThumbnailTask taskcount=" + sPendingThumbnailTasks.size());

    		if (!sPendingThumbnailTasks.isEmpty() ) {

        		NexEditor editor = getEditor();
        		if( editor==null ) {
        			return;
        		}
    			MediaInfoTask<Thumbnails, ThumbnailParams> nextTask = null;
    			boolean isEnd = true;
    			while(!sPendingThumbnailTasks.isEmpty()) {
    				nextTask = sPendingThumbnailTasks.remove();
    				if (nextTask.cancel) {
    					if(LL.D) Log.d(LOG_TAG,"startPendingThumbnailTask cancel thumbnail TaskId=" + nextTask.getTaskId());
    					nextTask.sendFailure(/*ThumbnailError.UserCancel*/ ErrorCode.GETCLIPINFO_USER_CANCEL );
    				}else{
    					isEnd = false;
    					break;
    				}
    			}

    			if(isEnd ){
    				if(LL.D) Log.d(LOG_TAG,"startPendingThumbnailTask all canceled.");
    				return;
    			}

    			ThumbnailParams params = nextTask.getParams();
    	    	try {
    //                sThumbnailsRunTaskId = nextTask.getTaskId();
    				if( params.timeTable != null ){

    					if(LL.D) Log.d(LOG_TAG,"startPendingThumbnailTask use TimeTable sThumbnailsRunTaskId=" + nextTask.getTaskId());

    					editor.makeVideoThumbs(
    							params.mediaUrl,
    							params.rawThumbFile,
    							params.reqWidth,
    							params.reqHeight,
    							params.timeTable.length,
    							params.timeTable,
    							params.flags,
    							nextTask.getTaskId());
    				}else {
    					if(LL.D) Log.d(LOG_TAG,"startPendingThumbnailTask range time sThumbnailsRunTaskId=" + nextTask.getTaskId());

    					editor.makeVideoThumbs(
    							params.mediaUrl,
    							params.rawThumbFile,
    							params.reqWidth,
    							params.reqHeight,
    							params.startTime,
    							params.endTime,
    							params.reqCount,
    							params.flags,
    							nextTask.getTaskId());
    				}
    			} catch (IOException e) {
    				nextTask.sendFailure(Task.makeTaskError(e));
    			}
    		}else{
    //            sThumbnailsRunTaskId = 0;
    //            sThumbnailCancel = false;
                if(LL.D) Log.d(LOG_TAG,"startPendingThumbnailTask all run end.");
            }
        }
    }

	//yoon
	private static NexEditor.OnGetThumbDoneListener sThumbDoneListener =  new NexEditor.OnGetThumbDoneListener() {
		@Override
		public void onGetThumbDoneListener(int iMode, int iTime, int iWidth, int iHeight, int iSize, byte[] arrayData, int iCount, int iTotal, int iTag) {
			MediaInfoTask<?,?> task = sActiveTasks.get(iTag);
			if(LL.D) Log.d(LOG_TAG,"sThumbDoneListener onGetThumbDoneListener : tag=" + iTag );
			if( task!=null ) {
				if(LL.D) Log.d(LOG_TAG,"sThumbDoneListener NOTIFY TASK!");
				//task.notifyOnEditorAsyncDoneListener(resultCode);
				task.notifyOnThumbDoneListener(iMode, iTime, iWidth, iHeight, iSize, arrayData,iCount,iTotal);
			}
		}
	};

	private static OnGetClipInfoDoneListener sClipInfoDoneListener = new OnGetClipInfoDoneListener(){
		@Override
		public void onGetClipInfoDone(ErrorCode resultCode, int tag) {
			MediaInfoTask<?,?> task = sActiveTasks.get(tag);
			if(LL.D) Log.d(LOG_TAG,"sClipInfoDoneListener onGetClipInfoDone : tag=" + tag + " resultCode=" + resultCode);
			if( task!=null ) {
				if(LL.D) Log.d(LOG_TAG,"sClipInfoDoneListener NOTIFY TASK!");
				task.notifyOnEditorAsyncDoneListener(resultCode);
			}
		}
	};
	
	private static OnEditorDestroyedListener sOnEditorDestroyedListener = new OnEditorDestroyedListener() {
		
		@Override
		public void onEditorDestroyed() {
			if(LL.D) Log.d(LOG_TAG,"sOnEditorDestroyedListener onEditorDestroyed");
			for( int i=0; i<sActiveTasks.size(); i++ ) {
				sActiveTasks.valueAt(i).sendFailure(NexEditor.ErrorCode.EDITOR_INSTANCE_DESTROYED);
			}
			sActiveTasks = new SparseArray<MediaInfoTask<?,?>>();
			sPendingThumbnailTasks = new ArrayDeque<>(); //yoon add
			sPendingPCMLevelParcelTasks = new ArrayDeque<>();
			sThumbnailDetailSerial = 0;
		}
	};
	
	private static OnIdleListener sIdleListener = new OnIdleListener() {
		@Override
		public void onIdle() {
			if(LL.D) Log.d(LOG_TAG,"sIdleListener onIdle!");
			startPendingPCMLevelTask();
			startPendingThumbnailTask();
			startPendingPCMLevelParcelTask();
		}
	}; 

}
