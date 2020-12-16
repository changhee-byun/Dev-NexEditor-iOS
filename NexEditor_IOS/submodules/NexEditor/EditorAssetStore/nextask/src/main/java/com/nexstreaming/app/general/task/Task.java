package com.nexstreaming.app.general.task;

import java.lang.ref.WeakReference;
import java.security.InvalidParameterException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicBoolean;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.util.Log;
import android.util.Pair;
import android.util.SparseArray;

/**
 * \package com.nexstreaming.kinemaster.task
 * 
 * Classes for working with asynchronous operations, which allow multiple
 * separate event handlers to be attached to individual tasks.  These are particularly
 * useful for working with one-time background tasks.
 * 
 * A function that would normally block and return a result can instead return a Task 
 * representing the work to be done, perform the work in a separate thread, and signal
 * that task when the work has completed.
 * 
 * The main class is the Task class (see that class documentation for more information).
 * 
 */

/**
 * Represents asynchronous operations, and allows multiple event handlers
 * for those operations to be attached.<p>
 * 
 * A Task object or subclass thereof does not actually perform any asynchronous work.  The actual
 * asynchronous work is separate and independent of the Task class, and could be anything from
 * an AsyncTask or Thread to a completely separate application that communicates via IPC.  The purpose
 * of the Task class is to provide a uniform interface to all of the different types of asynchronous
 * work, independent of the underlying implementation, and allowing multiple listeners to be 
 * attached to events from the same task. <p>
 * 
 * For example, consider the following hypothetical blocking function that takes an indeterminate
 * amount of time (because it waits for data from the network):<p>
 * <pre>
 * void refreshThemesFromServer() {...}
 * </pre>
 * 
 * The normal approach to changing this into a non-blocking function would be to pass a 
 * listener to this function.  But what if you need to attach multiple listeners?  The simplest way 
 * is to return a Task object, to which listeners can be attached:<p>
 * <pre>
 * Task refreshThemesFromServer() {...}
 * </pre> 
 * 
 * This can be used as follows:<p>
 * <pre>
 * Task t = refreshThemesFromServer();
 * 
 * t.onComplete( new Task.OnTaskEventListener {
 *     public void onTaskEvent(Task t, Event e) {
 *     		// This code runs when the task is complete
 *     }
 * });
 * 
 * t.onComplete( new Task.OnTaskEventListener {
 *     public void onTaskEvent(Task t, Event e) {
 *     		// This code ALSO runs when the task is complete
 *     }
 * });
 * </pre>
 * 
 * As you can see, multiple listeners can be registered for the same task and the same event.<p>
 * 
 * There are several common patterns for functions that return tasks.  The most common one is to use
 * an AsyncTask to actually do the background work:<p>
 * <pre>
 * Task refreshThemesFromServer() {
 *    final Task task = new Task();
 * 
 *    new AsyncTask<P,S,R>(){
 *	  
 *        {@literal @}Override
 *        protected R doInBackground(P... params) {
 *            // do background work
 *        } 
 *        
 *        {@literal @}Override
 *        protected void onPostExecute(R result) {
 *            task.signalEvent(Task.Event.COMPLETE);
 *        }
 *    }.execute();
 *        
 *    return task;
 * }
 * </pre>
 * 
 * Whether you use an AsyncTask or some other means to perform the asynchronous operation, the
 * general approach is the same:  Return a Task object immediately, and later call signalEvent
 * to notify the listeners on that object.<p>
 * 
 * It is also okay to signal a task before returning it.  In this case, the listeners are called
 * as soon as they are registered (since the relevant event is in the past).  This keeps events
 * from being missed.<p>
 * 
 * For a function that would normally return a result, you can use ResultTask rather than Task.  For
 * example, consider a function that searches for a string in a large file and returns the first
 * line containing a match.  A blocking version of the function might look like this:<p>
 * <pre>
 * String findInFile(File path, String target) {
 *     // search the file for the first line containing "target"
 *     return matchingLine;
 * }
 * </pre>
 * 
 * This can be made asynchronous as follows:<p>
 * <pre>
 * ResultTask&lt;String&gt; findInFile(final File path, final String target) {
 *    final ResultTask&lt;String&gt; task = new ResultTask&lt;String&gt;();
 * 
 *    new AsyncTask<Void,Void,String>(){
 *	  
 *        {@literal @}Override
 *        protected String doInBackground(Void... params) {
 *            // search the file for the first line containing "target"
 *            return matchingLine;
 *        } 
 *        
 *        {@literal @}Override
 *        protected void onPostExecute(String result) {
 *            task.sendResult(result);
 *        }
 *    }.execute();
 *        
 *    return task;
 * }
 * </pre>
 * 
 * Sometimes, it's desirable to perform a given task only once.  For example, if there are multiple
 * requests to download a file, it may be better to download the file only once and provide a single
 * task that multiple listeners can attach to.  This is very easy:<p>
 * <pre>
 * private Task m_existingTask = null;
 * Task downloadBigFile() {
 *     if( m_existingTask!=null &&
 *     	   (m_existingTask.isRunning() || m_existingTask.didSignalEvent(Task.Event.COMPLETE))) 
 *     {
 *         return m_existingTask;
 *     }
 *     
 *     m_existingTask = new Task();
 *     
 *     // ... start asynchronous operation, for example with AsyncTask
 *     
 *     return m_existingTask;
 * }
 * </pre>
 * 
 * Using the approach above, if the task has stopped for some other reason (for example, a failure)
 * then calling downloadBigFile() will cause the task to be run again (retrying the operation).  If
 * the task has succeeded or is still running, the existing task is returned.<p>
 * 
 * Generally, a function that would normally take a long time to complete can, rather than blocking
 * and returning a result, can instead immediately return a Task or ResultTask that can be used
 * to attach listeners.<p> 
 * 
 * Many functions that perform one-time asynchronous or background operations 
 * return a Task object (or a subclass of Task, such as ResultTask) that represents
 * the operation.<p>
 * 
 * When an asynchronous operation completes, an event is signaled on the associated
 * Task object.  Event handlers (listeners) that execute on the UI thread can be
 * associated with Task object events, and they are called when the event is
 * signaled (if an event has already been signaled, the event handler will be
 * called immediately upon registration).<p>
 * 
 * There are several events that are normally signaled on a Task:<p>
 * 
 * - {@link Event#SUCCESS SUCCESS} means that the task has succeeded.  For example, when requesting
 *  								video playback, this event happens when the play command succeeds
 *  								and playback actually starts.
 * - {@link Event#COMPLETE COMPLETE} means that the task has completed successfully.  For most tasks,
 *  								this event and the SUCCESS event are sent together.  However, for
 *  								a task like video playback, SUCCESS happens when video playback
 *  								starts, and COMPLETE happens when the end of content is reached
 *  								(and COMPLETE may not happen if playback is stopped before
 *  								reaching the end of content).
 * - {@link Event#RESULT_AVAILABLE RESULT_AVAILABLE} applies only to the ResultTask subclass.  This happens when there
 *  								is a result available (for example, when making a thumbnail, if
 *  								a thumbnail bitmap is available).  Usually, this event is sent
 *  								together with SUCCESS and COMPLETE.
 * - {@link Event#CANCEL CANCEL} is sent if a task is cancelled.
 * - {@link Event#FAIL FAIL} is sent if a task fails.
 * - {@link Event#PROGRESS PROGRESS} is the only event that can occur multiple times for a given task.
 *  								For tasks that have meaningful progress (for example, download
 * 									percentage when downloading a file, or the current playhead time
 * 									when playing video).  Some tasks do not report progress.
 *  
 *  The precise meaning and use of each event depends on the particular task.  Check the documentation
 *  for individual functions that return Task objects for details on how to use them.<p>
 *  
 *  <b>THREAD SAFETY WARNING:</b> This class is generally <i>not thread safe</i>.  All methods (including the
 *  constructor) must be called only from the UI thread.  If you are implementing a method that returns
 *  a task, use standard android patterns to communicate status back to wrappers on the UI thread, which
 *  in turn signal the Task object appropriately.  The recommended way is to use Android's <i>AsyncTask</i> class
 *  for tasks implemented in Java, or Android's <i>Handler</i> class for tasks implemented in native code.<p>
 *  
 *  <b>USE IN NON-UI THREADS:</b>  This class can be used in non-UI threads, but under very specific 
 *  conditions only:
 *  - The constructor and all signaling methods must be called from the same thread
 *  - No listeners may be attached
 *  - Instead of listeners, a single other thread may wait on the task using awaitTaskCompletion() or the equivalent ResultTask method.
 *  - Before waiting on the task, call makeWaitable.
 *
 */
