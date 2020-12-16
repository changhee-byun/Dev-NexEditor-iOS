//
//  NXTNodeTriangleStrip.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/5/14.
//
//

#ifndef __NexVideoEditor__NXTNodeTriangleStrip__
#define __NexVideoEditor__NXTNodeTriangleStrip__

#include <iostream>
#include "NXTNode.h"
#include "NXTImage.h"
#include "NXTVector.h"

class NXTNodeCoordinateSpace;

class NXTNodeTriangleStrip : public NXTNode {
    
public:
    
    class PointComponent {
    public:
        enum {
            location    = 0x01,
            texcoord    = 0x02,
            color       = 0x04,
            maskcoord   = 0x08,
            normal      = 0x10
        };
    };

private:

    NXTImageAndSettings texture_;
    NXTImageAndSettings mask_;
    NXTImage* texture_image = nullptr;
    NXTImage* mask_image = nullptr;
    NXTDrawArraysMode type = NXTDrawArraysMode::TriangleStrip;
    NXTNodeCoordinateSpace *vertexSpace = nullptr;
    NXTNodeCoordinateSpace *maskSpace = nullptr;
    NXTNodeCoordinateSpace *textureSpace = nullptr;
    int numpoints = 0;
    std::vector<float> float_buffers;
    std::vector<unsigned char> component_flags;

    float               *vertexCoord        = nullptr;
    float               *adjVertexCoord     = nullptr;
    float               *vertexColor        = nullptr;
    float               *vertexNormal       = nullptr;
    float               *textureCoord       = nullptr;
    float               *adjTextureCoord    = nullptr;
    float               *maskCoord          = nullptr;
    float               *adjMaskCoord       = nullptr;

public:
    
    virtual const char* nodeClassName() override {
        return "TriangleStrip";
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
        } else if( attrName == "type") {
            if( attrValue == "trianglestrip") {
                type = NXTDrawArraysMode::TriangleStrip;
            } else if( attrValue == "trianglefan") {
                type = NXTDrawArraysMode::TriangleFan;
            } else if( attrValue == "triangles") {
                type = NXTDrawArraysMode::Triangles;
            } else if( attrValue == "strip") {
                type = NXTDrawArraysMode::TriangleStrip;
            } else if( attrValue == "fan") {
                type = NXTDrawArraysMode::TriangleFan;
            } else if( attrValue == "triangles") {
                type = NXTDrawArraysMode::Triangles;
            } else if( attrValue == "points") {
                type = NXTDrawArraysMode::Points;
            } else if( attrValue == "lines") {
                type = NXTDrawArraysMode::Lines;
            } else if( attrValue == "linestrip") {
                type = NXTDrawArraysMode::LineStrip;
            } else if( attrValue == "lineloop") {
                type = NXTDrawArraysMode::LineLoop;
            } else {
                throw NXTNodeParseError("type must be one of: trianglestrip,trianglefan,triangles,strip,fan,points,lines,linestrip,lineloop");
            }
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override;
    
    virtual void childrenDone() override;

    
private:
    
};

#endif /* defined(__NexVideoEditor__NXTNodeTriangleStrip__) */
