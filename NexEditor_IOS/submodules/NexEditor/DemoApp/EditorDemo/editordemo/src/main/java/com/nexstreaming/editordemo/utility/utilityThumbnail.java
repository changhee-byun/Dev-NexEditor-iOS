package com.nexstreaming.editordemo.utility;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.ImageView;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.editordemo.R;

import java.lang.ref.WeakReference;

/**
 * Created by mj.kong on 2015-05-20.
 */
public class utilityThumbnail {

    private static final String TAG = "KM_utilityThumbnail";

    private Context mContext;
    private nexProject mProject;

    public utilityThumbnail(Context context, nexProject project) {
        mContext = context;
        mProject = project;
    }

    /**
     * http://developer.android.com/training/displaying-bitmaps/process-bitmap.html 참조.
     */
    static class AsyncDrawable extends BitmapDrawable {
        private final WeakReference<ThumbnailWorkerTask> bitmapWorkerTaskReference;

        public AsyncDrawable(Resources res, Bitmap bitmap, ThumbnailWorkerTask bitmapWorkerTask) {
            super(res, bitmap);
            bitmapWorkerTaskReference =
                    new WeakReference<ThumbnailWorkerTask>(bitmapWorkerTask);
        }

        public ThumbnailWorkerTask getBitmapWorkerTask() {
            return bitmapWorkerTaskReference.get();
        }
    }

    public void loadBitmap(int resId, Bitmap defaultIcon, ImageView imageView) {
        if (cancelPotentialWork(resId, imageView)) {
            Log.d(TAG, "loadBitmap execute. resId="+resId);
            final ThumbnailWorkerTask task = new ThumbnailWorkerTask(imageView);
            final AsyncDrawable asyncDrawable =
                    new AsyncDrawable(mContext.getResources(), defaultIcon, task);
            imageView.setImageDrawable(asyncDrawable);
            task.execute(resId);
        }
    }

    public void loadBitmapEx(Bitmap defaultIcon, ImageView imageView) {
        final ThumbnailWorkerTask task = new ThumbnailWorkerTask(imageView);
        task.execute(0);
    }

    public static boolean cancelPotentialWork(int data, ImageView imageView) {
        final ThumbnailWorkerTask bitmapWorkerTask = getBitmapWorkerTask(imageView);

        if (bitmapWorkerTask != null) {
            final int bitmapData = bitmapWorkerTask.data;
            // If bitmapData is not yet set or it differs from the new data
            if (bitmapData == 0 || bitmapData != data) {
                // Cancel previous task
                bitmapWorkerTask.cancel(true);
            } else {
                // The same work is already in progress
                Log.d(TAG, "The same work is already in progress");
                return false;
            }
        }
        // No task associated with the ImageView, or an existing task was cancelled
        return true;
    }

    private static ThumbnailWorkerTask getBitmapWorkerTask(ImageView imageView) {
        if (imageView != null) {
            final Drawable drawable = imageView.getDrawable();
            if (drawable instanceof AsyncDrawable) {
                final AsyncDrawable asyncDrawable = (AsyncDrawable) drawable;
                return asyncDrawable.getBitmapWorkerTask();
            }
        }
        return null;
    }

    private class ThumbnailWorkerTask extends AsyncTask<Integer, Void, Bitmap> {

        private final WeakReference<ImageView> imageViewReference;
        private int data = 0;

        public ThumbnailWorkerTask(ImageView imageView) {
            imageViewReference = new WeakReference<ImageView>(imageView);
        }

        @Override
        protected Bitmap doInBackground(Integer... params) {
            if(mProject != null) {
                data = params[0];

                return mProject.getClip(data, true).getMainThumbnail(240f, mContext.getResources().getDisplayMetrics().density);
            }
            return null;
        }

        @Override
        protected void onPostExecute(Bitmap bitmap) {
            if (isCancelled()) {
                bitmap = null;
            }

            if(imageViewReference != null) {
                final ImageView imageView = imageViewReference.get();
                final ThumbnailWorkerTask bitmapWorkerTask =
                        getBitmapWorkerTask(imageView);
                if (this == bitmapWorkerTask && imageView != null) {
                    if (bitmap != null) {
                        imageView.setBackgroundColor(mContext.getResources().getColor(R.color.timelineBackground));
                        imageView.setImageBitmap(bitmap);
                    } else {
                        imageView.setImageResource(R.mipmap.ic_clip_fail_pic);
                    }
                }
            }
        }
    }
}
