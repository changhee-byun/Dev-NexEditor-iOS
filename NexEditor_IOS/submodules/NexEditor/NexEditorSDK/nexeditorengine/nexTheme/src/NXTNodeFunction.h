//
//  NXTNodeFunction.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/10/14.
//
//

#ifndef __NexVideoEditor__NXTNodeFunction__
#define __NexVideoEditor__NXTNodeFunction__

#include <iostream>
#include <vector>
#include "NXTNode.h"
#include "NXTRenderer.h"

class NXTNodeFunction : public NXTNode {
    
private:

    NXT_Function        function        = NXT_Function_SimplexPure;
    NXT_Vector4f        inputBias       = (NXT_Vector4f){{0.0, 0.0, 0.0, 0.0}};
    NXT_Vector4f        inputScale      = (NXT_Vector4f){{1.0, 1.0, 1.0, 1.0}};
    NXT_Vector4f        outputBias      = (NXT_Vector4f){{0.0, 0.0, 0.0, 0.0}};;
    NXT_Vector4f        outputScale     = (NXT_Vector4f){{1.0, 1.0, 1.0, 1.0}};
    int                 octaves         = 8;
    bool                bNormalize      = true;

    std::vector<NXTUpdateTarget> updateTargets;

public:
    
    virtual const char* nodeClassName() override  {
        return "Function";
    }

    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName == "type" ) {
            if( attrValue == "simplexpure" ) {
                function = NXT_Function_SimplexPure;
            } else if( attrValue == "simplexfractal" ) {
                function = NXT_Function_SimplexFractal;
            } else if( attrValue == "simplexturbulence" ) {
                function = NXT_Function_SimplexTurbulence;
            } else if( attrValue == "simplexwavefractal" ) {
                function = NXT_Function_SimplexWaveFractal;
            } else {
                throw NXTNodeParseError("type must be one of: simplexpure,simplexfractal,simplexturbulence,simplexwavefractal");
            }
        } else if( attrName == "inputbias" ) {
            floatVectorFromString(attrValue, inputBias);
        } else if( attrName == "inputscale" ) {
            floatVectorFromString(attrValue, inputScale);
        } else if( attrName == "outputbias" ) {
            floatVectorFromString(attrValue, outputBias);
        } else if( attrName == "outputscale" ) {
            floatVectorFromString(attrValue, outputScale);
        } else if( attrName == "normalize" ) {
            parseBool(attrValue, bNormalize);
        } else if( attrName == "octaves" ) {
            intFromString(attrValue, octaves, 1, 32);
        }

    }
    
    virtual void renderBegin( IRenderContext& renderer ) override  {
        
    }
    
    virtual Action renderEnd( IRenderContext &renderer ) override  {
        return Action::Next;
    }
    
    virtual void updateNotify( int what ) override  {
        processIndividualUpdateTarget(updateTargets[what]);
    }

    int getElementCount() {
        return 1;
    }
    
    void processUpdateTargets( NXTRenderer *renderer ) {
        
        for( auto target: updateTargets ) {
            processIndividualUpdateTarget(target);
        }
        
    }
    
    virtual void addUpdateTarget(NXTUpdateTarget& updateTarget,
                                 std::string* params,
                                 NXTNode* paramContext ) override 
    {
        updateTargets.push_back(updateTarget);
        if( params!=nullptr ) {
            updateTargets[updateTargets.size()-1].parseParams(this,updateTargets.size()-1,paramContext,*params);
        }
    }
    
private:
    void processIndividualUpdateTarget( NXTUpdateTarget &target );

};

#endif /* defined(__NexVideoEditor__NXTNodeFunction__) */
