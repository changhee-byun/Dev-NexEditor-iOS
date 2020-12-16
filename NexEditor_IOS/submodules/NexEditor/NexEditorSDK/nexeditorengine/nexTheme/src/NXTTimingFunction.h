//
//  NXTTimingFunctions.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/4/14.
//
//

#ifndef __NexVideoEditor__NXTTimingFunction__
#define __NexVideoEditor__NXTTimingFunction__

#include <iostream>
#include "NXTVector.h"
#include "NXTNode.h"

class NXTTimingFunction {
    
public:
    
    NXTTimingFunction() {
    }
    
    NXTTimingFunction(const float x0, const float y0, const float x1, const float y1) {
        timingCurve = NXTVector(x0,y0,x1,y1);
    }
    
    NXTTimingFunction( NXTNode& baseNode, const std::string& timingFunctionSpec ) {
        parse(baseNode, timingFunctionSpec);
    }
    
    void setFromString( NXTNode& baseNode, const std::string& timingFunctionSpec ) {
        parse(baseNode, timingFunctionSpec);
    }
    
    void setLinear() {
        timingType = NXTTimingFunction::Type::Linear;
        timingCurve = NXTVector( 0.0,  0.0,  1.0,  1.0 );
    }
    
    void setCubicBezier(const float x0, const float y0, const float x1, const float y1) {
        timingType = NXTTimingFunction::Type::CubicBezier;
        timingCurve = NXTVector(x0,y0,x1,y1);
    }
    
    enum class Type {
        Linear, CubicBezier
    };
    
    Type getType() {
        return timingType;
    }
    
    float operator[](const int index) {
        return timingCurve[index];
    }
    
private:
    NXTVector timingCurve;
    NXTTimingFunction::Type timingType = Type::CubicBezier;
//    void parse( const std::string& timingFunctionSpec );
    void parse( NXTNode& baseNode, const std::string &timingFunctionSpec);
};


#endif /* defined(__NexVideoEditor__NXTTimingFunction__) */
