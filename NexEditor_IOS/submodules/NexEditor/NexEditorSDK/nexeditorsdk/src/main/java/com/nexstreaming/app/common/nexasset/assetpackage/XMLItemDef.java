package com.nexstreaming.app.common.nexasset.assetpackage;

import java.util.List;

public interface XMLItemDef {
    List<ItemParameterDef> getParameterDefinitions();
    int getTransitionOffset();
    int getTransitionOverlap();
    int getIntrinsicWidth();
    int getIntrinsicHeight();
}
