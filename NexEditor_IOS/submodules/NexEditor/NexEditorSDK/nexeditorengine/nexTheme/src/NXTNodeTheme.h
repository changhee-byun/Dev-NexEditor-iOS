//
//  NXTNodeTheme.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/17/14.
//
//

#ifndef __NexVideoEditor__NXTNodeTheme__
#define __NexVideoEditor__NXTNodeTheme__

#include <iostream>
#include "NXTNode.h"

class NXTNodeTheme : public NXTNode {
    
private:
    std::string name;
    std::string thumbnailPath;
    
public:
    
    static const std::string* CLASS_NAME() {
        static const std::string s("Theme");
        return &s;
    }
    
    virtual const char* nodeClassName() override  {
        return "Theme";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="description" ) {
            name = attrValue;
        } else if( attrName=="thumbnail" ) {
            thumbnailPath = attrValue;
        }
    }
    
    
};

#endif /* defined(__NexVideoEditor__NXTNodeTheme__) */
