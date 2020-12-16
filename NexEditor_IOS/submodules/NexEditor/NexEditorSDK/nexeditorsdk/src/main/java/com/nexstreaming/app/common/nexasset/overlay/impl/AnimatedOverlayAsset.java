package com.nexstreaming.app.common.nexasset.overlay.impl;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Picture;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.PictureDrawable;
import android.util.Log;
import android.util.LruCache;

import com.larvalabs.svgandroid.SVG;
import com.larvalabs.svgandroid.SVGParser;
import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageReader;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.overlay.AwakeAsset;
import com.nexstreaming.app.common.nexasset.overlay.OverlayMotion;
import com.nexstreaming.app.common.nexasset.overlay.OverlaySpec;
import com.nexstreaming.app.common.util.CloseUtil;
import com.nexstreaming.app.common.util.PathUtil;
import com.nexstreaming.kminternal.nexvideoeditor.LayerRenderer;
//import com.nexstreaming.kinemaster.editorwrapper.NexLayerItem;
//import com.nextreaming.nexeditorui.LL;
//import com.nextreaming.nexvideoeditor.LayerRenderer;

import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;
import java.util.Map;

public class AnimatedOverlayAsset extends AbstractOverlayAsset {

    private static final String LOG_TAG = "AnimOverlayAsset";
    private static final int MAX_TEX_SIZE = 2000;

    private OverlaySpec overlaySpec;
    private AssetPackageReader reader;
    private String baseFile;
    private float vectorScale;

    public AnimatedOverlayAsset(ItemInfo itemInfo) throws IOException, XmlPullParserException {
        super(itemInfo);
        InputStream in = null;
        InputStream firstFrameIn = null;
        AssetPackageReader reader = null;
        try {
            reader = getAssetPackageReader();
            in = reader.openFile(itemInfo.getFilePath());
            overlaySpec = OverlaySpec.fromInputStream(in);
            if( overlaySpec.width<=0 || overlaySpec.height <=0 ) {
                if (overlaySpec.layers != null && overlaySpec.layers.size() > 0) {
                    OverlaySpec.Layer firstLayer = overlaySpec.layers.get(0);
                    if (firstLayer.frames != null && firstLayer.frames.size() > 0) {
                        OverlaySpec.Frame firstFrame = firstLayer.frames.get(0);
                        if( !firstFrame.blank ) {
                            firstFrameIn = reader.openFile(firstFrame.src);
                            Picture p = SVGParser.getSVGFromInputStream(firstFrameIn).getPicture();
                            overlaySpec.width = p.getWidth();
                            overlaySpec.height = p.getHeight();
                        }
                    }
                }
            }
            if( overlaySpec.width<=0 || overlaySpec.height <=0 ) {
                overlaySpec.width = 100;
                overlaySpec.height = 100;
            }
        } finally {
            CloseUtil.closeSilently(firstFrameIn);
            CloseUtil.closeSilently(in);
            CloseUtil.closeSilently(reader);
        }
        /*
        InputStream in = null;
        AssetPackageReader reader = null;
        try {
            reader = getAssetPackageReader();
            in = reader.openFile(itemInfo.getFilePath());
            overlaySpec = OverlaySpec.fromInputStream(in);
        } finally {
            CloseUtil.closeSilently(in);
            CloseUtil.closeSilently(reader);
        }
        */
    }

    @Override
    public int getIntrinsicWidth() {
        return overlaySpec.width;
    }

    @Override
    public int getIntrinsicHeight() {
        return overlaySpec.height;
    }

    @Override
    public int getDefaultDuration() {
        if( overlaySpec.duration > 0 )
            return overlaySpec.duration * 1000 / overlaySpec.fps;
        int duration = 0;
        for(OverlaySpec.Layer l: overlaySpec.layers) {
            if( l.iterationCount < 0 )
                return 0;
            duration = Math.max(duration, l.duration*l.iterationCount * 1000 / overlaySpec.fps );
        }
        if( duration > 30000 )
            return 0;
        if( duration < 1000 )
            return 1000;
        return duration;
    }

    @Override
    public AwakeAsset onAwake(LayerRenderer renderer, final RectF bounds, final String effectOptions, Map<String,String> unencodedEffectOptions ){
        AssetPackageReader reader = null;
        try {
            reader = getAssetPackageReader();
        } catch (IOException e) {
            Log.e( LOG_TAG,"Error getting package reader",e);
        }
        float maxTexScale = MAX_TEX_SIZE / Math.max(overlaySpec.width, overlaySpec.height);
        float vectorScale = 1;//Math.min(Math.min(scaleRange.weightedAverageScale * 2, scaleRange.maxScale), maxTexScale);
        return new AwakeAssetImpl(bounds, overlaySpec, reader, getItemInfo().getFilePath(), vectorScale);

    }

    private static class AwakeAssetImpl implements AwakeAsset {

