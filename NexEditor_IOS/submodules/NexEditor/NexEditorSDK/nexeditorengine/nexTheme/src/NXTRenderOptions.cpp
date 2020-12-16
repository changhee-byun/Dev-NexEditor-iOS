//
//  NXTRenderOptions.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 12/1/14.
//
//

#include <string>
#include "NXTRenderOptions.h"

void NXTRenderOptions::parse(const char* pOptions) {
    
//    LOGI("[%s %d] NXTRenderOptions::parse : '%s'", __func__, __LINE__, pOptions);
    
    const char *pChar = pOptions;
    
    while( *pChar ) {
        
        int value = 1;
        
        while( *pChar == ' ' || *pChar == '\t' )
            pChar++;
        
        char tag[32] = {0};
        int taglen = 0;
        
        while( (*pChar >='a' && *pChar <='z') || (*pChar >='A' && *pChar <='Z') || (*pChar >='0' && *pChar <='9') || *pChar=='_' ) {
            if( taglen < sizeof(tag)-1 ) {
                tag[taglen++]=*pChar++;
            }
        }
        
        while( *pChar == ' ' || *pChar == '\t' )
            pChar++;
        
        if( *pChar=='=' ) {
            pChar++;
            
            if( *pChar=='{' ) {
                pChar++;
                
                float params[32] = {0};
                int paramCount;
                
                do {
                    
                    const char *st = pChar;
                    char *ed = 0;
                    
                    while( *pChar == ' ' || *pChar == '\t' )
                        pChar++;
                    
                    float f = strtof(pChar, &ed);
                    
                    pChar = ed;
                    
                    if( paramCount < 32 ) {
                        params[paramCount++] = f;
                    }
                    
                    while( *pChar == ' ' || *pChar == '\t' )
                        pChar++;
                    
                    while( *pChar == ',' )
                        pChar++;
                    
                    while( *pChar == ' ' || *pChar == '\t' )
                        pChar++;
                    
                    if( st==pChar ) {
                        // Malformed
                        break;
                    }
                    
                } while ( *pChar && *pChar!='}' );
                
                if( *pChar == '}' )
                    pChar++;
                
                if( paramCount > 0 ) {
                    handleFastFloatArrayPreviewOption(tag, paramCount, params);
                }
                continue;
                
            } else {
                
                while( *pChar == ' ' || *pChar == '\t' )
                    pChar++;
                
                value = 0;
                
                int sign = 1;
                
                if( *pChar=='-' ) {
                    pChar++;
                    sign = -1;
                }
                
                while( (*pChar >='0' && *pChar <='9') ) {
                    value *= 10;
                    value += ((*pChar++)-'0');
                }
                
                value *= sign;
                
            }
        }
        
        while( *pChar == ' ' || *pChar == '\t' )
            pChar++;
        
        if( taglen < 1 )
            break;
        
        handleFastPreviewOption(tag, value);
        
    }
//    LOGI("[%s %d] DONE parseFastPreviewOptions", __func__, __LINE__);
    
}


void NXTRenderOptions::handleFastFloatArrayPreviewOption( const char* optionName, int paramCount, float* params ) {
//    LOGI("[%s %d] handleFastFloatArrayPreviewOption('%s',%d)", __func__, __LINE__, optionName, paramCount);
    if( strcasecmp(optionName, "matrix")==0 ) {
        //        renderer->getVideoSrc(0).brightness = (float)value/255.0f;
    }
}


void NXTRenderOptions::handleFastPreviewOption( const char* optionName, int value ) {
    
//    LOGI("[%s %d] handleFastPreviewOption('%s',%d)", __func__, __LINE__, optionName, value);
    
    if( strcasecmp(optionName, "brightness")==0 ) {
        brightness_ = (float)value/255.0f;
    } else if( strcasecmp(optionName, "contrast")==0 ) {
        contrast_ = (float)value/255.0f;
    } else if( strcasecmp(optionName, "saturation")==0 ) {
        saturation_ = (float)value/255.0f;
    } else if( strcasecmp(optionName, "tintColor")==0 ) {
        tint_color_ = value;
    } else if( strcasecmp(optionName, "nofx")==0 ) {
        no_fx_ = value?true:false;
    }
    
}

