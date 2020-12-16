#ifndef NEXTHEME_NODES_H
#define NEXTHEME_NODES_H

#if defined(ANDROID)
#define NEXTHEME_SUPPORT_OPENGL
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#elif defined(__APPLE__)
#include "TargetConditionals.h"
#if TARGET_OS_IOS > 0
#define NEXTHEME_SUPPORT_OPENGL
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#endif

#define NXT_ALLOW_DIRECT_THEME_ACCESS
#include "NexTheme.h"
#undef NXT_ALLOW_DIRECT_THEME_ACCESS
#include "NexThemeRenderer.h"
#include "NexTheme_Math.h"

#ifdef NEXTHEME_SUPPORT_OPENGL
#ifdef NEX_ENABLE_GLES_IMAGE
#include <EGL/egl.h>
#include <EGL/eglext.h>
//#include "nexgraphicbuffer_api.h"
#endif
#endif

#define NXT_ELEMENT_REF_CHAR ('@')
#define MAX_POLY_POINTS 32767
#define NXT_UPDATETARGET_MAXPARAMS 4

// ---- Types ------------------------------------------------------------------
typedef enum NXT_NodeAction_ {
    NXT_NodeAction_Next         = 0,
    NXT_NodeAction_Repeat       = 1,
    NXT_NodeAction_Stop         = 2
} NXT_NodeAction;

typedef enum NXT_UserFieldType_ {
    NXT_UserFieldType_UNDEFINED = 0,
    NXT_UserFieldType_Text      = 1,
    NXT_UserFieldType_Color     = 2,
    NXT_UserFieldType_Overlay   = 3,
    NXT_UserFieldType_Selection   = 4,
    NXT_UserFieldType_Range = 5
} NXT_UserFieldType;

