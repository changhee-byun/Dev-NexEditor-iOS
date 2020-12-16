//
//  NXTNodeTexture.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/23/14.
//
//

#include "NXTNodeTexture.h"

void NXTNodeTexture::renderBegin( IRenderContext& renderer ) {
    
    if( textureType==NXT_TextureType_File ) {
        
        if( texImage.texturesLoaded() )
            return;     // Nothing to do; correct texture is already loaded
        
        if( path.empty() )
            return;     // Can't load the texture if we didn't get a path
        
        // TODO: Handled cached case??
        if( pixelData.isCachedInternally() ) {
            texImage.setImageData(pixelData);
            pixelData.freePixelData();
        } else if(pixelData.loadPixelData(renderer, path, true)) {
            texImage.setImageData(pixelData);
            pixelData.freePixelData();
        }
        
    } else if ( textureType==NXT_TextureType_Overlay ) {
        
        if( !renderer.isEffectActive() )
            return;
        
        int renderChangeSerial = renderer.getActiveEffect().changeSerial;
        
        if( texImage.texturesLoaded() && renderChangeSerial==changeSerial && !bNoCache ) {
            // Nothing to do; correct texture is already loaded, and nothing that
            // would affect it has changed.
            return;
        }
        
        changeSerial = renderChangeSerial;
        
        if( renderer.getActiveEffect().effectOptions.count(srcfield) < 1 ) {
            return;
        }
        
        std::string overlayPath = "[Overlay]";
        overlayPath += renderer.getActiveEffect().effectOptions[srcfield];
        
        // TODO: Handled cached case??
        if(pixelData.loadPixelData(renderer, overlayPath, true)) {
            texImage.setImageData(pixelData);
            pixelData.freePixelData();
        }
        
    } else if ( textureType==NXT_TextureType_Text ) {
        
        if( !renderer.isEffectActive() )
            return;
        
        int renderChangeSerial = renderer.getActiveEffect().changeSerial;
        
        if( texImage.texturesLoaded() && renderChangeSerial==changeSerial && !bNoCache ) {
            // Nothing to do; correct texture is already loaded, and nothing that
            // would affect it has changed.
            return;
        }
        
        std::string baseId = "";
        for( NXTNode* node: parents() ) {
            if(node->is<NXTNodeTheme>() || node->is<NXTNodeEffect>())
                baseId = node->getId();
        }
        
        std::stringstream query;
        
        query << "[Text]baseid="  << baseId;
        query << ";typeface="     << textStyle.typeface;
        query << std::fixed       << std::setprecision(4);
        query << ";skewx="        << textStyle.textSkewX;
        query << ";scalex="       << textStyle.textScaleX;
        query << ";spacingmult="  << textStyle.spacingMult;
        query << ";spacingadd="   << textStyle.spacingAdd;
        query << ";size="         << textStyle.textSize;
        query << ";strokewidth="  << textStyle.textStrokeWidth;
        query << ";shadowradius=" << textStyle.shadowRadius;
        query << ";textblur="     << textStyle.textBlurRadius;
        query << ";blurtype="     << textStyle.blurType;
        query << ";margin"        << textStyle.textMargin;
        query << ";flags="        << std::hex << textStyle.textFlags << std::dec;
        query << ";align="        << textStyle.textAlign;
        query << ";longtext"      << textStyle.longText;
        query << ";maxlines"      << textStyle.maxLines;
        query << std::hex;
        query << ";fillcolor"     << NXT_Vector4f_ToARGBIntColor(textStyle.fillColor);
        query << ";strokecolor"   << NXT_Vector4f_ToARGBIntColor(textStyle.strokeColor);
        query << ";shadowcolor"   << NXT_Vector4f_ToARGBIntColor(textStyle.shadowColor);
        query << ";bgcolor"       << NXT_Vector4f_ToARGBIntColor(textStyle.bgColor);
        query << std::dec;
        query << ";shadowoffsx"   << textStyle.shadowOffset.e[0];
        query << ";shadowoffsy"   << textStyle.shadowOffset.e[1];
        query << ";width="        << textStyle.reqWidth;
        query << ";height="       << textStyle.reqHeight;
        query << ";text="         << text;
        query << ";;"             << renderer.getActiveEffect().effectOptions[srcfield];
        
        
        // TODO: Handled cached case??
        if(pixelData.loadPixelData(renderer, query.str(), true)) {
            texImage.setImageData(pixelData);
            pixelData.freePixelData();
        }
        
    }
}

void NXTNodeTexture::precache( IRenderContext& renderer, bool openGLContextAvailable ) {
    
    if( textureType==NXT_TextureType_File ) {
        
        if( texImage.texturesLoaded() )
            return;     // Nothing to do; correct texture is already loaded
        
        if( path.empty() )
            return;     // Can't load the texture if we didn't get a path
        
        if(pixelData.loadPixelData(renderer, path, true)) {
            if( openGLContextAvailable ) {
                texImage.setImageData(pixelData);
                pixelData.freePixelData();
            } else {
                pixelData.cacheInternally();
            }
        }
        
    }
}
