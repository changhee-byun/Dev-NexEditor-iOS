#pragma once

struct RendererState {

    NXT_Vector4f color;
    NXT_Matrix4f basetransform;
    NXT_Matrix4f transform;
    NXT_Matrix4f proj;
    NXT_Matrix4f texture_transform;
    NXT_Matrix4f mask_transform;
    NXT_Matrix4f mask_sample_transform;

    float alpha;
    float animframe;
    NXT_Node_Texture *texture;
    NXT_Node_Texture *mask;
    NXT_RenderDest renderDest;
    NXT_RenderTest renderTest;
    unsigned int bAntiAlias;
    unsigned int bDepthTest;
    unsigned int bDepthMask;
    NXT_CompMode  compMode;

    NXT_RandSeed seed;
    int cur_effect_overlap;

    NXT_AppliedEffect* pActiveEffect;
    NXT_EffectType effectType;
    int clip_index;
    int clip_count;
    
    unsigned int cur_time;
    unsigned int max_set_time;
    unsigned int max_time;
    unsigned int bSkipChildren; // Set to 1 in beginRenderFunc to skip children of current node
    unsigned int inTime;
    unsigned int outTime;
    unsigned int cycleTime;
    NXT_RepeatType repeatType;
    NXT_PartType part;
    
    float settime;
    float time;
    
    unsigned int view_width;
    unsigned int view_height;
    NXT_CullFace cullFace;
    
    int effectStartTime;
    int effectEndTime;
    
    float adj_brightness;
    float adj_contrast;
    float adj_saturation;    
    NXT_Vector4f adj_tintColor;
    
    NXT_Vector4f        ambientColor;
    NXT_Vector4f        diffuseColor;
    NXT_Vector4f        specularColor;
    NXT_Vector4f        lightDirection;
    float               specularExponent;
    NXT_ShadingMode     shadingMode;
    NXT_LightingNormals lightingNormals;

    void operator >>(NXT_HThemeRenderer renderer);
    void operator <<(NXT_HThemeRenderer renderer);
};