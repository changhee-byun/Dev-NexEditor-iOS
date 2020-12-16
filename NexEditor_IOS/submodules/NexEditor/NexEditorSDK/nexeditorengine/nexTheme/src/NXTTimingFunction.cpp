//
//  NXTTimingFunctions.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/4/14.
//
//

#include "NXTTimingFunction.h"

static struct {
    std::string name;
    NXTTimingFunction::Type timingType;
    NXTVector t;
} table[] = {
    {"ease",        NXTTimingFunction::Type::CubicBezier, NXTVector( 0.25, 0.10, 0.25, 1.0 )},
    {"ease-in",     NXTTimingFunction::Type::CubicBezier, NXTVector( 0.42, 0.0,  1.0,  1.0 )},
    {"ease-out",    NXTTimingFunction::Type::CubicBezier, NXTVector( 0.0,  0.0,  0.58, 1.0 )},
    {"ease-in-out", NXTTimingFunction::Type::CubicBezier, NXTVector( 0.42, 0.0,  0.58, 1.0 )},
    {"linear",      NXTTimingFunction::Type::Linear,      NXTVector( 0.0,  0.0,  1.0,  1.0 )},
    {"default",     NXTTimingFunction::Type::CubicBezier, NXTVector( 0.25, 0.10, 0.25, 1.0 )}
};

void NXTTimingFunction::parse( NXTNode& baseNode, const std::string &timingFunctionSpec)
{
    
    if( timingFunctionSpec.empty() ) {
        timingCurve.set(0.0, 0.0, 1.0, 1.0);
        timingType = NXTTimingFunction::Type::Linear;
        return;
    }
    
    if( isNodeRef(timingFunctionSpec) ) {
        NXTNode* pNode = baseNode.findNodeById(timingFunctionSpec.substr(1));
        if( pNode ) {
            if(pNode->getDataProviderType() == NXTNode::DataProviderType::ConstString) {
                // TODO: (const and timing function as providers)
            }
        }
    }
    
    for( auto entry: table ) {
        if(entry.name==timingFunctionSpec ) {
            timingCurve = entry.t;
            timingType = entry.timingType;
            return;
        }
    }
    
    
    baseNode.floatVectorFromString(timingFunctionSpec,timingCurve);
    timingType = NXTTimingFunction::Type::CubicBezier;
}
