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

import com.nexstreaming.nexeditorsdk.nexClip;
import com.nexstreaming.nexeditorsdk.nexProject;
import com.nexstreaming.editordemo.R;

import java.lang.ref.WeakReference;

/**
 * Created by mj.kong on 2015-05-20.
 */
public class utilityTimelineTransitionIcon {

    private Context mContext;
    private nexProject mProject;

    public utilityTimelineTransitionIcon(Context context, nexProject project) {
        mContext = context;
        mProject = project;
    }

    /**
     * http://developer.android.com/training/displaying-bitmaps/process-bitmap.html 참조.
     */
    static class AsyncDrawable extends BitmapDrawable {
        private final WeakReference<TimelineTransitionWorkTask> bitmapWorkerTaskReference;

        public AsyncDrawable(Resources res, Bitmap bitmap, TimelineTransitionWorkTask bitmapWorkerTask) {
            super(res, bitmap);
            bitmapWorkerTaskReference =
                    new WeakReference<TimelineTransitionWorkTask>(bitmapWorkerTask);
        }

        public TimelineTransitionWorkTask getBitmapWorkerTask() {
            return bitmapWorkerTaskReference.get();
        }
    }

    public void loadBitmap(int resId, Bitmap defaultIcon, ImageView imageView) {
        if (cancelPotentialWork(resId, imageView)) {
            final TimelineTransitionWorkTask task = new TimelineTransitionWorkTask(imageView);
            final AsyncDrawable asyncDrawable =
                    new AsyncDrawable(mContext.getResources(), defaultIcon, task);
            imageView.setImageDrawable(asyncDrawable);
            task.execute(resId);
        }
    }

    public void loadBitmapEx(Bitmap defaultIcon, ImageView imageView) {
        final TimelineTransitionWorkTask task = new TimelineTransitionWorkTask(imageView);
        task.execute(0);
    }

    public static boolean cancelPotentialWork(int data, ImageView imageView) {
        final TimelineTransitionWorkTask bitmapWorkerTask = getBitmapWorkerTask(imageView);

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

    private static TimelineTransitionWorkTask getBitmapWorkerTask(ImageView imageView) {
        if (imageView != null) {
            final Drawable drawable = imageView.getDrawable();
            if (drawable instanceof AsyncDrawable) {
                final AsyncDrawable asyncDrawable = (AsyncDrawable) drawable;
                return asyncDrawable.getBitmapWorkerTask();
            }
        }
        return null;
    }

    private class TimelineTransitionWorkTask extends AsyncTask<Integer, Void, Bitmap> {

        private final WeakReference<ImageView> imageViewReference;
        private int data = 0;

        public TimelineTransitionWorkTask(ImageView imageView) {
            imageViewReference = new WeakReference<ImageView>(imageView);
        }

        @Override
        protected Bitmap doInBackground(Integer... params) {
            if(mProject != null) {
                data = params[0];
                nexClip clip = mProject.getClip(data, true);
                String id = clip.getTransitionEffect().getName(mContext);
                switch(id) {
                    /*  3D Transition
                    **/
                    case "3D Flip":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_3d_flip_28dp);
                    case "3D Zoom Flip":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_3d_zoom_flip_28dp);
                    case "Bouncy Box":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_bouncy_box_28dp);
                    case "Checker Flip":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_checker_flip_28dp);
                    case "Strips":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_strips_28dp);
                    /*  Classic Transition
                    **/
                    case "Crossfade":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_crossfade_28dp);
                    case "Fade Through Color":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_fade_through_color_28dp);
                    case "Zoom out":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_zoom_out_28dp);
                    /*  Fun Transition
                    **/
                    case "Circle Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_circlewipe_28dp);
                    case "Heart Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_heartwipe_28dp);
                    case "Knock Aside":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_knock_aside_28dp);
                    case "Many Circles":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_manycircles_28dp);
                    case "Star Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_starwipe_28dp);
                    /*  Picture-in-Picture and Transition
                    **/
                    case "Color Tiles":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_color_tiles_28dp);
                    case "Corners":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_corners_28dp);
                    case "Inset Video":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_insert_video_28dp);
                    case "Split-screen":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_split_screen_28dp);
                    case "Video Tiles":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_video_tiles_28dp);
                    /*  Presentation
                    **/
                    case "Block In":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_presentation_blockin_28dp);
                    case "Clock Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_presentation_clockwipe_28dp);
                    case "Cover":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_presentation_cover_28dp);
                    case "Double Split":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_presentation_doublesplit_28dp);
                    case "Slide":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_presentation_slide_28dp);
                    case "Spin":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_presentation_spin_28dp);
                    case "Split":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_presentation_split_28dp);
                    case "Strip Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_presentation_stripwipe_28dp);
                    case "Uncover":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_presentation_uncover_28dp);
                    case "Wipe":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_presentation_wipe_28dp);
                    case "Zoom out, then in":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_presentation_zoomout_thenin_28dp);
                    /*  Text Transition
                    **/
                    case "Captioned Inset":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_captioned_inset_28dp);
                    case "Retro Pastel Title":
                    case "Retro Terminal Title":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_retro_title_28dp);
                    case "Split Title":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_split_title_28dp);
                    case "Square Title":
                        return BitmapFactory.decodeResource(mContext.getResources(), R.mipmap.ic_timeline_transition_square_title_28dp);
                    default:
                        Log.d("TimelineTransition", "Name=" + id);
                }
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
                final TimelineTransitionWorkTask bitmapWorkerTask =
                        getBitmapWorkerTask(imageView);
                if (this == bitmapWorkerTask && imageView != null) {
                    if (bitmap != null) {
                        imageView.setImageBitmap(bitmap);
                    } else {
                        imageView.setImageResource(R.mipmap.ic_timeline_list_transition_add_28dp);
                    }
                }
            }
        }
    }
}
