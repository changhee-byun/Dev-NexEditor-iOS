package com.nexstreaming.editordemo.utility;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;
import android.util.LruCache;

import com.nexstreaming.editordemo.R;
import com.nexstreaming.nexeditorsdk.nexColorEffect;
import com.nexstreaming.nexeditorsdk.nexProject;

import java.util.List;

/**
 * Created by pg on 16. 2. 2..
 */
public class utilityColorEffect2Thumbnail {

    static utilityColorEffect2Thumbnail colorEffect2Thumbnail;
    static LruCache<String, Bitmap> lruCache;
    static Bitmap thumbnail;

    Context context;
    nexProject project;

    public utilityColorEffect2Thumbnail(nexProject project, Context contet) {
        this.project = project;
        this.context = contet;

        if(colorEffect2Thumbnail == null) {
            colorEffect2Thumbnail = new utilityColorEffect2Thumbnail();
        }
    }

    public utilityColorEffect2Thumbnail() {
        if(lruCache == null) {
            lruCache = new LruCache<String, Bitmap>(40/*pre-defined count*/) {
                @Override
                protected void entryRemoved(boolean evicted, String key, Bitmap oldBitmap, Bitmap newBitmap) {
                    if(evicted) {
                        Log.d("CE2Thumbnail", "the entry is being removed to make space / key:" + key);
                    } else {
                        Log.d("CE2Thumbnail", "the removal was caused by a put(K, V) or remove(K) / key:"+key);
                    }
                    if(oldBitmap != null) {
                        oldBitmap.recycle();
                    }
                }
            };
        }
    }

    public static utilityColorEffect2Thumbnail getInstance() {
        return colorEffect2Thumbnail;
    }

    public void setThumbnail(int clipId) {
        thumbnail = project.getClip(clipId, true).getMainThumbnail(120f, context.getResources().getDisplayMetrics().density);
        if(thumbnail != null) {
            Log.d("CE2Thumbnail", "width:" + thumbnail.getWidth() + " height:" + thumbnail.getHeight());
        } else {
            thumbnail = BitmapFactory.decodeResource(context.getResources(), R.mipmap.ic_clip_background);
        }
    }

    public void setColorEffect2Thumbnail(String effectName) {
        nexColorEffect colorEffect = null;
        List<nexColorEffect> colorEffects = nexColorEffect.getPresetList();
        for(nexColorEffect effect: colorEffects) {
            if (effect.getPresetName().equals(effectName)) {
                colorEffect = effect;
                break;
            }
        }
        if(colorEffect != null) {
            if(lruCache.get(effectName) == null) {
                Bitmap colorEffect2Thumbnail = nexColorEffect.applyColorEffectOnBitmap(thumbnail, colorEffect);
                if(colorEffect2Thumbnail != null) {
                    Log.d("CE2Thumbnail", "newly put in cache: "+effectName);
                    lruCache.put(effectName, colorEffect2Thumbnail);
                }
            }
        }
    }

    public Bitmap getColorEffect2Thumbnail(String effectName) {
        if(lruCache.get(effectName) != null) {
            Log.d("CE2Thumbnail", "Already exist, get in cache: "+effectName);
            return lruCache.get(effectName);
        } else {
            setColorEffect2Thumbnail(effectName);
            return getColorEffect2Thumbnail(effectName);
        }
    }

    public void releaseResources() {
        lruCache.evictAll();
        lruCache = null;

        colorEffect2Thumbnail = null;
    }

    public void evictCache() {
        lruCache.evictAll();
    }
}
