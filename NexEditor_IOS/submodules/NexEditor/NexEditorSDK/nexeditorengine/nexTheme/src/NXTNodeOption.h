//
//  NXTNodeOption.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/14/14.
//
//

#ifndef __NexVideoEditor__NXTNodeOption__
#define __NexVideoEditor__NXTNodeOption__

#include <iostream>
#include "NXTNode.h"

class NXTNodeOption : public NXTNode {
    
public:
    
    virtual const char* nodeClassName() override  {
        return "Option";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="value" ) {
            // Nothing to do; this is handled in the UI side
        }
    }
    
};

#endif /* defined(__NexVideoEditor__NXTNodeOption__) */
