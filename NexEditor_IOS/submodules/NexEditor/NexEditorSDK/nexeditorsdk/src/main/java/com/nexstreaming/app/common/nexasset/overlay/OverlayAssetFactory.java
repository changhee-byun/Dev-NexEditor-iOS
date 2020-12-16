package com.nexstreaming.app.common.nexasset.overlay;

import com.nexstreaming.app.common.nexasset.assetpackage.AssetPackageManager;
import com.nexstreaming.app.common.nexasset.assetpackage.ItemInfo;
import com.nexstreaming.app.common.nexasset.overlay.impl.AnimatedOverlayAsset;
import com.nexstreaming.app.common.nexasset.overlay.impl.BitmapOverlayAsset;
import com.nexstreaming.app.common.nexasset.overlay.impl.RenderItemOverlayAsset;
import com.nexstreaming.app.common.nexasset.overlay.impl.NexEDLOverlayAsset;
import com.nexstreaming.app.common.nexasset.overlay.impl.SVGOverlayAsset;
import com.nexstreaming.app.common.util.RefUtil;

import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

public class OverlayAssetFactory {

    private OverlayAssetFactory(){} // Prevent instantiation

    private static Map<String,WeakReference<OverlayAsset>> cache = new HashMap<>();
    private static int deadRefCheck = 0;

    public static OverlayAsset forItem(String itemId) throws IOException, XmlPullParserException {
        WeakReference<OverlayAsset> cachedRef = cache.get(itemId);
        if( cachedRef!=null ) {
            OverlayAsset cached = cachedRef.get();
            if( cached!=null )
                return cached;
        }
        OverlayAsset result = null;
        ItemInfo itemInfo  = AssetPackageManager.getInstance().getInstalledItemById(itemId);
        if( itemInfo==null )
            throw new IOException("Asset not found: " + itemId);
        switch( itemInfo.getType() ) {
            case overlay:
                String path = itemInfo.getFilePath();
                String ext = path.substring(path.lastIndexOf('.')+1);
                if( ext.equalsIgnoreCase("png") || ext.equalsIgnoreCase("jpeg") || ext.equalsIgnoreCase("jpg") || ext.equalsIgnoreCase("webp") ) {
                    result = new BitmapOverlayAsset(itemInfo);
                } else if( ext.equalsIgnoreCase("svg") ) {
                    result = new SVGOverlayAsset(itemInfo);
                } else if( ext.equalsIgnoreCase("xml") ) {
                    result = new AnimatedOverlayAsset(itemInfo);
                } else {
                    throw new IOException("Asset load error: " + itemId + " (unknown overlay type for '" + path + "')");
                }
                break;
            case renderitem:
                return new RenderItemOverlayAsset(itemInfo);
            case kedl:
                return new NexEDLOverlayAsset(itemInfo);
        }
        if( result!=null ) {
            cache.put(itemId, new WeakReference<>(result));
            if( deadRefCheck++ > 32 ) {
                RefUtil.removeDeadReferences(cache);
                deadRefCheck=0;
            }
        }
        return result;
    }

}
