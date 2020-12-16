package com.nexstreaming.kminternal.kinemaster.transcoder;

import java.io.File;
import java.lang.ref.WeakReference;
import java.security.InvalidParameterException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.content.Context;
import android.util.Log;

import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.app.common.task.Task.Event;
import com.nexstreaming.app.common.task.Task.OnTaskEventListener;
import com.nexstreaming.kminternal.kinemaster.mediainfo.MediaInfo;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.kinemaster.config.NexExportProfile;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor.ErrorCode;

/*
 * NexExportProfile[] supportedProfiles = NexExportProfile.supportedProfiles();
 * 
 * NexExportProfile useProfile = supportedProfiles[0];
 * 
 * for( NexExportProfile p: supportedProfiles ) {
 * 	  //if( p.height() == 720 ) {
 *    //   useProfile = p;
 *    //   break;
 * 	  //}
 *    if( p.height() > useProfile.height() ) {
 *       useProfile = p;
 *    }
 * }
 * 
 * TranscodingTask task = Transcoder.transcodeFile( srcPath, src + ".720.mp4" , useProfile );
 * 
 */

public class Transcoder {
	
	private static final String LOG_TAG = "Transcoder";

	private Transcoder() {
	}
	
	private static NexEditor getEditor() {
		return EditorGlobal.getEditor();
	}
	
	private static List<TranscodingTask> s_pendingTasks = new ArrayList<TranscodingTask>();
	private static Context s_appContext = null;
	private static TranscodingTask s_currentTask = null;

    public static void setApplicationContext( Context ctx ) {
        if( s_appContext == null ) {
            s_appContext = ctx.getApplicationContext();
        }
    }

	public static boolean isTranscoderBusy() {
		return s_currentTask!=null || s_pendingTasks.size() > 0;
	}

	private static TranscodingTask transcodeFile_internal( Context ctx, File sourcePath, File destinationPath, NexExportProfile exportProfile ) {
		return transcodeFile_internal(ctx, sourcePath, new File(destinationPath.getAbsolutePath() + ".temp"), destinationPath, exportProfile);
	}

	private static TranscodingTask transcodeFile_internal( Context ctx, File sourcePath, File tempPath, File destinationPath, NexExportProfile exportProfile ) {
		
		if( s_appContext == null ) {
			s_appContext = ctx.getApplicationContext();
		}
		
		// Validate parameters
		if( sourcePath==null ) {
			throw new InvalidParameterException("null source path");
		}
		if( destinationPath==null ) {
			throw new InvalidParameterException("null destination path");
		}
		if( tempPath==null ) {
			throw new InvalidParameterException("null temp path");
		}
		if( exportProfile==null ) {
			throw new InvalidParameterException("null export profile");
		}
		
		TranscodingTask result = new TranscodingTask(sourcePath, tempPath, destinationPath, exportProfile);

		if( LL.D ) Log.d(LOG_TAG,"transcodeFile; task=" + result + " sourcePath=" + sourcePath + " destinationPath=" + destinationPath + " tempPath=" + tempPath);
		
		if( !sourcePath.exists() ) {
			result.sendFailure(NexEditor.ErrorCode.SOURCE_FILE_NOT_FOUND);
		} else if( destinationPath.exists() ) {
			result.sendFailure(NexEditor.ErrorCode.DESTINATION_FILE_ALREADY_EXISTS);
		} else if( tempPath.exists() ) {
			tempPath.delete();
			s_pendingTasks.add(result);
			updateTasks();
//			result.sendFailure(NexEditor.ErrorCode.TEMP_FILE_ALREADY_EXISTS);
		} else {
			s_pendingTasks.add(result);
			updateTasks();
		}
		
		return result;
	}
	