typedef struct NXT_NodeHeader_ NXT_NodeHeader;
typedef struct NXT_Iter_ NXT_Iter;
typedef struct NXT_NodeClass_ NXT_NodeClass;
typedef void (*NXT_NodeInitFunc)(NXT_NodeHeader *node);
typedef void (*NXT_NodeFreeFunc)(NXT_NodeHeader *node);
typedef void (*NXT_NodeUpdateNotify)(NXT_NodeHeader *node, void* notifyPrivateData);
typedef void (*NXT_NodeSetAttrFunc)(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
typedef void (*NXT_NodeChildrenDoneFunc)(NXT_NodeHeader *node);
typedef void (*NXT_NodeFreeTextureFunc)(NXT_HThemeRenderer_Context context, NXT_NodeHeader *node); // for only Texture Node.
typedef void (*NXT_DebugPrintFunc)(NXT_NodeHeader *node);
typedef void (*NXT_RenderNode_BeginFunc)(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
typedef int (*NXT_RenderNode_PrecacheFunc)(NXT_NodeHeader *node, NXT_HThemeRenderer renderer, NXT_PrecacheMode precacheMode, int* asyncmode, int* max_replaceable);
typedef int (*NXT_RenderNode_GetPrecacheResourceFunc)(NXT_NodeHeader* node, NXT_HThemeRenderer renderer, void* load_resource_list);
typedef NXT_NodeAction (*NXT_RenderNode_EndFunc)(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
typedef NXT_NodeHeader* (*NXT_NextNodeFunc)(NXT_Iter *pIter);

typedef struct NXT_Node_CoordinateSpace_ NXT_Node_CoordinateSpace;
struct NXT_AppliedEffect;

typedef struct NXT_UpdateTarget_ {
    float           *targetVector;
    int             numElements;
    int             elementOffset;
    float           factor;
    float           bias;
    int             numParams;
    float           params[NXT_UPDATETARGET_MAXPARAMS];
    NXT_NodeHeader  *notifyNode;
    void*           notifyPrivateData;
} NXT_UpdateTarget;

typedef struct NXT_KeyFrame_ {
    float       e[4];
    float       t;
    float       timing[4];
    int         framefit;
} NXT_KeyFrame;

typedef enum NXT_StartAt_ {
    NXT_StartAt_Root                                = 1,
    NXT_StartAt_Root_IncludeCurrentTheme            = 2,
    NXT_StartAt_Root_IncludeCurrentThemeAndEffect   = 3,
    NXT_StartAt_Root_IncludeAllThemes               = 4,
    NXT_StartAt_Theme                               = 5,
    NXT_StartAt_Theme_IncludeCurrentEffect          = 6,
    NXT_StartAt_Theme_IncludeAllEffects             = 7,
    NXT_StartAt_Effect                              = 8,
    NXT_StartAt_CurrentNode                         = 9,
    NXT_StartAt_Root_IncludeEverything              = 10
} NXT_StartAt;

struct NXT_Iter_ {
    NXT_NextNodeFunc    next;
    NXT_NodeHeader      *nextNode;
    NXT_NodeHeader      *startNode;
    NXT_NodeHeader      *rootNode;
    NXT_NodeHeader      *themeNode;
    NXT_NodeHeader      *effectNode;
    NXT_NodeClass       *findClass;
    NXT_StartAt         startAt;
    int                 depth;
};

typedef enum NXT_TextureType_ {
    NXT_TextureType_None        = 0,
    NXT_TextureType_File        = 1,
    NXT_TextureType_Text        = 2,
    NXT_TextureType_Video       = 3,
    NXT_TextureType_Overlay     = 4
} NXT_TextureType;

typedef enum NXT_SeqType_ {
    NXT_SeqType_Interp      = 0,        // Interpolate start->end
    NXT_SeqType_Rand        = 1,        // Random interpolated points between start and end
    NXT_SeqType_RandMix     = 2,        // Random mix of components of start and end
    NXT_SeqType_VRand       = 3,        // Random interpolated points between start and end, but varied by clip index
    NXT_SeqType_VRandMix    = 4,        // Random mix of components of start and end, but varied by clip index
    NXT_SeqType_Exp         = 5
} NXT_SeqType;

typedef enum NXT_RepeatType_ {
    NXT_RepeatType_None     = 0,
    NXT_RepeatType_Repeat   = 1,
    NXT_RepeatType_Sawtooth = 2,
    NXT_RepeatType_Sine     = 3
} NXT_RepeatType;

typedef enum NXT_CoordType_ {
    NXT_CoordType_None = 0,
    NXT_CoordType_Vertex = 1,
    NXT_CoordType_Texture = 2,
    NXT_CoordType_Mask = 3
} NXT_CoordType;

typedef enum NXT_ClipType_ {
    NXT_ClipType_All = 0,
    NXT_ClipType_First = 1,
    NXT_ClipType_Last = 2,
    NXT_ClipType_Middle = 3,
    NXT_ClipType_Even = 4,
    NXT_ClipType_Odd = 5,
    NXT_ClipType_NotFirst = 6,
    NXT_ClipType_NotLast = 7,
    NXT_ClipType_NotMiddle = 8,
    NXT_ClipType_NotEven = 9,
    NXT_ClipType_NotOdd = 10
} NXT_ClipType;

typedef enum NXT_NoiseType_ {
    NXT_NoiseType_None          = 0,
    NXT_NoiseType_PerlinSimplex = 1
} NXT_NoiseType;

typedef enum NXT_NoiseFunction_ {
    NXT_NoiseFunction_Pure          = 0,    // noise(p)
    NXT_NoiseFunction_Fractal       = 1,    // noise(p) + (1/2)noise(2p) + (1/4)noise(4p) ...
    NXT_NoiseFunction_Turbulence    = 2,    // abs(noise(p)) + abs((1/2)noise(2p)) + abs((1/4)noise(4p)) ...
    NXT_NoiseFunction_WaveFractal   = 3     // sin( n + abs(noise(p)) + abs((1/2)noise(2p)) + abs((1/4)noise(4p)) ... )
} NXT_NoiseFunction;

typedef enum NXT_Function_ {
    NXT_Function_SimplexPure            = 0,
    NXT_Function_SimplexFractal         = 1,
    NXT_Function_SimplexTurbulence      = 2,
    NXT_Function_SimplexWaveFractal     = 3
} NXT_Function;


typedef struct NXT_KeyFrameSet_ {
    NXT_KeyFrame        *keyframes;
    int                 numKeyFrames;    
    NXT_PartType        partType;
    NXT_ClipType        clipType;
    unsigned int        bDefault;
    unsigned int        framefit;
} NXT_KeyFrameSet;

typedef enum NXT_CullFace_ {
    NXT_CullFace_None = 0,
    NXT_CullFace_Back = 1,
    NXT_CullFace_Front = 2,
    NXT_CullFace_All = 3
} NXT_CullFace;

typedef enum NXT_ProjectionType_ {
    NXT_ProjectionType_NONE         = 0,
    NXT_ProjectionType_Ortho        = 1,
    NXT_ProjectionType_Frustum      = 2,
    NXT_ProjectionType_Perspective  = 3,
    NXT_ProjectionType_TitleBottom  = 4,
    NXT_ProjectionType_TitleTop     = 5,
    NXT_ProjectionType_TitleLeft    = 6,
    NXT_ProjectionType_TitleRight   = 7,
    NXT_ProjectionType_TitleMiddle  = 8
} NXT_ProjectionType;

typedef enum NXT_TransformTarget_ {
    NXT_TransformTarget_Vertex        = 0,
    NXT_TransformTarget_Texture       = 1,
    NXT_TransformTarget_Mask          = 2,
} NXT_TransformTarget;

typedef enum NXT_InterpBasis_ {
    NXT_InterpBasis_Time              = 0,
    NXT_InterpBasis_ClipIndex         = 1,
    NXT_InterpBasis_ClipIndexPermuted = 2
} NXT_InterpBasis;

typedef enum NXT_Anchor_ {
    NXT_Anchor_None             = 0,
    NXT_Anchor_TopLeft          = 1,
    NXT_Anchor_TopCenter        = 2,
    NXT_Anchor_TopRight         = 3,
    NXT_Anchor_RightCenter      = 4,
    NXT_Anchor_BottomRight      = 5,
    NXT_Anchor_BottomCenter     = 6,
    NXT_Anchor_BottomLeft       = 7,
    NXT_Anchor_LeftCenter       = 8,
    NXT_Anchor_Center           = 9
} NXT_Anchor;

typedef enum NXT_ScaleMode_ {
    NXT_ScaleMode_Stretch       = 0,
    NXT_ScaleMode_Fit           = 1,
    NXT_ScaleMode_Fill          = 2,
    NXT_ScaleMode_FitMask       = 3,
    NXT_ScaleMode_FillMask      = 4
} NXT_ScaleMode;

typedef enum NXT_RenderDest_ {
    NXT_RenderDest_Screen       = 0,
    NXT_RenderDest_Stencil      = 1,
    NXT_RenderDest_Mask         = 2,
} NXT_RenderDest;

typedef enum NXT_CompMode_ {
    NXT_CompMode_Normal         = 0,
    NXT_CompMode_Multiply       = 1,
    NXT_CompMode_Screen         = 2,
    NXT_CompMode_Diff           = 3
} NXT_CompMode;

typedef enum NXT_LightingNormals_ {
    NXT_LightingNormals_Front    = 0,
    NXT_LightingNormals_Back		= 1,
    NXT_LightingNormals_Both		= 2
} NXT_LightingNormals;

typedef enum NXT_MaskSampling_ {
    NXT_MaskSampling_Normal      = 0,
    NXT_MaskSampling_Invert      = 1
} NXT_MaskSampling;

typedef enum NXT_ShadingMode_ {
    NXT_ShadingMode_Off          = 0,
    NXT_ShadingMode_Flat         = 1,
    NXT_ShadingMode_FlatFwd      = 2
} NXT_ShadingMode;

typedef enum NXT_AnchorMode_ {
    NXT_AnchorMode_Camera        = 0,
    NXT_AnchorMode_World         = 1
} NXT_AnchorMode;

typedef enum NXT_RenderTest_ {
    NXT_RenderTest_None         = 0,
    NXT_RenderTest_Stencil      = 1,
    NXT_RenderTest_NegStencil   = 2,
    NXT_RenderTest_Mask         = 3,
    NXT_RenderTest_NegMask      = 4,
} NXT_RenderTest;

typedef enum NXT_TextureWrap_ {
    NXT_TextureWrap_None            = 0,
    NXT_TextureWrap_MirrorRepeat    = 1,
    NXT_TextureWrap_Repeat          = 2
} NXT_TextureWrap;

#define NXT_StyleField_Color            0x00000001
#define NXT_StyleField_Alpha            0x00000002
#define NXT_StyleField_BlendFuncSFactor 0x00000004
#define NXT_StyleField_BlendFuncDFactor 0x00000008
#define NXT_StyleField_Shader           0x00000010
#define NXT_StyleField_Texture          0x00000020
#define NXT_StyleField_RenderDest       0x00000040
#define NXT_StyleField_RenderTest       0x00000080
#define NXT_StyleField_MaskSample       0x00000100
#define NXT_StyleField_AnimFrame        0x00000200
#define NXT_StyleField_Brightness       0x00000400
#define NXT_StyleField_Contrast         0x00000800
#define NXT_StyleField_Saturation       0x00001000
#define NXT_StyleField_Tint             0x00002000
#define NXT_StyleField_Antialias		0x00004000
#define NXT_StyleField_DepthTest		0x00008000
#define NXT_StyleField_DepthMask		0x00010000
#define NXT_StyleField_ShadingMode		0x00020000
#define NXT_StyleField_AmbientColor		0x00040000
#define NXT_StyleField_DiffuseColor		0x00080000
#define NXT_StyleField_SpecularColor	0x00100000
#define NXT_StyleField_LightDirection	0x00200000
#define NXT_StyleField_LightAnchor      0x00400000
#define NXT_StyleField_LightingNormals  0x00800000
#define NXT_StyleField_SpecExp		    0x01000000
#define NXT_StyleField_CompMode		    0x02000000

#define NXT_PointComponent_Location     0x01
#define NXT_PointComponent_TexCoord     0x02
#define NXT_PointComponent_Color        0x04
#define NXT_PointComponent_MaskCoord    0x08
#define NXT_PointComponent_Normal	    0x10

#define NXT_VectorComponent_X           0x00000001
#define NXT_VectorComponent_Y           0x00000002
#define NXT_VectorComponent_Z           0x00000004
#define NXT_VectorComponent_W           0x00000008
#define NXT_VectorComponent_Uniform     0x00001000

/**
 * Describes a texture.
 *
 * The texture width and height may be larger than the 
 * source width and height in order to satisfy the
 * power-of-two size rule for OpenGL textures.
 *
 * For RGB, RBGA and Luminance textures, only texName[0]
 * is used.  For YUV textures, three separate textures
 * must actually be created, so texName[1] and texName[2]
 * will also be used.
 */

struct NXT_RenderTarget;

typedef struct FilteredResource_{

    unsigned int serial_;//check for matching
    NXT_RenderTarget* prender_target_;
}FilteredResource;

typedef struct NXT_TextureInfo_ {
    unsigned int    base_track_id_;
    NXT_PixelFormat origin_textureformat_;
    int color_transfer_function_id_;
    float max_cll_;
    unsigned int    track_id_;
    unsigned int    track_update_id_;
    unsigned int    bValidTexture;          // 1 if this is a valid texture; 0 if not
    NXT_PixelFormat textureFormat;          // Format of texture
    unsigned int    serial_;
    unsigned int    texNameInitCount;       // The number of initialized textures
    GLuint          texName[3];             // OpenGL texture names
	GLuint          texName_for_rgb;             // OpenGL texture names
    NXT_RenderTarget* prendertarget_;
    FilteredResource filtered_;
    
    // Texture wrapping modes for S and T axis
    NXT_TextureWrap texWrapS;
    NXT_TextureWrap texWrapT;
    
    // Texture animation
    unsigned int    bAnimated;
    unsigned int    animRows;
    unsigned int    animCols;
    unsigned int    animFrames;
    float           animfps;
    
    // OpenGL
#ifdef NEX_ENABLE_GLES_IMAGE
    void*			pEGLImage;
    unsigned int    bUseNativeBuffer;
    EGLImageKHR     eglImageID;
#endif
    unsigned int    bUseSurfaceTexture;
    NXT_Matrix4f    surfaceMatrix;

#ifdef __APPLE__
    int             bTextureSetByPlatform;
#endif
    // Input rectangle within the texture
    // Must satisfy the constraints:
    //    srcWidth > right > left >= 0
    //    srcHeight > top > bottom >= 0
    float           left;
    float           bottom;
    float           right;
    float           top;
    unsigned int    bHasInputRect;
    
    // Fit-in frame for the texture
    unsigned int    fitInWidth;              
    unsigned int    fitInHeight;
    
    // Color adjustments
    float           brightness;
    float           contrast;
    float           saturation;
    float           hue;
    unsigned int    tintColor;
    int             vignette;
    int             lut;
    int             custom_lut_a;
    int             custom_lut_b;
    int             custom_lut_power;
    
    // Rotation before fitting to 720x1280 frame
    unsigned int    rotation;               // Must be a multiple of 90 degrees
    unsigned int    user_rotation;

    //user translation

    int translate_x;
    int translate_y;
    
    // Horizontal and vertical mirroring (applied before rotation)
    unsigned int    mirror_h;               // 1 = mirror horizontally; 0 = don't mirror
    unsigned int    mirror_v;               // 1 = mirror vertically; 0 = don't mirror
    
    // These dimensions apply to the Y/Luminance/RGB/RGBA texture
    // represented by texName[0]
    unsigned int    srcWidth;               // Width of source data (in pixels)
    unsigned int    srcHeight;              // Height of source data (in pixels)
    unsigned int    textureWidth;           // Width of texture (in pixels)
    unsigned int    textureHeight;          // Height of texture (in pixels)
    
    // These dimensions apply to the U and V textures
    // represented by texName[1] and texName[2]
    unsigned int    srcUVWidth;             // Width of source data (in pixels)
    unsigned int    srcUVHeight;            // Height of source data (in pixels)
    unsigned int    textureUVWidth;         // Width of texture (in pixels)
    unsigned int    textureUVHeight;        // Height of texture (in pixels)
    //For support 360Video
    unsigned int    is360video;             //1 for 360 and 0 for normal
    float moveX_value;
    float moveY_value;
} NXT_TextureInfo;

typedef struct NXT_TextStyle_ {
    char*               typeface;
    char*               default_typeface;
    float               textSkewX;
    float               textScaleX;
    float               textSize;
    float               textStrokeWidth;
    float               spacingMult;
    float               spacingAdd;
    unsigned int        textFlags;      // Any combination of NXT_TEXTFLAG_?????
    float               textAlign;      // NXT_ALIGN_LEFT, NXT_ALIGN_CENTER, NXT_ALIGN_RIGHT
    float               textVAlign;      // NXT_VALIGN_TOP, NXT_VALIGN_CENTER, NXT_VALIGN_BOTTOM
    unsigned int        longText;       // NXT_LONGTEXT_WRAP, NXT_LONGTEXT_CROP_END, NXT_LONGTEXT_ELLIPSIZE_START, NXT_LONGTEXT_ELLIPSIZE_MID, NXT_LONGTEXT_ELLIPSIZE_END
    unsigned int		blurType;      // NXT_BLUR_NORMAL, NXT_BLUR_SOLID, NXT_BLUR_INNER, NXT_BLUR_OUTER
    unsigned int		shadowblurType;      // NXT_BLUR_NORMAL, NXT_BLUR_SOLID, NXT_BLUR_INNER, NXT_BLUR_OUTER
    unsigned int        maxLines;
    NXT_Vector4f        fillColor;
    NXT_Vector4f        strokeColor;
    NXT_Vector4f        shadowColor;
    NXT_Vector4f        bgColor;
    float               shadowRadius;
    float               textBlurRadius;
    float               textMargin;
    NXT_Vector4f        shadowOffset;
    int                 reqWidth;
    int                 reqHeight;
} NXT_TextStyle;

// ---- Node Class Structure ---------------------------------------------------

struct NXT_NodeClass_ {
    
    NXT_NodeInitFunc            initNodeFunc;
    NXT_NodeFreeFunc            freeNodeFunc;
    NXT_NodeSetAttrFunc         setAttrFunc;
    NXT_NodeChildrenDoneFunc    childrenDoneFunc;
    NXT_DebugPrintFunc          debugPrintFunc;
    NXT_RenderNode_BeginFunc    beginRenderFunc;
    NXT_RenderNode_EndFunc      endRenderFunc;
    NXT_RenderNode_PrecacheFunc precacheFunc;
    NXT_RenderNode_GetPrecacheResourceFunc getPrecacheResourceFunc;
    NXT_NodeUpdateNotify        updateNotifyFunc;
    NXT_NodeFreeTextureFunc     freeTextureFunc;
    const char*                 className;
    int                         size;
    
};

// ---- Node Classes -----------------------------------------------------------

extern NXT_NodeClass NXT_NodeClass_AnimatedValue;
extern NXT_NodeClass NXT_NodeClass_Function;
extern NXT_NodeClass NXT_NodeClass_UserField;
extern NXT_NodeClass NXT_NodeClass_Parameter;
extern NXT_NodeClass NXT_NodeClass_Texture;
extern NXT_NodeClass NXT_NodeClass_TimingFunc;
extern NXT_NodeClass NXT_NodeClass_KeyFrame;
extern NXT_NodeClass NXT_NodeClass_Effect;
extern NXT_NodeClass NXT_NodeClass_FieldLabel;
extern NXT_NodeClass NXT_NodeClass_Option;
extern NXT_NodeClass NXT_NodeClass_String;
extern NXT_NodeClass NXT_NodeClass_Visibility;
extern NXT_NodeClass NXT_NodeClass_Cull;
extern NXT_NodeClass NXT_NodeClass_CoordinateSpace;
extern NXT_NodeClass NXT_NodeClass_TriangleStrip;
extern NXT_NodeClass NXT_NodeClass_Polygon;
extern NXT_NodeClass NXT_NodeClass_Rotate;
extern NXT_NodeClass NXT_NodeClass_Style;
extern NXT_NodeClass NXT_NodeClass_Theme;
extern NXT_NodeClass NXT_NodeClass_ThemeSet;
extern NXT_NodeClass NXT_NodeClass_Translate;
extern NXT_NodeClass NXT_NodeClass_Point;
extern NXT_NodeClass NXT_NodeClass_Const;
extern NXT_NodeClass NXT_NodeClass_Scale;
extern NXT_NodeClass NXT_NodeClass_Clear;
extern NXT_NodeClass NXT_NodeClass_Circle;
extern NXT_NodeClass NXT_NodeClass_Rect;
extern NXT_NodeClass NXT_NodeClass_Part;
extern NXT_NodeClass NXT_NodeClass_Projection;
extern NXT_NodeClass NXT_NodeClass_Sequence;
extern NXT_NodeClass NXT_NodeClass_RenderItem;
extern NXT_NodeClass NXT_NodeClass_KEDL;
extern NXT_NodeClass NXT_NodeClass_EffectQueue;
extern NXT_NodeClass NXT_NodeClass_EffectItem;


// ---- Node Header ------------------------------------------------------------

struct NXT_NodeHeader_ {
    NXT_NodeClass   *isa;
    NXT_NodeHeader  *child;
    NXT_NodeHeader  *parent;
    NXT_NodeHeader  *next;
    char*           node_id;
};

// ---- Nodes ------------------------------------------------------------------

typedef struct NXT_Node_Theme_ {
    NXT_NodeHeader      header;
    char                *name;
    char                *thumbnailPath;
} NXT_Node_Theme;

typedef struct NXT_Node_ThemeSet_ {
    NXT_NodeHeader      header;
    char                *name;
    char                *defaultTheme;
    char                *defaultTransition;
    char                *defaultTitle;
} NXT_Node_ThemeSet;

typedef struct NXT_Node_Function_ {
    NXT_NodeHeader      header;
    
    NXT_UpdateTarget    *updateTargets;
    int                 numUpdateTargets;
    int                 allocUpdateTargets;
    
    NXT_Function        function;
    
    NXT_Vector4f        inputBias;
    NXT_Vector4f        inputScale;
    
    NXT_Vector4f        outputBias;
    NXT_Vector4f        outputScale;
    
    int                 octaves;
    
    unsigned int        bNormalize;
} NXT_Node_Function;

typedef struct NXT_Node_UserField_ {
    NXT_NodeHeader      header;
    
    NXT_UserFieldType   type;
    int                 element_count;
    NXT_UpdateTarget    *updateTargets;
    int                 numUpdateTargets;
    int                 allocUpdateTargets;
    char*               pDefault;
} NXT_Node_UserField;

typedef struct NXT_Node_AnimatedValue_ {
    NXT_NodeHeader      header;
    NXT_NodeHeader      *altChildNode;
    NXT_UpdateTarget    *updateTargets;
    int                 numUpdateTargets;
    int                 allocUpdateTargets;
    NXT_KeyFrameSet     *keyframeSet;
    int                 numKeyframeSets;
    int                 element_count;
    float               timing[4];
    unsigned int        bUseOverallTime;
    NXT_NoiseType       noiseType;
    NXT_NoiseFunction   noiseFunc;
    float               noiseBias;
    float               noiseScale;
    float               noiseSampleBias;
    float               noiseSampleScale;
    float               startTime;
    float               endTime;
    int                 noiseOctaves;
    NXT_InterpBasis     basis;
    int                 framefit;
} NXT_Node_AnimatedValue;

typedef struct NXT_Node_Sequence_ {
    NXT_NodeHeader      header;
    NXT_UpdateTarget    *updateTargets;
    int                 numUpdateTargets;
    int                 allocUpdateTargets;
    int                 count;
    NXT_Vector4f        start;
    NXT_Vector4f        end;
    int                 seed1;
    int                 seed2;
    NXT_SeqType         type;
    int                 repeats_done;
    int                 element_count;
    NXT_RandSeed        cur_seed;
} NXT_Node_Sequence;


#define NXT_TEXTFLAG_BOLD       0x00000001
#define NXT_TEXTFLAG_ITALIC     0x00000002
#define NXT_TEXTFLAG_FILL       0x00000004
#define NXT_TEXTFLAG_STROKE     0x00000008
#define NXT_TEXTFLAG_UNDERLINE  0x00000010
#define NXT_TEXTFLAG_STRIKE     0x00000020
#define NXT_TEXTFLAG_HINTING    0x00000040
#define NXT_TEXTFLAG_SUBPIXEL   0x00000080
#define NXT_TEXTFLAG_SHADOW     0x00000100
#define NXT_TEXTFLAG_LINEAR     0x00000200
#define NXT_TEXTFLAG_AUTOSIZE   0x00000400
#define NXT_TEXTFLAG_CUTOUT     0x00000800
#define NXT_TEXTFLAG_STROKEBACK 0x00001000

#define NXT_ALIGN_LEFT          0x00
#define NXT_ALIGN_CENTER        0x01
#define NXT_ALIGN_RIGHT         0x02
#define NXT_VALIGN_TOP          0x00
#define NXT_VALIGN_CENTER       0x10
#define NXT_VALIGN_BOTTOM       0x20

#define NXT_LONGTEXT_CROP_END           0
#define NXT_LONGTEXT_WRAP               1
#define NXT_LONGTEXT_ELLIPSIZE_START    2
#define NXT_LONGTEXT_ELLIPSIZE_MIDDLE   3
#define NXT_LONGTEXT_ELLIPSIZE_END      4

#define NXT_BLUR_NORMAL					0
#define NXT_BLUR_SOLID					1
#define NXT_BLUR_INNER					2
#define NXT_BLUR_OUTER					3

typedef enum NXT_Renderitem_KeyValueType_{
    NXT_Renderitem_KeyValueType_Color,
    NXT_Renderitem_KeyValueType_Selection,
    NXT_Renderitem_KeyValueType_Choice,
    NXT_Renderitem_KeyValueType_Range,
    NXT_Renderitem_KeyValueType_Rect,
    NXT_Renderitem_KeyValueType_Point
} NXT_Renderitem_KeyValueType;

#include <string>
#include <vector>
#include <map>

typedef struct NXT_RenderItem_KeyValue_{

    int type_;
    std::string key_;
    std::vector<float> value_;

    bool operator!=(const NXT_RenderItem_KeyValue_& cmp){

        if(value_.size() != cmp.value_.size())
            return false;
        int value_count = value_.size();
        for(int i = 0; i < value_count; ++i){

            if(value_[i] != cmp.value_[i])
                return false;
        }

        return true;
    }
} NXT_RenderItem_KeyValue;

struct NXT_RenderItem_KeyValues{

    typedef std::vector<NXT_RenderItem_KeyValue*> list_t;
    typedef list_t::iterator iter_t;
    list_t items_;

    NXT_RenderItem_KeyValues();

    ~NXT_RenderItem_KeyValues();

    void clear();
};


typedef struct NXT_Node_RenderItem_ {
    NXT_NodeHeader      header;
    int                 id;
    void*               imageCallbackPvtData;
    char*               src;                   // Path for file texture
    char*               path;
    char*               uid;
} NXT_Node_RenderItem;

typedef struct NXT_Node_KEDL_ NXT_Node_KEDL;

typedef struct NXT_Node_Texture_ {
    NXT_NodeHeader      header;
    NXT_TextureType     textureType;
    NXT_ImageInfo       imgInfo;
    void*               imageCallbackPvtData;
    unsigned int        bLoadedImage;
    unsigned int        bCachedImage;
    unsigned int        videoSource;  // 0=none, 1=ending (or sole), 2=starting
    char*               path;                   // Path for file texture
    char*               key;                    // key for texture manager
    char*               srcfld;                 // Source field
    int                 index;                  // Index for video or text texture
    NXT_TextureInfo     texinfo_preview;

    char*               text;
    NXT_TextStyle       textStyle;
    int                 textSerial;     // Title text serial number (to detect changes in the title text and invalidate cache)
    unsigned int        bNoCache;
    unsigned int        bMipmap;

    int check_for_src_update_;
    void* prender_target_;
    NXT_Node_RenderItem *renderitem;
    NXT_AppliedEffect   *kedlstateblock;
    NXT_Node_KEDL* kedlitem;
    NXT_RenderItem_KeyValues renderitem_parameter_values_;
    NXT_RenderItem_KeyValues renderitem_parameter_pre_values_;
    
} NXT_Node_Texture;

typedef struct NXT_Node_TimingFunc_ {
    NXT_NodeHeader      header;
    float               e[4];
} NXT_Node_TimingFunc;

typedef struct NXT_Node_KeyFrame_ {
    NXT_NodeHeader      header;
    float   t;
    float   e[4];
    int     element_count;
    float   timing[4];
    int     framefit;
} NXT_Node_KeyFrame;

#define NXT_EFFECT_FLAG_SETOFFSET   0x00000001
#define NXT_EFFECT_FLAG_SETOVERLAP  0x00000002

typedef struct NXT_Node_Effect_ {
    NXT_NodeHeader      header;
    NXT_EffectType      effectType;
    char*               name;
    char*               categoryTitle;
    char*               icon;
    char*               encodedName;
    int                 effectOffset;
    int                 videoOverlap;
    int                 minDuration;
    int                 maxDuration;
    int                 defaultDuration;
    int                 inTime;
    int                 outTime;
    int                 inTimeFirst;
    int                 outTimeLast;
    int                 cycleTime;
    int                 maxTitleDuration;
    unsigned int        bUserDuration;
    NXT_RepeatType      repeatType;
    unsigned int        effectFlags;
} NXT_Node_Effect;

typedef struct NXT_Node_KEDL_ {
    NXT_NodeHeader      header;
    NXT_HThemeSet       themeset;
    NXT_HEffect         effect;
    unsigned int        total_video_slot;
    void*               imageCallbackPvtData;
    char*               uid;
} NXT_Node_KEDL;

typedef struct NXT_Node_Visibility_ {
    NXT_NodeHeader      header;
    float               startTime;
    float               endTime;
    float               check;
	unsigned int		bUseCheckValue;
} NXT_Node_Visibility;

typedef struct NXT_Node_Cull_ {
    NXT_NodeHeader      header;
    NXT_CullFace        cullFace;
    NXT_CullFace        prevCullFace;
} NXT_Node_Cull;

typedef struct NXT_Node_TriangleStrip_ {
    NXT_NodeHeader      header;
    NXT_Node_Texture    *texture;
    NXT_Node_Texture    *mask;
    int                 mapping;
    float               scale;
    int                 numPoints;
    unsigned char       *bComponentSet;         // OR of NXT_PointComponent_?????
    float               *vertexCoord;
    float               *adjVertexCoord;
    float               *vertexColor;
    float               *vertexNormal;
    float               *textureCoord;
    float               *adjTextureCoord;
    float               *maskCoord;
    float               *adjMaskCoord;
    void*				*buffer;
    GLenum              type;
    NXT_Node_CoordinateSpace *vertexSpace;
    NXT_Node_CoordinateSpace *maskSpace;
    NXT_Node_CoordinateSpace *textureSpace;
} NXT_Node_TriangleStrip;

typedef struct NXT_Node_Polygon_ {
    NXT_NodeHeader      header;
    int                 numPolyPoints;
    float               *polyPoints;
    unsigned int        bNoCache;
    unsigned int        bSetColor;
    int                 vertexCapacity;
    int                 vertexCount;
    float               *vertexCoord;
    float               *vertexColor;
    NXT_Vector4f        polyColor;
    
    
    //NXT_Node_Texture    *texture;
    //NXT_Node_Texture    *mask;
} NXT_Node_Polygon;

typedef struct NXT_Node_Rotate_ {
    NXT_NodeHeader      header;
    float               angle;
    NXT_Vector4f        axis;
    NXT_Matrix4f        saved_transform;
    NXT_TransformTarget target;
} NXT_Node_Rotate;

typedef struct NXT_Node_Style_ {
    NXT_NodeHeader      header;
    
    // Flags indicating which style fields are set 
    unsigned int        fieldset;       // OR of NXT_StyleField_???
    
    // Values for fields of this node
    NXT_Node_Texture    *texture;
    NXT_Vector4f        color;
    NXT_Vector4f        ambientColor;
    NXT_Vector4f        diffuseColor;
    NXT_Vector4f        specularColor;
    NXT_Vector4f        lightDirection;
    float               alpha;
    float               animframe;
    unsigned int        blendFuncSFactor;
    unsigned int        blendFuncDFactor;
    unsigned int        shader;
    NXT_CompMode        compMode;
    NXT_RenderDest      renderDest;
    NXT_RenderTest      renderTest;
    NXT_MaskSampling    maskSampling;
    NXT_ShadingMode     shadingMode;
    NXT_AnchorMode      lightAnchor;
    float				specexp;
    float               brightness;
    float               contrast;
    float               saturation;
    NXT_Vector4f        tintColor;
	unsigned char		antialias;
	unsigned char		depthtest;
	unsigned char		depthmask;
    NXT_LightingNormals lightingNormals;

    // Previous values (used to restore old values when traversing the tree)
    NXT_Vector4f        saved_color;
    NXT_Matrix4f        saved_mask_sample_matrix;
    NXT_Vector4f        saved_ambientColor;
    NXT_Vector4f        saved_diffuseColor;
    NXT_Vector4f        saved_specularColor;
    NXT_Vector4f        saved_lightDirection;
    NXT_ShadingMode     saved_shadingMode;
    float               saved_alpha;
    float               saved_animframe;
    NXT_RenderDest      saved_renderDest;
    NXT_RenderTest      saved_renderTest;
    NXT_CompMode        saved_compMode;
    float               saved_brightness;
    float               saved_contrast;
    float               saved_saturation;
    float				saved_specexp;
    NXT_Vector4f        saved_tintColor;
	unsigned char		saved_antialias;
	unsigned char		saved_depthtest;
	unsigned char		saved_depthmask;
    NXT_LightingNormals saved_lightingNormals;
    
} NXT_Node_Style;

typedef struct NXT_Node_Translate_ {
    NXT_NodeHeader      header;
    NXT_Vector4f        vector;
    NXT_Vector4f        texoffs;
    NXT_Vector4f        maskoffs;
    int                 repeat;
    int                 repeats_done;
    unsigned int        bJitter;
    unsigned int        jitterSeed;
    
    unsigned int        bJitterAlpha;
    float               jitter_alpha;
    
    // Previous values (used to restore old values when traversing the tree)
    float               saved_alpha;
    NXT_Matrix4f        saved_transform;
    NXT_Matrix4f        saved_tex_transform;
    NXT_Matrix4f        saved_mask_transform;
    
} NXT_Node_Translate;

typedef struct NXT_Node_Scale_ {
    NXT_NodeHeader      header;
    unsigned int        components; // OR of NXT_VectorComponent_???
    NXT_Vector4f        factor;
    NXT_Matrix4f        saved_transform;
    NXT_TransformTarget target;
    unsigned int        bInverse;
} NXT_Node_Scale;

typedef struct NXT_Node_Clear_ {
    NXT_NodeHeader      header;
    NXT_Vector4f        color;
    unsigned int        bStencil;
} NXT_Node_Clear;

typedef struct NXT_Node_Rect_ {
    NXT_NodeHeader      header;
    NXT_AppliedEffect   *kedlstateblock;
    NXT_Node_KEDL       *kedlitem;
    NXT_Node_RenderItem *renderitem;
    NXT_RenderItem_KeyValues renderitem_parameter_values_;
    NXT_Node_Texture    *texture;
    NXT_Node_Texture    *mask;
    NXT_Vector4f        bounds;
    NXT_Vector4f        color;
    unsigned int        bColorSet;
    
    NXT_Vector4f        location;
    float               width;
    float               height;
    NXT_Anchor          anchor;
    NXT_ScaleMode       scaleMode;
} NXT_Node_Rect;

typedef struct NXT_Node_Circle_ {
    NXT_NodeHeader      header;
    NXT_Vector4f        color;
    NXT_Vector4f        inner_color;
    unsigned int        bColorSet;
    unsigned int        bInnerColorSet;
    NXT_Vector4f        center;
    float               r;
    float               inner_r;
	int					segments;
} NXT_Node_Circle;

typedef struct NXT_Node_Const_ {
    NXT_NodeHeader      header;
    char                *value;
} NXT_Node_Const;

typedef struct NXT_Node_FieldLabel_ {
    NXT_NodeHeader      header;
} NXT_Node_FieldLabel;

typedef struct NXT_Node_Option_ {
    NXT_NodeHeader      header;
} NXT_Node_Option;

struct NXT_Node_CoordinateSpace_ {
    NXT_NodeHeader      header;
    float               width;
    float               height;
    float               originx;
    float               originy;
    unsigned int        bYUp;
    NXT_CoordType       applyTo;
};

typedef struct NXT_Node_Point_ {
    NXT_NodeHeader      header;
    char                *location;
    char                *texcoord;
    char                *maskcoord;
    char                *color;
    char                *normal;
} NXT_Node_Point;

typedef struct NXT_Node_Part_ {
    NXT_NodeHeader      header;
    NXT_PartType        type;
    NXT_ClipType        clipType;
} NXT_Node_Part;

typedef struct NXT_Node_Projection_ {
    NXT_NodeHeader      header;
    NXT_ProjectionType  type;
    NXT_Matrix4f        saved_proj;
    float               left;
    float               right;
    float               top;
    float               bottom;
    float               near;
    float               far;
    float               fov;
    float               aspect;
} NXT_Node_Projection;

typedef struct NXT_Node_EffectItem_{

    NXT_NodeHeader      header;
    int                 effect_index_;
    int                 effect_count_;
    int                 effect_type;
    float               start_time;
    float               end_time;
    NXT_Node_KEDL*      kedlitem;
    NXT_AppliedEffect*  kedlstateblock;
}NXT_Node_EffectItem;


typedef struct NXT_Node_EffectQueue_ {

    NXT_NodeHeader  header;
} NXT_Node_EffectQueue;

// ---- API Functions ----------------------------------------------------------

NXT_NodeHeader *NXT_AllocateNode( NXT_NodeClass* nodeClass );
NXT_NodeHeader *NXT_RootForNode( NXT_NodeHeader *node );
void NXT_AddChildNode( NXT_NodeHeader *node, NXT_NodeHeader *newChild );
void NXT_AddSiblingNode( NXT_NodeHeader *node, NXT_NodeHeader *newSibling );
void NXT_FreeNode(NXT_HThemeRenderer_Context context, NXT_NodeHeader *node, unsigned int isDetachedContext);
void NXT_FreeLastSiblingNode( NXT_NodeHeader *node );
NXT_NodeClass *NXT_NodeClassByName( char* name );
NXT_NodeHeader *NXT_FindFirstNodeWithId( NXT_NodeHeader *node, const char *node_id, NXT_NodeClass *nodeClass );
NXT_NodeHeader *NXT_FindFirstNodeOfClass( NXT_NodeHeader *node, NXT_NodeClass *nodeClass );
NXT_NodeHeader *NXT_FindNextNodeOfClass( NXT_NodeHeader *node, NXT_NodeClass *nodeClass );
void NXT_SetNodeId( NXT_NodeHeader *node, const char *node_id );
void NXT_CompletedNodeChildren( NXT_NodeHeader *node );
void NXT_SetNodeAttr( NXT_NodeHeader *node, const char* attrName, const char* attrValue );
int NXT_AnimFloatVectorFromString( NXT_NodeHeader *node, const char* string, float* vector, int numElements );
void NXT_FreeRenderitemKeyValues(NXT_RenderItem_KeyValues* pkeyvalues);
int NXT_ProcParamKeyValueFromString(NXT_NodeHeader* node, const char* string, NXT_RenderItem_KeyValues* pkeyvalues);
int NXT_AnimFloatVectorFromStringEx( NXT_NodeHeader *node, const char* string, float* vector, int numElements, int *pNumUpdateTargets );
int NXT_AnimFloatVectorFromStringNotify( NXT_NodeHeader *node, char* string, float* vector, int numElements, int *pNumUpdateTargets, NXT_NodeHeader *notifyNode, void* notifyPrivateData );
int NXT_CountChildNotesOfClass( NXT_NodeHeader *node, NXT_NodeClass *nodeClass );
void NXT_DeleteChildNodes(NXT_HThemeRenderer_Context context, NXT_NodeHeader *node, unsigned int isDetachedContext );
int NXT_Node_AnimatedValue_GetElementCount( NXT_Node_AnimatedValue *animvalNode );
void NXT_Node_AnimatedValue_AddUpdateTarget(NXT_Node_AnimatedValue *animvalNode, 
                                            float *targetVector, int elementOffset, int numElements, 
                                            float factor, float bias, NXT_NodeHeader *notifyNode, void* notifyPrivateData );
int NXT_Node_Sequence_GetElementCount( NXT_Node_Sequence *seqNode );
void NXT_Node_Sequence_AddUpdateTarget( NXT_Node_Sequence *seqNode, float *targetVector, int elementOffset, int numElements, float factor, float bias, NXT_NodeHeader *notifyNode, void* notifyPrivateData );
int NXT_Node_Function_GetElementCount( NXT_Node_Function *funcNode );
void NXT_Node_Function_AddUpdateTarget( NXT_Node_Function *funcNode, float *targetVector, int elementOffset, int numElements, float factor, float bias, NXT_NodeHeader *paramsBaseNode, char* params );
int NXT_Node_UserField_GetElementCount( NXT_Node_UserField *userFieldNode );
void NXT_Node_UserField_AddUpdateTarget( NXT_Node_UserField *userFieldNode, float *targetVector, int elementOffset, int numElements, float factor, float bias, NXT_NodeHeader *paramsBaseNode, char* params );
void NXT_Node_Function_ProcessUpdateTargets( NXT_Node_Function *funcNode );
void NXT_Node_UserField_ProcessUpdateTargets( NXT_Node_UserField *userFieldNode, NXT_HThemeRenderer renderer );
unsigned int NXT_Node_ClassCheck( NXT_NodeHeader *node, NXT_NodeClass *nodeClass );
void NXT_TextStyle_Free( NXT_TextStyle *textStyle );
unsigned int NXT_TextStyle_Compare( NXT_TextStyle *textStyle1, NXT_TextStyle *textStyle2 );

NXT_Iter NXT_ForEachNodeOfClass( NXT_NodeHeader *startNode, NXT_NodeClass *nodeClass, NXT_StartAt startAt );

// -----------------------------------------------------------------------------

#endif //NEXTHEME_NODES_H
