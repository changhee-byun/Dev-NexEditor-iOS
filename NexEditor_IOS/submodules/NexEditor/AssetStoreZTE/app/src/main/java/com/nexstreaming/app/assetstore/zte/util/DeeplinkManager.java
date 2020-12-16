package com.nexstreaming.app.assetstore.zte.util;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by ojin.kwon on 2016-11-22.
 */

public class DeeplinkManager {

    private static final String TAG = "DeeplinkManager";

    private static final String SCHEME = "nexasset";
    private static final Map<String, Class<? extends Activity>> ACTIVITY_MAP;

    public static final String ASSET_DETAIL_PAGE = "assetDetail";

    static {
        ACTIVITY_MAP = new HashMap<>();
//        ACTIVITY_MAP.put(ASSET_DETAIL_PAGE, AssetDetailActivity.class);
    }


    public static Uri makeLink(String page, String... parameters){
        String link = SCHEME + "://" + page + "?";
        for(String param : parameters){
            link += param + "&";
        }
        return Uri.parse(link);
    }

    public static void goLink(Context context, Uri uri){
        if(context != null && uri != null){
            if(uri.getScheme().equals(SCHEME)){
                String page = uri.getHost();
                Class<?> cls = ACTIVITY_MAP.get(page);
                if(cls != null){
                    Intent intent = new Intent(context, cls);
                    intent.setData(uri);
                    context.startActivity(intent);
                }
            }
        }
    }
}
