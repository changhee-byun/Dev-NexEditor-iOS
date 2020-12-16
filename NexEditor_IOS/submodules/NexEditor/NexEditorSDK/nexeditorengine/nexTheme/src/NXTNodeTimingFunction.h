//
//  NXTNodeTimingFunction.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/17/14.
//
//

#ifndef __NexVideoEditor__NXTNodeTimingFunction__
#define __NexVideoEditor__NXTNodeTimingFunction__

#include <iostream>
#include "NXTNode.h"

class NXTNodeTimingFunction : public NXTNode {
    
private:
    const char* values = nullptr;
    
public:
    
    virtual const char* nodeClassName() override  {
        return "TimingFunction";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="values" ) {
            values = attrValue.c_str();
            // TODO: Is this safe???
        }
    }
    
    
};

#endif /* defined(__NexVideoEditor__NXTNodeTimingFunction__) */
