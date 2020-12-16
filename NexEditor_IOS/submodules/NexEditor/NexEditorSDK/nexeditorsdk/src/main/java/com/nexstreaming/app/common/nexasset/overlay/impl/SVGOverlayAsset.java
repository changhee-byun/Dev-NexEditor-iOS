package com.nexstreaming.app.common.nexasset.overlay.impl;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Picture;
import android.graphics.RectF;
import android.util.Log;

import com.larvalabs.svgandroid.SVG;
import com.larvalabs.svgandroid.SVGParseException;
import com.larvalabs.svgandroid.SVGParser;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.overlay.AwakeAsset;
import com.nexstreaming.app.common.nexasset.overlay.OverlayMotion;
import com.nexstreaming.app.common.util.CloseUtil;
import com.nexstreaming.app.common.util.ColorUtil;
import com.nexstreaming.kminternal.nexvideoeditor.LayerRenderer;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

public class SVGOverlayAsset extends AbstractOverlayAsset {

    private static final String LOG_TAG = "SVGOverlayAsset";
    private static final int MAX_TEX_SIZE = 2000;
    private int width, height;

    public static final int COLOR_REPLACEMENT_TOLERANCE = 50;

    public SVGOverlayAsset(ItemInfo itemInfo) throws IOException {
        super(itemInfo);
        SVG svg = getSVG(null);
        if (svg != null) {
            width = svg.getPicture().getWidth();
            height = svg.getPicture().getHeight();
        }
    }

    private SVG getSVG(Map<String,String> unencodedEffectOptions) throws IOException {
        AssetPackageReader reader = null;
        InputStream in = null;
        SVG svg = null;
        Map<Integer,Integer> colorReplacements = null;
        if( unencodedEffectOptions!=null ) {
            for(Map.Entry<String,String> entry: unencodedEffectOptions.entrySet() ) {
                if( entry.getKey().startsWith("color:svgcolor_") ) {
                    int src = ColorUtil.parseColor(entry.getKey().replace("color:svgcolor_","#"));
                    int dst = ColorUtil.parseColor(entry.getValue());
                    if( colorReplacements==null) colorReplacements = new HashMap<>();
                    colorReplacements.put(src,dst);
                }
            }
        }
        try {
            reader = getAssetPackageReader();
            in = reader.openFile(getItemInfo().getFilePath());
            svg = SVGParser.getSVGFromInputStream(in,colorReplacements,COLOR_REPLACEMENT_TOLERANCE);
        } catch (SVGParseException e) {
            Log.e(LOG_TAG, e.getMessage(), e);
        } finally {
            CloseUtil.closeSilently(in);
            CloseUtil.closeSilently(reader);
        }
        return svg;
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

    @Override
    public AwakeAsset onAwake(LayerRenderer renderer, final RectF bounds, final String effectOptions, Map<String,String> unencodedEffectOptions) {
        final int awakeSerial = serial++;

        SVG svg;
        try {
            svg = getSVG(unencodedEffectOptions);
            if (svg == null)
                return null;
        } catch (IOException e) {
            Log.e(LOG_TAG,"Error loading asset",e);
            return null;
        }

        float maxTexScale = MAX_TEX_SIZE / Math.max(width, height);
        final float actualScale = 1;//Math.min(Math.min(scaleRange.weightedAverageScale * 2, scaleRange.maxScale), maxTexScale);

        Picture picture = svg.getPicture();
        final Bitmap bm = Bitmap.createBitmap((int)Math.floor(width*actualScale),(int)Math.floor(height*actualScale), Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bm);
        canvas.scale(actualScale,actualScale);
        picture.draw(canvas);
        Log.d(LOG_TAG,"onAwake OUT : [#" + awakeSerial + "] actualScale=" + actualScale + " bm=" + bm.getWidth() + "x" + bm.getHeight());

        return new AwakeAsset() {
            @Override
            public void onAsleep(LayerRenderer renderer) {
                Log.d(LOG_TAG,"onAsleep [#" + awakeSerial + "]");
            }

            @Override
            public void onRender(LayerRenderer renderer, OverlayMotion motion, int startTime, int endTime ) {
                Log.d(LOG_TAG,"onRender [#" + awakeSerial + "]="+bounds.toString());
                renderer.drawBitmap(bm, bounds.left, bounds.top, bounds.right, bounds.bottom);
                //renderer.drawBitmap(bm, 0,0);
            }

            @Override
            public boolean needRendererReawakeOnEditResize() {
                return true;
            }

            @Override
            public boolean onRefresh(LayerRenderer renderer,RectF bounds, String effectOptions) {
                return false;
            }
        };
    }



    private static int serial = 0;

}