public class Task {
	
	private static int sTaskId = 0;
	private static long sLongTaskId = (SystemClock.uptimeMillis() & 0xFFFFFFFF) << 32;
	private static Map<Long,WeakReference<Task>> sLongIdTaskMap = new HashMap<Long,WeakReference<Task>>();
	private static SparseArray<WeakReference<Task>> sIntIdTaskMap = new SparseArray<WeakReference<Task>>();
	private static int sRegisterCount = 0;

	public static final Task COMPLETED_TASK = new Task(Event.COMPLETE,Event.SUCCESS);

	/**
	 * Interface implemented by classes that can be used in calls to {@link Task#sendFailure(TaskError) sendFailure}.
	 * 
	 * Any object can be used, as long as it implements this interface.  For example, if the set of possible errors is fixed,
	 * an enum could be defined:
	 * 
	 * ~~~~~~~~~~~{.java}
	 * public enum MyErrorCode implements TaskError {
	 *     FILE_NOT_FOUND, INSUFFICIANT_MEMORY, SOME_ERROR, OTHER_ERROR;
	 *     
	 *     @Override
	 *     Exception getException() {
	 *         return null;
	 *     }
	 *     
	 *     @Override
	 *     String getMessage() {
	 *         return name();
	 *     }
	 * }
	 * ~~~~~~~~~~~
	 * 
	 * Or in the case of an exception, an anonymous class could be created wrapping the exception.
	 *
	 */
	public interface TaskError {
		/**
		 * Returns the exception associated with this error, if there is one.
		 * @return	An exception, or null if none applies.
		 */
		Exception getException();
		
		/**
		 * Returns a description of the error. 
		 * @return A string describing the error (must not return null).
		 */
		String getMessage();
		
		/**
		 * Returns a localized description of the error, if one is available.
		 * 
		 * If no localized description is available, this must return the same
		 * value that getMessage() woruld return.
		 * 
		 * @return A string describing the error (must not return null).
		 */
		String getLocalizedMessage(Context context);		
	}
	
	/**
	 * An exception that wraps a TaskError object.
	 * 
	 */
	public static class TaskErrorException extends Exception {
		
		private static final long serialVersionUID = 1L;
		private final TaskError taskError;
		
		/**
		 * Constructs a new exception based on the specified task error
		 * @param taskError
		 */
		public TaskErrorException(TaskError taskError) {
			super(taskError.getMessage());
			this.taskError = taskError;
		}
		
		/**
		 * Constructs an exception caused by another exception, and wrapping
		 * the specified task error.
		 * @param taskError
		 */
		public TaskErrorException(TaskError taskError, Throwable cause) {
			super(taskError.getMessage(),cause);
			this.taskError = taskError;
		}
		
		public TaskError getTaskError() {
			return taskError;
		}
	}
	
	/**
	 * Simple implementation of the TaskError interface.
	 * 
	 * Use this if you just need to wrap an exception or error message.
	 * 
	 * This returns the exception's error message if an explicit error
	 * message was not given.
	 * 
	 * @author matthew
	 *
	 */
	public static class SimpleTaskError implements TaskError {

		private final String mMessage;
		private final Exception mException;
		public SimpleTaskError( Exception e, String message) {
			mException = e;
			mMessage = message;
		}
		
		@Override
		public Exception getException() {
			return mException;
		}

		@Override
		public String getMessage() {
			if( mMessage!=null ) {
				return mMessage;
			} else if( mException.getMessage()!=null ) {
				return mException.getMessage();
			} else {
				return "Unknown";
			}
		}

		@Override
		public String getLocalizedMessage(Context context) {
			if( mMessage!=null ) {
				return mMessage;
			} else if( mException.getLocalizedMessage()!=null ) {
				return mException.getLocalizedMessage();
			} else if( mException.getMessage()!=null ) {
				return mException.getMessage();
			} else {
				return "Unknown";
			}
		}
		
		@Override
		public String toString() {
			if( getException()!=null ) {
				return "<SimpleTaskError: " + getException().getClass().getName() + ">";

			} else {
				return "<SimpleTaskError: " + getMessage() + ">";
			}
		}
		
	}
	
