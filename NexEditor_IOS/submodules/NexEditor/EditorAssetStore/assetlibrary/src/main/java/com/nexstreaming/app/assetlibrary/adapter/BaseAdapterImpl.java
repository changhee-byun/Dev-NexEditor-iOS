package com.nexstreaming.app.assetlibrary.adapter;

import java.util.List;

/**
 * Created by ojin.kwon on 2016-11-18.
 */

public interface BaseAdapterImpl<T> {

    T getItem(int pos);
    int getItemCount();
    List<T> getData();
}
