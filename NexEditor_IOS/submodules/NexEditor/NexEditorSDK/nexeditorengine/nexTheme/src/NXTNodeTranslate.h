//
//  NXTNodeTranslate.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/17/14.
//
//

#ifndef __NexVideoEditor__NXTNodeTranslate__
#define __NexVideoEditor__NXTNodeTranslate__

#include <iostream>

#include "NXTNode.h"
#include "NXTRenderStyle.h"

class NXTNodeTranslate : public NXTNode {
    
private:
    NXTVector           vector;
    int                 repeat                  = 0;
    int                 repeats_done            = 0;
    bool                bJitter                 = false;
    unsigned int        jitterSeed              = 0;
    
    bool                bJitterAlpha            = false;
    float               jitter_alpha            = 0;
    
    NXTTransformTarget  target                  = NXTTransformTarget::Vertex;
    
    // Previous values (used to restore old values when traversing the tree)
    float               saved_alpha             = 0;
    NXTMatrix           saved_transform;
    
public:
    
    virtual const char* nodeClassName() override  {
        return "Translate";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName == "offset" ) {
            animFloatVectorFromString( attrValue, vector );
        } else if( attrName == "textureoffset" ) {
//            animFloatVectorFromString( attrValue, texoffs );
            throw NXTNodeParseError("'textureoffset' is obsolete; use type='texture' instead");
        } else if( attrName == "maskoffset" ) {
//            animFloatVectorFromString( attrValue, maskoffs );
            throw NXTNodeParseError("'maskoffset' is obsolete; use type='mask' instead");
        } else if( attrName == "repeat" ) {
            intFromString(attrValue,repeat);
        } else if( attrName == "jitterseed" ) {
            float jitterSeed = 0.0;
            floatVectorFromString( attrValue, jitterSeed );
            jitterSeed = (int)roundf(jitterSeed);
        } else if( attrName == "jitteralpha" ) {
            floatVectorFromString( attrValue, jitter_alpha );
            bJitterAlpha = 1;
        } else if( attrName == "jitter" ) {
            parseBool(attrValue, bJitter);
        } else if( attrName == "target" ) {
            parseTransformTarget(attrValue, target);
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override  {
        float jitter = 1.0;
        if(bJitter) {
            if(jitterSeed) {
                renderer.seed(jitterSeed, 295863);
            }
            jitter = renderer.randf();
        }
        
        NXTVector adjusted_vector = vector;
        adjusted_vector[0] *= jitter;
        adjusted_vector[1] *= jitter;
        adjusted_vector[2] *= jitter;
        
        saved_alpha = renderer.getRenderStyle().alpha;
        
        if( bJitterAlpha )
            renderer.getRenderStyle().alpha = jitter*jitter_alpha + (1-jitter)*saved_alpha;
        
        NXTMatrix& transform_matrix = renderer.getTransformMatrix(target);
        saved_transform = transform_matrix;
        if( repeat < 1 ) {
            transform_matrix *= NXTMatrix::translate( adjusted_vector );
        }
        repeats_done = 0;
    }
    
    virtual Action renderEnd( IRenderContext& renderer ) override  {
        repeats_done++;
        if( repeats_done < repeat ) {
            
            float jitter = 1.0;
            if(bJitter) {
                if(jitterSeed) {
                    renderer.seed(jitterSeed, 295863);
                }
                int i;
                for( i=0; i<repeats_done; i++ ) {
                    renderer.randf();
                }
                jitter = renderer.randf();
            }
            
            if( bJitterAlpha )
                renderer.getRenderStyle().alpha = jitter*renderer.getRenderStyle().alpha + (1-jitter)*saved_alpha;
            
            NXTVector adjusted_vector = vector * repeats_done;

            NXTMatrix& transform_matrix = renderer.getTransformMatrix(target);

            transform_matrix = saved_transform * NXTMatrix::translate(adjusted_vector);
            
            return Action::Repeat;
        } else {
            renderer.getRenderStyle().alpha = saved_alpha;
            renderer.getTransformMatrix(target) = saved_transform;
            return Action::Next;
        }
    }
    
    
};

#endif /* defined(__NexVideoEditor__NXTNodeTranslate__) */
