
#ifndef __NexVideoEditor__NXTNodeVisibility__
#define __NexVideoEditor__NXTNodeVisibility__

#include <iostream>
#include "NXTNode.h"

class NXTNodeVisibility : public NXTNode {
    
private:
    float startTime = 0.0f;
    float endTime = 1.0f;
    float check = 0;
	bool  bUseCheckValue = false;
    
public:
    
    virtual const char* nodeClassName() override  {
        return "Visibility";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName == "start" ) {
            animFloatVectorFromString( attrValue, startTime );
        } else if( attrName == "end" ) {
            animFloatVectorFromString( attrValue, endTime );
        } else if( attrName == "check" ) {
            animFloatVectorFromString( attrValue, check );
            bUseCheckValue = true;
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override  {
        if( bUseCheckValue ) {
            if( check < startTime || check > endTime ) {
                renderer.skipChildren();
            }
        } else {
            int time = renderer.getCurrentTime();
            if( time < startTime || time > endTime ) {
                renderer.skipChildren();
            }
        }
    }
    
};

#endif /* defined(__NexVideoEditor__NXTNodeVisibility__) */
