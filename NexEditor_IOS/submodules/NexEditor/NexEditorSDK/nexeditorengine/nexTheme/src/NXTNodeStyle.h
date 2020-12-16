//
//  NXTNodeStyle.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/17/14.
//
//

#ifndef __NexVideoEditor__NXTNodeStyle__
#define __NexVideoEditor__NXTNodeStyle__

#include <iostream>

#include "NXTNode.h"
#include "NXTRenderStyle.h"

class NXTNodeStyle : public NXTNode {
    
private:
    NXTRenderStyle style;
    NXTRenderStyle savedStyle;
    
public:
    
    virtual const char* nodeClassName() override  {
        return "Style";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName == "color" ) {
            animFloatVectorFromString( attrValue, style.color );
            style.fieldSet[NXTRenderStyle::F::color] = true;
        } else if( attrName == "alpha" ) {
            animFloatVectorFromString( attrValue, style.alpha );
            style.fieldSet[NXTRenderStyle::F::alpha] = true;
        } else if( attrName == "brightness" ) {
            animFloatVectorFromString( attrValue, style.brightness );
            style.fieldSet[NXTRenderStyle::F::brightness] = true;
        } else if( attrName == "contrast" ) {
            animFloatVectorFromString( attrValue, style.contrast );
            style.fieldSet[NXTRenderStyle::F::contrast] = true;
        } else if( attrName == "saturation" ) {
            animFloatVectorFromString( attrValue, style.saturation );
            style.fieldSet[NXTRenderStyle::F::saturation] = true;
        } else if( attrName == "tint" ) {
            animFloatVectorFromString( attrValue, style.tintColor );
            style.fieldSet[NXTRenderStyle::F::tintColor] = true;
        } else if( attrName == "animframe" ) {
            animFloatVectorFromString( attrValue, style.animframe );
            style.fieldSet[NXTRenderStyle::F::animframe] = true;
        } else if( attrName == "specularexponent" ) {
            animFloatVectorFromString( attrValue, &style.specexp, 1 );
            style.fieldSet[NXTRenderStyle::F::specexp] = true;
        } else if( attrName == "antialias" ) {
            parseBool(attrValue,style.antialias);
            style.fieldSet[NXTRenderStyle::F::antialias] = true;
        } else if( attrName == "depthtest" ) {
            parseBool(attrValue,style.depthtest);
            style.fieldSet[NXTRenderStyle::F::depthtest] = true;
        } else if( attrName == "depthmask" ) {
            parseBool(attrValue,style.depthmask);
            style.fieldSet[NXTRenderStyle::F::depthmask] = true;
        } else if( attrName == "ambientcolor" ) {
            animFloatVectorFromString( attrValue, style.ambientColor );
            style.fieldSet[NXTRenderStyle::F::ambientColor] = true;
        } else if( attrName == "diffusecolor" ) {
            animFloatVectorFromString( attrValue, style.diffuseColor );
            style.fieldSet[NXTRenderStyle::F::diffuseColor] = true;
        } else if( attrName == "specularcolor" ) {
            animFloatVectorFromString( attrValue, style.specularColor );
            style.fieldSet[NXTRenderStyle::F::specularColor] = true;
        } else if( attrName == "lightdirection" ) {
            animFloatVectorFromString( attrValue, style.lightDirection );
            style.fieldSet[NXTRenderStyle::F::lightDirection] = true;
        } else if( attrName == "shading" ) {
            if( attrValue == "off" ) {
                style.shadingMode = NXT_ShadingMode_Off;
                style.fieldSet[NXTRenderStyle::F::shadingMode] = true;
            } else if( attrValue == "flat" ) {
                style.shadingMode = NXT_ShadingMode_Flat;
                style.fieldSet[NXTRenderStyle::F::shadingMode] = true;
            }
        } else if( attrName == "lightingnormals" ) {
            if( attrValue == "front" ) {
                style.lightingNormals = NXT_LightingNormals_Front;
                style.fieldSet[NXTRenderStyle::F::lightingNormals] = true;
            } else if( attrValue == "back" ) {
                style.lightingNormals = NXT_LightingNormals_Back;
                style.fieldSet[NXTRenderStyle::F::lightingNormals] = true;
            } else if( attrValue == "both" ) {
                style.lightingNormals = NXT_LightingNormals_Both;
                style.fieldSet[NXTRenderStyle::F::lightingNormals] = true;
            }
        } else if( attrName == "lightanchor" ) {
            if( attrValue == "camera" ) {
                style.lightAnchor = NXT_AnchorMode_Camera;
                style.fieldSet[NXTRenderStyle::F::lightAnchor] = true;
            } else if( attrValue == "flat" ) {
                style.lightAnchor = NXT_AnchorMode_World;
                style.fieldSet[NXTRenderStyle::F::lightAnchor] = true;
            }
        } else if( attrName == "masksampling" ) {
            if( attrValue == "normal" ) {
                style.maskSampling = NXT_MaskSampling_Normal;
                style.fieldSet[NXTRenderStyle::F::maskSampling] = true;
            } else if( attrValue == "invert" ) {
                style.maskSampling = NXT_MaskSampling_Invert;
                style.fieldSet[NXTRenderStyle::F::maskSampling] = true;
            }
        } else if( attrName == "dest" ) {
            if( attrValue == "screen" ) {
                style.renderDest = NXT_RenderDest_Screen;
                style.fieldSet[NXTRenderStyle::F::renderDest] = true;
            } else if( attrValue == "stencil" ) {
                style.renderDest = NXT_RenderDest_Stencil;
                style.fieldSet[NXTRenderStyle::F::renderDest] = true;
            }
        } else if( attrName == "rendertest" ) {
            if( attrValue == "none" ) {
                style.renderTest = NXT_RenderTest_None;
                style.fieldSet[NXTRenderStyle::F::renderTest] = true;
            } else if( attrValue == "stencil" ) {
                style.renderTest = NXT_RenderTest_Stencil;
                style.fieldSet[NXTRenderStyle::F::renderTest] = true;
            } else if( attrValue == "stencilinvert" ) {
                style.renderTest = NXT_RenderTest_NegStencil;
                style.fieldSet[NXTRenderStyle::F::renderTest] = true;
            }
        }
    }
    
    virtual void renderBegin( IRenderContext &renderer ) override  {
        savedStyle = renderer.getRenderStyle();
        renderer.getRenderStyle() |= style;
        applyStyle( renderer, style, style.fieldSet );
    }
    
    virtual Action renderEnd( IRenderContext &renderer ) override  {
        
        renderer.getRenderStyle() = savedStyle;
        applyStyle( renderer, savedStyle, style.fieldSet );
        
        return Action::Next;
    }
    
