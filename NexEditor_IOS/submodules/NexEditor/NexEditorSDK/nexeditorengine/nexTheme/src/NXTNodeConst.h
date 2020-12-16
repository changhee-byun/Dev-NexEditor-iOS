//
//  NXTNodeConst.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/17/14.
//
//

#ifndef __NexVideoEditor__NXTNodeConst__
#define __NexVideoEditor__NXTNodeConst__

#include <iostream>
#include "NXTNode.h"

class NXTNodeConst : public NXTNode {
    
private:
    const char* value = nullptr;
    
public:
    
    virtual const char* nodeClassName() override  {
        return "Const";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="value" ) {
            // TODO: Is this safe???
            value = attrValue.c_str();
        }
    }
    
    
};
#endif /* defined(__NexVideoEditor__NXTNodeConst__) */
