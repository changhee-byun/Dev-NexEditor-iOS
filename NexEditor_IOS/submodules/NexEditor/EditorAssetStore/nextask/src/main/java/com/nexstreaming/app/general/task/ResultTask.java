package com.nexstreaming.app.general.task;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * A ResultTask is an extension of Task that allow for result data to be sent.
 * 
 * Result tasks support the additional {@link Task.Event#RESULT_AVAILABLE RESULT_AVAILABLE} event.
 *
 * @param <T> The type to use for the result data.
 */
public class ResultTask<T> extends Task {

	private T mResult = null;
	private long mResultTime;
	
	/**
	 * A listener interface for the {@link Task.Event#RESULT_AVAILABLE RESULT_AVAILABLE} event.
	 * 
	 * @param <T> The type of result data this listener can accept (must match the ResultTask type parameter)
	 */
	public interface OnResultAvailableListener<T> {
		/**
		 * Called when a result is available
		 * @param task		The task that signaled the result (useful if the same listener is registered for multiple tasks)
		 * @param event		The event that caused the listener to be called (alwyas {@link Task.Event#RESULT_AVAILABLE RESULT_AVAILABLE})
		 * @param result	The result value (same as the one obtained by calling {@link ResultTask<T>#getResult() getResult()}.
		 */
		void onResultAvailable( ResultTask<T> task, Task.Event event, T result );
	}
	
	@Override
	public ResultTask<T> setTimeout(long delayMillis) {
		super.setTimeout(delayMillis);
		return this;
	}
	
	@Override
	public ResultTask<T> onFailure(OnFailListener listener) {
		super.onFailure(listener);
		return this;
	}
	
	@Override
	public ResultTask<T> onCancel(OnTaskEventListener listener) {
		super.onCancel(listener);
		return this;
	}
	
	@Override
	public ResultTask<T> onComplete(OnTaskEventListener listener) {
		super.onComplete(listener);
		return this;
	}
	
	@Override
	public ResultTask<T> onProgress(OnProgressListener listener) {
		super.onProgress(listener);
		return this;
	}
	
	@Override
	public ResultTask<T> onSuccess(OnTaskEventListener listener) {
		super.onSuccess(listener);
		return this;
	}
	
	public long getTimeSinceResult() {
		if( !didSignalEvent(Event.RESULT_AVAILABLE) ) {
			return -1;
		} else {
			return (System.nanoTime()-mResultTime)/1000000;
		}
	}
	
	/**
	 * Sole constructor.
	 */
	public ResultTask() {
		super();
	}
	
	/**
	 * Sets a result for the task and signals {@link Task.Event#RESULT_AVAILABLE RESULT_AVAILABLE} if
	 * it has not already been signaled.
	 * 
	 * @param result Result to set.  Can be null.
	 */
	public void setResult( T result ) {
		boolean updated = (mResult!=result);
		mResult = result;
		mResultTime = System.nanoTime();
		signalEvent(Event.RESULT_AVAILABLE);
		if(updated) {
			signalEvent(Event.UPDATE_OR_RESULT_AVAILABLE);
		}
	}
	
	/**
	 * Gets the result value for this task.
	 * 
	 * If no result is available yet (if {@link Task.Event#RESULT_AVAILABLE RESULT_AVAILABLE} has not
	 * been signaled yet), this will throw an exception.
	 * 
	 * @return The result value for this task, as set by {@link #setResult(T) setResult}.
	 */
	public T getResult() {
		if( !didSignalEvent(Event.RESULT_AVAILABLE) ) {
			throw new RuntimeException("Result not available");
		}
		return mResult;
	}
	
	/**
	 * Same as {@link #setResult(T) setResult}, but also signals SUCCESS and COMPLETE events atuomatically.
	 * @param result Result to set.  Can be null.
	 */
	public void sendResult( T result ) {
		setResult(result);
		super.signalEvent(Task.Event.RESULT_AVAILABLE, Task.Event.SUCCESS, Task.Event.COMPLETE);
	}
	
	/**
	 * Attaches a listener to handle {@link Task.Event#RESULT_AVAILABLE RESULT_AVAILABLE} events.
	 * @param listener The listener to attach.
	 * @return  The original task (this) to allow chaining.
	 */
	public ResultTask<T> onResultAvailable( final OnResultAvailableListener<T> listener ) {
		onEvent(Event.RESULT_AVAILABLE, new OnTaskEventListener() {
			@Override
			public void onTaskEvent(Task t, Event e) {
				@SuppressWarnings("unchecked")
				ResultTask<T> resultTask = (ResultTask<T>)t;
				listener.onResultAvailable(resultTask, e, resultTask.getResult());
				removeListenerForFail();
			}
		});
		return this;
	}
	
	/**
	 * Attaches a listener to handle {@link Task.Event#UPDATE_OR_RESULT_AVAILABLE UPDATE_OR_RESULT_AVAILABLE} events.
	 * @param listener The listener to attach.
	 * @return  The original task (this) to allow chaining.
	 */
	public ResultTask<T> onUpdateOrResultAvailable( final OnResultAvailableListener<T> listener ) {
		onEvent(Event.UPDATE_OR_RESULT_AVAILABLE, new OnTaskEventListener() {
			@Override
			public void onTaskEvent(Task t, Event e) {
				@SuppressWarnings("unchecked")
				ResultTask<T> resultTask = (ResultTask<T>)t;
				listener.onResultAvailable(resultTask, e, resultTask.getResult());
			}
		});
		if( didSignalEvent(Event.RESULT_AVAILABLE)) {
			listener.onResultAvailable(this, Event.UPDATE_OR_RESULT_AVAILABLE, mResult);
		}
		return this;
	}
	
	
	public T awaitResult() throws TaskErrorException {
		makeWaitable();
		awaitTaskCompletion();
		if( didSignalEvent(Event.FAIL)) {
			throw new TaskErrorException(getTaskError());
		} else if ( !didSignalEvent(Event.RESULT_AVAILABLE)) {
			throw new TaskErrorException(Task.NO_RESULT_AVAILABLE);
		}
		return getResult();
	}

	
	/**
	 * Combines the results of multiple ResultTasks.
	 * 
	 * The ResultTasks must all return collections of some type, and the returned types must all be the same.
	 * 
	 * If the original collection of tasks has an order, the results will be in the same order.
	 * 
	 * @param tasks		An array of ResultTask instances, all of which return collections of the same type.
	 * @return			A single ResultTask that, upon completion, will have a single collection 
	 * 						combining the results of the individual tasks.
	 */
	public static <T> ResultTask<List<T>> combineResults( final ResultTask<Collection<T>>... tasks ) {
		return combineResults(Arrays.asList(tasks));
	}
	
	/**
	 * Returns a result task that is already marked as completed
	 * with the specified parameter. 
	 * 
	 * Use this if you need to return a task, but already have the
	 * result.
	 * 
	 * @param result	The result to return from the result task.
	 * @return			A result task that completes instantly with the specified result.
	 */
	public static <T> ResultTask<T> completedResultTask( T result ) {
		ResultTask<T> resultTask = new ResultTask<T>();
		resultTask.sendResult(result);
		return resultTask;
	}
	
	/**
	 * Combines the results of multiple ResultTasks.
	 * 
	 * The ResultTasks must all return collections of some type, and the returned types must all be the same.
	 * 
	 * If the original collection of tasks has an order, the results will be in the same order.
	 * 
	 * @param tasks		A collection of ResultTask instances, all of which return collections of the same type.
	 * @return			A single ResultTask that, upon completion, will have a single collection 
	 * 						combining the results of the individual tasks.
	 */
	public static <
				INDIVIDUAL_RESULT_TYPE, 
				RESULT_COLLECTION extends Collection<INDIVIDUAL_RESULT_TYPE>, 
				TASK_COLLECTION extends Collection<ResultTask<RESULT_COLLECTION>>> 
		ResultTask<List<INDIVIDUAL_RESULT_TYPE>> combineResults( final TASK_COLLECTION tasks ) 
		
	{
					
		final ResultTask<List<INDIVIDUAL_RESULT_TYPE>> resultTask = new ResultTask<List<INDIVIDUAL_RESULT_TYPE>>();
		
		final Map<ResultTask<RESULT_COLLECTION>,Collection<INDIVIDUAL_RESULT_TYPE>> intermediateResults
			= new HashMap<ResultTask<RESULT_COLLECTION>,Collection<INDIVIDUAL_RESULT_TYPE>>(); 

		
		for( ResultTask<RESULT_COLLECTION> task: tasks ) {
			task.onResultAvailable(new OnResultAvailableListener<RESULT_COLLECTION>() {
				@Override
				public void onResultAvailable(ResultTask<RESULT_COLLECTION> task, Event event, RESULT_COLLECTION result) {
					if( resultTask.isRunning() ) {
						intermediateResults.put(task, result);
						if( intermediateResults.size() >= tasks.size() ) {
							List<INDIVIDUAL_RESULT_TYPE> finalResults = new ArrayList<INDIVIDUAL_RESULT_TYPE>();
							for( ResultTask<RESULT_COLLECTION> t: tasks) {
								finalResults.addAll(intermediateResults.get(t));
							}
							resultTask.sendResult(finalResults);
						}
					}
				}
			}).onFailure(new OnFailListener() {
				
				@Override
				public void onFail(Task t, Event e, TaskError failureReason) {
					resultTask.sendFailure(failureReason);
					intermediateResults.clear();
				}
			});
		}
		
		
		return resultTask;
	}

    public static <T> ResultTask<T> failedResultTask(TaskError failureReason) {
        ResultTask<T> resultTask = new ResultTask<>();
        resultTask.sendFailure(failureReason);
        return resultTask;
    }

}
