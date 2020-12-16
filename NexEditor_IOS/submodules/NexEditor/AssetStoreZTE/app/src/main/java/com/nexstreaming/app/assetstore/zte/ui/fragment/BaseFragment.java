package com.nexstreaming.app.assetstore.zte.ui.fragment;

import android.content.Context;
import android.support.v4.app.Fragment;

/**
 * Created by ojin.kwon on 2016-11-18.
 */

public abstract class BaseFragment extends Fragment {



    public abstract String getTitle(Context context);

    public abstract int getImageResource();
}
