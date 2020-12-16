#include "NexTheme_Config.h"
#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"
#include "nxtRenderItem.h"
#include "NexTheme_RendererState.h"

#define LOG_TAG "NexTheme_Node_Rect"

#ifdef ANDROID
#include <android/log.h>
#define LOGFORCE(...)    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
extern "C" {
    void NSLog(CFStringRef format, ...);
}
#define LOGFORCE(format, ...)   NSLog(CFSTR(format), ##__VA_ARGS__)
#endif

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Rect = {
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
    "Rect",
    sizeof(NXT_Node_Rect)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {

    NXT_Node_Rect *nodeData = (NXT_Node_Rect*)node;
    NXT_FreeRenderitemKeyValues(&nodeData->renderitem_parameter_values_);
    if(nodeData->kedlstateblock){

        NXT_ThemeRenderer_FreeAppliedEffect(nodeData->kedlstateblock);
        delete nodeData->kedlstateblock;
        nodeData->kedlstateblock = NULL;
    }
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Rect *nodeData = (NXT_Node_Rect*)node;

    if(strcasecmp(attrName, "kedlitem") == 0){

        LOGXV("id: '%s'", attrValue);
        if( attrValue[0]==NXT_ELEMENT_REF_CHAR ) {

            nodeData->kedlitem = (NXT_Node_KEDL*)NXT_FindFirstNodeWithId(node, attrValue+1, &NXT_NodeClass_KEDL);
            if(NULL == nodeData->kedlstateblock){

                nodeData->kedlstateblock = new NXT_AppliedEffect();
            }

            NXT_ThemeRenderer_FreeAppliedEffect(nodeData->kedlstateblock);
        }
    }
    else if( strcasecmp(attrName, "renderitem")==0 ) {

        LOGXV("id: '%s'", attrValue);
        if( attrValue[0]==NXT_ELEMENT_REF_CHAR ) {

            nodeData->renderitem = (NXT_Node_RenderItem*)NXT_FindFirstNodeWithId( node, attrValue+1, &NXT_NodeClass_RenderItem );
        }
    }
    else if( strcasecmp(attrName, "ritmparam")==0 ) {

        NXT_ProcParamKeyValueFromString(node, attrValue, &nodeData->renderitem_parameter_values_);

    } else if( strcasecmp(attrName, "texture")==0 ) {
        LOGXV("Texture: '%s'", attrValue);
        if( attrValue[0]==NXT_ELEMENT_REF_CHAR ) {
            nodeData->texture = (NXT_Node_Texture*)NXT_FindFirstNodeWithId( node, attrValue+1, &NXT_NodeClass_Texture );
            LOGXV("Found texture=0x%p.", nodeData->texture);
            
        }
    } else if( strcasecmp(attrName, "mask")==0 ) {
        LOGXV("Mask: '%s'", attrValue);
        if( attrValue[0]==NXT_ELEMENT_REF_CHAR ) {
            nodeData->mask = (NXT_Node_Texture*)NXT_FindFirstNodeWithId( node, attrValue+1, &NXT_NodeClass_Texture );
            LOGD("Found mask=0x%p.", nodeData->mask);
            
        }
    } else if( strcasecmp(attrName, "location")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->location.e, sizeof(nodeData->location.e)/sizeof(*(nodeData->location.e)) );
        if( nodeData->anchor == NXT_Anchor_None ) {
            nodeData->anchor = NXT_Anchor_Center;
        }
    } else if( strcasecmp(attrName, "width")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->width, 1 );
    } else if( strcasecmp(attrName, "height")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->height, 1 );
    } else if( strcasecmp(attrName, "scalemode")==0 ) {
        if( strcasecmp(attrValue, "stretch")==0 ) {
            nodeData->scaleMode = NXT_ScaleMode_Stretch;
        } else if( strcasecmp(attrValue, "fit")==0 ) {
            nodeData->scaleMode = NXT_ScaleMode_Fit;
        } else if( strcasecmp(attrValue, "fill")==0 ) {
            nodeData->scaleMode = NXT_ScaleMode_Fill;
        } else if( strcasecmp(attrValue, "fitmask")==0 ) {
            nodeData->scaleMode = NXT_ScaleMode_FitMask;
        } else if( strcasecmp(attrValue, "fillmask")==0 ) {
            nodeData->scaleMode = NXT_ScaleMode_FillMask;
        }
    } else if( strcasecmp(attrName, "anchor")==0 ) {
        unsigned int bLeft = 0;
        unsigned int bRight = 0;
        unsigned int bTop = 0;
        unsigned int bBottom = 0;
        unsigned int bCenter = 0;
        if( strcasestr(attrValue, "left") )
            bLeft = 1;
        if( strcasestr(attrValue, "right") )
            bRight = 1;
        if( strcasestr(attrValue, "top") )
            bTop = 1;
        if( strcasestr(attrValue, "bottom") )
            bBottom = 1;
        if( strcasestr(attrValue, "center") )
            bCenter = 1;
        
        if( bLeft && bRight ) {
            bLeft=bRight=0;
            bCenter=1;
        }
        if( bTop && bBottom ) {
            bTop=bBottom=0;
            bCenter=1;
        }
        
        if( bTop && bLeft ) {
            nodeData->anchor = NXT_Anchor_TopLeft;
        } else if( bTop && bCenter ) {
            nodeData->anchor = NXT_Anchor_TopCenter;
        } else if( bTop && bRight ) {
            nodeData->anchor = NXT_Anchor_TopRight;
        } else if( bRight && bCenter ) {
            nodeData->anchor = NXT_Anchor_RightCenter;
        } else if( bBottom && bRight ) {
            nodeData->anchor = NXT_Anchor_BottomRight;
        } else if( bBottom && bCenter ) {
            nodeData->anchor = NXT_Anchor_BottomCenter;
        } else if( bBottom && bLeft ) {
            nodeData->anchor = NXT_Anchor_BottomLeft;
        } else if( bLeft && bCenter ) {
            nodeData->anchor = NXT_Anchor_LeftCenter;
        } else if( bCenter ) {
            nodeData->anchor = NXT_Anchor_Center;
        } else if( bTop ) {
            nodeData->anchor = NXT_Anchor_TopCenter;
        } else if( bBottom ) {
            nodeData->anchor = NXT_Anchor_BottomCenter;
        } else if( bLeft ) {
            nodeData->anchor = NXT_Anchor_LeftCenter;
        } else if( bRight ) {
            nodeData->anchor = NXT_Anchor_RightCenter;
        } else {
            nodeData->anchor = NXT_Anchor_Center;
        }
        
    } else if( strcasecmp(attrName, "color")==0 ) {
        LOGXV("Color: '%s'", attrValue);
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->color.e, sizeof(nodeData->color.e)/sizeof(*(nodeData->color.e)) );
        nodeData->bColorSet = 1;
    } else if( strcasecmp(attrName, "bounds")==0 ) {
        LOGXV("Bounds: '%s'", attrValue);
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->bounds.e, sizeof(nodeData->bounds.e)/sizeof(*(nodeData->bounds.e)) );
    } else {
        // TODO: Error        
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    //    NXT_Node_Rect *nodeData = (NXT_Node_Rect*)node;
    LOGD( "(Rect)" );
}

