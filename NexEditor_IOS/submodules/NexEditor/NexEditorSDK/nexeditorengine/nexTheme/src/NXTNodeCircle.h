//
//  NXTNodeCircle.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/9/14.
//
//

#ifndef __NexVideoEditor__NXTNodeCircle__
#define __NexVideoEditor__NXTNodeCircle__

#include <iostream>
#include <sstream>

#include "NXTNode.h"
#include "NXTVector.h"

class NXTNodeCircle : public NXTNode {
    
private:
    NXTVector           color;
    NXTVector           inner_color;
    NXTVector           center;
    float               r                 = 0;
    float               inner_r           = 0;
    int                 segments          = 0;
    bool                bColorSet         = false;
    bool                bInnerColorSet    = false;

public:
    
    virtual const char* nodeClassName() override {
        return "Circle";
    }
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override {
        if( attrName == "center" ) {
            animFloatVectorFromString(attrValue, center);
        } else if ( attrName == "radius" ) {
            animFloatVectorFromString(attrValue, r);
        } else if ( attrName == "innerradius" ) {
            animFloatVectorFromString(attrValue, inner_r);
        } else if ( attrName == "color" ) {
            animFloatVectorFromString(attrValue, color);
            bColorSet = true;
        } else if ( attrName == "innercolor" ) {
            animFloatVectorFromString(attrValue, inner_color);
            bInnerColorSet = true;
        } else if ( attrName == "segments" ) {
            std::istringstream(attrValue) >> segments;
            if( segments < 0 ) segments = 0;
            if( segments > 2048 ) segments = 2048;
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override;
        
private:
    
};


#endif /* defined(__NexVideoEditor__NXTNodeCircle__) */
