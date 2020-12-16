#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"
#include "nxtRenderItem.h"
#include "NexTheme_RendererState.h"
#include "nexTexturesLogger.h"

#define LOG_TAG "NexTheme_Node_Texture"

#ifdef ANDROID
#include <android/log.h>
#define LOGFORCE(...)    0
//__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#elif defined(__APPLE__)
#define LOGFORCE(...)    0
#endif

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeTextureFunc(NXT_HThemeRenderer_Context context, NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static int nodePrecacheFunc(NXT_NodeHeader *node, NXT_HThemeRenderer renderer, NXT_PrecacheMode precacheMode, int* asyncmode, int* max_replaceable);
static void premultAlpha(unsigned int *pixels, unsigned int numPixels);
static void uploadTexture(NXT_Node_Texture *nodeData, NXT_HThemeRenderer renderer, const char* videoPlaceHolder);
static void uploadTexture(NXT_Node_Texture *nodeData, NXT_HThemeRenderer renderer, const char* videoPlaceHolder, const char* key);
static void updateTexture(NXT_Node_Texture *nodeData, NXT_HThemeRenderer renderer, NXT_TextureInfo* psrc);
static NXT_TextureInfo * validateContextAndGetTexInfo( NXT_HThemeRenderer renderer, NXT_Node_Texture *nodeData);
static int nodeGetPrecacheResource(NXT_NodeHeader* node, NXT_HThemeRenderer renderer, void* load_resource_list);

//#define ALTERNATE_PREVIEW_RESOLUTION
struct nxStringValue:ValueMapObj{

    nxStringValue(std::string value):obj_(value){

    }

    virtual ~nxStringValue(){

    }

    std::string obj_;
};

NXT_NodeClass NXT_NodeClass_Texture = {
    nodeInitFunc,
    nodeFreeFunc,
    nodeSetAttrFunc,
    nodeChildrenDoneFunc,
    nodeDebugPrintFunc,
    nodeBeginRender,
    nodeEndRender,
    nodePrecacheFunc,//precacheFunc
    nodeGetPrecacheResource,//getPrecacheResourceFunc
    NULL,//nodeUpdateNotify
    nodeFreeTextureFunc,//freeTextureFunc
    "Texture",
    sizeof(NXT_Node_Texture)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_Texture *nodeData = (NXT_Node_Texture*)node;
    nodeData->bMipmap = 0;
    nodeData->textStyle.spacingMult = 1;
    nodeData->textStyle.textFlags = NXT_TEXTFLAG_AUTOSIZE;
    nodeData->texinfo_preview.texWrapS = NXT_TextureWrap_None;
    nodeData->texinfo_preview.texWrapT = NXT_TextureWrap_None;
    nodeData->textStyle.shadowblurType = NXT_BLUR_SOLID;
}

static void nodeFreeTextureFunc(NXT_HThemeRenderer_Context context, NXT_NodeHeader *node)
{
    LOGI("[NodeTexture %d]nodeFreeTextureFunc",  __LINE__ );
    NXT_Node_Texture *nodeData = (NXT_Node_Texture*)node;
    NXT_TextureInfo* texinfo = &nodeData->texinfo_preview;

    NXT_FreeRenderitemKeyValues(&nodeData->renderitem_parameter_values_);
    NXT_FreeRenderitemKeyValues(&nodeData->renderitem_parameter_pre_values_);
    RenderTargetManager* pmanager = (RenderTargetManager*)context->getRendertargetManager();
    pmanager->releaseRenderTarget((NXT_RenderTarget*)nodeData->prender_target_);
    nodeData->prender_target_ = NULL;
    // Allocate OpenGL texture name
    if( texinfo->texNameInitCount > 0 ) 
    {
        texinfo->texNameInitCount = 0;
        if(nodeData->textureType == NXT_TextureType_Video)
            return;
        if(nodeData->path && texinfo->bValidTexture){

            if(nodeData->textureType==NXT_TextureType_File || nodeData->textureType==NXT_TextureType_Text)
                return;
        }
        GL_DeleteTextures( texinfo->texNameInitCount, &texinfo->texName[0] );
    }
    return;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    NXT_Node_Texture *nodeData = (NXT_Node_Texture*)node;
    if( nodeData->path ) {
        free( nodeData->path );
        nodeData->path = NULL;
    }
    if(nodeData->key){

        free(nodeData->key);
        nodeData->key = NULL;
    }
    if( nodeData->text ) {
        free(nodeData->text);
        nodeData->text = NULL;
    }
    if( nodeData->srcfld ) {
        free(nodeData->srcfld);
        nodeData->srcfld = NULL;
    }
    NXT_TextStyle_Free(&nodeData->textStyle);
    
    if( nodeData->bLoadedImage ) {
        if( nodeData->bCachedImage ) {
            if(nodeData->imgInfo.pixels) {
                free(nodeData->imgInfo.pixels);
                nodeData->imgInfo.pixels = NULL;
            }
            nodeData->bCachedImage = 0;
        } else if ( nodeData->imgInfo.freeImageCallback ) {
            nodeData->imgInfo.freeImageCallback(&nodeData->imgInfo, nodeData->imageCallbackPvtData);
            nodeData->imgInfo.freeImageCallback = NULL;
            nodeData->imageCallbackPvtData = NULL;
        }
        nodeData->bLoadedImage = 0;
    }

    if(nodeData->kedlstateblock){

        NXT_ThemeRenderer_FreeAppliedEffect(nodeData->kedlstateblock);
        delete nodeData->kedlstateblock;
        nodeData->kedlstateblock = NULL;
    }
}

#include <string>

static std::string convertEscape(const char* val){

    const char* escape[] = {

        "&amp;",
        "&lt;",
        "&gt;",
        "&quot;",
        "&apos;"
    };

    const char* replace[] = {

        "&",
        "<",
        ">",
        "\"",
        "'"
    };

    std::string str(val);

    int findpos = -1;

    for(int i = 0; i < 5; ++i){

        while((findpos = str.find(escape[i])) > -1){

            str.replace(findpos, strlen(escape[i]), replace[i]);
        }
    }

    return str;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {

    NXT_Node_Texture *nodeData = (NXT_Node_Texture*)node;

    if(strcasecmp(attrName, "kedlitem") == 0){

        LOGXV("id: '%s'", attrValue);
        if( attrValue[0]==NXT_ELEMENT_REF_CHAR ) {

            nodeData->kedlitem = (NXT_Node_KEDL*)NXT_FindFirstNodeWithId(node, attrValue+1, &NXT_NodeClass_KEDL);
            if(NULL == nodeData->kedlstateblock){

                nodeData->kedlstateblock = new NXT_AppliedEffect();
                nodeData->kedlstateblock->textSerial = 1;
            }

            NXT_ThemeRenderer_FreeAppliedEffect((NXT_AppliedEffect*)nodeData->kedlstateblock);
            nodeData->check_for_src_update_ = 1;
        }
    }
    else if(strcasecmp(attrName, "mipmap") ==0) {

        if( strcasecmp(attrValue,"true")==0 ) {
            
            nodeData->bMipmap = 1;
        } else if( strcasecmp(attrValue,"false")==0 ) {
            
            nodeData->bMipmap = 0;
        }
    }
    else if(strcasecmp(attrName, "renderitem") == 0){

        if( attrValue[0]==NXT_ELEMENT_REF_CHAR ) {
            nodeData->renderitem = (NXT_Node_RenderItem*)NXT_FindFirstNodeWithId( node, attrValue+1, &NXT_NodeClass_RenderItem );
        }
    }
    else if(strcasecmp(attrName, "ritmparam")== 0){

        NXT_ProcParamKeyValueFromString(node, attrValue, &nodeData->renderitem_parameter_values_);
    }
    else if( strcasecmp(attrName, "src")==0 ) {
        if( nodeData->path ) {
            free( nodeData->path );
        }
        
        const char *base_id = "";
        NXT_NodeHeader *pn;
        
        for( pn = node; pn; pn = pn->parent ) {
            if( pn->isa==&NXT_NodeClass_Effect || pn->isa==&NXT_NodeClass_Theme ) {
                base_id = pn->node_id;
            }
        }
        
        const char *tag = "[ThemeImage]";
        int attrlen = (int)strlen(attrValue);
        int taglen = (int)strlen(tag);
        int baseidlen = (int)strlen(base_id);
        nodeData->path = (char*)malloc(taglen + attrlen + baseidlen + 2);
        strcpy(nodeData->path, tag);
        strcpy(nodeData->path + taglen, base_id);
        strcpy(nodeData->path + taglen + baseidlen, "/");
        strcpy(nodeData->path + taglen + baseidlen + 1, attrValue);
        nodeData->textureType = NXT_TextureType_File;
    } else if( strcasecmp(attrName, "video")==0 ) {
        if( strcasecmp(attrValue,"1")==0 ) {
            nodeData->videoSource = 1;
            nodeData->textureType = NXT_TextureType_Video;
        } else if( strcasecmp(attrValue,"2")==0 ) {
            nodeData->videoSource = 2;
            nodeData->textureType = NXT_TextureType_Video;
        }
    } else if( strcasecmp(attrName, "srcfield")==0 ) {
        if( nodeData->srcfld ) {
            free( nodeData->srcfld );
        }
        if( *attrValue=='@' )
            attrValue++;
        nodeData->srcfld = (char*)malloc(strlen(attrValue)+1);
        strcpy(nodeData->srcfld, attrValue);
        if(nodeData->textureType == NXT_TextureType_None) {
            nodeData->textureType = NXT_TextureType_Overlay;
        }
        
    } else if( strcasecmp(attrName, "text")==0 ) {
        static const char *hexchars = "012345678ABCDEF";
        static const char *needEsc = ";=%'\"";
        const char *s;
        const char *e;
        char *t;
        if( nodeData->text ) {
            free( nodeData->text );
        }
        int rqlen = 1;
        std::string processed_str = convertEscape(attrValue);
        const char* check = processed_str.c_str();
        for( s=check; *s; s++ ) {
            rqlen++;
            for( e=needEsc; *e; e++ ) {
                if( *e == *s ) {
                    rqlen+=2;
                    break;
                }
            }
        }
//        nodeData->text = malloc(strlen(attrValue)+1);
        nodeData->text = (char*)malloc(rqlen);
        for( s=check, t=nodeData->text; *s; s++, t++ ) {
            *t = *s;
            for( e=needEsc; *e; e++ ) {
                if( *e == *s ) {
                    *t = '%';
                    t++;
                    *t = hexchars[((*s)>>4)&0xF];
                    t++;
                    *t = hexchars[(*s)&0xF];
                    break;
                }
            }
        }
        *t = 0;
        
//        strcpy(nodeData->text, attrValue);
        nodeData->textureType = NXT_TextureType_Text;
    } else if( strcasecmp(attrName, "typeface")==0 ) {
        if( nodeData->textStyle.typeface ) {
            free( nodeData->textStyle.typeface );
            nodeData->textStyle.typeface = NULL;
        }

        if( nodeData->textStyle.default_typeface ) {
            free( nodeData->textStyle.default_typeface );
            nodeData->textStyle.default_typeface = NULL;
        }

        nodeData->textStyle.typeface = (char*)malloc(strlen(attrValue)+1);
        strcpy(nodeData->textStyle.typeface, attrValue);
        nodeData->textureType = NXT_TextureType_Text;
    } else if( strcasecmp(attrName, "width")==0 ) {
        float value = 0;
        NXT_FloatVectorFromString( attrValue, &value, 1 );
        nodeData->textStyle.reqWidth = (int)value;
    } else if( strcasecmp(attrName, "height")==0 ) {
        float value = 0;
        NXT_FloatVectorFromString( attrValue, &value, 1 );
        nodeData->textStyle.reqHeight = (int)value;
    } else if( strcasecmp(attrName, "maxlines")==0 ) {
        float value = 0;
        NXT_FloatVectorFromString( attrValue, &value, 1 );
        nodeData->textStyle.maxLines = (int)value;
    } else if( strcasecmp(attrName, "skewx")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->textStyle.textSkewX, 1 );
    } else if( strcasecmp(attrName, "scalex")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->textStyle.textScaleX, 1 );
    } else if( strcasecmp(attrName, "textsize")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->textStyle.textSize, 1 );
    } else if( strcasecmp(attrName, "textstrokewidth")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->textStyle.textStrokeWidth, 1 );
    } else if( strcasecmp(attrName, "fillcolor")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->textStyle.fillColor.e, 4 );
    } else if( strcasecmp(attrName, "strokecolor")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->textStyle.strokeColor.e, 4 );
    } else if( strcasecmp(attrName, "shadowcolor")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->textStyle.shadowColor.e, 4 );
    } else if( strcasecmp(attrName, "bgcolor")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->textStyle.bgColor.e, 4 );
    } else if( strcasecmp(attrName, "shadowoffset")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->textStyle.shadowOffset.e, 4 );
    } else if( strcasecmp(attrName, "shadowradius")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->textStyle.shadowRadius, 1 );
    } else if( strcasecmp(attrName, "textblur")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->textStyle.textBlurRadius, 1 );
    } else if( strcasecmp(attrName, "textmargin")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->textStyle.textMargin, 1 );
    } else if( strcasecmp(attrName, "spacingmult")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->textStyle.spacingMult, 1 );
    } else if( strcasecmp(attrName, "spacingadd")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->textStyle.spacingAdd, 1 );
    } else if( strcasecmp(attrName, "nocache")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->bNoCache = 1;
                break;
            case TFUFalse:
                nodeData->bNoCache = 0;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "bold")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_BOLD;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_BOLD;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "strokebehind")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_STROKEBACK;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_STROKEBACK;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "cutout")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_CUTOUT;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_CUTOUT;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "italic")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_ITALIC;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_ITALIC;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "fill")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_FILL;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_FILL;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "autosize")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_AUTOSIZE;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_AUTOSIZE;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "stroke")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_STROKE;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_STROKE;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "underline")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_UNDERLINE;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_UNDERLINE;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "strike")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_STRIKE;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_STRIKE;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "hinting")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_HINTING;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_HINTING;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "subpixel")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_SUBPIXEL;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_SUBPIXEL;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "shadow")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_SHADOW;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_SHADOW;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "linear")==0 ) {
        switch( NXT_ParseBool(attrValue) ) {
            case TFUTrue:
                nodeData->textStyle.textFlags |= NXT_TEXTFLAG_LINEAR;
                break;
            case TFUFalse:
                nodeData->textStyle.textFlags &= ~NXT_TEXTFLAG_LINEAR;
                break;
            default:
                break;
        }
    } else if( strcasecmp(attrName, "textblurtype")==0 ) {
        if( strcasecmp(attrValue,"normal")==0 ) {
            nodeData->textStyle.blurType = NXT_BLUR_NORMAL;
        } else if( strcasecmp(attrValue,"inner")==0 ) {
            nodeData->textStyle.blurType = NXT_BLUR_INNER;
        } else if( strcasecmp(attrValue,"outer")==0 ) {
            nodeData->textStyle.blurType = NXT_BLUR_OUTER;
        } else if( strcasecmp(attrValue,"solid")==0 ) {
            nodeData->textStyle.blurType = NXT_BLUR_SOLID;
        }
    } else if( strcasecmp(attrName, "shadowblurtype")==0 ) {
        if( strcasecmp(attrValue,"normal")==0 ) {
            nodeData->textStyle.shadowblurType = NXT_BLUR_NORMAL;
        } else if( strcasecmp(attrValue,"inner")==0 ) {
            nodeData->textStyle.shadowblurType = NXT_BLUR_INNER;
        } else if( strcasecmp(attrValue,"outer")==0 ) {
            nodeData->textStyle.shadowblurType = NXT_BLUR_OUTER;
        } else if( strcasecmp(attrValue,"solid")==0 ) {
            nodeData->textStyle.shadowblurType = NXT_BLUR_SOLID;
        }
    } else if( strcasecmp(attrName, "textalign")==0 ) {
        if( strcasecmp(attrValue,"left")==0 ) {
            nodeData->textStyle.textAlign = NXT_ALIGN_LEFT;
        } else if( strcasecmp(attrValue,"center")==0 ) {
            nodeData->textStyle.textAlign = NXT_ALIGN_CENTER;
        } else if( strcasecmp(attrValue,"right")==0 ) {
            nodeData->textStyle.textAlign = NXT_ALIGN_RIGHT;
        } else {
            NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->textStyle.textAlign, 1 );
        }
    } else if( strcasecmp(attrName, "textvalign")==0 ) {
        if( strcasecmp(attrValue,"top")==0 ) {
            nodeData->textStyle.textVAlign = NXT_VALIGN_TOP;
        } else if( strcasecmp(attrValue,"center")==0 ) {
            nodeData->textStyle.textVAlign = NXT_VALIGN_CENTER;
        } else if( strcasecmp(attrValue,"bottom")==0 ) {
            nodeData->textStyle.textVAlign = NXT_VALIGN_BOTTOM;
        } else {
            NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->textStyle.textVAlign, 1 );
        }
    } else if( strcasecmp(attrName, "animated")==0 ) {
        float value[3] = {0};
        NXT_FloatVectorFromString( attrValue, value, 3 );
        nodeData->texinfo_preview.animRows = (int)(value[0]);
        nodeData->texinfo_preview.animCols = (int)(value[1]);
        nodeData->texinfo_preview.animFrames = (int)(value[2]);
        if( nodeData->texinfo_preview.animRows < 1 ) {
            nodeData->texinfo_preview.animRows = 1;
        }
        if( nodeData->texinfo_preview.animCols < 1 ) {
            nodeData->texinfo_preview.animCols = 1;
        }
        unsigned int maxFrames = nodeData->texinfo_preview.animRows * nodeData->texinfo_preview.animCols;
        if( nodeData->texinfo_preview.animFrames < 1 || nodeData->texinfo_preview.animFrames > maxFrames ) {
            nodeData->texinfo_preview.animFrames = maxFrames;
        }
        nodeData->texinfo_preview.bAnimated = 1;
    } else if( strcasecmp(attrName, "wrap")==0 ) {
        if( strcasecmp(attrValue,"none")==0 ) {
            nodeData->texinfo_preview.texWrapS = NXT_TextureWrap_None;
            nodeData->texinfo_preview.texWrapT = NXT_TextureWrap_None;
        } else if( strcasecmp(attrValue,"repeat")==0 ) {
            nodeData->texinfo_preview.texWrapS = NXT_TextureWrap_Repeat;
            nodeData->texinfo_preview.texWrapT = NXT_TextureWrap_Repeat;
        } else if( strcasecmp(attrValue,"mirror")==0 ) {
            nodeData->texinfo_preview.texWrapS = NXT_TextureWrap_MirrorRepeat;
            nodeData->texinfo_preview.texWrapT = NXT_TextureWrap_MirrorRepeat;
        }
    } else if( strcasecmp(attrName, "wraps")==0 ) {
        if( strcasecmp(attrValue,"none")==0 ) {
            nodeData->texinfo_preview.texWrapS = NXT_TextureWrap_None;
        } else if( strcasecmp(attrValue,"repeat")==0 ) {
            nodeData->texinfo_preview.texWrapS = NXT_TextureWrap_Repeat;
        } else if( strcasecmp(attrValue,"mirror")==0 ) {
            nodeData->texinfo_preview.texWrapS = NXT_TextureWrap_MirrorRepeat;
        }
    } else if( strcasecmp(attrName, "wrapt")==0 ) {
        if( strcasecmp(attrValue,"none")==0 ) {
            nodeData->texinfo_preview.texWrapT = NXT_TextureWrap_None;
        } else if( strcasecmp(attrValue,"repeat")==0 ) {
            nodeData->texinfo_preview.texWrapT = NXT_TextureWrap_Repeat;
        } else if( strcasecmp(attrValue,"mirror")==0 ) {
            nodeData->texinfo_preview.texWrapT = NXT_TextureWrap_MirrorRepeat;
        }
    } else if( strcasecmp(attrName, "longtext")==0 ) {
        if( strcasecmp(attrValue,"crop-end")==0 || strcasecmp(attrValue,"crop")==0 ) {
            nodeData->textStyle.longText = NXT_LONGTEXT_CROP_END;
        } else if( strcasecmp(attrValue,"wrap")==0 ) {
            nodeData->textStyle.longText = NXT_LONGTEXT_WRAP;
        } else if( strcasecmp(attrValue,"ellipsize-start")==0 ) {
            nodeData->textStyle.longText = NXT_LONGTEXT_ELLIPSIZE_START;
        } else if( strcasecmp(attrValue,"ellipsize-middle")==0 ) {
            nodeData->textStyle.longText = NXT_LONGTEXT_ELLIPSIZE_MIDDLE;
        } else if( strcasecmp(attrValue,"ellipsize-end")==0 ) {
            nodeData->textStyle.longText = NXT_LONGTEXT_ELLIPSIZE_END;
        }

    } else {
        // TODO: Error        
    }
}