	@Override
	public String toString() {
		String events = "";
		boolean first = true;
		for( Event e: mSignalledEvents ) {
			events = events + (first?"":",") + e.name();
			first = false;
		}
		if( events.length()<1 ) {
			events = "none";
		}
		if( mProgressSignalled ) {
			return "[Task " + mTaskId + ": progress=" + mProgress + "/"+ mMaxProgress + " events=" + events + "]";
		} else {
			return "[Task " + mTaskId + ": events=" + events + "]";
		}
	}
	
	/**
	 * Convenience method to make a TaskError wrapping an exception.
	 * @param e
	 * @return
	 */
	public static TaskError makeTaskError( Exception e ) {
		return new SimpleTaskError(e, null);
	}
	
	/**
	 * Convenience method to make a TaskError wrapping a message and an exception.
	 * @param message
	 * @param e
	 * @return
	 */
	public static TaskError makeTaskError( String message, Exception e ) {
		return new SimpleTaskError(e, message);
	}
	
	/**
	 * Convenience method to make a TaskError wrapping a message.
	 * @param message
	 * @return
	 */
	public static TaskError makeTaskError( String message ) {
		return new SimpleTaskError(null, message);
	}

	/**
	 * Convenience method to make a failed task wrapping an exception.
	 * @param e
	 * @return
	 */
	public static Task makeFailedTask( Exception e ) {
		Task task = new Task();
		task.sendFailure(new SimpleTaskError(e, null));
		return task;
	}

	/**
	 * Convenience method to make a failed task wrapping a message and an exception.
	 * @param message
	 * @param e
	 * @return
	 */
	public static Task makeFailedTask( String message, Exception e ) {
		Task task = new Task();
		task.sendFailure(new SimpleTaskError(e, message));
		return task;
	}

	/**
	 * Convenience method to make a failed task wrapping a message.
	 * @param message
	 * @return
	 */
	public static Task makeFailedTask( String message ) {
		Task task = new Task();
		task.sendFailure(new SimpleTaskError(null, message));
		return task;
	}

	/**
	* Convenience method to make a failed Task wrapping a TaskError.
	* @param failureReason
	* @return
	*/
	public static Task makeFailedTask( TaskError failureReason) {
		Task task = new Task();
		task.sendFailure(failureReason);
		return task;
	}


	/**
	 * Used when {@link Event#FAIL FAIL} is signaled without a specific failure reason provided.
	 */
	public static final TaskError UNKNOWN_ERROR = new TaskError(){

		@Override
		public Exception getException() {
			return null;
		}

		@Override
		public String getMessage() {
			return "Unknown";
		}

		@Override
		public String getLocalizedMessage(Context context) {
			return getMessage();
		}
		
	};

	/**
	 * Used when a timeout occurs.
	 */
	public static final TaskError TIMEOUT = new TaskError(){

		@Override
		public Exception getException() {
			return null;
		}

		@Override
		public String getMessage() {
			return "Timeout";
		}

		@Override
		public String getLocalizedMessage(Context context) {
			return getMessage();
		}
		
	};

	/**
	 * Used when awaiting a result and the task is signalled without a result.
	 */
	public static final TaskError NO_RESULT_AVAILABLE = new TaskError(){

		@Override
		public Exception getException() {
			return null;
		}

		@Override
		public String getMessage() {
			return "No result available";
		}

		@Override
		public String getLocalizedMessage(Context context) {
			return getMessage();
		}
		
	};
	
	/**
	 * Types of events that can be signaled on a Task.
	 * 
	 * For any given task, an event can only be signaled one time,
	 * except for the {@link #PROGRESS} event which can be signaled multiple
	 * times.
	 * 
	 * Note that RESULT_AVAILABLE only applies to the ResultTask subclass.
	 */
	public static enum Event {
		/**
		 * The SUCCESS event is signaled when the task has succeeded.
		 * 
		 * For most tasks, COMPLETE is signaled immediately after SUCCESS.  However, some
		 * tasks may signal COMPLETE at a later time.
		 * 
		 * For example, a file download task would probably signal SUCCESS and COMPLETE together.  But a video
		 * playback task might signal SUCCESS when playback successfully starts, and COMPLETE when the end of
		 * the media is reached.
		 * 
		 * Functions that return Task objects should mention in their documentation if COMPLETE is sent separately.
		 * 
		 * If a task that has not signaled SUCCESS or FAIL tries to signal COMPLETE, then SUCCESS will
		 * automatically be signaled first.
		 * 
		 */
		SUCCESS, 
		/**
		 * The FAIL event is signaled when a task has failed.
		 * 
		 * Although this can be signaled directly, it is better to call {@link Task#sendFailure(TaskError) sendFailure}
		 * instead to include error information (if FAIL is signaled directly, {@link Task#UNKNOWN_ERROR UNKNOWN_ERROR}
		 * will be automatically set).
		 */
		FAIL, 
		/**
		 * The COMPLETE event is signaled after a task has succeeded and completed.
		 * 
		 * This means it comes after the SUCCESS event.
		 * 
		 * For most tasks, success and completion are the same thing.  But for some tasks, completion may be sent later.  
		 * For example a media playback task may send SUCCESS as soon as playback successfully starts, and COMPLETE when
		 * the end of content is reached.
		 */
		COMPLETE, 
		/**
		 * The CANCEL event is signaled when a task is canceled.
		 * 
		 * Cancellation doesn't apply to all tasks.  For some tasks cancellation can happen before SUCCESS is sent (for example,
		 * canceling a file download), and for some tasks CANCEL could be signaled after SUCCESS (for example, if a stop 
		 * request is sent during media playback).
		 */
		CANCEL, 
		/**
		 * This event is signaled every time progress is updated.
		 * 
		 * To get the current progress, call {@link Task#getProgress() getProgress()}.
		 * 
		 * Do not try to signal this event directly; instead call {@link Task#setProgress(int, int) setProgress(int, int)}. 
		 */
		PROGRESS, 
		/**
		 * This event applies only to \link ResultTask<T> ResultTask\endlink, and is signaled when a result is available on the result task.
		 * This is signalled only once.
		 */
		RESULT_AVAILABLE,
		/**
		 * This event applies only to \link ResultTask<T> ResultTask\endlink, and is signaled when a result is available on the result task.
		 * This can be signalled repeatedly if 
		 */
		UPDATE_OR_RESULT_AVAILABLE
	}
	
