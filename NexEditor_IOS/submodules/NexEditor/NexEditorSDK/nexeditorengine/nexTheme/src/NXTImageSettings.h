//
//  NXTImageSettings.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/24/14.
//
//

#ifndef __NexVideoEditor__NXTImageSettings__
#define __NexVideoEditor__NXTImageSettings__

#include <iostream>
#include "NXTImage.h"

class NXTImageSettings {
    
public:
    enum class Wrap {
        None, MirrorRepeat, Repeat
    };
    
    static Wrap wrapModeFromString(const std::string &s) {
        if( s == "none" ) {
            return Wrap::None;
        } else if( s == "repeat" ) {
            return Wrap::Repeat;
        } else if( s == "mirror" ) {
            return Wrap::MirrorRepeat;
        } else {
            return Wrap::None;
        }
    }
    
    // Texture wrapping
    Wrap wrapS = Wrap::None;
    Wrap wrapT = Wrap::None;

    // Texture animation
    bool   bAnimated = false;
    int    animRows = 0;
    int    animCols = 0;
    int    animFrames = 0;

};

enum class PixelSource {
    Image, Video, VideoNext
};

class NXTImageAndSettings {
public:
    // The 'source' field has two purposes; if there is pixel data in
    // the pImage field, this is simply a hint about where that data
    // came from (an image, video clip, etc.)
    //
    // If the pImage field is null, but the source field indicates
    // another source such as Video or VideoNext, this is used by
    // the renderer to fill in the pImage field appropriately based
    // on the requested source.
    //
    // For example, an NXTNodeTexture that has an associated image
    // texture will simply provide a NXTImageAndSettings data source
    // with Source::Image set and the appropriate image data in NXTImage,
    // but an NXTNodeTexture that needs video input will provide a
    // NXTImageAndSettings with pImage set to nullptrand source set
    // to either PixelSource::Video or PixelSource::VideoNext.  The
    // renderer will later fill in pImage based on the requested
    // source.
    NXTImageAndSettings() = default;
    NXTImageAndSettings(PixelSource source) {
        this->source = source;
    }
    NXTImageAndSettings(NXTImage* pImage, NXTImageSettings *pSettings = nullptr) {
        this->source = PixelSource::Image;
        this->pImage = pImage;
        this->pSettings = pSettings;
    }
    PixelSource source = PixelSource::Image;
    NXTImage *pImage = nullptr;
    NXTImageSettings *pSettings = nullptr;
    bool isVideo() const { return source==PixelSource::Video || source==PixelSource::VideoNext; }
    explicit operator bool() const {
        return ((pImage != nullptr && pSettings != nullptr) || isVideo());
    }
};

#endif /* defined(__NexVideoEditor__NXTImageSettings__) */