static void updateKey(NXT_HThemeRenderer renderer, NXT_Node_Texture *nodeData){

    if(NULL != nodeData->key)
        return;

    char real_key[256];
    char* pkey = real_key;
    if(0 == NXT_ThemeRenderer_GetEffectImagePath(renderer, nodeData->path, real_key))
        pkey = nodeData->path;

    int len = strlen(pkey);
    nodeData->key = (char*)malloc(sizeof(char) * (len + 1));
    memcpy(nodeData->key, pkey, sizeof(char) * len);
    nodeData->key[len] = 0x0;
}

static int nodeGetPrecacheResource(NXT_NodeHeader* node, NXT_HThemeRenderer renderer, void* load_resource_list){
    
	NXT_Node_Texture *nodeData = (NXT_Node_Texture*)node;
    
    NXT_TextureInfo *texinfo = validateContextAndGetTexInfo(renderer, nodeData);
    
    if( nodeData->textureType==NXT_TextureType_File ) {

        if( !nodeData->path ) {
            // Can't load the texture if we didn't get a path
            return 0;
        }

        updateKey(renderer, nodeData);

        if(NXT_Theme_GetTextureInfo(renderer->getTexManager(), nodeData->key, texinfo)){
            return 0;
        }

        NXT_PrecacheResourceInfo info;
		info.name = NULL;
		info.uid = NULL;
        int len = strlen(nodeData->path);
        info.type = 0;
        info.name = new char[len + 1];
        strcpy((char*)info.name, nodeData->path);
        info.name[len] = 0;

        ((std::vector<NXT_PrecacheResourceInfo>*)load_resource_list)->push_back(info);
	}
    return 0;
}