	/**
	 * A listener interface for task events that
	 * don't have any special parameters (all events
	 * except for {@link Event#PROGRESS PROGRESS}, {@link Event#FAIL FAIL} and {@link Event#RESULT_AVAILABLE RESULT_AVAILABLE})
	 */
	public interface OnTaskEventListener {
		/**
		 * Called when a relevant task event becomes signaled
		 * @param t	The task on which the event was signaled
		 * @param e The event that was signaled
		 */
		public void onTaskEvent(Task t, Event e);
	}
	
	/**
	 * A listener interface for the {@link Event#PROGRESS PROGRESS} event.
	 */
	public interface OnProgressListener {
		/**
		 * Called when {@link Event#PROGRESS PROGRESS} is signaled (when {@link Task#setProgress(int, int) setProgress} is called).
		 * @param t				The task on which the event was signaled
		 * @param e 			The event that was signaled (always {@link Event#PROGRESS PROGRESS})
		 * @param progress		The current progress (from 0 to maxProgress)
		 * @param maxProgress	The maximum possible progress
		 */
		public void onProgress(Task t, Event e, int progress, int maxProgress );
	}
	
	/**
	 * A listener interface for the {@link Event#FAIL FAIL} event.
	 */
	public interface OnFailListener {
		/**
		 * Called when {@link Event#FAIL FAIL} is signaled.
		 * @param t				The task on which the event was signaled
		 * @param e 			The event that was signaled (always {@link Event#FAIL FAIL})
		 * @param failureReason	The reason for the failure, or {@link Task#UNKNOWN_ERROR UNKNOWN_ERROR} if the reason is not known.
		 */
		public void onFail(Task t, Event e, TaskError failureReason );
	}
	
	private List<Pair<OnTaskEventListener, Event>> mListeners = new LinkedList<Pair<OnTaskEventListener, Event>>();
	private EnumSet<Event> mSignalledEvents = EnumSet.noneOf(Event.class);
	private int mProgress = 0;
	private int mMaxProgress = 0;
	private boolean mProgressSignalled = false;
	private boolean mRegistered = false;
	private final int mTaskId;
	private final long mLongTaskId;
	private TaskError mTaskError = null;
	private AtomicBoolean mCancelRequested;
	private boolean mCancellable = false;
	private Handler mHandler = null;
	private Runnable mTimeoutRunnable = null;
	
	public Task setTimeout( long delayMillis ) {
		if( mHandler==null ) {
			mHandler = new Handler();
		}
		if( mTimeoutRunnable==null ) {
			mTimeoutRunnable = new Runnable() {
				@Override
				public void run() {
					if( isRunning() ) {
						sendFailure(TIMEOUT);
					}
					mTimeoutRunnable = null;
					mHandler = null;
				}
			};
		} else {
			mHandler.removeCallbacks(mTimeoutRunnable);
		}
		mHandler.postDelayed(mTimeoutRunnable, delayMillis);
		return this;
	}
	
	public void cancel() {
		if( !mCancellable ) {
			throw new RuntimeException("Not a cancellable task");
		}
		mCancelRequested.set(true);
	}
	
	public boolean isCancelRequested() {
		if( !mCancellable ) {
			throw new RuntimeException("Not a cancellable task");
		}
		return mCancelRequested.get();
	}
	
	public boolean isCancellable() {
		return mCancellable;
	}
	
	/**
	 * Controls whether or not a task can be canceled.
	 * 
	 * Since a Task object is an abstract representation, this doesn't actually affect the
	 * underlying asynchronous operation in anyway.  Rather, it simply indicates that the 
	 * asynchronous operation supports cancellation.<p>
	 * 
	 * To cancel a cancellable task, call cancel().<p>
	 * 
	 * It is the responsibility of the underlying asynchronous operation to poll the
	 * task's cancellation status periodically by calling isCancelRequested() and abort
	 * if necessary.  isCancelRequested() is threadsafe and can be called from a non-UI thread.
	 * 
	 * @param cancellable
	 */
	public void setCancellable( boolean cancellable ) {
		mCancellable = cancellable;
		if( mCancellable && mCancelRequested==null ) {
			mCancelRequested = new AtomicBoolean(false);
		} else if( !mCancellable ) {
			mCancelRequested = null;
		}
	}
	
	/**
	 * A special value that is guaranteed never to be
	 * returned by {@link #getTaskId()} or {@link #getLongTaskId()}.
	 */
	public static final int INVALID_TASK_ID = 0;
	
	/**
	 * Sole constructor for Task objects.<p>
	 * 
	 * There are no special parameters to pass when creating a task.  Functions that start
	 * asynchronous processes create and return Task objects to represent those processes.<p>
	 * 
	 * A reference to the relevant task object is also passed into the thread that is
	 * executing the asynchronous process.  When the process completes or fails, that thread can
	 * signal the task object, which will in turn notify the appropriate listeners.<p>
	 * 
	 * For tasks that don't have Java threads associated with them (for example, tasks handled in
	 * native code or in other cases where it may be difficult to keep a task reference), each task
	 * also has a unique integer ID.  See {@link #getTaskId} for details.
	 */
	public Task() {
		++sTaskId;
		++sLongTaskId;
		if( sTaskId==INVALID_TASK_ID ) {
			++sTaskId;
		}
		if( sLongTaskId==INVALID_TASK_ID ) {
			++sLongTaskId;
		}
		mTaskId = sTaskId;
		mLongTaskId = sLongTaskId;
	}

	private Task(Event... events) {
		this();
		signalEvent(events);
	}
	
	private CountDownLatch mLatch = null;
	public synchronized void makeWaitable() {
		if( mLatch==null ) {
			mLatch = new CountDownLatch(1);
		}
        if( !isRunning() )
            signalCompletionEvent();
	}
	
	public Task awaitTaskCompletion() {
		if( mLatch==null ) {
			throw new RuntimeException("Not a waitable task");
		} else if( Looper.getMainLooper().getThread() == Thread.currentThread() ) {
			throw new RuntimeException("Would block on UI thread");
		}
		boolean interrupted = false;
		try {
			while(true) {
				try {
					mLatch.await();
					break;
				} catch (InterruptedException e) {
					interrupted = true;
				}
			}
		} finally {
			if( interrupted ) {
				Thread.currentThread().interrupt();
			}
		}
		return this;
	}
	
	private void signalCompletionEvent() {
		if( mLatch!=null ) {
			mLatch.countDown();
		}
	}
	
