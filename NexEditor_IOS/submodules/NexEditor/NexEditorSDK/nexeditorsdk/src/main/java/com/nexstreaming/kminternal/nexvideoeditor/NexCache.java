package com.nexstreaming.kminternal.nexvideoeditor;

import android.graphics.Bitmap;
import android.util.Log;
import android.util.LruCache;

import com.nexstreaming.kminternal.kinemaster.config.LL;


/**
 * Created by pg on 16. 1. 8..
 */
public class NexCache {

    private final static String TAG = "NexCache";

    private static NexCache nexCache;
    private LruCache memoryCache;

    public static NexCache getInstance() {
        if(nexCache == null) {

            // Get max available VM memory, exceeding this amount will throw an
            // OutOfMemory exception. Stored in kilobytes as LruCache takes an
            // int in its constructor.
            // Use 1/8th of the available memory for this memory cache.
            int cacheSize = (int)(Runtime.getRuntime().maxMemory()/1024)/8;//4*1024*1024;
            nexCache = new NexCache(cacheSize);
        }
        return nexCache;
    }

    private NexCache(int cacheSize ){
        createMemoryCache(cacheSize);
    }

    private void createMemoryCache(int size) {
        memoryCache = new LruCache<Object, Bitmap>(size) {
            @Override
            protected void entryRemoved(boolean evicted, Object key, Bitmap oldValue, Bitmap newValue) {
                if(evicted) {
                    if(LL.D) Log.d(TAG, "the entry is being removed to make space / key:"+key);
                } else {
                    if(LL.D) Log.d(TAG, "the removal was caused by a put(K, V) or remove(K) / key:"+key);
                }
                if(oldValue != null) {
                    oldValue.recycle();
                }
            }

            @Override
            protected int sizeOf(Object key, Bitmap value) {
                if(LL.D) Log.d(TAG, "returns the size of the entry: <key, value>:"+key+","+value.getByteCount()/1024);
                return value.getByteCount()/1024;
            }
        };
        if(LL.D) Log.d(TAG, "cache create (size, isUsedToEntryCnt)="+size);
    }

    public void addBitmapToMemoryCache(Object key, Bitmap bitmap) throws NullPointerException {
        if(getBitmapFromMemoryCache(key) == null) {
            memoryCache.put(key, bitmap);
        }
    }

    public Bitmap getBitmapFromMemoryCache(Object key) throws NullPointerException {
        return (Bitmap) memoryCache.get(key);
    }

    public void removeBitmapFromMemoryCache(Object key) {
        Log.d(TAG, "remove id:"+key.toString());
        if(memoryCache.remove(key) == null) {
            if(LL.D) Log.d(TAG, key.toString()+" isn't in the cache.");
        } else {
            if(LL.D) Log.d(TAG, key.toString()+" removes in the cache.");
        }
    }

    public void releaseCache() {
        Log.d(TAG, "releaseCache()");
        //if(nexCache != null) {
            memoryCache.evictAll();
            //memoryCache = null;
            //nexCache = null;
        //}
    }
}