static int nodePrecacheFunc(NXT_NodeHeader *node, NXT_HThemeRenderer renderer, NXT_PrecacheMode precacheMode, int* asyncmode, int* max_replaceable) {
    
    NXT_Node_Texture *nodeData = (NXT_Node_Texture*)node;
    
    NXT_TextureInfo *texinfo = validateContextAndGetTexInfo(renderer, nodeData);

    if(nodeData->videoSource){

        nodeData->textureType = NXT_TextureType_Video;
    }
    
    if( nodeData->textureType==NXT_TextureType_File ) {

        updateKey(renderer, nodeData);

        if(NXT_Theme_GetTextureInfo(renderer->getTexManager(), nodeData->key, texinfo)){

            updateTexture(nodeData, renderer, texinfo);
            (*max_replaceable) -= (texinfo->textureWidth * texinfo->textureHeight);
            if(*max_replaceable <= 0)
                return 1;
            return 0;
        }

        if( !nodeData->path ) {
            // Can't load the texture if we didn't get a path
            return 0;
        }
        
        // Load the image (if necessary)
        if( !nodeData->bLoadedImage && renderer->loadImageCallback ) {
#ifdef ALTERNATE_PREVIEW_RESOLUTION
            if( renderer->rendererType==NXT_RendererType_InternalPreviewContext && strncmp(nodeData->path, "[ThemeImage]", 12)==0 ) {
                memcpy(nodeData->path+1, "PvwTh", 5);
            }
#endif
            nodeData->bLoadedImage = 0;
            NXT_Error result;
            LOGI("About to load image (%s)", nodeData->path);
            renderer->lockCachedBitmap();
            if(!renderer->getCachedBitmap(nodeData->path, &nodeData->imgInfo)){

                result = renderer->loadImageCallback(&nodeData->imgInfo,
                                                 nodeData->path,
                                                 *asyncmode,
                                                 renderer->imageCallbackPvtData);
				nodeData->imageCallbackPvtData = renderer->imageCallbackPvtData;
                premultAlpha((unsigned int*)nodeData->imgInfo.pixels, nodeData->imgInfo.width*nodeData->imgInfo.height);
            }
			else{
				nodeData->imgInfo.freeImageCallback = NULL;
			}

            nodeData->bLoadedImage = 1;
            (*max_replaceable) -= (nodeData->imgInfo.width * nodeData->imgInfo.height);
            if(*max_replaceable < 0)
            {
                renderer->unlockCachedBitmap();
                return 1;
            }
            else
            {
                uploadTexture(nodeData, renderer, NULL);
                renderer->unlockCachedBitmap();
                return 0;
            }
        }
        else{

            return 0;
        }
        
        //LOGI("Make new image texture");
        
    } else {
        return 0;
    }
    
    //LOGD("uploadTexture : RENDERER");
    uploadTexture(nodeData, renderer, NULL);
    return 0;
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
    
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_Texture *nodeData = (NXT_Node_Texture*)node;
    LOGD( "(Texture src='%s')", nodeData->path );
}