	/**
	 * Make this task searchable using {@link #findTaskById} and {@link #findTaskByLongId}.
	 */
	public void register() {
		if( mRegistered ) {
			return;
		}
		sIntIdTaskMap.put(mTaskId, new WeakReference<Task>(this));
		sLongIdTaskMap.put(mLongTaskId, new WeakReference<Task>(this));
		
		sRegisterCount++;
		
		if( sRegisterCount > 32 ) {
			
			// Integer ids
			List<Integer> toRemoveInt = new ArrayList<Integer>();
			int count = sIntIdTaskMap.size();
			for(int i = 0; i<count; i++ ) {
				int key = sIntIdTaskMap.keyAt(i);
				WeakReference<Task> value = sIntIdTaskMap.valueAt(i);
				if( value==null || value.get()==null ) {
					toRemoveInt.add(key);
				}
			}
			for( Integer key: toRemoveInt ) {
				sIntIdTaskMap.remove(key);
			}
			
			// Long ids
			List<Long> toRemoveLong = new ArrayList<Long>();
			for( Entry<Long, WeakReference<Task>> e: sLongIdTaskMap.entrySet() ) {
				if( e.getValue()==null || e.getValue().get()==null ) {
					toRemoveLong.add(e.getKey());
				}
			}
			for( Long key: toRemoveLong ) {
				sLongIdTaskMap.remove(key);
			}
			
			sRegisterCount = 0;
			
		}
		
		mRegistered = true;
	}
	
	/**
	 * Find a task by the task ID.
	 * 
	 * The task ID is the ID returned by {@link #getTaskId()}.
	 * 
	 * This only works if {@link #register()} has been called for the task.
	 * 
	 * Note that unless a reference is kept elsewhere, a task may be garbage collected.
	 * 
	 * @param id	The ID of the task
	 * @return		The Task object with the specified ID, or `null` if no task exists with that ID (also `null` if the task was garbage collected).
	 */
	public static Task findTaskById( int id ) {
		WeakReference<Task> taskref = sIntIdTaskMap.get(id);
		if( taskref==null ) {
			return null;
		} else {
			return taskref.get();
		}
	}
	
	/**
	 * Find a task by the long task ID.
	 * 
	 * The long task ID is the ID returned by {@link #getLongTaskId()}.
	 * 
	 * This only works if {@link #register()} has been called for the task.
	 * 
	 * Note that unless a reference is kept elsewhere, a task may be garbage collected.
	 * 
	 * @param id	The ID of the task
	 * @return		The Task object with the specified ID, or `null` if no task exists with that ID (also `null` if the task was garbage collected).
	 */
	public static Task findTaskByLongId( long id ) {
		WeakReference<Task> taskref = sLongIdTaskMap.get(id);
		if( taskref==null ) {
			return null;
		} else {
			return taskref.get();
		}
	}
	
	/** 
	 * Each task has a unique integer ID that can be passed to native
	 * code or other places where it may be difficult to pass a task
	 * reference.<p>
	 * 
	 * Note that it is still necessary to keep an actual reference 
	 * (besides the ID) somewhere to prevent the Task object from being
	 * garbage collected.<p>
	 * 
	 * Generally, when a task ID is passed to native code, the task is
	 * placed in a List or Map so that it can be looked up later when
	 * a notification is received from native code.  Tracking the task
	 * in this way is beyond the scope of the Task class (it should be
	 * handled externally by the function, class or library that
	 * constructs the class).<p> 
	 * 
	 * If you need to store the task ID in intent parameters or in an instance
	 * state, you should use {@link #getLongTaskId} instead.<p>
	 *  
	 * @return		A unique integer ID for the task. This may be any
	 * 				value, but is guaranteed not to be {@link #INVALID_TASK_ID}
	 */
	public int getTaskId() {
		return mTaskId;
	}
	
	/**
	 * An alternative longer task ID that can be used in the special case
	 * where a task ID needs to be unique even if the process is destroyed
	 * and recreated (for example, if the task ID is included in a saved
	 * instance state bundle or with intent parameters).<p>
	 * 
	 * If you need to use the task ID as a key for a sparse array or other
	 *  {@link #getLongTaskId} instead.<p>
	 * 
	 * @return		A unique long ID for the task. This may be any
	 * 				value, but is guaranteed not to be {@link #INVALID_TASK_ID}
	 */
	public long getLongTaskId() {
		return mLongTaskId;
	}	
	/**
	 * Registers an event listener to be called when the given event
	 * occurs.<p>
	 * 
	 * This does not affect any event listeners that have already been
	 * registered (so if you register an event listener twice, it will
	 * be called twice).<p>
	 * 
	 * It is not possible to remove a registered listener (however, the
	 * listener will be called only once, when the event occurs).<p>
	 * 
	 * <b>NOTE:</b> If the event has already occurred, the listener 
	 * will be called immediately BEOFRE this function returns.  If the
	 * event has not yet occurred, the listener will be called on the
	 * UI thread when the event occurs (if the event does not occur,
	 * the listener will never be called).
	 * 
	 * @param event		The event to listen for
	 * @param listener	The listener to call when the event occurs
	 * @return			The original Task object (itself) to allow
	 * 					for chaining.
	 */
	public Task onEvent(Event event, OnTaskEventListener listener) {
		if( event==null || listener==null ) {
			return this;
		}
		
		if( didSignalEvent(event) ) {
			listener.onTaskEvent(this, event);
			return this;
		}
		
		mListeners.add(new Pair<OnTaskEventListener, Event>(listener,event));
		
		return this;
	}
	
	/**
	 * Checks if a given event can be repeated in the
	 * context of this given task.<p>
	 * 
	 * In the current version, only the PROGRESS event can
	 * be repeated, and it is considered repeatable for all
	 * tasks.  However, if the set of events is expanded in
	 * the future, this could change.<p>
	 * 
	 * @param e	Event to check
	 * @return	<i>true</i> if the event is repeatable, 
	 * 			<i>false</i> if the event occurs only once.
	 */
	public boolean isRepeatableEvent( Event e ) {
		return (e==Event.PROGRESS||e==Event.UPDATE_OR_RESULT_AVAILABLE);
	}
	
	/**
	 * Checks if an event has already occurred for this task.<p>
	 * 
	 * This always returns false for repeatable events (events
	 * for which {@link #isRepeatableEvent(Event)} returns <i>true</i>; currently
	 * only the PROGRESS event).
	 * 
	 * @param e	 	The event to check
	 * @return		<i>true</i> if the event has already occurred, 
	 * 				<i>false</i> if the event has not occurred.
	 */
	public boolean didSignalEvent( Event e ) {
		return mSignalledEvents.contains(e);
	}

