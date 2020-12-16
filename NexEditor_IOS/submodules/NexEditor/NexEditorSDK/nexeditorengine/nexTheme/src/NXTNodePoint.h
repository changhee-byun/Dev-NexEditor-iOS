#ifndef __NexVideoEditor__NXTNodePoint__
#define __NexVideoEditor__NXTNodePoint__

#include <iostream>
#include "NXTNode.h"

class NXTNodePoint : public NXTNode {
    
public:
    const char* location = nullptr;
    const char* texcoord = nullptr;
    const char* maskcoord = nullptr;
    const char* color = nullptr;
    const char* normal = nullptr;
    
public:
    
    static const std::string* CLASS_NAME() {
        static const std::string s("Point");
        return &s;
    }

    virtual const char* nodeClassName() override  {
        return "Point";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="location" ) {
            location = attrValue.c_str();
        } else if( attrName=="texcoord" ) {
            texcoord = attrValue.c_str();
        } else if( attrName=="maskcoord" ) {
            maskcoord = attrValue.c_str();
        } else if( attrName=="color" ) {
            color = attrValue.c_str();
        } else if( attrName=="normal" ) {
            normal = attrValue.c_str();
        }
    }
    
    
};

#endif /* defined(__NexVideoEditor__NXTNodePoint__) */