static NXT_TextureInfo * validateContextAndGetTexInfo( NXT_HThemeRenderer renderer, NXT_Node_Texture *nodeData) {
    return &(nodeData->texinfo_preview);
}

static int checkChangeOnParameter(NXT_RenderItem_KeyValues* pprekeyvalues, NXT_RenderItem_KeyValues* pkeyvalues){

    if(pprekeyvalues->items_.size() != pkeyvalues->items_.size()){

        pprekeyvalues->clear();

        NXT_RenderItem_KeyValues::list_t& items = pkeyvalues->items_;
        NXT_RenderItem_KeyValues::list_t& targetitems = pprekeyvalues->items_;

        for(NXT_RenderItem_KeyValues::iter_t itor = items.begin(); itor != items.end(); ++itor){

            targetitems.push_back(new NXT_RenderItem_KeyValue(**itor));
        }

        return 1;
    }

    int value_count = pkeyvalues->items_.size();

    for(int i = 0; i < value_count; ++i){

        if(*pprekeyvalues->items_[i] != *pkeyvalues->items_[i])
            return 1;
    }
    return 0;
}

static void setParameterKeyValue(NXT_HThemeRenderer renderer, NXT_RenderItem_KeyValues* pkeyvalues){

    NXT_Theme_ResetDefaultValuemap(renderer->getRenderItemManager());


    NXT_RenderItem_KeyValues::list_t& values = pkeyvalues->items_;

    for(NXT_RenderItem_KeyValues::iter_t itor = values.begin(); itor != values.end(); ++itor){

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

static void applyKEDLItem(NXT_NodeHeader* node, NXT_HThemeRenderer renderer){

    NXT_Node_Texture* nodeData = (NXT_Node_Texture*)node;
    if(NULL == nodeData->kedlitem)
        return;

    
    if(nodeData->check_for_src_update_)
        NXT_Effect_UpdateUserFields(nodeData->kedlitem->effect, renderer);

    nodeData->check_for_src_update_ = 0;

    renderer->pushVideoTrack();

    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, (NXT_RenderTarget*)nodeData->prender_target_);
    nodeData->prender_target_ = NXT_ThemeRenderer_GetRenderTarget(renderer, renderer->view_width, renderer->view_height, 1, 0, 0);
    NXT_RenderTarget* pdepthonly = NXT_ThemeRenderer_GetDepthonly(renderer, renderer->view_width, renderer->view_height, 1);
    NXT_RenderTarget* prender_target = (NXT_RenderTarget*)nodeData->prender_target_;
    NXT_RenderTarget* pbackup_main_rendertarget = renderer->pmain_render_target_;
    renderer->pmain_render_target_ = prender_target;
    NXT_ThemeRenderer_SetRenderTargetCombined(renderer, prender_target, pdepthonly);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
    renderer->pmain_render_target_ = pbackup_main_rendertarget;
    NXT_ThemeRenderer_UnbindDepth(renderer, prender_target);
    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, pdepthonly);
    NXT_ThemeRenderer_SetRenderToDefault(renderer);
    if(1){

        nodeData->texinfo_preview.srcWidth = prender_target->width_;
        nodeData->texinfo_preview.srcHeight = prender_target->height_;
        nodeData->texinfo_preview.textureWidth = prender_target->width_;
        nodeData->texinfo_preview.textureHeight = prender_target->height_;
        nodeData->texinfo_preview.left = 0;
        nodeData->texinfo_preview.bottom = prender_target->height_;
        nodeData->texinfo_preview.right = prender_target->width_;
        nodeData->texinfo_preview.top = 0;
        nodeData->texinfo_preview.textureFormat = NXT_PixelFormat_RGBA8888;
        nodeData->texinfo_preview.texName_for_rgb = prender_target->target_texture_;
    }
    
    // NXT_ThemeRenderer_ReleaseRenderTarget(renderer, prender_target);

    LOGFORCE("[%s %d] nodeData->texinfo_preview.texName_for_rgb:%d", __func__, __LINE__, nodeData->texinfo_preview.texName_for_rgb);

    if(nodeData->videoSource){

        nodeData->textureType = NXT_TextureType_File;
    }

    renderer->popVideoTrack();
}