	/**
	 * Shorthand for {@link #onEvent(Event, OnTaskEventListener) onEvent}`(listener,`{@link Event#SUCCESS Event.SUCCESS}`)` 
	 * @param listener	The listener to call when the {@link Event#SUCCESS SUCCESS} event occurs
	 * @return			The original Task object (itself) to allow
	 * 					for chaining.
	 */
	public Task onSuccess( OnTaskEventListener listener ) {
		return onEvent(Event.SUCCESS, listener);
	}
	
	/**
	 * Shorthand for {@link #onEvent(Event, OnTaskEventListener) onEvent}`(listener,`{@link Event#FAIL Event.FAIL}`)` 
	 * @param listener	The listener to call when the {@link Event#FAIL FAIL} event occurs
	 * @return			The original Task object (itself) to allow
	 * 					for chaining.
	 */
	public Task onFailure( final OnFailListener listener ) {
		return onEvent(Event.FAIL, new OnTaskEventListener() {
			
			@Override
			public void onTaskEvent(Task t, Event e) {
				listener.onFail(t, e, getTaskError());
			}
		});
	}

	/**
	 * Shorthand for forwarding all failures from one task to another
	 * @param other		The receiving task
	 * @return			The original Task object (itself) to allow
	 * 					for chaining.
	 */
	public Task onFailure( final Task other ) {

		if( other==this )
			throw new InvalidParameterException();

		return onEvent(Event.FAIL, new OnTaskEventListener() {
			@Override
			public void onTaskEvent(Task t, Event e) {
				other.sendFailure(getTaskError());
			}
		});
	}
	
	/**
	 * Shorthand for {@link #onEvent(Event, OnTaskEventListener) onEvent}`(listener,`{@link Event#COMPLETE Event.COMPLETE}`)` 
	 * @param listener	The listener to call when the {@link Event#COMPLETE COMPLETE} event occurs
	 * @return			The original Task object (itself) to allow
	 * 					for chaining.
	 */
	public Task onComplete( OnTaskEventListener listener ) {
		return onEvent(Event.COMPLETE, listener);
	}
	
	/**
	 * Shorthand for {@link #onEvent(Event, OnTaskEventListener) onEvent}`(listener,`{@link Event#CANCEL Event.CANCEL}`)` 
	 * @param listener	The listener to call when the {@link Event#CANCEL CANCEL} event occurs
	 * @return			The original Task object (itself) to allow
	 * 					for chaining.
	 */
	public Task onCancel( OnTaskEventListener listener ) {
		return onEvent(Event.CANCEL, listener);
	}

	/**
	 * Registers a listener for {@link Event#PROGRESS PROGRESS} events.<p>
	 * 
	 * This is almost a shorthand for {@link #onEvent(Event, OnTaskEventListener) onEvent}`(listener,`{@link Event#PROGRESS Event.PROGRESS}`)`
	 * except that <i>onEvent</i> takes a plain <i>OnTaskEventListener</i>, so this creates the appropriate OnTaskEventListener listener
	 * automatically, wrapping the supplied <i>OnProgressListener</i>.<p>
	 * 
	 * @param listener	The listener to call when the {@link Event#PROGRESS PROGRESS} event occurs
	 * @return			The original Task object (itself) to allow
	 * 					for chaining.
	 */
	public Task onProgress( final OnProgressListener listener ) {
		return onEvent(Event.PROGRESS, new OnTaskEventListener() {
			
			@Override
			public void onTaskEvent(Task t, Event e) {
				listener.onProgress(t, e, getProgress(), getMaxProgress());
			}
		});
	}
	
	/**
	 * Sets the current progress for this task, and automatically signals
	 * the {@link Event#PROGRESS} event.
	 * 
	 * @param progress		Tasks's current progress.  This value should be between 0 and <i>maxProgress</i>, inclusive.
	 * @param maxProgress	The maximum progress value for this task.
	 */
	public void setProgress( int progress, int maxProgress ) {
		mProgress = progress;
		mMaxProgress = maxProgress;
		mProgressSignalled = true;
		signalOneEvent(Event.PROGRESS);
	}
	
	/**
	 * Determine if progress information is available.
	 * 
	 * This is useful for determining if it's safe to call {@link #getProgress()}.
	 * 
	 * @return	`true` if progress information is available, otherwise `false`.
	 */
	public boolean isProgressAvailable() {
		return mProgressSignalled;
	}
	
	/**
	 * Gets the current progress value for the task.<p>
	 * 
	 * If the task has not yet signaled progress, this with throw a {@link ProgressNotAvailableException}.  The
	 * best way to avoid this is to call this function from the event listener for the {@link Event#PROGRESS PROGRESS} event.<p>
	 * 
	 * The progress value is arbitrary (not a percent value) and the meaning varies depending on the task (for example, 
	 * for a file copy operation this may be the number of bytes copied).  To calculate a percentage complete, compare this
	 * with the maximum progress as returned by {@link #getMaxProgress()}.
	 * 
	 * @return	The current progress as set with {@link #setProgress(int, int)}
	 */
	public int getProgress() {
		if( !mProgressSignalled ) {
			throw new ProgressNotAvailableException();
		}
		return mProgress;
	}
	
	/**
	 * Gets the maximum progress value for the task.<p>
	 * 
	 * If the task has not yet signaled progress, this with throw a {@link ProgressNotAvailableException}.  The
	 * best way to avoid this is to call this function from the event listener for the {@link Event#PROGRESS PROGRESS} event.<p>
	 * 
	 * @return	The current maximum progress as set with {@link #setProgress(int, int)}
	 */
	public int getMaxProgress() {
		if( !mProgressSignalled ) {
			throw new ProgressNotAvailableException();
		}
		return mMaxProgress;
	}
	
	/**
	 * Returns the reason for task failure.
	 * 
	 * This only applies to tasks that have signaled failure.  If a task has not signaled the {@link Event#FAIL FAIL} event,
	 * calling this function will raise an exception.
	 * 
	 * @return	The reason why the task failed.
	 */
	public TaskError getTaskError() {
		if( !didSignalEvent(Event.FAIL) ) {
			throw new RuntimeException("Error not available (task did not fail)");
		}
		return mTaskError==null?UNKNOWN_ERROR:mTaskError;
	}
	
	public void setTaskError(TaskError error) {
		mTaskError=error;
//		signalEvent(Task.Event.FAIL);
	}
	
