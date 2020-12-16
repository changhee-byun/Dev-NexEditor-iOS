package com.nexstreaming.app.common.util;

import android.view.MotionEvent;

public class RotationGestureDetector {
	
	private final OnRotationGestureListener mListener;
	private boolean mInProgress = false;
	private int mPrimaryPointerId;
	private int mSecondaryPointerId;
	private float mFocusX;
	private float mFocusY;
	private double mInitialTheta;
	
	public interface OnRotationGestureListener {
		boolean onBeginRotate(float focusX, float focusY);
		void onRotate(float focusX, float focusY, float angle);
		void onEndRotate(boolean cancel);
	}
	
	public boolean isInProgress() {
		return mInProgress;
	}

	public RotationGestureDetector(OnRotationGestureListener listener ) {
		mListener = listener;
	}

	public boolean onTouchEvent(MotionEvent event) {
		
		switch(event.getActionMasked()) {
		case MotionEvent.ACTION_DOWN:
			mPrimaryPointerId = event.getPointerId(0);
			return true;
		case MotionEvent.ACTION_CANCEL:
			if( mInProgress ) {
				mListener.onEndRotate(true);
				mInProgress = false;
			}
			return true;
		case MotionEvent.ACTION_POINTER_DOWN:
			if( !mInProgress ) {
				mSecondaryPointerId = event.getPointerId(event.getActionIndex());
				if( mSecondaryPointerId==mPrimaryPointerId ) {
					return true;
				}
				int primaryIdx = event.findPointerIndex(mPrimaryPointerId);
				int secondaryIdx = event.findPointerIndex(mSecondaryPointerId);
				if( primaryIdx < 0 || secondaryIdx < 0 ) {
					mListener.onEndRotate(false);
					mInProgress = false;
					return true;
				}
				float primaryX = event.getX(primaryIdx);
				float primaryY = event.getY(primaryIdx);
				float secondaryX = event.getX(secondaryIdx);
				float secondaryY = event.getY(secondaryIdx);
				mFocusX = (primaryX + secondaryX)/2;
				mFocusY = (primaryY + secondaryY)/2;
				mInitialTheta = Math.atan2(primaryY-secondaryY, primaryX-secondaryX);
				if( mListener.onBeginRotate(mFocusX, mFocusY) ) {
					mInProgress = true;
				}
			}
			return true;
		case MotionEvent.ACTION_POINTER_UP:
			int pointerId = event.getPointerId(event.getActionIndex());
			if( mInProgress && (mSecondaryPointerId == pointerId || mPrimaryPointerId == pointerId) ) {
				mListener.onEndRotate(false);
				mInProgress = false;
			}
			return true;
		case MotionEvent.ACTION_MOVE:
			if( mInProgress ) {
				int primaryIdx = event.findPointerIndex(mPrimaryPointerId);
				int secondaryIdx = event.findPointerIndex(mSecondaryPointerId);
				if( primaryIdx < 0 || secondaryIdx < 0 ) {
					mListener.onEndRotate(false);
					mInProgress = false;
					return true;
				}
				float primaryX = event.getX(primaryIdx);
				float primaryY = event.getY(primaryIdx);
				float secondaryX = event.getX(secondaryIdx);
				float secondaryY = event.getY(secondaryIdx);
				double newTheta = Math.atan2(primaryY-secondaryY, primaryX-secondaryX);
				mListener.onRotate(mFocusX, mFocusY,(float)((newTheta-mInitialTheta)*180/Math.PI));
			}
			return true;
		case MotionEvent.ACTION_UP:
			if( mInProgress ) {
				mListener.onEndRotate(false);
				mInProgress = false;
			}
			return true;
		default:
			return true;
		}
		
	}
	
}