static void applyRenderItem(NXT_NodeHeader *node, NXT_HThemeRenderer renderer){

    NXT_Node_Texture *nodeData = (NXT_Node_Texture*)node;
    if(NULL == nodeData->renderitem)
        return;

    if(0 == nodeData->check_for_src_update_ && 0 == checkChangeOnParameter(&nodeData->renderitem_parameter_pre_values_, &nodeData->renderitem_parameter_values_))
        return;

    renderer->pushVideoTrack();

    LOGFORCE("[%s %d] nodeData->check_for_src_update_:%d", __func__, __LINE__, nodeData->check_for_src_update_);

    nodeData->check_for_src_update_ = 0;

    if(nodeData->videoSource){

        LOGFORCE("[%s %d]", __func__, __LINE__);
        nodeData->textureType = NXT_TextureType_Video;
    }

    nodeData->texinfo_preview.texName_for_rgb = nodeData->texinfo_preview.texName[0];

    NXT_TextureInfo *texture = NULL;
    NXT_TextureInfo *mask = NULL;
    NXT_Vector4f savedColor;
    NXT_Matrix4f texmat, colormat;

    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, (NXT_RenderTarget*)nodeData->prender_target_);

    NexThemeRenderer_PrepStateForRender2(renderer, nodeData, NULL, &texture, &mask, &texmat, &colormat, 0);
    nodeData->prender_target_ = NXT_ThemeRenderer_GetRenderTarget(renderer, texture->textureWidth, texture->textureHeight,  0, 0, 0);
    NXT_RenderTarget* pdepthonly = NXT_ThemeRenderer_GetDepthonly(renderer, renderer->view_width, renderer->view_height, 1);
    NXT_RenderTarget* prender_target = (NXT_RenderTarget*)nodeData->prender_target_;
    NXT_ThemeRenderer_SetRenderTargetCombined(renderer, prender_target, pdepthonly);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_BLEND);
    CHECK_GL_ERROR();
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    NXT_Theme_BeginRenderItem(renderer->getRenderItemManager(), nodeData->renderitem->id, prender_target->width_, prender_target->height_, prender_target);
    if(texture){

        if(texture->textureFormat != NXT_PixelFormat_RGBA8888)
           NXT_ThemeRenderer_ConvertTextureToRGB(renderer, texture, 0, 0, 0, 0, NULL, 0, 10000.0);

        LOGFORCE("[%s %d] prender_target->width_:%d prender_target->height_:%d texture->texName_for_rgb:%d", __func__, __LINE__, prender_target->width_, prender_target->height_, texture->texName_for_rgb);

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
    static NXT_Matrix4f idmat = NXT_Matrix4f(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        );
    setParameterKeyValue(renderer, &nodeData->renderitem_parameter_values_);
    NXT_Theme_SetRect(renderer->getRenderItemManager(), -1, 1, 1, -1, 1);
    NXT_Theme_SetMatrix(renderer->getRenderItemManager(), idmat.e);
    int ret = NXT_Theme_ApplyRenderItem(renderer->getRenderItemManager(), renderer->time);
    NXT_Theme_ResetMatrix(renderer->getRenderItemManager());
    NXT_Theme_EndRenderItem(renderer->getRenderItemManager());
    NXT_ThemeRenderer_UnbindDepth(renderer, prender_target);
    NXT_ThemeRenderer_ReleaseRenderTarget(renderer, pdepthonly);
    NXT_ThemeRenderer_SetRenderToDefault(renderer);
    if(ret){

        nodeData->texinfo_preview.srcWidth = prender_target->width_;
        nodeData->texinfo_preview.srcHeight = prender_target->height_;
        nodeData->texinfo_preview.textureWidth = prender_target->width_;
        nodeData->texinfo_preview.textureHeight = prender_target->height_;
        nodeData->texinfo_preview.left = 0;
        nodeData->texinfo_preview.bottom = 0;
        nodeData->texinfo_preview.right = prender_target->width_;
        nodeData->texinfo_preview.top = prender_target->height_;
        nodeData->texinfo_preview.textureFormat = NXT_PixelFormat_RGBA8888;
        nodeData->texinfo_preview.texName_for_rgb = prender_target->target_texture_;
    }
    
    // NXT_ThemeRenderer_ReleaseRenderTarget(renderer, prender_target);

    LOGFORCE("[%s %d] nodeData->texinfo_preview.texName_for_rgb:%d", __func__, __LINE__, nodeData->texinfo_preview.texName_for_rgb);

    if(nodeData->videoSource){

        nodeData->textureType = NXT_TextureType_File;
    }

    renderer->popVideoTrack();
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Texture *nodeData = (NXT_Node_Texture*)node;
    LOGI("[NexTheme_Node_Texture %d] Traverse texture node [%d] (%s) type=%d", __LINE__, nodeData->videoSource, nodeData->path, nodeData->textureType);
    
    NXT_TextureInfo *texinfo = validateContextAndGetTexInfo(renderer, nodeData);
    
    char *videoPlaceholder = NULL;

    if(nodeData->videoSource){

        nodeData->textureType = NXT_TextureType_Video;
    }
    
    if( nodeData->textureType==NXT_TextureType_Video ) {

        NXT_TextureID texid = (NXT_TextureID)0;
        switch( nodeData->videoSource ) {
            case 1: 
                videoPlaceholder    = renderer->videoPlaceholder1; 
                texid               = NXT_TextureID_Video_1;
                break;
            case 2:
                videoPlaceholder    = renderer->videoPlaceholder2; 
                texid               = NXT_TextureID_Video_2;
                break;
            default: 
                // Invalid value
                goto last_work;
        }

        // renderer->resetVideoSrc(texid);
        if(renderer->getCurrentTrackID() >= 0){

            LOGI("TM30 getVideoSrc setting begin");

            if(false == renderer->checkVideoTrack(texid)){

                int track = renderer->getCurrentTrackID();
                renderer->setVideoTrack(texid, track);
                int src = renderer->getDecoVideoSrcForUID(renderer->addCurrentTrackID());
                renderer->setVideoSrc(texid, &renderer->getRawVideoSrc(src));
                LOGI("TM30 getVideoSrc setting1 track:%d deco src:%d texid:%d nexttrack:%d", renderer->getVideoTrack(texid), src, texid, renderer->getCurrentTrackID());
            }
            else{

                int track = renderer->getVideoTrack(texid);
                int src = renderer->getDecoVideoSrcForUID(track);
                renderer->setVideoSrc(texid, &renderer->getRawVideoSrc(src));
                LOGI("TM30 getVideoSrc setting2 track:%d deco src:%d texid:%d", track, src, texid);
            }
            LOGI("TM30 getVideoSrc setting end");
        }

        NXT_TextureInfo& texinfo = renderer->getVideoSrc(texid);

        if( texinfo.bValidTexture && (!renderer->videoPlaceholder1 || !renderer->videoPlaceholder2))       // Don't use the placeholder if we have a valid texture 
        {
            if(texinfo.track_update_id_ != nodeData->texinfo_preview.track_update_id_){

                nodeData->texinfo_preview.track_update_id_ = texinfo.track_update_id_;
                nodeData->check_for_src_update_ = 1;
            }
            LOGI("[NexTheme_Node_Texture %d] videoPlaceholder='%s'; nodeData->path='%s'; %s/%s", __LINE__, videoPlaceholder, nodeData->path, renderer->videoPlaceholder1, renderer->videoPlaceholder2);
            goto last_work;
        }
    }
    
    LOGFORCE("[NexTheme_Node_Texture %d] videoPlaceholder='%s'; nodeData->path='%s'", __LINE__, videoPlaceholder, nodeData->path);
    
    if( videoPlaceholder ) {
        if(nodeData->path && strcmp(videoPlaceholder, nodeData->path)!=0) 
        {
            LOGFORCE("Change path");
            texinfo->bValidTexture = 0;
            nodeData->imgInfo.width = 0;
            nodeData->imgInfo.height = 0;
            free(nodeData->path);
            nodeData->path = NULL;
        }
        if( !nodeData->path ) {
            LOGFORCE("Alloc new path");
            nodeData->path = (char*)malloc( strlen(videoPlaceholder)+1 );
            strcpy( nodeData->path, videoPlaceholder );
        }
    }
    
    if( nodeData->textureType==NXT_TextureType_File || (nodeData->textureType==NXT_TextureType_Video && videoPlaceholder) ) {

        updateKey(renderer, nodeData);

        LOGFORCE("nodeData->key:%s", nodeData->key);

        if(NXT_Theme_GetTextureInfo(renderer->getTexManager(), nodeData->key, texinfo)){

            updateTexture(nodeData, renderer, texinfo);
            goto last_work;
        }

        if( !nodeData->path ) {
            // Can't load the texture if we didn't get a path
            goto last_work;
        }
        
        // Load the image (if necessary)
        if( !nodeData->bLoadedImage && renderer->loadImageCallback ) {
#ifdef ALTERNATE_PREVIEW_RESOLUTION
            if( renderer->rendererType==NXT_RendererType_InternalPreviewContext && strncmp(nodeData->path, "[ThemeImage]", 12)==0 ) {
                memcpy(nodeData->path+1, "PvwTh", 5);
            }
#endif
            NXT_Error result = NXT_Error_None;
            renderer->lockCachedBitmap();
			if(!renderer->getCachedBitmap(nodeData->path, &nodeData->imgInfo)){

                result = renderer->loadImageCallback(&nodeData->imgInfo,
                                                 nodeData->path,
                                                 0,
                                                 renderer->imageCallbackPvtData);
				nodeData->imageCallbackPvtData = renderer->imageCallbackPvtData;
                premultAlpha((unsigned int*)nodeData->imgInfo.pixels, nodeData->imgInfo.width*nodeData->imgInfo.height);
            }
			else{
				nodeData->imgInfo.freeImageCallback = NULL;
			}
            nodeData->bLoadedImage = 1;
            uploadTexture(nodeData, renderer, videoPlaceholder);
            renderer->unlockCachedBitmap();
            goto last_work;
        }
        
        //LOGI("Make new image texture");
        
    } else if( nodeData->textureType==NXT_TextureType_Overlay ) {
        
        if( !renderer->pActiveEffect ) {
            goto last_work;
        }
        if( texinfo->bValidTexture && nodeData->textSerial==renderer->pActiveEffect->textSerial && !nodeData->bNoCache ) {
            // Nothing to do; correct texture is already loaded
            goto last_work;
        }
        
        nodeData->textSerial=renderer->pActiveEffect->textSerial;
        
        // If there's already a loaded image, unload it
        if( nodeData->bLoadedImage ) {
            if( nodeData->bCachedImage ) {
                if(nodeData->imgInfo.pixels) {
                    free(nodeData->imgInfo.pixels);
                    nodeData->imgInfo.pixels = NULL;
                }
                nodeData->bCachedImage = 0;
            } else if ( nodeData->imgInfo.freeImageCallback ) {
                nodeData->imgInfo.freeImageCallback(&nodeData->imgInfo,
                                                    renderer->imageCallbackPvtData);
                nodeData->imgInfo.freeImageCallback = NULL;
            }
            nodeData->bLoadedImage = 0;
        }

        char *path = NXT_ThemeRenderer_GetEffectOption(renderer, nodeData->srcfld);
        if( !path )
            goto last_work;
        
        char *p;
        for( p = path; *p; p++ ) {
            if( *p==' ' )
                path = p+1;
        }
        
        const char *prefix = "[Overlay]";
        
        int pathlen = (int)strlen(path);
        int prefixlen = (int)strlen(prefix);
        char *query = (char*)malloc(prefixlen + pathlen + 1);
        if( !query )
            goto last_work;
        strcpy(query, prefix);
        strcpy(query + prefixlen, path);


        // Load the image (if necessary)
        if( !nodeData->bLoadedImage && renderer->loadImageCallback ) {
            nodeData->bLoadedImage = 1;
            NXT_Error result;
            LOGI("About to load OVERLAY image (%s)", query);
            int t_startLoad = NXT_Profile_TickCount();
            result = renderer->loadImageCallback(&nodeData->imgInfo,
                                                 query,
                                                 0,
                                                 renderer->imageCallbackPvtData);
            nodeData->imageCallbackPvtData = renderer->imageCallbackPvtData;
            
            
            // TODO: Check for errors
            int t_endLoad = NXT_Profile_TickCount();
            premultAlpha((unsigned int*)nodeData->imgInfo.pixels, nodeData->imgInfo.width*nodeData->imgInfo.height);
            int t_endAlpha = NXT_Profile_TickCount();
            LOGI("After load OVERLAY image; width=%d height=%d pixels=0x%p loadtime(%d) alphatime(%d)",
                 nodeData->imgInfo.width, nodeData->imgInfo.height,
                 nodeData->imgInfo.pixels, t_endLoad-t_startLoad, t_endAlpha-t_endLoad);
        }
        
        //LOGI("Make new OVERLAY texture");

    
    } else if( nodeData->textureType==NXT_TextureType_Text ) {

        if( !renderer->pActiveEffect ) {
            goto last_work;
        }

        KeyValueMap_t& keyvaluemap = renderer->pActiveEffect->keyvaluemap_;
        KeyValueMap_t::iterator itor = keyvaluemap.find(nodeData->header.node_id);
        if(itor != keyvaluemap.end()){

            std::string& key = ((nxStringValue*)itor->second)->obj_;

            if(NXT_Theme_GetTextureInfo(renderer->getTexManager(), key.c_str(), texinfo)){

                updateTexture(nodeData, renderer, texinfo);
                goto last_work;
            }
            else{

                delete itor->second;
                keyvaluemap.erase(itor);
            }
        }
        
        // If there's already a loaded image, unload it
        if( nodeData->bLoadedImage ) {
            if( nodeData->bCachedImage ) {
                if(nodeData->imgInfo.pixels) {
                    free(nodeData->imgInfo.pixels);
                    nodeData->imgInfo.pixels = NULL;
                }
                nodeData->bCachedImage = 0;
            } else if ( nodeData->imgInfo.freeImageCallback ) {
                nodeData->imgInfo.freeImageCallback(&nodeData->imgInfo,
                                                    renderer->imageCallbackPvtData);
                nodeData->imgInfo.freeImageCallback = NULL;
            }
            nodeData->bLoadedImage = 0;
        }
        
        // Build the string for loading the image
        char prefix[1024];
        
        prefix[sizeof(prefix)-1]=0;     // snprintf doesn't null-terminate if the length is exceeded; this ensures null termination
        
        const char *base_id = "";
        NXT_NodeHeader *pn;
        
        for( pn = node; pn; pn = pn->parent ) {
            if( pn->isa==&NXT_NodeClass_Effect || pn->isa==&NXT_NodeClass_Theme ) {
                base_id = pn->node_id;
            }
        }

        const char* typeface = nodeData->textStyle.typeface;
        if(nodeData->textStyle.typeface){

            if(nodeData->textStyle.typeface[0] == NXT_ELEMENT_REF_CHAR){

                const char* user_typeface = NXT_ThemeRenderer_GetEffectOption(renderer, &nodeData->textStyle.typeface[1]);
                if(user_typeface){

                    typeface = user_typeface;
                }
                else{

                    if(nodeData->textStyle.default_typeface == NULL){

                        NXT_Node_UserField* userfield = (NXT_Node_UserField*)NXT_FindFirstNodeWithId(node, &nodeData->textStyle.typeface[1], &NXT_NodeClass_UserField);
                        if(userfield && userfield->pDefault){

                            const char* typeface = userfield->pDefault;
                            nodeData->textStyle.default_typeface = (char*)malloc(strlen(typeface)+1);
                            strcpy(nodeData->textStyle.default_typeface, typeface);
                        }
                    }
                    typeface = nodeData->textStyle.default_typeface;
                }
            }
        }

        snprintf( prefix, sizeof(prefix)-1, 
                 "[Text]baseid=%s;typeface=%s;skewx=%010.4f;scalex=%010.4f;spacingmult=%010.4f;spacingadd=%010.4f;size=%010.4f;"
                 "strokewidth=%010.4f;shadowradius=%010.4f;textblur=%010.4f;blurtype=%i;shadowblurtype=%i;margin=%i;flags=%08X;align=%i;longtext=%i;maxlines=%i;"
                 "fillcolor=%08X;strokecolor=%08X;shadowcolor=%08X;bgcolor=%08X;"
                 "shadowoffsx=%010.4f;shadowoffsy=%010.4f;"
                 "width=%d;height=%d;text=%s;;",
                 base_id,
                 typeface,
                 nodeData->textStyle.textSkewX,
                 nodeData->textStyle.textScaleX,
                 nodeData->textStyle.spacingMult,
                 nodeData->textStyle.spacingAdd,
                 nodeData->textStyle.textSize,
                 nodeData->textStyle.textStrokeWidth,
                 nodeData->textStyle.shadowRadius,
                 nodeData->textStyle.textBlurRadius,
                 nodeData->textStyle.blurType,
                 nodeData->textStyle.shadowblurType,
                 (int)nodeData->textStyle.textMargin,
                 nodeData->textStyle.textFlags,
                 (((int)nodeData->textStyle.textAlign)&0xF) | ((int)nodeData->textStyle.textVAlign),
                 nodeData->textStyle.longText,
                 nodeData->textStyle.maxLines,
                 NXT_Vector4f_ToARGBIntColor(nodeData->textStyle.fillColor),
                 NXT_Vector4f_ToARGBIntColor(nodeData->textStyle.strokeColor),
                 NXT_Vector4f_ToARGBIntColor(nodeData->textStyle.shadowColor),
                 NXT_Vector4f_ToARGBIntColor(nodeData->textStyle.bgColor),
                 nodeData->textStyle.shadowOffset.e[0],
                 nodeData->textStyle.shadowOffset.e[1],
                 nodeData->textStyle.reqWidth,
                 nodeData->textStyle.reqHeight,
                 nodeData->text);
        
        /*
        char *titleText = "";
        if( renderer->titleText )
            titleText = renderer->titleText;
         */
        
        const char *titleText = NXT_ThemeRenderer_GetEffectOption(renderer, nodeData->srcfld?nodeData->srcfld:"__title_text__" );
        if( !titleText )
            titleText="";
        
        int titlelen = (int)strlen(titleText);
        int prefixlen = (int)strlen(prefix);
        char* query = (char*)malloc(prefixlen + titlelen + 1);
        strcpy(query, prefix);
        strcpy(query + prefixlen, titleText);
        
        nodeData->bLoadedImage = 1;
        LOGI("About to render text (%s)", query);
        
        if(NXT_Theme_GetTextureInfo(renderer->getTexManager(), query, texinfo)){

            ValueMapObj* pobj = new nxStringValue(std::string(query));
            keyvaluemap.insert(make_pair(std::string(nodeData->header.node_id), pobj));
            updateTexture(nodeData, renderer, texinfo);
            free(query);
            goto last_work;
        }

        NXT_Error result = renderer->loadImageCallback(&nodeData->imgInfo,
                                             query,
                                             0,
                                             renderer->imageCallbackPvtData);
        nodeData->imageCallbackPvtData = renderer->imageCallbackPvtData;
        
        if( result != NXT_Error_None ) {
            LOGE("[Node_Texture %d] Error loading image (%d)", __LINE__, result );
        }
        premultAlpha((unsigned int*)nodeData->imgInfo.pixels, nodeData->imgInfo.width*nodeData->imgInfo.height);
        
        LOGI("After render text; width=%d height=%d pixels=0x%p", 
             nodeData->imgInfo.width, nodeData->imgInfo.height,
             nodeData->imgInfo.pixels);
        ValueMapObj* pobj = new nxStringValue(std::string(query));
        keyvaluemap.insert(make_pair(std::string(nodeData->header.node_id), pobj));
        uploadTexture(nodeData, renderer, videoPlaceholder, query);
        free(query);

        goto last_work;
        
        
    } else {
        goto last_work;
    }
    
    //LOGD("uploadTexture : RENDERER");
    uploadTexture(nodeData, renderer, videoPlaceholder);
last_work:
    applyRenderItem(node, renderer);
    applyKEDLItem(node, renderer);
}