        private final RectF bounds;
        private final OverlaySpec overlaySpec;
        private final AssetPackageReader reader;
        private final String baseFile;
        private final float vectorScale;

        AwakeAssetImpl(RectF bounds, OverlaySpec overlaySpec, AssetPackageReader reader, String baseFile, float vectorScale) {
            this.bounds = new RectF(bounds);
            this.overlaySpec = overlaySpec;
            this.reader = reader;
            this.baseFile = baseFile;
            this.vectorScale = vectorScale;
        }

        @Override
        public void onAsleep(LayerRenderer renderer) {
            bitmapCache.evictAll();
            CloseUtil.closeSilently(reader);
        }

        @Override
        public void onRender(LayerRenderer renderer, OverlayMotion motion, int startTime, int endTime) {
            if( overlaySpec==null ) return;

            int elapsed = renderer.getCurrentTime() - startTime;
            int frameNumber = elapsed * overlaySpec.fps / 1000;

            int layerCount = overlaySpec.layers.size();
            for( int layeridx=0; layeridx < layerCount; layeridx++ ) {
                OverlaySpec.Layer layer = overlaySpec.layers.get(layeridx);
                int layerFrameNumber;
                if( layer.iterationCount>=0 && frameNumber > layer.iterationCount * layer.duration - 1 ) {
                    layerFrameNumber = layer.iterationCount * layer.duration - 1;
                } else {
                    layerFrameNumber = frameNumber;
                }
                boolean oddIteration = ((layerFrameNumber / layer.duration)%2 == 0);
                layerFrameNumber %= layer.duration;
                switch( layer.direction ) {
                    case NORMAL:
                        break;
                    case REVERSE:
                        layerFrameNumber = layer.duration - layerFrameNumber;
                        break;
                    case ALTERNATE:
                        if( !oddIteration ) {
                            layerFrameNumber = layer.duration - layerFrameNumber;
                        }
                        break;
                    case ALTERNATE_REVERSE:
                        if( oddIteration ) {
                            layerFrameNumber = layer.duration - layerFrameNumber;
                        }
                        break;
                }
                int frameCount = layer.frames.size();
                OverlaySpec.Frame frame = null;
                int hold = 0;
                for( int i=0; i<frameCount; i++ ) {
                    frame = layer.frames.get(i);
                    if( layerFrameNumber <= hold )
                        break;
                    hold += Math.max(1,frame.hold);
                }
                if( frame != null ) {
                    if( !frame.blank ) {
                        Bitmap bm = getImage(frame.src);
                        if (bm != null) {
                            renderer.drawBitmap(bm,bounds.left, bounds.top, bounds.right, bounds.bottom);
                        }
                    }
                }
            }

        }

        @Override
        public boolean needRendererReawakeOnEditResize() {
            return true;
        }

        @Override
        public boolean onRefresh(LayerRenderer renderer,RectF bounds, String effectOptions) {
            return false;
        }
        private LruCache<String,Bitmap> bitmapCache = new LruCache<String,Bitmap>(1024*1024*40){
            @Override
            protected int sizeOf(String key, Bitmap value) {
                return value.getByteCount();
            }
        };

        private Bitmap getImage(String src) {
            if( reader==null )
                return null;
            Bitmap bm = bitmapCache.get(src);
            if( bm!=null )
                return bm;
            bm = loadImage(src);
            if( bm!=null )
                bitmapCache.put(src,bm);
            return bm;
        }

        private Bitmap loadImage(String src) {
            InputStream in = null;
            String path = PathUtil.relativePath(baseFile,src);
            String ext = PathUtil.getExt(src);
            Bitmap result;
            try {
                in = reader.openFile(path);
                if( ext.equalsIgnoreCase("svg") ) {
                    result = loadSVG(in);
                } else {
                    result = loadBitmap(in);
                }
            } catch (IOException e) {
                Log.e( LOG_TAG,"Error reading frame image",e);
                return null;
            } finally {
                CloseUtil.closeSilently(in);
            }
            return result;
        }

        private Bitmap loadBitmap(InputStream in) {
            return BitmapFactory.decodeStream(in);
        }

        private Bitmap loadSVG(InputStream in) {
            SVG svg = SVGParser.getSVGFromInputStream(in);
//            Picture picture = svg.getPicture();
            PictureDrawable pd = svg.createPictureDrawable();
            final Bitmap bm = Bitmap.createBitmap((int)Math.floor(overlaySpec.width*vectorScale),(int)Math.floor(overlaySpec.height*vectorScale), Bitmap.Config.ARGB_8888);
            Canvas canvas = new Canvas(bm);
            pd.setBounds(0,0,bm.getWidth(),bm.getHeight());
            pd.draw(canvas);
//            canvas.scale(vectorScale,vectorScale);
//            picture.draw(canvas);
            return bm;
        }
    }
}
