//
//  ILayer.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/25/14.
//
//

#ifndef NexVideoEditor_ILayer_h
#define NexVideoEditor_ILayer_h
#include "IRenderer.h"

class ILayer {
public:
    ILayer(const ILayer& other) = delete;
    ILayer& operator=( const ILayer& other) = delete;
    ILayer() = default;

    virtual ~ILayer(){};

    virtual void setClipIndex(int index, int total_clip_count) = 0;
    
    virtual void setClipEffect(const std::string& effect_id,
                       const std::string& effect_options,
                       unsigned int start_cts,
                       unsigned int end_cts) = 0;
    
    virtual NXTImage& getImage() = 0;
    
    virtual void set_input_cropping( float left, float bottom, float right, float top ) = 0;
    virtual void set_color_adjust(int brightness,
                          int contrast,
                          int saturation,
                          unsigned int tintcolor) = 0;
    virtual void set_rotation(int rotation) = 0;
    virtual void set_fit_in_frame(int width, int height) = 0;
    
    virtual int getId() = 0;
    virtual LayerType getLayerType() = 0;
};


#endif