static void setParameterKeyValue(NXT_HThemeRenderer renderer, NXT_RenderItem_KeyValues* pkeyvalues){

    NXT_Theme_ResetDefaultValuemap(renderer->getRenderItemManager());

    NXT_RenderItem_KeyValues::list_t& valuelist = pkeyvalues->items_;

    for(NXT_RenderItem_KeyValues::iter_t itor = valuelist.begin(); itor != valuelist.end(); ++itor){


        NXT_RenderItem_KeyValue& item = **itor;
        float* pvalue = &item.value_.front();
        char value_str[512];
        switch(item.type_){

            case NXT_Renderitem_KeyValueType_Color:
                snprintf(value_str, sizeof(value_str), "{x=0x%x,y=0x%x,z=0x%x,w=0x%x}", (int)pvalue[0], (int)pvalue[1], (int)pvalue[2], (int)pvalue[3]);
                break;
            case NXT_Renderitem_KeyValueType_Selection:
                snprintf(value_str, sizeof(value_str), "{x=%f,y=%f,z=%f,w=%f}", pvalue[0], pvalue[1], pvalue[2], pvalue[3]);
                break;
            case NXT_Renderitem_KeyValueType_Choice:
                snprintf(value_str, sizeof(value_str), "{x=%f,y=%f,z=%f,w=%f}", pvalue[0], pvalue[1], pvalue[2], pvalue[3]);
                break;
            case NXT_Renderitem_KeyValueType_Range:
                snprintf(value_str, sizeof(value_str), "%f", pvalue[0]);
                break;
            case NXT_Renderitem_KeyValueType_Rect:
                snprintf(value_str, sizeof(value_str), "{x0=%f,y0=%f,x1=%f,y1=%f}", pvalue[0], pvalue[1], pvalue[2], pvalue[3]);
                break;
            case NXT_Renderitem_KeyValueType_Point:
                snprintf(value_str, sizeof(value_str), "{x=%f,y=%f,z=%f}", pvalue[0], pvalue[1], pvalue[2]);
                break;
        };

        NXT_Theme_SetValue(renderer->getRenderItemManager(), item.key_.c_str(), value_str);
    }

    NXT_Theme_ApplyDefaultValues(renderer->getRenderItemManager());
}

