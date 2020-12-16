package com.nexstreaming.editordemo.utility;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.MediaStore;
import android.util.Log;
import android.widget.ImageView;

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.editordemo.R;

import java.io.IOException;
import java.lang.ref.WeakReference;

/**
 * Created by mj.kong on 2015-05-20.
 */
public class utilityAlbumArt {

    private static final String TAG = "KM_utilityAlbumArt";

    private Context mContext;
    private ContentResolver mContentResolver;
    private musicRetriever mRetriever;

    public utilityAlbumArt(Context context, ContentResolver contentResolver, musicRetriever retriever) {
        mContext = context;
        mContentResolver = contentResolver;
        mRetriever = retriever;
    }

    /**
     * http://developer.android.com/training/displaying-bitmaps/process-bitmap.html 참조.
     */
    static class AsyncDrawable extends BitmapDrawable {
        private final WeakReference<AlbumArtWorkTask> bitmapWorkerTaskReference;

        public AsyncDrawable(Resources res, Bitmap bitmap, AlbumArtWorkTask bitmapWorkerTask) {
            super(res, bitmap);
            bitmapWorkerTaskReference =
                    new WeakReference<AlbumArtWorkTask>(bitmapWorkerTask);
        }

        public AlbumArtWorkTask getBitmapWorkerTask() {
            return bitmapWorkerTaskReference.get();
        }
    }

    public void loadBitmap(int resId, Bitmap defaultIcon, ImageView imageView) {
        if (cancelPotentialWork(resId, imageView)) {
            Log.d(TAG, "loadBitmap execute. resId="+resId);
            final AlbumArtWorkTask task = new AlbumArtWorkTask(imageView);
            final AsyncDrawable asyncDrawable =
                    new AsyncDrawable(mContext.getResources(), defaultIcon, task);
            imageView.setImageDrawable(asyncDrawable);
            task.execute(resId);
        }
    }

    public void loadBitmapEx(Bitmap defaultIcon, ImageView imageView) {
        final AlbumArtWorkTask task = new AlbumArtWorkTask(imageView);
        task.execute(0);
    }

    public static boolean cancelPotentialWork(int data, ImageView imageView) {
        final AlbumArtWorkTask bitmapWorkerTask = getBitmapWorkerTask(imageView);

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

    private static AlbumArtWorkTask getBitmapWorkerTask(ImageView imageView) {
        if (imageView != null) {
            final Drawable drawable = imageView.getDrawable();
            if (drawable instanceof AsyncDrawable) {
                final AsyncDrawable asyncDrawable = (AsyncDrawable) drawable;
                return asyncDrawable.getBitmapWorkerTask();
            }
        }
        return null;
    }

    private class AlbumArtWorkTask extends AsyncTask<Integer, Void, Bitmap> {

        private final WeakReference<ImageView> imageViewReference;
        private int data = 0;

        public AlbumArtWorkTask(ImageView imageView) {
            imageViewReference = new WeakReference<ImageView>(imageView);
        }

        @Override
        protected Bitmap doInBackground(Integer... params) {
            data = params[0];
            musicRetriever.Item item = (musicRetriever.Item)mRetriever.getItem(data);
            if(item != null) {
                if(item.getAlbumId() != -1) {
                    final Uri ART_CONTENT_URI = Uri.parse("content://media/external/audio/albumart");
                    Uri albumArtUri = ContentUris.withAppendedId(ART_CONTENT_URI, item.getAlbumId());
                    try {
                        return MediaStore.Images.Media.getBitmap(mContentResolver, albumArtUri);
                    } catch (IOException e) {
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_list_avatar_soundtrack_40dp);
                    }
                } else {
                    return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_list_avatar_soundtrack_40dp);
                }
            }
            else {
                return null;
            }
        }

        @Override
        protected void onPostExecute(Bitmap bitmap) {
            if (isCancelled()) {
                bitmap = null;
            }

            if(imageViewReference != null) {
                final ImageView imageView = imageViewReference.get();
                final AlbumArtWorkTask bitmapWorkerTask =
                        getBitmapWorkerTask(imageView);
                if (this == bitmapWorkerTask && imageView != null) {
                    imageView.setImageBitmap(bitmap);
                }
            }
        }
    }
}
