package com.nexstreaming.app.common.nexasset.overlay;

import android.graphics.RectF;

import com.nexstreaming.kminternal.nexvideoeditor.LayerRenderer;

/**
 * Created by jeongwook.yoon on 2017-03-31.
 */

public interface AwakeAsset {
    void onAsleep(LayerRenderer renderer);
    void onRender(LayerRenderer renderer, OverlayMotion motion, int startTime, int endTime);
    boolean needRendererReawakeOnEditResize();
    boolean onRefresh(LayerRenderer renderer, RectF bounds, String effectOptions);
}
