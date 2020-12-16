package com.nexstreaming.app.common.nexasset.overlay.impl;

import android.graphics.Bitmap;
import android.graphics.RectF;
import android.util.Log;

import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.assetpackage.XMLItemDef;
import com.nexstreaming.app.common.nexasset.assetpackage.XMLItemDefReader;
import com.nexstreaming.app.common.nexasset.overlay.AwakeAsset;
import com.nexstreaming.app.common.nexasset.overlay.OverlayMotion;
import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.kinemaster.config.KineMasterSingleTon;
import com.nexstreaming.kminternal.nexvideoeditor.LayerRenderer;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;

import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.Map;
//import com.nexstreaming.kinemaster.editorwrapper.NexLayerItem;
//import com.nextreaming.nexeditorui.EditorGlobal;
//import com.nextreaming.nexvideoeditor.LayerRenderer;
//import com.nextreaming.nexvideoeditor.NexEditor;


public class RenderItemOverlayAsset extends AbstractOverlayAsset {

    private static final String LOG_TAG = "RenderItemOverlayAsset";
    private int effect_id_[] = {-1, -1};

    private int width;
    private int height;

    public RenderItemOverlayAsset(ItemInfo itemInfo) {
        super(itemInfo);
        try {
            XMLItemDef itemDef = XMLItemDefReader.getItemDef(KineMasterSingleTon.getApplicationInstance().getApplicationContext(),itemInfo.getId());
            width = itemDef.getIntrinsicWidth();
            height = itemDef.getIntrinsicHeight();
            Log.d(LOG_TAG,"Read itemdef: " + itemInfo.getId() + " : w,h=" + itemDef.getIntrinsicWidth() + "," + itemDef.getIntrinsicHeight());
        } catch (XmlPullParserException e) {
            Log.e(LOG_TAG,"Error reading itemdef: " + itemInfo.getId(),e);
        } catch (IOException e) {
            Log.e(LOG_TAG,"Error reading itemdef: " + itemInfo.getId(),e);
        }
    }

    @Override
    public int getIntrinsicWidth() {
        if( width <= 0 || height <= 0 )
            return 700;
        return width;
    }

    @Override
    public int getIntrinsicHeight() {
        if( width <= 0 || height <= 0 )
            return 700;
        return height;
    }

    @Override
    public int getDefaultDuration() {
        return 0;
    }

    @Override
    public AwakeAsset onAwake(LayerRenderer renderer, final RectF bounds, final String effectOptions, Map<String,String> unencodedEffectOptions ){

        return new AwakeAsset() {

            private String currentEffectOptions = effectOptions;
            private RectF currentBound = bounds;
            @Override
            public void onAsleep(LayerRenderer renderer) {

                //delete renderitem proxy..parameter will be engine id
                NexEditor editor = EditorGlobal.getEditor();
                if( editor!=null ) {

                    if(effect_id_[renderer.getRenderMode().id] >= 0) {

                        editor.releaseRenderItemJ(effect_id_[renderer.getRenderMode().id], renderer.getRenderMode().id);
                        effect_id_[renderer.getRenderMode().id] = -1;
                    }
                }
            }

            @Override
            public void onRender(LayerRenderer renderer, OverlayMotion motion, int startTime, int endTime ) {

                if(effect_id_[renderer.getRenderMode().id] < 0) {

                    NexEditor editor = EditorGlobal.getEditor();
                    if( editor!=null ) {
                        effect_id_[renderer.getRenderMode().id] = editor.createRenderItemJ(getItemInfo().getId(), renderer.getRenderMode().id);
                    }
                }
                if(effect_id_[renderer.getRenderMode().id] >= 0)
                    renderer.drawRenderItem(effect_id_[renderer.getRenderMode().id], currentEffectOptions, renderer.getCurrentTime(), startTime, endTime, currentBound.left, currentBound.top, currentBound.right, currentBound.bottom, renderer.getAlpha(), renderer.getMaskEnabled());
            }

            @Override
            public boolean needRendererReawakeOnEditResize() {
                return false;
            }

            @Override
            public boolean onRefresh(LayerRenderer renderer,RectF bounds, String effectOptions) {
                currentEffectOptions = effectOptions;
                currentBound = bounds;
                return true;
            }
        };
    }


}
