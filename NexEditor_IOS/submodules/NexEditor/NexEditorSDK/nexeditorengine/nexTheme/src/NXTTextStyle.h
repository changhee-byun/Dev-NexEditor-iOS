//
//  NXTTextStyle.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/23/14.
//
//

#ifndef __NexVideoEditor__NXTTextStyle__
#define __NexVideoEditor__NXTTextStyle__

#include <iostream>
#include "NexThemeRenderer_Internal.h"

class NXTTextStyle {
public:
    std::string         typeface;
    float               textSkewX       = 0.0;
    float               textScaleX      = 0.0;
    float               textSize        = 0.0;
    float               textStrokeWidth;
    float               spacingMult     = 1.0;
    float               spacingAdd      = 0.0;
    unsigned int        textFlags       = NXT_TEXTFLAG_AUTOSIZE;      // Any combination of NXT_TEXTFLAG_?????
    float               textAlign       = NXT_ALIGN_LEFT;      // NXT_ALIGN_LEFT, NXT_ALIGN_CENTER, NXT_ALIGN_RIGHT
    unsigned int        textVAlign      = NXT_VALIGN_TOP;      // NXT_VALIGN_TOP, NXT_VALIGN_CENTER, NXT_VALIGN_BOTTOM
    unsigned int        longText        = NXT_LONGTEXT_CROP_END;       // NXT_LONGTEXT_WRAP, NXT_LONGTEXT_CROP_END, NXT_LONGTEXT_ELLIPSIZE_START, NXT_LONGTEXT_ELLIPSIZE_MID, NXT_LONGTEXT_ELLIPSIZE_END
    unsigned int		blurType        = NXT_BLUR_NORMAL;      // NXT_BLUR_NORMAL, NXT_BLUR_SOLID, NXT_BLUR_INNER, NXT_BLUR_OUTER
    unsigned int        maxLines        = 0;
    NXT_Vector4f        fillColor       = (NXT_Vector4f){{0,0,0,0}};
    NXT_Vector4f        strokeColor     = (NXT_Vector4f){{0,0,0,0}};
    NXT_Vector4f        shadowColor     = (NXT_Vector4f){{0,0,0,0}};
    NXT_Vector4f        bgColor         = (NXT_Vector4f){{0,0,0,0}};
    float               shadowRadius    = 0.0;
    float               textBlurRadius  = 0.0;
    float               textMargin      = 0.0;
    NXT_Vector4f        shadowOffset    = (NXT_Vector4f){{0,0,0,0}};
    int                 reqWidth        = 0;
    int                 reqHeight       = 0;
};

#endif /* defined(__NexVideoEditor__NXTTextStyle__) */