private:
    template<unsigned int N>
    void applyStyle( IRenderContext &renderer, const NXTRenderStyle& style, const std::bitset<N>& fieldSet ) {
        
        if( fieldSet[NXTRenderStyle::F::depthtest ] ) {
            if( style.lightAnchor==NXT_AnchorMode_World ) {
                renderer.getRenderStyle().lightDirection =
                    renderer.getTransformMatrix(NXTTransformTarget::Vertex) * style.lightDirection;
            } else {
                renderer.getRenderStyle().lightDirection = style.lightDirection;
            }
        }

        if( fieldSet[NXTRenderStyle::F::depthtest ] ) {
            if( style.depthtest ) {
                glEnable(GL_DEPTH_TEST);
            } else {
                glDisable(GL_DEPTH_TEST);
            }
        }
        
        if( fieldSet[NXTRenderStyle::F::depthmask ] ) {
            if( style.depthmask ) {
                glDepthMask(GL_TRUE);
            } else {
                glDepthMask(GL_FALSE);
            }
        }
        
        if( fieldSet[NXTRenderStyle::F::maskSampling ] ) {
            NXTMatrix& mask_sample_transform = renderer.getTransformMatrix(NXTTransformTarget::MaskSample);
            switch( style.maskSampling ) {
                case NXT_MaskSampling_Invert:
                    mask_sample_transform = NXTMatrix::identity();
                    mask_sample_transform *= NXTMatrix::translate(1.0, 1.0, 1.0);
                    mask_sample_transform *= NXTMatrix::scale(-1.0, -1.0, -1.0);
                    break;
                case NXT_MaskSampling_Normal:
                default:
                    mask_sample_transform = NXTMatrix::identity();
                    break;
            }
        }
        
        if( fieldSet[NXTRenderStyle::F::renderDest ] ||
            fieldSet[NXTRenderStyle::F::renderTest ] )
        {
            bool bNeedStencil = false;
            switch (renderer.getRenderStyle().renderDest) {
                    
                case NXT_RenderDest_Stencil:
                    bNeedStencil = true;
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                    glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFU);
                    break;
                    
                case NXT_RenderDest_Screen:
                default:
                    renderer.resetColorMask();
                    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                    glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFU);
                    
                    break;
            }
            
            switch (renderer.getRenderStyle().renderTest) {
                case NXT_RenderTest_Stencil:
                    bNeedStencil = true;
                    glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFFU);
                    break;
                case NXT_RenderTest_NegStencil:
                    bNeedStencil = true;
                    glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFFU);
                    break;
                    
                case NXT_RenderTest_None:
                default:
                    break;
            }
            if( bNeedStencil ) {
                glEnable(GL_STENCIL_TEST);
            } else {
                glDisable(GL_STENCIL_TEST);
            }
        }
        
    }
    
    
};

#endif /* defined(__NexVideoEditor__NXTNodeStyle__) */
