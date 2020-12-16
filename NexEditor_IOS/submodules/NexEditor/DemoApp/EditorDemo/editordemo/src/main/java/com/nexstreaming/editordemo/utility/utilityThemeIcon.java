package com.nexstreaming.editordemo.utility;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.widget.ImageView;

import com.nexstreaming.nexeditorsdk.nexTheme;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * Created by mj.kong on 2015-03-11.
 */
public class utilityThemeIcon {

    private Context mContext;
    private ArrayList<nexTheme> mThemes;

    public utilityThemeIcon(Context context, ArrayList<nexTheme> themes) {
        mContext = context;
        mThemes = themes;
    }

    public utilityThemeIcon(Context context, nexTheme theme) {
        mContext = context;
        if(mThemes == null) mThemes = new ArrayList<>();
        mThemes.add(theme);
    }

    /**
     * http://developer.android.com/training/displaying-bitmaps/process-bitmap.html 참조.
     */
    static class AsyncDrawable extends BitmapDrawable {
        private final WeakReference<ThemeIconWorkerTask> bitmapWorkerTaskReference;

        public AsyncDrawable(Resources res, Bitmap bitmap, ThemeIconWorkerTask bitmapWorkerTask) {
            super(res, bitmap);
            bitmapWorkerTaskReference =
                    new WeakReference<ThemeIconWorkerTask>(bitmapWorkerTask);
        }

        public ThemeIconWorkerTask getBitmapWorkerTask() {
            return bitmapWorkerTaskReference.get();
        }
    }

    public void loadBitmap(int resId, Bitmap defaultIcon, ImageView imageView) {
        if (cancelPotentialWork(resId, imageView)) {
            final ThemeIconWorkerTask task = new ThemeIconWorkerTask(imageView);
            final AsyncDrawable asyncDrawable =
                    new AsyncDrawable(mContext.getResources(), defaultIcon, task);
            imageView.setImageDrawable(asyncDrawable);
            task.execute(resId);
        }
    }

    public void loadBitmapEx(Bitmap defaultIcon, ImageView imageView) {
        final ThemeIconWorkerTask task = new ThemeIconWorkerTask(imageView);
        task.execute(0);
    }

    public static boolean cancelPotentialWork(int data, ImageView imageView) {
        final ThemeIconWorkerTask bitmapWorkerTask = getBitmapWorkerTask(imageView);

        if (bitmapWorkerTask != null) {
            final int bitmapData = bitmapWorkerTask.data;
            // If bitmapData is not yet set or it differs from the new data
            if (bitmapData == 0 || bitmapData != data) {
                // Cancel previous task
                bitmapWorkerTask.cancel(true);
            } else {
                // The same work is already in progress
                return false;
            }
        }
        // No task associated with the ImageView, or an existing task was cancelled
        return true;
    }

    private static ThemeIconWorkerTask getBitmapWorkerTask(ImageView imageView) {
        if (imageView != null) {
            final Drawable drawable = imageView.getDrawable();
            if (drawable instanceof AsyncDrawable) {
                final AsyncDrawable asyncDrawable = (AsyncDrawable) drawable;
                return asyncDrawable.getBitmapWorkerTask();
            }
        }
        return null;
    }

    private class ThemeIconWorkerTask extends AsyncTask<Integer, Void, Bitmap> {

        private final WeakReference<ImageView> imageViewReference;
        private int data = 0;

        public ThemeIconWorkerTask(ImageView imageView) {
            imageViewReference = new WeakReference<ImageView>(imageView);
        }

        @Override
        protected Bitmap doInBackground(Integer... params) {
            if(mThemes != null) {
                data = params[0];
                return mThemes.get(data).getIconSyncEx();
            } else {
                return null;
            }
        }

        @Override
        protected void onPostExecute(Bitmap bitmap) {
            if (isCancelled()) {
                bitmap = null;
            }

            if (imageViewReference != null && bitmap != null) {
                final ImageView imageView = imageViewReference.get();
                final ThemeIconWorkerTask bitmapWorkerTask =
                        getBitmapWorkerTask(imageView);
                if (this == bitmapWorkerTask && imageView != null) {
                    imageView.setImageBitmap(bitmap);
                }
            }
        }
    }
}