	private static NexEditor.OnTranscodingListener s_transcodingListener = new NexEditor.OnTranscodingListener() {
		
		@Override
		public void onTranscodingProgress(int percent, int currentTime, int maxTime) {
			if( s_currentTask!=null ) {
				s_currentTask.setProgress(currentTime, maxTime);
			}
		}
		
		@Override
		public void onTranscodingDone(ErrorCode result, int userTag) {
			if( LL.D ) Log.d(LOG_TAG,"onTranscodingDone : result=" + result + " userTag=" + userTag + " s_currentTask=" + s_currentTask);
			if( s_currentTask!=null ) {
				if( result==ErrorCode.THUMBNAIL_BUSY ) {
					s_currentTask.signalTranscodingRetry();
					s_pendingTasks.add(s_currentTask);
					s_currentTask = null;
					MediaInfo.waitForMediaTaskNotBusy().onComplete(new OnTaskEventListener() {
						@Override
						public void onTaskEvent(Task t, Event e) {
							updateTasks();
						}
					});
				} else if( result==ErrorCode.TRANSCODING_USER_CANCEL) {
					File tempPath = s_currentTask.getTempPath();
					if( tempPath.exists() ) {
						tempPath.delete();
					}
					if(LL.I) Log.d(LOG_TAG, "onTranscodingDone      result==ErrorCode.TRANSCODING_USER_CANCEL!!!!!!!!!!!!!!!!!!!!!!");
					s_currentTask.signalEvent(Task.Event.CANCEL);
				} else if( result.isError() ) {
					File tempPath = s_currentTask.getTempPath();
					if( tempPath.exists() ) {
						tempPath.delete();
					}
					s_currentTask.sendFailure(result);
				} else {
					File tempPath = s_currentTask.getTempPath();
					if( tempPath.exists() ) {
						File destPath = s_currentTask.getDestinationPath();
						if( LL.D ) Log.d(LOG_TAG,"onTranscodingDone : before rename T=" + tempPath.exists() + " D=" + destPath.exists());
						tempPath.renameTo(destPath);
						if( LL.D ) Log.d(LOG_TAG,"onTranscodingDone : after rename T=" + tempPath.exists() + " D=" + destPath.exists());
						s_currentTask.signalEvent(Task.Event.SUCCESS,Task.Event.COMPLETE);
					} else {
						s_currentTask.sendFailure(NexEditor.ErrorCode.FILE_MISSING);
					}
					if(LL.I) Log.d(LOG_TAG, "onTranscodingDone      Task.Event.SUCCESS,Task.Event.COMPLETE!!!!!!!!!!!!!!!!!!!!!!");
				}
				s_currentTask = null;
			}
			updateTasks();
		}
	};

	private static void updateTasks() {
		final NexEditor nexEditor = getEditor();
		
		nexEditor.setOnTranscodingListener(s_transcodingListener);
		
		if( nexEditor.isTranscoding() || s_pendingTasks.isEmpty() ) {
			if( LL.D ) Log.d(LOG_TAG,"updateTasks : no task to start : isTranscoding=" + nexEditor.isTranscoding() + " pendingSize=" + s_pendingTasks.size());
			return;
		}
		
		final TranscodingTask task = s_pendingTasks.remove(0);
		
		long longFreeSpace = task.getTempPath().getParentFile().getFreeSpace();
		final long freeSpace = longFreeSpace;
		final NexExportProfile exportProfile = task.getExportProfile();
		
		if( LL.D ) Log.d(LOG_TAG,"Check color format before starting task : " + task.getSourcePath().getName() + " task=" + task);
		
		nexEditor.detectAndSetEditorColorFormat(s_appContext).onComplete(new OnTaskEventListener() {
			
			@Override
			public void onTaskEvent(Task t, Event e) {
				
				if( LL.D ) Log.d(LOG_TAG,"Start task for : " + task.getSourcePath().getName() + " task=" + task);
//				KMUsage.Transcode_Start.logEvent();
				task.onSuccess(transcodeSuccessReporter);
				task.onCancel(transcodeCancelReporter);
				task.onFailure(transcodeFailureReporter);
				NexEditor.ErrorCode result = nexEditor.transcodingStart_internal(
						task.getSourcePath().getAbsolutePath(),
						task.getTempPath().getAbsolutePath(),
						exportProfile.width(), exportProfile.height(),
						exportProfile.width(), exportProfile.height(),
						exportProfile.bitrate(), freeSpace, 30, 0,
						EditorGlobal.getProjectEffect("up"));
				if( result.isError() ) {
					if( LL.D ) Log.d(LOG_TAG,"Error : " + result.name());
					task.sendFailure(result);
					updateTasks();
				} else {
					if( LL.D ) Log.d(LOG_TAG,"Started; signalling task");
					task.signalTranscodingStart();
					task.setProgress(0, 100);
					s_currentTask = task;
				}

			}
		});
		

	}

