package com.nexstreaming.kminternal.nexvideoeditor;

/**
 * Provides information on rectangle. 
 * @author Nextreaming Corporation
 *
 */
public final class NexRectangle
{
	public NexRectangle(int iLeft, int iTop, int iRight, int iBottom)
	{
		mLeft	= iLeft;
		mTop	= iTop;
		mRight	= iRight;
		mBottom	= iBottom;
	}
	public void setRect(int iLeft, int iTop, int iRight, int iBottom)
	{
		mLeft	= iLeft;
		mTop	= iTop;
		mRight	= iRight;
		mBottom	= iBottom;
	}
	public int mLeft;
	public int mTop;
	public int mRight;
	public int mBottom;

	@Override
	public boolean equals(Object o) {
		if (this == o) return true;
		if (o == null || getClass() != o.getClass()) return false;

		NexRectangle that = (NexRectangle) o;

		if (mLeft != that.mLeft) return false;
		if (mTop != that.mTop) return false;
		if (mRight != that.mRight) return false;
		return mBottom == that.mBottom;

	}

	@Override
	public int hashCode() {
		int result = mLeft;
		result = 31 * result + mTop;
		result = 31 * result + mRight;
		result = 31 * result + mBottom;
		return result;
	}
}
