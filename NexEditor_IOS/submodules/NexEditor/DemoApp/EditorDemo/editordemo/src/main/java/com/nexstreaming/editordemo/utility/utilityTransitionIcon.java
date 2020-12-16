package com.nexstreaming.editordemo.utility;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.ImageView;


import com.nexstreaming.nexeditorsdk.nexTransitionEffect;
import com.nexstreaming.editordemo.R;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * Created by mj.kong on 2015-03-11.
 */
public class utilityTransitionIcon {

    private Context mContext;
    private ArrayList<nexTransitionEffect> mTransEffect;

    public utilityTransitionIcon(Context context, ArrayList<nexTransitionEffect> transEffect) {
        mContext = context;
        mTransEffect = transEffect;
    }

    public utilityTransitionIcon(Context context, nexTransitionEffect transEffect) {
        mContext = context;
        if(mTransEffect == null) mTransEffect = new ArrayList<>();
        mTransEffect.add(transEffect);
    }

    /**
     * http://developer.android.com/training/displaying-bitmaps/process-bitmap.html 참조.
     */
    static class AsyncDrawable extends BitmapDrawable {
        private final WeakReference<TransEffectIconWorkerTask> bitmapWorkerTaskReference;

        public AsyncDrawable(Resources res, Bitmap bitmap, TransEffectIconWorkerTask bitmapWorkerTask) {
            super(res, bitmap);
            bitmapWorkerTaskReference =
                    new WeakReference<>(bitmapWorkerTask);
        }

        public TransEffectIconWorkerTask getBitmapWorkerTask() {
            return bitmapWorkerTaskReference.get();
        }
    }

    public void loadBitmap(int resId, Bitmap defaultIcon, ImageView imageView) {
        if (cancelPotentialWork(resId, imageView)) {
            final TransEffectIconWorkerTask task = new TransEffectIconWorkerTask(imageView);
            final AsyncDrawable asyncDrawable =
                    new AsyncDrawable(mContext.getResources(), defaultIcon, task);
            imageView.setImageDrawable(asyncDrawable);
            task.execute(resId);
        }
    }

    public static boolean cancelPotentialWork(int data, ImageView imageView) {
        final TransEffectIconWorkerTask bitmapWorkerTask = getBitmapWorkerTask(imageView);

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

    private static TransEffectIconWorkerTask getBitmapWorkerTask(ImageView imageView) {
        if (imageView != null) {
            final Drawable drawable = imageView.getDrawable();
            if (drawable instanceof AsyncDrawable) {
                final AsyncDrawable asyncDrawable = (AsyncDrawable) drawable;
                return asyncDrawable.getBitmapWorkerTask();
            }
        }
        return null;
    }

    private class TransEffectIconWorkerTask extends AsyncTask<Integer, Void, Bitmap> {

        private final WeakReference<ImageView> imageViewReference;
        private int data = 0;

        public TransEffectIconWorkerTask(ImageView imageView) {
            imageViewReference = new WeakReference<>(imageView);
        }

        @Override
        protected Bitmap doInBackground(Integer... params) {
            if(mTransEffect != null) {
                data = params[0];
                String id = mTransEffect.get(data).getName(mContext);
                switch(id) {
                    /*  3D Transition
                    **/
                    case "3D Flip":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_3d_flip);
                    case "3D Zoom Flip":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_3d_zoom_flip);
                    case "Bouncy Box":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_bouncy_box);
                    case "Checker Flip":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_checker_flip);
                    case "Strips":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_strips);
                    /*  Classic Transition
                    **/
                    case "Crossfade":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_crossfade);
                    case "Fade Through Color":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_fade_through_color);
                    case "Zoom out":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_zoom_out);
                    /*  Fun Transition
                    **/
                    case "Circle Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transiton_circlewipe);
                    case "Heart Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_heartwipe);
                    case "Knock Aside":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_knock_aside);
                    case "Many Circles":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transiton_manycircles);
                    case "Star Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_starwipe);
                    /*  Picture-in-Picture and Transition
                    **/
                    case "Color Tiles":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_color_tiles);
                    case "Corners":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_corners);
                    case "Inset Video":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_insert_video);
                    case "Split-screen":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_split_screen);
                    case "Video Tiles":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_video_tiles);
                    /*  Presentation
                    **/
                    case "Block In":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_presentation_blockin);
                    case "Clock Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_presentation_clockwipe);
                    case "Cover":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_presentation_cover);
                    case "Double Split":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_presentation_doublesplit);
                    case "Slide":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_presentation_slide);
                    case "Spin":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_presentation_spin);
                    case "Split":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_presentation_split);
                    case "Strip Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_presentation_stripwipe);
                    case "Uncover":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_presentation_uncover);
                    case "Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_presentation_wipe);
                    case "Zoom out, then in":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_presentation_zoomout_thenin);
                    /*  Text Transition
                    **/
                    case "Captioned Inset":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_captioned_inset);
                    case "Retro Pastel Title":
                    case "Retro Terminal Title":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_retro_title);
                    case "Split Title":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_split_title);
                    case "Square Title":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_transition_squaetitle);
                    default:
                        Log.d("utilityTransitionIcon", "Name="+id);
                }
            }
            return null;
        }

        @Override
        protected void onPostExecute(Bitmap bitmap) {
            if (isCancelled()) {
                bitmap = null;
            }

            if (imageViewReference != null && bitmap != null) {
                final ImageView imageView = imageViewReference.get();
                final TransEffectIconWorkerTask bitmapWorkerTask =
                        getBitmapWorkerTask(imageView);
                if (this == bitmapWorkerTask && imageView != null) {
                    imageView.setImageBitmap(bitmap);
                }
            }
        }
    }
}
