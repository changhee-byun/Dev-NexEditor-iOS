#ifndef __NexVideoEditor__NXTNodeClear__
#define __NexVideoEditor__NXTNodeClear__

#include <iostream>
#include "NXTNode.h"
#include "NXTShader.h"
#include "NXTVertexAttr.h"

class NXTNodeClear : public NXTNode {
    
private:
    NXT_Vector4f        color       = (NXT_Vector4f){0};
    bool                bStencil    = false;
    
public:
    
    virtual const char* nodeClassName() override {
        return "Clear";
    }

    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override {
        if( attrName=="color" ) {
            animFloatVectorFromString(attrValue, color);
        } else if ( attrName=="stencil" ) {
            parseBool(attrValue, bStencil);
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override {
        
        if( bStencil ) {
            glClearStencil(0);
            glClearStencil(GL_STENCIL_BUFFER_BIT);
            return;
        }
        
        renderer.prepStateForRender();
        
        GLfloat vertexCoord[] = {
            -1.0,     -1.0,
            1.0,      -1.0,
            -1.0,     1.0,
            1.0,      1.0
        };
        
        GLfloat vertexColor[] = {
            color.e[0],color.e[1],color.e[2],color.e[3],
            color.e[0],color.e[1],color.e[2],color.e[3],
            color.e[0],color.e[1],color.e[2],color.e[3],
            color.e[0],color.e[1],color.e[2],color.e[3]
        };
        
        renderer.getShader().vertexLocations().set(vertexCoord,2);
        renderer.getShader().vertexColors().set(vertexColor,4);
        renderer.drawArrays(NXTDrawArraysMode::TriangleStrip);
        
    }
    
};
#endif /* defined(__NexVideoEditor__NXTNodeClear__) */