static void updateTexture(NXT_Node_Texture *nodeData, NXT_HThemeRenderer renderer, NXT_TextureInfo* psrc){

    NXT_TextureInfo *texinfo = validateContextAndGetTexInfo(renderer, nodeData);

    if(psrc->srcWidth == nodeData->imgInfo.width && psrc->srcHeight == nodeData->imgInfo.height)
        return;
    
    texinfo->srcWidth = psrc->srcWidth;
    texinfo->srcHeight = psrc->srcHeight;
    texinfo->left = 0;
    texinfo->bottom = 0;
    texinfo->right = texinfo->srcWidth;
    texinfo->top = texinfo->srcHeight;
    texinfo->textureFormat = NXT_PixelFormat_RGBA8888;
    // texinfo->texNameInitCount = 1;
    texinfo->texName[0] = psrc->texName_for_rgb;
    texinfo->texName_for_rgb = texinfo->texName[0];

    nodeData->imgInfo.width = texinfo->srcWidth;
    nodeData->imgInfo.height = texinfo->srcHeight;
    nodeData->imgInfo.pitch = texinfo->srcWidth;
    nodeData->imgInfo.pixels = NULL;
    nodeData->imgInfo.freeImageCallback = NULL;

    texinfo->bValidTexture=1;
    texinfo->textureWidth = texinfo->srcWidth;;//NXT_NextHighestPowerOfTwo(nodeData->imgInfo.pitch);
    texinfo->textureHeight = texinfo->srcHeight;//NXT_NextHighestPowerOfTwo(nodeData->imgInfo.height);
}

