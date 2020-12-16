//
//  NXTNodeRotate.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/15/14.
//
//

#ifndef __NexVideoEditor__NXTNodeRotate__
#define __NexVideoEditor__NXTNodeRotate__

#include <iostream>

#include "NXTNode.h"

class NXTNodeRotate : public NXTNode {
    
private:
    float               angle               = 0.0;
    NXTVector           axis                = NXTVector(0,0,1,0);
    NXTMatrix           saved_transform;
    NXTTransformTarget  target              = NXTTransformTarget::Vertex;
    
public:
    
    virtual const char* nodeClassName() override  {
        return "Rotate";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="axis" ) {
            animFloatVectorFromString(attrValue, axis);
        } else if( attrName == "angle" ) {
            animFloatVectorFromString(attrValue, angle);
        } else if( attrName == "type" ) {
            parseTransformTarget(attrValue,target);
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override  {
        NXTMatrix& target_matrix = renderer.getTransformMatrix(target);
        saved_transform = target_matrix;
        target_matrix *= NXTMatrix::rotate(axis,angle*0.0174532925);
    }
    
    virtual Action renderEnd( IRenderContext& renderer ) override  {
        renderer.getTransformMatrix(target) = saved_transform;
        return Action::Next;
    }
    
    
};
#endif /* defined(__NexVideoEditor__NXTNodeRotate__) */
