//
//  IRenderContext.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/28/14.
//
//

#ifndef NexVideoEditor_IRenderContext_h
#define NexVideoEditor_IRenderContext_h
#include "NXTImageSettings.h"
#include "NXTMatrix.h"

enum class NXTCullFace {
    None, Back, Front, All
};

enum class NXTDrawArraysMode {
    Points, LineStrip, LineLoop, Lines, TriangleStrip, TriangleFan, Triangles
};

enum class NXTTransformTarget {
    Vertex,Texture,Mask,MaskSample,Color
};

enum class NXTPart {
    All,In,Out,Mid
};

enum class NXTShadingMode {
    Off, Flat, FlatFwd
};


class NXTShader;
class NXTRenderStyle;
class NXTAppliedEffect;
class NXTRenderer;

class IRenderContext {
    
public:
    IRenderContext(const IRenderContext& other) = delete;
    IRenderContext& operator=( const IRenderContext& other) = delete;
    IRenderContext() = default;
    virtual ~IRenderContext(){};
    
    virtual void setTexture( const NXTImageAndSettings& texture_image ) = 0;
    virtual void setMask( const NXTImageAndSettings& texture_image ) = 0;
    virtual void clearTexture() = 0;
    virtual void clearMask() = 0;
    
    virtual bool isMaskActive() = 0;
    virtual bool isTextureActive() = 0;
    
    virtual void prepStateForRender() = 0;
    
    virtual NXTCullFace getCullFace() = 0;
    virtual void setCullFace( NXTCullFace cullFace ) = 0;
    virtual NXTVector getColor() = 0;
    virtual void setColor(const NXTVector& color) = 0;
    virtual NXTShader& getShader() = 0;
    virtual void drawArrays(NXTDrawArraysMode mode = NXTDrawArraysMode::TriangleStrip,
                            int first = 0,
                            int count = -1) = 0;
    
    virtual NXTMatrix& getTransformMatrix(NXTTransformTarget target = NXTTransformTarget::Vertex) = 0;
    virtual void skipChildren(bool skip=true) = 0;
    virtual bool isSkipChildren() = 0;
    
    virtual NXTPart getCurrentPart() = 0;
    virtual int getTotalClipCount() = 0;
    virtual int getCurrentClipIndex() = 0;
    virtual int getCurrentTime() = 0;
    virtual int getEffectStartTime() = 0;
    virtual int getEffectEndTime() = 0;
    virtual int getTotalProjectTime() = 0;
    virtual float getEffectProgressOverall() = 0;
    virtual float getEffectProgressInPart() = 0;
    

    virtual NXTMatrix& getProjectionMatrix() = 0;
    virtual int getViewWidth() = 0;
    virtual int getViewHeight() = 0;
    virtual NXTRenderStyle& getRenderStyle() = 0;
    virtual void resetColorMask() = 0;
    virtual void seed( int seed1, int seed2 ) = 0;
    virtual float randf() = 0;
    virtual NXTAppliedEffect& getActiveEffect() = 0;
    virtual bool isEffectActive() = 0;
    virtual NXTShadingMode getShadingMode() = 0;
    virtual void fillRect( float left, float bottom, float right, float top) = 0;

private:
    friend class NXTExternalPixelData;
    virtual NXTRenderer& getRenderer() = 0;

};


#endif
