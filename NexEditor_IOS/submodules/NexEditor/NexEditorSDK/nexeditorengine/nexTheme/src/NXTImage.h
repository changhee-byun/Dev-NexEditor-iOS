//
//  NXTTexture.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/17/14.
//
//

#ifndef __NexVideoEditor__NXTTexture__
#define __NexVideoEditor__NXTTexture__

#include <iostream>
#include <GLES2/gl2.h>
#include <android/native_window_jni.h>
#include "NexThemeRenderer.h"
#include "NexTheme_Math.h"
#include "ITexture.h"

class NXTExternalPixelData;

class NXTImage : public ITexture {
    
public:
    
    NXTImage( const NXTImage& other ) = delete;
    NXTImage& operator= ( const NXTImage& other ) = delete;
    NXTImage() = default;
    
    ~NXTImage() {
    }

    // Returns true the textures required by this images have been loaded in OpenGL
    bool texturesLoaded() {
        return bTexturesLoaded;
    }
    
    // Returns true if this image supports text wrapping (rather than just clamping)
    // In order to allow wrapping, the image width and height must be a power of two
    bool allowsWrapping() {
        return false;
    }
    
    // Sets the pixel data for this image.
    //
    // If no OpenGL textures exist for this image, they are created
    // and the image data is set using glTexImage2D.  If OpenGL textures
    // already exists, they are updated using glTexSubImage2D.  Note that
    // if the previous pixel format and the current pixel format do not
    // match, any current textures will be destroyed and new textures
    // will be created.
    //
    // After calling this, texturesLoaded() will return true.
    void setImageData(int width,
                      int height,
                      int width_pitch,
                      int height_pitch,
                      NXT_PixelFormat pixelDataFormat,
                      void* data0,
                      void* data1 = nullptr,
                      void* data2 = nullptr );
    
    void setImageDataRGBA8888(int width,
                              int height,
                              int width_pitch,
                              int height_pitch,
                              void* pRGBA );
    
    void setImageDataLuminance(int width,
                               int height,
                               int width_pitch,
                               int height_pitch,
                               void* pY );
    
    void setImageDataYUV(int width,
                         int height,
                         int width_pitch,
                         int height_pitch,
                         void* pY,
                         void* pU,
                         void* pV );
    
    void setImageDataSurfaceTexture(int width,
                         int height,
                         int width_pitch,
                         int height_pitch,
                         ANativeWindow* pNativeWindow );
    
    void setImageDataNV12(int width,
                          int height,
                          int width_pitch,
                          int height_pitch,
                          bool useJPEGColorSpace,
                          void* pY,
                          void* pUV );
    
    void setImageData( NXTExternalPixelData& pixelData );
    
    
    // Frees and deletes any OpenGL textures used by this image.
    //
    // After calling this, texturesLoaded() will return false.
    void freeTextures();
    
    // Binds this image's textures on the GPU. There may be between one and three
    // textures requried depending on the texture format that was used to represent
    // the image (see getTextureFormat).
    //
    // Textures are bound to texture units as follows:
    //
    //                      RGB     YUV     NV12    SurfaceTexture
    //                      ------- ------- ------- ---------------
    //      GL_TEXTURE0  |  RGB     Y       Y       External
    //      GL_TEXTURE1  |          U       UV
    //      GL_TEXTURE2  |          V
    //
    virtual void bindTextures() override;
    
    // Gets the text format used by this image.  This may be different
    // from the original image data format.  This is the format used by
    // the OpenGL textures, and determines what shader to use to render
    // this image using the supplied texture or textures
    virtual ITexture::Format getTextureFormat() override;
    
    int getWidth() {
        return width;
    }
    
    int getHeight() {
        return height;
    }
    
    virtual NXTMatrix getMatrix() override {
        // TODO
        return NXTMatrix();
    }

    
private:
    
    // This is the internal format of the image.  This format determines which
    // shader (if any) is needed to convert this texture to RGB format.  Thus, different
    // source pixel formats (such as RGBA8888, RGB565 and LUMINANCE) may all be represented
    // by a single internal format (RGBA) if the representation in OpenGL is the same.
    Format internalFormat = Format::NONE;
    
    // True if this texture is loaded and available on the GPU.
    // When loading a texture, if this is already true, the existing
    // texture needs to be unloaded.  True implies that the GPU texture
    // names required by the format have been allocated, etc.  False
    // implies that no GPU texture names are currently in use.
    bool bTexturesLoaded = false;
    
    // The OpenGL texture names.  Valid only if bIsTextureLoaded==true.  The number of
    // elements used is based on NXTTexture::Format::reqTexNames.  Unused elements have
    // undefined values.
    GLuint texName[3];
    
    // Matrix to apply to the texture to map the entire usable area to the range
    // 0,0 -> 1,1 in texture coordinate space.  For example, if the texture contains
    // unused regions (video codec scratch area, etc.) this matrix would scale the
    // texture so that those regions fall outside the 0,0 -> 1,1 range.
    NXT_Matrix4f texMatrix;
    
    // The logical width and height (in pixels) of the area represented by texture
    // coordinates 0,0 -> 1,1 after applying texMatrix. Generally, this is the pixel
    // of the original image or video.
    int width;
    int height;

};

#endif /* defined(__NexVideoEditor__NXTTexture__) */

