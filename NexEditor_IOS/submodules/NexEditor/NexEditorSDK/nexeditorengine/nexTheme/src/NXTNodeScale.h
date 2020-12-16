#ifndef __NexVideoEditor__NXTNodeScale__
#define __NexVideoEditor__NXTNodeScale__

#include <iostream>
#include <bitset>
#include "NXTNode.h"
#include "NXTMatrix.h"

class NXTNodeScale : public NXTNode {
    
private:
    
    enum {
        COMPONENT_X,
        COMPONENT_Y,
        COMPONENT_Z,
        COMPONENT_UNIFORM
    };
    
    std::bitset<4>      components          = 0;
    NXTVector           factor;
    NXTMatrix           saved_transform;
    NXTTransformTarget  target              = NXTTransformTarget::Vertex;
    bool                bInverse            = false;
    
public:
    
    virtual const char* nodeClassName() override  {
        return "Scale";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName == "x" && !components[COMPONENT_X] ) {
            animFloatVectorFromString( attrName, &factor[0], 1 );
            components[COMPONENT_X]         = true;
        } else if( attrName == "y" && !components[COMPONENT_Y] ) {
            animFloatVectorFromString( attrName, &factor[1], 1 );
            components[COMPONENT_Y]         = true;
        } else if( attrName == "z" && !components[COMPONENT_Z] ) {
            animFloatVectorFromString( attrName, &factor[2], 1 );
            components[COMPONENT_Z]         = true;
        } else if( attrName == "uniform" && !components[COMPONENT_UNIFORM] ) {
            animFloatVectorFromString( attrName, &factor[0], 1 );
            components[COMPONENT_X]         = true;
            components[COMPONENT_Y]         = true;
            components[COMPONENT_Z]         = true;
            components[COMPONENT_UNIFORM]   = true;
        } else if( attrName == "inverse" ) {
            parseBool(attrValue,bInverse);
        } else if( attrName == "type" ) {
            parseTransformTarget(attrValue,target);
        }
    }

    virtual void renderBegin( IRenderContext& renderer ) override {
        
        if( components[COMPONENT_UNIFORM] ) {
            factor[1] = factor[0];
            factor[2] = factor[0];
        } else {
            if( !components[COMPONENT_X] ) factor[0] = 1.0;
            if( !components[COMPONENT_Y] ) factor[1] = 1.0;
            if( !components[COMPONENT_Z] ) factor[2] = 1.0;
        }
        factor[3] = 1.0;
        
        if( bInverse ) {
            factor[0] = 1.0/factor[0];
            factor[1] = 1.0/factor[1];
            factor[2] = 1.0/factor[2];
            factor[3] = 1.0/factor[3];
        }
        
        NXTMatrix& targetMatrix = renderer.getTransformMatrix(target);
        saved_transform = targetMatrix;
        targetMatrix *= NXTMatrix::scale(factor[0], factor[1], 1.0);
        
    }
    
    virtual Action renderEnd( IRenderContext& renderer ) override {

        renderer.getTransformMatrix(target) = saved_transform;

        return Action::Next;
    }
    
};

#endif /* defined(__NexVideoEditor__NXTNodeScale__) */
