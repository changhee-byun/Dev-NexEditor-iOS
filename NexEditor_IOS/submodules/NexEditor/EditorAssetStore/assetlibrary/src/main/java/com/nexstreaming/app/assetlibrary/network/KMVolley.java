package com.nexstreaming.app.assetlibrary.network;


import android.content.Context;
import android.graphics.Bitmap;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.LruCache;

import com.android.volley.Cache;
import com.android.volley.Network;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.toolbox.BasicNetwork;
import com.android.volley.toolbox.DiskBasedCache;
import com.android.volley.toolbox.HurlStack;
import com.android.volley.toolbox.ImageLoader;
import com.nexstreaming.app.assetlibrary.LL;

import java.io.File;

public class KMVolley {

    private static KMVolley instance;
    private RequestQueue requestQueue;
    private ImageLoader imageLoader;
    private ImageLoader categoryIconImageLoader;

    public static KMVolley getInstance(Context context) {
        if( instance==null ) {
            instance = new KMVolley(context.getApplicationContext());
        }
        return instance;
    }

    private KMVolley(Context context) {
        Cache cache = new DiskBasedCache(new File(context.getFilesDir(),".kmvnetcache"),25*1024*1024);
        Network network = new BasicNetwork(new HurlStack());
        requestQueue = new RequestQueue(cache, network, 8);
        imageLoader = new ImageLoader(requestQueue,new LruImageCache(context));
        categoryIconImageLoader = new ImageLoader(requestQueue,new LruImageCache(context));
        requestQueue.start();
    }

    public ImageLoader getImageLoader() {
        return imageLoader;
    }

    public ImageLoader getCategoryIconImageLoader() {
        return categoryIconImageLoader;
    }

    public RequestQueue getRequestQueue() {
        return requestQueue;
    }

    public <T> Request<T> add(Request<T> request) {
        return requestQueue.add(request);
    }

    private static class PermanentImageCache implements ImageLoader.ImageCache {

        @Override
        public Bitmap getBitmap(String url) {
            return null;
        }

        @Override
        public void putBitmap(String url, Bitmap bitmap) {

        }
    }

    private static class LruImageCache extends LruCache<String,Bitmap> implements ImageLoader.ImageCache {

        private static final String LOG_TAG = "LruImageCache";

        public LruImageCache(Context ctx) {
            super(getCacheSize(ctx));
        }

        @Override
        protected int sizeOf(String key, Bitmap value) {
            return value.getRowBytes() * value.getHeight();
        }
        @Override
        public Bitmap getBitmap(String url) {
            return get(url);
        }

        @Override
        public void putBitmap(String url, Bitmap bitmap) {
            put(url, bitmap);
        }

        // Returns a cache size equal to approximately three screens worth of images.
        public static int getCacheSize(Context ctx) {
            final DisplayMetrics displayMetrics = ctx.getResources().
                    getDisplayMetrics();
            final int screenWidth = displayMetrics.widthPixels;
            final int screenHeight = displayMetrics.heightPixels;
            // 4 bytes per pixel
            final int screenBytes = screenWidth * screenHeight * 4;
            if( LL.D ) Log.d(LOG_TAG,"Cache size:" + screenBytes*3 + " (w,h=" + screenWidth + "," + screenHeight + ")");

            return screenBytes * 3;
        }
    }

}
