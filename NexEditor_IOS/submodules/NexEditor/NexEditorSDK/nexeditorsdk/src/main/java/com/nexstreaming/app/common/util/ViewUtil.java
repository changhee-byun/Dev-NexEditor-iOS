package com.nexstreaming.app.common.util;

import android.view.View;
import android.view.ViewParent;
import android.widget.AbsListView;
import android.widget.GridView;
import android.widget.ListView;
import android.widget.ScrollView;

public class ViewUtil {
    private ViewUtil(){}

    public static boolean hasScrollableParent(View v) {
        if( v instanceof ListView || v instanceof ScrollView || v instanceof GridView || v instanceof AbsListView )
            return true;
        return v != null && (v.isScrollContainer() || hasScrollableParent(v.getParent()));
    }

    public static boolean hasScrollableParent(ViewParent v) {
        if( v instanceof ListView || v instanceof ScrollView || v instanceof GridView || v instanceof AbsListView )
            return true;
        return v != null && ( (v instanceof View && ((View)v).isScrollContainer()) || hasScrollableParent(v.getParent()));
    }

}
