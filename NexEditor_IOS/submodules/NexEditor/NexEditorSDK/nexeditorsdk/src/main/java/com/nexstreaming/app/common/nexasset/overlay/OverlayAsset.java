package com.nexstreaming.app.common.nexasset.overlay;


import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.RectF;

import com.nexstreaming.kminternal.nexvideoeditor.LayerRenderer;

import java.util.Map;

//import com.nexstreaming.kinemaster.editorwrapper.NexLayerItem;
//import com.nextreaming.nexvideoeditor.LayerRenderer;

public interface OverlayAsset {

    /**
     * The intrinsic width of the asset.  For bitmaps, this is the width of the bitmap.  For
     * vector formats, this is merely a recommended size.
     * @return  The width in pixels, or 0 if this asset does not have an intrinsic size.
     */
    int getIntrinsicWidth();

    /**
     * The intrinsic height of the asset.  For bitmaps, this is the height of the bitmap.  For
     * vector formats, this is merely a recommended size.
     * @return  The height in pixels, or 0 if this asset does not have an intrinsic size.
     */
    int getIntrinsicHeight();

    /**
     * For animated assets, the default duration of this asset, when added to the timeline.
     * @return duration in milliseconds, or 0 if there is no recommended duration.
     */
    int getDefaultDuration();

    AwakeAsset onAwake(LayerRenderer renderer, RectF bounds, String effectOptions, Map<String,String> unencodedEffectOptions);

}
