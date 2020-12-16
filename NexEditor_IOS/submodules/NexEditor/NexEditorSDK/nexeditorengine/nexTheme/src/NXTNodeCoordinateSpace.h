//
//  NXTNodeCoordinateSpace.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/9/14.
//
//

#ifndef __NexVideoEditor__NXTNodeCoordinateSpace__
#define __NexVideoEditor__NXTNodeCoordinateSpace__

#include <iostream>
#include "NXTNode.h"

enum class NXTCoordType {
    None, Vertex, Texture, Mask
};

class NXTNodeCoordinateSpace : public NXTNode {
    
private:
    float               width;
    float               height;
    float               originx;
    float               originy;
    bool                bYUp;
    NXTCoordType        applyTo;
    
public:
    
    void adjustCoordBounded(float& coord, int index) {
        adjustCoord(coord,index);
        if( coord<0.0 )
            coord=0.0;
        if( coord>1.0 )
            coord=1.0;
    }
    
    static const std::string* CLASS_NAME() {
        static const std::string s("CoordinateSpace");
        return &s;
    }

    NXTCoordType getApplyTo() {
        return applyTo;
    }
    
    virtual const char* nodeClassName() override {
        return "CoordinateSpace";
    }
    
    NXTNodeCoordinateSpace() {
        width = 0;
        height = 0;
        originx = 0;
        originy = 0;
        bYUp = false;
        applyTo = NXTCoordType::None;
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override {
        if( attrName=="width" ) {
            floatVectorFromString(attrValue, width);
            if( applyTo == NXTCoordType::None )
                applyTo = NXTCoordType::Vertex;
        } else if ( attrName=="height" ) {
            floatVectorFromString(attrValue, height);
            if( applyTo == NXTCoordType::None )
                applyTo = NXTCoordType::Vertex;
        } else if ( attrName=="origin" ) {
            float origin[] = {0,0};
            floatVectorFromString(attrValue, origin);
            originx = origin[0];
            originy = origin[1];
            if( applyTo == NXTCoordType::None )
                applyTo = NXTCoordType::Vertex;
        } else if ( attrName=="yup" ) {
            parseBool(attrValue, bYUp);
            if( applyTo == NXTCoordType::None )
                applyTo = NXTCoordType::Vertex;
        } else if ( attrName=="applyto" ) {
            if( attrValue=="vertex" ) {
                applyTo = NXTCoordType::Vertex;
            } else if( attrValue=="mask" ) {
                applyTo = NXTCoordType::Mask;
            } else if( attrValue=="texture" ) {
                applyTo = NXTCoordType::Texture;
            } else {
                throw NXTNodeParseError("applyto must be one of: vertex,mask,texture");
            }
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override  {
        
    }
    
    void adjustCoord( float& coord, int index ) {
        switch( index ) {
            case 0:         // X coord
                if( applyTo==NXTCoordType::Vertex ) {
                    coord = (coord + originx) / width * 2.0 - 1.0;
                } else {
                    coord = (coord + originx) / width;
                }
                break;
            case 1:         // Y coord
                if( bYUp ) {
                    if( applyTo==NXTCoordType::Vertex ) {
                        coord = (coord + originy) / height * 2.0 - 1.0;
                    } else {
                        coord = (coord + originy) / height;
                    }
                } else {
                    if( applyTo==NXTCoordType::Vertex ) {
                        coord = ((height-coord) + originy) / height * 2.0 - 1.0;
                    } else {
                        coord = ((height-coord) + originy) / height;
                    }
                }
                break;
            case 2:         // Z coord
            case 3:         // W coord
            default:
                break;
        }
    }
    
};

#endif /* defined(__NexVideoEditor__NXTNodeCoordinateSpace__) */