static void uploadTexture(NXT_Node_Texture *nodeData, NXT_HThemeRenderer renderer, const char* videoPlaceholder, const char* key) {
  
    NXT_TextureInfo *texinfo = validateContextAndGetTexInfo(renderer, nodeData);
    
    texinfo->bValidTexture=1;
    
    texinfo->srcWidth = nodeData->imgInfo.width;
    texinfo->srcHeight = nodeData->imgInfo.height;
    texinfo->left = 0;
    texinfo->bottom = 0;
    texinfo->right = texinfo->srcWidth;
    texinfo->top = texinfo->srcHeight;
    texinfo->textureFormat = NXT_PixelFormat_RGBA8888;
    
    LOGI("[NexTheme_Node_Texture %d] uploadTexture In(%s) [%d %d]", __LINE__, nodeData->path, texinfo->texWrapS, texinfo->texWrapT);
    
    
    // Allocate OpenGL texture name
    if( texinfo->texNameInitCount < 1 ) {
        GL_GenTextures( 1, &texinfo->texName[0] );
        texinfo->texNameInitCount = 1;
        texinfo->texName_for_rgb = texinfo->texName[0];
    }

    if(nodeData->textureType==NXT_TextureType_File || (nodeData->textureType==NXT_TextureType_Video && videoPlaceholder) || nodeData->textureType == NXT_TextureType_Text){

        if(nodeData->textureType==NXT_TextureType_File || (nodeData->textureType==NXT_TextureType_Video && videoPlaceholder)){
            updateKey(renderer, nodeData);
            NXT_Theme_SetTextureInfo(renderer->getTexManager(), nodeData->key, texinfo);
        }
        else
            NXT_Theme_SetTextureInfo(renderer->getTexManager(), key, texinfo);
        
    }
    
    // Initialize
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texinfo->texName[0]);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nodeData->bMipmap?GL_LINEAR_MIPMAP_LINEAR:GL_LINEAR /*GL_NEAREST*/ );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/ );
    switch( texinfo->texWrapS ) {
        case NXT_TextureWrap_Repeat:
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            break;
        case NXT_TextureWrap_MirrorRepeat:
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
            break;
        case NXT_TextureWrap_None:
        default:
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            break;
    }
    switch( texinfo->texWrapT ) {
        case NXT_TextureWrap_Repeat:
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
            break;
        case NXT_TextureWrap_MirrorRepeat:
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
            break;
        case NXT_TextureWrap_None:
        default:
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            break;
    }
    
    // Choose texture format
    GLint texInternalFormat;
    GLenum texDataFormat;
    GLenum texDataType;
    int bytesPerPixel;
    switch( nodeData->imgInfo.pixelFormat ) {
        case NXT_PixelFormat_RGB565:
            LOGD("[NexTheme_Node_Texture %d] uploadTexture : color format RGB565", __LINE__, nodeData->path);
            texInternalFormat=GL_RGB;
            texDataFormat=GL_RGB;
            texDataType=GL_UNSIGNED_SHORT_5_6_5;
            bytesPerPixel = 2;
            break;
        case NXT_PixelFormat_RGBA8888:
        default:
            LOGD("[NexTheme_Node_Texture %d] uploadTexture : color format RGBA8888", __LINE__, nodeData->path);
            texInternalFormat=GL_RGBA;
            texDataFormat=GL_RGBA;
            texDataType=GL_UNSIGNED_BYTE;
            bytesPerPixel = 4;
            break;
    }
    
    /***** TESTING CODE - CHECK EMPTY *****/
    /*unsigned char *p =  (unsigned char*)nodeData->imgInfo.pixels;
    unsigned char *d = p + (nodeData->imgInfo.pitch*nodeData->imgInfo.height*bytesPerPixel);
    unsigned char bEmpty = 1;
    while( p < d ) {
        if( *p && *p != 0xFF ) {
            bEmpty = 0;
            break;
        }
        p++;
    }*/
    /**************************************/
    
    // Calculate texture size
    texinfo->textureWidth = NXT_NextHighestPowerOfTwo(nodeData->imgInfo.pitch);
    texinfo->textureHeight = NXT_NextHighestPowerOfTwo(nodeData->imgInfo.height);
    
    int t_startUpload = NXT_Profile_TickCount();
    
    // Make the texture
    if(texinfo->textureWidth == nodeData->imgInfo.pitch &&
       texinfo->textureHeight == nodeData->imgInfo.height)
    {
        //LOGD("[NexTheme_Node_Texture %d] uploadTexture : pitch match (%d x %d == %d x %d) bEmpty=%d", __LINE__, texinfo->textureWidth, texinfo->textureHeight, nodeData->imgInfo.pitch, nodeData->imgInfo.height, bEmpty);
        glTexImage2D(GL_TEXTURE_2D, 0, texInternalFormat,
                     texinfo->textureWidth, texinfo->textureHeight, 0,
                     texDataFormat, texDataType, 
                     (const GLvoid*)nodeData->imgInfo.pixels );
        CHECK_GL_ERROR();
    }
    else
    {
        //LOGD("[NexTheme_Node_Texture %d] uploadTexture : pitch mismatch (%d x %d != %d x %d) bEmpty=%d", __LINE__, texinfo->textureWidth, texinfo->textureHeight, nodeData->imgInfo.pitch, nodeData->imgInfo.height, bEmpty);

        char* buf = (char*)malloc(sizeof(char) * texinfo->textureWidth * texinfo->textureHeight * 4);
        memset(buf, 0x0, sizeof(char) * texinfo->textureWidth * texinfo->textureHeight * 4);

        glTexImage2D(GL_TEXTURE_2D, 0, texInternalFormat,
                     texinfo->textureWidth, texinfo->textureHeight, 0,
                     texDataFormat, texDataType, 
                     buf);
        free(buf);
        CHECK_GL_ERROR();
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                        nodeData->imgInfo.pitch, nodeData->imgInfo.height, 
                        texDataFormat, texDataType, 
                        (const GLvoid*)nodeData->imgInfo.pixels );
        CHECK_GL_ERROR();
    }

    if(nodeData->bMipmap)
        glGenerateMipmap(GL_TEXTURE_2D);
    
    int t_totalUpload = NXT_Profile_TickCount()-t_startUpload;
    
    LOGD("[NexTheme_Node_Texture %d] uploadTexture took %dms", t_totalUpload );
    // TODO: Free the image at the Java side?
    /*if( nodeData->imgInfo.freeImageCallback ) {
     nodeData->imgInfo.freeImageCallback(&nodeData->imgInfo,
     renderer->imageCallbackPvtData);
     nodeData->bLoadedImage = 0;
     }*/
    
    if( nodeData->bLoadedImage ) {
        LOGD("[NexTheme_Node_Texture %d] uploadTexture : bLoadedImage TRUE --> bCachedImage=%d", __LINE__, nodeData->bCachedImage);
        if( nodeData->bCachedImage ) {
            if(nodeData->imgInfo.pixels) {
                LOGD("[NexTheme_Node_Texture %d] uploadTexture : direct free", __LINE__, nodeData->bCachedImage);
                free(nodeData->imgInfo.pixels);
                nodeData->imgInfo.pixels = NULL;
            }
            nodeData->bCachedImage = 0;
        } else if ( nodeData->imgInfo.freeImageCallback ) {
            LOGD("[NexTheme_Node_Texture %d] uploadTexture : callback free", __LINE__, nodeData->bCachedImage);
            nodeData->imgInfo.freeImageCallback(&nodeData->imgInfo,
                                                renderer->imageCallbackPvtData);
            nodeData->imgInfo.freeImageCallback = NULL;
        }
        nodeData->bLoadedImage = 0;
    }
    LOGD("[NexTheme_Node_Texture %d] uploadTexture Out(%s)", __LINE__, nodeData->path);
    nodeData->check_for_src_update_ = 1;    
}

