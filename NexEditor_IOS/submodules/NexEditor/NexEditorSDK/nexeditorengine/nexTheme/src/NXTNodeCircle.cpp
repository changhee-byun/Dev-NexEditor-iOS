//
//  NXTNodeCircle.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/9/14.
//
//

#include "NXTNodeCircle.h"
#include "NXTImageSettings.h"
#include "NXTVector.h"
#include "NXTShader.h"
#include "NXTVertexAttr.h"

void NXTNodeCircle::renderBegin( IRenderContext& renderer ) {

    renderer.clearMask();
    renderer.clearTexture();
    renderer.prepStateForRender();
    
    NXTVector color = bColorSet?this->color:renderer.getColor();
    
    NXTVector inner_color;
    if( bInnerColorSet ) {
        inner_color = this->inner_color;
    } else {
        inner_color = color;
    }
    
    float r0, r1;
    if( r < inner_r ) {
        r0 = r;
        r1 = inner_r;
    } else {
        r0 = inner_r;
        r1 = r;
    }
    
    if( r<=0 )
        return;
    
    int segments = this->segments;
    if( segments < 3 ) {
        segments = (int)(sqrtf(r1)*10);
        if( segments < 3 )
            segments = 3;
    }
    
    int maxVertexCount = (segments + 1)*2;
    int vertexCount = 0;
    
    GLfloat vertexCoord[maxVertexCount*4];
    GLfloat vertexColor[maxVertexCount*4];
    
    NXTDrawArraysMode mode;
    
    if( r0 > 0 ) {
        
        // Circle algorithm from http://slabode.exofire.net/circle_draw.shtml
        // (original code is marked "public domain" by the author)
        //
        // ... with some modifications (not public domain) by Matthew...
        
        float a = 6.283185307179586 / (float)segments;
        float c = cosf(a);
        float s = sinf(a);
        float t;
        
        float x = r1;
        float y = 0;
        float xi = r0;
        float yi = 0;
        
        int i;
        
        for(i = 0; i <= segments; i++)
        {
            vertexColor[vertexCount*4+0] = inner_color[0];
            vertexColor[vertexCount*4+1] = inner_color[1];
            vertexColor[vertexCount*4+2] = inner_color[2];
            vertexColor[vertexCount*4+3] = inner_color[3];
            vertexCoord[vertexCount*4+0] = center[0] + xi;
            vertexCoord[vertexCount*4+1] = center[1] + yi;
            vertexCoord[vertexCount*4+2] = center[2];
            vertexCoord[vertexCount*4+3] = center[3];
            vertexCount++;
            
            vertexColor[vertexCount*4+0] = color[0];
            vertexColor[vertexCount*4+1] = color[1];
            vertexColor[vertexCount*4+2] = color[2];
            vertexColor[vertexCount*4+3] = color[3];
            vertexCoord[vertexCount*4+0] = center[0] + x;
            vertexCoord[vertexCount*4+1] = center[1] + y;
            vertexCoord[vertexCount*4+2] = center[2];
            vertexCoord[vertexCount*4+3] = center[3];
            vertexCount++;
            
            t = x;
            x = c * x - s * y;
            y = s * t + c * y;
            
            t = xi;
            xi = c * xi - s * yi;
            yi = s * t + c * yi;
        }
        
        mode = NXTDrawArraysMode::TriangleStrip;
        
    } else {
        
        // Circle algorithm from http://slabode.exofire.net/circle_draw.shtml
        // (original code is marked "public domain" by the author)
        //
        // ... with some modifications (not public domain) by Matthew...
        
        float a = 6.283185307179586 / (float)segments;
        float c = cosf(a);
        float s = sinf(a);
        float t;
        
        float x = r1;
        float y = 0;
        
        int i;
        
        vertexColor[vertexCount*4+0] = inner_color[0];
        vertexColor[vertexCount*4+1] = inner_color[1];
        vertexColor[vertexCount*4+2] = inner_color[2];
        vertexColor[vertexCount*4+3] = inner_color[3];
        vertexCoord[vertexCount*4+0] = center[0];
        vertexCoord[vertexCount*4+1] = center[1];
        vertexCoord[vertexCount*4+2] = center[2];
        vertexCoord[vertexCount*4+3] = center[3];
        vertexCount++;
        
        for(i = 0; i < segments; i++)
        {
            vertexColor[vertexCount*4+0] = color[0];
            vertexColor[vertexCount*4+1] = color[1];
            vertexColor[vertexCount*4+2] = color[2];
            vertexColor[vertexCount*4+3] = color[3];
            vertexCoord[vertexCount*4+0] = center[0] + x;
            vertexCoord[vertexCount*4+1] = center[1] + y;
            vertexCoord[vertexCount*4+2] = center[2];
            vertexCoord[vertexCount*4+3] = center[3];
            vertexCount++;
            
            t = x;
            x = c * x - s * y;
            y = s * t + c * y;
        }
        
        vertexColor[vertexCount*4+0] = color[0];
        vertexColor[vertexCount*4+1] = color[1];
        vertexColor[vertexCount*4+2] = color[2];
        vertexColor[vertexCount*4+3] = color[3];
        vertexCoord[vertexCount*4+0] = vertexCoord[1*4+0];
        vertexCoord[vertexCount*4+1] = vertexCoord[1*4+1];
        vertexCoord[vertexCount*4+2] = vertexCoord[1*4+2];
        vertexCoord[vertexCount*4+3] = vertexCoord[1*4+3];
        vertexCount++;
        
        
        mode = NXTDrawArraysMode::TriangleFan;
        
    }
    
    renderer.getShader().vertexLocations().set( vertexCoord, vertexCount*4, 4);
    renderer.getShader().vertexColors().set(    vertexColor, vertexCount*4, 4);
    renderer.drawArrays(mode);
    
}
