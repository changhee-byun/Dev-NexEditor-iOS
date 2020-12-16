#ifndef __NexVideoEditor__NXTNodePolygon__
#define __NexVideoEditor__NXTNodePolygon__

#include <iostream>
#include <vector>
#include "NXTNode.h"
#include "NXTImageSettings.h"
#include "NXTShader.h"
#include "NXTVertexAttr.h"

class NXTNodePolygon : public NXTNode {
    
private:
    bool                bNoCache = false;
    bool                bSetColor = false;

    int                 numPolyPoints;
    std::vector<float>  polyPoints;
    int                 vertexCapacity;
    int                 vertexCount = 0;
    std::vector<float>  vertexCoord;
    std::vector<float>  vertexColor;
    NXTVector           polyColor;

public:
    
    virtual const char* nodeClassName() override  {
        return "Polygon";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName == "fill" ) {
            animFloatVectorFromString(attrValue, polyColor);
            bSetColor = true;
        } else if( attrName == "color" ) {
            animFloatVectorFromString(attrValue, polyColor);
            bSetColor = true;
        } else if( attrName == "points" ) {
            
            animFloatVectorFromString(attrValue, polyPoints, &bNoCache );
            numPolyPoints = polyPoints.size()/2;
            vertexCapacity = (numPolyPoints-2)*3;
            vertexCoord.resize(vertexCapacity*4,0.0);
            vertexColor.resize(vertexCapacity*4,0.0);
            
            if( !bNoCache ) {
                tesselate();
            }
            
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override  {
        
        if( bNoCache ) {
            tesselate();
        }
        
        NXTVector color = bSetColor?polyColor:renderer.getColor();
        for( int i=0; i<vertexCount; i++ ) {
            vertexColor[4*i+0] = color[0];
            vertexColor[4*i+1] = color[1];
            vertexColor[4*i+2] = color[2];
            vertexColor[4*i+3] = color[3];
        }
        
        renderer.clearTexture();
        renderer.clearMask();
        renderer.prepStateForRender();
        
        renderer.getShader().vertexLocations().set(vertexCoord);
        renderer.getShader().vertexColors().set(vertexColor);
        renderer.drawArrays(NXTDrawArraysMode::Triangles);
        
    }

    
private:
    void tesselate();
    
};
#endif /* defined(__NexVideoEditor__NXTNodePolygon__) */