void RendererState::operator <<(NXT_HThemeRenderer renderer){

    color = renderer->color;
    basetransform = renderer->basetransform;
    transform = renderer->transform;
    proj = renderer->proj;
    texture_transform = renderer->texture_transform;
    mask_transform = renderer->mask_transform;
    mask_sample_transform = renderer->mask_sample_transform;
    alpha = renderer->alpha;
    animframe = renderer->animframe;
    texture = renderer->texture;
    mask = renderer->mask;
    renderDest = renderer->renderDest;
    renderTest = renderer->renderTest;
    bAntiAlias = renderer->bAntiAlias;
    bDepthTest = renderer->bDepthTest;
    bDepthMask = renderer->bDepthMask;
    compMode = renderer->compMode;
    seed = renderer->seed;
    cur_effect_overlap = renderer->cur_effect_overlap;
    pActiveEffect = renderer->pActiveEffect;
    effectType = renderer->effectType;
    clip_index = renderer->clip_index;
    clip_count = renderer->clip_count;    
    cur_time = renderer->cur_time;
    max_set_time = renderer->max_set_time;
    max_time = renderer->max_time;
    bSkipChildren = renderer->bSkipChildren;
    inTime = renderer->inTime;
    outTime = renderer->outTime;
    cycleTime = renderer->cycleTime;
    repeatType = renderer->repeatType;
    part = renderer->part;    
    settime = renderer->settime;
    time = renderer->time;
    view_width = renderer->view_width;
    view_height = renderer->view_height;
    cullFace = renderer->cullFace;
    effectStartTime = renderer->effectStartTime;
    effectEndTime = renderer->effectEndTime;
    adj_brightness = renderer->adj_brightness;
    adj_contrast = renderer->adj_contrast;
    adj_saturation = renderer->adj_saturation;
    adj_tintColor = renderer->adj_tintColor;
    ambientColor = renderer->ambientColor;
    diffuseColor = renderer->diffuseColor;
    specularColor = renderer->specularColor;
    lightDirection = renderer->lightDirection;
    specularExponent = renderer->specularExponent;
    shadingMode = renderer->shadingMode;
    lightingNormals = renderer->lightingNormals;
}

