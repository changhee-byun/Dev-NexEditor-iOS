package com.nexstreaming.app.assetlibrary.adapter;

import android.support.v4.view.PagerAdapter;

/**
 * Created by ojin.kwon on 2016-04-03.
 */
abstract public class BasePagerAdapter extends PagerAdapter {

    public abstract Object getItem(int position);

    public abstract int getRealCount();

    public abstract int getVirtualPosition(int position);
}
