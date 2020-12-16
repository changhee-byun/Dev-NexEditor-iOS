//
//  NXTNodeFieldLabel.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/9/14.
//
//

#ifndef __NexVideoEditor__NXTNodeFieldLabel__
#define __NexVideoEditor__NXTNodeFieldLabel__

#include <iostream>
#include "NXTNode.h"

class NXTNodeFieldLabel : public NXTNode {
    
public:
    
    virtual const char* nodeClassName() override  {
        return "FieldLabel";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="value" ) {
            // Nothing to do; this is handled in the UI side
        }
    }
    
};

#endif /* defined(__NexVideoEditor__NXTNodeFieldLabel__) */
