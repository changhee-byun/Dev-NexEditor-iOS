
#ifndef __NexVideoEditor__NXTNodeTexture__
#define __NexVideoEditor__NXTNodeTexture__

#include <iostream>
#include <sstream>
#include <iomanip>
#include "NXTNode.h"
#include "NXTNodeTheme.h"
#include "NXTNodeEffect.h"
#include "NXTUtil.h"
#include "NXTTextStyle.h"
#include "NXTImage.h"
#include "NXTImageSettings.h"
#include "NXTExternalPixelData.h"

class NXTNodeTexture : public NXTNode {
    
private:
    int videoSource = 0;
    NXT_TextureType textureType = NXT_TextureType_None;
    std::string path;
    std::string srcfield = "__title_text__";
    std::string text;
    std::string typeface;
    NXTTextStyle textStyle;
    NXTImage texImage;
    NXTImageSettings imageSettings;
    bool bNoCache = false;
    NXTExternalPixelData pixelData;
    int changeSerial = 0;
    
public:
    
    static const std::string* CLASS_NAME() {
        static const std::string s("Texture");
        return &s;
    }
    
    virtual const char* nodeClassName() override  {
        return "Texture";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        
        if( attrName=="src" ) {
            
            std::string baseId = "";
            for( NXTNode* node: parents() ) {
                if(node->is<NXTNodeTheme>() || node->is<NXTNodeEffect>())
                    baseId = node->getId();
            }
            path = "[ThemeImage]";
            path += baseId;
            path += "/";
            path += attrValue;
            textureType = NXT_TextureType_File;
            
        } else if( attrName=="video" ) {
            intFromString(attrValue, videoSource);
            textureType = NXT_TextureType_Video;
        } else if( attrName=="srcfield" ) {
            
            srcfield = isNodeRef(attrValue)?attrValue.substr(1):attrValue;
            if(textureType == NXT_TextureType_None) {
                textureType = NXT_TextureType_Overlay;
            }
        } else if( attrName=="text" ) {
            static const std::string needsEsc(";=%'\"");
            static const char *hexchars = "012345678ABCDEF";
            text = "";
            for( const char& c: attrValue ) {
                if( contains(needsEsc, c) ) {
                    text += '%';
                    text += hexchars[(c>>4)&0xF];
                    text += hexchars[c&0xF];
                } else {
                    text += c;
                }
            }
        } else if( attrName=="typeface" ) {
            typeface = attrValue;
            textureType = NXT_TextureType_Text;
        } else if( attrName=="width" ) {
            intFromString(attrValue, textStyle.reqWidth);
        } else if( attrName=="height" ) {
            intFromString(attrValue, textStyle.reqHeight);
        } else if( attrName=="maxlines" ) {
            intFromString(attrValue, textStyle.maxLines);
        } else if( attrName=="skewx" ) {
            animFloatVectorFromString(attrValue, textStyle.textSkewX);
        } else if( attrName=="scalex" ) {
            animFloatVectorFromString(attrValue, textStyle.textScaleX);
        } else if( attrName=="textsize" ) {
            animFloatVectorFromString(attrValue, textStyle.textSize);
        } else if( attrName=="textstrokewidth" ) {
            animFloatVectorFromString(attrValue, textStyle.textStrokeWidth);
        } else if( attrName=="fillcolor" ) {
            animFloatVectorFromString(attrValue, textStyle.fillColor);
        } else if( attrName == "strokecolor" ) {
            animFloatVectorFromString( attrValue, textStyle.strokeColor );
        } else if( attrName == "shadowcolor" ) {
            animFloatVectorFromString( attrValue, textStyle.shadowColor );
        } else if( attrName == "bgcolor" ) {
            animFloatVectorFromString( attrValue, textStyle.bgColor );
        } else if( attrName == "shadowoffset" ) {
            animFloatVectorFromString( attrValue, textStyle.shadowOffset );
        } else if( attrName == "shadowradius" ) {
            animFloatVectorFromString( attrValue, textStyle.shadowRadius );
        } else if( attrName == "textblur" ) {
            animFloatVectorFromString( attrValue, textStyle.textBlurRadius );
        } else if( attrName == "textmargin" ) {
            animFloatVectorFromString( attrValue, textStyle.textMargin );
        } else if( attrName == "spacingmult" ) {
            animFloatVectorFromString( attrValue, textStyle.spacingMult );
        } else if( attrName == "spacingadd" ) {
            animFloatVectorFromString( attrValue, textStyle.spacingAdd );
        } else if( attrName == "nocache" ) {
            parseBool( attrValue, bNoCache );
        } else if( attrName == "bold" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_BOLD );
        } else if( attrName == "strokebehind" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_STROKEBACK );
        } else if( attrName == "cutout" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_CUTOUT );
        } else if( attrName == "italic" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_ITALIC );
        } else if( attrName == "fill" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_FILL );
        } else if( attrName == "autosize" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_AUTOSIZE );
        } else if( attrName == "stroke" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_STROKE );
        } else if( attrName == "underline" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_UNDERLINE );
        } else if( attrName == "strike" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_STRIKE );
        } else if( attrName == "hinting" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_HINTING );
        } else if( attrName == "subpixel" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_SUBPIXEL );
        } else if( attrName == "shadow" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_SHADOW );
        } else if( attrName == "linear" ) {
            parseBool( attrValue, textStyle.textFlags, NXT_TEXTFLAG_LINEAR );
        } else if( attrName == "textblurtype" ) {
            if( attrValue == "normal" ) {
                textStyle.blurType = NXT_BLUR_NORMAL;
            } else if( attrValue == "inner" ) {
                textStyle.blurType = NXT_BLUR_INNER;
            } else if( attrValue == "outer" ) {
                textStyle.blurType = NXT_BLUR_OUTER;
            } else if( attrValue == "solid" ) {
                textStyle.blurType = NXT_BLUR_SOLID;
            } else {
                throw NXTNodeParseError("textblurtype must be one of: normal,inner,outer,solid");
            }
        } else if( attrName == "textalign" ) {
            if( attrValue == "left" ) {
                textStyle.textAlign = NXT_ALIGN_LEFT;
            } else if( attrValue == "center" ) {
                textStyle.textAlign = NXT_ALIGN_CENTER;
            } else if( attrValue == "right" ) {
                textStyle.textAlign = NXT_ALIGN_RIGHT;
            } else {
                animFloatVectorFromString(attrValue, textStyle.textAlign );
            }
        } else if( attrName == "textvalign" ) {
            if( attrValue == "top" ) {
                textStyle.textVAlign = NXT_VALIGN_TOP;
            } else if( attrValue == "center" ) {
                textStyle.textVAlign = NXT_VALIGN_CENTER;
            } else if( attrValue == "bottom" ) {
                textStyle.textVAlign = NXT_VALIGN_BOTTOM;
            } else {
                throw NXTNodeParseError("textvalign must be one of: top,center,bottom");
            }
        } else if( attrName == "animated" ) {
            float value[3] = {0};
            floatVectorFromString( attrValue, value );
            imageSettings.animRows = (int)(value[0]);
            imageSettings.animCols = (int)(value[1]);
            imageSettings.animFrames = (int)(value[2]);
            if( imageSettings.animRows < 1 ) {
                imageSettings.animRows = 1;
            }
            if( imageSettings.animCols < 1 ) {
                imageSettings.animCols = 1;
            }
            unsigned int maxFrames = imageSettings.animRows * imageSettings.animCols;
            if( imageSettings.animFrames < 1 || imageSettings.animFrames > maxFrames ) {
                imageSettings.animFrames = maxFrames;
            }
            imageSettings.bAnimated = 1;
        } else if( attrName == "wrap" ) {
            imageSettings.wrapS = NXTImageSettings::wrapModeFromString(attrValue);
            imageSettings.wrapT = imageSettings.wrapS;
        } else if( attrName == "wraps" ) {
            imageSettings.wrapS = NXTImageSettings::wrapModeFromString(attrValue);
        } else if( attrName == "wrapt" ) {
            imageSettings.wrapT = NXTImageSettings::wrapModeFromString(attrValue);
        } else if( attrName == "longtext" ) {
            if( attrValue == "crop-end" || attrValue == "crop" ) {
                textStyle.longText = NXT_LONGTEXT_CROP_END;
            } else if( attrValue == "wrap" ) {
                textStyle.longText = NXT_LONGTEXT_WRAP;
            } else if( attrValue == "ellipsize-start" ) {
                textStyle.longText = NXT_LONGTEXT_ELLIPSIZE_START;
            } else if( attrValue == "ellipsize-middle" ) {
                textStyle.longText = NXT_LONGTEXT_ELLIPSIZE_MIDDLE;
            } else if( attrValue == "ellipsize-end" ) {
                textStyle.longText = NXT_LONGTEXT_ELLIPSIZE_END;
            } else {
                throw NXTNodeParseError("longtext must be one of: crop-end,crop,wrap,ellipsize-start,ellipsize-middle,ellipsize-end");
            }
        }
    }
    
    virtual void precache( IRenderContext& renderer, bool openGLContextAvailable ) override;
    virtual void renderBegin( IRenderContext& renderer ) override;
    
    virtual DataProviderType getDataProviderType() override {
        return DataProviderType::Image;
    }
    
    virtual NXTImageAndSettings getDataProviderImageData() override {
        return NXTImageAndSettings(&texImage,&imageSettings);
    }

    
};

#endif /* defined(__NexVideoEditor__NXTNodeTexture__) */
