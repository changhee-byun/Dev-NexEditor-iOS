#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_Style"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Style = {
    nodeInitFunc,
    nodeFreeFunc,
    nodeSetAttrFunc,
    nodeChildrenDoneFunc,
    nodeDebugPrintFunc,
    nodeBeginRender,
    nodeEndRender,
    NULL,//precacheFunc
    NULL,//getPrecacheResourceFunc
    NULL,//nodeUpdateNotify
    NULL,//freeTextureFunc
    "Style",
    sizeof(NXT_Node_Style)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_Style *nodeData = (NXT_Node_Style*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Style *nodeData = (NXT_Node_Style*)node;
    if( strcasecmp(attrName, "color")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->color.e, sizeof(nodeData->color.e)/sizeof(*(nodeData->color.e)) );
        nodeData->fieldset |= NXT_StyleField_Color;
    } else if( strcasecmp(attrName, "alpha")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->alpha, 1 );
        nodeData->fieldset |= NXT_StyleField_Alpha;
    } else if( strcasecmp(attrName, "brightness")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->brightness, 1 );
        nodeData->fieldset |= NXT_StyleField_Brightness;
    } else if( strcasecmp(attrName, "contrast")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->contrast, 1 );
        nodeData->fieldset |= NXT_StyleField_Contrast;
    } else if( strcasecmp(attrName, "saturation")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->saturation, 1 );
        nodeData->fieldset |= NXT_StyleField_Saturation;
    } else if( strcasecmp(attrName, "tint")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->tintColor.e, sizeof(nodeData->tintColor.e)/sizeof(*(nodeData->tintColor.e)) );
        nodeData->fieldset |= NXT_StyleField_Tint;
    } else if( strcasecmp(attrName, "animframe")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->animframe, 1 );
        nodeData->fieldset |= NXT_StyleField_AnimFrame;
    } else if( strcasecmp(attrName, "specularexponent")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->specexp, 1 );
        nodeData->fieldset |= NXT_StyleField_SpecExp;
    } else if( strcasecmp(attrName, "antialias")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->fieldset |= NXT_StyleField_Antialias;
				nodeData->antialias = 1;
                break;
            case TFUFalse:
                nodeData->fieldset |= NXT_StyleField_Antialias;
				nodeData->antialias = 0;
                break;
            default:
                break;
                
        }
    } else if( strcasecmp(attrName, "depthtest")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->fieldset |= NXT_StyleField_DepthTest;
				nodeData->depthtest = 1;
                break;
            case TFUFalse:
                nodeData->fieldset |= NXT_StyleField_DepthTest;
				nodeData->depthtest = 0;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "depthmask")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->fieldset |= NXT_StyleField_DepthMask;
				nodeData->depthmask = 1;
                break;
            case TFUFalse:
                nodeData->fieldset |= NXT_StyleField_DepthMask;
				nodeData->depthmask = 0;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "ambientcolor")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->ambientColor.e, sizeof(nodeData->ambientColor.e)/sizeof(*(nodeData->ambientColor.e)) );
        nodeData->fieldset |= NXT_StyleField_AmbientColor;
    } else if( strcasecmp(attrName, "diffusecolor")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->diffuseColor.e, sizeof(nodeData->diffuseColor.e)/sizeof(*(nodeData->diffuseColor.e)) );
        nodeData->fieldset |= NXT_StyleField_DiffuseColor;
    } else if( strcasecmp(attrName, "specularcolor")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->specularColor.e, sizeof(nodeData->specularColor.e)/sizeof(*(nodeData->specularColor.e)) );
        nodeData->fieldset |= NXT_StyleField_SpecularColor;
    } else if( strcasecmp(attrName, "lightdirection")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->lightDirection.e, sizeof(nodeData->lightDirection.e)/sizeof(*(nodeData->lightDirection.e)) );
        nodeData->fieldset |= NXT_StyleField_LightDirection;
    } else if( strcasecmp(attrName, "shading")==0 ) {
        if( strcasecmp(attrValue,"off")==0 ) {
            nodeData->shadingMode = NXT_ShadingMode_Off;
            nodeData->fieldset |= NXT_StyleField_ShadingMode;
        } else if( strcasecmp(attrValue,"flat")==0 ) {
            nodeData->shadingMode = NXT_ShadingMode_Flat;
            nodeData->fieldset |= NXT_StyleField_ShadingMode;
        }
    } else if( strcasecmp(attrName, "lightingnormals")==0 ) {
        if( strcasecmp(attrValue,"front")==0 ) {
            nodeData->lightingNormals = NXT_LightingNormals_Front;
            nodeData->fieldset |= NXT_StyleField_LightingNormals;
        } else if( strcasecmp(attrValue,"back")==0 ) {
            nodeData->lightingNormals = NXT_LightingNormals_Back;
            nodeData->fieldset |= NXT_StyleField_LightingNormals;
        } else if( strcasecmp(attrValue,"both")==0 ) {
            nodeData->lightingNormals = NXT_LightingNormals_Both;
            nodeData->fieldset |= NXT_StyleField_LightingNormals;
        }
    } else if( strcasecmp(attrName, "lightanchor")==0 ) {
        if( strcasecmp(attrValue,"camera")==0 ) {
            nodeData->lightAnchor = NXT_AnchorMode_Camera;
            nodeData->fieldset |= NXT_StyleField_LightAnchor;
        } else if( strcasecmp(attrValue,"flat")==0 ) {
            nodeData->lightAnchor = NXT_AnchorMode_World;
            nodeData->fieldset |= NXT_StyleField_LightAnchor;
        }
    } else if( strcasecmp(attrName, "masksampling")==0 ) {
        if( strcasecmp(attrValue,"normal")==0 ) {
            nodeData->maskSampling = NXT_MaskSampling_Normal;
            nodeData->fieldset |= NXT_StyleField_MaskSample;
        } else if( strcasecmp(attrValue,"invert")==0 ) {
            nodeData->maskSampling = NXT_MaskSampling_Invert;
            nodeData->fieldset |= NXT_StyleField_MaskSample;
        }
    } else if( strcasecmp(attrName, "dest")==0 ) {
        if( strcasecmp(attrValue,"screen")==0 ) {
            nodeData->renderDest = NXT_RenderDest_Screen;
            nodeData->fieldset |= NXT_StyleField_RenderDest;
        } else if( strcasecmp(attrValue,"stencil")==0 ) {
            nodeData->renderDest = NXT_RenderDest_Stencil;
            nodeData->fieldset |= NXT_StyleField_RenderDest;
        } else if( strcasecmp(attrValue,"mask")==0 ) {
            nodeData->renderDest = NXT_RenderDest_Mask;
            nodeData->fieldset |= NXT_StyleField_RenderDest;
        }

    } else if( strcasecmp(attrName, "compmode")==0 ) {
        if( strcasecmp(attrValue,"screen")==0 ) {
            nodeData->compMode = NXT_CompMode_Screen;
            nodeData->fieldset |= NXT_StyleField_CompMode;
        } else if( strcasecmp(attrValue,"multiply")==0 ) {
            nodeData->compMode = NXT_CompMode_Multiply;
            nodeData->fieldset |= NXT_StyleField_CompMode;
        } else if( strcasecmp(attrValue,"diff")==0 ) {
            nodeData->compMode = NXT_CompMode_Diff;
            nodeData->fieldset |= NXT_StyleField_CompMode;
        }
    } else if( strcasecmp(attrName, "rendertest")==0 ) {
        if( strcasecmp(attrValue,"none")==0 ) {
            nodeData->renderTest = NXT_RenderTest_None;
            nodeData->fieldset |= NXT_StyleField_RenderTest;
        } else if( strcasecmp(attrValue,"stencil")==0 ) {
            nodeData->renderTest = NXT_RenderTest_Stencil;
            nodeData->fieldset |= NXT_StyleField_RenderTest;
        } else if( strcasecmp(attrValue,"stencilinvert")==0 ) {
            nodeData->renderTest = NXT_RenderTest_NegStencil;
            nodeData->fieldset |= NXT_StyleField_RenderTest;
        } else if( strcasecmp(attrValue,"mask")==0 ) {
            nodeData->renderTest = NXT_RenderTest_Mask;
            nodeData->fieldset |= NXT_StyleField_RenderTest;
        } else if( strcasecmp(attrValue,"maskinvert")==0 ) {
            nodeData->renderTest = NXT_RenderTest_NegMask;
            nodeData->fieldset |= NXT_StyleField_RenderTest;
        } 
    } else {
        // TODO: Error        
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    //    NXT_Node_Style *nodeData = (NXT_Node_Style*)node;
    LOGD( "(Style)" );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Style *nodeData = (NXT_Node_Style*)node;
    LOGV("Traverse style node (0x%08x)", nodeData->fieldset);
    
    NXT_AnchorMode lightAnchor = NXT_AnchorMode_Camera;
    
    if( nodeData->fieldset & NXT_StyleField_AmbientColor ) {
        nodeData->saved_ambientColor = renderer->ambientColor;
        renderer->ambientColor = nodeData->ambientColor;
    }
    if( nodeData->fieldset & NXT_StyleField_DiffuseColor ) {
        nodeData->saved_diffuseColor = renderer->diffuseColor;
        renderer->diffuseColor = nodeData->diffuseColor;
    }
    if( nodeData->fieldset & NXT_StyleField_SpecularColor ) {
        nodeData->saved_specularColor = renderer->specularColor;
        renderer->specularColor = nodeData->specularColor;
    }
    if( nodeData->fieldset & NXT_StyleField_LightingNormals ) {
        nodeData->saved_lightingNormals = renderer->lightingNormals;
        renderer->lightingNormals = nodeData->lightingNormals;
    }
    if( nodeData->fieldset & NXT_StyleField_LightAnchor ) {
        lightAnchor = nodeData->lightAnchor;
    }
    if( nodeData->fieldset & NXT_StyleField_LightDirection ) {
        nodeData->saved_lightDirection = renderer->lightDirection;
        if( lightAnchor==NXT_AnchorMode_World ) {
            renderer->lightDirection = NXT_Matrix4f_MultVector(renderer->transform, nodeData->lightDirection);
        } else {
            renderer->lightDirection = nodeData->lightDirection;
        }
    }
    if( nodeData->fieldset & NXT_StyleField_ShadingMode ) {
        nodeData->saved_shadingMode = renderer->shadingMode;
        renderer->shadingMode = nodeData->shadingMode;
    }
    if( nodeData->fieldset & NXT_StyleField_SpecExp ) {
        nodeData->saved_specexp = renderer->specularExponent;
        renderer->specularExponent = nodeData->specexp;
    }
    
    
    
    if( nodeData->fieldset & NXT_StyleField_Color ) {
        nodeData->saved_color = renderer->color;
        renderer->color = nodeData->color;
    }
    if( nodeData->fieldset & NXT_StyleField_Alpha ) {
        nodeData->saved_alpha = renderer->alpha;
        renderer->alpha *= nodeData->alpha;
    }
    if( nodeData->fieldset & NXT_StyleField_Brightness ) {
        nodeData->saved_brightness = renderer->adj_brightness;
        renderer->adj_brightness = nodeData->brightness;
    }
    if( nodeData->fieldset & NXT_StyleField_Contrast ) {
        nodeData->saved_contrast = renderer->adj_contrast;
        renderer->adj_contrast = nodeData->contrast;
    }
    if( nodeData->fieldset & NXT_StyleField_Saturation ) {
        nodeData->saved_saturation = renderer->adj_saturation;
        renderer->adj_saturation = nodeData->saturation;
    }
    if( nodeData->fieldset & NXT_StyleField_Tint ) {
        nodeData->saved_tintColor = renderer->adj_tintColor;
        renderer->adj_tintColor = nodeData->tintColor;
    }
    if( nodeData->fieldset & NXT_StyleField_AnimFrame ) {
        nodeData->saved_animframe = renderer->animframe;
        renderer->animframe = nodeData->animframe;
    }
    if( nodeData->fieldset & NXT_StyleField_Antialias ) {
        nodeData->saved_antialias = renderer->bAntiAlias;
        renderer->bAntiAlias = nodeData->antialias;
		if( renderer->bAvailableMultisample ) {
			if( renderer->bAntiAlias ) {
				//glEnable(GL_SAMPLE_COVERAGE);
			} else {
				//glDisable(GL_SAMPLE_COVERAGE);
			}
		}
    }
    if( nodeData->fieldset & NXT_StyleField_DepthTest ) {
        nodeData->saved_depthtest = renderer->bDepthTest;
        renderer->bDepthTest = nodeData->depthtest;
		if( renderer->bDepthTest ) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
    }
    if( nodeData->fieldset & NXT_StyleField_DepthMask ) {
        nodeData->saved_depthmask = renderer->bDepthMask;
        renderer->bDepthMask = nodeData->depthmask;
		if( renderer->bDepthMask ) {
			glDepthMask(GL_TRUE);
		} else {
			glDepthMask(GL_FALSE);
		}
    }
    if( nodeData->fieldset & NXT_StyleField_CompMode ) {
        nodeData->saved_compMode = renderer->compMode;
        renderer->compMode = nodeData->compMode;
    }
    if( nodeData->fieldset & NXT_StyleField_RenderDest ) {
        nodeData->saved_renderDest = renderer->renderDest;
        renderer->renderDest = nodeData->renderDest;
    }
    if( nodeData->fieldset & NXT_StyleField_RenderTest ) {
        nodeData->saved_renderTest = renderer->renderTest;
        renderer->renderTest = nodeData->renderTest;
    }
    if( nodeData->fieldset & NXT_StyleField_MaskSample ) {
        nodeData->saved_mask_sample_matrix = renderer->mask_sample_transform;
        switch( nodeData->maskSampling ) {
            case NXT_MaskSampling_Invert:
                renderer->mask_sample_transform = NXT_Matrix4f_Identity();
                renderer->mask_sample_transform = NXT_Matrix4f_MultMatrix(renderer->mask_sample_transform, NXT_Matrix4f_Translate(NXT_Vector4f(1.0, 1.0, 1.0, 0.0)));
                renderer->mask_sample_transform = NXT_Matrix4f_MultMatrix(renderer->mask_sample_transform, NXT_Matrix4f_Scale(-1.0, -1.0, -1.0));
                break;
            case NXT_MaskSampling_Normal:
            default:
                renderer->mask_sample_transform = NXT_Matrix4f_Identity();
                break;
        }
    }
    if( (nodeData->fieldset & NXT_StyleField_RenderDest) || (nodeData->fieldset & NXT_StyleField_RenderTest) ) {
        unsigned int bNeedStencil = 0;
        switch (renderer->renderDest) {
                
            case NXT_RenderDest_Stencil:
                bNeedStencil = 1;
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFU);
                break;
            case NXT_RenderDest_Mask:
                NXT_ThemeRenderer_SetRenderToMask(renderer);
                break;
            case NXT_RenderDest_Screen:
                NXT_ThemeRenderer_SetRenderToDefault(renderer);
            default:
                glColorMask(renderer->colorMask[0], renderer->colorMask[1], renderer->colorMask[2], renderer->colorMask[3]);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFU);
                
                break;
        }
        
        switch (renderer->renderTest) {
            case NXT_RenderTest_Stencil:
                bNeedStencil = 1;
                glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFFU);
                break;
            case NXT_RenderTest_NegStencil:
                bNeedStencil = 1;
                glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFFU);
                break;
            case NXT_RenderTest_Mask:
                break;
            case NXT_RenderTest_NegMask:
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

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Style *nodeData = (NXT_Node_Style*)node;
    
    
    if( nodeData->fieldset & NXT_StyleField_AmbientColor ) {
        renderer->ambientColor = nodeData->saved_ambientColor;
    }
    if( nodeData->fieldset & NXT_StyleField_DiffuseColor ) {
        renderer->diffuseColor = nodeData->saved_diffuseColor;
    }
    if( nodeData->fieldset & NXT_StyleField_SpecularColor ) {
        renderer->specularColor = nodeData->saved_specularColor;
    }
    if( nodeData->fieldset & NXT_StyleField_LightDirection ) {
        renderer->lightDirection = nodeData->saved_lightDirection;
    }
    if( nodeData->fieldset & NXT_StyleField_ShadingMode ) {
        renderer->shadingMode = nodeData->saved_shadingMode;
    }
    if( nodeData->fieldset & NXT_StyleField_SpecExp ) {
        renderer->specularExponent = nodeData->saved_specexp;
    }
    if( nodeData->fieldset & NXT_StyleField_LightingNormals ) {
        renderer->lightingNormals = nodeData->saved_lightingNormals;
    }
    
    
    if( nodeData->fieldset & NXT_StyleField_Color ) {
        renderer->color = nodeData->saved_color;
    }
    if( nodeData->fieldset & NXT_StyleField_Alpha ) {
        renderer->alpha = nodeData->saved_alpha;
    }
    if( nodeData->fieldset & NXT_StyleField_Brightness ) {
        renderer->adj_brightness = nodeData->saved_brightness;
    }
    if( nodeData->fieldset & NXT_StyleField_Contrast ) {
        renderer->adj_contrast = nodeData->saved_contrast;
    }
    if( nodeData->fieldset & NXT_StyleField_Saturation ) {
        renderer->adj_saturation = nodeData->saved_saturation;
    }
    if( nodeData->fieldset & NXT_StyleField_Tint ) {
        renderer->adj_tintColor = nodeData->saved_tintColor;
    }
    if( nodeData->fieldset & NXT_StyleField_AnimFrame ) {
        renderer->animframe = nodeData->saved_animframe;
    }
	if( nodeData->fieldset & NXT_StyleField_Antialias ) {
        renderer->bAntiAlias = nodeData->saved_antialias;
		if( renderer->bAvailableMultisample ) {
			if( renderer->bAntiAlias ) {
				//glEnable(GL_SAMPLE_COVERAGE);
			} else {
				//glDisable(GL_SAMPLE_COVERAGE);
			}
		}
    }
    if( nodeData->fieldset & NXT_StyleField_DepthTest ) {
		renderer->bDepthTest = nodeData->saved_depthtest;
		if( renderer->bDepthTest ) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
    }
    if( nodeData->fieldset & NXT_StyleField_DepthMask ) {
        renderer->bDepthMask = nodeData->saved_depthmask;
		if( renderer->bDepthMask ) {
			glDepthMask(GL_TRUE);
		} else {
			glDisable(GL_FALSE);
		}
    }

    if( nodeData->fieldset & NXT_StyleField_RenderDest ) {
        renderer->renderDest = nodeData->saved_renderDest;
    }
    if( nodeData->fieldset & NXT_StyleField_RenderTest ) {
        renderer->renderTest = nodeData->saved_renderTest;
    }
    if( nodeData->fieldset & NXT_StyleField_CompMode ) {
        renderer->compMode = nodeData->saved_compMode;
    }
    if( nodeData->fieldset & NXT_StyleField_MaskSample ) {
        renderer->mask_sample_transform = nodeData->saved_mask_sample_matrix;
    }    
    if( (nodeData->fieldset & NXT_StyleField_RenderDest) || (nodeData->fieldset & NXT_StyleField_RenderTest) ) {
        unsigned int bNeedStencil = 0;
        switch (renderer->renderDest) {
                
            case NXT_RenderDest_Stencil:
                bNeedStencil = 1;
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFU);
                break;
                
            case NXT_RenderDest_Mask:
                NXT_ThemeRenderer_SetRenderToMask(renderer);
                break;
            case NXT_RenderDest_Screen:
                NXT_ThemeRenderer_SetRenderToDefault(renderer);
            default:
                glColorMask(renderer->colorMask[0], renderer->colorMask[1], renderer->colorMask[2], renderer->colorMask[3]);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFU);
                
                break;
        }
        
        switch (renderer->renderTest) {
            case NXT_RenderTest_Stencil:
                bNeedStencil = 1;
                glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFFU);
                break;
            case NXT_RenderTest_NegStencil:
                bNeedStencil = 1;
                glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFFU);
                break;
            case NXT_RenderTest_Mask:
                break;
            case NXT_RenderTest_NegMask:
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
    
    return NXT_NodeAction_Next;
}
