package com.nexstreaming.app.assetlibrary.view;

import android.content.Context;
import android.graphics.drawable.Drawable;

/**
 * Created by ojin.kwon on 2016-04-26.
 */
public interface PagerTabContent {
    String getTabTitle(Context context);
    Drawable getTabDrawable(Context context);
    boolean isShowingIcon();
}
