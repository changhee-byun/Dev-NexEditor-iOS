//
//  NXTNodeThemeSet.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/17/14.
//
//

#ifndef __NexVideoEditor__NXTNodeThemeSet__
#define __NexVideoEditor__NXTNodeThemeSet__

#include <iostream>
#include "NXTNode.h"

class NXTNodeThemeSet : public NXTNode {
    
private:
    std::string name;
    std::string defaultTheme;
    std::string defaultTransition;
    std::string defaultTitle;
    
public:
    
    virtual const char* nodeClassName() override  {
        return "ThemeSet";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="name" ) {
            name = attrValue;
        } else if( attrName=="defaultTheme" ) {
            defaultTheme = attrValue;
        } else if( attrName=="defaultTitle" ) {
            defaultTitle = attrValue;
        } else if( attrName=="defaultTransition" ) {
            defaultTransition = attrValue;
        }
    }
    
    
};
#endif /* defined(__NexVideoEditor__NXTNodeThemeSet__) */
