package com.nexstreaming.app.common.nexasset.assetpackage;

import android.graphics.RectF;

import java.util.List;
import java.util.Map;

public interface ItemParameterDef {

    interface Option {
        Map<String,Map<String,String>> getStrings();
        String getIconPath();
        String getValue();
    }

    ItemParameterType getType();
    String getDefaultValue();
    String getOffValue();
    String getOnValue();
    String getId();
    int getMaxLength();
    boolean isMultiline();
    boolean isPrivate();
    int getMinimumValue();
    int getMaximumValue();
    int getStepSize();
    RectF getBounds();
    Map<String,Map<String,String>> getStrings();
    List<Option> getOptions();
    String getIconPath();

}