	/**
	 * Signals the specified events.<p>
	 * 
	 * If the event has already been signaled, this does nothing.<p>
	 * 
	 * Signaling {@link Event#COMPLETE COMPLETE} also automatically signals {@link Event#SUCCESS SUCCESS} if no {@link Event#SUCCESS SUCCESS} or {@link Event#FAIL FAIL} event has
	 * been sent yet.<p>
	 * 
	 * All listeners registered for the specified event will be called before this function returns.<p>
	 * 
	 * The {@link Event#PROGRESS PROGRESS} event cannot be signaled directly; use {@link #setProgress(int, int)}
	 * instead (attempting to signal the PROGRESS event directly has no effect)
	 * 
	 * @param events
	 */
	public void signalEvent( Event... events ) {
		for( Event e: events ) {
			if( e != Event.PROGRESS ) {
				signalOneEvent(e);
			}
		}
	}
	
	/**
	 * Sets the error associated with a task, and signals {@link Event#FAIL FAIL} if it hasn't already been signaled.
	 * @param failureReason  The error to set on the task.  If this is null, {@link #UNKNOWN_ERROR} will be set.
	 */
	public void sendFailure( TaskError failureReason ) {
		setTaskError(failureReason);
		signalEvent(Task.Event.FAIL);
	}

	public synchronized void removeListenerForFail(){

		List<Pair<OnTaskEventListener, Event>> toNotify = new ArrayList<Pair<OnTaskEventListener, Event>>(8);
		for( Pair<OnTaskEventListener, Event> p: mListeners ) {
			if( p.second == Event.FAIL ) {
				toNotify.add(p);
			}
		}

		mListeners.removeAll(toNotify);
	}
	
	private synchronized void signalOneEvent( Event e ) {
		if( e==null || didSignalEvent(e) ) {
			return;
		}
		
		if( didSignalEvent(Event.CANCEL) ) {
			Log.w("Task", "Ingoring attempt to signal a cancelled task.");
			return;
		}
		
		if( e==Event.FAIL && didSignalEvent(Event.SUCCESS) ) {
			Log.w("Task", "Ingoring attempt to signal failure on task that already succeeded.");
			return;
		}
		
		if( e==Event.COMPLETE && !didSignalEvent(Event.SUCCESS)  && !didSignalEvent(Event.FAIL) && !didSignalEvent(Event.CANCEL) ) {
			signalOneEvent(Event.SUCCESS);
		}
		
		if( !isRepeatableEvent(e) ) {
			mSignalledEvents.add(e);
		}
		
		List<Pair<OnTaskEventListener, Event>> toNotify = new ArrayList<Pair<OnTaskEventListener, Event>>(8);
		for( Pair<OnTaskEventListener, Event> p: mListeners ) {
			if( p.second == e ) {
				toNotify.add(p);
			}
		}
		
		if( !isRepeatableEvent(e) ) {
			mListeners.removeAll(toNotify);
		}
		for( Pair<OnTaskEventListener, Event> p: toNotify ) {
			p.first.onTaskEvent(this, e);
		}
		if( e==Event.COMPLETE || e==Event.FAIL || e==Event.CANCEL ) {
			signalCompletionEvent();
		}
	}
	
	/**
	 * True if the task is running (that is, if it has not signaled COMPLETE, FAIL or CANCEL). <p>
	 * 
	 * Tasks are considered running as soon as they are created, even if they are held or queued
	 * for some time before starting.  The model is that if a task is no longer running, it may be necessary
	 * to re-start it (for example, to fetch fresh data from the network), but if it is runinning or
	 * queued to run in some way, the existing task can be re-used by attaching more listeners.
	 * 
	 * @return  <i>false</i> if any one of COMPLETE, FAIL or CANCEL has been signaled; otherwise <i>true</i>.
	 */
	public boolean isRunning() {
		return !(didSignalEvent(Event.COMPLETE)
				|| didSignalEvent(Event.FAIL)
				|| didSignalEvent(Event.CANCEL));
	}
	
	/**
	 * Checks if the {@link Event#COMPLETE COMPLETE} event has been signaled.
	 * @return <i>true</i> if the COMPLETE event has been signaled, otherwise <i>false</i>.
	 */
	public boolean isComplete() {
		return didSignalEvent(Event.COMPLETE);
	}
	
	/**
	 * This exception is thrown if {@link Task#getProgress()} or {@link Task#getMaxProgress()} is
	 * called before a {@link Task.Event#PROGRESS PROGRESS} event has been signaled.
	 *
	 */
	public static class ProgressNotAvailableException extends RuntimeException {
		private static final long serialVersionUID = 1L;

		/**
		 * Constructs a new ProgressNotAvailableException that includes the current stack trace
		 */
		public ProgressNotAvailableException() {
			super();
		}

		/**
		 * Constructs a new ProgressNotAvailableException that includes the current stack trace
		 * and a detailed message, referencing another exception as the cause.
		 * 
		 * @param detailMessage detailed information about this exception
		 * @param throwable	the cause of this exception
		 */
		public ProgressNotAvailableException(String detailMessage,
				Throwable throwable) {
			super(detailMessage, throwable);
		}

		/**
		 * Constructs a new ProgressNotAvailableException that includes the current stack trace
		 * and a detailed message.
		 * 
		 * @param detailMessage detailed information about this exception
		 */
		public ProgressNotAvailableException(String detailMessage) {
			super(detailMessage);
		}

		/**
		 * Constructs a new ProgressNotAvailableException that includes the current stack 
		 * trace, referencing another exception as the cause.
		 * 
		 * @param throwable	the cause of this exception
		 */
		public ProgressNotAvailableException(Throwable throwable) {
			super(throwable);
		}
		
	}
	
	/**
	 * Wrapper class for multiple tasks that allows waiting for all tasks to be signaled.
	 * 
	 * To get an instance, call \link waitForAll Task.waitForAll(Task...)\endlink.
	 *
	 */
	public static class MultiplexTask extends Task {
		
		/**
		 * Interface for listening for all tasks signaled on a {@link MultiplexTask}.
		 * 
		 * See \link waitForAll waitForAll(Task...)\endlink for details.
		 *
		 */
		public interface OnAllTasksSignalledListener {
			
			/**
			 * Called when all tasks in a MultiplexTask are signaled.
			 * @param t			The MultiplexTask on which all tasks have been signaled
			 * @param tasks		The list of tasks that belong to this MultiplexTask
			 * @param e			The event that caused the final task to become signaled
			 */
			public void onAllTasksSignalled(MultiplexTask t, Task[] tasks, Event e );
		}
		
