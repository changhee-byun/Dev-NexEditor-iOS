package com.nexstreaming.app.common.nexasset.overlay.impl;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.RectF;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.overlay.AwakeAsset;
import com.nexstreaming.app.common.nexasset.overlay.OverlayAsset;
import com.nexstreaming.app.common.nexasset.overlay.OverlayMotion;
import com.nexstreaming.app.common.util.CloseUtil;
import com.nexstreaming.kminternal.kinemaster.config.LL;
import com.nexstreaming.kminternal.nexvideoeditor.LayerRenderer;
//import com.nexstreaming.kinemaster.editorwrapper.NexLayerItem;
//import com.nextreaming.nexeditorui.LL;
//import com.nextreaming.nexvideoeditor.LayerRenderer;

import java.io.IOException;
import java.io.InputStream;
import java.util.Map;

public class BitmapOverlayAsset extends AbstractOverlayAsset {

    private static final String LOG_TAG = "BitmapOverlayAsset";
    private int width, height;
    private static final int MAX_TEX_SIZE = 2000;

    public BitmapOverlayAsset(ItemInfo itemInfo) throws IOException {
        super(itemInfo);
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        InputStream in = getAssetPackageReader().openFile(itemInfo.getFilePath());
        try {
            BitmapFactory.decodeStream(in,null,options);

        } finally {
            CloseUtil.closeSilently(in);
        }
        width = options.outWidth;
        height = options.outHeight;
    }

    @Override
    public int getIntrinsicWidth() {
        return width;
    }

    @Override
    public int getIntrinsicHeight() {
        return height;
    }

    @Override
    public int getDefaultDuration() {
        return 0;
    }

    private Bitmap loadBitmap(BitmapFactory.Options options) {
        InputStream in = null;
        try {
            in = getAssetPackageReader().openFile(getItemInfo().getFilePath());
            return BitmapFactory.decodeStream(in,null,options);
        } catch (IOException e) {
            if( LL.E ) Log.e(LOG_TAG,"");
            return null;
        } finally {
            CloseUtil.closeSilently(in);
        }
    }

    @Override
    public AwakeAsset onAwake(LayerRenderer renderer, final RectF bounds, final String effectOptions, Map<String,String> unencodedEffectOptions ){

        BitmapFactory.Options opt = new BitmapFactory.Options();
        opt.inSampleSize = 1;
        while( opt.inSampleSize<16 && (width/opt.inSampleSize > MAX_TEX_SIZE || height / opt.inSampleSize > MAX_TEX_SIZE) ) {
            opt.inSampleSize *= 2;
        }

        final Bitmap bm = loadBitmap(opt);

        return new AwakeAsset() {
            @Override
            public void onAsleep(LayerRenderer renderer) {

            }

            @Override
            public void onRender(LayerRenderer renderer, OverlayMotion motion, int startTime, int endTime ) {
                if( bounds.left==0 && bounds.top == 0 && bounds.right == 0 && bounds.bottom == 0 ){
                    renderer.drawBitmap(bm, 0,0);
                }else {
                    renderer.drawBitmap(bm, bounds.left, bounds.top, bounds.right, bounds.bottom);
                }
            }

            @Override
            public boolean needRendererReawakeOnEditResize() {
                return false;
            }

            @Override
            public boolean onRefresh(LayerRenderer renderer,RectF bounds, String effectOptions) {
                return false;
            }
        };

    }

    public Bitmap getBitmap(float Scale, int num) {
        BitmapFactory.Options opt = new BitmapFactory.Options();
        opt.inSampleSize = 1;
        while( opt.inSampleSize<16 && (width/opt.inSampleSize > MAX_TEX_SIZE || height / opt.inSampleSize > MAX_TEX_SIZE) ) {
            opt.inSampleSize *= 2;
        }
        return loadBitmap(opt);
    }


}
