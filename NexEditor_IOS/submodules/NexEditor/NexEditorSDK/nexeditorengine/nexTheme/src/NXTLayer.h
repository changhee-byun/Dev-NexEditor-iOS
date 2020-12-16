//
//  NXTLayer.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/22/14.
//
//

#ifndef __NexVideoEditor__NXTLayer__
#define __NexVideoEditor__NXTLayer__

#include <iostream>
#include "NXTImage.h"
#include "NXTImageSettings.h"
#include "ILayer.h"
#include "IRenderer.h"

class NXTLayer : public ILayer {
    
public:
    NXTLayer(const NXTLayer& other) = delete;
    NXTLayer& operator=( const NXTLayer& other) = delete;
    
    NXTLayer(int layer_id, LayerType layer_type, unsigned int start_cts, unsigned int end_cts) {
        start_cts_ = start_cts;
        end_cts_ = end_cts;
        layer_type_ = layer_type;
        layer_id_ = layer_id;
    }
    
    void setClipIndex(int index, int total_clip_count) {
        clip_index_ = index;
        total_clip_count_ = total_clip_count;
    }
    
    void setClipEffect(const std::string& effect_id,
                       const std::string& effect_options,
                       unsigned int start_cts,
                       unsigned int end_cts)
    {
        clip_effect_id_ = effect_id;
        effect_start_cts_ = start_cts;
        effect_end_cts_ = end_cts;
        // TODO: Effect options
    }
    
    NXTImage& getImage()
    {
        return image_;
    }
    
    NXTImageSettings& getImageSettings() {
        return img_settings_;
    }
    
    void set_input_cropping( float left, float bottom, float right, float top )
    {
        // TODO:  set cropping in img_settings_
    }
    
    void set_color_adjust(int brightness,
                          int contrast,
                          int saturation,
                          unsigned int tintcolor)
    {
        // TODO
    }
    void set_rotation(int rotation)
    {
        // TODO
    }
    void set_fit_in_frame(int width, int height)
    {
        // TODO
    }

    int getId() {
        return layer_id_;
    }
    
    LayerType getLayerType() {
        return layer_type_;
    }
    
public:
    std::string clip_effect_id_;
    unsigned int start_cts_;
    unsigned int end_cts_;
    unsigned int effect_start_cts_;
    unsigned int effect_end_cts_;
    int layer_id_;
    LayerType layer_type_;
    int clip_index_ = -1;
    int total_clip_count_ = -1;
    int z_order = 0;
    NXTImage image_;
    NXTImageSettings img_settings_;
};

#endif /* defined(__NexVideoEditor__NXTLayer__) */