static void uploadTexture(NXT_Node_Texture* nodeData, NXT_HThemeRenderer renderer, const char* videoPlaceholder){

    uploadTexture(nodeData, renderer, videoPlaceholder, nodeData->key);
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}



static void premultAlpha(unsigned int *pixels, unsigned int numPixels) {
    
#ifdef __APPLE__ // NESI-380 pixels provided from iOS SDK is already premultified alpha
    return;
#else
    if( !pixels )
        return;
    
    unsigned char *p = (unsigned char*)pixels;
    unsigned char *end = p + (numPixels*4);
    
    for( ; p<end; p+=4 ) {
        unsigned char a = *(p+3);
        *(p+0) = *(p+0) * a / 255;
        *(p+1) = *(p+1) * a / 255;
        *(p+2) = *(p+2) * a / 255;

    }
#endif
}



void NXT_Theme_SetTextureInfoAsync(NXT_HThemeRenderer renderer, const char* path, int* pixels, int width, int height){

    if(NULL == renderer)
        return;

    LOGD("[NexTheme_Node_Texture %d] NXT_Theme_SetTextureInfoAsync(%s)", __LINE__, path);
    premultAlpha((unsigned int*)pixels, width * height);
    char real_key[256];
    const char* pkey = real_key;
    if(0 == NXT_ThemeRenderer_GetEffectImagePath(renderer, path, real_key))
        pkey = path;

    NXT_TextureInfo texinfo;
   
    texinfo.bValidTexture=1;    
    texinfo.srcWidth = width;
    texinfo.srcHeight = height;
    texinfo.left = 0;
    texinfo.bottom = 0;
    texinfo.right = width;
    texinfo.top = height;
    texinfo.textureFormat = NXT_PixelFormat_RGBA8888;

    NXT_Error error = NXT_ThemeRenderer_AquireContext(renderer);

    if(error != NXT_Error_None && error != NXT_Error_WouldDeadlock)
        return;

    GL_GenTextures( 1, &texinfo.texName[0] );
    texinfo.texNameInitCount = 1;
    texinfo.texName_for_rgb = texinfo.texName[0];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texinfo.texName[0]);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR /*GL_NEAREST*/ );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/ );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        
    // Choose texture format
    GLint texInternalFormat;
    GLenum texDataFormat;
    GLenum texDataType;
    int bytesPerPixel;
    texInternalFormat=GL_RGBA;
#ifdef __APPLE__
            texDataFormat=GL_BGRA;
#else
            texDataFormat=GL_RGBA;
#endif
    texDataType=GL_UNSIGNED_BYTE;
    bytesPerPixel = 4;
            
    
    texinfo.textureWidth = width;
    texinfo.textureHeight = height;
    glTexImage2D(GL_TEXTURE_2D, 0, texInternalFormat,
                     texinfo.textureWidth, texinfo.textureHeight, 0,
                     texDataFormat, texDataType, 
                     (const GLvoid*)pixels);
    
    NXT_Theme_SetTextureInfo(renderer->getTexManager(), pkey, &texinfo);

    if(error != NXT_Error_WouldDeadlock)
        NXT_ThemeRenderer_ReleaseContext(renderer, 0);
}

void NXT_Theme_RemoveTextureInfoAsync(NXT_HThemeRenderer renderer, const char* path){

    if(NULL == renderer)
        return;

    char real_key[256];
    const char* pkey = real_key;
    if(0 == NXT_ThemeRenderer_GetEffectImagePath(renderer, path, real_key))
        pkey = path;

    NXT_Error error = NXT_ThemeRenderer_AquireContext(renderer);

    if(error != NXT_Error_None && error != NXT_Error_WouldDeadlock)
        return;
    
    NXT_Theme_RemoveTextureInfo(renderer->getTexManager(), pkey);

    if(error != NXT_Error_WouldDeadlock)
        NXT_ThemeRenderer_ReleaseContext(renderer, 0);
}