void RendererState::operator >>(NXT_HThemeRenderer renderer){

    renderer->color = color;
    renderer->basetransform = basetransform;
    renderer->transform = transform;
    renderer->proj = proj;
    renderer->texture_transform = texture_transform;
    renderer->mask_transform = mask_transform;
    renderer->mask_sample_transform = mask_sample_transform;
    renderer->alpha = alpha;
    renderer->animframe = animframe;
    renderer->texture = texture;
    renderer->mask = mask;
    renderer->renderDest = renderDest;
    renderer->renderTest = renderTest;
    renderer->bAntiAlias = bAntiAlias;
    renderer->bDepthTest = bDepthTest;
    renderer->bDepthMask = bDepthMask;
    renderer->compMode = compMode;
    renderer->seed = seed;
    renderer->cur_effect_overlap = cur_effect_overlap;
    renderer->pActiveEffect = pActiveEffect;
    renderer->effectType = effectType;
    renderer->clip_index = clip_index;
    renderer->clip_count = clip_count;    
    renderer->cur_time = cur_time;
    renderer->max_set_time = max_set_time;
    renderer->max_time = max_time;
    renderer->bSkipChildren = bSkipChildren;
    renderer->inTime = inTime;
    renderer->outTime = outTime;
    renderer->cycleTime = cycleTime;
    renderer->repeatType = repeatType;
    renderer->part = part;    
    renderer->settime = settime;
    renderer->time = time;
    renderer->view_width = view_width;
    renderer->view_height = view_height;
    renderer->cullFace = cullFace;
    renderer->effectStartTime = effectStartTime;
    renderer->effectEndTime = effectEndTime;
    renderer->adj_brightness = adj_brightness;
    renderer->adj_contrast = adj_contrast;
    renderer->adj_saturation = adj_saturation;
    renderer->adj_tintColor = adj_tintColor;
    renderer->ambientColor = ambientColor;
    renderer->diffuseColor = diffuseColor;
    renderer->specularColor = specularColor;
    renderer->lightDirection = lightDirection;
    renderer->specularExponent = specularExponent;
    renderer->shadingMode = shadingMode;
    renderer->lightingNormals = lightingNormals;
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
#ifdef NEXTHEME_SUPPORT_OPENGL

    //int i;
    
    NXT_Node_Rect *nodeData = (NXT_Node_Rect*)node;
    
    if(nodeData->renderitem || nodeData->kedlitem){
        
        if(renderer->alpha <= 0.0f)
            return;
    }
    
    GLfloat left = nodeData->bounds.e[0];
    GLfloat bottom = nodeData->bounds.e[1];
    GLfloat right = nodeData->bounds.e[2];
    GLfloat top = nodeData->bounds.e[3];
    
    NXT_TextureInfo *texture = NULL;
    NXT_TextureInfo *mask = NULL;
    NXT_Vector4f savedColor;
    if( nodeData->bColorSet ) {
        savedColor = renderer->color;
        renderer->color = nodeData->color;
    }

    NXT_Matrix4f texmat, colormat;

    int backup_required = 0;
    NXT_TextureInfo* backup_texinfo = NULL;

    int first_processed = 0;

    if(nodeData->kedlitem){

        if(nodeData->texture && nodeData->texture->textureType==NXT_TextureType_Video && (!renderer->videoPlaceholder1 || !renderer->videoPlaceholder2)){

        }
        else if(nodeData->texture){

            backup_required = 1;
            backup_texinfo = &renderer->getVideoSrc(NXT_TextureID_Video_1);
            nodeData->texture->texinfo_preview.bValidTexture = 1;
            renderer->setVideoSrc(NXT_TextureID_Video_1, &nodeData->texture->texinfo_preview);
        }
    }

    if(nodeData->renderitem || nodeData->kedlitem){

        NexThemeRenderer_PrepStateForRender2(renderer,nodeData->texture,nodeData->mask,
            nodeData->texture?&texture:NULL,
            &mask, &texmat, &colormat, 0);
    }
    else{

        first_processed = NexThemeRenderer_PrepStateForRender(renderer,nodeData->texture,nodeData->mask,&texture,&mask, 1);        
    }
    
    if( nodeData->bColorSet ) {
        renderer->color = savedColor;
    }

    if( nodeData->anchor ) {
        
        float twidth, theight, width, height;
        float x = nodeData->location.e[0];
        float y = nodeData->location.e[1];
        
        if( (nodeData->scaleMode==NXT_ScaleMode_FitMask || nodeData->scaleMode==NXT_ScaleMode_FillMask) && mask ) {
            twidth = (float)mask->srcWidth;
            theight = (float)mask->srcHeight;
            LOGV("[%s %d] Mask size: %f %f", __func__, __LINE__, twidth, theight);
        } else if( texture ) {
            twidth = (float)texture->srcWidth;
            theight = (float)texture->srcHeight;
            LOGV("[%s %d] Texture size: %f %f", __func__, __LINE__, twidth, theight);
        } else if( mask ) {
            twidth = (float)mask->srcWidth;
            theight = (float)mask->srcHeight;
            LOGV("[%s %d] Mask size: %f %f", __func__, __LINE__, twidth, theight);
        } else {
            twidth = 10.0;
            theight = 10.0;
            LOGV("[%s %d] Texture size/def: %f %f", __func__, __LINE__, twidth, theight);
        }
        
        if( nodeData->width==0.0 && nodeData->height==0.0 ) {
            if( twidth > theight ) {
                width = 1.0;
                height = (theight/twidth);
                LOGV("[%s %d] Calculated size(w>h): %f %f", __func__, __LINE__, width, height);
            } else {
                width = (twidth/theight);
                height = 1.0;
                LOGV("[%s %d] Calculated size(w<h): %f %f", __func__, __LINE__, width, height);
            }
        } else if(nodeData->width==0.0 ) {
            width = (twidth/theight)*nodeData->height;
            height = nodeData->height;
            LOGV("[%s %d] Calculated size(w0): %f %f", __func__, __LINE__, width, height);
        } else if(nodeData->height==0.0 ) {
            width = nodeData->width;
            height = (theight/twidth)*nodeData->width;
            LOGV("[%s %d] Calculated size(h0): %f %f", __func__, __LINE__, width, height);
        } else {
            switch( nodeData->scaleMode ) {
                case NXT_ScaleMode_Fit:
                case NXT_ScaleMode_FitMask:
                {
                    float fitWidth = (twidth/theight)*nodeData->height;
                    float fitHeight = (theight/twidth)*nodeData->width;
                    if( fitWidth/nodeData->width < fitHeight/nodeData->height ) {
                        width = fitWidth;
                        height = nodeData->height;
                    } else {
                        height = fitHeight;
                        width = nodeData->width;
                    }
                    break;
                }
                case NXT_ScaleMode_Fill:
                case NXT_ScaleMode_FillMask:
                {
                    float fitWidth = (twidth/theight)*nodeData->height;
                    float fitHeight = (theight/twidth)*nodeData->width;
                    if( fitWidth/nodeData->width > fitHeight/nodeData->height ) {
                        width = fitWidth;
                        height = nodeData->height;
                    } else {
                        height = fitHeight;
                        width = nodeData->width;
                    }
                    break;
                }
                case NXT_ScaleMode_Stretch:
                default:
                    width = nodeData->width;
                    height = nodeData->height;
                    break;
            }
            LOGV("[%s %d] Calculated size(wh): %f %f", __func__, __LINE__, width, height);
        }
        
        switch( nodeData->anchor ) {
            case NXT_Anchor_TopLeft:
                left = x;
                top = y;
                right = left+width;
                bottom = top-height;
                LOGV("[%s %d] Calculated coords/TL: %f %f %f %f", __func__, __LINE__, left, bottom, right, top);
                break;
            case NXT_Anchor_TopRight:
                right = x;
                top = y;
                left = right-width;
                bottom = top-height;
                LOGV("[%s %d] Calculated coords/TR: %f %f %f %f", __func__, __LINE__, left, bottom, right, top);
                break;
            case NXT_Anchor_TopCenter:
                left = x-(width/2.0);
                top = y;
                right = left+width;
                bottom = top-height;
                LOGV("[%s %d] Calculated coords/TC: %f %f %f %f", __func__, __LINE__, left, bottom, right, top);
                break;
            case NXT_Anchor_BottomLeft:
                left = x;
                bottom = y;
                right = left+width;
                top = bottom+height;
                LOGV("[%s %d] Calculated coords/BL: %f %f %f %f", __func__, __LINE__, left, bottom, right, top);
                break;
            case NXT_Anchor_BottomRight:
                right = x;
                bottom = y;
                left = right-width;
                top = bottom+height;
                LOGV("[%s %d] Calculated coords/BR: %f %f %f %f", __func__, __LINE__, left, bottom, right, top);
                break;
            case NXT_Anchor_BottomCenter:
                left = x-(width/2.0);
                bottom = y;
                right = left+width;
                top = bottom+height;
                LOGV("[%s %d] Calculated coords/BC: %f %f %f %f", __func__, __LINE__, left, bottom, right, top);
                break;
            case NXT_Anchor_LeftCenter:
                left = x;
                top = y+(height/2.0);
                right = left+width;
                bottom = top-height;
                LOGV("[%s %d] Calculated coords/LC: %f %f %f %f", __func__, __LINE__, left, bottom, right, top);
                break;
            case NXT_Anchor_RightCenter:
                right = x;
                top = y+(height/2.0);
                left = right-width;
                bottom = top-height;
                LOGV("[%s %d] Calculated coords/RC: %f %f %f %f", __func__, __LINE__, left, bottom, right, top);
                break;
            case NXT_Anchor_Center:
                left = x-(width/2.0);
                top = y+(height/2.0);
                right = left+width;
                bottom = top-height;
                LOGV("[%s %d] Calculated coords/C: %f %f %f %f", __func__, __LINE__, left, bottom, right, top);
                break;
            default:
                left = x-(width/2.0);
                top = y+(height/2.0);
                right = left+width;
                bottom = top-height;
                LOGV("[%s %d] Calculated coords/D: %f %f %f %f", __func__, __LINE__, left, bottom, right, top);
                break;
        }
        
        float swap = top;
        top = bottom;
        bottom = swap;
        
    }
    
    GLfloat vertexCoord[] = {
        left,   bottom,
        right,  bottom,
        left,   top,   
        right,  top
    };
    
    //LOGV("Rendering coords: %f %f %f %f", left, bottom, right, top);

    if(nodeData->kedlitem){

        RendererState state;
        state << renderer;
        NXT_Matrix4f mvp = NXT_Matrix4f_MultMatrix(renderer->proj, renderer->transform);
        renderer->basetransform = NXT_Matrix4f_MultMatrix(renderer->basetransform, mvp);
        renderer->proj = NXT_Matrix4f_Identity();
        renderer->transform = NXT_Matrix4f_Identity();
        renderer->pActiveEffect = nodeData->kedlstateblock;

        NXT_ThemeRenderer_SetAppliedEffect(renderer,
                             renderer->pActiveEffect,
                             NXT_EffectType_Title,
                             nodeData->kedlitem->uid,  // ID of clip effect, or NULL to clear current effect
                             NULL,
                             0,         // Index of this clip, from 0
                             1,    // Total number of clips in the project
                             renderer->actualEffectStartCTS,     // Start time for this clip
                             renderer->actualEffectEndCTS,       // End time for this clip
                             renderer->actualEffectStartCTS,   // Start time for this effect (must be >= clipStartTime)
                             renderer->actualEffectEndCTS);

    //Do something to render kedl item
        NXT_ThemeRenderer_TimeClipEffect(renderer, nodeData->kedlitem->effect, renderer->currentTime, renderer->actualEffectStartCTS, renderer->actualEffectEndCTS);
        NXT_ThemeRenderer_RenderEffect(renderer, nodeData->kedlitem->effect, renderer->currentTime);
        
        state >> renderer;
        if(backup_required){

            renderer->setVideoSrc(NXT_TextureID_Video_1, backup_texinfo);
        }
        return;
    }

    if( nodeData->renderitem){

        if(nodeData->renderitem->id <0)
            return;

        NXT_Vector4f color;
        if( nodeData->bColorSet ) {
            color = nodeData->color;
        } else {
            color = renderer->color;
        }

        glClear(GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);

        NXT_Theme_BeginRenderItem(renderer->getRenderItemManager(), nodeData->renderitem->id, renderer->view_width, renderer->view_height, renderer->pmain_render_target_);
        NXT_Matrix4f mvp = NXT_Matrix4f_MultMatrix(renderer->basetransform, NXT_Matrix4f_MultMatrix(renderer->proj, renderer->transform));

        if(texture){

            if(texture->textureFormat != NXT_PixelFormat_RGBA8888)
               NXT_ThemeRenderer_ConvertTextureToRGB(renderer, texture, 0, 0, 0, 0, NULL, 0, 10000.0);

            NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                        texture->texName_for_rgb, 
                                                        texture->textureWidth, 
                                                        texture->textureHeight, 
                                                        texture->srcWidth,
                                                        texture->srcHeight,
                                                        E_TARGET_VIDEO_SRC);

            NXT_Theme_SetTextureInfoTargetRenderItem(renderer->getRenderItemManager(), 
                                                        texture->texName_for_rgb, 
                                                        texture->textureWidth, 
                                                        texture->textureHeight, 
                                                        texture->srcWidth,
                                                        texture->srcHeight,
                                                        E_TARGET_VIDEO_LEFT);

            NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat, E_TARGET_VIDEO_SRC);
            NXT_Theme_SetTexMatrix(renderer->getRenderItemManager(), &texmat, E_TARGET_VIDEO_LEFT);
            NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), &colormat, E_TARGET_VIDEO_SRC);
            NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC, texture);
            NXT_Theme_SetColorconvMatrix(renderer->getRenderItemManager(), &colormat, E_TARGET_VIDEO_LEFT);
            NXT_Theme_SetRealXYForRenderItem(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT, texture);

        }
        else{

            NXT_Theme_SaveSrcInfo(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC);
            NXT_Theme_SaveSrcInfo(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT);
            NXT_Theme_ClearSrcInfoTarget(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC);
            NXT_Theme_ClearSrcInfoTarget(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT);
        }
        setParameterKeyValue(renderer, &nodeData->renderitem_parameter_values_);
        NXT_Theme_SetRect(renderer->getRenderItemManager(), left, top, right, bottom, renderer->alpha, renderer->renderTest == NXT_RenderTest_Mask);
        NXT_Theme_SetMatrix(renderer->getRenderItemManager(), mvp.e);
        NXT_Theme_ApplyRenderItem(renderer->getRenderItemManager(), renderer->time);
        NXT_Theme_ResetMatrix(renderer->getRenderItemManager());
        NXT_Theme_EndRenderItem(renderer->getRenderItemManager());

        if(NULL == texture){

            NXT_Theme_RestoreSrcInfo(renderer->getRenderItemManager(), E_TARGET_VIDEO_SRC);
            NXT_Theme_RestoreSrcInfo(renderer->getRenderItemManager(), E_TARGET_VIDEO_LEFT);
        }

        return;

    } else if( mask && texture ) {
        
        LOGV("Textured and masked rendering: src=%dx%d tex=%dx%d",
             texture->srcWidth, texture->srcHeight, 
             texture->textureWidth, texture->textureHeight);
        
        float lowx = NexThemeRenderer_AdjTexCoordX(texture,0);
        float lowy = NexThemeRenderer_AdjTexCoordY(texture,0);
        float highx = NexThemeRenderer_AdjTexCoordX(texture,1);
        float highy = NexThemeRenderer_AdjTexCoordY(texture,1);
        
        float mxscale = (float)(mask->srcWidth-1)/(float)mask->textureWidth;
        float myscale = (float)(mask->srcHeight-1)/(float)mask->textureHeight;
        
        float mlowx = 0;
        float mlowy = 0;
        float mhighx = 1.0*mxscale;
        float mhighy = 1.0*myscale;
        
        //LOGV("(Texture+Mask Rect) Textured rendering: texcoords=%f %f %f %f", lowx, lowy, highx, highy );
        //LOGV("(Texture+Mask Rect) Textured rendering: maskcoords=%f %f %f %f", mlowx, lowy, highx, highy );
        
        GLfloat texCoords[] = {
            lowx,       lowy,
            highx,      lowy,
            lowx,       highy,
            highx,      highy
        };
        
        GLfloat maskCoords[] = {
            mlowx,      mlowy,
            mhighx,     mlowy,
            mlowx,      mhighy,
            mhighx,     mhighy
        };
        
        glVertexAttribPointer(renderer->pMaskedShaderActive->a_position,2,GL_FLOAT,0,0,vertexCoord);
        CHECK_GL_ERROR();
        
        glVertexAttribPointer(renderer->pMaskedShaderActive->a_texCoord,2,GL_FLOAT,0,0,texCoords);
        CHECK_GL_ERROR();
        
        glVertexAttribPointer(renderer->pMaskedShaderActive->a_maskCoord,2,GL_FLOAT,0,0,maskCoords);
        CHECK_GL_ERROR();
        
        
    } else if( mask ) {
        
        //LOGV("Textured and masked rendering");
        
        float mxscale = (float)(mask->srcWidth-1)/(float)mask->textureWidth;
        float myscale = (float)(mask->srcHeight-1)/(float)mask->textureHeight;
        
        float mlowx = 0;
        float mlowy = 0;
        float mhighx = 1.0*mxscale;
        float mhighy = 1.0*myscale;
                
        GLfloat texCoords[] = {
            0,      0,
            0,      0,
            0,      0,
            0,      0
        };
        
        GLfloat maskCoords[] = {
            mlowx,      mlowy,
            mhighx,     mlowy,
            mlowx,      mhighy,
            mhighx,     mhighy
        };
        
        glVertexAttribPointer(renderer->pMaskedShaderActive->a_position,2,GL_FLOAT,0,0,vertexCoord);
        CHECK_GL_ERROR();
        
        glVertexAttribPointer(renderer->pMaskedShaderActive->a_texCoord,2,GL_FLOAT,0,0,texCoords);
        CHECK_GL_ERROR();
        
        glVertexAttribPointer(renderer->pMaskedShaderActive->a_maskCoord,2,GL_FLOAT,0,0,maskCoords);
        CHECK_GL_ERROR();
        
        
    } else if( texture ) {
        LOGV("Textured rendering: src=%dx%d tex=%dx%d",
             texture->srcWidth, texture->srcHeight, 
             texture->textureWidth, texture->textureHeight);
        
        float lowx = NexThemeRenderer_AdjTexCoordX(texture,0);
        float lowy = NexThemeRenderer_AdjTexCoordY(texture,0);
        float highx = NexThemeRenderer_AdjTexCoordX(texture,1);
        float highy = NexThemeRenderer_AdjTexCoordY(texture,1);
        
        //LOGV("Textured rendering: texcoords=%f %f %f %f", lowx, lowy, highx, highy );
        
        GLfloat texCoords[] = {
            lowx,       lowy,
            highx,      lowy,
            lowx,       highy,
            highx,      highy
        };
        
        glVertexAttribPointer(renderer->pTexturedShaderActive->a_position,2,GL_FLOAT,0,0,vertexCoord);
        CHECK_GL_ERROR();
        
        glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord,2,GL_FLOAT,0,0,texCoords);
        CHECK_GL_ERROR();
        
    } else {
        //LOGV("Solid rendering");
             
        glVertexAttribPointer(renderer->pSolidShaderActive->a_position,2,GL_FLOAT,0,0,vertexCoord);
        CHECK_GL_ERROR();
        
        NXT_Vector4f color;
        if( nodeData->bColorSet ) {
            color = nodeData->color;
        } else {
            color = renderer->color;
        }
        
        GLfloat vertexColor[16];
        memcpy(vertexColor+0, color.e, sizeof(GLfloat)*4);
        memcpy(vertexColor+4, color.e, sizeof(GLfloat)*4);
        memcpy(vertexColor+8, color.e, sizeof(GLfloat)*4);
        memcpy(vertexColor+12, color.e, sizeof(GLfloat)*4);
        
        glVertexAttribPointer(renderer->pSolidShaderActive->a_color,4,GL_FLOAT,0,0,vertexColor );
        CHECK_GL_ERROR();
    }
    
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    CHECK_GL_ERROR();
    
    //LOGV("glDrawArrays done");
    
#endif //NEXTHEME_SUPPORT_OPENGL
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
//    NXT_Node_Rect *nodeData = (NXT_Node_Rect*)node;
    return NXT_NodeAction_Next;
}
