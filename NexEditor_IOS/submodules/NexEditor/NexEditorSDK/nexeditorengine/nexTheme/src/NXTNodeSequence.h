//
//  NXTNodeSequence.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/7/14.
//
//

#ifndef __NexVideoEditor__NXTNodeSequence__
#define __NexVideoEditor__NXTNodeSequence__

#include <iostream>
#include <vector>
#include "NXTNode.h"
#include "NXTRenderer.h"

class NXTNodeSequence : public NXTNode {
    
private:
    
    std::vector<NXTUpdateTarget> updateTargets;
    NXTVector       start_ = NXTVector(0,0,0,0);
    NXTVector       end_ = NXTVector(1,1,1,1);
    int             seed1 = 9374513;
    int             seed2 = 3489572;
    int             count_ = 10;
    NXT_SeqType     type_ = NXT_SeqType_Interp;
    int             element_count = 0;
    
    int             repeats_done;
    NXT_RandSeed    cur_seed;

    
public:
    
    virtual const char* nodeClassName() override  {
        return "Sequence";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName == "start" ) {
            element_count = std::max(element_count, animFloatVectorFromString(attrValue, start_));
        } else if( attrName == "start" ) {
            element_count = std::max(element_count, animFloatVectorFromString(attrValue, start_));
        } else if( attrName == "count" ) {
            intFromString(attrValue, count_);
        } else if( attrName == "type" ) {
            if( attrValue == "random" ) {
                type_ = NXT_SeqType_Rand;
            } else if( attrValue == "randmix" ) {
                type_ = NXT_SeqType_RandMix;
            } else if( attrValue == "vrandom" ) {
                type_ = NXT_SeqType_VRand;
            } else if( attrValue == "vrandmix" ) {
                type_ = NXT_SeqType_VRandMix;
            } else if( attrValue == "linear" ) {
                type_ = NXT_SeqType_Interp;
            } else if( attrValue == "exp" ) {
                type_ = NXT_SeqType_Exp;
            } else {
                throw NXTNodeParseError("type must be one of: random,randmix,vrandom,vrandmix,linear,exp");
            }
        } else if( attrName == "seed" ) {
            long long int combined_seed = 0;
            intFromString(attrValue, combined_seed);
            seed1 = (combined_seed>>32) & 0xFFFFFFFFLL;
            seed2 = combined_seed & 0xFFFFFFFFLL;
            if( seed1==0 ) seed1 = 1;
            if( seed2==0 ) seed2 = seed1 * 3;
        }
        
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override;
    virtual Action renderEnd( IRenderContext& renderer ) override;
    
    int getElementCount() {
        return element_count;
    }
    
    virtual void addUpdateTarget(NXTUpdateTarget& updateTarget,
                                 std::string* params,
                                 NXTNode* paramContext ) override
    {
        updateTargets.push_back(updateTarget);
    }
    
private:
    
    void processUpdateTargets();
    
    
};

#endif /* defined(__NexVideoEditor__NXTNodeSequence__) */