		final Task[] tasks;
		private MultiplexTask( Task[] tasks ) {
			super();
			this.tasks = tasks;
		}
		
		/**
		 * Gets a list of all of the tasks that this MultiplexTask wraps.
		 * @return  Array of tasks.
		 */
		public Task[] getTasks() {
			return tasks;
		}

		/**
		 * Attaches a listener to be called when all tasks wrapped by this MultiplexTask have been signaled.
		 * 
		 * A task is considered signaled if it sends FAIL, COMPLETE, or CANCEL.  SUCCESS and PROGRESS events are not counted.
		 * 
		 * @param listener	The listener to call when all tasks have been signaled.
		 * @return The original Task object (itself) to allow for chaining.
		 */
		public Task onAllTasksSignalled( final OnAllTasksSignalledListener listener ) {
			return onEvent(Event.COMPLETE, new OnTaskEventListener() {
				@Override
				public void onTaskEvent(Task t, Event e) {
					listener.onAllTasksSignalled(MultiplexTask.this, tasks, e);
				}
			});
		}
		
	}
	
	/**
	 * Create a wrapper that represents the combination of several tasks.
	 * 
	 * This is different from {@link #waitForAll}, which simply waits
	 * for all tasks to be signaled.
	 * 
	 * Rather, this combines the tasks, so it should generally be used on several
	 * tasks of the same type only.  The combined task behaves as follows:
	 * 
	 * - When all child tasks have signaled progress at least once, begins publishing
	 *   combined progress information (the current and max progress are a sum of all
	 *   of the child tasks). Therefore, you should use child tasks where the progress
	 *   values have the same meaning.  This is useful, for example, to get an overall
	 *   combined progress for several separate file download tasks.
	 * - When all child tasks have signaled SUCCESS, this task signals SUCCESS.
	 * - When all child tasks have signaled COMPLETE, this task signals COMPLETE.
	 * - If ANY child task signals FAIL, this task also signals FAIL.
	 * 
	 * @param tasks	A collection of tasks to combine
	 * @return A wrapper task that represents the combination of the specified tasks.
	 */
	public static Task combinedTask( Collection<? extends Task> tasks ) {
		final Task task = new Task();
		final List<Task> taskList = new ArrayList<Task>(tasks);
		
//		if( LL.D ) Log.d(LOG_TAG,"combinedTask : " + task + " containing " + taskList);
		
		for( Task t: taskList ) {
			t.onProgress(new OnProgressListener() {
				@Override
				public void onProgress(Task _t, Event e, int progress, int maxProgress) {
					
					boolean progressAvailable = true;
					int combinedProgress = 0;
					int combinedMaxProgress = 0;
					for( Task t: taskList ) {
						if( !t.isProgressAvailable() ) {
							progressAvailable = false;
							break;
						} else {
							combinedProgress += t.getProgress();
							combinedMaxProgress += t.getMaxProgress();
						}
					}
//					if( LL.D ) Log.d(LOG_TAG,"combinedTask/onProgress : " + _t + " p/m=" + progress + "/" + maxProgress + " progressAvailable=" + progressAvailable + " combined=" + combinedProgress + "/" + combinedMaxProgress);
					if( progressAvailable ) {
						task.setProgress(combinedProgress, combinedMaxProgress);
					}
					
				}
			});
			t.onComplete( new OnTaskEventListener() {
				
				@Override
				public void onTaskEvent(Task _t, Event e) {
					boolean complete = true;
					for( Task t: taskList ) {
						if( !t.didSignalEvent(Event.COMPLETE) ) {
							complete = false;
							break;
						}
					}
//					if( LL.D ) Log.d(LOG_TAG,"combinedTask/onComplete : " + _t + " complete=" + complete );
					if( complete ) {
						task.signalEvent(Event.COMPLETE);
					}
				}
			});
			t.onSuccess( new OnTaskEventListener() {
				
				@Override
				public void onTaskEvent(Task _t, Event e) {
					boolean success = true;
					for( Task t: taskList ) {
						if( !t.didSignalEvent(Event.SUCCESS) ) {
							success = false;
							break;
						}
					}
//					if( LL.D ) Log.d(LOG_TAG,"combinedTask/onSuccess : " + _t + " success=" + success );
					if( success ) {
						task.signalEvent(Event.SUCCESS);
					}
				}
			});
			t.onFailure( new OnFailListener() {
				
				@Override
				public void onFail(Task t, Event e, TaskError failureReason) {
//					if( LL.D ) Log.d(LOG_TAG,"combinedTask/onFail : " + failureReason );
					task.sendFailure(failureReason);
				}
			});
		}
		
		return task;
	}
	
	/**
	 * Creates a MultiplexTask wrapping the specified tasks.
	 * 
	 * This can be used to wait for a group of tasks to all complete or fail before performing some further operation.
	 * 
	 * This is designed for multiple tasks of possibly different types, so task progress data is not combined.  If you
	 * need to get unified progress for a group of tasks of the same type, use {@link Task#combinedTask(Collection<? extends Task>)} instead.
	 * 
	 * @param tasks	Tasks to wrap
	 * @return	A MultiplexTask wrapping the specified tasks
	 */
	public static MultiplexTask waitForAll( final Task... tasks ) {
		final MultiplexTask result = new MultiplexTask(tasks);
		final Event[] signalled = new Event[tasks.length];
		for( int i=0; i<tasks.length; i++ ) {
			signalled[i] = null;
		}
		OnTaskEventListener eventListener = new OnTaskEventListener() {
			@Override
			public void onTaskEvent(Task t, Event e) {
				Event finalEvent = Event.COMPLETE;
				for( int i=0; i<tasks.length; i++ ) {
					if( tasks[i]==t ) {
						signalled[i] = e;
						break;
					}
				}
				for( int i=0; i<tasks.length; i++ ) {
					if( signalled[i]==null ) {
						return;
					} else if( signalled[i]==Event.FAIL ) {
						finalEvent = Event.FAIL;
					} else if( signalled[i]==Event.CANCEL && finalEvent==Event.COMPLETE ) {
						finalEvent = Event.CANCEL;
					}
				}
				result.signalEvent(finalEvent);
			}
		};
		for( Task t: tasks ) {
			t.onEvent(Event.COMPLETE, eventListener);
			t.onEvent(Event.FAIL, eventListener);
			t.onEvent(Event.CANCEL, eventListener);
		}
		return result;
	}

}
