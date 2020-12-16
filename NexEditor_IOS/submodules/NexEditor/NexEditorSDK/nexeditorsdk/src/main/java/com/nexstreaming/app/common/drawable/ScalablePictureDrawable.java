package com.nexstreaming.app.common.drawable;

import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Picture;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.PictureDrawable;
import android.util.SparseArray;

import com.larvalabs.svgandroid.SVG;
import com.larvalabs.svgandroid.SVGParser;

import java.lang.ref.WeakReference;

public class ScalablePictureDrawable extends PictureDrawable {

    public ScalablePictureDrawable(Picture p) {
        super(p);
    }

    public ScalablePictureDrawable(Drawable d) {
        super(((PictureDrawable) d).getPicture());
    }

    @Override
    public void draw(Canvas canvas) {
        Picture p = getPicture();
        if( p==null )
            return;
        Rect bounds = getBounds();
        canvas.save();
        canvas.clipRect(bounds);
        canvas.translate(bounds.left, bounds.top);
        canvas.scale((float)bounds.width()/(float)getIntrinsicWidth(), (float)bounds.height()/(float)getIntrinsicHeight());
        canvas.drawPicture(p);
        canvas.restore();
    }

    public static SparseArray<WeakReference<ScalablePictureDrawable>> sDrawableCache = new SparseArray<>();

    public static ScalablePictureDrawable fromSVG(Resources resources, int rsrcId ) {
        WeakReference<ScalablePictureDrawable> drawableRef = sDrawableCache.get(rsrcId);
        if( drawableRef!=null ) {
            ScalablePictureDrawable drawable = drawableRef.get();
            if( drawable!=null )
                return drawable;
        }
        SVG svg = SVGParser.getSVGFromResource(resources, rsrcId);
        ScalablePictureDrawable result = new ScalablePictureDrawable(svg.createPictureDrawable());
        sDrawableCache.put(rsrcId, new WeakReference<ScalablePictureDrawable>(result));
        return result;
    }

}
