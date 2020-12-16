//
//  NXTRenderStyle.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/15/14.
//
//

#ifndef __NexVideoEditor__NXTRenderStyle__
#define __NexVideoEditor__NXTRenderStyle__

#include <iostream>
#include <bitset>
#include "NexThemeRenderer_Internal.h"
#include "NXTVector.h"

class NXTRenderStyle {
    
public:
    
    class F {
    public:
        enum {
            color,
            ambientColor,
            diffuseColor,
            specularColor,
            lightDirection,
            alpha,
            animframe,
            renderDest,
            renderTest,
            maskSampling,
            shadingMode,
            lightAnchor,
            specexp,
            brightness,
            contrast,
            saturation,
            tintColor,
            antialias,
            depthtest,
            depthmask,
            lightingNormals,
            NUM_FIELDS
        };
    };
    
    std::bitset<static_cast<int>(F::NUM_FIELDS)> fieldSet;
    
    NXTVector           color;
    NXTVector           ambientColor;
    NXTVector           diffuseColor;
    NXTVector           specularColor;
    NXTVector           lightDirection;
    float               alpha;
    float               animframe;
    NXT_RenderDest      renderDest;
    NXT_RenderTest      renderTest;
    NXT_MaskSampling    maskSampling;
    NXT_ShadingMode     shadingMode;
    NXT_AnchorMode      lightAnchor;
    float				specexp;
    float               brightness;
    float               contrast;
    float               saturation;
    NXTVector           tintColor;
	bool                antialias;
	bool                depthtest;
	bool                depthmask;
    NXT_LightingNormals lightingNormals;
//    NXT_Matrix4f        mask_sample_transform;
//    bool                colorMask[4] = {false,false,false,false};

    NXTRenderStyle( const NXTRenderStyle& other) = default;
    
    NXTRenderStyle& operator|=( const NXTRenderStyle& other ) {
        if( other.fieldSet[F::color] )           color = other.color;
        if( other.fieldSet[F::ambientColor] )    ambientColor = other.ambientColor;
        if( other.fieldSet[F::diffuseColor] )    diffuseColor = other.diffuseColor;
        if( other.fieldSet[F::specularColor] )   specularColor = other.specularColor;
        if( other.fieldSet[F::lightDirection] )  lightDirection = other.lightDirection;
        if( other.fieldSet[F::alpha] )           alpha = other.alpha;
        if( other.fieldSet[F::animframe] )       animframe = other.animframe;
        if( other.fieldSet[F::renderDest] )      renderDest = other.renderDest;
        if( other.fieldSet[F::renderTest] )      renderTest = other.renderTest;
        if( other.fieldSet[F::maskSampling] )    maskSampling = other.maskSampling;
        if( other.fieldSet[F::shadingMode] )     shadingMode = other.shadingMode;
        if( other.fieldSet[F::lightAnchor] )     lightAnchor = other.lightAnchor;
        if( other.fieldSet[F::specexp] )         specexp = other.specexp;
        if( other.fieldSet[F::brightness] )      brightness = other.brightness;
        if( other.fieldSet[F::contrast] )        contrast = other.contrast;
        if( other.fieldSet[F::saturation] )      saturation = other.saturation;
        if( other.fieldSet[F::tintColor] )       tintColor = other.tintColor;
        if( other.fieldSet[F::antialias] )       antialias = other.antialias;
        if( other.fieldSet[F::depthtest] )       depthtest = other.depthtest;
        if( other.fieldSet[F::depthmask] )       depthmask = other.depthmask;
        if( other.fieldSet[F::lightingNormals] ) lightingNormals = other.lightingNormals;
    }
    
    NXTRenderStyle() {
        color               = NXTVector(0.5,0.5,0.5,1.0);
        ambientColor        = NXTVector(1.0,1.0,1.0,1.0);
        diffuseColor        = NXTVector(0.0,0.0,0.0,0.0);
        specularColor       = NXTVector(0.0,0.0,0.0,0.0);
        lightDirection      = NXTVector(10000.0,10000.0,10000.0,0.0);
        alpha               = 1.0;
        animframe           = 0;
        renderDest          = NXT_RenderDest_Screen;
        renderTest          = NXT_RenderTest_None;
        maskSampling        = NXT_MaskSampling_Normal;
        shadingMode         = NXT_ShadingMode_Off;
        lightAnchor         = NXT_AnchorMode_Camera;
        specexp             = 10;
        brightness          = 0.0;
        contrast            = 0.0;
        saturation          = 0.0;
        tintColor           = NXTVector(0.0,0.0,0.0,0.0);
        antialias           = false;
        depthtest           = false;
        depthmask           = false;
        lightingNormals     = NXT_LightingNormals_Front;
//        mask_sample_transform = NXT_Matrix4f_Identity();
    }
    
};

#endif /* defined(__NexVideoEditor__NXTRenderStyle__) */