	private static OnTaskEventListener transcodeSuccessReporter = new OnTaskEventListener() {
		@Override
		public void onTaskEvent(Task t, Event e) {
			((TranscodingTask)t).signalTranscodingDone();
//			KMUsage.Transcode_Result.logEvent("result","success");
		}
	};

	private static OnTaskEventListener transcodeCancelReporter = new OnTaskEventListener() {
		@Override
		public void onTaskEvent(Task t, Event e) {
			((TranscodingTask)t).signalTranscodingDone();
//			KMUsage.Transcode_Result.logEvent("result","cancel");
		}
	};

	private static Task.OnFailListener transcodeFailureReporter = new Task.OnFailListener() {
		@Override
		public void onFail(Task t, Event e, Task.TaskError failureReason) {
			((TranscodingTask)t).signalTranscodingDone();
//			KMUsage.Transcode_Result.logEvent("result", "fail", "failureReason", failureReason.getMessage());
		}
	};

	public static void cancelAllTasks() {
		if( LL.D ) Log.d(LOG_TAG,"cancelAllTasks");
		for( Task t: s_pendingTasks ) {
			t.sendFailure(NexEditor.ErrorCode.TRANSCODING_ABORTED);
		}
		s_pendingTasks.clear();
		if( s_currentTask != null ) {
			s_currentTask.sendFailure(NexEditor.ErrorCode.TRANSCODING_ABORTED);
		}
		s_currentTask = null;
		NexEditor nexEditor = getEditor();
		if( nexEditor!=null ) {
			if( nexEditor.isTranscoding() ) {
				nexEditor.transcodingStop_internal();
				if( LL.D ) Log.d(LOG_TAG,"called transcodingStop_internal");
			} else {
				if( LL.D ) Log.d(LOG_TAG,"not currently transcoding");
			}
		}
	}
	
	static void cancelTask( TranscodingTask task ) {
		if( task==null ) {
			return;
		}
		if( LL.D ) Log.d(LOG_TAG,"cancelTask; task=" + task);
		if( task==s_currentTask ) {
			NexEditor nexEditor = getEditor();
			nexEditor.transcodingStop_internal();
			File dest = s_currentTask.getTempPath();
			if( dest.exists() ) {
				dest.delete();
			}
			task.signalEvent(Task.Event.CANCEL);
			// s_currentTask = null;
			// updateTasks();
		} else {
			if( LL.D ) Log.d(LOG_TAG,"cancelTask not current");
			if(s_pendingTasks.remove(task)) {
				if(LL.I) Log.d(LOG_TAG, "s_pendingTasks.remove(task)      Task.Event.CANCEL!!!!!!!!!!!!!!!!!!!!!!");
				task.signalEvent(Task.Event.CANCEL);
			}
		}
	}

    private static Map<File,WeakReference<TranscodingTask>> sTranscodingTasks = new HashMap<File,WeakReference<TranscodingTask>>();

    private static void removeUnreferencedTasks() {
        List<File> toRemove = new ArrayList<File>();
        for( Map.Entry<File, WeakReference<TranscodingTask>> e: sTranscodingTasks.entrySet() )  {
            if( e.getValue().get() == null ) {
                toRemove.add(e.getKey());
            }
        }
        for( File key: toRemove )
            sTranscodingTasks.remove(key);
    }

    public static TranscodingTask transcodeFile( Context ctx, File sourcePath, File destinationPath, NexExportProfile exportProfile ) {
        WeakReference<TranscodingTask> refExistingTask = sTranscodingTasks.get(sourcePath);
        if( refExistingTask!=null ) {
            TranscodingTask existingTask = refExistingTask.get();
            if( existingTask != null && existingTask.isRunning() )
                return existingTask;
        }

        TranscodingTask newTask =  transcodeFile_internal(ctx, sourcePath, destinationPath, exportProfile);
        sTranscodingTasks.put(sourcePath, new WeakReference<TranscodingTask>(newTask));
        removeUnreferencedTasks();
        return newTask;
    }

    public static TranscodingTask findTranscodeTask(File sourcePath) {
        WeakReference<TranscodingTask> refExistingTask = sTranscodingTasks.get(sourcePath);
        if( refExistingTask!=null ) {
            TranscodingTask existingTask = refExistingTask.get();
            if( existingTask != null ) {
                if(existingTask.isRunning())
                    return existingTask;
                else if(existingTask.getDestinationPath().exists())
                    return existingTask;
            }
        }
        return null;
    }

}


