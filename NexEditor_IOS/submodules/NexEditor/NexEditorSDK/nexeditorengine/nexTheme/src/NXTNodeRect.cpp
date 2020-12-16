#include "NXTNodeRect.h"
#include "NXTShader.h"
#include "NXTVertexAttr.h"

void NXTNodeRect::renderBegin( IRenderContext& renderer ) {
    
    
    NXTVector savedColor = renderer.getColor();
    if( bColorSet ) renderer.setColor(color_);
    
    renderer.setTexture(texture_);
    renderer.setMask(mask_);
    renderer.prepStateForRender();
    renderer.setColor(savedColor);

    GLfloat left = bounds_[0];
    GLfloat bottom = bounds_[1];
    GLfloat right = bounds_[2];
    GLfloat top = bounds_[3];
    
    if( anchor != NXTAnchor::None ) {
        processAnchor(left, top, right, bottom);
    }
    
    GLfloat vertexCoord[] = {
        left,   bottom,
        right,  bottom,
        left,   top,
        right,  top
    };
    
    GLfloat texCoords[] = {
        0,      0,
        1,      0,
        0,      1,
        1,      1
    };

    GLfloat maskCoords[] = {
        0,      0,
        1,      0,
        0,      1,
        1,      1
    };

    NXTVector color = bColorSet?color_:renderer.getColor();
    GLfloat vertexColor[16];
    for( int i=0; i<16; i++ ) {
        vertexColor[i] = color[i%4];
    }

    renderer.getShader().vertexLocations().set(vertexCoord,2);
    renderer.getShader().vertexTexCoords().set(texCoords,2);
    renderer.getShader().vertexMaskCoords().set(maskCoords,2);
    renderer.getShader().vertexColors().set(vertexColor,2);
    renderer.drawArrays(NXTDrawArraysMode::TriangleStrip);
    
}



void NXTNodeRect::processAnchor( float& left, float& top, float& right, float& bottom ) {
    
    if( anchor == NXTAnchor::None )
        return;
    
    float twidth, theight, width, height;
    float x = location_[0];
    float y = location_[1];
    
    if( (scaleMode==NXTScaleMode::FitMask || scaleMode==NXTScaleMode::FillMask) && mask_ ) {
        twidth = (float)mask_.pImage->getWidth();
        theight = (float)mask_.pImage->getHeight();
    } else if( texture_ ) {
        twidth = (float)texture_.pImage->getWidth();
        theight = (float)texture_.pImage->getHeight();
    } else if( mask_ ) {
        twidth = (float)mask_.pImage->getWidth();
        theight = (float)mask_.pImage->getHeight();
    } else {
        twidth = 10.0;
        theight = 10.0;
    }
    
    if( width_==0.0 && height_==0.0 ) {
        if( twidth > theight ) {
            width = 1.0;
            height = (theight/twidth);
        } else {
            width = (twidth/theight);
            height = 1.0;
        }
    } else if(width_==0.0 ) {
        width = (twidth/theight)*height_;
        height = height_;
    } else if(height_==0.0 ) {
        width = width_;
        height = (theight/twidth)*width_;
    } else {
        switch( scaleMode ) {
            case NXTScaleMode::Fit:
            case NXTScaleMode::FitMask:
            {
                float fitWidth = (twidth/theight)*height_;
                float fitHeight = (theight/twidth)*width_;
                if( fitWidth/width_ < fitHeight/height_ ) {
                    width = fitWidth;
                    height = height_;
                } else {
                    height = fitHeight;
                    width = width_;
                }
                break;
            }
            case NXTScaleMode::Fill:
            case NXTScaleMode::FillMask:
            {
                float fitWidth = (twidth/theight)*height_;
                float fitHeight = (theight/twidth)*width_;
                if( fitWidth/width_ > fitHeight/height_ ) {
                    width = fitWidth;
                    height = height_;
                } else {
                    height = fitHeight;
                    width = width_;
                }
                break;
            }
            case NXTScaleMode::Stretch:
            default:
                width = width_;
                height = height_;
                break;
        }
    }
    
    switch( anchor ) {
        case NXTAnchor::TopLeft:
            left = x;
            top = y;
            right = left+width;
            bottom = top-height;
            break;
        case NXTAnchor::TopRight:
            right = x;
            top = y;
            left = right-width;
            bottom = top-height;
            break;
        case NXTAnchor::TopCenter:
            left = x-(width/2.0);
            top = y;
            right = left+width;
            bottom = top-height;
            break;
        case NXTAnchor::BottomLeft:
            left = x;
            bottom = y;
            right = left+width;
            top = bottom+height;
            break;
        case NXTAnchor::BottomRight:
            right = x;
            bottom = y;
            left = right-width;
            top = bottom+height;
            break;
        case NXTAnchor::BottomCenter:
            left = x-(width/2.0);
            bottom = y;
            right = left+width;
            top = bottom+height;
            break;
        case NXTAnchor::LeftCenter:
            left = x;
            top = y+(height/2.0);
            right = left+width;
            bottom = top-height;
            break;
        case NXTAnchor::RightCenter:
            right = x;
            top = y+(height/2.0);
            left = right-width;
            bottom = top-height;
            break;
        case NXTAnchor::Center:
            left = x-(width/2.0);
            top = y+(height/2.0);
            right = left+width;
            bottom = top-height;
            break;
        default:
            left = x-(width/2.0);
            top = y+(height/2.0);
            right = left+width;
            bottom = top-height;
            break;
    }
    
    float swap = top;
    top = bottom;
    bottom = swap;

    
}
