#ifndef __NexVideoEditor__NXTNodeKeyframe__
#define __NexVideoEditor__NXTNodeKeyframe__

#include <iostream>
#include "NXTNode.h"
#include "NXTTimingFunction.h"

class NXTNodeKeyframe : public NXTNode {
    
public:
    float               time                   = 0;
    NXTVector           value;
    int                 element_count           = 0;
    NXTTimingFunction   timingFunction;
    
public:
    
    static const std::string* CLASS_NAME() {
        static const std::string s("Keyframe");
        return &s;
    }

    virtual const char* nodeClassName() override  {
        return "Keyframe";
    }

    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName == "time" ) {
            floatVectorFromString(attrValue, time);
        } else if( attrName == "value" ) {
            element_count = floatVectorFromString(attrValue, value);
        } else if( attrName == "timingfunction" ) {
            timingFunction.setFromString(*this,attrValue);
//            timing = parseTimingFunction( attrValue );
        }
    }
    
//private:
//    NXT_Vector4f parseTimingFunction(std::string timingFunctionSpec );
    
};

#endif /* defined(__NexVideoEditor__NXTNodeKeyframe__) */
