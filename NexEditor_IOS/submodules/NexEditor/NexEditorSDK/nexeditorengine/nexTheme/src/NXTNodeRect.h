#ifndef __NexVideoEditor__NXTNodeRect__
#define __NexVideoEditor__NXTNodeRect__

#include <iostream>

#include "NXTNode.h"
#include "NXTVector.h"

enum class NXTAnchor {
    None             = 0,
    TopLeft          = 1,
    TopCenter        = 2,
    TopRight         = 3,
    RightCenter      = 4,
    BottomRight      = 5,
    BottomCenter     = 6,
    BottomLeft       = 7,
    LeftCenter       = 8,
    Center           = 9
};

enum class NXTScaleMode {
    Stretch       = 0,
    Fit           = 1,
    Fill          = 2,
    FitMask       = 3,
    FillMask      = 4
};


class NXTNodeRect : public NXTNode {
    
private:
    NXTImageAndSettings texture_;
    NXTImageAndSettings mask_;
    NXTVector           bounds_;
    NXTVector           color_;
    bool                bColorSet = false;
    NXTVector           location_;
    float               width_        = 0.0;
    float               height_       = 0.0;
    NXTAnchor           anchor = NXTAnchor::None;
    NXTScaleMode        scaleMode = NXTScaleMode::Stretch;
    
public:
    
    virtual const char* nodeClassName() override {
        return "Rect";
    }
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override {
        if( attrName == "texture" ) {
            if( NXTNode *pNode = findNodeByRef(attrName, DataProviderType::Image) ) {
                texture_ = pNode->getDataProviderImageData();
            }
        } else if( attrName == "mask" ) {
            if( NXTNode *pNode = findNodeByRef(attrName, DataProviderType::Image) ) {
                mask_ = pNode->getDataProviderImageData();
            }
        } else if( attrName == "location" ) {
            if( anchor==NXTAnchor::None )
                anchor==NXTAnchor::Center;
            animFloatVectorFromString(attrValue, location_);
        } else if( attrName == "width" ) {
            animFloatVectorFromString(attrValue, width_);
        } else if( attrName == "height" ) {
            animFloatVectorFromString(attrValue, height_);
        } else if( attrName == "scalemode" ) {
            if( attrValue == "stretch") {
                scaleMode = NXTScaleMode::Stretch;
            } else if( attrValue == "fit") {
                scaleMode = NXTScaleMode::Fit;
            } else if( attrValue == "fill") {
                scaleMode = NXTScaleMode::Fill;
            } else if( attrValue == "fitmask") {
                scaleMode = NXTScaleMode::FitMask;
            } else if( attrValue == "fillmask") {
                scaleMode = NXTScaleMode::FillMask;
            } else {
                throw NXTNodeParseError("scalemode must be one of: fit,fill,fitmask,fillmask");
            }
        } else if( attrName == "anchor" ) {
            bool bLeft = 0;
            bool bRight = 0;
            bool bTop = 0;
            bool bBottom = 0;
            bool bCenter = 0;
            if( attrValue.find("left"  )!=std::string::npos ) bLeft   = true;
            if( attrValue.find("right" )!=std::string::npos ) bRight  = true;
            if( attrValue.find("top"   )!=std::string::npos ) bTop    = true;
            if( attrValue.find("bottom")!=std::string::npos ) bBottom = true;
            if( attrValue.find("center")!=std::string::npos ) bCenter = true;
            if( bLeft && bRight ) {
                bLeft=bRight=false;
                bCenter=true;
            }
            if( bTop && bBottom ) {
                bTop=bBottom=false;
                bCenter=true;
            }
            if( bTop && bLeft ) {
                anchor = NXTAnchor::TopLeft;
            } else if( bTop && bCenter ) {
                anchor = NXTAnchor::TopCenter;
            } else if( bTop && bRight ) {
                anchor = NXTAnchor::TopRight;
            } else if( bRight && bCenter ) {
                anchor = NXTAnchor::RightCenter;
            } else if( bBottom && bRight ) {
                anchor = NXTAnchor::BottomRight;
            } else if( bBottom && bCenter ) {
                anchor = NXTAnchor::BottomCenter;
            } else if( bBottom && bLeft ) {
                anchor = NXTAnchor::BottomLeft;
            } else if( bLeft && bCenter ) {
                anchor = NXTAnchor::LeftCenter;
            } else if( bCenter ) {
                anchor = NXTAnchor::Center;
            } else if( bTop ) {
                anchor = NXTAnchor::TopCenter;
            } else if( bBottom ) {
                anchor = NXTAnchor::BottomCenter;
            } else if( bLeft ) {
                anchor = NXTAnchor::LeftCenter;
            } else if( bRight ) {
                anchor = NXTAnchor::RightCenter;
            } else {
                anchor = NXTAnchor::Center;
            }
        } else if( attrName == "color" ) {
            animFloatVectorFromString(attrValue, color_);
            bColorSet = true;
        } else if( attrName == "bounds" ) {
            animFloatVectorFromString(attrValue, bounds_);
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override;
    
private:
    
    void processAnchor( float& left, float& top, float& right, float& bottom );
    
};


#endif /* defined(__NexVideoEditor__NXTNodeRect__) */
